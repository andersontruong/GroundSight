/*******************************************************************************
 * Copyright (C) 2020-2021 Maxim Integrated Products, Inc., All rights Reserved.
 *
 * This software is protected by copyright laws of the United States and
 * of foreign countries. This material may also be protected by patent laws
 * and technology transfer regulations of the United States and of foreign
 * countries. This software is furnished under a license agreement and/or a
 * nondisclosure agreement and may only be used or reproduced in accordance
 * with the terms of those agreements. Dissemination of this information to
 * any party or parties not specified in the license agreement and/or
 * nondisclosure agreement is expressly prohibited.
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
 * OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of Maxim Integrated
 * Products, Inc. shall not be used except as stated in the Maxim Integrated
 * Products, Inc. Branding Policy.
 *
 * The mere transfer of this software does not imply any licenses
 * of trade secrets, proprietary technology, copyrights, patents,
 * trademarks, maskwork rights, or any other form of intellectual
 * property whatsoever. Maxim Integrated Products, Inc. retains all
 * ownership rights.
 *******************************************************************************/

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "mxc_device.h"
#include "mxc_sys.h"
#include "fcr_regs.h"
#include "icc.h"
#include "led.h"
#include "tmr.h"
#include "dma.h"
#include "pb.h"
#include "cnn.h"
#include "weights.h"
#include "sampledata.h"
#include "mxc_delay.h"
#include "camera.h"

/***** Camera/Image *****/

#define IMAGE_SIZE_X  (64*2)
#define IMAGE_SIZE_Y  (64*2)

#define CAMERA_FREQ   (5 * 1000 * 1000)

volatile uint32_t cnn_time; // Stopwatch

static uint32_t input_0[IMAGE_SIZE_X * IMAGE_SIZE_Y]; // buffer for camera image

/***** I2C LED *****/
#include "nvic_table.h"
#include "i2c_regs.h"
#include "i2c.h"

#define PMIC_I2C      		MXC_I2C1
#define I2C_FREQ        	100000
#define PMIC_SLAVE_ADDR 	(0x28)
#define INIT_LEN			2
#define	LED_SET_LEN			4
#define	LED_CFG_REG_ADDR	0x2C
#define LED_SET_REG_ADDR	0x2D
#define LED_I2C_BLUE		0x1
#define LED_I2C_RED			0x2
#define LED_I2C_GREEN		0x4

static uint8_t tx_buf[LED_SET_LEN];

enum COLOR {
	_BLACK,
	_BLUE,
	_RED,
	_MAGENTA,
	_GREEN,
	_CYAN,
	_YELLOW,
	_WHITE
};

void D1_LED(int state)
{
	if (state == _RED || state == _MAGENTA || state == _YELLOW || state == _WHITE)
		LED_On(LED1);
	else
		LED_Off(LED1);

	if (state == _GREEN || state == _CYAN || state == _YELLOW || state == _WHITE)
			LED_On(LED2);
		else
			LED_Off(LED2);

	if (state == _BLUE || state == _MAGENTA || state == _CYAN || state == _WHITE)
			LED_On(LED3);
		else
			LED_Off(LED3);
}

void D2_LED(int state) {
	int error;
    // Set the LED Color
    mxc_i2c_req_t reqMaster;
    reqMaster.i2c = PMIC_I2C;
    reqMaster.addr = PMIC_SLAVE_ADDR;
    reqMaster.tx_buf = tx_buf;
    reqMaster.tx_len = LED_SET_LEN;
    reqMaster.rx_buf = NULL;
    reqMaster.rx_len = 0;
    reqMaster.restart = 0;

    tx_buf[0] = LED_SET_REG_ADDR;
    tx_buf[1] = (uint8_t) ((state & LED_I2C_BLUE) 	<< 5);		//Set Blue LED?
    tx_buf[2] = (uint8_t) ((state & LED_I2C_RED) 	<< 4);		//Set Red LED?
    tx_buf[3] = (uint8_t) ((state & LED_I2C_GREEN) 	<< 3);		//Set Green LED?

    if ((error = MXC_I2C_MasterTransaction(&reqMaster)) != 0) {
        printf("Error writing: %d\n", error);
        while(1);
    }
}

/* **************************************************************************** */

void serial_send_image(uint32_t *img)
{
	uint8_t *p = (uint8_t*) img;
	uint8_t r, g, b;
	for (int i = 0; i < IMAGE_SIZE_Y; i++)
	{
		for (int j = 0; j < IMAGE_SIZE_X; j++)
		{
			r = *p++ ^ 0x80;
			g = *(p++) ^ 0x80;
			b = *(p++) ^ 0x80;
			printf("%i,%i,%i\n", r, g, b);
			p++;
		}
	}
}

/* **************************************************************************** */
void fail(void) {
	printf("\n*** FAIL ***\n\n");

	while (1);
}

/* **************************************************************************** */

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
	for (int row = 0; row < h; row++) {
		// Wait until camera streaming buffer is full
		while ((data = get_camera_stream_buffer()) == NULL) {
			if (camera_is_image_rcv()) {
				break;
			}
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
		while (1)
			;
	}

}

/* **************************************************************************** */
int main(void) {
	int ret = 0;
	int dma_channel;

    // Wait for PMIC 1.8V to become available, about 180ms after power up.
    MXC_Delay(200000);
    /* Enable camera power */
    Camera_Power(POWER_ON);
    //MXC_Delay(300000);

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

	ret = camera_setup(IMAGE_SIZE_X, IMAGE_SIZE_Y, PIXFORMAT_RGB888,
			FIFO_THREE_BYTE, STREAMING_DMA, dma_channel);
	if (ret != STATUS_OK) {
		printf("Error returned from setting up camera. Error %d\n", ret);
		return -1;
	}

	camera_write_reg(0x11, 0x3); // set camera clock prescaller to prevent streaming overflow

	// White LEDs as camera light

	D1_LED(_WHITE);

	D2_LED(_BLACK);
	capture_process_camera();

	// Wait for SW1 button press
	while (!PB_Get(0));

	// Enable CNN clock
	MXC_SYS_ClockEnable(MXC_SYS_PERIPH_CLOCK_CNN);

	while (1) {

		capture_process_camera();

		serial_send_image(input_0);

		while (!PB_Get(0));

	}

	return 0;
}
