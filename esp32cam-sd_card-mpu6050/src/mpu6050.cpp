#include <Arduino.h>
#include "mpu6050.h"
#include <Wire.h>

/**
 * @brief Read from a register of MPU6050
 * @param [in] reg - Register to read from
 * @param [in] buffer - Buffer to write to (must have size "length" or greater)
 * @param [in] size - Size of buffer (in bytes). Default: 1
 */
void MPU6050_read(uint8_t reg, uint8_t* buffer, int size=1)
{
  Wire.beginTransmission(MPU6050_ADDRESS);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU6050_ADDRESS, size, true);
  
  for (int i = 0; Wire.available() && i < size; i++)
    buffer[i] = Wire.read();
}

/**
 * @brief Write to a register of MPU6050
 * @param [in] reg - Register to write to
 * @param [in] buffer - Buffer to read from (must have size "length" or greater)
 * @param [in] size - Size of buffer (in bytes). Default: 1
 */
void MPU6050_write(uint8_t reg, uint8_t* buffer, int size=1)
{
  Wire.beginTransmission(MPU6050_ADDRESS);
  Wire.write(reg);
  Wire.write(buffer, size);
  Wire.endTransmission(true);
}

/**
 * @brief Write single byte to a register of MPU6050
 * @param [in] reg - Register to write to
 * @param [in] data - Byte data
 */
void MPU6050_write(uint8_t reg, uint8_t data)
{
    MPU6050_write(reg, &data);
}

/**
 * @brief Read accelerometer axis data from MPU6050
 * @param [in] AXIS - Axis to read (AXIS_X, AXIS_Y, or AXIS_Z)
 * @param [in] buffer - Buffer to write to
 */
void MPU6050_getAccelAxis(int AXIS, int16_t* buffer)
{
  uint8_t LH_buffer[2];

  uint8_t address = MPU6050_RA_ACCEL_XOUT_H + AXIS*2;
  for (uint8_t i = 0; i < 2; i++)
    MPU6050_read(address++, LH_buffer + i);

  *buffer = LH_buffer[0] << 8 | LH_buffer[1];
}

/**
 * @brief Read accelerometer all axes data from MPU6050
 * @param [in] buffer - Buffer to write to (must be able to store 6 bytes)
 */
void MPU6050_getAccel(int16_t* buffer)
{
  for (uint8_t i = 0; i < 3; i++)
    MPU6050_getAccelAxis(i, buffer++);
}