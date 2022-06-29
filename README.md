# GroundSight
MAX78000FTHR

## Capture and save images on SD card
1. Modify main.c for for a specific button to trigger a capture. By default, it uses a pull-down switch connected to [P1.0 (https://datasheets.maximintegrated.com/en/ds/MAX78000FTHR.pdf)
1. Compile and upload the .c project in 'data-collection/'.
2. The D1 LED should be blinking green. Press the assigned button to capture an image. An LED can be connected to P1.1 to act as a flash.
3. The D1 LED will now switch to red as it writes. Once it goes back to blinking green, the file should be saved.
2. Install the required Python packages in 'Python/requirements.txt'.
3. Move the image_# files into the 'Python/src' folder
4. Run the script and the images should appear in the 'Python/dest' folder
