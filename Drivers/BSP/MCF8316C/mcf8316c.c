/*
 * mcf8316c.c
 *
 *  Created on: 2026. 7. 31.
 */

#include "mcf8316c.h"
#include "i2c.h"

void MCF8316C_Read_Faults(MCF8316C_FaultStatus_t *faults) {
	faults->gate_driver_fault = MCF8316C_ReadReg32(REG_GATE_DRIVER_FAULT_STATUS);
	faults->controller_fault = MCF8316C_ReadReg32(REG_CONTROLLER_FAULT_STATUS);
}

void MCF8316C_WriteReg32(uint32_t reg_addr, uint32_t data) {
	uint8_t tx_buffer[7];
	uint32_t ctrl_word = (0x00 << 23) | (0x01 << 20) | (reg_addr & 0xFFFF);

	tx_buffer[0] = (ctrl_word >> 16) & 0xFF;
	tx_buffer[1] = (ctrl_word >> 8) & 0xFF;
	tx_buffer[2] = ctrl_word & 0xFF;

	tx_buffer[3] = data & 0xFF;
	tx_buffer[4] = (data >> 8) & 0xFF;
	tx_buffer[5] = (data >> 16) & 0xFF;
	tx_buffer[6] = (data >> 24) & 0xFF;

	HAL_I2C_Master_Transmit(MCF_I2C, MCF8316C_I2C_ADDR_WRITE, tx_buffer, 7, 100);
}

uint32_t MCF8316C_ReadReg32(uint32_t reg_addr) {
	uint8_t ctrl_buffer[3];
	uint8_t rx_buffer[4] = { 0, 0, 0, 0 };
	uint32_t ctrl_word = (0x01 << 23) | (0x01 << 20) | (reg_addr & 0xFFFF);

	ctrl_buffer[0] = (ctrl_word >> 16) & 0xFF;
	ctrl_buffer[1] = (ctrl_word >> 8) & 0xFF;
	ctrl_buffer[2] = ctrl_word & 0xFF;

	HAL_I2C_Master_Transmit(MCF_I2C, MCF8316C_I2C_ADDR_WRITE, ctrl_buffer, 3, 100);
	HAL_I2C_Master_Receive(MCF_I2C, MCF8316C_I2C_ADDR_READ, rx_buffer, 4, 100);

	return ((uint32_t) rx_buffer[3] << 24) | ((uint32_t) rx_buffer[2] << 16)
			| ((uint32_t) rx_buffer[1] << 8) | rx_buffer[0];
}

void MCF8316C_Config_MPET(void) {
    uint32_t reg_val;

    // 1. FAULT_CONFIG1 - 2212 모터에 맞춰 ILIMIT 5.0A 유지
    reg_val = MCF8316C_ReadReg32(REG_FAULT_CONFIG1);
    reg_val &= ~(0x0F << 27);
    reg_val |=  (0x0B << 27);
    MCF8316C_WriteAndVerifyReg32(REG_FAULT_CONFIG1, reg_val);

    // 2. MOTOR_STARTUP1 - MTR_STARTUP 방식을 Align (0x00)으로 설정
    reg_val = MCF8316C_ReadReg32(REG_MOTOR_STARTUP1);
    reg_val &= ~(0x1F << 9);
    reg_val |=  (0x05 << 9);
    reg_val &= ~(0x03 << 29);
    reg_val |=  (0x00 << 29);          // MTR_STARTUP = Align
    reg_val &= ~(0x0F << 21);
    reg_val |=  (0x0B << 21);
    reg_val &= ~(0x0F << 25);
    reg_val |=  (0x06 << 25);
    MCF8316C_WriteAndVerifyReg32(REG_MOTOR_STARTUP1, reg_val);

    // 3. MOTOR_STARTUP2 - 개방 루프(Open Loop) 전류 제한 1.5A
    reg_val = MCF8316C_ReadReg32(REG_MOTOR_STARTUP2);
    reg_val &= ~(0x0F << 27);
    reg_val |=  (0x04 << 27);
    reg_val &= ~(0x0F << 23);
    reg_val |=  (0x08 << 23);
    reg_val &= ~(0x0F << 19);
    reg_val |=  (0x06 << 19);
    MCF8316C_WriteAndVerifyReg32(REG_MOTOR_STARTUP2, reg_val);

    // 4. CLOSED_LOOP2 - ★ MPET IPD 단계를 건너뛰기 위해 R, L 값 수동 기록 ★
    // 데이터시트 기준: R = 0.1옴 (0x37), L = 0.02mH (0x0F)
    reg_val = MCF8316C_ReadReg32(REG_CLOSED_LOOP2);
    reg_val &= ~0xFFFF; // 하위 16비트(MOTOR_RES, MOTOR_IND) 초기화
    reg_val |= (0x37 << 8) | 0x0F;
    MCF8316C_WriteAndVerifyReg32(REG_CLOSED_LOOP2, reg_val);
}




void MCF8316C_Start_MPET(void) {
	uint32_t mpet_enable_cmd = 0x0000003F;
	MCF8316C_WriteReg32(REG_ALGO_DEBUG2, mpet_enable_cmd);
}

void MCF8316C_Read_MPET_Results(MCF8316C_MotorParams_t *params) {
    uint32_t mtr_params = MCF8316C_ReadReg32(REG_MTR_PARAMS);
    uint32_t current_pi = MCF8316C_ReadReg32(REG_CURRENT_PI);
    uint32_t speed_pi   = MCF8316C_ReadReg32(REG_SPEED_PI);

    params->resistance_hex = (mtr_params >> 24) & 0xFF;
    params->bemf_const_hex = (mtr_params >> 16) & 0xFF;
    params->inductance_hex = (mtr_params >> 8)  & 0xFF;

    params->curr_loop_kp = current_pi & 0x3FF;
    params->curr_loop_ki = (current_pi >> 16) & 0x3FF;
    params->spd_loop_kp  = speed_pi & 0x3FF;
    params->spd_loop_ki  = (speed_pi >> 16) & 0x3FF;
}

uint8_t MCF8316C_Check_Connection(void) {
	if (HAL_I2C_IsDeviceReady(MCF_I2C, MCF8316C_I2C_ADDR_WRITE, 3, 100) == HAL_OK) {
		return 1;
	}
	return 0;
}

void MCF8316C_Save_To_EEPROM(void) {
	MCF8316C_WriteReg32(REG_EEPROM_CTRL, 0x8A500000);
	HAL_Delay(800);
}

void MCF8316C_Restore_Defaults(void) {
	MCF8316C_WriteReg32(0x000080, 0x00000000);
	MCF8316C_WriteReg32(0x000082, 0x00000000);
	MCF8316C_WriteReg32(0x000084, 0x00000000);
	MCF8316C_WriteReg32(0x000086, 0x00000000);
	MCF8316C_WriteReg32(0x000088, 0x00000000);
	MCF8316C_WriteReg32(0x00008A, 0x00000000);
	MCF8316C_WriteReg32(0x00008C, 0x00000000);
	MCF8316C_WriteReg32(0x00008E, 0x00000000);
	MCF8316C_WriteReg32(0x000090, 0x00000000);
	MCF8316C_WriteReg32(0x000092, 0x00000000);
	MCF8316C_WriteReg32(0x000094, 0x00000000);
	MCF8316C_WriteReg32(0x000096, 0x00000000);
	MCF8316C_WriteReg32(0x000098, 0x00000000);
	MCF8316C_WriteReg32(0x00009A, 0x00000000);
	MCF8316C_WriteReg32(0x00009C, 0x00000000);
	MCF8316C_WriteReg32(0x00009E, 0x00000000);
	MCF8316C_WriteReg32(0x0000A0, 0x00000000);
	MCF8316C_WriteReg32(0x0000A2, 0x00000000);
	MCF8316C_WriteReg32(0x0000A4, 0x00000000);
	MCF8316C_WriteReg32(0x0000A6, 0x00100000);
	MCF8316C_WriteReg32(0x0000A8, 0x00000000);
	MCF8316C_WriteReg32(0x0000AA, 0x40000000);
	MCF8316C_WriteReg32(0x0000AC, 0x10228100);
	MCF8316C_WriteReg32(0x0000AE, 0x01100000);
}

void MCF8316C_Emergency_Recovery(void) {
	uint8_t bricked_addr = 0x00;
	uint8_t new_addr = MCF8316C_I2C_ADDR_WRITE;
	uint8_t tx_buffer[7];

	uint32_t reg_device_config1 = 0x0000A6;
	uint32_t data_device_config1 = 0x00100000;
	uint32_t ctrl_word_cfg = (0x00 << 23) | (0x01 << 20) | (reg_device_config1 & 0xFFFF);

	tx_buffer[0] = (ctrl_word_cfg >> 16) & 0xFF;
	tx_buffer[1] = (ctrl_word_cfg >> 8) & 0xFF;
	tx_buffer[2] = ctrl_word_cfg & 0xFF;
	tx_buffer[3] = data_device_config1 & 0xFF;
	tx_buffer[4] = (data_device_config1 >> 8) & 0xFF;
	tx_buffer[5] = (data_device_config1 >> 16) & 0xFF;
	tx_buffer[6] = (data_device_config1 >> 24) & 0xFF;

	HAL_I2C_Master_Transmit(MCF_I2C, bricked_addr, tx_buffer, 7, 100);
	HAL_Delay(50);

	if (HAL_I2C_IsDeviceReady(MCF_I2C, new_addr, 3, 100) == HAL_OK) {
		uint32_t eeprom_cmd = 0x8A500000;
		uint32_t reg_eeprom_ctrl = REG_EEPROM_CTRL;
		uint32_t ctrl_word_eeprom = (0x00 << 23) | (0x01 << 20) | (reg_eeprom_ctrl & 0xFFFF);

		tx_buffer[0] = (ctrl_word_eeprom >> 16) & 0xFF;
		tx_buffer[1] = (ctrl_word_eeprom >> 8) & 0xFF;
		tx_buffer[2] = ctrl_word_eeprom & 0xFF;
		tx_buffer[3] = eeprom_cmd & 0xFF;
		tx_buffer[4] = (eeprom_cmd >> 8) & 0xFF;
		tx_buffer[5] = (eeprom_cmd >> 16) & 0xFF;
		tx_buffer[6] = (eeprom_cmd >> 24) & 0xFF;

		HAL_I2C_Master_Transmit(MCF_I2C, new_addr, tx_buffer, 7, 100);
		HAL_Delay(1000);
	}
}

void MCF8316C_Clear_Faults(void) {
	MCF8316C_WriteReg32(REG_EEPROM_CTRL, 0x20000000);
	HAL_Delay(50);
}

void MCF8316C_Set_Speed(float speed_percent) {
	if (speed_percent > 100.0f) speed_percent = 100.0f;
	if (speed_percent < 0.0f) speed_percent = 0.0f;

	uint32_t speed_cmd = (uint32_t) ((speed_percent / 100.0f) * 32767.0f);
	uint32_t reg_val = (1 << 31) | (speed_cmd << 16);

	MCF8316C_WriteReg32(0x0000EC, reg_val);
}

uint8_t MCF8316C_Wait_SysEnable(uint32_t timeout_ms) {
	uint32_t elapsed = 0;
	while (elapsed < timeout_ms) {
		if (MCF8316C_ReadReg32(REG_ALGO_STATUS) & (1u << 2)) return 1;
		HAL_Delay(5);
		elapsed += 5;
	}
	return 0;
}

void MCF8316C_Set_SpeedMode_I2C(void) {
	uint32_t reg_val = MCF8316C_ReadReg32(REG_PIN_CONFIG);
	reg_val &= ~(0x03u << 0);
	reg_val |=  (0x02u << 0);
	MCF8316C_WriteReg32(REG_PIN_CONFIG, reg_val);
}

uint8_t MCF8316C_Is_NFAULT_Active(void) {
	return (HAL_GPIO_ReadPin(MTR_R_nFAULT_GPIO_Port, MTR_R_nFAULT_Pin) == GPIO_PIN_RESET) ? 1 : 0;
}

// mcf8316c.c 에 추가

// I2C 데이터 쓰기 후 검증하는 함수
uint8_t MCF8316C_WriteAndVerifyReg32(uint32_t reg_addr, uint32_t data) {
    uint8_t retry = 3;
    uint32_t read_data = 0;

    while(retry > 0) {
        MCF8316C_WriteReg32(reg_addr, data);
        HAL_Delay(10); // I2C 처리 대기

        read_data = MCF8316C_ReadReg32(reg_addr);
        if (read_data == data) {
            return 1; // 성공
        }
        retry--;
        HAL_Delay(10);
    }

    // 검증 실패 시 시리얼이나 LCD로 출력하도록 처리 가능
    // printf("I2C Write Fail - Addr: 0x%02X\n", reg_addr);
    return 0; // 실패
}
