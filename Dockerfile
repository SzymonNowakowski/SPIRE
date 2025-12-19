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
# ----------------------------------------------------------
RUN python3 - <<EOF
import site, glob, shutil, os

sd = site.getsitepackages()[0]

# find the python-generated module name, whatever it is
matches = glob.glob("/opt/spire/build/spirepy*.so")
if not matches:
    raise SystemExit("NO spirepy .so FOUND in /opt/spire/build — build failed")

src = matches[0]

dst = os.path.join(sd, "spirepy.so")
shutil.copy2(src, dst)

print("Installed:", dst)
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
