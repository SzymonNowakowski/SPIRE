import argparse
from pathlib import Path

from PIL import Image

from spirepy_client import SPIRE_HOST, SPIRE_PORT, generate_spire_image, health_check


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Generate a single SPIRE image via the SPIRE API",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("spire_image.png"),
        help="Output image file path",
    )

    # Output resolution
    parser.add_argument(
        "--W",
        type=int,
        default=256,
        help="Image width in pixels (API computes height from slice_height/slice_width)",
    )

    # Structure / geometry
    parser.add_argument(
        "--structure-type",
        type=int,
        default=1,
        help="Structure selector: 0=gyroid, 1=diamond, 2=primitive, ...",
    )

    parser.add_argument(
        "--uc-scale-ab",
        type=float,
        default=1.0,
        help="Unit-cell scaling factor in a/b directions",
    )
    parser.add_argument(
        "--uc-scale-c",
        type=float,
        default=1.0,
        help="Unit-cell scaling factor along c axis",
    )
    parser.add_argument(
        "--channel-vol-prop",
        type=float,
        default=0.5,
        help="Target channel volume fraction (0..1)",
    )

    # Slice / projection settings (in unit cells)
    parser.add_argument(
        "--slice-height",
        type=float,
        default=1.0,
        help="Physical slice height in unit cells (UC)",
    )
    parser.add_argument(
        "--slice-width",
        type=float,
        default=1.0,
        help="Physical slice width in unit cells (UC)",
    )
    parser.add_argument(
        "--slice-thickness",
        type=float,
        default=1.0,
        help="Projection depth / slice thickness in unit cells (UC)",
    )
    parser.add_argument(
        "--slice-position",
        type=float,
        default=0.0,
        help="Relative slice position (0=centered)",
    )

    # Orientation (Miller indices)
    parser.add_argument("--h", type=int, default=0, help="Miller index h")
    parser.add_argument("--k", type=int, default=1, help="Miller index k")
    parser.add_argument("--l", type=int, default=1, help="Miller index l")

    # Sampling density
    parser.add_argument(
        "--image-depth",
        type=int,
        default=76,
        help="Sampling density along projection axis (higher = slower, more accurate)",
    )

    # Client-side options
    parser.add_argument(
        "--timeout",
        type=float,
        default=120.0,
        help="HTTP timeout in seconds",
    )
    return parser.parse_args()


if __name__ == "__main__":
    args = parse_args()

    is_healthy = health_check()
    if is_healthy:
        print(f"SPIRE API is running at {SPIRE_HOST}:{SPIRE_PORT}")
    else:
        raise SystemExit(f"SPIRE API failed, running at {SPIRE_HOST}:{SPIRE_PORT}")

    image = generate_spire_image(
        W=args.W,
        structure_type=args.structure_type,
        uc_scale_ab=args.uc_scale_ab,
        uc_scale_c=args.uc_scale_c,
        channel_vol_prop=args.channel_vol_prop,
        slice_height=args.slice_height,
        slice_width=args.slice_width,
        slice_thickness=args.slice_thickness,
        slice_position=args.slice_position,
        h=args.h,
        k=args.k,
        l=args.l,
        image_depth=args.image_depth,
        timeout_s=args.timeout,
    )

    args.output.parent.mkdir(parents=True, exist_ok=True)
    Image.fromarray(image).save(args.output)
    print(f"Image saved to {args.output}")