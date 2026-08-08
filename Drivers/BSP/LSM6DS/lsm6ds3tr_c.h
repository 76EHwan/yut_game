/*
 * lsm6ds3tr_c.h
 *
 *  Created on: 2026. 7. 31.
 *      Author: kth59
 */

#ifndef BSP_LSM6DS_LSM6DS3TR_C_H_
#define BSP_LSM6DS_LSM6DS3TR_C_H_

#include "main.h"

#define LSM6DS3TR_C_WHO_AM_I   0x0F
#define LSM6DS3TR_C_CTRL1_XL   0x10
#define LSM6DS3TR_C_CTRL2_G    0x11
#define LSM6DS3TR_C_OUTX_L_G   0x22
#define LSM6DS3TR_C_OUTX_L_XL  0x28

uint8_t LSM6DS3TR_C_Init(void);
void LSM6DS3TR_C_Read_Accel(int16_t *x, int16_t *y, int16_t *z);
void LSM6DS3TR_C_Read_Gyro(int16_t *x, int16_t *y, int16_t *z);
uint8_t LSM6DS3TR_C_Check_WhoAmI(void);

// Roll, Pitch, Yaw 값을 계산하여 반환하는 함수 선언
void LSM6DS3TR_C_Get_RPY(float *roll, float *pitch, float *yaw);

#endif /* BSP_LSM6DS_LSM6DS3TR_C_H_ */
