#ifndef _MPU6050_H_
#define _MPU6050_H_

#include <stdint.h>
#include "mpu6050_reg.h"

#define AXIS_X 0
#define AXIS_Y 1
#define AXIS_Z 2

void MPU6050_read(uint8_t reg, uint8_t* buffer, int size);

void MPU6050_write(uint8_t reg, uint8_t* buffer, int size);

void MPU6050_write(uint8_t reg, uint8_t data);

void MPU6050_getAccelAxis(int AXIS, int16_t* buffer);

void MPU6050_getAccel(int16_t* buffer);

#endif