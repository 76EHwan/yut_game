/*
 * lsm6ds3tr_c.c
 *
 *  Created on: 2026. 7. 31.
 *      Author: kth59
 */

#include "lsm6ds3tr_c.h"
#include "spi.h"
#include <math.h>

static void LSM6DS3TR_C_CS_Low(void) {
	HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_RESET);
}

static void LSM6DS3TR_C_CS_High(void) {
	HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_SET);
}

static void LSM6DS3TR_C_WriteReg(uint8_t reg, uint8_t data) {
	uint8_t tx_data[2] = { reg & 0x7F, data }; // MSB 0 for Write
	LSM6DS3TR_C_CS_Low();
	HAL_SPI_Transmit(&hspi2, tx_data, 2, 100);
	LSM6DS3TR_C_CS_High();
}

static uint8_t LSM6DS3TR_C_ReadReg(uint8_t reg) {
	uint8_t tx_data = reg | 0x80; // MSB 1 for Read
	uint8_t rx_data = 0;
	LSM6DS3TR_C_CS_Low();
	HAL_SPI_Transmit(&hspi2, &tx_data, 1, 100);
	HAL_SPI_Receive(&hspi2, &rx_data, 1, 100);
	LSM6DS3TR_C_CS_High();
	return rx_data;
}

uint8_t LSM6DS3TR_C_Init(void) {
	uint8_t who_am_i = LSM6DS3TR_C_ReadReg(LSM6DS3TR_C_WHO_AM_I);
	if (who_am_i != 0x6A)
		return 1; // 오류 발생

	// Accel 설정: 104Hz, +/- 2g
	LSM6DS3TR_C_WriteReg(LSM6DS3TR_C_CTRL1_XL, 0x40);
	// Gyro 설정: 104Hz, 2000 dps
	LSM6DS3TR_C_WriteReg(LSM6DS3TR_C_CTRL2_G, 0x4C);
	return 0; // 정상 완료
}

void LSM6DS3TR_C_Read_Accel(int16_t *x, int16_t *y, int16_t *z) {
	uint8_t data[6];
	uint8_t reg = LSM6DS3TR_C_OUTX_L_XL | 0x80;
	LSM6DS3TR_C_CS_Low();
	HAL_SPI_Transmit(&hspi2, &reg, 1, 100);
	HAL_SPI_Receive(&hspi2, data, 6, 100);
	LSM6DS3TR_C_CS_High();

	*x = (int16_t) (data[1] << 8 | data[0]);
	*y = (int16_t) (data[3] << 8 | data[2]);
	*z = (int16_t) (data[5] << 8 | data[4]);
}

// 자이로 데이터 읽기 함수 구현
void LSM6DS3TR_C_Read_Gyro(int16_t *x, int16_t *y, int16_t *z) {
	uint8_t data[6];
	uint8_t reg = LSM6DS3TR_C_OUTX_L_G | 0x80;
	LSM6DS3TR_C_CS_Low();
	HAL_SPI_Transmit(&hspi2, &reg, 1, 100);
	HAL_SPI_Receive(&hspi2, data, 6, 100);
	LSM6DS3TR_C_CS_High();

	*x = (int16_t) (data[1] << 8 | data[0]);
	*y = (int16_t) (data[3] << 8 | data[2]);
	*z = (int16_t) (data[5] << 8 | data[4]);
}

uint8_t LSM6DS3TR_C_Check_WhoAmI(void) {
	uint8_t tx_data = LSM6DS3TR_C_WHO_AM_I | 0x80;
	uint8_t rx_data = 0;

	HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi2, &tx_data, 1, 100);
	HAL_SPI_Receive(&hspi2, &rx_data, 1, 100);
	HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_SET);

	return rx_data;
}

// Roll, Pitch, Yaw 계산 함수
void LSM6DS3TR_C_Get_RPY(float *roll, float *pitch, float *yaw) {
	int16_t ax, ay, az;
	int16_t gx, gy, gz;
	static float current_yaw = 0.0f;

	// main.c에서 HAL_Delay(10)을 사용하므로 대략적인 루프 주기를 0.01초(10ms)로 가정
	float dt = 0.01f;

	// 가속도 및 자이로 원시 데이터 읽어오기
	LSM6DS3TR_C_Read_Accel(&ax, &ay, &az);
	LSM6DS3TR_C_Read_Gyro(&gx, &gy, &gz);

	// 가속도 기반 Roll, Pitch 계산// 가속도 기반 Pitch 계산 (-90도 ~ +90도 범위)
	*pitch = atan2((float) -ax, sqrt((float) (ay * ay + az * az)))
			* (180.0f / 3.14159265f);

	// 가속도 기반 Roll 계산 (-180도 ~ +180도 범위)
	*roll = atan2((float)ay, (float)-az) * (180.0f / 3.14159265f);

	// 자이로 설정이 2000 dps인 경우, 민감도는 약 70 mdps/LSB (0.07 dps)
	float gz_dps = (float) gz * 0.07f;

	// 시간에 따른 각속도 적분하여 Yaw 도출
	current_yaw += gz_dps * dt;
	*yaw = current_yaw;
}
