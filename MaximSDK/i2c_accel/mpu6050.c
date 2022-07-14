/*
	mpu6050.c

	MPU6050 driver

	Helpful documents:

		MPU-6000 Datasheet
		https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Datasheet1.pdf

		MPU-6000/MPU-6050 Register Map
		https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Register-Map1.pdf
*/

#include "mpu6050.h"
#include "i2c_regs.h"
#include <stdint.h>
#include "mxc.h"

mxc_i2c_req_t i2c_req;

int reg_read(uint8_t reg, uint8_t *data)
{
	uint8_t buf[1] = { reg };

	i2c_req.tx_buf = buf;
	i2c_req.tx_len = sizeof(buf);
	i2c_req.rx_buf = data;
	i2c_req.rx_len = 1;

	return MXC_I2C_MasterTransaction(&i2c_req);
}

int reg_write(uint8_t reg, uint8_t val)
{
  uint8_t buf[2] = { reg, val };

  i2c_req.tx_buf = buf;
  i2c_req.tx_len = sizeof(buf);
  i2c_req.rx_len = 0;

  return MXC_I2C_MasterTransaction(&i2c_req);
}

int mpu6050_init(mxc_i2c_regs_t *i2c_master)
{
	int result;
	uint8_t id;

	if (!i2c_master) return E_NULL_PTR;

	i2c_req.i2c = i2c_master;
	i2c_req.addr = MPU6050_ADDRESS;
	i2c_req.restart = 0;
	i2c_req.callback = NULL;

	if ((result = reg_read(MPU6050_RA_WHO_AM_I, &id)) != E_NO_ERROR)
	  return result;

	if (id != MPU6050_ADDRESS)
	  return E_NOT_SUPPORTED;

	reg_write(MPU6050_RA_PWR_MGMT_1, 0x00);
	reg_write(MPU6050_RA_CONFIG, 0x01);
	reg_write(MPU6050_RA_GYRO_CONFIG, 1<<4); // 1000 deg/s
	reg_write(MPU6050_RA_ACCEL_CONFIG, 0x01);// 2g scale

	return E_NO_ERROR;
}

int mpu6050_get_accel(int16_t *ptr)
{
	uint8_t *accel_reg;
	int result;
	*accel_reg = MPU6050_RA_ACCEL_XOUT_H;
	for (uint8_t i = 0; i < 6; i++)
	{
		i2c_req.tx_buf = accel_reg;
		i2c_req.tx_len = sizeof(accel_reg);
		i2c_req.rx_buf = (uint8_t *) ptr + i;
		i2c_req.rx_len = 1;

		result = MXC_I2C_MasterTransaction(&i2c_req);
		if (result != E_NO_ERROR)
		{
			return result;
		}
	}

  return E_NO_ERROR;
}
