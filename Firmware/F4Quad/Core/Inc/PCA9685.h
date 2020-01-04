#ifndef PCA9685_H_
#define PCA9685_H_

#include "stm32f4xx_hal.h"

#define PCA9685_I2C_ADDR (0x40 << 1)
#define PCA9685_I2C_TIMEOUT 100
#define PCA9685_OSC_FREQ 25000000

/* Registers */
#define PCA9685_REG_MODE1 0x00
#define PCA9685_REG_MODE2 0x01
#define PCA9685_REG_DATA  0x06
#define PCA9685_REG_PRSCL 0xFE

typedef struct {
	I2C_HandleTypeDef *I2Chandle;
	uint8_t preScale;
	uint16_t setting[8];
} PCA9685;

void PCA9685_Init(PCA9685 *pca, I2C_HandleTypeDef *I2Chandle, uint8_t pwmFreq);
void PCA9685_SetMicros(PCA9685 *pca, uint8_t channel, uint16_t micros);
void PCA9685_SetPulseWidth(PCA9685 *pca, uint8_t channel, uint16_t val);
void PCA9685_SetPWM(PCA9685 *pca, uint8_t channel, uint16_t on, uint16_t off);

#endif
