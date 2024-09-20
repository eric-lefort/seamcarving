import os
from PIL import Image

def write_image(image, filename):
    height = image.height
    width = image.width

    f = open(filename, "wb")

    f.write(height.to_bytes(2, byteorder='big'))
    f.write(width.to_bytes(2, byteorder='big'))
    img_raster = []
    for i in range(height):
        for j in range(width):
            img_raster.extend(image.getpixel((j, i)))

    f.write(bytearray(img_raster))
    f.close()

def read_2bytes(f):
    bytes = f.read(2) # [int(f.read(1)), int(f.read(1))]
    return int.from_bytes(bytes, byteorder = 'big')


def read_image(filename):
    f = open(filename, "rb")
    height = read_2bytes(f)
    width = read_2bytes(f)
    image = Image.new("RGB", (width, height))
    bytes = f.read()
    for i in range(height):
        for j in range(width):
            image.putpixel((j, i), (bytes[3*(i*width + j)+0],
                                    bytes[3*(i*width + j)+1],
                                    bytes[3*(i*width + j)+2]))

    return image



#Write a png image to bin
# image = Image.open("img_sm_1.jpg")
#write_image(image, "img_sm_1.bin")
#image = Image.open("img_sm_2.jpg")
#write_image(image, "img_sm_2.bin")
#image = Image.open("img_sm_3.jpg")
#write_image(image, "img_sm_3.bin")
#image = Image.open("img_sm_4.jpg")
#write_image(image, "img_sm_4.bin")
# image = Image.open("img_sm_6.jpg")
# write_image(image, "img_sm_6.bin")
# image = Image.open("img_sm_7.jpg")
# write_image(image, "img_sm_7.bin")
# image = Image.open("img_sm_8.jpg")
# write_image(image, "img_sm_8.bin")
# print("Done converting images to bin")
# exit()

# #Read image from a bin file, save it to png
# im2 = read_image("a.bin")
# im3 = read_image("6x5_grad.bin")
# im3.save("grad.png")


scripts_dir = os.path.dirname(__file__)
root_dir = os.path.abspath(os.path.join(scripts_dir, os.pardir))
output_dir = os.path.join(root_dir, "img_out")

# Write multiple images from bin to png
files = [file for file in os.listdir(output_dir) if file.endswith(".bin")]
files.sort()
for file in files:
    file = os.path.join(output_dir, file)
    file_name = file.split(".")[0]
    print(file_name)
    image = read_image(file)
    image.save(f"{file_name}.jpg")

# image = Image.open("src\\lab7_extras\\UofTPresidentMericGertler-smaller.jpg")
# write_image(image, "src\\lab7_extras\\UofTPresidentMericGertler-smaller.bin")

# for i in range(5):
#     image = read_image(f"src\\lab7_extras\\MericGertler.bin-brightened-{i}")
#     image.save(f"src\\lab7_extras\\MericGertler-brightened-{i}.png")
