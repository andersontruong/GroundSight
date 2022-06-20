""" Saves RGB image from serial

Reads in RGB values pixel by pixel
in the string format 'R,G,B\r\n'
where R/G/B are 0-255

"""

import serial
import tqdm
import numpy as np
from PIL import Image

IMAGE_SIZE_X = 64*2
IMAGE_SIZE_Y = 64*2
CHANNELS = 3

FILE_DIR = 'captures/test-capture.png'
SERIAL_PORT = 'COM7'

ser = serial.Serial('COM7', 115200, timeout=1)

img_array = np.empty((IMAGE_SIZE_Y, IMAGE_SIZE_X, CHANNELS), dtype=np.uint8)

with tqdm.tqdm(total=IMAGE_SIZE_Y*IMAGE_SIZE_X) as pbar:
    for y in range(IMAGE_SIZE_Y):
        for x in range(IMAGE_SIZE_X):
            value = ser.readline()
            while value == b'':
                value = ser.readline()
            value = value.decode('utf-8').rstrip('\r\n').split(',')
            img_array[y, x] = value
            pbar.update(1)

# Convert and save array as image
img = Image.fromarray(img_array)
img.save(FILE_DIR)

ser.close()
