from PIL import Image, ImageDraw
import os

def create_background(image_size, background_color='white', pattern=False):
    """ Create a background image of the specified size. """
    background = Image.new('RGBA', image_size, background_color)
    if pattern:
        # Create a checkered background
        draw = ImageDraw.Draw(background)
        tile_size = 15  # Size of each tile
        for x in range(0, image_size[0], tile_size):
            for y in range(0, image_size[1], tile_size):
                if (x // tile_size) % 2 == (y // tile_size) % 2:
                    draw.rectangle([x, y, x + tile_size, y + tile_size], fill='#eee')
    return background

def process_images(image_paths, output_gif_path, image_size, background_color='white', pattern=False, palette_mode='adaptive', dither=True):
    """ Process a list of images and save as a GIF. """
    frames = []
    for image_path in image_paths:
        cropped_image = Image.open(image_path).convert('RGBA')
        background = create_background(image_size, background_color, pattern)
        
        # Center the cropped image on the background
        bg_width, bg_height = background.size
        img_width, img_height = cropped_image.size
        offset = ((bg_width - img_width) // 2, (bg_height - img_height) // 2)
        
        # Composite the images
        frame = Image.new('RGBA', background.size)
        frame.paste(background, (0, 0))
        frame.paste(cropped_image, offset, cropped_image)
        frame = frame.convert('P')

        frames.append(frame)
    
    # Save as GIF
    frames[0].save(output_gif_path, save_all=True, append_images=frames[0:], duration=50, loop=0, optimize=True, dither=Image.NONE if not dither else Image.FLOYDSTEINBERG)

if __name__ == "__main__":
    # Example usage
    scripts_dir = os.path.dirname(__file__)
    root_dir = os.path.abspath(os.path.join(scripts_dir, os.pardir))
    output_dir = os.path.join(root_dir, "img_out")

    files = [file for file in os.listdir(output_dir) if file.endswith(".jpg")]
    files.sort()
    files = [os.path.join(output_dir, file) for file in files]


    output_gif_path = 'output.gif'
    # open first image to get size
    image = Image.open(files[0])
    image_size = image.size
    image.close()
    process_images(files, output_gif_path, image_size, background_color='white', pattern=True, palette_mode='web', dither=True)
