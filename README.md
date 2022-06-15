# GroundSight
MAX78000FTHR

## Capture and save images from the onboard CMOS sensor
1. Compile and upload the .c project in '/camera-demo/' to the MAX78000FTHR.
2. Install the required Python packages with '/Python/requirements.txt'.
3. Connect the MAX78000FTHR to a USB port.
4. Modify FILE_DIR and SERIAL_PORT in '/Python/camera_serial_read.py' to match the saved image's directory and serial COM port.
5. Run '/Python/camera_serial_read.py' and press the SW1 button on the MAX78000FTHR to capture an image. In the Python terminal, a tqdm progress bar will show the serial data transfer progress.
