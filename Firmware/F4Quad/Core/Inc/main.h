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
#define INT_ACC_Pin GPIO_PIN_13
#define INT_ACC_GPIO_Port GPIOC
#define INT_GYR_Pin GPIO_PIN_14
#define INT_GYR_GPIO_Port GPIOC
#define INT_MAG_Pin GPIO_PIN_15
#define INT_MAG_GPIO_Port GPIOC
#define LED_A_Pin GPIO_PIN_0
#define LED_A_GPIO_Port GPIOC
#define LED_B_Pin GPIO_PIN_1
#define LED_B_GPIO_Port GPIOC
#define LED_C_Pin GPIO_PIN_2
#define LED_C_GPIO_Port GPIOC
#define LED_D_Pin GPIO_PIN_3
#define LED_D_GPIO_Port GPIOC
#define RC1_Pin GPIO_PIN_0
#define RC1_GPIO_Port GPIOA
#define RC1_EXTI_IRQn EXTI0_IRQn
#define RC2_Pin GPIO_PIN_1
#define RC2_GPIO_Port GPIOA
#define RC2_EXTI_IRQn EXTI1_IRQn
#define SPI1_CS_Pin GPIO_PIN_4
#define SPI1_CS_GPIO_Port GPIOA
#define FLASH_NWP_Pin GPIO_PIN_4
#define FLASH_NWP_GPIO_Port GPIOC
#define FLASH_NHOLD_Pin GPIO_PIN_5
#define FLASH_NHOLD_GPIO_Port GPIOC
#define SPI2_CS_Pin GPIO_PIN_6
#define SPI2_CS_GPIO_Port GPIOC
#define INT_BAR_Pin GPIO_PIN_7
#define INT_BAR_GPIO_Port GPIOC
#define BAR_NRST_Pin GPIO_PIN_8
#define BAR_NRST_GPIO_Port GPIOC
#define SPI3_CSACC_Pin GPIO_PIN_3
#define SPI3_CSACC_GPIO_Port GPIOB
#define SPI3_CSGYR_Pin GPIO_PIN_4
#define SPI3_CSGYR_GPIO_Port GPIOB
#define SPI3_CSMAG_Pin GPIO_PIN_5
#define SPI3_CSMAG_GPIO_Port GPIOB
#define RC3_Pin GPIO_PIN_6
#define RC3_GPIO_Port GPIOB
#define RC3_EXTI_IRQn EXTI9_5_IRQn
#define RC4_Pin GPIO_PIN_7
#define RC4_GPIO_Port GPIOB
#define RC4_EXTI_IRQn EXTI9_5_IRQn
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
