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
#define E3_Pin GPIO_PIN_3
#define E3_GPIO_Port GPIOE
#define KEY_Pin GPIO_PIN_13
#define KEY_GPIO_Port GPIOC
#define LCD_CS_Pin GPIO_PIN_11
#define LCD_CS_GPIO_Port GPIOE
#define LCD_WR_RS_Pin GPIO_PIN_13
#define LCD_WR_RS_GPIO_Port GPIOE
#define SPI2_CS_Pin GPIO_PIN_12
#define SPI2_CS_GPIO_Port GPIOB
#define MTR_L_nFAULT_Pin GPIO_PIN_10
#define MTR_L_nFAULT_GPIO_Port GPIOD
#define MTR_L_DRVOFF_Pin GPIO_PIN_13
#define MTR_L_DRVOFF_GPIO_Port GPIOD
#define MTR_L_DIR_Pin GPIO_PIN_14
#define MTR_L_DIR_GPIO_Port GPIOD
#define MTR_L_BRAKE_Pin GPIO_PIN_15
#define MTR_L_BRAKE_GPIO_Port GPIOD
#define MTR_L_PWM_Pin GPIO_PIN_6
#define MTR_L_PWM_GPIO_Port GPIOC
#define MTR_R_PWM_Pin GPIO_PIN_7
#define MTR_R_PWM_GPIO_Port GPIOC
#define MTR_R_DRVOFF_Pin GPIO_PIN_8
#define MTR_R_DRVOFF_GPIO_Port GPIOA
#define MTR_R_DIR_Pin GPIO_PIN_9
#define MTR_R_DIR_GPIO_Port GPIOA
#define MTR_R_BRAKE_Pin GPIO_PIN_10
#define MTR_R_BRAKE_GPIO_Port GPIOA
#define MTR_R_nFAULT_Pin GPIO_PIN_15
#define MTR_R_nFAULT_GPIO_Port GPIOA
#define SWC_Pin GPIO_PIN_0
#define SWC_GPIO_Port GPIOD
#define SWU_Pin GPIO_PIN_1
#define SWU_GPIO_Port GPIOD
#define SWL_Pin GPIO_PIN_3
#define SWL_GPIO_Port GPIOD
#define SWR_Pin GPIO_PIN_4
#define SWR_GPIO_Port GPIOD
#define SWD_Pin GPIO_PIN_5
#define SWD_GPIO_Port GPIOD

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
