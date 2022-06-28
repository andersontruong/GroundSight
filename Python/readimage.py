import numpy as np
import matplotlib.pyplot as plt
import argparse
from PIL import Image
import os

def binary_to_image(src, dest, file_extension, recursive):
    if os.path.isfile(src):
        file = open(src, 'rb')

        img_array = np.empty((3 * 128 * 128, 1), dtype=np.uint8)

        index = 0
        for i, item in enumerate(file.read()):
            if (i + 1) % 4 != 0:
                img_array[index] = item
                index += 1

        img_array = img_array.reshape((128, 128, 3))
        img_pil = Image.fromarray(img_array)
        img_pil.save(dest)
    elif recursive:
        if not os.path.exists(dest):
            os.mkdir(dest)
        for root, dirs, files in os.walk(src, topdown=False):
            for name in files:
                s = os.path.join(root, name)
                d = os.path.join(dest, os.path.splitext(name)[0] + '.' + file_extension)
                # Recursively convert files to image
                binary_to_image(s, d, file_extension, recursive)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Recursively converts files from binary to image')
    parser.add_argument('-s', '--src', default='src')
    parser.add_argument('-d', '--dest', default='dest')
    parser.add_argument('-f', '--file_extension', default='png')
    parser.add_argument('-r', '--recursive', default=True)

    args = parser.parse_args()

    SRC = args.src
    DEST = args.dest
    FILE_EXTENSION = args.file_extension
    RECURSIVE = args.recursive

    binary_to_image(SRC, DEST, FILE_EXTENSION, RECURSIVE)
