# Check if GUI genrated image is the same as `spirepy` generated (using client)

| GUI Generated | spirepy Generated |
|---|---|
| ![GUI Image](gui_image.png) | ![spirepy Image](spirepy_image.png) |

## GUI generation

Sourceforge GUI version, parameters stored in `gui_params.txt`

Output: `gui_image.png`

## `spirepy` generation

Run container
```
docker run --rm -p 8000:8000 spire:api
```

Generate image (in `SPIRE/client`)
```
python generate_single.py --output example/spirepy_image.png --W 256 --structure-type 1 --uc-scale-ab 0.4 --uc-scale-c 0.4 --channel-vol-prop 0.2 --slice-thickness 0.23 --h 0 --k 0 --l 1 --image-depth 75
```

## Exact comparison

In this directory:
```
python compare_images.py 
```