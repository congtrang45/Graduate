/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
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
#define Buzzer_Pin GPIO_PIN_3
#define Buzzer_GPIO_Port GPIOE
#define AI0_Pin GPIO_PIN_2
#define AI0_GPIO_Port GPIOC
#define AI1_Pin GPIO_PIN_3
#define AI1_GPIO_Port GPIOC
#define DI13_Pin GPIO_PIN_0
#define DI13_GPIO_Port GPIOA
#define DI12_Pin GPIO_PIN_1
#define DI12_GPIO_Port GPIOA
#define DI11_Pin GPIO_PIN_2
#define DI11_GPIO_Port GPIOA
#define DE3_RE3_Pin GPIO_PIN_3
#define DE3_RE3_GPIO_Port GPIOA
#define AO1_Pin GPIO_PIN_4
#define AO1_GPIO_Port GPIOA
#define AO2_Pin GPIO_PIN_5
#define AO2_GPIO_Port GPIOA
#define DI10_Pin GPIO_PIN_6
#define DI10_GPIO_Port GPIOA
#define DI9_Pin GPIO_PIN_7
#define DI9_GPIO_Port GPIOA
#define DI8_Pin GPIO_PIN_4
#define DI8_GPIO_Port GPIOC
#define DI7_Pin GPIO_PIN_2
#define DI7_GPIO_Port GPIOB
#define DI6_Pin GPIO_PIN_7
#define DI6_GPIO_Port GPIOE
#define DI6_EXTI_IRQn EXTI9_5_IRQn
#define DI5_Pin GPIO_PIN_8
#define DI5_GPIO_Port GPIOE
#define DI4_Pin GPIO_PIN_9
#define DI4_GPIO_Port GPIOE
#define DI3_Pin GPIO_PIN_10
#define DI3_GPIO_Port GPIOE
#define DI2_Pin GPIO_PIN_11
#define DI2_GPIO_Port GPIOE
#define DI1_Pin GPIO_PIN_12
#define DI1_GPIO_Port GPIOE
#define DI1_EXTI_IRQn EXTI15_10_IRQn
#define DI0_Pin GPIO_PIN_13
#define DI0_GPIO_Port GPIOE
#define DE2_RE2_Pin GPIO_PIN_15
#define DE2_RE2_GPIO_Port GPIOE
#define TX2_Pin GPIO_PIN_10
#define TX2_GPIO_Port GPIOB
#define RX2_Pin GPIO_PIN_11
#define RX2_GPIO_Port GPIOB
#define DO11_Pin GPIO_PIN_12
#define DO11_GPIO_Port GPIOB
#define DO10_Pin GPIO_PIN_10
#define DO10_GPIO_Port GPIOD
#define DO9_Pin GPIO_PIN_11
#define DO9_GPIO_Port GPIOD
#define DO8_Pin GPIO_PIN_12
#define DO8_GPIO_Port GPIOD
#define DO7_Pin GPIO_PIN_13
#define DO7_GPIO_Port GPIOD
#define DO6_Pin GPIO_PIN_14
#define DO6_GPIO_Port GPIOD
#define DO5_Pin GPIO_PIN_15
#define DO5_GPIO_Port GPIOD
#define DO4_Pin GPIO_PIN_6
#define DO4_GPIO_Port GPIOC
#define DO3_Pin GPIO_PIN_7
#define DO3_GPIO_Port GPIOC
#define DO2_Pin GPIO_PIN_8
#define DO2_GPIO_Port GPIOC
#define DO1_Pin GPIO_PIN_9
#define DO1_GPIO_Port GPIOC
#define DO0_Pin GPIO_PIN_8
#define DO0_GPIO_Port GPIOA
#define TX1_Pin GPIO_PIN_9
#define TX1_GPIO_Port GPIOA
#define RX1_Pin GPIO_PIN_10
#define RX1_GPIO_Port GPIOA
#define TX3_Pin GPIO_PIN_10
#define TX3_GPIO_Port GPIOC
#define RX3_Pin GPIO_PIN_11
#define RX3_GPIO_Port GPIOC
#define LCDD4_Pin GPIO_PIN_0
#define LCDD4_GPIO_Port GPIOD
#define LCDD5_Pin GPIO_PIN_1
#define LCDD5_GPIO_Port GPIOD
#define LCDD6_Pin GPIO_PIN_2
#define LCDD6_GPIO_Port GPIOD
#define LCDD7_Pin GPIO_PIN_3
#define LCDD7_GPIO_Port GPIOD
#define LCDLED_Pin GPIO_PIN_4
#define LCDLED_GPIO_Port GPIOD
#define LCDEN_Pin GPIO_PIN_5
#define LCDEN_GPIO_Port GPIOD
#define LCDRW_Pin GPIO_PIN_6
#define LCDRW_GPIO_Port GPIOD
#define LCDRS_Pin GPIO_PIN_7
#define LCDRS_GPIO_Port GPIOD
#define BT_CLK_Pin GPIO_PIN_3
#define BT_CLK_GPIO_Port GPIOB
#define LED_DATA_Pin GPIO_PIN_5
#define LED_DATA_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
