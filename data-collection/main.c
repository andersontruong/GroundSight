
/**
 * @file    main.c
 * @brief   read and write sdhc
 * @details This example uses the sdhc and ffat to read/write the file system on
 *          an SD card. The Fat library used supports long filenames (see ffconf.h)
 *          the max length is 256 characters.
 *a
 *          You must connect an sd card to the sd card slot.
 */

/***** Includes *****/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <mxc_device.h>
#include "mxc_sys.h"
#include <mxc_delay.h>
#include <gpio.h>
#include <uart.h>

/***** FatFS SD Card *****/
#include "ff.h"

#define MAXLEN 256

FATFS* fs;      //FFat Filesystem Object
FATFS fs_obj;
FIL file;       //FFat File Object
FRESULT err;    //FFat Result (Struct)
FILINFO fno;    //FFat File Information Object
DIR dir;        //FFat Directory Object
TCHAR message[MAXLEN], directory[MAXLEN], cwd[MAXLEN], filename[MAXLEN], volume_label[24], volume = '0';
TCHAR* FF_ERRORS[20];
DWORD clusters_free = 0, sectors_free = 0, sectors_total = 0, volume_sn = 0;
UINT bytes_written = 0, bytes_read = 0, mounted = 0;
BYTE work[4096];
mxc_gpio_cfg_t SDPowerEnablePin = {MXC_GPIO1, MXC_GPIO_PIN_12, MXC_GPIO_FUNC_OUT, MXC_GPIO_PAD_NONE, MXC_GPIO_VSSEL_VDDIO};

int mount()
{
    fs = &fs_obj;

    if ((err = f_mount(fs, "", 1)) != FR_OK) {          //Mount the default drive to fs now
        printf("Error opening SD card: %s\n", FF_ERRORS[err]);
        f_mount(NULL, "", 0);
    }
    else {
        printf("SD card mounted.\n");
        mounted = 1;
    }

    f_getcwd(cwd, sizeof(cwd));                         //Set the Current working directory

    return err;
}

int umount()
{
    if ((err = f_mount(NULL, "", 0)) != FR_OK) {        //Unmount the default drive from its mount point
        printf("Error unmounting volume: %s\n", FF_ERRORS[err]);
    }
    else {
        printf("SD card unmounted.\n");
        mounted = 0;
    }

    return err;
}

void FF_setup()
{
	FF_ERRORS[0] = "FR_OK";
	FF_ERRORS[1] = "FR_DISK_ERR";
	FF_ERRORS[2] = "FR_INT_ERR";
	FF_ERRORS[3] = "FR_NOT_READY";
	FF_ERRORS[4] = "FR_NO_FILE";
	FF_ERRORS[5] = "FR_NO_PATH";
	FF_ERRORS[6] = "FR_INVLAID_NAME";
	FF_ERRORS[7] = "FR_DENIED";
	FF_ERRORS[8] = "FR_EXIST";
	FF_ERRORS[9] = "FR_INVALID_OBJECT";
	FF_ERRORS[10] = "FR_WRITE_PROTECTED";
	FF_ERRORS[11] = "FR_INVALID_DRIVE";
	FF_ERRORS[12] = "FR_NOT_ENABLED";
	FF_ERRORS[13] = "FR_NO_FILESYSTEM";
	FF_ERRORS[14] = "FR_MKFS_ABORTED";
	FF_ERRORS[15] = "FR_TIMEOUT";
	FF_ERRORS[16] = "FR_LOCKED";
	FF_ERRORS[17] = "FR_NOT_ENOUGH_CORE";
	FF_ERRORS[18] = "FR_TOO_MANY_OPEN_FILES";
	FF_ERRORS[19] = "FR_INVALID_PARAMETER";
}

// External Button/LED
#include "nvic_table.h"
#include "board.h"

#define BTN_PORT						MXC_GPIO1
#define BTN_PIN							MXC_GPIO_PIN_0

#define LED_PORT						MXC_GPIO1
#define LED_PIN							MXC_GPIO_PIN_1

mxc_gpio_cfg_t btn, led;

void GPIO_setup()
{
	led.port = LED_PORT;
	led.mask = LED_PIN;
	led.pad = MXC_GPIO_PAD_NONE;
	led.func = MXC_GPIO_FUNC_OUT;
	led.vssel = MXC_GPIO_VSSEL_VDDIOH;
	MXC_GPIO_Config(&led);

	btn.port = BTN_PORT;
	btn.mask = BTN_PIN;
	btn.pad = MXC_GPIO_PAD_PULL_DOWN;
	btn.func = MXC_GPIO_FUNC_IN;
	MXC_GPIO_Config(&btn);
}

/***** CAMERA *****/

#include "icc.h"
#include "tmr.h"
#include "dma.h"
#include "pb.h"
#include "camera.h"

#define IMAGE_SIZE_X (64*2)

#define IMAGE_SIZE_Y  (64*2)

#define CAMERA_FREQ   (5 * 1000 * 1000)

volatile uint32_t cnn_time; // Stopwatch

static uint32_t input_0[IMAGE_SIZE_X * IMAGE_SIZE_Y]; // buffer for camera image

void cam_setup()
{
	int ret = 0;
	int dma_channel;

	// Wait for PMIC 1.8V to become available, about 180ms after power up.
	MXC_Delay(200000);

	/* Enable camera power */
	Camera_Power(POWER_ON);

	/* Enable cache */
	MXC_ICC_Enable(MXC_ICC0);

	/* Switch to 100 MHz clock */
	MXC_SYS_Clock_Select(MXC_SYS_CLOCK_IPO);
	SystemCoreClockUpdate();

	// Initialize DMA for camera interface
	MXC_DMA_Init();
	dma_channel = MXC_DMA_AcquireChannel();

	// Initialize camera.
	camera_init(CAMERA_FREQ);

	ret = camera_setup(
		IMAGE_SIZE_X,
		IMAGE_SIZE_Y,
		PIXFORMAT_RGB888,
		FIFO_THREE_BYTE,
		STREAMING_DMA,
		dma_channel
	);
	if (ret != STATUS_OK) {
		printf("Error returned from setting up camera. Error %d\n", ret);
	}

	camera_write_reg(0x11, 0x3); // set camera clock prescaller to prevent streaming overflow

	camera_set_brightness(-2);
	camera_set_contrast(2);
}

void capture_process_camera(void) {

	uint8_t *raw;
	uint32_t imgLen;
	uint32_t w, h;

	int cnt = 0;

	uint8_t r, g, b;

	uint8_t *data = NULL;
	stream_stat_t *stat;

	camera_start_capture_image();

	// Get the details of the image from the camera driver.
	camera_get_image(&raw, &imgLen, &w, &h);

	// Get image line by line
	for (int row = 0; row < h + 1; row++) {
		// Wait until camera streaming buffer is full
		while ((data = get_camera_stream_buffer()) == NULL) {
			if (camera_is_image_rcv()) {
				break;
			}
		}
		if (row == 0) {
			release_camera_stream_buffer();
			continue;
		}

		int j = 0;
		for (int k = 0; k < 4 * w; k += 4) {

			// data format: 0x00bbggrr
			r = data[k];
			g = data[k + 1];
			b = data[k + 2];
			//skip k+3

			// change the range from [0,255] to [-128,127] and store in buffer for CNN
			input_0[cnt++] = ((b << 16) | (g << 8) | r) ^ 0x00808080;

			j += 2;
		}

		// Release stream buffer
		release_camera_stream_buffer();
	}

	//camera_sleep(1);
	stat = get_camera_stream_statistic();

	if (stat->overflow_count > 0) {
		printf("OVERFLOW DISP = %d\n", stat->overflow_count);
		LED_On(LED2); // Turn on red LED if overflow detected

		while (1);
	}

	for (int i = 0; i < IMAGE_SIZE_X * IMAGE_SIZE_Y; i++)
	{
		input_0[i] ^= 0x00808080;
	}
}

/***** D1 LED CONTROL *****/
#include "led.h"

enum COLOR {
	_BLACK, _BLUE, _RED, _MAGENTA, _GREEN, _CYAN, _YELLOW, _WHITE
};

void D1_LED(int state) {
	if (state == _RED || state == _MAGENTA || state == _YELLOW
			|| state == _WHITE)
		LED_On(LED1);
	else
		LED_Off(LED1);

	if (state == _GREEN || state == _CYAN || state == _YELLOW
			|| state == _WHITE)
		LED_On(LED2);
	else
		LED_Off(LED2);

	if (state == _BLUE || state == _MAGENTA || state == _CYAN
			|| state == _WHITE)
		LED_On(LED3);
	else
		LED_Off(LED3);
}

/***** FUNCTIONS *****/

int createFile()
{
    if (!mounted) {
        mount();
    }

    // printf("Creating file %s\n", filename);

    if ((err = f_open(&file, (const TCHAR*) filename, FA_CREATE_ALWAYS | FA_WRITE)) != FR_OK) {
        printf("Error opening file: %s\n", FF_ERRORS[err]);
        f_mount(NULL, "", 0);
        return err;
    }

    // printf("File opened!\n");

    if ((err = f_write(&file, &input_0, 128*128*4, &bytes_written)) != FR_OK) {
        printf("Error writing file: %s\n", FF_ERRORS[err]);
        f_mount(NULL, "", 0);
        return err;
    }

    // printf("%d bytes written to file!\n", bytes_written);

    if ((err = f_close(&file)) != FR_OK) {
        printf("Error closing file: %s\n", FF_ERRORS[err]);
        f_mount(NULL, "", 0);
        return err;
    }

    // printf("File Closed!\n");
    return err;
}

/******************************************************************************/
int main(void)
{
	FF_setup();
	GPIO_setup();
	cam_setup();
    srand(12347439);
    int run = 1;
    
    // printf("\n\n***** SDHC FAT Filesystem Example *****\n");

    // printf("Card inserted.\n");
    
    int index = 0;

    while (run) {
        f_getcwd(cwd, sizeof(cwd));
        while (!MXC_GPIO_InGet(btn.port, btn.mask))
		{
			D1_LED(_GREEN);
			MXC_Delay(MXC_DELAY_MSEC(100));
			D1_LED(_BLACK);
			MXC_Delay(MXC_DELAY_MSEC(100));
		}

        D1_LED(_BLACK);

		MXC_GPIO_OutSet(led.port, led.mask);

		MXC_Delay(MXC_DELAY_SEC(3));

        capture_process_camera();

        MXC_GPIO_OutClr(led.port, led.mask);

        D1_LED(_RED);

        sprintf(filename, "image_%i", index++);

        createFile();

        MXC_Delay(MXC_DELAY_SEC(1));
    }
    return 0;
}

