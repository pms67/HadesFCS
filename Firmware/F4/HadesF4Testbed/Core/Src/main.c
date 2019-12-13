#include "main.h"

#include <stdio.h>
#include <string.h>

#include "MPRLS.h"
#include "IIS2MDC.h"
#include "BMI088.h"
#include "TMP100.h"
#include "UBLOX.h"

#include "GPSNMEAParser.h"

#include "KalmanQuatAtt.h"

I2C_HandleTypeDef hi2c1; /* GPS */
I2C_HandleTypeDef hi2c2;
I2C_HandleTypeDef hi2c3;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2C2_Init(void);
static void MX_I2C3_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART3_UART_Init(void);

/* Sensors */
MPRLSBarometer bar;
IISMagnetometer mag;
BMI088IMU imu;
TMP100 tmp;

/* GPS receiver */
UBloxGPS gps;

/* GPS parsed data container */
GPSData gpsData;

/* Timing */
uint32_t timerBar = 0;
uint32_t timerMag = 0;
uint32_t timerAcc = 0;
uint32_t timerGyr = 0;
uint32_t timerTmp = 0;
uint32_t timerEKF = 0;
uint32_t timerGPSDbg = 0;
uint32_t timerDbg = 0;
uint32_t timerLED = 0;

const uint32_t SAMPLE_TIME_BAR_MS = 10;
const uint32_t SAMPLE_TIME_MAG_MS = 10;
const uint32_t SAMPLE_TIME_ACC_MS = 5;
const uint32_t SAMPLE_TIME_GYR_MS = 1;
const uint32_t SAMPLE_TIME_TMP_MS = 320;
const uint32_t SAMPLE_TIME_EKF_MS = 100;
const uint32_t SAMPLE_TIME_GPSDBG_MS = 1000;
const uint32_t SAMPLE_TIME_DBG_MS = 250;
const uint32_t SAMPLE_TIME_LED_MS = 1000;

/* UART-to-USB debug output */
void printDebug(char *buf) {
	HAL_UART_Transmit(&huart3, (uint8_t *) buf, strlen(buf), HAL_MAX_DELAY);
}

int main(void)
{
   HAL_Init();

    SystemClock_Config();

    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_I2C2_Init();
    MX_I2C3_Init();
    MX_USART1_UART_Init();
  	MX_USART2_UART_Init();
  	MX_USART3_UART_Init();

    HAL_GPIO_WritePin(GPIOB, LEDA_Pin|LEDB_Pin|LEDC_Pin|LEDD_Pin, GPIO_PIN_SET);
    HAL_Delay(100);

  	printDebug("NAVC started.\r\n");

  	initPeripherals();

  	updateEKFQuatAtt_initialize();

  	//GPSNMEAParser_Init(&gpsData);

    printDebug("Starting main loop...\r\n");

    float roll_deg = 0.0f;
    float pitch_deg = 0.0f;
    float yaw_deg = 0.0f;

    while (1)
    {
    	pollSensors();
    	//pollGPS();

		//printSensorData();

    	if (HAL_GetTick() - timerEKF >= SAMPLE_TIME_EKF_MS) {
    	//	updateEKFQuatAtt(imu.gyr, imu.acc, mag.xyz, 0.0f, 0.0f, (SAMPLE_TIME_EKF_MS / 1000.0f), 1.0f, &roll_deg, &pitch_deg, &yaw_deg);

    		timerEKF += SAMPLE_TIME_EKF_MS;
    	}



    	/* Heartbeat LED */
    	if (HAL_GetTick() - timerLED >= SAMPLE_TIME_LED_MS) {
    		HAL_GPIO_TogglePin(GPIOB, LEDA_Pin);
    		HAL_GPIO_TogglePin(GPIOB, LEDC_Pin);
    		HAL_GPIO_TogglePin(GPIOB, LEDD_Pin);

    		char buf[32];
    		sprintf(buf, "%f %f %f\r\n", roll_deg, pitch_deg, yaw_deg);

    		printDebug(buf);

    		timerLED += SAMPLE_TIME_LED_MS;
    	}

    }
}

void printSensorData() {
	/* Debug USB output */
	if (HAL_GetTick() - timerDbg >= SAMPLE_TIME_DBG_MS) {
		char buf[256];
		sprintf(buf, "[%ld] Bar: %f | Mag: %f %f %f | Acc: %f %f %f | Gyr: %f %f %f | Tmp: %f\r\n",
				HAL_GetTick(),
				bar.pressurePa,
				mag.xyz[0], mag.xyz[1], mag.xyz[2],
				imu.acc[0], imu.acc[1], imu.acc[2],
			  	  imu.gyr[0], imu.gyr[1], imu.gyr[2],
				  tmp.temp_C);

		printDebug(buf);

		timerDbg += SAMPLE_TIME_DBG_MS;
	}
}

void initPeripherals() {
	printDebug("Initialising sensors...\r\n");
	HAL_Delay(100);

	/* Initialise pressure sensor */
	uint8_t statBar = (MPRLSBarometer_Init(&bar, &hi2c1, BARNRST_GPIO_Port, BARNRST_Pin, INTBAR_GPIO_Port, INTBAR_Pin) == MPRLS_STATUS_POWERED);
	if (statBar == 1) {
	  printDebug("Barometer initialised.\r\n");
	}

	/* Initialise magnetometer */
	uint8_t statMag = IISMagnetometer_Init(&mag, &hi2c1, GPIOA, INTMAG_Pin);
	if (statMag == 1) {
	  printDebug("Magnetometer initialised.\r\n");
	}

	/* Initialise IMU */
	uint8_t statIMU = BMI088_Init(&imu, &hi2c1, GPIOA, INTACC_Pin, GPIOA, INTGYR_Pin);
	if (statIMU == 1) {
	  printDebug("IMU initialised.\r\n");
	}

	/* Initialise temperature sensor */
	TMP100_Init(&tmp, &hi2c1);
	printDebug("Temperature sensor initialised.\r\n");

	/* Initialise GPS receiver */
	UBloxGPS_Init(&gps, &huart1, GPIOC, GPSNRST_Pin, GPIOC, GPSPPS_Pin, GPIOC, GPSLNAEN_Pin);
	UBloxGPS_Reset(&gps);
	printDebug("GPS receiver initialised.\r\n");

	uint8_t status = statBar + statMag + statIMU;
	if (status < 3) {
		printDebug("Error: at least one sensor could not be initialised!\r\n");
		HAL_GPIO_WritePin(GPIOB, LEDB_Pin, GPIO_PIN_SET);
	} else {
		HAL_GPIO_WritePin(GPIOB, LEDB_Pin, GPIO_PIN_RESET);
	}
}

void pollSensors() {
	 /* Gyroscope and Kalman filter prediction */
	  	  if (HAL_GetTick() - timerGyr >= SAMPLE_TIME_GYR_MS) {
	  		//  BMI088_ReadGyr(&imu);

	  		  timerGyr += SAMPLE_TIME_GYR_MS;
	  	  }

	  	  /* Accelerometer and Kalman filter update */
	  	  if (HAL_GetTick() - timerAcc >= SAMPLE_TIME_ACC_MS) {
	  	//	  BMI088_ReadAcc(&imu);

	  		  timerAcc += SAMPLE_TIME_ACC_MS;
	  	  }

	  	  /* Barometer */
	  	  if (HAL_GetTick() - timerBar >= SAMPLE_TIME_BAR_MS) {
	  		  /* Read pressure */
	  		  MPRLSBarometer_ReadPressure(&bar);

	  		  timerBar += SAMPLE_TIME_BAR_MS;
	  	  }

	  	  /* Magnetometer */
	  	  if (HAL_GetTick() - timerMag >= SAMPLE_TIME_MAG_MS) {
	  		  IISMagnetomer_Read(&mag);

	  		  timerMag += SAMPLE_TIME_MAG_MS;
	  	  }

	  	  /* Temperature sensor */
	  	  if (HAL_GetTick() - timerTmp >= SAMPLE_TIME_TMP_MS) {
	  		  TMP100_Read(&tmp);

	  		  timerTmp += SAMPLE_TIME_TMP_MS;
	  	  }
}

void pollGPS() {
	char gpsRxBuf;
	HAL_UART_Receive(&huart1, (uint8_t *) &gpsRxBuf, 1, HAL_MAX_DELAY);

	GPSNMEAParser_Feed(&gpsData, gpsRxBuf);



	/* GPS debug output */
	if (HAL_GetTick() - timerGPSDbg >= SAMPLE_TIME_GPSDBG_MS) {

		/*
    	if (gpsData.fixQuality > 0 || gpsData.fix == 1) {
    		HAL_GPIO_WritePin(GPIOB, LEDC_Pin, GPIO_PIN_SET);
    	} else {
    		HAL_GPIO_WritePin(GPIOB, LEDC_Pin, GPIO_PIN_RESET);
    	}
    	*/

		char gpsDebugBuf[1024];
		sprintf(gpsDebugBuf, "Fix: %d | Num: %d | Lat: %f | Lon: %f | Alt: %f | Spd: %f | Crs: %f | Mag: %f | MSL: %f\r\n", gpsData.fixQuality, gpsData.numSatellites,
				gpsData.latitude_dec, gpsData.longitude_dec, gpsData.altitude_m,
				gpsData.groundSpeed_mps, gpsData.course_deg, gpsData.magVariation_deg, gpsData.meanSeaLevel_m);

		printDebug(gpsDebugBuf);

		timerGPSDbg += SAMPLE_TIME_GPSDBG_MS;
	}
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 100000;
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief I2C3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C3_Init(void)
{

  /* USER CODE BEGIN I2C3_Init 0 */

  /* USER CODE END I2C3_Init 0 */

  /* USER CODE BEGIN I2C3_Init 1 */

  /* USER CODE END I2C3_Init 1 */
  hi2c3.Instance = I2C3;
  hi2c3.Init.ClockSpeed = 100000;
  hi2c3.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c3.Init.OwnAddress1 = 0;
  hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c3.Init.OwnAddress2 = 0;
  hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C3_Init 2 */

  /* USER CODE END I2C3_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9800;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPSNRST_Pin|FCCTXA_Pin|FCCTXB_Pin|FCCTXC_Pin 
                          |FCCTXD_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(BARNRST_GPIO_Port, BARNRST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LEDA_Pin|LEDB_Pin|LEDC_Pin|LEDD_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : GPSLNAEN_Pin FCCRXA_Pin FCCRXB_Pin FCCRXC_Pin 
                           FCCRXD_Pin GPSPPS_Pin */
  GPIO_InitStruct.Pin = GPSLNAEN_Pin|FCCRXA_Pin|FCCRXB_Pin|FCCRXC_Pin 
                          |FCCRXD_Pin|GPSPPS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : GPSNRST_Pin FCCTXA_Pin FCCTXB_Pin FCCTXC_Pin 
                           FCCTXD_Pin */
  GPIO_InitStruct.Pin = GPSNRST_Pin|FCCTXA_Pin|FCCTXB_Pin|FCCTXC_Pin 
                          |FCCTXD_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : INTACC_Pin INTGYR_Pin INTMAG_Pin */
  GPIO_InitStruct.Pin = INTACC_Pin|INTGYR_Pin|INTMAG_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : BARNRST_Pin */
  GPIO_InitStruct.Pin = BARNRST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(BARNRST_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : INTBAR_Pin */
  GPIO_InitStruct.Pin = INTBAR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(INTBAR_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LEDA_Pin LEDB_Pin LEDC_Pin LEDD_Pin */
  GPIO_InitStruct.Pin = LEDA_Pin|LEDB_Pin|LEDC_Pin|LEDD_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
