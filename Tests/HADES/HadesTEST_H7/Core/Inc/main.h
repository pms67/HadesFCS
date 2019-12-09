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
#include "stm32h7xx_hal.h"

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
#define SPI1_CS_FLASH_Pin GPIO_PIN_4
#define SPI1_CS_FLASH_GPIO_Port GPIOA
#define LEDA_Pin GPIO_PIN_0
#define LEDA_GPIO_Port GPIOB
#define LEDB_Pin GPIO_PIN_1
#define LEDB_GPIO_Port GPIOB
#define LEDC_Pin GPIO_PIN_2
#define LEDC_GPIO_Port GPIOB
#define NAVCRXA_Pin GPIO_PIN_0
#define NAVCRXA_GPIO_Port GPIOD
#define NAVCRXB_Pin GPIO_PIN_1
#define NAVCRXB_GPIO_Port GPIOD
#define NAVCRXC_Pin GPIO_PIN_2
#define NAVCRXC_GPIO_Port GPIOD
#define NAVRXD_Pin GPIO_PIN_3
#define NAVRXD_GPIO_Port GPIOD
#define NAVCTXA_Pin GPIO_PIN_4
#define NAVCTXA_GPIO_Port GPIOD
#define NAVCTXB_Pin GPIO_PIN_5
#define NAVCTXB_GPIO_Port GPIOD
#define NAVCTXC_Pin GPIO_PIN_6
#define NAVCTXC_GPIO_Port GPIOD
#define NAVCTXD_Pin GPIO_PIN_7
#define NAVCTXD_GPIO_Port GPIOD
#define LEDD_Pin GPIO_PIN_3
#define LEDD_GPIO_Port GPIOB
#define LEDE_Pin GPIO_PIN_4
#define LEDE_GPIO_Port GPIOB
#define LEDF_Pin GPIO_PIN_5
#define LEDF_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
