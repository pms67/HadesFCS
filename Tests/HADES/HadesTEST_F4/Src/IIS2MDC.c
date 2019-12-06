#include "IIS2MDC.h"

uint8_t IISMagnetometer_Init(IISMagnetometer *mag, I2C_HandleTypeDef *I2Chandle, GPIO_TypeDef *intPinBank, uint16_t intPin) {
	mag->I2Chandle  = I2Chandle;
	mag->intPinBank = intPinBank;
	mag->intPin     = intPin;
	mag->x          = 0;
	mag->y          = 0;
	mag->z          = 0;
	mag->temp_C     = 0.0f;

	/* Check device ID register */
	uint8_t whoAmI;
	HAL_I2C_Mem_Read(mag->I2Chandle, IIS_I2C_ADDR, IIS_WHOAMI_REG, I2C_MEMADD_SIZE_8BIT, &whoAmI, 1, IIS_I2C_TIMEOUT);

	if (whoAmI != IIS_WHOAMI) {
		return 0;
	}

	/* Configure sensor */
	uint8_t txBuf[2];

	/* Temperature compensation = 0, Reboot = 0, Soft_Rst = 0, Low Power = 0, ODR 100 Hz = 11, MODE CONTINUOUS 00 */
	uint8_t cfgRegA = 0x0C;
	txBuf[0] = IIS_CFG_REG_A; txBuf[1] = cfgRegA;
	HAL_I2C_Master_Transmit(mag->I2Chandle, IIS_I2C_ADDR, txBuf, 2, IIS_I2C_TIMEOUT);

	/* 0 0 0, Offset cancelation = 0, INT_on_DataOff = 0, Set_Freq = 0, Offset cancellation = 0, Low-pass filter = 1 */
	uint8_t cfgRegB = 0x01;
	txBuf[0] = IIS_CFG_REG_B; txBuf[1] = cfgRegB;
	HAL_I2C_Master_Transmit(mag->I2Chandle, IIS_I2C_ADDR, txBuf, 2, IIS_I2C_TIMEOUT);

	/* 0, INT_on_PIN = 0, I2C_DIS = 0, BDU = 0, BLE = 0, 0, Self_test = 0, DRDY_on_PIN = 1 */
	uint8_t cfgRegC = 0x01;
	txBuf[0] = IIS_CFG_REG_C; txBuf[1] = cfgRegC;
	HAL_I2C_Master_Transmit(mag->I2Chandle, IIS_I2C_ADDR, txBuf, 2, IIS_I2C_TIMEOUT);

	return 1;
}

void IISMagnetomer_Read(IISMagnetometer *mag) {
	/* Wait until DRDY pin is set */
	while (!HAL_GPIO_ReadPin(mag->intPinBank, mag->intPin)) {
		HAL_Delay(5);
	}

	/* Read raw X, Y, and Z values */
	uint8_t rxBuf[2];
	HAL_I2C_Mem_Read(mag->I2Chandle, IIS_I2C_ADDR, IIS_OUTX_LOW, I2C_MEMADD_SIZE_8BIT, rxBuf, 2, IIS_I2C_TIMEOUT);

	mag->x = rxBuf[1];
	mag->x <<= 8;
	mag->x |= rxBuf[0];

	HAL_I2C_Mem_Read(mag->I2Chandle, IIS_I2C_ADDR, IIS_OUTY_LOW, I2C_MEMADD_SIZE_8BIT, rxBuf, 2, IIS_I2C_TIMEOUT);
	mag->y = rxBuf[1];
	mag->y <<= 8;
	mag->y |= rxBuf[0];

	HAL_I2C_Mem_Read(mag->I2Chandle, IIS_I2C_ADDR, IIS_OUTZ_LOW, I2C_MEMADD_SIZE_8BIT, rxBuf, 2, IIS_I2C_TIMEOUT);
	mag->z = rxBuf[1];
	mag->z <<= 8;
	mag->z |= rxBuf[0];

	/* Read temperature */
	HAL_I2C_Mem_Read(mag->I2Chandle, IIS_I2C_ADDR, IIS_TEMP_LOW, I2C_MEMADD_SIZE_8BIT, rxBuf, 2, IIS_I2C_TIMEOUT);
	int16_t temp = rxBuf[1];
			temp <<= 8;
			temp |= rxBuf[0];

	mag->temp_C = temp / 8.0f;
}
