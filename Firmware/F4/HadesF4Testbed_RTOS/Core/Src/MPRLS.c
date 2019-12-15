#include "MPRLS.h"

uint8_t MPRLSBarometer_Init(MPRLSBarometer *bar, I2C_HandleTypeDef *I2Chandle, GPIO_TypeDef *rstPinBank, uint16_t rstPin, GPIO_TypeDef *intPinBank, uint16_t intPin) {
	bar->I2Chandle  = I2Chandle;
	bar->rstPinBank = rstPinBank;
	bar->rstPin     = rstPin;
	bar->intPinBank = intPinBank;
	bar->intPin     = intPin;
	bar->pressurePa = 0.0f;

	MPRLSBarometer_Reset(bar);

	HAL_Delay(10);

	uint8_t status = MPRLSBarometer_ReadStatus(bar);

	return status;
}

void MPRLSBarometer_Reset(MPRLSBarometer *bar) {
	HAL_GPIO_WritePin(bar->rstPinBank, bar->rstPin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(bar->rstPinBank, bar->rstPin, GPIO_PIN_RESET);
	HAL_Delay(10);
	HAL_GPIO_WritePin(bar->rstPinBank, bar->rstPin, GPIO_PIN_SET);
	HAL_Delay(50);
}

uint8_t MPRLSBarometer_ReadStatus(MPRLSBarometer *bar) {
	uint8_t status;
	HAL_I2C_Master_Receive(bar->I2Chandle, MPRLS_I2C_ADDR, &status, 1, MPRLS_I2C_TIMEOUT);

	return status;
}

uint8_t MPRLSBarometer_ReadPressure(MPRLSBarometer *bar) {
	/* Send read data request */
	uint8_t txBuf[3] = {0xAA, 0x00, 0x00};
	HAL_I2C_Master_Transmit(bar->I2Chandle, MPRLS_I2C_ADDR, txBuf, 3, MPRLS_I2C_TIMEOUT);

	/* Wait until EOC indicator is set */
	while (!HAL_GPIO_ReadPin(bar->intPinBank, bar->intPin)) {
		HAL_Delay(5);
	}

	/* Request four bytes (1x status, 3x data) */
	uint8_t rxBuf[4];
	HAL_I2C_Master_Receive(bar->I2Chandle, MPRLS_I2C_ADDR, rxBuf, 4, MPRLS_I2C_TIMEOUT);

	/* Check status byte */
	if ((rxBuf[0] & MPRLS_STATUS_MATHSAT) || (rxBuf[0] & MPRLS_STATUS_FAILED)) {
		return 0;
	}

	/* Compute raw pressure reading */
	uint32_t pressureRaw = rxBuf[1];
			 pressureRaw <<= 8;
			 pressureRaw |= rxBuf[2];
			 pressureRaw <<= 8;
			 pressureRaw |= rxBuf[3];

	/* Convert to pressure reading in Pascal */
	float psi  = (pressureRaw - 0x19999A) * (MPRLS_PSI_MAX - MPRLS_PSI_MIN);
		  psi /= (float) (0xE66666 - 0x19999A);
		  psi += MPRLS_PSI_MIN;

	bar->pressurePa = MPRLS_PSI_TO_PA * psi;

	/* Success */
	return 1;
}
