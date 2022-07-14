################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../main.c \
../mpu6050.c 

OBJS += \
./main.o \
./mpu6050.o 

C_DEPS += \
./main.d \
./mpu6050.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -I"C:\MaximSDK/Libraries/CMSIS/Include" -I"C:\MaximSDK/Libraries/CMSIS/Device/Maxim/MAX78000/Include" -I"C:\MaximSDK/Libraries/PeriphDrivers/Include/MAX78000/" -I"C:\MaximSDK/Libraries/Boards/MAX78000/Include" -I"C:\MaximSDK/Libraries/Boards/MAX78000/FTHR_RevA/Include" -I"C:\Users\flami\eclipse-workspace\i2c_accel" -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

mpu6050.o: C:/Users/flami/eclipse-workspace/i2c_accel/mpu6050.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -I"C:\MaximSDK/Libraries/CMSIS/Include" -I"C:\MaximSDK/Libraries/CMSIS/Device/Maxim/MAX78000/Include" -I"C:\MaximSDK/Libraries/PeriphDrivers/Include/MAX78000/" -I"C:\MaximSDK/Libraries/Boards/MAX78000/Include" -I"C:\MaximSDK/Libraries/Boards/MAX78000/FTHR_RevA/Include" -I"C:\Users\flami\eclipse-workspace\i2c_accel" -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


