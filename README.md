# SPIRE: Structure Projection Image Recognition Environment

A tool to compute planar projections of 3D surfaces

License: GPLv3

## Install

### Docker Container

A pre-built docker container with the `spirepy` Python module is available on DockerHub:

```bash
docker pull snowakowski/spirepy:latest
```

or you can consider downloading the dated tag with an explicit predefined Ubuntu and Python versions:

```bash
docker pull snowakowski/spirepy:U24.04.3_P3.12_20251219
```

For building it yourself from the provided Dockerfile:

```bash
docker build -t spirepy:latest .
```

#### Usage Examples

- Run a height calculation:

  ```bash
  docker run --rm snowakowski/spirepy:latest \
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
      1,          # structure type: diamond
      1.0,        # unit cell scale factors
      1,0,        # unit cell scale in Z direction (1.0) 
      0.5,        # channel volume proportion
      2.0, 2.0,   # slice height and width (image proportions)
      1.0,        # slice thickness
      0.0,        # slice position (slice shift)
      0, 1, 1,    # h, k, l
      [0.0, 5.0], # membrane distance and width
      H, W,       # image height and width, should keep image proportion specified by slice height and width
      76,         # image quality (voxels along Z axis)
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
        1,          # structure type: diamond
        1.0,        # unit cell scale factors
        1,0,        # unit cell scale in Z direction (1.0) 
        0.5,        # channel volume proportion
        2.0, 2.0,   # slice height and width (image proportions)
        1.0,        # slice thickness
        0.0,        # slice position (slice shift)
        0, 1, 1,    # h, k, l
        [0.0, 5.0], # membrane distance and width
        H, W,       # image height and width, should keep image proportion specified by slice height and width
        76,         # image quality (voxels along Z axis)
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
        1,          # structure type: diamond
        1.0,        # unit cell scale factors
        1,0,        # unit cell scale in Z direction (1.0) 
        0.5,        # channel volume proportion
        2.0, 2.0,   # slice height and width (image proportions)
        1.0,        # slice thickness
        0.0,        # slice position (slice shift)
        0, 1, 1,    # h, k, l
        [0.0, 5.0], # membrane distance and width
        H, W,       # image height and width, should keep image proportion specified by slice height and width
        76,         # image quality (voxels along Z axis)
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
        1,          # structure type: diamond
        1.0,        # unit cell scale factors
        1,0,        # unit cell scale in Z direction (1.0) 
        0.5,        # channel volume proportion
        2.0, 2.0,   # slice height and width (image proportions)
        1.0,        # slice thickness
        0.0,        # slice position (slice shift)
        0, 1, 1,    # h, k, l
        [0.0, 5.0], # membrane distance and width
        H, W,       # image height and width, should keep image proportion specified by slice height and width
        76,         # image quality (voxels along Z axis)
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
### Linux/Unix (and also Mac)

This software uses CMake as build system. To build from the sources, following packages are needed:

- CMake (tested with 3.16.3)
- C++ compiler with C++ 2011 support
- Qt min vers. 6.9.3 ( and its dependencies ). Components required: Widgets, Gui, Svg
- libpng and libz
- libgmp
- libgmpfr
- Integer Matrix Library (included)
- any BLAS system (openblas, intel mkl, ...)

Optional:

- CGAL (tested with 4.14) (and its dependencies)

The code will build and have most of its functionality without CGAL, some measurements (surface area of membranes) can **NOT** be computed correctly.

To build, navigate to the root directory of the git repository and execute:

```
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_QT_GUI=On ../
make
```

If successfull, the compiled binary ( "SPIRE" ) should be in the directory <sources>/build/SPIRE.

There are some options to be used with the cmake command to be considered:

```-DBUILD_QT_GUI=ON(OFF)```: build the QT GUI tool

```-DBUILD_BATCH_TOOL=ON(OFF)``` build the batch tool for command line

```-DBUILD_SPIRE_PY=ON(OFF)``` build the `spirepy` python module



### Windows


Pre-compiled Windows binaries, including all dependencies, are available online. Just extract the archive and run the executable.

If you wish to compile from source and need help, please contact hain [at] uni-potsdam.de . I used QT Creator with mingw to compile these binaries, so if you are using that I could maybe help you with that.

#### CLion with MinGW

The sources can also be compiled in CLion using MinGW.

#### CLion CMake requires additional options for MinGW

These are the basic options (below, you can also use `OFF` instead of `ON`):
```{bash}
--DBUILD_QT_GUI=ON -DBUILD_BATCH_TOOL=ON -DBUILD_SPIRE_PY=ON
```

These additional options are required. They should follow the following template:
```{bash}
-DQt6_DIR=C:/Qt/6.9.3/mingw_64/lib/cmake/Qt6 -DCMAKE_TOOLCHAIN_FILE=C:\Users\<username>\.vcpkg-clion\vcpkg\scripts\buildsystems\vcpkg.cmake -DCMAKE_PREFIX_PATH="C:\Qt\6.9.3\mingw_64"
```

#### CLion with MSVS

There is an (apparently unsolvable) issue when compiling with Microsoft Visual Studio (MSVC): due to an ABI incompatibility, the IML library which is written in pure C and assumes 32-bit `long` types, leads to conflicts with the C++ routines that invoke it.


## Usage

The main window is split in two: the right panel shows the projected
image of the 3D surface as well as some information about the
structure and the projection, the left panel contains several tabs
controlling the tool: tune parameters, run measurements, bulk creation
of projections and export and saving of data and images.

### General concepts

For details about the concepts of compute projections we refer to our
publication XXX. Here only the most important concepts are provided.

#### Units

The only relevant unit in the software is that of a length. This is
not fixed to a specific value, however, all values representing a
length are provided consistently. That means if one length value is
interpreted as being in nanometers, each other length value has to be
read as nanometers as well. For example, if the unit cell scale factor
is set to 5nm, then the slice dimension parameters has to be read to
be in nanometers as well.

### Status Bar

The status bar provides important information about the currently
displayed structure
- The size of the primitive unit cell of the current structure in length units
- The orientation of the normal vector onto the slice as rotation angles in radians
- The resolution of the voxelized slice, as well as the size of a single voxel in length units. Note that the resolution of the projection is always identical to the X and Y resolution of the voxelized slice
- Unit cell size in the current orientation
- Two status indicators (projection, measurement): if red, the software is either busy computing a projection or measuring

While the software is still responsive and allows changes while
computing projections or measuring, additional computations will only
be queued. We strongly recommend not to start another compuatation
until the software finsihed its current task.

### Control Tabs

#### Parameters

This is the main view of the software. Here each parameter can be
individually tuned. Here only information relevant to the usage of the
GUI will be provided about the parameters. For a more comprehensive
explanation of how they are incorporated into the projection see XXX.

##### Unit Cell Scale Factor

This factor determines the size of the primitive unit cell. Each
surface type in the software has a specific primitive unit cell with a
fixed proportion of edge lengths in each dimension. Structures with a
cubic symmtery must have a cubic unit cell. This parameter allows to
scale that intrinsic primitive unit cell size to any desired
value. Note, that for surfaces with cubic symmetries the unit cell
scale factor in z direction must be identical to the on in xy
direction to keep a cubic unit cell. It is thus deactivated for
structures with cubic symmetries.

##### Surface control parameter

The position of the inital membrane can be tuned using the
mathematical level-set value, or the volume proportion of the two
channels. The value here is interpreted as either, depending on the
choice made here. Note that the selection here will carry over to the
batch creation tab, i.e. if the projection is tuned using the volume
proportion value, so will be the projections in the batch creation
tab.


##### Slice dimensions

These parameters control the dimensions in the slice and are given in
length units.

##### Orientation

The hkl value provide the orientation of the slice. The small sketch
next to it visualises the orientation of the slice inside a single
unit cell of the structure. The red line is the normal vector, the
green and yello lines represent the inplane lattice vectors. (see XXX
for details)

##### Resolution

WARNING: chosing a high resolution may result in a extremely high
deman in memory an may crash your computer. So please be aware and
reasonable to choose the resolution


##### Autoupdate

If this box is ticked, the software will automatically recompute the
projection as soon as a parameter is changed. Note, that we strongly
discourage to use this feature for high resolutions, since it wil
result in a slow and poor user experience.

##### Render Parameters

If checked, all parameters used to create this projection will be
written into a margin in the saved pixel image of the projection

#### Measurements

This tabs contains a list of measurement about the current
structure. The buttons on the bottom allow to compute some
quantities. Depending on the choice of the dropdown menue on the
bottom left it computes the latte for the entire slice or just for a
single, primitive unit cell.

Note, that the measurements can only be accurate down to a couple of
voxel sizes. As a help to judge this, the dimension of the voxels used
to compute the quantities are listed below the tables containing the
measurement quantities.

The software does *NOT* use the resolution from the parameters tab,
but rather a predefined resolution for which the algorithm have the
best cost-yield efficancy. They may be tuned in the config file found
in the same directoy as the binary.

#### Batch creation

This tab allows you to automate bulk creation of projections. You can
provide parameter list ( provide a list of comma sperated values,
e.g. 1,2,3,4, ) or a range ( start:stop:step, e.g. 0:1:0.2 will sweep
through the the value 0,0.2,0.4,0.6,0.8.1 ). Surface type only accept
strings of the options provided in the dropdown menue in the parameter
tab. The surface control parameter will be the same as chosen in the
parameter tab. So if you chose "level-set" in the parameter tab, the
same parameter will be presented in the bulk tab.

All files will be name using a prefix and an incremental integer. If
you chose prefix as "projection" the files will be called
"projection_1.png", "projection_2.png", "projection_3.png", .... An
overview file called "[prefix]_summary.txt" will be generated
containing a list of the parameters used in each file.

The option "render parameters" writes the parameters in the marign of
the projections. In its basic form only the parameters which will
differ between two projections will be written, enabling the option
"render all parameters" will write *ALL* relevant paramters in the
margins.

#### Export

This tab is not needed for the usage of SPIRE. However, it allows you
to export the 3D voxel array to a text file with the following
structure: ```pos_x pos_y pos_y marked? channel_number``` You can use
this file to further analyse or visualise the three dimensional
structure in the slixe

## Config File

There is a small config file, called ```global_settings.conf ``` in
which some advanced options can be set to. The file needs to be placed
in the same directory as the executable binary. If it is not found,
standard values will be used. The options with a short description is
listed in the config file. Only change this if you are sure what you
are doing!
