#include "UBLOX.h"

void UBloxGPS_Init(UBloxGPS *gps, UART_HandleTypeDef *uart, GPIO_TypeDef *rstPinBank, uint16_t rstPin, GPIO_TypeDef *ppsPinBank, uint16_t ppsPin, GPIO_TypeDef *lnaEnablePinBank, uint16_t lnaEnablePin) {
	gps->uart             = uart;
	gps->rstPinBank       = rstPinBank;
	gps->rstPin           = rstPin;
	gps->ppsPinBank       = ppsPinBank;
	gps->ppsPin           = ppsPin;
	gps->lnaEnablePinBank = lnaEnablePinBank;
	gps->lnaEnablePin     = lnaEnablePin;

	gps->latitude    = 0.0f;
	gps->longitude   = 0.0f;
	gps->altitude    = 0.0f;
	gps->course      = 0.0f;
	gps->groundSpeed = 0.0f;
}

void UBloxGPS_Reset(UBloxGPS *gps) {
	HAL_GPIO_WritePin(gps->rstPinBank, gps->rstPin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(gps->rstPinBank, gps->rstPin, GPIO_PIN_RESET);
	HAL_Delay(50);
	HAL_GPIO_WritePin(gps->rstPinBank, gps->rstPin, GPIO_PIN_SET);
}
