#ifndef INA219_H_
#define INA219_H_

#include "stm32h7xx_hal.h"

#define INA219_I2C_ADDR (0x40 << 1)
#define INA219_I2C_TIMEOUT 100

/* Registers */
#define INA219_REG_CONF 0x00
#define INA219_REG_SHNT 0x01
#define INA219_REG_BUS  0x02
#define INA219_REG_PWR  0x03
#define INA219_REG_CUR  0x04
#define INA219_REG_CALI 0x05

typedef struct {
	I2C_HandleTypeDef *I2Chandle;
	float currentmA;
	float voltageBusmV;
	float voltageShuntmV;
	float powermW;
	uint16_t calVal;
	uint16_t curDiv;
	uint16_t powMul;
} INA219;

void INA219_Init32V9A(INA219 *ina, I2C_HandleTypeDef *I2Chandle);
void INA219_Init32V2A(INA219 *ina, I2C_HandleTypeDef *I2Chandle);
void INA219_Read(INA219 *ina);
void INA219_WriteReg16(INA219 *ina, uint8_t reg, uint16_t val);
void INA219_ReadReg16(INA219 *ina, uint8_t reg, uint16_t *val);

#endif
