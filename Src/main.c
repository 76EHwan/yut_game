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

	char lcd_buf[20];

	// ==========================================================
	// 1. 칩 Wake-Up (SPEED 핀을 무조건 HIGH 상태로 만들기)
	// ==========================================================
	HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_1);
	__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1,
			__HAL_TIM_GET_AUTORELOAD(&htim8)); // 100% Duty
	HAL_Delay(100); // 칩이 완전히 깨어날 때까지 대기

	ST7735_LCD_Driver.FillRect(&st7735_pObj, 0, 0, ST7735Ctx.Width,
			ST7735Ctx.Height, BLACK);

	// ==========================================================
	// 2. I2C 자동 스캐너
	// ==========================================================
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
	HAL_Delay(1000);

	// ==========================================================
	// 3. MPET 전 초기화 및 레지스터 설정
	// ==========================================================
	MCF8316C_Clear_Faults(); // 부팅 시 발생한 일시적 Fault 찌꺼기 제거 (매우 중요)
	HAL_Delay(100);

	MCF8316C_Config_MPET();  // 레지스터 설정 덮어쓰기 (전압 리미트 무제한 등 적용)
	HAL_Delay(100);

	// 통신 생존 여부 재확인
	if (MCF8316C_ReadReg32(REG_DEVICE_CONFIG1) == 0) {
		sprintf(lcd_buf, "I2C READ FAIL!");
		LCD_ShowString(2, 40, ST7735Ctx.Width, 16, 16, (uint8_t*) lcd_buf);
		while (1)
			;
	}

	// ==========================================================
	// 4. MPET 측정 시작 및 대기
	// ==========================================================
	ST7735_LCD_Driver.FillRect(&st7735_pObj, 0, 0, ST7735Ctx.Width,
			ST7735Ctx.Height, BLACK);

	sprintf(lcd_buf, "MPET Start...");
	LCD_ShowString(2, 4, ST7735Ctx.Width, 16, 16, (uint8_t*) lcd_buf);

	MCF8316C_Start_MPET();

	// 칩이 MPET를 시작하고 상태를 변경할 시간을 줍니다.
	HAL_Delay(500);

	uint32_t wait_time_ms = 0;
	while (1) {
		// ALGO_STATUS 레지스터 읽기
		uint32_t current_status = MCF8316C_ReadReg32(REG_ALGO_STATUS);
		uint8_t motor_state = current_status & 0x0F; // 하위 4비트가 현재 모터 상태

		// 진행 중 결함이 발생했는지 실시간 확인
		MCF8316C_FaultStatus_t current_faults = { 0 };
		MCF8316C_Read_Faults(&current_faults);

		if (current_faults.controller_fault != 0
				|| current_faults.gate_driver_fault != 0) {
			break; // 폴트 발생 시 불필요한 대기를 멈추고 즉시 탈출
		}

		// 모터 상태가 0(MOTOR_IDLE)으로 돌아오면 측정이 모두 끝난 것입니다.
		if (motor_state == 0) {
			break;
		}

		// 무한 루프 방지를 위한 타임아웃 안전장치 (최대 30초 대기)
		if (wait_time_ms >= 30000) {
			break;
		}

		// 진행 경과 시간 출력 (예: "Wait 3.4 s")
		sprintf(lcd_buf, "Wait %lu.%lu s   ", wait_time_ms / 1000,
				(wait_time_ms % 1000) / 100);
		LCD_ShowString(2, 22, ST7735Ctx.Width, 16, 16, (uint8_t*) lcd_buf);

		HAL_Delay(100);
		wait_time_ms += 100;
	}

	// ==========================================================
	// 5. MPET 완료 후 상태 및 결함(Fault) 확인
	// ==========================================================
	MCF8316C_FaultStatus_t faults = { 0 };
	MCF8316C_Read_Faults(&faults);
	uint32_t algo_status = MCF8316C_ReadReg32(REG_ALGO_STATUS); // 0xE4

	ST7735_LCD_Driver.FillRect(&st7735_pObj, 0, 0, ST7735Ctx.Width,
			ST7735Ctx.Height, BLACK);

	if (faults.controller_fault != 0 || faults.gate_driver_fault != 0) {
		sprintf(lcd_buf, "MPET FAILED!");
	} else {
		sprintf(lcd_buf, "MPET SUCCESS!");
	}
	LCD_ShowString(2, 4, ST7735Ctx.Width, 16, 16, (uint8_t*) lcd_buf);

	sprintf(lcd_buf, "CT:%08lX", faults.controller_fault);
	LCD_ShowString(2, 22, ST7735Ctx.Width, 16, 16, (uint8_t*) lcd_buf);

	sprintf(lcd_buf, "ST:%08lX", algo_status);
	LCD_ShowString(2, 40, ST7735Ctx.Width, 16, 16, (uint8_t*) lcd_buf);

	HAL_Delay(3000); // 상태 코드를 눈으로 확인할 수 있도록 3초 대기

	// ==========================================================
	// 6. MPET 파라미터 결과 읽기 및 출력
	// ==========================================================
	MCF8316C_MotorParams_t mpet_params = { 0 };
	MCF8316C_Read_MPET_Results(&mpet_params);

	ST7735_LCD_Driver.FillRect(&st7735_pObj, 0, 0, ST7735Ctx.Width,
			ST7735Ctx.Height, BLACK);

	sprintf(lcd_buf, "--- RESULT ---");
	LCD_ShowString(2, 4, ST7735Ctx.Width, 16, 16, (uint8_t*) lcd_buf);

	// R, L, Ke
	sprintf(lcd_buf, "R%02X L%02X K%02X", mpet_params.resistance_hex,
			mpet_params.inductance_hex, mpet_params.bemf_const_hex);
	LCD_ShowString(2, 22, ST7735Ctx.Width, 16, 16, (uint8_t*) lcd_buf);

	// Current PI
	sprintf(lcd_buf, "C:%lu,%lu", mpet_params.curr_loop_kp,
			mpet_params.curr_loop_ki);
	LCD_ShowString(2, 40, ST7735Ctx.Width, 16, 16, (uint8_t*) lcd_buf);

	// Speed PI
	sprintf(lcd_buf, "S:%lu,%lu", mpet_params.spd_loop_kp,
			mpet_params.spd_loop_ki);
	LCD_ShowString(2, 58, ST7735Ctx.Width, 16, 16, (uint8_t*) lcd_buf);

	// ==========================================================
	// 7. 실 구동 준비 및 속도 지령 전송 (I2C)
	// ==========================================================
	MCF8316C_Clear_Faults();
	HAL_Delay(100);

	// [매우 중요] MPET 디버그 측정 플래그를 끄고 일반 운전 모드로 칩 설정을 재적용합니다.
	// (이 과정을 거쳐야 칩이 측정 모드에서 벗어나 정상 제어 루프로 진입합니다.)
	MCF8316C_WriteReg32(REG_ALGO_DEBUG2, 0x00000000); // 디버그/MPET 비트 전부 클리어
	HAL_Delay(100);

	// 클로즈드 루프 활성화 (CLOSED_LOOP 비트 켬)
	uint32_t algo_dbg1 = MCF8316C_ReadReg32(REG_ALGO_DEBUG1);
	algo_dbg1 &= ~CLOSED_LOOP_DIS; // Closed-loop Enable (0)
	MCF8316C_WriteReg32(REG_ALGO_DEBUG1, algo_dbg1);
	HAL_Delay(100);

	// 이제 I2C를 통해 안전하게 속도 지령 전송 (예: 25%)
	MCF8316C_Set_Speed(25.0f);

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
