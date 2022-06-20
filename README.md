# GroundSight

## Capture and save images from MAX78000FTHR's onboard camera
1. Compile and upload the .c project in 'camera-demo/' to the MAX78000FTHR.
2. Install the required Python packages in 'Python/requirements.txt'.
3. Connect the board to a USB port.
4. Run 'Python/camera_serial_read.py' with the command-line arguments below
5. Press the SW1/PB1 button on the MAX78000FTHR to capture an image.

### Command-Line Arguments
| Argument | Description | Examples |
| --- | --- | --- |
| directory | File directory to save the image at | ex: capture.png, captures/test.jpg, ./photos/image.bmp |
| serial_port | COM port connected to the board | ex: COM1, COM5, COM7 |
| --baud_rate | Baud rate for serial communication. Default: 115200 | ex: 9600, 19200 |
| --time_out | Serial read time-out in seconds. Default: 5 | ex: 2, 7 |
| -x, --width | Image width in pixels. Default: 128 | ex: 28, 64 |
| -y, --height | Image height in pixels. Default: 128 | ex: 28, 64 |
