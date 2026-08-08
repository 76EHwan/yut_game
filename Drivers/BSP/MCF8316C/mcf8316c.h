/*
 * mcf8316c.h
 *
 *  Created on: 2026. 7. 31.
 *      Author: kth59
 */

#ifndef BSP_MCF8316C_MCF8316C_H_
#define BSP_MCF8316C_MCF8316C_H_

#include "main.h"

#define MCF8316C_I2C_ADDR_WRITE  0x02
#define MCF8316C_I2C_ADDR_READ   0x03

#define REG_CLOSED_LOOP2         		0x00008A
#define REG_CLOSED_LOOP3         		0x00008C
#define REG_INT_ALGO_1           		0x0000A0
#define REG_INT_ALGO_2           		0x0000A2
#define REG_PIN_CONFIG           		0x0000A4
#define REG_GATE_DRIVER_FAULT_STATUS 	0x0000E0
#define REG_CONTROLLER_FAULT_STATUS   	0x0000E2
#define REG_ALGO_STATUS          		0x0000E4
#define REG_MTR_PARAMS  		 		0x0000E6
#define REG_EEPROM_CTRL          		0x0000EA
#define REG_ALGO_DEBUG2          		0x0000EE

#define REG_MOTOR_STARTUP1       		0x000084
#define REG_MOTOR_STARTUP2       		0x000086
#define REG_CLOSED_LOOP1         		0x000088
#define REG_FAULT_CONFIG1        		0x000090
#define REG_FAULT_CONFIG2        		0x000092

#define REG_CURRENT_PI           		0x0000F0
#define REG_SPEED_PI             		0x0000F2

#define MCF_I2C		(&hi2c4)

typedef struct {
	uint32_t gate_driver_fault;
	uint32_t controller_fault;
} MCF8316C_FaultStatus_t;

typedef struct {
	uint8_t resistance_hex;
	uint8_t inductance_hex;
	uint8_t bemf_const_hex;
	uint16_t curr_loop_kp;
	uint16_t curr_loop_ki;
	uint16_t spd_loop_kp;
	uint16_t spd_loop_ki;
} MCF8316C_MotorParams_t;

void MCF8316C_Read_Faults(MCF8316C_FaultStatus_t *faults);
void MCF8316C_WriteReg32(uint32_t reg_addr, uint32_t data);
uint32_t MCF8316C_ReadReg32(uint32_t reg_addr);
void MCF8316C_Config_MPET(void);
void MCF8316C_Start_MPET(void);
void MCF8316C_Read_MPET_Results(MCF8316C_MotorParams_t *params);
uint8_t MCF8316C_Check_Connection(void);
void MCF8316C_Restore_Defaults(void);
void MCF8316C_Save_To_EEPROM(void);
void MCF8316C_Emergency_Recovery(void);
void MCF8316C_Clear_Faults(void);
void MCF8316C_Set_Speed(float speed_percent);

uint8_t  MCF8316C_Wait_SysEnable(uint32_t timeout_ms);
void     MCF8316C_Set_SpeedMode_I2C(void);
uint8_t  MCF8316C_Is_NFAULT_Active(void);

uint8_t MCF8316C_WriteAndVerifyReg32(uint32_t reg_addr, uint32_t data);

#endif /* BSP_MCF8316C_MCF8316C_H_ */
