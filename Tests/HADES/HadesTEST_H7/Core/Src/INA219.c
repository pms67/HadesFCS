#include "INA219.h"

void INA219_Init32V9A(INA219 *ina, I2C_HandleTypeDef *I2Chandle) {
	ina->I2Chandle       = I2Chandle;
	ina->currentmA       = 0.0f;
	ina->voltageBusmV    = 0.0f;
	ina->voltageShuntmV  = 0.0f;
	ina->powermW         = 0.0f;

	/* Set calibration parameters */
	ina->calVal = 8192;
	ina->curDiv = 2;
	ina->powMul = 10;

	INA219_WriteReg16(ina, INA219_REG_CALI, ina->calVal);

	/* Write to config register */
	uint16_t conf = (0x2000) | (0x1000) | (0x0180) | (0x0018) | (0x0007);
	INA219_WriteReg16(ina, INA219_REG_CONF, conf);

}

void INA219_Init32V2A(INA219 *ina, I2C_HandleTypeDef *I2Chandle) {
	ina->I2Chandle       = I2Chandle;
	ina->currentmA       = 0.0f;
	ina->voltageBusmV    = 0.0f;
	ina->voltageShuntmV  = 0.0f;
	ina->powermW         = 0.0f;

	/* Set calibration parameters */
	ina->calVal = 40960;
	ina->curDiv = 10;
	ina->powMul = 2;

	INA219_WriteReg16(ina, INA219_REG_CALI, ina->calVal);

	/* Write to config register */
	uint16_t conf = (0x2000) | (0x1000) | (0x0180) | (0x0018) | (0x0007);
	INA219_WriteReg16(ina, INA219_REG_CONF, conf);
}

void INA219_Read(INA219 *ina) {
	uint16_t regRaw;
	int16_t regVal;

	/* Bus voltage */
	INA219_ReadReg16(ina, INA219_REG_BUS, &regRaw);
	regVal = (int16_t) ((regRaw >> 3) * 4);
	ina->voltageBusmV = (float) regVal;

	/* Shunt voltage */
	INA219_ReadReg16(ina, INA219_REG_SHNT, &regRaw);
	ina->voltageShuntmV = ((int16_t) regRaw) * 0.01f;

	/*
	 * FROM ADAFRUIT DRIVERL: Sharp load may reset INA219, which also resets cal register. Thus, always set to make sure...
	 */
	INA219_WriteReg16(ina, INA219_REG_CALI, ina->calVal);

	/* Current */
	INA219_ReadReg16(ina, INA219_REG_CUR, &regRaw);
	ina->currentmA = ((int16_t) regRaw) / ((float) ina->curDiv);

	/* Power */
	INA219_ReadReg16(ina, INA219_REG_PWR, &regRaw);
	ina->powermW = ((int16_t) regRaw) * ina->powMul;
}

void INA219_WriteReg16(INA219 *ina, uint8_t reg, uint16_t val) {
	uint8_t txBuf[3];
	txBuf[0] = reg;
	txBuf[1] = (val >> 8) & 0xFF;
	txBuf[2] = val & 0xFF;
	HAL_I2C_Master_Transmit(ina->I2Chandle, INA219_I2C_ADDR, txBuf, 3, INA219_I2C_TIMEOUT);
}

void INA219_ReadReg16(INA219 *ina, uint8_t reg, uint16_t *val) {
	uint8_t rxBuf[2];
	HAL_I2C_Mem_Read(ina->I2Chandle, INA219_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, rxBuf, 2, INA219_I2C_TIMEOUT);

	*val = (rxBuf[0] << 8) | rxBuf[1];
}
