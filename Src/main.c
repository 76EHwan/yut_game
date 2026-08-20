/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Balance cube - reaction wheel flip test
 *
 *  스핀업 → 유지 → 점프(제동 또는 반전) → 자이로로 전달 임펄스 측정
 *  FLIP_MODE 매크로 하나만 바꿔서 두 방식을 비교합니다.
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
#include <math.h>
/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

// ============================================================
// 실험 설정 — 여기만 바꿔가며 비교하세요
// ============================================================
#define FLIP_BRAKE          0   // 저측 단락 제동 (ΔL = Jω, 피크 토크 큼)
#define FLIP_REVERSE        1   // 역구동 반전   (ΔL = 2Jω, 영속도 구간 존재)

#define FLIP_MODE           FLIP_BRAKE   // ← 0 / 1 로 전환

#define SPINUP_START        30.0f   // 시동 지령 (핸드오프 임계 10%보다 위)
#define SPINUP_TARGET       95.0f   // 목표 지령
#define RAMP_STEP           5.0f    // 램프 1스텝
#define RAMP_PERIOD_MS      200     // 램프 주기
#define HANDOFF_WAIT_MS     2000    // 시동 후 클로즈드 루프 안착 대기
#define HOLD_MS             500    // 목표 도달 후 유지 시간
#define POST_FLIP_MS        3000    // 점프 후 자이로 관측 시간

#define LOOP_PERIOD_MS      20      // 메인 루프 주기 (점프 순간 분해능)
#define LCD_PERIOD_MS       200     // LCD 갱신 주기 (SPI가 느려서 분리)

#define SPIN_DIR            0       // 0 = CW, 1 = CCW

/* USER CODE END PD */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
typedef enum {
	ST_SPINUP = 0, ST_HANDOFF, ST_RAMP, ST_HOLD, ST_FLIP, ST_DONE
} FlipState_t;

static const char *state_name[] = { "SPINUP", "HANDOFF", "RAMP", "HOLD", "FLIP",
		"DONE" };
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void ExitRun0Mode(void) {
}

// ------------------------------------------------------------
// 브레이크 GPIO — 실장된 채널만 제어 (L쪽 미실장)
// ------------------------------------------------------------
static void Brake_Pin_Set(GPIO_PinState s) {
	HAL_GPIO_WritePin(MTR_R_BRAKE_GPIO_Port, MTR_R_BRAKE_Pin, s);
}

// ------------------------------------------------------------
// 자이로 Z축 각속도 [dps]
//
// TODO: lsm6ds3tr_c 드라이버의 실제 함수명으로 채우세요.
//       (프로젝트마다 API가 달라 임의로 호출하지 않았습니다)
//       예) int16_t raw[3]; LSM6DS3TR_C_GetGyro(raw);
//           return raw[2] * 0.070f;   // ±2000dps 설정 시 70 mdps/LSB
//
// 이 함수를 채우기 전까지는 항상 0을 반환하므로
// LCD의 PK 값은 0으로 표시됩니다. 나머지 동작에는 영향 없습니다.
// ------------------------------------------------------------
static float Gyro_ReadZ_dps(void) {
	return 0.0f;
}

// ------------------------------------------------------------
// 점프 동작 — 두 방식 중 하나 실행
// ------------------------------------------------------------
static void Flip_Execute(void) {
#if (FLIP_MODE == FLIP_BRAKE)
	// GPIO가 가장 빠르므로 먼저, I2C 경로는 보조로
	Brake_Pin_Set(GPIO_PIN_SET);
	MCF8316C_Set_Speed(0.0f);
	MCF8316C_Brake(1);
#else
	// DIR_CHANGE_MODE_REVERSE_DRIVE 이므로 재시동 없이 역구동으로 통과한다.
	// 속도 지령 크기는 그대로 두고 방향만 뒤집는다.
	MCF8316C_SetDir(1);
	MCF8316C_Set_Speed(SPINUP_TARGET);
#endif
}
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 */
int main(void) {

	/* MCU Configuration--------------------------------------------------------*/
	HAL_Init();
	SystemClock_Config();

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_SPI4_Init();
	MX_TIM1_Init();
	MX_I2C1_Init();
	MX_SPI2_Init();
	MX_TIM8_Init();
	MX_I2C4_Init();

	/* USER CODE BEGIN 2 */
	// 제동 해제 상태로 시작 — HIGH로 떠 있으면 모터가 아예 안 돈다
	Brake_Pin_Set(GPIO_PIN_RESET);

	LCD_Test();
	HAL_Delay(500);

	char lcd_buf[32];

	// ==========================================================
	// 1. 칩 Wake-Up (SPEED 핀 HIGH 유지)
	// ==========================================================
	HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_1);
	__HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1,
			__HAL_TIM_GET_AUTORELOAD(&htim8));
	HAL_Delay(100);

	ST7735_LCD_Driver.FillRect(&st7735_pObj, 0, 0, ST7735Ctx.Width,
			ST7735Ctx.Height, BLACK);

	// ==========================================================
	// 2. I2C 주소 확인 (기본 0x02 우선, 실패 시 스캔)
	// ==========================================================
	sprintf(lcd_buf, "I2C Scanning...");
	LCD_ShowString(2, 4, ST7735Ctx.Width, 16, 16, (uint8_t*) lcd_buf);

	uint8_t found_addr = MCF8316C_FindAddress();
	if (found_addr == 0) {
		sprintf(lcd_buf, "I2C Not Found!");
		LCD_ShowString(2, 22, ST7735Ctx.Width, 16, 16, (uint8_t*) lcd_buf);
		while (1)
			;
	}
	mcf_i2c_addr_8bit = found_addr;
	sprintf(lcd_buf, "Found: 0x%02X", found_addr);
	LCD_ShowString(2, 22, ST7735Ctx.Width, 16, 16, (uint8_t*) lcd_buf);
	HAL_Delay(500);

	// ==========================================================
	// 3. 레지스터 설정
	// ==========================================================
	sprintf(lcd_buf, "Configuring...");
	LCD_ShowString(2, 40, ST7735Ctx.Width, 16, 16, (uint8_t*) lcd_buf);

	MCF8316C_Clear_Faults();
	HAL_Delay(100);

	MCF8316C_Config_Manual();
	HAL_Delay(100);

	if (MCF8316C_ReadReg32(REG_DEVICE_CONFIG1) == 0) {
		sprintf(lcd_buf, "I2C READ FAIL!");
		LCD_ShowString(2, 58, ST7735Ctx.Width, 16, 16, (uint8_t*) lcd_buf);
		while (1)
			;
	}

	// 회전 방향을 명시적으로 고정 (반전 모드에서 기준점이 필요)
	MCF8316C_SetDir(SPIN_DIR);
	HAL_Delay(20);

	// ==========================================================
	// 4. 클로즈드 루프 활성화
	// ==========================================================
	uint32_t algo_dbg1 = MCF8316C_ReadReg32(REG_ALGO_DEBUG1);
	algo_dbg1 &= ~CLOSED_LOOP_DIS;
	MCF8316C_WriteReg32(REG_ALGO_DEBUG1, algo_dbg1);
	HAL_Delay(100);

	ST7735_LCD_Driver.FillRect(&st7735_pObj, 0, 0, ST7735Ctx.Width,
			ST7735Ctx.Height, BLACK);
	/* USER CODE END 2 */

	// ==========================================================
	// 5. 상태 머신
	// ==========================================================
	FlipState_t state = ST_SPINUP;
	float current_speed = 0.0f;

	uint32_t t_state = HAL_GetTick();   // 현재 상태 진입 시각
	uint32_t t_ramp = HAL_GetTick();    // 마지막 램프 스텝 시각
	uint32_t t_lcd = 0;                 // 마지막 LCD 갱신 시각
	uint32_t t_flip = 0;                // 점프 실행 시각

	uint32_t first_fault_c = 0;         // 처음 잡힌 컨트롤러 폴트
	uint32_t first_fault_g = 0;         // 처음 잡힌 게이트 드라이버 폴트
	float first_fault_spd = 0.0f;

	float gyro_peak = 0.0f;             // 점프 후 자이로 피크 [dps]

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		uint32_t now = HAL_GetTick();

		// ---------- 상태 전이 ----------
		switch (state) {

		case ST_SPINUP:
			// 고정 지령으로 시동을 건다 (여기서 지령을 흔들면 정렬이 리셋됨)
			current_speed = SPINUP_START;
			MCF8316C_Set_Speed(current_speed);
			state = ST_HANDOFF;
			t_state = now;
			break;

		case ST_HANDOFF:
			// 정렬(500ms) + 오픈루프 가속 + 클로즈드 루프 핸드오프 완료 대기
			if (now - t_state >= HANDOFF_WAIT_MS) {
				state = ST_RAMP;
				t_state = now;
				t_ramp = now;
			}
			break;

		case ST_RAMP:
			if (now - t_ramp >= RAMP_PERIOD_MS) {
				t_ramp = now;
				current_speed += RAMP_STEP;
				if (current_speed >= SPINUP_TARGET) {
					current_speed = SPINUP_TARGET;
					state = ST_HOLD;
					t_state = now;
				}
				MCF8316C_Set_Speed(current_speed);
			}
			break;

		case ST_HOLD:
			// 목표 속도에서 안정화될 시간을 준다
			if (now - t_state >= HOLD_MS) {
				Flip_Execute();
				t_flip = now;
				state = ST_FLIP;
				t_state = now;
			}
			break;

		case ST_FLIP:
			// 점프 직후 구간 — 자이로 피크를 잡는다
			{
				float w = Gyro_ReadZ_dps();
				if (fabsf(w) > fabsf(gyro_peak))
					gyro_peak = w;
			}
			if (now - t_state >= POST_FLIP_MS) {
				state = ST_DONE;
				t_state = now;
			}
			break;

		case ST_DONE:
		default:
			// 아무것도 하지 않고 결과만 표시한다
			break;
		}

		// ---------- 폴트 감시 (매 루프) ----------
		MCF8316C_FaultStatus_t faults = { 0 };
		MCF8316C_Read_Faults(&faults);

		if (first_fault_c == 0 && faults.controller_fault != 0) {
			first_fault_c = faults.controller_fault;
			first_fault_spd = current_speed;
		}
		if (first_fault_g == 0 && faults.gate_driver_fault != 0) {
			first_fault_g = faults.gate_driver_fault;
		}

		// ---------- LCD (모든 줄 y <= 58) ----------
		if (now - t_lcd >= LCD_PERIOD_MS || state == ST_DONE) {
			t_lcd = now;

			sprintf(lcd_buf, "%-7s %5.1f%%", state_name[state], current_speed);
			LCD_ShowString(2, 4, ST7735Ctx.Width, 16, 16, (uint8_t*) lcd_buf);

			sprintf(lcd_buf, "C %08lX", faults.controller_fault);
			LCD_ShowString(2, 22, ST7735Ctx.Width, 16, 16, (uint8_t*) lcd_buf);

			sprintf(lcd_buf, "G %08lX", faults.gate_driver_fault);
			LCD_ShowString(2, 40, ST7735Ctx.Width, 16, 16, (uint8_t*) lcd_buf);

			if (state >= ST_FLIP) {
				// 점프 후: 자이로 피크와 점프 이후 경과 시간
				sprintf(lcd_buf, "PK%6.0f  %4lums", gyro_peak, now - t_flip);
			} else {
				// 점프 전: 처음 잡힌 폴트
				sprintf(lcd_buf, "1st %08lX@%2.0f", first_fault_c,
						first_fault_spd);
			}
			LCD_ShowString(2, 58, ST7735Ctx.Width, 16, 16, (uint8_t*) lcd_buf);
		}

		HAL_Delay(LOOP_PERIOD_MS);
		/* USER CODE END WHILE */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

	while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {
	}

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
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	__disable_irq();
	while (1) {
	}
	/* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif /* USE_FULL_ASSERT */
