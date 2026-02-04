#!/usr/bin/env bash
set -euo pipefail

# Configurable ports/screen geometry
: "${DISPLAY:=:1}"
: "${XVFB_WHD:=1400x900x24}"
: "${VNC_PORT:=5900}"
: "${NOVNC_PORT:=6080}"

# Helps Qt behave better under Xvfb
export DISPLAY
export QT_X11_NO_MITSHM=1

# Start virtual framebuffer (headless X server)
Xvfb "$DISPLAY" -screen 0 "$XVFB_WHD" -ac +extension GLX +render -noreset &
sleep 1

# Start VNC server (no password; suitable for local dev)
x11vnc -display "$DISPLAY" -nopw -forever -shared -rfbport "$VNC_PORT" -listen 0.0.0.0 &
sleep 1

# Start noVNC websocket proxy
websockify --web=/usr/share/novnc 0.0.0.0:"$NOVNC_PORT" localhost:"$VNC_PORT" &
sleep 1

echo "=========================================="
echo "SPIRE GUI available at: http://localhost:${NOVNC_PORT}/vnc.html"
echo "=========================================="

# SPIRE loads global_settings.conf via a relative path; ensure CWD is /opt/spire
cd /opt/spire
exec /opt/spire/build/SPIRE
