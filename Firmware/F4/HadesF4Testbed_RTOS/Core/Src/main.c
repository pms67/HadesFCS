#include "main.h"
#include "cmsis_os.h"

#include <stdio.h>
#include <string.h>

#include "MPRLS.h"
#include "IIS2MDC.h"
#include "BMI088.h"
#include "TMP100.h"
#include "UBLOX.h"

#include "FIR.h"
#include "FIRFilterArrays.h"

#include "GPSNMEAParser.h"
#include "UAVDataLink.h"


I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;
I2C_HandleTypeDef hi2c3;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;

osThreadId heartbeatHandle;
osThreadId barometerReadHandle;
osThreadId imuGyroReadHandle;
osThreadId imuAccReadHandle;
osThreadId debugSerialHandle;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2C2_Init(void);
static void MX_I2C3_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART3_UART_Init(void);

void heartbeatTask(void const * argument);
void barometerReadTask(void const *argument);
void imuGyroReadTask(void const *argument);
void imuAccReadTask(void const *argument);
void debugSerialTask(void const *argument);

/* Sensors */
MPRLSBarometer bar;
IISMagnetometer mag;
BMI088IMU imu;
TMP100 tmp;

/* GPS receiver */
UBloxGPS gps;

/* GPS parsed data container */
GPSData gpsData;

/* Sample time definitions */
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
	HAL_UART_Transmit(&huart3, (uint8_t *) "\n", 1, HAL_MAX_DELAY);
}

struct NavDataStruct {
	float acc[3];
	float gyr[3];
	float mag[3];
	float bar;
	float Va;
	uint8_t fix;
	float lat;
	float lon;
	float Vg;
	float roll;
	float pitch;
	float yaw;
} NavData;

FIRFilter firGyr[3];
FIRFilter firAcc[3];
FIRFilter firMag[3];
FIRFilter firBar;

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

  printDebug("NAVC started.\n");

  printDebug("Initialising sensors...\n");
  initPeripherals();

  printDebug("Loading filters...\n");

  FIRFilter_Init(&firGyr[0], firCoeffGyr, firGyrXBuf, FIRGYRN);
  FIRFilter_Init(&firGyr[1], firCoeffGyr, firGyrYBuf, FIRGYRN);
  FIRFilter_Init(&firGyr[2], firCoeffGyr, firGyrZBuf, FIRGYRN);

  FIRFilter_Init(&firAcc[0], firCoeffAcc, firAccXBuf, FIRACCN);
  FIRFilter_Init(&firAcc[1], firCoeffAcc, firAccYBuf, FIRACCN);
  FIRFilter_Init(&firAcc[2], firCoeffAcc, firAccZBuf, FIRACCN);

  FIRFilter_Init(&firMag[0], firCoeffMag, firMagXBuf, FIRMAGN);
  FIRFilter_Init(&firMag[1], firCoeffMag, firMagYBuf, FIRMAGN);
  FIRFilter_Init(&firMag[2], firCoeffMag, firMagZBuf, FIRMAGN);

  FIRFilter_Init(&firBar, firCoeffBar, firBarBuf, FIRBARN);

  osThreadDef(heartbeatLEDTask, heartbeatTask, osPriorityNormal, 0, 128);
  heartbeatHandle = osThreadCreate(osThread(heartbeatLEDTask), NULL);

  /* Sensor tasks */
  osThreadDef(barometerReadTask, barometerReadTask, osPriorityNormal, 0, 128);
  barometerReadHandle = osThreadCreate(osThread(barometerReadTask), NULL);

  osThreadDef(imuGyroReadTask, imuGyroReadTask, osPriorityNormal, 0, 128);
  imuGyroReadHandle = osThreadCreate(osThread(imuGyroReadTask), NULL);

  osThreadDef(imuAccReadTask, imuAccReadTask, osPriorityNormal, 0, 128);
  imuAccReadHandle = osThreadCreate(osThread(imuAccReadTask), NULL);

  /* Serial debug output task */
  osThreadDef(debugSerialTask, debugSerialTask, osPriorityAboveNormal, 0, 2048);
  debugSerialHandle = osThreadCreate(osThread(debugSerialTask), NULL);

  printDebug("Starting RTOS scheduler...\n");
  osKernelStart();
  
  while (1)
  {
  }

}

void heartbeatTask(void const * argument)
{

  for(;;)
  {
	HAL_GPIO_TogglePin(GPIOB, LEDA_Pin);
    osDelay(1000);
  }

}

void imuGyroReadTask (void const *argument) {

	for (;;) {
		BMI088_ReadGyr(&imu);

		/* Filter measurements */
		FIRFilter_Update(&firGyr[0], imu.gyr[0]);
		FIRFilter_Update(&firGyr[1], imu.gyr[1]);
		FIRFilter_Update(&firGyr[2], imu.gyr[2]);

		osDelay(1);
	}

}

void imuAccReadTask (void const *argument) {

	for (;;) {
		BMI088_ReadAcc(&imu);

		/* Filter measurements */
		FIRFilter_Update(&firAcc[0], imu.acc[0]);
		FIRFilter_Update(&firAcc[1], imu.acc[1]);
		FIRFilter_Update(&firAcc[2], imu.acc[2]);

		osDelay(5);
	}

}

void magReadTask (void const *argument) {

	for (;;) {
		IISMagnetomer_Read(&mag);

		/* Filter measurements */
		FIRFilter_Update(&firAcc[0], mag.xyz[0]);
		FIRFilter_Update(&firAcc[1], mag.xyz[1]);
		FIRFilter_Update(&firAcc[2], mag.xyz[2]);

		osDelay(5);
	}

}

void barometerReadTask (void const *argument) {

	for (;;) {
		MPRLSBarometer_ReadPressure(&bar);

		/* Filter measurement */
		FIRFilter_Update(&firBar, bar.pressurePa);

		osDelay(10);
	}

}

void debugSerialTask (void const *argument) {
	for (;;) {
		//printDebug((char *) SensorData.uiData);

		NavData.acc[0] = firAcc[0].out;
		NavData.acc[1] = firAcc[1].out;
		NavData.acc[2] = firAcc[2].out;
		NavData.gyr[0] = firGyr[0].out;
		NavData.gyr[1] = firGyr[1].out;
		NavData.gyr[2] = firGyr[2].out;
		NavData.mag[0] = firMag[0].out;
		NavData.mag[1] = firMag[1].out;
		NavData.mag[2] = firMag[2].out;
		NavData.bar = firBar.out;
		NavData.Va = 0.0f;
		NavData.fix = gpsData.fixQuality;
		NavData.lat = gpsData.latitude_dec;
		NavData.lon = gpsData.longitude_dec;
		NavData.Vg = gpsData.groundSpeed_mps;
		NavData.roll = 0.0f;
		NavData.pitch = 0.0f;
		NavData.yaw = 0.0f;

		uint8_t UAVDataPacket[128];
		uint8_t UAVDataPacketLength = UAVDataLink_Pack(0, 0, sizeof(NavData), (const uint8_t *) &NavData, UAVDataPacket);

		HAL_UART_Transmit(&huart3, UAVDataPacket, UAVDataPacketLength, HAL_MAX_DELAY);

		osDelay(1000);
	}
}

void initPeripherals() {
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



/**
  * @brief System Clock Configuration
  * @retval None
  */
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



/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

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
