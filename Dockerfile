FROM nvcr.io/nvidia/pytorch:25.11-py3

ENV DEBIAN_FRONTEND=noninteractive

# ----------------------------------------------------------
# 1. System dependencies (no GUI, no QT)
# ----------------------------------------------------------
RUN apt-get update && apt-get install -y \
    build-essential cmake ninja-build pkg-config \
    libopenblas-dev libpng-dev libcgal-dev \
    libgmp-dev libmpfr-dev \
    python3-dev python3-pip python3-setuptools \
    && rm -rf /var/lib/apt/lists/*

# Upgrade pip and install pybind11
RUN python3 -m pip install --upgrade pip pybind11

# ----------------------------------------------------------
# 2. Copy SPIRE source
# ----------------------------------------------------------
WORKDIR /opt/spire
COPY . /opt/spire

# ----------------------------------------------------------
# 3. Configure & build (NO Qt GUI)
# ----------------------------------------------------------
RUN cmake -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_QT_GUI=OFF \
    -DBUILD_SPIRE_PY=ON \
    -DPython3_EXECUTABLE=/usr/bin/python3

RUN cmake --build build --parallel $(nproc)

# ----------------------------------------------------------
# 4. Install spirepy *.so module into Python site-packages
#  Note there is only Python 3.10 in this base image. Also, spirepy.so is symlinked to spirepy310.so
# ----------------------------------------------------------
RUN python3 - <<EOF
import site, glob, shutil, os

sd = site.getsitepackages()[0]

# Find versioned module (spirepy312.so)
so = glob.glob("/opt/spire/build/spirepy312.so")[0]

# Install as spirepy312.so
dst1 = os.path.join(sd, os.path.basename(so))
shutil.copy2(so, dst1)

# Create generic symlink: spirepy.so -> spirepy312.so
dst2 = os.path.join(sd, "spirepy.so")
if not os.path.exists(dst2):
    os.symlink(os.path.basename(dst1), dst2)

print("Installed:", dst1)
print("Symlink:", dst2)
EOF


# ----------------------------------------------------------
# 5. Test import (optional, remove for production)
# ----------------------------------------------------------
RUN python3 - <<EOF
import torch
import spirepy
print("PyTorch:", torch.__version__)
print("spirepy OK, methods:", dir(spirepy)[:8])
EOF

CMD ["/bin/bash"]
