/**
 * @file        main.c
 * @brief       MPU6050 I2C
 * @details     Reads IMU data from MPU6050 through I2C
 */

/*******************************************************************************
* Copyright (C) Maxim Integrated Products, Inc., All Rights Reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
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
*
******************************************************************************/

/***** Includes *****/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "mxc_device.h"
#include "mxc_delay.h"
#include "nvic_table.h"
#include "i2c_regs.h"
#include "i2c.h"

#include "mpu6050.h"

/***** Definitions *****/
#define I2C_MASTER  MXC_I2C1  // SCL P0_16; SDA P0_17
#define I2C_FREQ    400000    // 400 kHZ

uint8_t txBuffer[2];
uint8_t rxBuffer[1];

mxc_i2c_req_t request;


// *****************************************************************************
int main()
{
    printf("\nMPU6050 I2C\n\n");

	if (MXC_I2C_Init(I2C_MASTER, 1, 0) != E_NO_ERROR) {
		printf("Trouble initializing I2C master.");
	}
	MXC_I2C_SetFrequency(I2C_MASTER, I2C_FREQ);
	request.i2c = I2C_MASTER;
	request.addr = MPU6050_ADDRESS;
	request.restart = 0;
	request.callback = NULL;
	float multiplier = 4000./65535;

	uint16_t vals[3];
	int16_t val;

	while (1)
	{
		mpu6050_init(I2C_MASTER);
		mpu6050_get_accel(vals);

		val = vals[0];
		printf("Receive: %f\n", val*multiplier);

		MXC_Delay(MXC_DELAY_MSEC(200));
	}
}

















