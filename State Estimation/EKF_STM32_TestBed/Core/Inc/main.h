/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
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
#define GPS_LNA_EN_Pin GPIO_PIN_13
#define GPS_LNA_EN_GPIO_Port GPIOC
#define GPS_NRST_Pin GPIO_PIN_14
#define GPS_NRST_GPIO_Port GPIOC
#define INT_ACC_Pin GPIO_PIN_4
#define INT_ACC_GPIO_Port GPIOA
#define INT_GYR_Pin GPIO_PIN_5
#define INT_GYR_GPIO_Port GPIOA
#define INT_MAG_Pin GPIO_PIN_6
#define INT_MAG_GPIO_Port GPIOA
#define BAR_RESET_Pin GPIO_PIN_7
#define BAR_RESET_GPIO_Port GPIOA
#define LED_A_Pin GPIO_PIN_12
#define LED_A_GPIO_Port GPIOB
#define LED_B_Pin GPIO_PIN_13
#define LED_B_GPIO_Port GPIOB
#define LED_C_Pin GPIO_PIN_14
#define LED_C_GPIO_Port GPIOB
#define LED_D_Pin GPIO_PIN_15
#define LED_D_GPIO_Port GPIOB
#define GPS_PPS_Pin GPIO_PIN_12
#define GPS_PPS_GPIO_Port GPIOC
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
