#include "PCA9685.h"

void PCA9685_Init(PCA9685 *pca, I2C_HandleTypeDef *I2Chandle, uint8_t pwmFreq) {
	pca->I2Chandle = I2Chandle;
	pca->preScale = (uint8_t) ((PCA9685_OSC_FREQ / (pwmFreq * 4096.0f) + 0.5f) - 1.0f);

	/* Reset */
	uint8_t txBuf[] = {PCA9685_REG_MODE1, 0x80};
	HAL_I2C_Master_Transmit(pca->I2Chandle, PCA9685_I2C_ADDR, txBuf, 2, PCA9685_I2C_TIMEOUT);
	HAL_Delay(10);

	/* Put to sleep */
	uint8_t rxBuf;
	HAL_I2C_Mem_Read(pca->I2Chandle, PCA9685_I2C_ADDR, PCA9685_REG_MODE1, I2C_MEMADD_SIZE_8BIT, &rxBuf, 1, PCA9685_I2C_TIMEOUT);

	txBuf[0] = PCA9685_REG_MODE1;
	txBuf[1] = (rxBuf & ~0x80) | 0x10;
	HAL_I2C_Master_Transmit(pca->I2Chandle, PCA9685_I2C_ADDR, txBuf, 2, PCA9685_I2C_TIMEOUT);

	/* Write pre-scale value */
	txBuf[0] = PCA9685_REG_PRSCL;
	txBuf[1] = pca->preScale;
	HAL_I2C_Master_Transmit(pca->I2Chandle, PCA9685_I2C_ADDR, txBuf, 2, PCA9685_I2C_TIMEOUT);

	/* Wake up */
	txBuf[0] = PCA9685_REG_MODE1;
	txBuf[1] = rxBuf;

	HAL_Delay(5);

	/* Enable auto-increment */
	txBuf[1] = rxBuf | 0x80 | 0x20;
	HAL_I2C_Master_Transmit(pca->I2Chandle, PCA9685_I2C_ADDR, txBuf, 2, PCA9685_I2C_TIMEOUT);

	/* Set output mode to totem pole */
	HAL_I2C_Mem_Read(pca->I2Chandle, PCA9685_I2C_ADDR, PCA9685_REG_MODE2, I2C_MEMADD_SIZE_8BIT, &rxBuf, 1, PCA9685_I2C_TIMEOUT);
	rxBuf |= 0x04;

	txBuf[0] = PCA9685_REG_MODE2;
	txBuf[1] = rxBuf;
	HAL_I2C_Master_Transmit(pca->I2Chandle, PCA9685_I2C_ADDR, txBuf, 2, PCA9685_I2C_TIMEOUT);
}

void PCA9685_Set(PCA9685 *pca, uint8_t channel, uint16_t val) {
	/* Limit */
	if (val > 4095) {
		val = 4095;
	}

	if (val == 4095) {
		PCA9685_SetPWM(pca, channel, 4096, 0);
	} else if (val == 0) {
		PCA9685_SetPWM(pca, channel, 0, 4096);
	} else {
		PCA9685_SetPWM(pca, channel, 0, val);
	}

	pca->setting[channel] = val;
}

void PCA9685_SetAll(PCA9685 *pca) {
	int channel;

	for (channel = 0; channel < 8; channel++) {
		PCA9685_Set(pca, channel, pca->setting[channel]);
	}
}

void PCA9685_SetPWM(PCA9685 *pca, uint8_t channel, uint16_t on, uint16_t off) {
	uint8_t txBuf[] = {PCA9685_REG_DATA + 4 * channel,
					  (uint8_t)  on,
					  (uint8_t) (on  >> 8),
					  (uint8_t)  off,
					  (uint8_t) (off >> 8)};
	HAL_I2C_Master_Transmit(pca->I2Chandle, PCA9685_I2C_ADDR, txBuf, 5, PCA9685_I2C_TIMEOUT);
}
