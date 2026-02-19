from fastapi import APIRouter, FastAPI, Response, HTTPException
import numpy as np

import spirepy
import os


app = FastAPI()
router = APIRouter(prefix="/compute")


@app.get("/healthz")
def healthz():
    # Basic liveness + minimal functional check that the native module loads.
    try:
        _ = spirepy.compute_height(1.0, 1.0, 1)
    except Exception as exc:
        raise HTTPException(status_code=503, detail=str(exc))
    return {"status": "ok", "pid": os.getpid()}


@router.post("/generate_spire_image")
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
    k: int = 0,
    l: int = 1,
    membrane_distance: float = 0.0,
    membrane_thickness: float = 0.02,
    image_depth: int = 76,
):
    # spirepy exports: compute_height(slice_height, slice_width, image_width)
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
        [membrane_distance, membrane_thickness],
        H,
        W,
        image_depth,
        filename,
    )

    # Return raw bytes; include shape as headers for easy reconstruction.
    return Response(
        content=image.tobytes(),
        media_type="application/octet-stream",
        headers={
            "X-Image-Width": str(W),
            "X-Image-Height": str(H),
            "X-DType": "uint8",
            "X-Worker-Pid": str(os.getpid()),
        },
    )


app.include_router(router)
