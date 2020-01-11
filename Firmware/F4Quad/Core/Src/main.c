#include "main.h"
#include "cmsis_os.h"

#include <stdio.h>

#include "BMX055.h"
//#include "MPRLS_SPI.h"
#include "PCA9685.h"

#include "LowPassFilter.h"
#include "KalmanRollPitch.h"
#include "PIController.h"

#include "defines.h"

I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c3;

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;
SPI_HandleTypeDef hspi3;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;

osThreadId heartbeatTaskHandle;
osThreadId accelerometerTaskHandle;
osThreadId gyroscopeTaskHandle;
osThreadId magnetometerTaskHandle;
osThreadId barometerTaskHandle;
osThreadId rcTaskHandle;
osThreadId pwmTaskHandle;
osThreadId controllerTaskHandle;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2C3_Init(void);
static void MX_SPI1_Init(void);
static void MX_SPI2_Init(void);
static void MX_SPI3_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART3_UART_Init(void);
void startHeartbeatTask(void const * argument);
void startAccelerometerTask(void const * argument);
void startGyroscopeTask(void const * argument);
void startMagnetometerTask(void const * argument);
void startBarometerTask(void const * argument);
void startRCTask(void const * argument);
void startPWMTask(void const * argument);
void startControllerTask(void const * argument);

/* Peripherals */
BMX055 imu;
//MPRLS bar;
PCA9685 pwm;

/* Filters */
LPFTwoPole lpfAcc[3];
LPFTwoPole lpfGyr[3];
LPFTwoPole lpfMag[3];
LPFTwoPole lpfRC[4];

/* State estimation */
KalmanRollPitch ekf;
float psi;
float psiMag;

/* Controllers */
PIController ctrlRoll;
PIController ctrlPitch;

/* PWM input */
volatile uint32_t pwmRisingEdgeStart[] = {0,0,0,0};
volatile uint32_t pwmOnPeriod[] = {0,0,0,0};
float rcThrottle, rcRoll, rcPitch, rcYaw;
uint8_t running = 0;

/* RC channel 1 interrupt */
void EXTI0_IRQHandler(void)
{
	if(GPIOA->IDR & RC1_Pin) { /* Rising edge */
		pwmRisingEdgeStart[0] = TIM5->CNT;
	} else { /* Falling edge */
		pwmOnPeriod[0] = (TIM5->CNT - pwmRisingEdgeStart[0]) / 2;
	}

	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}

/* RC channel 2 interrupt */
void EXTI1_IRQHandler(void)
{
	if (GPIOA->IDR & RC2_Pin) {
		pwmRisingEdgeStart[1] = TIM5->CNT;
	} else { /* Falling edge */
		pwmOnPeriod[1] = (TIM5->CNT - pwmRisingEdgeStart[1]) / 2;
	}

	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
}

/* RC channel 3 and 4 interrupts */
void EXTI9_5_IRQHandler(void)
{
	if (EXTI->PR & (1 << 6)) {

		if (GPIOB->IDR & RC3_Pin) {
		  pwmRisingEdgeStart[2] = TIM5->CNT;
		} else if ((GPIOB->IDR & RC3_Pin) == 0) {
		  pwmOnPeriod[2] = (TIM5->CNT - pwmRisingEdgeStart[2]) / 2;
		}
		HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_6);

	} else if (EXTI->PR & (1 << 7)) {

		if (GPIOB->IDR & RC4_Pin) {
		  pwmRisingEdgeStart[3] = TIM5->CNT;
		} else if ((GPIOB->IDR & RC4_Pin) == 0) {
		  pwmOnPeriod[3] = (TIM5->CNT - pwmRisingEdgeStart[3]) / 2;
		}
		HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_7);

	}
}

int main(void)
{
  HAL_Init();

  /* Initialise clocks */
  SystemClock_Config();

  /* Initialise hardware interfaces */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_I2C3_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  MX_SPI3_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();

  /* Enable TIM5 for elapsed microseconds count (reading PWM RC input) */
  __HAL_RCC_TIM5_CLK_ENABLE();
  TIM5->PSC = HAL_RCC_GetPCLK1Freq()/1000000 - 1;
  TIM5->CR1 = TIM_CR1_CEN;
  TIM5->ARR = 0xFFFFFFFF;
  TIM5->CNT = 0xFFFFFFFE; /* Dirty fix... PWM values are only correct once timer overruns once. */

  /* Create task handles */
  osThreadDef(heartbeatTask, startHeartbeatTask, osPriorityLow, 0, 128);
  heartbeatTaskHandle = osThreadCreate(osThread(heartbeatTask), NULL);

  osThreadDef(accelerometerTask, startAccelerometerTask, osPriorityRealtime, 0, 128);
  accelerometerTaskHandle = osThreadCreate(osThread(accelerometerTask), NULL);

  osThreadDef(gyroscopeTask, startGyroscopeTask, osPriorityRealtime, 0, 128);
  gyroscopeTaskHandle = osThreadCreate(osThread(gyroscopeTask), NULL);

  osThreadDef(magnetometerTask, startMagnetometerTask, osPriorityHigh, 0, 128);
  magnetometerTaskHandle = osThreadCreate(osThread(magnetometerTask), NULL);

  //osThreadDef(barometerTask, startBarometerTask, osPriorityHigh, 0, 128);
  //barometerTaskHandle = osThreadCreate(osThread(barometerTask), NULL);

  osThreadDef(rcTask, startRCTask, osPriorityNormal, 0, 128);
  rcTaskHandle = osThreadCreate(osThread(rcTask), NULL);

  osThreadDef(pwmTask, startPWMTask, osPriorityHigh, 0, 128);
  pwmTaskHandle = osThreadCreate(osThread(pwmTask), NULL);

  osThreadDef(controllerTask, startControllerTask, osPriorityHigh, 0, 128);
  controllerTaskHandle = osThreadCreate(osThread(controllerTask), NULL);

  /* Initialise peripherals */
  if (BMX055_Init(&imu, &hspi3, GPIOB, SPI3_CSACC_Pin, SPI3_CSGYR_Pin, SPI3_CSMAG_Pin) == 0) {
	  HAL_GPIO_WritePin(GPIOC, LED_C_Pin, GPIO_PIN_SET);
  }

  //MPRLS_Init(&bar, &hspi2, GPIOC, SPI2_CS_Pin, GPIOC, BAR_NRST_Pin, GPIOC, INT_BAR_Pin);

  /* Initialise PMW driver */
  PCA9685_Init(&pwm, &hi2c1, 50);

  PCA9685_SetMicros(&pwm, 0, 0);
  PCA9685_SetMicros(&pwm, 1, 0);
  PCA9685_SetMicros(&pwm, 2, 0);
  PCA9685_SetMicros(&pwm, 3, 0);

  HAL_Delay(1000);

  /* Arm motors */
  PCA9685_SetMicros(&pwm, 0, 500);
  PCA9685_SetMicros(&pwm, 1, 500);
  PCA9685_SetMicros(&pwm, 2, 500);
  PCA9685_SetMicros(&pwm, 3, 500);
  HAL_Delay(1000);

  running = 1;

  /* Initialise filters */
  int n;
  for (n = 0; n < 3; n++) {
	  LPFTwoPole_Init(&lpfAcc[n], LPF_TYPE_BESSEL, LPF_ACC_CUTOFF_HZ, 0.001f * SAMPLE_TIME_ACC_MS);
	  LPFTwoPole_Init(&lpfGyr[n], LPF_TYPE_BESSEL, LPF_GYR_CUTOFF_HZ, 0.001f * SAMPLE_TIME_GYR_MS);
	  LPFTwoPole_Init(&lpfMag[n], LPF_TYPE_BESSEL, LPF_MAG_CUTOFF_HZ, 0.001f * SAMPLE_TIME_MAG_MS);
  }

  for (n = 0; n < 4; n++) {
	  LPFTwoPole_Init(&lpfRC[n], LPF_TYPE_BESSEL, LPF_RC_CUTOFF_HZ, 0.001f * SAMPLE_TIME_RC_MS);
  }

  /* Initialise Kalman filter */
  float Q[] = {3.0f * EKF_N_GYR, 2.0f * EKF_N_GYR};
  float R[] = {EKF_N_ACC, EKF_N_ACC, EKF_N_ACC};
  KalmanRollPitch_Init(&ekf, EKF_P_INIT, Q, R);

  /* Initialise controllers */
  PI_Init(&ctrlRoll,  CTRL_ROLL_P, CTRL_ROLL_I, CTRL_ROLL_LIM_MIN, CTRL_ROLL_LIM_MAX);
  PI_Init(&ctrlPitch, CTRL_PITCH_P, CTRL_PITCH_I, CTRL_PITCH_LIM_MIN, CTRL_PITCH_LIM_MAX);




  osKernelStart();

  while (1)
  {

  }

}


/*
 *
 *
 *
 *
 * TASKS
 *
 *
 *
 *
 */

uint32_t readingCountGyr = 0;
float gyrAverage[] = {0.0f, 0.0f, 0.0f};

void startAccelerometerTask(void const * argument)
{
	for (;;) {
		/* Acquire measurements from sensor */
		BMX055_ReadAccelerometer(&imu);

		/* Correct scale and bias errors */
		imu.acc[0] = CALIB_ACC_SCALE_X * imu.acc[0] + CALIB_ACC_BIAS_X;
		imu.acc[1] = CALIB_ACC_SCALE_Y * imu.acc[1] + CALIB_ACC_BIAS_Y;
		imu.acc[2] = CALIB_ACC_SCALE_Z * imu.acc[2] + CALIB_ACC_BIAS_Z;

		/* Filter measurements */
		float accFilt[3];
		for (int n = 0; n < 3; n++) {
		  accFilt[n] = LPFTwoPole_Update(&lpfAcc[n], imu.acc[n]);
		}

		/* Update Kalman filter */
		KalmanRollPitch_Update(&ekf, accFilt, 0.001f * SAMPLE_TIME_ACC_MS);

		osDelay(SAMPLE_TIME_ACC_MS); /* Output data rate is 62.5 Hz */
	}
}


void startGyroscopeTask(void const * argument)
{
	for (;;) {
		/* Acquire measurements from sensor */
		BMX055_ReadGyroscope(&imu);

		/* Calculate mean value */
		readingCountGyr++;
		for (int n = 0; n < 3; n++) {
			gyrAverage[n] = (gyrAverage[n] * (readingCountGyr - 1) + imu.gyr[n]) / ((float) readingCountGyr);
		}

		/* Filter measurements */
		float gyrFilt[3];
		for (int n = 0; n < 3; n++) {
		  gyrFilt[n] = LPFTwoPole_Update(&lpfGyr[n], imu.gyr[n]);
		}

		/* Update Kalman filter */
		KalmanRollPitch_Predict(&ekf, gyrFilt, 0.001f * SAMPLE_TIME_GYR_MS);

		/* Update yaw complementary filter */
		psi = CF_ALPHA * psiMag + (1.0f - CF_ALPHA) * (psi + 0.001f * SAMPLE_TIME_GYR_MS * (sin(ekf.phi) * gyrFilt[1] + cos(ekf.phi) * gyrFilt[2]) / cos(ekf.theta));

		osDelay(SAMPLE_TIME_GYR_MS);
	}
}

void startMagnetometerTask(void const * argument)
{
	for (;;) {
		/* Acquire measurements from sensor */
		BMX055_ReadMagnetometer(&imu);

		/* Convert to unit vector */
		float inorm = 1.0f / sqrt(imu.mag[0] * imu.mag[0] + imu.mag[1] * imu.mag[1] + imu.mag[2] * imu.mag[2]);
		imu.mag[0] *= inorm;
		imu.mag[1] *= inorm;
		imu.mag[2] *= inorm;

		/* Filter measurements */
		float magFilt[3];
		for (int n = 0; n < 3; n++) {
		  magFilt[n] = LPFTwoPole_Update(&lpfMag[n], imu.mag[n]);
		}

		float sp = sin(ekf.phi);   float cp = cos(ekf.phi);
		float st = sin(ekf.theta); float ct = cos(ekf.theta);

		/* De-rotate readings to flat plane */
		float mx =  magFilt[0] * ct + magFilt[1] * st * sp + magFilt[2] * st * cp;
		float my = 					  magFilt[1]	  * cp - magFilt[2]		 * sp;
		//float mz = -magFilt[0] * st + magFilt[1] * ct * sp + magFilt[2] * ct * cp;

		/* Estimate yaw angle */
		psiMag = -atan2(my, mx);

		osDelay(SAMPLE_TIME_MAG_MS);
	}
}

/*
void startBarometerTask(void const * argument)
{
	uint8_t dataRequested = 0;
	for(;;) {
		if (dataRequested == 1) {
			if (MPRLS_DataReady(&bar) == 1) {
				MPRLS_ReadPressure(&bar);
				dataRequested = 0;
			}
		} else {
			MPRLS_RequestData(&bar);
			dataRequested = 1;
		}

		osDelay(SAMPLE_TIME_BAR_MS);
	}
}
*/

void startRCTask(void const * argument) {
	for(;;) {
		/* Filter radio control inputs */
		rcThrottle =  0.001f * LPFTwoPole_Update(&lpfRC[0], (float) pwmOnPeriod[3]) - 1.0f; /* Min:  0, Max: 1 */
		rcRoll	   =  0.002f * LPFTwoPole_Update(&lpfRC[1], (float) pwmOnPeriod[1]) - 3.0f; /* Min: -1, Max: 1 */
		rcPitch	   = -0.002f * LPFTwoPole_Update(&lpfRC[2], (float) pwmOnPeriod[2]) + 3.0f; /* Min: -1, Max: 1 */
		rcYaw      =  0.002f * LPFTwoPole_Update(&lpfRC[3], (float) pwmOnPeriod[0]) - 3.0f; /* Min: -1, Max: 1 */

		/* Apply deadband to roll, pitch, and yaw setpoints (to avoid jittering at zero angle setpoints) */
		if (rcRoll >= -RC_DEADBAND && rcRoll <= RC_DEADBAND) {
			rcRoll = 0.0f;
		}

		if (rcPitch >= -RC_DEADBAND && rcPitch <= RC_DEADBAND) {
			rcPitch = 0.0f;
		}

		if (rcYaw >= -RC_DEADBAND && rcPitch <= RC_DEADBAND) {
			rcYaw = 0.0f;
		}

		/* Constrain values */
		if (rcThrottle < 0.0f) {
			rcThrottle = 0.0f;
		} else if (rcThrottle > 1.0f) {
			rcThrottle = 1.0f;
		}

		if (rcRoll < -1.0f) {
			rcRoll = -1.0f;
		} else if (rcRoll > 1.0f) {
			rcRoll =  1.0f;
		}

		if (rcPitch < -1.0f) {
			rcPitch = -1.0f;
		} else if (rcPitch > 1.0f) {
			rcPitch =  1.0f;
		}

		if (rcYaw < -1.0f) {
			rcYaw = -1.0f;
		} else if (rcYaw > 1.0f) {
			rcYaw =  1.0f;
		}

		/* Convert RC commands to setpoints */
		rcRoll  *= RC_TO_ROLL_ANGLE_SETPOINT;
		rcPitch *= RC_TO_PITCH_ANGLE_SETPOINT;
		rcYaw   *= RC_TO_YAW_RATE_SETPOINT;

		osDelay(SAMPLE_TIME_RC_MS);
	}
}

/* PWM driver: Control motor outputs */
void startPWMTask(void const * argument)
{
	for(;;) {
		/* Send microsecond settings to PWM driver */
		for (int n = 0; n < 4; n++) {
			PCA9685_SetMicros(&pwm, n, pwm.setting[n]);
		}

		osDelay(SAMPLE_TIME_PWM_MS);
	}
}

void startControllerTask(void const * argument)
{

	for (;;) {
		/* Disable motors if throttle is below threshold */
		if (rcThrottle < 0.1f) {
			running = 0;
		} else {
			running = 1;
		}

		/* Calculate throttle PWM value */
		int16_t throttleOutput = (uint16_t) (PWM_THR_LIMIT * rcThrottle);
		if (throttleOutput > PWM_THR_LIMIT) {
			throttleOutput = PWM_THR_LIMIT;
		} else if (throttleOutput < 0) {
			throttleOutput = 0;
		}

		/* Angle controllers (Desired angle to angular rate) */
		float rollRateSetpoint  = PI_Update(&ctrlRoll,  rcRoll,  ekf.phi,   0.001f * SAMPLE_TIME_CTRL_MS);
		float pitchRateSetpoint = PI_Update(&ctrlPitch, rcPitch, ekf.theta, 0.001f * SAMPLE_TIME_CTRL_MS);

		/* Rate controllers (Desired angular rate to force (...to PWM...)) */
		int16_t rollOutput  = (int16_t) (PWM_MICROS_PER_UNIT_FORCE * (CTRL_ROLL_D  * (rollRateSetpoint  - lpfGyr[0].out)));
		int16_t pitchOutput = (int16_t) (PWM_MICROS_PER_UNIT_FORCE * (CTRL_PITCH_D * (pitchRateSetpoint - lpfGyr[1].out)));

		/* Limit controller PWM values */
		if (rollOutput > 200) {
			rollOutput = 200;
		} else if (rollOutput < -200) {
			rollOutput = -200;
		}

		if (pitchOutput > 200) {
			pitchOutput = 200;
		} else if (pitchOutput < -200) {
			pitchOutput = -200;
		}

		if (running == 1) {
			/* Motor mixing (0 = Front Left, 1 = Front Right, 2 = Rear Right, 3 = Rear Left) */
			pwm.setting[0] = (uint16_t) (PWM_BASE_MICROS + throttleOutput + rollOutput + pitchOutput);
			pwm.setting[1] = (uint16_t) (PWM_BASE_MICROS + throttleOutput - rollOutput + pitchOutput);
			pwm.setting[2] = (uint16_t) (PWM_BASE_MICROS + throttleOutput - rollOutput - pitchOutput);
			pwm.setting[3] = (uint16_t) (PWM_BASE_MICROS + throttleOutput + rollOutput - pitchOutput);

			/* Limit PWM values */
			for (int n = 0; n < 4; n++) {
				if (pwm.setting[n] > PWM_MAX_MICROS) {
					pwm.setting[n] = PWM_MAX_MICROS;
				} else if (pwm.setting[n] < PWM_BASE_MICROS) {
					pwm.setting[n] = PWM_BASE_MICROS;
				}
			}
		} else {
			pwm.setting[0] = 500;
			pwm.setting[1] = 500;
			pwm.setting[2] = 500;
			pwm.setting[3] = 500;
		}

		osDelay(SAMPLE_TIME_CTRL_MS);
	}
}

/* Heartbeat: Flashes status LED to show system is running */
void startHeartbeatTask(void const * argument)
{
	for(;;) {
	  HAL_GPIO_TogglePin(GPIOC, LED_D_Pin);
	  osDelay(SAMPLE_TIME_LED_MS);
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
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
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
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief SPI3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI3_Init(void)
{

  /* USER CODE BEGIN SPI3_Init 0 */

  /* USER CODE END SPI3_Init 0 */

  /* USER CODE BEGIN SPI3_Init 1 */

  /* USER CODE END SPI3_Init 1 */
  /* SPI3 parameter configuration*/
  hspi3.Instance = SPI3;
  hspi3.Init.Mode = SPI_MODE_MASTER;
  hspi3.Init.Direction = SPI_DIRECTION_2LINES;
  hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi3.Init.NSS = SPI_NSS_SOFT;
  hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi3.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI3_Init 2 */

  /* USER CODE END SPI3_Init 2 */

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
  huart1.Init.BaudRate = 115200;
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
  HAL_GPIO_WritePin(GPIOC, LED_A_Pin|LED_B_Pin|LED_C_Pin|LED_D_Pin 
                          |FLASH_NWP_Pin|FLASH_NHOLD_Pin|SPI2_CS_Pin|BAR_NRST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, SPI3_CSACC_Pin|SPI3_CSGYR_Pin|SPI3_CSMAG_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : INT_ACC_Pin INT_GYR_Pin INT_MAG_Pin INT_BAR_Pin */
  GPIO_InitStruct.Pin = INT_ACC_Pin|INT_GYR_Pin|INT_MAG_Pin|INT_BAR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : LED_A_Pin LED_B_Pin LED_C_Pin LED_D_Pin 
                           FLASH_NWP_Pin FLASH_NHOLD_Pin SPI2_CS_Pin BAR_NRST_Pin */
  GPIO_InitStruct.Pin = LED_A_Pin|LED_B_Pin|LED_C_Pin|LED_D_Pin 
                          |FLASH_NWP_Pin|FLASH_NHOLD_Pin|SPI2_CS_Pin|BAR_NRST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : RC1_Pin RC2_Pin */
  GPIO_InitStruct.Pin = RC1_Pin|RC2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : SPI1_CS_Pin */
  GPIO_InitStruct.Pin = SPI1_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SPI1_CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : SPI3_CSACC_Pin SPI3_CSGYR_Pin SPI3_CSMAG_Pin */
  GPIO_InitStruct.Pin = SPI3_CSACC_Pin|SPI3_CSGYR_Pin|SPI3_CSMAG_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : RC3_Pin RC4_Pin */
  GPIO_InitStruct.Pin = RC3_Pin|RC4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6) {
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
