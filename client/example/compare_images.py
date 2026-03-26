from PIL import Image, ImageChops

gui_image = Image.open("gui_image.png").convert("L")
spirepy_image = Image.open("spirepy_image.png")

diffrence = ImageChops.difference(gui_image, spirepy_image).getbbox()
if diffrence is None:
    print("Images are identical")
else:
    print("Images differ")
