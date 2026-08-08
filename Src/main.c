/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lcd.h"
#include "lsm6ds3tr_c.h"
#include "mcf8316c.h"
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void ExitRun0Mode(void) {
	// 링커 에러 우회를 위한 더미 함수
}

static void LED_Blink(uint32_t Hdelay, uint32_t Ldelay) {
	HAL_GPIO_WritePin(E3_GPIO_Port, E3_Pin, GPIO_PIN_SET);
	HAL_Delay(Hdelay - 1);
	HAL_GPIO_WritePin(E3_GPIO_Port, E3_Pin, GPIO_PIN_RESET);
	HAL_Delay(Ldelay - 1);
}
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
	/* USER CODE BEGIN 1 */
	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/
	HAL_Init();

	/* USER CODE BEGIN Init */
	/* USER CODE END Init */

	/* USER CODE BEGIN SysInit */
	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_SPI4_Init();
	MX_TIM1_Init();
	MX_I2C1_Init();
	MX_SPI2_Init();
	MX_TIM8_Init();
	MX_I2C4_Init();

	/* USER CODE BEGIN 2 */
	HAL_GPIO_WritePin(E3_GPIO_Port, E3_Pin, GPIO_PIN_SET);
	HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
	__HAL_TIM_SetCompare(&htim1,TIM_CHANNEL_2,10);
	LCD_Test();
	HAL_GPIO_WritePin(E3_GPIO_Port, E3_Pin, GPIO_PIN_RESET);

	ST7735_LCD_Driver.FillRect(&st7735_pObj, 0, 0, ST7735Ctx.Width, ST7735Ctx.Height, BLACK);

	char lcd_buf[32];

	/* ---------------------------------------------------- */
	/* 1. IMU (LSM6DS3TR-C) WHO_AM_I 체크                   */
	/* ---------------------------------------------------- */
	for (uint8_t i = 0; i < 100; i++) {
		uint8_t imu_id = LSM6DS3TR_C_Check_WhoAmI();
		if (imu_id == 0x6A) {
			sprintf(lcd_buf, "IMU ID: %02X [OK]", imu_id);
			break;
		} else {
			sprintf(lcd_buf, "IMU ID: %02X [FAIL]", imu_id);
		}
	}
	LCD_ShowString(4, 0, ST7735Ctx.Width, 16, 16, (uint8_t*) lcd_buf);
	HAL_Delay(1000);

	ST7735_LCD_Driver.FillRect(&st7735_pObj, 0, 0, ST7735Ctx.Width, ST7735Ctx.Height, BLACK);

	/* ---------------------------------------------------- */
	/* 2. MCF8316C 활성화 및 nFAULT 읽기                    */
	/* ---------------------------------------------------- */
	HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_2);
	__HAL_TIM_SetCompare(&htim8, TIM_CHANNEL_1, 9600-1);
	__HAL_TIM_SetCompare(&htim8, TIM_CHANNEL_2, 9600-1);
	HAL_Delay(10);

	ST7735_LCD_Driver.FillRect(&st7735_pObj, 0, 0, ST7735Ctx.Width, ST7735Ctx.Height, BLACK);
	uint8_t found_address = 0;

	if (HAL_I2C_IsDeviceReady(MCF_I2C, 0x00, 3, 100) == HAL_OK) {
		sprintf(lcd_buf, "Brick Found at 0x00!");
		LCD_ShowString(4, 10, ST7735Ctx.Width, 16, 16, (uint8_t*) lcd_buf);
		HAL_Delay(1000);
		MCF8316C_Emergency_Recovery();
	}

	for (uint8_t i = 1; i < INT8_MAX; i++) {
		sprintf(lcd_buf, "%03d", i);
		LCD_ShowString(4, 26, ST7735Ctx.Width, 16, 16, (uint8_t*) lcd_buf);
		HAL_Delay(10);
		if (HAL_I2C_IsDeviceReady(MCF_I2C, (uint16_t) (i << 1), 3, 10) == HAL_OK) {
			found_address = (i << 1);
			sprintf(lcd_buf, "Found I2C: 0x%02X", found_address);
			LCD_ShowString(4, 26, ST7735Ctx.Width, 16, 16, (uint8_t*) lcd_buf);
			break;
		}
	}
	if (found_address == 0) {
		LCD_ShowString(4, 26, ST7735Ctx.Width, 16, 16, (uint8_t*) "I2C NOT FOUND");
	}
	HAL_Delay(3000);
	ST7735_LCD_Driver.FillRect(&st7735_pObj, 0, 0, ST7735Ctx.Width, ST7735Ctx.Height, BLACK);

	GPIO_PinState nfault_state = HAL_GPIO_ReadPin(MTR_R_nFAULT_GPIO_Port, MTR_R_nFAULT_Pin);

	/* ---------------------------------------------------- */
	/* 3. MCF8316C I2C 연결 체크 및 Fault 레지스터 읽기     */
	/* ---------------------------------------------------- */
	uint8_t mcf_ready = MCF8316C_Check_Connection();

	uint32_t dev_cfg1 = MCF8316C_ReadReg32(0x0000A6);
	dev_cfg1 |= (1 << 2);
	MCF8316C_WriteAndVerifyReg32(0x0000A6, dev_cfg1);
	HAL_Delay(10);

	MCF8316C_FaultStatus_t mcf_faults = { 0, 0 };

	if (mcf_ready == 1) {
		sprintf(lcd_buf, "MCF I2C:[OK] nF:%d", nfault_state);
		LCD_ShowString(4, 32, ST7735Ctx.Width, 16, 16, (uint8_t*) lcd_buf);

		MCF8316C_Read_Faults(&mcf_faults);

		sprintf(lcd_buf, "G FLT:%08lX", mcf_faults.gate_driver_fault);
		LCD_ShowString(4, 48, ST7735Ctx.Width, 16, 16, (uint8_t*) lcd_buf);

		sprintf(lcd_buf, "C FLT:%08lX", mcf_faults.controller_fault);
		LCD_ShowString(4, 64, ST7735Ctx.Width, 16, 16, (uint8_t*) lcd_buf);

	} else {
		sprintf(lcd_buf, "MCF I2C:[FAIL] nF:%d", nfault_state);
		LCD_ShowString(4, 32, ST7735Ctx.Width, 16, 16, (uint8_t*) lcd_buf);
	}

	HAL_Delay(1000);
	LSM6DS3TR_C_Init();
	HAL_Delay(5000);

	if (mcf_ready == 1) {
		uint8_t nfault_before = MCF8316C_Is_NFAULT_Active();
		sprintf(lcd_buf, "nFAULT(pre): %s", nfault_before ? "ACTIVE" : "ok");
		LCD_ShowString(4, 4, ST7735Ctx.Width, 16, 16, (uint8_t*) lcd_buf);
		HAL_Delay(500);

		MCF8316C_Clear_Faults();
		HAL_Delay(50);

		uint8_t sys_ready = MCF8316C_Wait_SysEnable(1000);
		sprintf(lcd_buf, "SYS_ENABLE: %s", sys_ready ? "OK" : "TIMEOUT");
		LCD_ShowString(4, 20, ST7735Ctx.Width, 16, 16, (uint8_t*) lcd_buf);
		HAL_Delay(500);

		ST7735_LCD_Driver.FillRect(&st7735_pObj, 0, 0, ST7735Ctx.Width, ST7735Ctx.Height, BLACK);
		LCD_ShowString(4, 4, ST7735Ctx.Width, 16, 16, (uint8_t*) "Running MPET...");

		MCF8316C_Config_MPET();
		MCF8316C_Start_MPET();

		uint8_t nfault_during = 0;
		for (uint32_t t = 0; t < 10000; t += 200) {
		    if (MCF8316C_Is_NFAULT_Active()) {
		        nfault_during = 1;
		        MCF8316C_Read_Faults(&mcf_faults);

		        sprintf(lcd_buf, "nF@%lums G:%08lX", (unsigned long) t, mcf_faults.gate_driver_fault);
		        LCD_ShowString(4, 20, ST7735Ctx.Width, 16, 16, (uint8_t*) lcd_buf);
		        HAL_Delay(2000);

		        sprintf(lcd_buf, "C:%08lX", mcf_faults.controller_fault);
		        LCD_ShowString(4, 36, ST7735Ctx.Width, 16, 16, (uint8_t*) lcd_buf);
		        HAL_Delay(2000);
		        break;
		    }
		    HAL_Delay(200);
		}
		if (!nfault_during) HAL_Delay(10000);

		if (!nfault_during) {
		    MCF8316C_Save_To_EEPROM();
		    HAL_Delay(50);
		}

		MCF8316C_MotorParams_t motor_params;
		MCF8316C_Read_MPET_Results(&motor_params);
		MCF8316C_Read_Faults(&mcf_faults);

		ST7735_LCD_Driver.FillRect(&st7735_pObj, 0, 0, ST7735Ctx.Width, ST7735Ctx.Height, BLACK);
		sprintf(lcd_buf, "R:%02X L:%02X Ke:%02X", motor_params.resistance_hex,
				motor_params.inductance_hex, motor_params.bemf_const_hex);
		LCD_ShowString(4, 4, ST7735Ctx.Width, 16, 16, (uint8_t*) lcd_buf);

		sprintf(lcd_buf, "CKp:%u CKi:%u", motor_params.curr_loop_kp, motor_params.curr_loop_ki);
		LCD_ShowString(4, 24, ST7735Ctx.Width, 16, 16, (uint8_t*) lcd_buf);

		sprintf(lcd_buf, "SKp:%u SKi:%u", motor_params.spd_loop_kp, motor_params.spd_loop_ki);
		LCD_ShowString(4, 44, ST7735Ctx.Width, 16, 16, (uint8_t*) lcd_buf);

		sprintf(lcd_buf, "FLT:%08lX nF:%d", mcf_faults.controller_fault, nfault_during);
		LCD_ShowString(4, 64, ST7735Ctx.Width, 16, 16, (uint8_t*) lcd_buf);

		if (mcf_faults.controller_fault == 0) {
			LCD_ShowString(4, 84, ST7735Ctx.Width, 16, 16, (uint8_t*) "Motor Spinning!");
			MCF8316C_Set_Speed(15.0f);
		} else {
			LCD_ShowString(4, 84, ST7735Ctx.Width, 16, 16, (uint8_t*) "MPET FAILED!");
		}
	}

	HAL_Delay(10000);
	ST7735_LCD_Driver.FillRect(&st7735_pObj, 0, 0, ST7735Ctx.Width, ST7735Ctx.Height, BLACK);

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */
		/* USER CODE BEGIN 3 */
		float roll, pitch, yaw;
		LSM6DS3TR_C_Get_RPY(&roll, &pitch, &yaw);

		sprintf(lcd_buf, "R:%5.1f", roll);
		LCD_ShowString(4, 4, ST7735Ctx.Width, 16, 16, (uint8_t*) lcd_buf);

		sprintf(lcd_buf, "P:%5.1f", pitch);
		LCD_ShowString(4, 24, ST7735Ctx.Width, 16, 16, (uint8_t*) lcd_buf);

		sprintf(lcd_buf, "Y:%5.1f", yaw);
		LCD_ShowString(4, 44, ST7735Ctx.Width, 16, 16, (uint8_t*) lcd_buf);

		HAL_Delay(10);
	}
	/* USER CODE END 3 */
}

void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

	while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {
	}

	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 5;
	RCC_OscInitStruct.PLL.PLLN = 96;
	RCC_OscInitStruct.PLL.PLLP = 2;
	RCC_OscInitStruct.PLL.PLLQ = 2;
	RCC_OscInitStruct.PLL.PLLR = 2;
	RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
	RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
	RCC_OscInitStruct.PLL.PLLFRACN = 0;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_D3PCLK1
			| RCC_CLOCKTYPE_D1PCLK1;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV1;
	RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
		Error_Handler();
	}
}

void Error_Handler(void) {
	LED_Blink(500, 500);
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif /* USE_FULL_ASSERT */
