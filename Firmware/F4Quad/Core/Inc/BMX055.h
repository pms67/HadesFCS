#ifndef BMX055_SPI_DRIVER_H
#define BMX055_SPI_DRIVER_H

#include "stm32f4xx_hal.h"

/* Accelerometer defines */
#define BMX055_ACC_CHIPID 0x00
#define BMX055_ACC_DATA   0x02
#define BMX055_ACC_TEMP   0x08
#define BMX055_ACC_RANGE  0x0F
#define BMX055_ACC_BW     0x10
#define BMX055_ACC_LPW    0x11
#define BMX055_ACC_LOPOW  0x12
#define BMX055_ACC_HBW    0x13
#define BMX055_ACC_SORST  0x14

/* Gyroscope defines */
#define BMX055_GYR_CHIPID 0x00
#define BMX055_GYR_DATA   0x02
#define BMX055_GYR_RANGE  0x0F
#define BMX055_GYR_BW     0x10
#define BMX055_GYR_LPM1   0x11
#define BMX055_GYR_LPM2   0x12
#define BMX055_GYR_HBW    0x13
#define BMX055_GYR_SORST  0x14

/* Magnetometer defines */
#define BMX055_MAG_CHIPID 0x40
#define BMX055_MAG_DATA   0x42
#define BMX055_MAG_PWR    0x4B
#define BMX055_MAG_MODE   0x4C

/* Conversion factors */
#define BMX055_ACC_CONV ((float) 0.03834400150f) /* mps2 per LSB @ +-8g range */
#define BMX055_GYR_CONV ((float) 0.00053211257f) /* radps per LSB @ +- 1000 deg/s range */
#define BMX055_MAG_CONV ((float) 0.00308794466f) /* uT per LSB */
#define BMX055_TMP_CONV ((float) 0.5f) /* deg C per LSB */

/* BMX data container */
typedef struct {
	SPI_HandleTypeDef *spiHandler;
	GPIO_TypeDef *csPinBank;
	uint16_t csPinAcc;
	uint16_t csPinGyr;
	uint16_t csPinMag;
	float acc[3]; /* Units: meters per second squared */
	float gyr[3]; /* Units: radians per second */
	float mag[3]; /* Units: micro Tesla */
	float temp;   /* Units: degrees Celsius */
} BMX055;

/* Register read functions */
uint8_t BMX055_ReadRegisterAcc(BMX055 *bmx, uint8_t regAddr, uint8_t *data, uint16_t numBytes);
uint8_t BMX055_ReadRegisterGyr(BMX055 *bmx, uint8_t regAddr, uint8_t *data, uint16_t numBytes);
uint8_t BMX055_ReadRegisterMag(BMX055 *bmx, uint8_t regAddr, uint8_t *data, uint16_t numBytes);

/* Write to register */
uint8_t BMX055_WriteRegisterAcc(BMX055 *bmx, uint8_t regAddr, uint8_t val);
uint8_t BMX055_WriteRegisterGyr(BMX055 *bmx, uint8_t regAddr, uint8_t val);
uint8_t BMX055_WriteRegisterMag(BMX055 *bmx, uint8_t regAddr, uint8_t val);

/* Initialise sensors */
uint8_t BMX055_Init(BMX055 *bmx, SPI_HandleTypeDef *spiHandler, GPIO_TypeDef *csPinBank, uint16_t csPinAcc, uint16_t csPinGyr, uint16_t csPinMag);

/* Data acquisition functions */
uint8_t BMX055_ReadAccelerometer(BMX055 *bmx);
uint8_t BMX055_ReadGyroscope(BMX055 *bmx);
uint8_t BMX055_ReadMagnetometer(BMX055 *bmx);

#endif
