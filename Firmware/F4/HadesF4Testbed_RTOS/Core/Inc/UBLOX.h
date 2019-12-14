#ifndef UBLOX_GPS_H
#define UBLOX_GPS_H

#include "stm32f4xx_hal.h"

typedef struct {
	UART_HandleTypeDef *uart;
	GPIO_TypeDef *rstPinBank;
	uint16_t rstPin;
	GPIO_TypeDef *ppsPinBank;
	uint16_t ppsPin;
	GPIO_TypeDef *lnaEnablePinBank;
	uint16_t lnaEnablePin;
	float latitude;
	float longitude;
	float altitude;
	float course;
	float groundSpeed;
	char uartBuf[128];
	uint8_t uartBufIndex;
	uint8_t uartBufLength;
} UBloxGPS;

void UBloxGPS_Init(UBloxGPS *gps, UART_HandleTypeDef *uart, GPIO_TypeDef *rstPinBank, uint16_t rstPin, GPIO_TypeDef *ppsPinBank, uint16_t ppsPin, GPIO_TypeDef *lnaEnablePinBank, uint16_t lnaEnablePin);
void UBloxGPS_Reset(UBloxGPS *gps);
uint8_t UBloxGPS_PutBuf(UBloxGPS *gps, char c);

#endif
