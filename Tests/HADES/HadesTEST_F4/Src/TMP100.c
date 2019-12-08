#include "TMP100.h"

void TMP100_Init(TMP100 *tmp, I2C_HandleTypeDef *I2Chandle) {
	tmp->I2Chandle = I2Chandle;
	tmp->temp_C = 0.0f;

	/* Configure sensor */
	uint8_t txBuf[] = {TMP100_REG_CONF,  0x60};
	HAL_I2C_Master_Transmit(tmp->I2Chandle, TMP100_I2C_ADDR, txBuf, 2, TMP100_I2C_TIMEOUT);
}

void TMP100_Read(TMP100 *tmp) {
	uint8_t rxBuf[2];
	HAL_I2C_Mem_Read(tmp->I2Chandle, TMP100_I2C_ADDR, TMP100_REG_DATA, I2C_MEMADD_SIZE_8BIT, rxBuf, 2, TMP100_I2C_TIMEOUT);

	int16_t temp = (rxBuf[0] * 256 + (rxBuf[1] & 0xF0)) / 16;

	if (temp > 2047) {
		temp -= 4096;
	}

	tmp->temp_C = temp * 0.0625f;
}
