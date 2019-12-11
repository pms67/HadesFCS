#ifndef MPRLS_SENSOR_H
#define MPRLS_SENSOR_H

#include "stm32f4xx_hal.h"

#define MPRLS_I2C_ADDR      (0x18 << 1)
#define MPRLS_I2C_TIMEOUT     100
#define MPRLS_STATUS_POWERED 0x40
#define MPRLS_STATUS_BUSY    0x20
#define MPRLS_STATUS_FAILED  0x04
#define MPRLS_STATUS_MATHSAT 0x01
#define MPRLS_PSI_MIN           0
#define MPRLS_PSI_MAX          25
#define MPRLS_PSI_TO_PA     ((float) 6894.75729f)

typedef struct {
	I2C_HandleTypeDef *I2Chandle;
	GPIO_TypeDef *rstPinBank;
	uint16_t rstPin;
	GPIO_TypeDef *intPinBank;
	uint16_t intPin;
	float pressurePa;
	float altitude;
} MPRLSBarometer;

uint8_t MPRLSBarometer_Init(MPRLSBarometer *bar, I2C_HandleTypeDef *I2Chandle, GPIO_TypeDef *rstPinBank, uint16_t rstPin, GPIO_TypeDef *intPinBank, uint16_t intPin);
void MPRLSBarometer_Reset(MPRLSBarometer *bar);
uint8_t MPRLSBarometer_ReadStatus(MPRLSBarometer *bar);
uint8_t MPRLSBarometer_ReadPressure(MPRLSBarometer *bar);

#endif
