#ifndef BMI088_H_
#define BMI088_H_

#include "stm32f4xx_hal.h"

#define BMI088_ACC_I2C_ADDR (0x19 << 1)
#define BMI088_GYR_I2C_ADDR (0x69 << 1)
#define BMI088_I2C_TIMEOUT 100

/* Registers */
#define BMI088_ACC_CHIP_ID        0x00
#define BMI088_ACC_ERR_REG        0x02
#define BMI088_ACC_STATUS         0x03
#define BMI088_ACC_DATA           0x12
#define BMI088_TEMP_DATA          0x22
#define BMI088_ACC_CONF           0x40
#define BMI088_ACC_RANGE          0x41
#define BMI088_INT1_IO_CONF       0x53
#define BMI088_INT2_IO_CONF       0x54
#define BMI088_INT1_INT2_MAP_DATA 0x58
#define BMI088_ACC_SELF_TEST      0x6D
#define BMI088_ACC_PWR_CONF       0x7C
#define BMI088_ACC_PWR_CTRL       0x7D
#define BMI088_ACC_SOFTRESET      0x7E

#define BMI088_GYR_CHIP_ID           0x00
#define BMI088_GYR_DATA              0x02
#define BMI088_GYR_RANGE             0x0F
#define BMI088_GYR_BANDWIDTH         0x10
#define BMI088_GYR_LPM1              0x11
#define BMI088_GYR_SOFTRESET         0x14
#define BMI088_GYR_INT_CTRL          0x15
#define BMI088_GYR_INT3_INT4_IO_CONF 0x16
#define BMI088_GYR_INT3_INT4_IO_MAP  0x18


typedef struct {
	I2C_HandleTypeDef *I2Chandle;
	GPIO_TypeDef *intAccPinBank;
	uint16_t intAccPin;
	GPIO_TypeDef *intGyrPinBank;
	uint16_t intGyrPin;
	float acc[3];
	float gyr[3];
} BMI088IMU;

uint8_t BMI088_Init(BMI088IMU *imu, I2C_HandleTypeDef *I2Chandle, GPIO_TypeDef *intAccPinBank, uint16_t intAccPin, GPIO_TypeDef *intGyrPinBank, uint16_t intGyrPin);
void BMI088_ResetAcc(BMI088IMU *imu);
void BMI088_ResetGyr(BMI088IMU *imu);
void BMI088_ReadAcc(BMI088IMU *imu);
void BMI088_ReadGyr(BMI088IMU *imu);

#endif
