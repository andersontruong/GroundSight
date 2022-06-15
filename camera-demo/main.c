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

void SetLEDs(int state) {
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

// #define CLASSIFY

// Comment out USE_SAMPLEDATA to use Camera module
//#define USE_SAMPLEDATA

//#define ASCII_ART

#define IMAGE_SIZE_X  (64*2)
#define IMAGE_SIZE_Y  (64*2)

#define CAMERA_FREQ   (5 * 1000 * 1000)

const char classes[CNN_NUM_OUTPUTS][10] = { "Cat", "Dog" };

// Classification layer:
static int32_t ml_data[CNN_NUM_OUTPUTS];
static q15_t ml_softmax[CNN_NUM_OUTPUTS];

volatile uint32_t cnn_time; // Stopwatch

static uint32_t input_0[IMAGE_SIZE_X * IMAGE_SIZE_Y]; // buffer for camera image

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
#ifdef ASCII_ART

//char * brightness = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'. "; // standard
char *brightness = "@%#*+=-:. "; // simple
#define RATIO 2  // ratio of scaling down the image to display in ascii
void asciiart(uint8_t *img) {
	int skip_x, skip_y;
	uint8_t r, g, b, Y;
	uint8_t *srcPtr = img;
	int l = strlen(brightness) - 1;

	skip_x = RATIO;
	skip_y = RATIO;
	for (int i = 0; i < IMAGE_SIZE_Y; i++) {
		for (int j = 0; j < IMAGE_SIZE_X; j++) {

			// 0x00bbggrr, convert to [0,255] range
			r = *srcPtr++ ^ 0x80;
			g = *(srcPtr++) ^ 0x80;
			b = *(srcPtr++) ^ 0x80;

			srcPtr++; //skip msb=0x00

			// Y = 0.2126 * r + 0.7152 * g + 0.0722 * b;
			Y = (3 * r + b + 4 * g) >> 3; // simple luminance conversion
			if ((skip_x == RATIO) && (skip_y == RATIO))
				printf("%c", brightness[l - (Y * l / 255)]);

			skip_x++;
			if (skip_x > RATIO)
				skip_x = 1;
		}
		skip_y++;
		if (skip_y > RATIO) {
			printf("\n");
			skip_y = 1;
		}
	}

}

#endif

/* **************************************************************************** */
void fail(void) {
	printf("\n*** FAIL ***\n\n");

	while (1);
}

/* **************************************************************************** */
void cnn_load_input(void) {
	int i;
	const uint32_t *in0 = input_0;

	for (i = 0; i < 16384; i++) {
		// Remove the following line if there is no risk that the source would overrun the FIFO:
		while (((*((volatile uint32_t*) 0x50000004) & 1)) != 0)
			; // Wait for FIFO 0
		*((volatile uint32_t*) 0x50000008) = *in0++; // Write FIFO 0
	}
}

/* **************************************************************************** */

#if !defined USE_SAMPLEDATA
void capture_process_camera(void) {

	uint8_t *raw;
	uint32_t imgLen;
	uint32_t w, h;

	int cnt = 0;

	uint8_t r, g, b;
	uint16_t rgb;
	int j = 0;

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
#endif

/* **************************************************************************** */
int main(void) {
	int i;
	int digs, tens;
	int ret = 0;
	int result[CNN_NUM_OUTPUTS]; // = {0};
	int dma_channel;

#ifdef BOARD_FTHR_REVA
    // Wait for PMIC 1.8V to become available, about 180ms after power up.
    MXC_Delay(200000);
    /* Enable camera power */
    Camera_Power(POWER_ON);
    //MXC_Delay(300000);

#ifdef CLASSIFY
    printf("\n\nCats-vs-Dogs Feather Demo\n");
#endif

#endif

	/* Enable cache */
	MXC_ICC_Enable(MXC_ICC0);

	/* Switch to 100 MHz clock */
	MXC_SYS_Clock_Select(MXC_SYS_CLOCK_IPO);
	SystemCoreClockUpdate();

#ifdef CLASSIFY
	/* Enable peripheral, enable CNN interrupt, turn on CNN clock */
	/* CNN clock: 50 MHz div 1 */
	cnn_enable(MXC_S_GCR_PCLKDIV_CNNCLKSEL_PCLK,
			MXC_S_GCR_PCLKDIV_CNNCLKDIV_DIV1);

	/* Configure P2.5, turn on the CNN Boost */
	cnn_boost_enable(MXC_GPIO2, MXC_GPIO_PIN_5);

	/* Bring CNN state machine into consistent state */
	cnn_init();
	/* Load CNN kernels */
	cnn_load_weights();
	/* Load CNN bias */
	cnn_load_bias();
	/* Configure CNN state machine */
	cnn_configure();
#endif

	// Initialize DMA for camera interface
	MXC_DMA_Init();
	dma_channel = MXC_DMA_AcquireChannel();

	// Initialize camera.
	// printf("Init Camera.\n");
	camera_init(CAMERA_FREQ);

	ret = camera_setup(IMAGE_SIZE_X, IMAGE_SIZE_Y, PIXFORMAT_RGB888,
			FIFO_THREE_BYTE, STREAMING_DMA, dma_channel);
	if (ret != STATUS_OK) {
		printf("Error returned from setting up camera. Error %d\n", ret);
		return -1;
	}

	camera_write_reg(0x11, 0x3); // set camera clock prescaller to prevent streaming overflow

	// White LEDs as camera light
	/*
	LED_On(LED1);
	LED_On(LED2);
	LED_On(LED3);*/

	SetLEDs(_WHITE);
	capture_process_camera();

	// printf("********** Press PB1(SW1) to capture an image **********\r\n");
	while (!PB_Get(0));

	// Enable CNN clock
	MXC_SYS_ClockEnable(MXC_SYS_PERIPH_CLOCK_CNN);

	while (1) {

		capture_process_camera();

		serial_send_image(input_0);

#ifdef CLASSIFY
		cnn_start();
		cnn_load_input();

		SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk; // SLEEPDEEP=0
		while (cnn_time == 0) {
			__WFI();    // Wait for CNN interrupt
		}

		// Unload CNN data
		cnn_unload((uint32_t*) ml_data);
		cnn_stop();

		// Softmax
		softmax_q17p14_q15((const q31_t*) ml_data, CNN_NUM_OUTPUTS, ml_softmax);

		printf("Time for CNN: %d us\n\n", cnn_time);

		printf("Classification results:\n");

		for (i = 0; i < CNN_NUM_OUTPUTS; i++) {
			digs = (1000 * ml_softmax[i] + 0x4000) >> 15;
			tens = digs % 10;
			digs = digs / 10;
			result[i] = digs;
			printf("[%7d] -> Class %d %8s: %d.%d%%\r\n", ml_data[i], i,
					classes[i], result[i], tens);
		}

		printf("\n");

		if (result[0] == result[1]) {
			LED_On(LED1);
			LED_On(LED2);

		} else if (ml_data[0] > ml_data[1]) {
			LED_On(LED1);
			LED_Off(LED2);
		} else {
			LED_Off(LED1);
			LED_On(LED2);
		}


#endif

#ifdef ASCII_ART
		asciiart((uint8_t*) input_0);
		printf("********** Press PB1(SW1) to capture an image **********\r\n");
#endif
		while (!PB_Get(0));

	}

	return 0;
}
