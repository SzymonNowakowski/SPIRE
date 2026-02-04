# SPIRE: Structure Projection Image Recognition Environment - Python microservise

A tool to compute planar projections of 3D surfaces

License: GPLv3



## Python via API from container

### Build container

```bash
docker build --target api -t spire:api .
```

### Usage

1. Start server with up to 8 workers:
  ```bash
  export NUM_WORKERS=8
  docker run --rm -p 8000:8000  -e UVICORN_WORKERS=$NUM_WORKERS spire:api
  ```
2. Install requirements for client from `client/requirements.txt`: `numpy`, `pillow`, `requests`
  ```
  pip install -r  client/requirements.txt
  ```
3. Generate default image:
  ```
  cd client
  python example.py
  ```
4. Example of a multi thread generator and speed benchmarking
  ```
  cd client
  python multithread_generator.py -n 128 -c 16 --W 256 --seed 123
  ```
  where concurrency (`-c`) should be not smaller than number of workers in order to take advantage of available cores

## GUI from container

### Build container

```bash
docker build --target gui -t spire:gui .
```

### Usage

1. Start container
  ```bash
  export GUI_PORT=8001
  docker run --rm -p $GUI_PORT:6080 spire:gui
  ```
2. Open: `http://localhost:GUI_PORT/vnc.html`, e.g. `http://localhost:8001/vnc.html`

## Work in container

### Build container
```bash
docker build --target bash -t spire:bash .
```

### Usage

- Interactively (`torch` version incompatible with my GPU)
  ```bash
  docker run --rm -it --gpus all spire:bash 
  ```
- Run a height calculation:
  ```bash
  docker run --rm spire:bash \
  bash -c 'python3 - <<EOF 2>/dev/null
  import spirepy
  print("H =", spirepy.compute_height(2.0, 2.0, 256))
  EOF'
  ```
- Generate an image buffer and show information about it:

  ```bash
  docker run --rm snowakowski/spirepy:latest \
  bash -c 'python3 - <<EOF
  import numpy as np, spirepy
  
  W = 256
  H = spirepy.compute_height(2.0, 2.0, W)
  
  buf = np.empty((H, W), dtype=np.uint8)
  spirepy.generate_spire_image(
      buf,
      1,           # structure type: diamond
      1.0,         # unit cell scale factors
      1.0,         # unit cell scale in Z direction (1.0) 
      0.5,         # channel volume proportion
      2.0, 2.0,    # slice height and width (image proportions)
      1.0,         # slice thickness
      0.0,         # slice position (slice shift)
      0, 1, 1,     # h, k, l
      [0.0, 0.02], # membrane distance and width
      H, W,        # image height and width, should keep image proportion specified by slice height and width
      76,          # image quality (voxels along Z axis)
      ""
  )
  
  print("Image shape:", buf.shape)
  print("First pixels:", buf.flatten()[:16])
  EOF'
  ```
- Generate and image and apply algorithmic distortions:
  - **Blur**

    ```{bash}
    docker run --rm snowakowski/spirepy:latest \
    bash -c 'python3 - <<EOF
    import numpy as np, spirepy

    W = 256
    H = spirepy.compute_height(2.0, 2.0, W)
  
    buf = np.empty((H, W), dtype=np.uint8)
    spirepy.generate_spire_image(
        buf,
        1,           # structure type: diamond
        1.0,         # unit cell scale factors
        1.0,         # unit cell scale in Z direction (1.0) 
        0.5,         # channel volume proportion
        2.0, 2.0,    # slice height and width (image proportions)
        1.0,         # slice thickness
        0.0,         # slice position (slice shift)
        0, 1, 1,     # h, k, l
        [0.0, 0.02], # membrane distance and width
        H, W,        # image height and width, should keep image proportion specified by slice height and width
        76,          # image quality (voxels along Z axis)
        ""
    )
  
    print("Before blur:", buf.flatten()[:16])

    spirepy.apply_gaussian_blur(
    buf,
    W, H,
    7,   # kernel_size (odd)
    )

    print("After blur:", buf.flatten()[:16])
    EOF'
    ```

  - **Noise**

    ```bash
    docker run --rm snowakowski/spirepy:latest \
    bash -c 'python3 - <<EOF
    import numpy as np, spirepy

    W = 256
    H = spirepy.compute_height(2.0, 2.0, W)
  
    buf = np.empty((H, W), dtype=np.uint8)
    spirepy.generate_spire_image(
        buf,
        1,           # structure type: diamond
        1.0,         # unit cell scale factors
        1.0,         # unit cell scale in Z direction (1.0) 
        0.5,         # channel volume proportion
        2.0, 2.0,    # slice height and width (image proportions)
        1.0,         # slice thickness
        0.0,         # slice position (slice shift)
        0, 1, 1,     # h, k, l
        [0.0, 0.02], # membrane distance and width
        H, W,        # image height and width, should keep image proportion specified by slice height and width
        76,          # image quality (voxels along Z axis)
        ""
    )
  
    print("Before noise:", buf.flatten()[:16])

    spirepy.add_gaussian_noise(
        buf,
        W, H,
        20.0   # magnitude (stddev)
    )

    print("After noise:", buf.flatten()[:16])
    EOF'
    ```

  - **Grains**

    ```bash
    docker run --rm snowakowski/spirepy:latest \
    bash -c 'python3 - <<EOF
    import numpy as np, spirepy

    W = 256
    H = spirepy.compute_height(2.0, 2.0, W)
  
    buf = np.empty((H, W), dtype=np.uint8)
    spirepy.generate_spire_image(
        buf,
        1,           # structure type: diamond
        1.0,         # unit cell scale factors
        1.0,         # unit cell scale in Z direction (1.0) 
        0.5,         # channel volume proportion
        2.0, 2.0,    # slice height and width (image proportions)
        1.0,         # slice thickness
        0.0,         # slice position (slice shift)
        0, 1, 1,     # h, k, l
        [0.0, 0.02], # membrane distance and width
        H, W,        # image height and width, should keep image proportion specified by slice height and width
        76,          # image quality (voxels along Z axis)
        ""
    )
  
    print("Before grains:", buf.flatten()[:16])

    spirepy.add_grains(
        buf,
        W, H,
        50,   # grain_size_center
        10,   # grain_size_std_dev
        50,   # grain_number_center
        1,    # grain_number_std_dev
        30,   # magnitude
        4,    # blur kernel size for grains image
        0.5   # original_image_proportion_to_the_grains_image
    )

    print("After grains:", buf.flatten()[:16])
    EOF'
    ```