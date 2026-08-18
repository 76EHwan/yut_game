/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();
	SystemClock_Config();
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
	LCD_Test();
	HAL_Delay(500);

	ST7735_LCD_Driver.FillRect(&st7735_pObj, 0, 0, ST7735Ctx.Width,
			ST7735Ctx.Height, BLACK);
	char lcd_buf[20];

	// 1. I2C 자동 스캐너 (0~127 7-bit 주소 탐색)
	sprintf(lcd_buf, "I2C Scanning...");
	LCD_ShowString(2, 4, ST7735Ctx.Width, 16, 16, (uint8_t*) lcd_buf);

	uint8_t found_addr = 0;
	for (uint8_t i = 1; i < 128; i++) {
		uint8_t addr_8bit = i << 1;
		if (HAL_I2C_IsDeviceReady(MCF_I2C, addr_8bit, 3, 10) == HAL_OK) {
			found_addr = addr_8bit;
			break; // 가장 먼저 찾은 주소 저장
		}
	}

	// 통신 연결 확인 및 주소 전역 변수 동기화
	if (found_addr != 0) {
		mcf_i2c_addr_8bit = found_addr; // 스캔된 주소를 라이브러리에 전달
		sprintf(lcd_buf, "Found: 0x%02X", found_addr);
	} else {
		sprintf(lcd_buf, "I2C Not Found!");
		LCD_ShowString(2, 22, ST7735Ctx.Width, 16, 16, (uint8_t*) lcd_buf);
		while (1)
			; // 통신 불가 시 여기서 무한 대기 (하드웨어 결선 확인 필요)
	}
	LCD_ShowString(2, 22, ST7735Ctx.Width, 16, 16, (uint8_t*) lcd_buf);
	HAL_Delay(1500); // 사용자 확인용 대기

	// 2. 레지스터 설정 (이때 주소 끊김이 방지됨)
	MCF8316C_Config_MPET();
	HAL_Delay(100);

	// 3. 통신 생존 여부 재확인 (여기서 에러가 난다면 주소가 바뀐 것임)
	if (MCF8316C_ReadReg32(REG_DEVICE_CONFIG1) == 0) {
		sprintf(lcd_buf, "I2C READ FAIL!");
		LCD_ShowString(2, 40, ST7735Ctx.Width, 16, 16, (uint8_t*) lcd_buf);
		while (1)
			;
	}

	// 4. MPET 시작
	MCF8316C_FaultStatus_t faults = { 0 };
	MCF8316C_Read_Faults(&faults);

	// LCD 화면을 지우고 Fault 상태 출력
	ST7735_LCD_Driver.FillRect(&st7735_pObj, 0, 0, ST7735Ctx.Width,
			ST7735Ctx.Height, BLACK);

	sprintf(lcd_buf, "GD Flt:%08lX", faults.gate_driver_fault);
	LCD_ShowString(2, 22, ST7735Ctx.Width, 16, 16, (uint8_t*) lcd_buf);

	sprintf(lcd_buf, "CT Flt:%08lX", faults.controller_fault);
	LCD_ShowString(2, 40, ST7735Ctx.Width, 16, 16, (uint8_t*) lcd_buf);

	// 값을 편하게 확인하기 위해 5초 대기
	HAL_Delay(5000);
	LCD_Clear();

	sprintf(lcd_buf, "MPET Start...");
	LCD_ShowString(2, 40, ST7735Ctx.Width, 16, 16, (uint8_t*) lcd_buf);
	MCF8316C_Start_MPET();

	// 5. 측정 완료 대기 (약 10초)
	for (int i = 0; i < 10; i++) {
		sprintf(lcd_buf, "Wait %d sec  ", 10 - i);
		LCD_ShowString(2, 58, ST7735Ctx.Width, 16, 16, (uint8_t*) lcd_buf);
		HAL_Delay(1000);
	}
	LCD_Clear();

	// 6. 결과 읽기 및 출력
	MCF8316C_MotorParams_t mpet_params = { 0 };
	MCF8316C_Read_MPET_Results(&mpet_params);

	ST7735_LCD_Driver.FillRect(&st7735_pObj, 0, 0, ST7735Ctx.Width,
			ST7735Ctx.Height, BLACK);

	sprintf(lcd_buf, "MPET Complete");
	LCD_ShowString(2, 4, ST7735Ctx.Width, 16, 16, (uint8_t*) lcd_buf);

	// 출력 포맷 맞춤 (가로 최대 14자 이내)
	sprintf(lcd_buf, "R%02X L%02X K%02X", mpet_params.resistance_hex,
			mpet_params.inductance_hex, mpet_params.bemf_const_hex);
	LCD_ShowString(2, 22, ST7735Ctx.Width, 16, 16, (uint8_t*) lcd_buf);

	sprintf(lcd_buf, "C:%lu,%lu", mpet_params.curr_loop_kp,
			mpet_params.curr_loop_ki);
	LCD_ShowString(2, 40, ST7735Ctx.Width, 16, 16, (uint8_t*) lcd_buf);

	sprintf(lcd_buf, "S:%lu,%lu", mpet_params.spd_loop_kp,
			mpet_params.spd_loop_ki);
	LCD_ShowString(2, 58, ST7735Ctx.Width, 16, 16, (uint8_t*) lcd_buf);

	MCF8316C_Clear_Faults();
	HAL_Delay(100);

	HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_2);
	MCF8316C_Set_Speed(15.0f);

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
		HAL_Delay(100);
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Supply configuration update enable
	 */
	HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

	/** Configure the main internal regulator output voltage
	 */
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

	while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {
	}

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 5;
	RCC_OscInitStruct.PLL.PLLN = 192;
	RCC_OscInitStruct.PLL.PLLP = 2;
	RCC_OscInitStruct.PLL.PLLQ = 2;
	RCC_OscInitStruct.PLL.PLLR = 2;
	RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
	RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
	RCC_OscInitStruct.PLL.PLLFRACN = 0;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_D3PCLK1
			| RCC_CLOCKTYPE_D1PCLK1;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
	RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK) {
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
	/* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
