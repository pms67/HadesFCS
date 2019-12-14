/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define GPSLNAEN_Pin GPIO_PIN_13
#define GPSLNAEN_GPIO_Port GPIOC
#define GPSNRST_Pin GPIO_PIN_14
#define GPSNRST_GPIO_Port GPIOC
#define FCCRXA_Pin GPIO_PIN_0
#define FCCRXA_GPIO_Port GPIOC
#define FCCRXB_Pin GPIO_PIN_1
#define FCCRXB_GPIO_Port GPIOC
#define FCCRXC_Pin GPIO_PIN_2
#define FCCRXC_GPIO_Port GPIOC
#define FCCRXD_Pin GPIO_PIN_3
#define FCCRXD_GPIO_Port GPIOC
#define INTACC_Pin GPIO_PIN_4
#define INTACC_GPIO_Port GPIOA
#define INTGYR_Pin GPIO_PIN_5
#define INTGYR_GPIO_Port GPIOA
#define INTMAG_Pin GPIO_PIN_6
#define INTMAG_GPIO_Port GPIOA
#define BARNRST_Pin GPIO_PIN_7
#define BARNRST_GPIO_Port GPIOA
#define FCCTXA_Pin GPIO_PIN_4
#define FCCTXA_GPIO_Port GPIOC
#define FCCTXB_Pin GPIO_PIN_5
#define FCCTXB_GPIO_Port GPIOC
#define INTBAR_Pin GPIO_PIN_0
#define INTBAR_GPIO_Port GPIOB
#define LEDA_Pin GPIO_PIN_12
#define LEDA_GPIO_Port GPIOB
#define LEDB_Pin GPIO_PIN_13
#define LEDB_GPIO_Port GPIOB
#define LEDC_Pin GPIO_PIN_14
#define LEDC_GPIO_Port GPIOB
#define LEDD_Pin GPIO_PIN_15
#define LEDD_GPIO_Port GPIOB
#define FCCTXC_Pin GPIO_PIN_6
#define FCCTXC_GPIO_Port GPIOC
#define FCCTXD_Pin GPIO_PIN_7
#define FCCTXD_GPIO_Port GPIOC
#define GPSPPS_Pin GPIO_PIN_12
#define GPSPPS_GPIO_Port GPIOC
/* USER CODE BEGIN Private defines */

void initPeripherals();
void pollSensors();
void pollGPS();

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
