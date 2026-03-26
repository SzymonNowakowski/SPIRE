FROM python:3.11-slim-bookworm AS base

ENV DEBIAN_FRONTEND=noninteractive

# ----------------------------------------------------------
# Common dependencies (shared by all targets)
# ----------------------------------------------------------
RUN apt-get update && apt-get install -y \
    build-essential cmake ninja-build pkg-config git \
    libopenblas-dev libpng-dev libcgal-dev \
    libgmp-dev libmpfr-dev \
    python3-dev \
    && rm -rf /var/lib/apt/lists/*

# Upgrade pip and install pybind11
RUN python3 -m pip install --upgrade pip pybind11

# ----------------------------------------------------------
# General Python ML packages (shared; keeps python usage consistent)
# ----------------------------------------------------------
RUN pip install numpy==2.4.2

# ----------------------------------------------------------
# Copy SPIRE source
# ----------------------------------------------------------
WORKDIR /opt/spire
COPY . /opt/spire


# ==========================================================
# Target: bash (default)
# - Builds Python wrapper (spirepy)
# - Does NOT build the Qt GUI
# ==========================================================
FROM base AS bash

RUN cmake -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_QT_GUI=OFF \
    -DBUILD_SPIRE_PY=ON \
    -DPython3_EXECUTABLE=/usr/bin/python3

RUN cmake --build build --parallel $(nproc)

RUN python3 - <<EOF
import site, glob, shutil, os

sd = site.getsitepackages()[0]
matches = glob.glob("/opt/spire/build/spirepy*.so")
if not matches:
    raise SystemExit("NO spirepy .so FOUND in /opt/spire/build — build failed")

src = matches[0]
dst = os.path.join(sd, "spirepy.so")
shutil.copy2(src, dst)
print("Installed:", dst)
EOF

RUN python3 - <<EOF
import spirepy
print("spirepy OK")
EOF

CMD ["/bin/bash"]


# ==========================================================
# Target: gui
# - Adds Qt6 + Xvfb/x11vnc/noVNC
# - Builds SPIRE GUI + spirepy
# - Serves GUI via http://localhost:6080/vnc.html
# ==========================================================
FROM base AS gui

RUN apt-get update && apt-get install -y \
    qt6-base-dev qt6-base-dev-tools qt6-svg-dev \
    xvfb x11vnc novnc websockify \
    fonts-dejavu \
    libgl1 libegl1 \
    libxkbcommon-x11-0 \
    libxcb-cursor0 \
    libxcb-icccm4 libxcb-image0 libxcb-keysyms1 libxcb-randr0 \
    libxcb-render-util0 libxcb-shape0 libxcb-xfixes0 libxcb-xinerama0 \
    libxrender1 libxext6 libxi6 libxrandr2 libxss1 libxtst6 \
    libdbus-1-3 \
    && rm -rf /var/lib/apt/lists/*

RUN cmake -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_QT_GUI=ON \
    -DBUILD_SPIRE_PY=ON \
    -DPython3_EXECUTABLE=/usr/bin/python3

RUN cmake --build build --parallel $(nproc)

RUN python3 - <<EOF
import site, glob, shutil, os

sd = site.getsitepackages()[0]
matches = glob.glob("/opt/spire/build/spirepy*.so")
if not matches:
    raise SystemExit("NO spirepy .so FOUND in /opt/spire/build — build failed")

src = matches[0]
dst = os.path.join(sd, "spirepy.so")
shutil.copy2(src, dst)
print("Installed:", dst)
EOF

EXPOSE 6080

RUN chmod +x /opt/spire/docker-start-gui.sh
CMD ["/opt/spire/docker-start-gui.sh"]


# ==========================================================
# Target: api (future)
# - Keeps python usage
# - Intended to run FastAPI via uvicorn
# - Placeholder CMD stays bash until API app code lands
# ==========================================================
FROM bash AS api

RUN pip install fastapi "uvicorn[standard]"

ENV UVICORN_HOST=0.0.0.0 \
    UVICORN_PORT=8000 \
    UVICORN_WORKERS=1

EXPOSE 8000

CMD ["/bin/bash", "-lc", "uvicorn server:app --host ${UVICORN_HOST} --port ${UVICORN_PORT} --workers ${UVICORN_WORKERS}"]
