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

#include "KalmanRollPitch.h"

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
osThreadId magReadHandle;
osThreadId gpsReadHandle;
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
void magReadTask(void const *argument);
void gpsReadTask(void const *argument);
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

/* Kalman filter */
KalmanRollPitch kal;
float heading;

/* Sample time definitions */
const uint32_t SAMPLE_TIME_ACC_MS = 10;
const uint32_t SAMPLE_TIME_GYR_MS = 5;

const uint32_t SAMPLE_TIME_MAG_MS = 10;

const uint32_t SAMPLE_TIME_BAR_MS = 10;

const uint32_t SAMPLE_TIME_TMP_MS = 320;

const uint32_t SAMPLE_TIME_EKF_MS = 10;

const uint32_t SAMPLE_TIME_GPS_MS = 1000;

const uint32_t SAMPLE_TIME_DBG_MS = 100;
const uint32_t SAMPLE_TIME_LED_MS = 1000;

/* UART-to-USB debug output */
void printDebug(char *buf) {
	HAL_UART_Transmit(&huart3, (uint8_t *) buf, strlen(buf), HAL_MAX_DELAY);
	HAL_UART_Transmit(&huart3, (uint8_t *) "\n", 1, HAL_MAX_DELAY);
}

float NavDataContainer[20];

FIRFilter firGyr[3];
FIRFilter firAcc[3];
FIRFilter firMag[3];
FIRFilter firBar;

int main(void)
{
  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();

  HAL_Delay(500);

  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_I2C3_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();

  initPeripherals();

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

  float kalQ[] = {3.0f * 0.000011941f, 2.0f * 0.000011941f};
  float kalR[] = {0.00024636441f, 0.00024636441f, 0.00034741232f};
  KalmanRollPitch_Init(&kal, 10.0f, kalQ, kalR);

  GPSNMEAParser_Init(&gpsData);

  /* Heartbeat LED task */
  osThreadDef(heartbeatLEDTask, heartbeatTask, osPriorityLow, 0, 128);
  heartbeatHandle = osThreadCreate(osThread(heartbeatLEDTask), NULL);

  /* Sensor tasks */
  osThreadDef(barometerReadTask, barometerReadTask, osPriorityAboveNormal, 0, 128);
  barometerReadHandle = osThreadCreate(osThread(barometerReadTask), NULL);

  osThreadDef(imuGyroReadTask, imuGyroReadTask, osPriorityRealtime, 0, 128);
  imuGyroReadHandle = osThreadCreate(osThread(imuGyroReadTask), NULL);

  osThreadDef(imuAccReadTask, imuAccReadTask, osPriorityRealtime, 0, 256);
  imuAccReadHandle = osThreadCreate(osThread(imuAccReadTask), NULL);

  osThreadDef(magReadTask, magReadTask, osPriorityRealtime, 0, 128);
  magReadHandle = osThreadCreate(osThread(magReadTask), NULL);

  osThreadDef(gpsReadTask, gpsReadTask, osPriorityNormal, 0, 128);
  gpsReadHandle = osThreadCreate(osThread(gpsReadTask), NULL);

  /* Serial debug output task */
  osThreadDef(debugSerialTask, debugSerialTask, osPriorityLow, 0, 256);
  debugSerialHandle = osThreadCreate(osThread(debugSerialTask), NULL);

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
    osDelay(SAMPLE_TIME_LED_MS);
  }

}

void imuGyroReadTask (void const *argument) {

	for (;;) {
		BMI088_ReadGyr(&imu);

		/* Filter measurements */
		FIRFilter_Update(&firGyr[0], imu.gyr[0]);
		FIRFilter_Update(&firGyr[1], imu.gyr[1]);
		FIRFilter_Update(&firGyr[2], imu.gyr[2]);

		osDelay(SAMPLE_TIME_GYR_MS);
	}

}

void imuAccReadTask (void const *argument) {

	for (;;) {
		BMI088_ReadAcc(&imu);

		/* Filter measurements */
		FIRFilter_Update(&firAcc[0], imu.acc[0]);
		FIRFilter_Update(&firAcc[1], imu.acc[1]);
		FIRFilter_Update(&firAcc[2], imu.acc[2]);

		/* Update kalman filter */
		float gyrFilt[] = {firGyr[0].out, firGyr[1].out, firGyr[2].out};
		float accFilt[] = {firAcc[0].out, firAcc[1].out, firAcc[2].out};
		KalmanRollPitch_Update(&kal, gyrFilt, accFilt, 0.0f, 0.01f);

		osDelay(SAMPLE_TIME_ACC_MS);
	}

}

void magReadTask (void const *argument) {

	for (;;) {
		IISMagnetometer_Read(&mag);

		/* Filter measurements */
		FIRFilter_Update(&firMag[0], mag.xyz[0]);
		FIRFilter_Update(&firMag[1], mag.xyz[1]);
		FIRFilter_Update(&firMag[2], mag.xyz[2]);

		/* Update heading estimate */
		float sp = sin(kal.phi);
		float cp = cos(kal.phi);
		heading = atan2(-mag.xyz[1] * cp + mag.xyz[2] * sp, mag.xyz[0] * cos(kal.theta) + (mag.xyz[1] * sp + mag.xyz[2] * cp) * sin(kal.theta));

		osDelay(SAMPLE_TIME_MAG_MS);
	}

}

void gpsReadTask (void const *argument) {

	for (;;) {
		char rxBuf[64];
		HAL_UART_Receive(&huart1, (uint8_t *) rxBuf, 64, 100);

		for (int n = 0; n < 64; n++) {
			GPSNMEAParser_Feed(&gpsData, rxBuf[n]);
		}

		osDelay(SAMPLE_TIME_GPS_MS);
	}

}

void barometerReadTask (void const *argument) {

	for (;;) {
		MPRLSBarometer_ReadPressure(&bar);

		/* Filter measurement */
		FIRFilter_Update(&firBar, bar.pressurePa);

		osDelay(SAMPLE_TIME_BAR_MS);
	}

}

void debugSerialTask (void const *argument) {

	for (;;) {

	    NavDataContainer[0] = firAcc[0].out;
		NavDataContainer[1] = firAcc[1].out;
		NavDataContainer[2] = firAcc[2].out;
		NavDataContainer[3] = firGyr[0].out;
		NavDataContainer[4] = firGyr[1].out;
		NavDataContainer[5] = firGyr[2].out;

		NavDataContainer[6] = mag.xyz[0]; //firMag[0].out;
		NavDataContainer[7] = mag.xyz[1]; //firMag[1].out;
		NavDataContainer[8] = mag.xyz[2]; //firMag[2].out;

		NavDataContainer[9]  = bar.pressurePa; //firBar.out;
		NavDataContainer[10] = 0.0f;

		NavDataContainer[11] = (float) gpsData.fixQuality;
		NavDataContainer[12] = gpsData.latitude_dec;
		NavDataContainer[13] = gpsData.longitude_dec;
		NavDataContainer[14] = gpsData.altitude_m;
		NavDataContainer[15] = gpsData.groundSpeed_mps;
		NavDataContainer[16] = gpsData.course_deg;

		NavDataContainer[17] = kal.phi   * 57.2957795131f;
		NavDataContainer[18] = kal.theta * 57.2957795131f;
		NavDataContainer[19] = heading   * 57.2957795131f;

		uint8_t UAVDataPacket[128];
		uint8_t UAVDataPacketLength = UAVDataLink_Pack(0, 0, sizeof(NavDataContainer), (const uint8_t *) NavDataContainer, UAVDataPacket);

		HAL_UART_Transmit(&huart3, UAVDataPacket, UAVDataPacketLength, HAL_MAX_DELAY);

		osDelay(SAMPLE_TIME_DBG_MS);
	}

}

void initPeripherals() {
	uint8_t status = 0;

	/* Initialise pressure sensor */
	uint8_t statBar = (MPRLSBarometer_Init(&bar, &hi2c1, BARNRST_GPIO_Port, BARNRST_Pin, INTBAR_GPIO_Port, INTBAR_Pin) == MPRLS_STATUS_POWERED);

	/* Initialise magnetometer */
	uint8_t statMag = IISMagnetometer_Init(&mag, &hi2c1, GPIOA, INTMAG_Pin);

	/* Initialise IMU */
	uint8_t statIMU = BMI088_Init(&imu, &hi2c1, GPIOA, INTACC_Pin, GPIOA, INTGYR_Pin);

	status = statBar + statMag + statIMU;

	/* Initialise temperature sensor */
	TMP100_Init(&tmp, &hi2c1);

	/* Initialise GPS receiver */
	UBloxGPS_Init(&gps, &huart1, GPIOC, GPSNRST_Pin, GPIOC, GPSPPS_Pin, GPIOC, GPSLNAEN_Pin);
	UBloxGPS_Reset(&gps);

	if (status < 3) {
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
  huart3.Init.BaudRate = 256000;
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
