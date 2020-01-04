#include "MPRLS_SPI.h"

void MPRLS_Init(MPRLS *bar, SPI_HandleTypeDef *spiHandler,
				   GPIO_TypeDef *csPinBank, uint16_t csPin,
				   GPIO_TypeDef *nResetPinBank, uint16_t nResetPin,
				   GPIO_TypeDef *eocPinBank, uint16_t eocPin) {

	bar->spiHandler    = spiHandler;
	bar->csPinBank     = csPinBank;
	bar->csPin         = csPin;
	bar->nResetPinBank = nResetPinBank;
	bar->nResetPin     = nResetPin;
	bar->eocPinBank    = eocPinBank;
	bar->eocPin        = eocPin;
	bar->pressure      = 0.0f;

	MPRLS_Reset(bar);
}

void MPRLS_Reset(MPRLS *bar) {
	HAL_GPIO_WritePin(bar->nResetPinBank, bar->nResetPin, GPIO_PIN_RESET);
	HAL_Delay(5);
	HAL_GPIO_WritePin(bar->nResetPinBank, bar->nResetPin, GPIO_PIN_SET);
}

uint8_t MPRLS_ReadPressure(MPRLS *bar) {
	/* Select chip */
	HAL_GPIO_WritePin(bar->csPinBank, bar->csPin, GPIO_PIN_RESET);

	uint8_t data[4];
	if (HAL_SPI_Receive(bar->spiHandler, data, 4, HAL_MAX_DELAY) != HAL_OK) {
		/* Transmission did not succeed, disable CS and return zero */
		HAL_GPIO_WritePin(bar->csPinBank, bar->csPin, GPIO_PIN_SET);
		return 0;
	}

	/* Pull CS high to deselect chip */
	HAL_GPIO_WritePin(bar->csPinBank, bar->csPin, GPIO_PIN_SET);

	/* First byte is 'Status Byte' (0 POWERED BUSY 0 0 !INTEGRITY 0 SATURATION) */
	uint8_t status = data[0];

	/* Check if device powered, not busy, integrity check passed, not saturated */
	if (status == 0x40) {
		int32_t pressure = (int32_t) ((data[1] << 16) | (data[2] << 8) | data[3]);

		/* Convert to pressure in Pascal */
		bar->pressure = pressure * 0.00770549173f;
	}

	return status;
}

uint8_t MPRLS_RequestData(MPRLS *bar) {
	/* Select chip */
	HAL_GPIO_WritePin(bar->csPinBank, bar->csPin, GPIO_PIN_RESET);

	uint8_t data[] = {0xAA, 0x00, 0x00};
	if (HAL_SPI_Transmit(bar->spiHandler, data, 3, HAL_MAX_DELAY) != HAL_OK) {
		/* Transmission did not succeed, disable CS and return zero */
		HAL_GPIO_WritePin(bar->csPinBank, bar->csPin, GPIO_PIN_SET);
		return 0;
	}

	/* Pull CS high to deselect chip */
	HAL_GPIO_WritePin(bar->csPinBank, bar->csPin, GPIO_PIN_SET);

	return 1;
}

uint8_t MPRLS_DataReady(MPRLS *bar) {
	return HAL_GPIO_ReadPin(bar->eocPinBank, bar->eocPin);
}
