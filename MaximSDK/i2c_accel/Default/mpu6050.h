/*
	mpu6050.h

	MPU6050 driver

	Helpful documents:

		MPU-6000 Datasheet
		https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Datasheet1.pdf

		MPU-6000/MPU-6050 Register Map
		https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Register-Map1.pdf
*/

#ifndef _MPU6050_H_
#define _MPU6050_H_

#include <stdint.h>
#include "mxc.h"
#include "mpu6050_regs.h"

/*
  Initialize MPU6050 device.

  i2c_master is pointer to I2C instance used to communicate with device.

  Returns 0 on success, negative if error.
*/
int mpu6050_init(mxc_i2c_regs_t *i2c_master);

/*
  Read X, Y and Z acceleration data.

  PTR parameter must be large enough to accept three int16_t values.
  Axis data order is X, Y, Z.

  Returns 0 on success, negative if error.
*/
int mpu6050_get_accel(int16_t *ptr);

#endif
