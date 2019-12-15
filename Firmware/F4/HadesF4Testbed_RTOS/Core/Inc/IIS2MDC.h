#ifndef IIS2MDC_H_
#define IIS2MDC_H_

#include "stm32f4xx_hal.h"
#include <math.h>

#define IIS_I2C_ADDR (0x1E << 1)
#define IIS_I2C_TIMEOUT 100
#define IIS_WHOAMI 0x40

/* Registers */
#define IIS_WHOAMI_REG 0x4F
#define IIS_CFG_REG_A 0x60
#define IIS_CFG_REG_B 0x61
#define IIS_CFG_REG_C 0x62
#define IIS_CTRL_REG  0x63
#define IIS_STATUS    0x67
#define IIS_OUTX_LOW  0x68
#define IIS_OUTY_LOW  0x6A
#define IIS_OUTZ_LOW  0x6C
#define IIS_TEMP_LOW  0x6E

typedef struct {
	I2C_HandleTypeDef *I2Chandle;
	GPIO_TypeDef *intPinBank;
	uint16_t intPin;
	float xyz[3];
	float tempC;
} IISMagnetometer;

uint8_t IISMagnetometer_Init(IISMagnetometer *mag, I2C_HandleTypeDef *I2Chandle, GPIO_TypeDef *intPinBank, uint16_t intPin);
void IISMagnetometer_Reset(IISMagnetometer *mag);
void IISMagnetometer_Read(IISMagnetometer *mag);


#endif
