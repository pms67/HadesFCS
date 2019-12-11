#ifndef TMP100_H_
#define TMP100_H_

#include "stm32f4xx_hal.h"

#define TMP100_I2C_ADDR (0x4E << 1)
#define TMP100_I2C_TIMEOUT 100

/* Registers */
#define TMP100_REG_DATA 0x00
#define TMP100_REG_CONF 0x01
#define TMP100_REG_TLO  0x02
#define TMP100_REG_THI  0x03

typedef struct {
	I2C_HandleTypeDef *I2Chandle;
	float temp_C;
} TMP100;

void TMP100_Init(TMP100 *tmp, I2C_HandleTypeDef *I2Chandle);
void TMP100_Read(TMP100 *tmp);

#endif
