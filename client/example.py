from PIL import Image

from spirepy_client import generate_spire_image
# health_check, debugging
from spirepy_client import health_check, SPIRE_PORT, SPIRE_HOST

is_healthy = health_check()
if is_healthy:
    print(f"SPIRE API is running at {SPIRE_HOST}:{SPIRE_PORT}")
else:
    raise SystemExit(f"SPIRE API failed, running at {SPIRE_HOST}:{SPIRE_PORT}")

arr = generate_spire_image()
Image.fromarray(arr).save("img.png")

print("Example image saved to img.png")