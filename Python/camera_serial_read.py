""" Saves RGB image from serial

Reads in RGB values pixel by pixel
in the string format 'R,G,B\r\n'
where R/G/B are 0-255

"""

import argparse
import serial
import tqdm
import numpy as np
from PIL import Image

parser = argparse.ArgumentParser(description='Save RGB image over serial')
parser.add_argument('directory')
parser.add_argument('serial_port')
parser.add_argument('-b', '--baud_rate', default=115200)
parser.add_argument('-tm', '--time_out', default=5)
parser.add_argument('-x', '--width', default=128)
parser.add_argument('-y', '--height', default=128)

args = parser.parse_args()

IMAGE_SIZE_X = args.width
IMAGE_SIZE_Y = args.height
CHANNELS = 3

FILE_DIR = args.directory
SERIAL_PORT = args.serial_port
BAUD_RATE = args.baud_rate
TIME_OUT = args.time_out

ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=TIME_OUT)

img_array = np.empty((IMAGE_SIZE_Y, IMAGE_SIZE_X, CHANNELS), dtype=np.uint8)

print(f'\nReading {SERIAL_PORT} at {BAUD_RATE} baud...')
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
print(f'Saved image as "{FILE_DIR}".\n')

ser.close()
