"""
Usage: 
docker run -it -v /data_hdd/projects/049/SPIRE:/spire --rm spire:bash python3 /spire/scripts/multithread_generator_in_container.py -n 256 -c 48 --W 256 --seed 123 --backend process
"""


import argparse
import random
import time
import multiprocessing as mp
from concurrent.futures import ProcessPoolExecutor, ThreadPoolExecutor
import itertools

from typing import Any, Dict, Iterator, Optional, Union
import spirepy
import numpy as np

# from spirepy_client import generate_spire_image, health_check


def _worker_generate(params: Dict[str, Any], discard_images: bool) -> Union[np.ndarray, int]:
    """Process/thread worker entrypoint (must be top-level for multiprocessing)."""
    img = generate_spire_image(**params)
    if discard_images:
        # Avoid transferring large arrays across processes.
        return 1
    return img

def generate_spire_image(
    W: int = 256,
    structure_type: int = 1,
    uc_scale_ab: float = 1.0,
    uc_scale_c: float = 1.0,
    channel_vol_prop: float = 0.5,
    slice_height: float = 1.0,
    slice_width: float = 1.0,
    slice_thickness: float = 1.0,
    slice_position: float = 0.0,
    h: int = 0,
    k: int = 1,
    l: int = 1,
    image_depth: int = 76,
):
    H = spirepy.compute_height(slice_height, slice_width, W)
    image = np.empty((H, W), dtype=np.uint8)

    # No file output
    filename = ""

    spirepy.generate_spire_image(
        image,
        structure_type,
        uc_scale_ab,
        uc_scale_c,
        channel_vol_prop,
        slice_height,
        slice_width,
        slice_thickness,
        slice_position,
        h,
        k,
        l,
        H,
        W,
        image_depth,
        filename,
    )
    return image

def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description="Generate many SPIRE images concurrently")
    p.add_argument("-n", "--num-samples", type=int, default=32, help="Number of images to generate")
    p.add_argument("-c", "--concurrency", type=int, default=8, help="Number of concurrent requests")
    p.add_argument("--W", type=int, default=256, help="Fixed image width (and height, since ratio=1)")
    p.add_argument("--seed", type=int, default=None)
    p.add_argument(
        "--backend",
        choices=["thread", "process"],
        default="thread",
        help="Concurrency backend. 'process' enables true parallel CPU execution (bypasses GIL).",
    )
    p.add_argument(
        "--discard-images",
        action="store_true",
        help="Do not return numpy arrays from workers (faster for process backend; main still counts samples).",
    )
    return p.parse_args()


def batch_generator(
    n: int,
    concurrency: int,
    W: int,
    seed: Optional[int],
    backend: str = "thread",
    discard_images: bool = False,
) -> Iterator[Union[np.ndarray, int]]:
    """Yield a batch of images, generated concurrently.

    Notes on determinism:
    - Parameter generation is done on the main thread using a single RNG.
    - Worker threads only run `generate_spire_image(**params)`.

    This avoids nondeterminism that comes from trying to associate RNGs with
    worker threads (tasks are not guaranteed to run on a specific thread).
    """

    n = max(int(n), 1)
    concurrency = max(int(concurrency), 1)
    backend = str(backend)

    def draw_params(rng: random.Random) -> Dict[str, Any]:
        structure_type = rng.choice([0, 1, 2])
        uc_scale_ab = rng.uniform(0.6, 2.0)
        uc_scale_c = rng.uniform(0.6, 2.0)
        channel_vol_prop = rng.uniform(0.15, 0.85)

        # Fix H == W by enforcing slice_height/slice_width == 1.
        slice_width = rng.uniform(0.8, 3.0)
        slice_height = slice_width

        slice_thickness = rng.uniform(0.5, 2.0)
        slice_position = rng.uniform(-0.4, 0.4)

        h, k, l = rng.choice([(0, 0, 1), (0, 1, 1), (1, 0, 1), (1, 1, 1)])
        image_depth = rng.choice([50, 76, 100, 128])

        return {
            "W": W,
            "structure_type": structure_type,
            "uc_scale_ab": uc_scale_ab,
            "uc_scale_c": uc_scale_c,
            "channel_vol_prop": channel_vol_prop,
            "slice_height": slice_height,
            "slice_width": slice_width,
            "slice_thickness": slice_thickness,
            "slice_position": slice_position,
            "h": h,
            "k": k,
            "l": l,
            "image_depth": image_depth,
        }

    # Generate all parameters deterministically up front (seeded), then fan out.
    rng = random.Random(seed) if seed is not None else random.Random()
    params_list = [draw_params(rng) for _ in range(n)]

    max_workers = min(concurrency, n)

    # executor.map preserves input order (deterministic) while still executing concurrently.
    if backend == "process":
        # 'spawn' is safer with native extensions than 'fork' (at some cost).
        ctx = mp.get_context("spawn")
        with ProcessPoolExecutor(max_workers=max_workers, mp_context=ctx) as ex:
            for item in ex.map(
                _worker_generate,
                params_list,
                itertools.repeat(discard_images),
                chunksize=1,
            ):
                yield item
    else:
        with ThreadPoolExecutor(max_workers=max_workers) as ex:
            for item in ex.map(_worker_generate, params_list, itertools.repeat(discard_images)):
                yield item


def main() -> None:
    args = parse_args()

    # if not health_check(timeout_s=5.0):
    #     raise SystemExit("SPIRE API health check failed")

    n = max(int(args.num_samples), 1)
    conc = max(int(args.concurrency), 1)
    W = int(args.W)

    t0 = time.perf_counter()
    count = 0
    for _img in batch_generator(
        n=n,
        concurrency=conc,
        W=W,
        seed=args.seed,
        backend=args.backend,
        discard_images=bool(args.discard_images),
    ):
        count += 1
    t1 = time.perf_counter()

    total = max(t1 - t0, 1e-9)
    avg = total / count

    print("SPIRE multithread generator")
    print(f"- samples: {count}")
    print(f"- concurrency: {conc}")
    print(f"- backend: {args.backend}")
    print(f"- discard images: {bool(args.discard_images)}")
    print(f"- image: {W}x{W} uint8")
    print(f"- total time: {total:.3f} s")
    print(f"- avg generation time: {avg*1000:.1f} ms")
    print(f"- throughput: {count / total:.2f} img/s")


if __name__ == "__main__":
    main()
