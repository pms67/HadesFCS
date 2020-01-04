#ifndef MPRLS_SPI_DRIVER_H
#define MPRLS_SPI_DRIVER_H

#include "stm32f4xx_hal.h"

typedef struct {
	SPI_HandleTypeDef *spiHandler;
	GPIO_TypeDef *csPinBank;
	uint16_t csPin;
	GPIO_TypeDef *nResetPinBank;
	uint16_t nResetPin;
	GPIO_TypeDef *eocPinBank;
	uint16_t eocPin;
	float pressure;
} MPRLS;

void MPRLS_Init(MPRLS *bar, SPI_HandleTypeDef *spiHandler,
				   GPIO_TypeDef *csPinBank, uint16_t csPin,
				   GPIO_TypeDef *nResetPinBank, uint16_t nResetPin,
				   GPIO_TypeDef *eocPinBank, uint16_t eocPin);
void MPRLS_Reset(MPRLS *bar);
uint8_t MPRLS_ReadPressure(MPRLS *bar);
uint8_t MPRLS_RequestData(MPRLS *bar);
uint8_t MPRLS_DataReady(MPRLS *bar);

#endif
