#!/usr/bin/env python3
import argparse
import os
import random
import time
from concurrent.futures import ThreadPoolExecutor, as_completed

from typing import Iterator, Optional

from spirepy_client import generate_spire_image, health_check


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description="Generate many SPIRE images concurrently via the API")
    p.add_argument("-n", "--num-samples", type=int, default=32, help="Number of images to generate")
    p.add_argument("-c", "--concurrency", type=int, default=8, help="Number of concurrent requests")
    p.add_argument("--W", type=int, default=256, help="Fixed image width (and height, since ratio=1)")
    p.add_argument("--timeout", type=float, default=120.0)
    p.add_argument("--seed", type=int, default=None)
    return p.parse_args()


def batch_generator(
    n: int,
    concurrency: int,
    W: int,
    timeout_s: float,
    seed: Optional[int],
) -> Iterator["object"]:
    """Yield a batch of images, generated concurrently via the API.

    Uses spirepy_client.generate_spire_image() so this is a reference example
    for consumers.
    """

    n = max(int(n), 1)
    concurrency = max(int(concurrency), 1)

    # Pre-create one RNG per worker thread (lower contention than a shared RNG).
    rngs = [
        random.Random((seed or 0) + i) if seed is not None else random.Random()
        for i in range(concurrency)
    ]

    def draw_params(rng: random.Random):
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
            "timeout_s": timeout_s,
        }

    def one(idx: int):
        slot = idx % concurrency
        params = draw_params(rngs[slot])
        return generate_spire_image(**params)

    ex = ThreadPoolExecutor(max_workers=concurrency)
    try:
        futures = [ex.submit(one, i) for i in range(n)]
        for fut in as_completed(futures):
            yield fut.result()
    finally:
        ex.shutdown(wait=True, cancel_futures=False)


def main() -> None:
    args = parse_args()

    if not health_check(timeout_s=5.0):
        raise SystemExit("SPIRE API health check failed")

    n = max(int(args.num_samples), 1)
    conc = max(int(args.concurrency), 1)
    W = int(args.W)

    t0 = time.perf_counter()
    count = 0
    for _img in batch_generator(n=n, concurrency=conc, W=W, timeout_s=args.timeout, seed=args.seed):
        count += 1
    t1 = time.perf_counter()

    total = max(t1 - t0, 1e-9)
    avg = total / count

    print("SPIRE multithread generator")
    print(f"- samples: {count}")
    print(f"- concurrency: {conc}")
    print(f"- image: {W}x{W} uint8")
    print(f"- total time: {total:.3f} s")
    print(f"- avg generation time: {avg*1000:.1f} ms")
    print(f"- throughput: {count / total:.2f} img/s")


if __name__ == "__main__":
    main()
