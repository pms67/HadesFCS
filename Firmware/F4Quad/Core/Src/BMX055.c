#include <BMX055.h>

/*
 *
 *
 * INITIALISATION
 *
 *
 */

uint8_t BMX055_Init(BMX055 *bmx, SPI_HandleTypeDef *spiHandler, GPIO_TypeDef *csPinBank, uint16_t csPinAcc, uint16_t csPinGyr, uint16_t csPinMag) {
	uint8_t status = 0;

	bmx->spiHandler = spiHandler;
	bmx->csPinBank  = csPinBank;
	bmx->csPinAcc   = csPinAcc;
	bmx->csPinGyr   = csPinGyr;
	bmx->csPinMag   = csPinMag;

	for (int n = 0; n < 3; n++) {
		bmx->acc[n] = 0.0f;
		bmx->gyr[n] = 0.0f;
		bmx->mag[n] = 0.0f;
	}

	HAL_GPIO_WritePin(bmx->csPinBank, bmx->csPinAcc, GPIO_PIN_SET);
	HAL_GPIO_WritePin(bmx->csPinBank, bmx->csPinGyr, GPIO_PIN_SET);
	HAL_GPIO_WritePin(bmx->csPinBank, bmx->csPinMag, GPIO_PIN_SET);

	/*
	 * ACCELEROMETER
	 */

	/* Check chip ID (should equal 0xFA) */
	uint8_t rxData;

	status += BMX055_ReadRegisterAcc(bmx, BMX055_ACC_CHIPID, &rxData, 1);
	if (rxData != 0xFA) {
		return 0;
	}

	status++;

	/* Reset accelerometer */
	status += BMX055_WriteRegisterAcc(bmx, BMX055_ACC_SORST, 0xB6);
	HAL_Delay(20);

	/* g-range (+- 8g, sensitvity is 256 LSB/g) ==> 0x08 */
	status += BMX055_WriteRegisterAcc(bmx, BMX055_ACC_RANGE, 0x08);
	HAL_Delay(5);

	/* Filter bandwidth (31.25 Hz) ==> 0x0A */
	status += BMX055_WriteRegisterAcc(bmx, BMX055_ACC_BW, 0x0A);
	HAL_Delay(5);

	/* Main power mode (NORMAL mode) ==> 0x00 */
	status += BMX055_WriteRegisterAcc(bmx, BMX055_ACC_LPW, 0x00);
	HAL_Delay(5);

	/* Data acquisition and data output format (Filtered data, no shadowing) ==> 0x40 */
	status += BMX055_WriteRegisterAcc(bmx, BMX055_ACC_HBW, 0x40);
	HAL_Delay(5);

	/*
	 *  GYROSCOPE
	 */

	/* Check chip ID (should equal 0x0F) */
	status += BMX055_ReadRegisterGyr(bmx, BMX055_GYR_CHIPID, &rxData, 1);
	if (rxData != 0x0F) {
		return 0;
	}

	/* Reset gyroscope */
	status += BMX055_WriteRegisterGyr(bmx, BMX055_GYR_SORST, 0xB6);
	HAL_Delay(20);

	/* Range (+- 1000 deg/s, sensitivity is 32.8 LSB/deg/s) ==> 0x01 */
	status += BMX055_WriteRegisterGyr(bmx, BMX055_GYR_RANGE, 0x01);
	HAL_Delay(5);

	/* Bandwidth (ODR = 100 Hz, Filter Bandwidth = 32 Hz) ==> 0x87 */
	status += BMX055_WriteRegisterGyr(bmx, BMX055_GYR_BW, 0x87);
	HAL_Delay(5);

	/* Main power mode (NORMAL mode) ==> 0x00 */
	status += BMX055_WriteRegisterGyr(bmx, BMX055_GYR_LPM1, 0x00);
	HAL_Delay(5);

	/* Fast power up and external trigger ==> 0x00 */
	status += BMX055_WriteRegisterGyr(bmx, BMX055_GYR_LPM2, 0x00);
	HAL_Delay(5);

	/* Angular rate data acquisition and data output format (Filtered, no shadowing) ==> 0x40 */
	status += BMX055_WriteRegisterGyr(bmx, BMX055_GYR_HBW, 0x40);
	HAL_Delay(5);

	/*
	 *  MAGNETOMETER
	 */

	/* Get magnetometer out of suspend mode */
	status += BMX055_WriteRegisterMag(bmx, BMX055_MAG_PWR, 0x00);
	HAL_Delay(5);

	/* Reset magnetometer */
	status += BMX055_WriteRegisterMag(bmx, BMX055_MAG_PWR, 0x83);
	HAL_Delay(20);

	/* Check chip ID (should equal 0x32) */
	status += BMX055_ReadRegisterMag(bmx, BMX055_MAG_CHIPID, &rxData, 1);
	if (rxData != 0x32) {
		return 0;
	}

	/* Operation mode and output data rate (ODR = 10 Hz, Normal mode) ==> 0x00 */
	status += BMX055_WriteRegisterMag(bmx, BMX055_MAG_MODE, 0x00);
	HAL_Delay(5);

	return status;
}

/*
 *
 *
 * HIGH LEVEL READ FUNCTIONS
 *
 *
 */

uint8_t BMX055_ReadAccelerometer(BMX055 *bmx) {
	uint8_t accData[7];
	int16_t rawData;

	/* Read all data registers */
	uint8_t status = BMX055_ReadRegisterAcc(bmx, BMX055_ACC_DATA, accData, 7);

	/* X */
	rawData = (int16_t) (((int16_t) accData[1] << 8) | (accData[0] & 0xF0)) >> 4;
	bmx->acc[0] =  BMX055_ACC_CONV * rawData;

	/* Y */
	rawData = (int16_t) (((int16_t) accData[3] << 8) | (accData[2] & 0xF0)) >> 4;
	bmx->acc[1] = -BMX055_ACC_CONV * rawData;

	/* Z */
	rawData = (int16_t) (((int16_t) accData[5] << 8) | (accData[4] & 0xF0)) >> 4;
	bmx->acc[2] = -BMX055_ACC_CONV * rawData;

	/* Temperature */
	bmx->temp = 23.0f + BMX055_TMP_CONV * ((int8_t) accData[6]);

	return status;
}

uint8_t BMX055_ReadGyroscope(BMX055 *bmx) {
	uint8_t gyrData[6];
	int16_t rawData;

	/* Read all data registers */
	uint8_t status = BMX055_ReadRegisterGyr(bmx, BMX055_GYR_DATA, gyrData, 6);

	/* X */
	rawData = ((int16_t) gyrData[1] << 8) | gyrData[0];
	bmx->gyr[0] =  BMX055_GYR_CONV * rawData;

	/* Y */
	rawData = ((int16_t) gyrData[3] << 8) | gyrData[2];
	bmx->gyr[1] = -BMX055_GYR_CONV * rawData;

	/* Z */
	rawData = ((int16_t) gyrData[5] << 8) | gyrData[4];
	bmx->gyr[2] = -BMX055_GYR_CONV * rawData;

	return status;
}

uint8_t BMX055_ReadMagnetometer(BMX055 *bmx) {
	uint8_t magData[6];
	int16_t rawData;

	/* Read all data registers */
	uint8_t status = BMX055_ReadRegisterMag(bmx, BMX055_MAG_DATA, magData, 6);

	/* X */
	rawData = (int16_t) ((int8_t) magData[1] << 5) | ((magData[0] & 0xF8) >> 3);
	bmx->mag[0] =  BMX055_MAG_CONV * rawData;

	/* Y */
	rawData = (int16_t) ((int8_t) magData[3] << 5) | ((magData[2] & 0xF8) >> 3);
	bmx->mag[1] = -BMX055_MAG_CONV * rawData;

	/* Z */
	rawData = (int16_t) ((int8_t) magData[5] << 7) | ((magData[4] & 0xFE) >> 1); /* Z MSB is shifted by 7 bits ! */
	bmx->mag[2] = -BMX055_MAG_CONV * rawData;

	return status;
}

/*
 *
 *
 * LOW-LEVEL REGISTER FUNCTIONS
 *
 *
 */

uint8_t BMX055_ReadRegisterAcc(BMX055 *bmx, uint8_t regAddr, uint8_t *data, uint16_t numBytes) {
	HAL_GPIO_WritePin(bmx->csPinBank, bmx->csPinAcc, GPIO_PIN_RESET);

	/* Select register by transmitting address with R/W bit set to 1 */
	uint8_t txData = regAddr | 0x80;
	if (HAL_SPI_Transmit(bmx->spiHandler, &txData, 1, HAL_MAX_DELAY) != HAL_OK) {
		/* Transmission did not succeed, disable CS and return zero */
		HAL_GPIO_WritePin(bmx->csPinBank, bmx->csPinAcc, GPIO_PIN_SET);
		return 0;
	}

	/* Read register content(s) */
	if (HAL_SPI_Receive(bmx->spiHandler, data, numBytes, HAL_MAX_DELAY) != HAL_OK) {
		/* Reception did not succeed, disable CS and return zero */
		HAL_GPIO_WritePin(bmx->csPinBank, bmx->csPinAcc, GPIO_PIN_SET);
		return 0;
	}

	/* Pull CS high to deselect chip */
	HAL_GPIO_WritePin(bmx->csPinBank, bmx->csPinAcc, GPIO_PIN_SET);

	/* All transmissions OK */
	return 1;
}

uint8_t BMX055_WriteRegisterAcc(BMX055 *bmx, uint8_t regAddr, uint8_t val) {
	HAL_GPIO_WritePin(bmx->csPinBank, bmx->csPinAcc, GPIO_PIN_RESET);

	uint8_t txData[] = {regAddr, val};
	uint8_t status = (HAL_SPI_Transmit(bmx->spiHandler, txData, 2, HAL_MAX_DELAY) == HAL_OK);

	HAL_GPIO_WritePin(bmx->csPinBank, bmx->csPinAcc, GPIO_PIN_SET);

	return status;
}

uint8_t BMX055_ReadRegisterGyr(BMX055 *bmx, uint8_t regAddr, uint8_t *data, uint16_t numBytes) {
	HAL_GPIO_WritePin(bmx->csPinBank, bmx->csPinGyr, GPIO_PIN_RESET);

	/* Select register by transmitting address with R/W bit set to 1 */
	uint8_t txData = regAddr | 0x80;
	if (HAL_SPI_Transmit(bmx->spiHandler, &txData, 1, HAL_MAX_DELAY) != HAL_OK) {
		/* Transmission did not succeed, disable CS and return zero */
		HAL_GPIO_WritePin(bmx->csPinBank, bmx->csPinGyr, GPIO_PIN_SET);
		return 0;
	}

	/* Read register content(s) */
	if (HAL_SPI_Receive(bmx->spiHandler, data, numBytes, HAL_MAX_DELAY) != HAL_OK) {
		/* Reception did not succeed, disable CS and return zero */
		HAL_GPIO_WritePin(bmx->csPinBank, bmx->csPinGyr, GPIO_PIN_SET);
		return 0;
	}

	/* Pull CS high to deselect chip */
	HAL_GPIO_WritePin(bmx->csPinBank, bmx->csPinGyr, GPIO_PIN_SET);

	/* All transmissions OK */
	return 1;
}

uint8_t BMX055_WriteRegisterGyr(BMX055 *bmx, uint8_t regAddr, uint8_t val) {
	HAL_GPIO_WritePin(bmx->csPinBank, bmx->csPinGyr, GPIO_PIN_RESET);

	uint8_t txData[] = {regAddr, val};
	uint8_t status = (HAL_SPI_Transmit(bmx->spiHandler, txData, 2, HAL_MAX_DELAY) == HAL_OK);

	HAL_GPIO_WritePin(bmx->csPinBank, bmx->csPinGyr, GPIO_PIN_SET);

	return status;
}

uint8_t BMX055_ReadRegisterMag(BMX055 *bmx, uint8_t regAddr, uint8_t *data, uint16_t numBytes) {
	HAL_GPIO_WritePin(bmx->csPinBank, bmx->csPinMag, GPIO_PIN_RESET);

	/* Select register by transmitting address with R/W bit set to 1 */
	uint8_t txData = regAddr | 0x80;
	if (HAL_SPI_Transmit(bmx->spiHandler, &txData, 1, HAL_MAX_DELAY) != HAL_OK) {
		/* Transmission did not succeed, disable CS and return zero */
		HAL_GPIO_WritePin(bmx->csPinBank, bmx->csPinMag, GPIO_PIN_SET);
		return 0;
	}

	/* Read register content(s) */
	if (HAL_SPI_Receive(bmx->spiHandler, data, numBytes, HAL_MAX_DELAY) != HAL_OK) {
		/* Reception did not succeed, disable CS and return zero */
		HAL_GPIO_WritePin(bmx->csPinBank, bmx->csPinMag, GPIO_PIN_SET);
		return 0;
	}

	/* Pull CS high to deselect chip */
	HAL_GPIO_WritePin(bmx->csPinBank, bmx->csPinMag, GPIO_PIN_SET);

	/* All transmissions OK */
	return 1;
}

uint8_t BMX055_WriteRegisterMag(BMX055 *bmx, uint8_t regAddr, uint8_t val) {
	HAL_GPIO_WritePin(bmx->csPinBank, bmx->csPinMag, GPIO_PIN_RESET);

	uint8_t txData[] = {regAddr, val};
	uint8_t status = (HAL_SPI_Transmit(bmx->spiHandler, txData, 2, HAL_MAX_DELAY) == HAL_OK);

	HAL_GPIO_WritePin(bmx->csPinBank, bmx->csPinMag, GPIO_PIN_SET);

	return status;
}
