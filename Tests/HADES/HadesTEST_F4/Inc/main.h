/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2019 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
#define INTBAR_Pin GPIO_PIN_7
#define INTBAR_GPIO_Port GPIOA
#define FCCTXA_Pin GPIO_PIN_4
#define FCCTXA_GPIO_Port GPIOC
#define FCCTXB_Pin GPIO_PIN_5
#define FCCTXB_GPIO_Port GPIOC
#define BARNRST_Pin GPIO_PIN_0
#define BARNRST_GPIO_Port GPIOB
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

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
