/*
 * mcf8316c.c
 */

#include "mcf8316c.h"
#include "i2c.h"
#include "tim.h"

// @formatter:off
// ==========================================
// 7.1.1 ISD_CONFIG (0x80)
// ==========================================
static const uint32_t REG_ISD_CONFIG_DATA =
// ISD enable
//	ISD_DIS |
	ISD_EN |

// ISD brake enable
//	BRAKE_DIS |
	BRAKE_EN |

// ISD Hi-Z enable
//	HIZ_DIS |
	HIZ_EN |

// Reverse drive enable
//	RVS_DR_DIS |
	RVS_DR_EN |

// Resynchronization enable
//	RESYNC_DIS |
	RESYNC_EN |

// Minimum speed threshold to resynchronize to close loop
//	FW_DRV_RESYN_THR_5_PER |
	FW_DRV_RESYN_THR_10_PER |
//	FW_DRV_RESYN_THR_15_PER |
//	FW_DRV_RESYN_THR_20_PER |
//	FW_DRV_RESYN_THR_25_PER |
//	FW_DRV_RESYN_THR_30_PER |
//	FW_DRV_RESYN_THR_35_PER |
//	FW_DRV_RESYN_THR_40_PER |
//	FW_DRV_RESYN_THR_45_PER |
//	FW_DRV_RESYN_THR_50_PER |
//	FW_DRV_RESYN_THR_NOT_APPLI |

// Brake mode
//	BRK_MODE_HIGH_FET_ON |
	BRK_MODE_LOW_FET_ON |

// Brake configuration
//	BRK_CURR_EN |
	BRK_TIME_EN |

// Brake current threshold
//	BRK_CURR_THR_0P1_A |
//	BRK_CURR_THR_0P2_A |
//	BRK_CURR_THR_0P3_A |
	BRK_CURR_THR_0P5_A |
//	BRK_CURR_THR_1_A |
//	BRK_CURR_THR_2_A |
//	BRK_CURR_THR_NOT_APPLI |

// Brake time
//	BRK_TIME_10_MS |
//	BRK_TIME_50_MS |
	BRK_TIME_100_MS |
//	BRK_TIME_200_MS |
//	BRK_TIME_300_MS |
//	BRK_TIME_400_MS |
//	BRK_TIME_500_MS |
//	BRK_TIME_750_MS |
//	BRK_TIME_1_S |
//	BRK_TIME_2_S |
//	BRK_TIME_3_S |
//	BRK_TIME_4_S |
//	BRK_TIME_5_S |
//	BRK_TIME_7P5_S |
//	BRK_TIME_10_S |
//	BRK_TIME_15_S |

// Hi-Z time
//	HIZ_TIME_10_MS |
//	HIZ_TIME_50_MS |
	HIZ_TIME_100_MS |
//	HIZ_TIME_200_MS |
//	HIZ_TIME_300_MS |
//	HIZ_TIME_400_MS |
//	HIZ_TIME_500_MS |
//	HIZ_TIME_750_MS |
//	HIZ_TIME_1_S |
//	HIZ_TIME_2_S |
//	HIZ_TIME_3_S |
//	HIZ_TIME_4_S |
//	HIZ_TIME_5_S |
//	HIZ_TIME_7P5_S |
//	HIZ_TIME_10_S |
//	HIZ_TIME_15_S |

// BEMF threshold to detect if motor is stationary
	STAT_DETECT_THR_50_MV |
//	STAT_DETECT_THR_75_MV |
//	STAT_DETECT_THR_100_MV |
//	STAT_DETECT_THR_250_MV |
//	STAT_DETECT_THR_500_MV |
//	STAT_DETECT_THR_750_MV |
//	STAT_DETECT_THR_1000_MV |
//	STAT_DETECT_THR_1500_MV |

// Speed threshold used to transition to open loop during reverse drive
//	REV_DRV_HANDOFF_THR_2P5_PER |
	REV_DRV_HANDOFF_THR_5_PER |
//	REV_DRV_HANDOFF_THR_7P5_PER |
//	REV_DRV_HANDOFF_THR_10_PER |
//	REV_DRV_HANDOFF_THR_12P5_PER |
//	REV_DRV_HANDOFF_THR_15_PER |
//	REV_DRV_HANDOFF_THR_20_PER |
//	REV_DRV_HANDOFF_THR_25_PER |
//	REV_DRV_HANDOFF_THR_30_PER |
//	REV_DRV_HANDOFF_THR_40_PER |
//	REV_DRV_HANDOFF_THR_50_PER |
//	REV_DRV_HANDOFF_THR_NOT_APPLI |

// Open loop current limit during reverse drive
//	REV_DRV_OPEN_LOOP_CURRENT_1P5_A |
//	REV_DRV_OPEN_LOOP_CURRENT_2P5_A |
//	REV_DRV_OPEN_LOOP_CURRENT_3P5_A |
	REV_DRV_OPEN_LOOP_CURRENT_5_A;


// ==========================================
// 7.1.2 REV_DRIVE_CONFIG (0x82)
// ==========================================
static const uint32_t REG_REV_DRIVE_CONFIG_DATA =
// Open loop acceleration coefficient A1 during reverse drive
//	REV_DRV_OPEN_LOOP_ACCEL_A1_0P1_HZ_S |
//	REV_DRV_OPEN_LOOP_ACCEL_A1_0P5_HZ_S |
	REV_DRV_OPEN_LOOP_ACCEL_A1_1_HZ_S |
//	REV_DRV_OPEN_LOOP_ACCEL_A1_2P5_HZ_S |
//	REV_DRV_OPEN_LOOP_ACCEL_A1_5_HZ_S |
//	REV_DRV_OPEN_LOOP_ACCEL_A1_10_HZ_S |
//	REV_DRV_OPEN_LOOP_ACCEL_A1_25_HZ_S |
//	REV_DRV_OPEN_LOOP_ACCEL_A1_50_HZ_S |
//	REV_DRV_OPEN_LOOP_ACCEL_A1_75_HZ_S |
//	REV_DRV_OPEN_LOOP_ACCEL_A1_100_HZ_S |
//	REV_DRV_OPEN_LOOP_ACCEL_A1_250_HZ_S |
//	REV_DRV_OPEN_LOOP_ACCEL_A1_500_HZ_S |
//	REV_DRV_OPEN_LOOP_ACCEL_A1_750_HZ_S |
//	REV_DRV_OPEN_LOOP_ACCEL_A1_1000_HZ_S |
//	REV_DRV_OPEN_LOOP_ACCEL_A1_5000_HZ_S |
//	REV_DRV_OPEN_LOOP_ACCEL_A1_10000_HZ_S |

// Open loop acceleration coefficient A2 during reverse drive
	REV_DRV_OPEN_LOOP_ACCEL_A2_0P0_HZ_S2 |
//	REV_DRV_OPEN_LOOP_ACCEL_A2_0P5_HZ_S2 |
//	REV_DRV_OPEN_LOOP_ACCEL_A2_1_HZ_S2 |
//	REV_DRV_OPEN_LOOP_ACCEL_A2_2P5_HZ_S2 |
//	REV_DRV_OPEN_LOOP_ACCEL_A2_5_HZ_S2 |
//	REV_DRV_OPEN_LOOP_ACCEL_A2_10_HZ_S2 |
//	REV_DRV_OPEN_LOOP_ACCEL_A2_25_HZ_S2 |
//	REV_DRV_OPEN_LOOP_ACCEL_A2_50_HZ_S2 |
//	REV_DRV_OPEN_LOOP_ACCEL_A2_75_HZ_S2 |
//	REV_DRV_OPEN_LOOP_ACCEL_A2_100_HZ_S2 |
//	REV_DRV_OPEN_LOOP_ACCEL_A2_250_HZ_S2 |
//	REV_DRV_OPEN_LOOP_ACCEL_A2_500_HZ_S2 |
//	REV_DRV_OPEN_LOOP_ACCEL_A2_750_HZ_S2 |
//	REV_DRV_OPEN_LOOP_ACCEL_A2_1000_HZ_S2 |
//	REV_DRV_OPEN_LOOP_ACCEL_A2_5000_HZ_S2 |
//	REV_DRV_OPEN_LOOP_ACCEL_A2_10000_HZ_S2 |

// Bus current limit during active braking
//	ACTIVE_BRAKE_CURRENT_LIMIT_0P5_A |
//	ACTIVE_BRAKE_CURRENT_LIMIT_1_A |
	ACTIVE_BRAKE_CURRENT_LIMIT_2_A |
//	ACTIVE_BRAKE_CURRENT_LIMIT_3_A |
//	ACTIVE_BRAKE_CURRENT_LIMIT_4_A |
//	ACTIVE_BRAKE_CURRENT_LIMIT_5_A |
//	ACTIVE_BRAKE_CURRENT_LIMIT_6_A |
//	ACTIVE_BRAKE_CURRENT_LIMIT_7_A |

// Active braking loop Kp
	(0x0 << ACTIVE_BRAKE_KP_BASE) |

// Active braking loop Ki
	(0x0 << ACTIVE_BRAKE_KI_BASE);


// ==========================================
// 7.1.3 MOTOR_STARTUP1 (0x84)
// ==========================================
static const uint32_t REG_MOTOR_STARTUP1_DATA =
// Motor start-up method
	MTR_STARTUP_ALIGN |
//	MTR_STARTUP_DOUBLE_ALIGN |
//	MTR_STARTUP_IPD |
//	MTR_STARTUP_SLOW_FIRST_CYCLE |

// Align, slow first cycle and open loop current ramp rate
//	ALIGN_SLOW_RAMP_RATE_0P1_A_S |
//	ALIGN_SLOW_RAMP_RATE_1_A_S |
//	ALIGN_SLOW_RAMP_RATE_5_A_S |
	ALIGN_SLOW_RAMP_RATE_10_A_S |
//	ALIGN_SLOW_RAMP_RATE_15_A_S |
//	ALIGN_SLOW_RAMP_RATE_25_A_S |
//	ALIGN_SLOW_RAMP_RATE_50_A_S |
//	ALIGN_SLOW_RAMP_RATE_100_A_S |
//	ALIGN_SLOW_RAMP_RATE_150_A_S |
//	ALIGN_SLOW_RAMP_RATE_200_A_S |
//	ALIGN_SLOW_RAMP_RATE_250_A_S |
//	ALIGN_SLOW_RAMP_RATE_500_A_S |
//	ALIGN_SLOW_RAMP_RATE_1000_A_S |
//	ALIGN_SLOW_RAMP_RATE_2000_A_S |
//	ALIGN_SLOW_RAMP_RATE_5000_A_S |
//	ALIGN_SLOW_RAMP_RATE_NO_LIMIT |

// Align time
//	ALIGN_TIME_10_MS |
//	ALIGN_TIME_50_MS |
//	ALIGN_TIME_100_MS |
//	ALIGN_TIME_200_MS |
//	ALIGN_TIME_300_MS |
//	ALIGN_TIME_400_MS |
	ALIGN_TIME_500_MS |
//	ALIGN_TIME_750_MS |
//	ALIGN_TIME_1_S |
//	ALIGN_TIME_1P5_S |
//	ALIGN_TIME_2_S |
//	ALIGN_TIME_3_S |
//	ALIGN_TIME_4_S |
//	ALIGN_TIME_5_S |
//	ALIGN_TIME_7P5_S |
//	ALIGN_TIME_10_S |

// Align or slow first cycle current limit
//	ALIGN_OR_SLOW_CURRENT_ILIMIT_0P125_A |
//	ALIGN_OR_SLOW_CURRENT_ILIMIT_0P25_A |
//	ALIGN_OR_SLOW_CURRENT_ILIMIT_0P5_A |
//	ALIGN_OR_SLOW_CURRENT_ILIMIT_1_A |
//	ALIGN_OR_SLOW_CURRENT_ILIMIT_1P5_A |
	ALIGN_OR_SLOW_CURRENT_ILIMIT_2_A |
//	ALIGN_OR_SLOW_CURRENT_ILIMIT_2P5_A |
//	ALIGN_OR_SLOW_CURRENT_ILIMIT_3_A |
//	ALIGN_OR_SLOW_CURRENT_ILIMIT_3P5_A |
//	ALIGN_OR_SLOW_CURRENT_ILIMIT_4_A |
//	ALIGN_OR_SLOW_CURRENT_ILIMIT_4P5_A |
//	ALIGN_OR_SLOW_CURRENT_ILIMIT_5_A |
//	ALIGN_OR_SLOW_CURRENT_ILIMIT_5P5_A |
//	ALIGN_OR_SLOW_CURRENT_ILIMIT_6_A |
//	ALIGN_OR_SLOW_CURRENT_ILIMIT_7_A |
//	ALIGN_OR_SLOW_CURRENT_ILIMIT_8_A |

// IPD clock frequency
//	IPD_CLK_FREQ_50_HZ |
//	IPD_CLK_FREQ_100_HZ |
//	IPD_CLK_FREQ_250_HZ |
//	IPD_CLK_FREQ_500_HZ |
	IPD_CLK_FREQ_1000_HZ |
//	IPD_CLK_FREQ_2000_HZ |
//	IPD_CLK_FREQ_5000_HZ |
//	IPD_CLK_FREQ_10000_HZ |

// IPD current threshold
//	IPD_CURR_THR_0P25_A |
//	IPD_CURR_THR_0P5_A |
//	IPD_CURR_THR_0P75_A |
//	IPD_CURR_THR_1_A |
//	IPD_CURR_THR_1P25_A |
//	IPD_CURR_THR_1P5_A |
//	IPD_CURR_THR_2_A |
//	IPD_CURR_THR_2P5_A |
//	IPD_CURR_THR_3_A |
//	IPD_CURR_THR_3P667_A |
//	IPD_CURR_THR_4_A |
//	IPD_CURR_THR_4P667_A |
	IPD_CURR_THR_5_A |
//	IPD_CURR_THR_5P333_A |
//	IPD_CURR_THR_6_A |
//	IPD_CURR_THR_6P667_A |
//	IPD_CURR_THR_7P333_A |
//	IPD_CURR_THR_8_A |
//	IPD_CURR_THR_NOT_APPLI |

// IPD release mode
	IPD_RLS_MODE_BRAKE |
//	IPD_RLS_MODE_TRISTATE |

// IPD advance angle
	IPD_ADV_ANGLE_0_DEG |
//	IPD_ADV_ANGLE_30_DEG |
//	IPD_ADV_ANGLE_60_DEG |
//	IPD_ADV_ANGLE_90_DEG |

// Number of times IPD is executed
	IPD_REPEAT_1_TIME |
//	IPD_REPEAT_2_TIMES |
//	IPD_REPEAT_3_TIMES |
//	IPD_REPEAT_4_TIMES |

// Iq reference ramp down during transition from open loop to closed loop
//	IQ_RAMP_DOWN_DIS |
	IQ_RAMP_DOWN_EN |

// Enable active braking
//	ACTIVE_BRAKE_DIS |
	ACTIVE_BRAKE_EN |

// Choose between forward and reverse drive setting for reverse drive
	REV_DRV_CONFIG_FORWARD;
//	REV_DRV_CONFIG_REVERSE;


// ==========================================
// 7.1.4 MOTOR_STARTUP2 (0x86)
// ==========================================
static const uint32_t REG_MOTOR_STARTUP2_DATA =
// Open loop current limit
//	OL_ILIMIT_0P125_A |
//	OL_ILIMIT_0P25_A |
//	OL_ILIMIT_0P5_A |
//	OL_ILIMIT_1_A |
//	OL_ILIMIT_1P5_A |
//	OL_ILIMIT_2_A |
//	OL_ILIMIT_2P5_A |
//	OL_ILIMIT_3_A |
//	OL_ILIMIT_3P5_A |
	OL_ILIMIT_4_A |
//	OL_ILIMIT_4P5_A |
//	OL_ILIMIT_5_A |
//	OL_ILIMIT_5P5_A |
//	OL_ILIMIT_6_A |
//	OL_ILIMIT_7_A |
//	OL_ILIMIT_8_A |

// Open loop acceleration coefficient A1
//	OL_ACC_A1_0P1_HZ_S |
//	OL_ACC_A1_0P5_HZ_S |
//	OL_ACC_A1_1_HZ_S |
	OL_ACC_A1_2P5_HZ_S |
//	OL_ACC_A1_5_HZ_S |
//	OL_ACC_A1_10_HZ_S |
//	OL_ACC_A1_25_HZ_S |
//	OL_ACC_A1_50_HZ_S |
//	OL_ACC_A1_75_HZ_S |
//	OL_ACC_A1_100_HZ_S |
//	OL_ACC_A1_250_HZ_S |
//	OL_ACC_A1_500_HZ_S |
//	OL_ACC_A1_750_HZ_S |
//	OL_ACC_A1_1000_HZ_S |
//	OL_ACC_A1_5000_HZ_S |
//	OL_ACC_A1_10000_HZ_S |

// Open loop acceleration coefficient A2
	OL_ACC_A2_0P0_HZ_S2 |
//	OL_ACC_A2_0P5_HZ_S2 |
//	OL_ACC_A2_1_HZ_S2 |
//	OL_ACC_A2_2P5_HZ_S2 |
//	OL_ACC_A2_5_HZ_S2 |
//	OL_ACC_A2_10_HZ_S2 |
//	OL_ACC_A2_25_HZ_S2 |
//	OL_ACC_A2_50_HZ_S2 |
//	OL_ACC_A2_75_HZ_S2 |
//	OL_ACC_A2_100_HZ_S2 |
//	OL_ACC_A2_250_HZ_S2 |
//	OL_ACC_A2_500_HZ_S2 |
//	OL_ACC_A2_750_HZ_S2 |
//	OL_ACC_A2_1000_HZ_S2 |
//	OL_ACC_A2_5000_HZ_S2 |
//	OL_ACC_A2_10000_HZ_S2 |

// Auto handoff enable
//	AUTO_HANDOFF_DIS |
	AUTO_HANDOFF_EN |

// Open to closed loop handoff threshold (% of MAX_SPEED)
//	OPN_CL_HANDOFF_THR_1_PER |
//	OPN_CL_HANDOFF_THR_5_PER |
	OPN_CL_HANDOFF_THR_10_PER |
//	OPN_CL_HANDOFF_THR_20_PER |
//	OPN_CL_HANDOFF_THR_30_PER |
//	OPN_CL_HANDOFF_THR_40_PER |
//	OPN_CL_HANDOFF_THR_50_PER |

// Align angle
	ALIGN_ANGLE_0_DEG |
//	ALIGN_ANGLE_90_DEG |
//	ALIGN_ANGLE_180_DEG |
//	ALIGN_ANGLE_270_DEG |

// Frequency of first cycle in open loop start-up
//	SLOW_FIRST_CYCLE_FREQ_0P1_PER |
	SLOW_FIRST_CYCLE_FREQ_1_PER |
//	SLOW_FIRST_CYCLE_FREQ_10_PER |
//	SLOW_FIRST_CYCLE_FREQ_25_PER |

// First cycle frequency in open loop for align, double align and IPD
	FIRST_CYCLE_FREQ_SEL_0_HZ |
//	FIRST_CYCLE_FREQ_SEL_SLOW_FIRST_CYC_FREQ |

// Ramp rate for reducing difference between estimated theta and open loop theta
//	THETA_ERROR_RAMP_RATE_0P01_DEG_MS |
	THETA_ERROR_RAMP_RATE_0P1_DEG_MS;
//	THETA_ERROR_RAMP_RATE_1_DEG_MS |
//	THETA_ERROR_RAMP_RATE_2_DEG_MS;


// ==========================================
// 7.1.5 CLOSED_LOOP1 (0x88)
// ==========================================
static const uint32_t REG_CLOSED_LOOP1_DATA =
// Enable overmodulation
	OVERMODULATION_DIS |
//	OVERMODULATION_EN |

// Closed loop acceleration
//	CL_ACC_10 |
//	CL_ACC_100 |
	CL_ACC_500 |
//	CL_ACC_NO_LIMIT |

// Closed loop deceleration
//	CL_DEC_10 |
//	CL_DEC_100 |
	CL_DEC_500 |
//	CL_DEC_NO_LIMIT |

// PWM output frequency
//	PWM_FREQ_OUT_10_KHZ |
//	PWM_FREQ_OUT_20_KHZ |
	PWM_FREQ_OUT_25_KHZ |
//	PWM_FREQ_OUT_50_KHZ |

// PWM modulation
	PWM_MODE_CONTINUOUS |
//	PWM_MODE_DISCONTINUOUS |

// FG select
	FG_SEL_FG_IN_ISD |
//	FG_SEL_ONLY_CLOSED_LOOP |
//	FG_SEL_OPEN_LOOP_FIRST_TRY |

// FG division factor
	FG_DIV_3X_ELECTRICAL |
//	FG_DIV_DIV_1 |
//	FG_DIV_DIV_2 |

// FG output configuration
	FG_CONFIG_ACTIVE_MOTOR_IS_DRIVEN |
//	FG_CONFIG_ACTIVE_BEMF_OVER_THR |

// FG output BEMF threshold
//	FG_BEMF_THR_1_MV |
	FG_BEMF_THR_5_MV |
//	FG_BEMF_THR_20_MV |

// AVS enable
//	AVS_DIS |
	AVS_EN |

// Deadtime compensation enable
//	DEADTIME_COMP_DIS |
	DEADTIME_COMP_EN |

// Stop mode applied when stop mode is recirculation brake and motor in align or open loop state
	LOW_SPEED_RECIRC_BRAKE_HI_Z;
//	LOW_SPEED_RECIRC_BRAKE_LOW_SIDE;


// ==========================================
// 7.1.6 CLOSED_LOOP2 (0x8A)
// ==========================================
static const uint32_t REG_CLOSED_LOOP2_DATA =
// Motor stop mode
//	MTR_STOP_HI_Z |
	MTR_STOP_RECIRCULATION_STOP |
//	MTR_STOP_LOW_SIDE_BRAKING |
//	MTR_STOP_HIGH_SIDE_BRAKING |
//	MTR_STOP_ACTIVE_SPIN_DOWN |

// Brake time during motor stop
//	MTR_STOP_BRK_TIME_1_MS |
//	MTR_STOP_BRK_TIME_50_MS |
	MTR_STOP_BRK_TIME_100_MS |
//	MTR_STOP_BRK_TIME_1000_MS |
//	MTR_STOP_BRK_TIME_15000_MS |

// Speed threshold for active spin down
//	ACT_SPIN_THR_100_PER |
//	ACT_SPIN_THR_50_PER |
	ACT_SPIN_THR_10_PER |
//	ACT_SPIN_THR_2P5_PER |

// Speed threshold for BRAKE pin and Motor stop
//	BRAKE_SPEED_THRESHOLD_100_PER |
//	BRAKE_SPEED_THRESHOLD_50_PER |
	BRAKE_SPEED_THRESHOLD_10_PER |
//	BRAKE_SPEED_THRESHOLD_2P5_PER |

// 8-bit values for motor phase resistance (MPET 자동 측정)
	MOTOR_RES_SELF_MEASUREMENT |
//	MOTOR_RES_0P010_OHM |
//	MOTOR_RES_1P00_OHM |

// 8-bit values for motor phase inductance (MPET 자동 측정)
	MOTOR_IND_SELF_MEASUREMENT;
//	MOTOR_IND_0P010_MH |
//	MOTOR_IND_1P00_MH;


// ==========================================
// 7.1.7 CLOSED_LOOP3 (0x8C)
// ==========================================
static const uint32_t REG_CLOSED_LOOP3_DATA =
// 8-bit values for motor BEMF Constant (MPET 자동 측정)
	MOTOR_BEMF_CONST_SELF_MEASUREMENT |
//	MOTOR_BEMF_CONST_1P0_MV_HZ |

// 10-bit value for current Iq and Id loop Kp (MPET 자동 계산)
	(0x0 << CURR_LOOP_KP_BASE) |

// 10-bit value for current Iq and Id loop Ki (MPET 자동 계산)
	(0x0 << CURR_LOOP_KI_BASE) |

// 3 MSB bits for speed loop Kp (MPET 자동 계산)
	(0x0 << SPD_LOOP_KP_MSB_BASE);


// ==========================================
// 7.1.8 CLOSED_LOOP4 (0x8E)
// ==========================================
static const uint32_t REG_CLOSED_LOOP4_DATA =
// 7 LSB bits for speed loop Kp (MPET 자동 계산)
	(0x0 << SPD_LOOP_KP_LSB_BASE) |

// 10-bit value for speed loop Ki (MPET 자동 계산)
	(0x0 << SPD_LOOP_KI_BASE) |

// Maximum motor electrical speed (Hz): {MAX_SPEED/6}
// ex: 1000Hz -> (1000 * 6) = 6000
	(7000 << MAX_SPEED_BASE);


// ==========================================
// 7.2.1 FAULT_CONFIG1 (0x90)
// ==========================================
static const uint32_t REG_FAULT_CONFIG1_DATA =
// Current limit for Iq axis (torque) current reference in closed loop
//	ILIMIT_0P5_A |
//	ILIMIT_1P0_A |
//	ILIMIT_2P0_A |
	ILIMIT_3P0_A |
//	ILIMIT_5P0_A |
//	ILIMIT_8P0_A |

// Comparator based lock detection current threshold
//	HW_LOCK_ILIMIT_1P0_A |
//	HW_LOCK_ILIMIT_2P0_A |
	HW_LOCK_ILIMIT_4P0_A |
//	HW_LOCK_ILIMIT_8P0_A |

// ADC based lock detection current threshold
//	LOCK_ILIMIT_1P0_A |
//	LOCK_ILIMIT_2P0_A |
	LOCK_ILIMIT_4P0_A |
//	LOCK_ILIMIT_8P0_A |

// Fault response type for EEPROM fault
	EEP_FAULT_MODE_LATCHED |
//	EEP_FAULT_MODE_REPORT_ONLY |

// Lock current limit mode
//	LOCK_ILIMIT_MODE_LATCH_TRISTATE |
//	LOCK_ILIMIT_MODE_LATCH_LS_BRAKE |
	LOCK_ILIMIT_MODE_RETRY_TRISTATE |
//	LOCK_ILIMIT_MODE_REPORT_ONLY |
//	LOCK_ILIMIT_MODE_DIS |

// Lock current limit detection deglitch time
//	LOCK_ILIMIT_DEG_NO_DEGLITCH |
//	LOCK_ILIMIT_DEG_1_MS |
	LOCK_ILIMIT_DEG_5_MS |
//	LOCK_ILIMIT_DEG_10_MS |
//	LOCK_ILIMIT_DEG_1000_MS |

// Lock detection retry time
//	LCK_RETRY_300_MS |
	LCK_RETRY_1_S |
//	LCK_RETRY_5_S |
//	LCK_RETRY_14_S |

// CRC (in I2C communication) error mode
	CRC_ERR_MODE_LATCHED |
//	CRC_ERR_MODE_REPORT_ONLY |

// Motor lock mode
//	MTR_LCK_MODE_LATCH_TRISTATE |
	MTR_LCK_MODE_RETRY_TRISTATE |
//	MTR_LCK_MODE_REPORT_ONLY |
//	MTR_LCK_MODE_DIS |

// IPD timeout fault enable
//	IPD_TIMEOUT_FAULT_DIS |
	IPD_TIMEOUT_FAULT_EN |

// IPD frequency fault enable
//	IPD_FREQ_FAULT_DIS |
	IPD_FREQ_FAULT_EN |

// Enable indication of current loop and speed loop saturation
	SATURATION_FLAGS_DIS;
//	SATURATION_FLAGS_EN;


// ==========================================
// 7.2.2 FAULT_CONFIG2 (0x92)
// ==========================================
static const uint32_t REG_FAULT_CONFIG2_DATA =
// Lock 1 (Abnormal Speed) Enable
//	LOCK1_DIS |
	LOCK1_EN |

// Lock 2 (Abnormal BEMF) Enable
//	LOCK2_DIS |
	LOCK2_EN |

// Lock 3 (No Motor) Enable
//	LOCK3_DIS |
	LOCK3_EN |

// Abnormal speed lock threshold (% of MAX_SPEED)
//	LOCK_ABN_SPEED_130_PER |
	LOCK_ABN_SPEED_150_PER |
//	LOCK_ABN_SPEED_200_PER |

// Abnormal BEMF lock threshold (% of expected BEMF)
//	ABNORMAL_BEMF_THR_40_PER |
	ABNORMAL_BEMF_THR_50_PER |
//	ABNORMAL_BEMF_THR_70_PER |

// No motor lock threshold
//	NO_MTR_THR_0P0375_A |
	NO_MTR_THR_0P1_A |
//	NO_MTR_THR_0P5_A |
//	NO_MTR_THR_1P0_A |

// Hardware lock current limit mode
//	HW_LOCK_ILIMIT_MODE_LATCH_TRISTATE |
	HW_LOCK_ILIMIT_MODE_RETRY_TRIS |
//	HW_LOCK_ILIMIT_MODE_REPORT_ONLY |
//	HW_LOCK_ILIMIT_MODE_DIS |

// Hardware lock current limit detection deglitch time
//	HW_LOCK_ILIMIT_DEG_NO_DEGLITCH |
	HW_LOCK_ILIMIT_DEG_3_US |
//	HW_LOCK_ILIMIT_DEG_7_US |

// Hysteresis for controller overvoltage and undervoltage faults
//	VOLTAGE_HYSTERESIS_0P5_V |
	VOLTAGE_HYSTERESIS_1_V |
//	VOLTAGE_HYSTERESIS_3_V |

// Controller under voltage fault threshold
	MIN_VM_MOTOR_NO_LIMIT |
//	MIN_VM_MOTOR_6_V |
//	MIN_VM_MOTOR_8_V |
//	MIN_VM_MOTOR_10_V |
//	MIN_VM_MOTOR_18_V |

// DC bus (controller) undervoltage fault recovery mode
	MIN_VM_MODE_LATCHED |
//	MIN_VM_MODE_AUTO_CLEAR |

// Controller over voltage fault threshold
	MAX_VM_MOTOR_NO_LIMIT |
//	MAX_VM_MOTOR_18_V |
//	MAX_VM_MOTOR_18_V |
//	MAX_VM_MOTOR_30_V |
//	MAX_VM_MOTOR_34_V |

// DC bus (controller) overvoltage fault recovery mode
	MAX_VM_MODE_LATCHED |
//	MAX_VM_MODE_AUTO_CLEAR |

// Number of automatic retry attempts
//	AUTO_RETRY_TIMES_NO_LIMIT |
//	AUTO_RETRY_TIMES_2 |
	AUTO_RETRY_TIMES_5;
//	AUTO_RETRY_TIMES_20;


// ==========================================
// 7.1.9 REF_PROFILES1 (0x94)
// ==========================================
static const uint32_t REG_REF_PROFILES1_DATA =
	REF_PROFILE_CONFIG_LINEAR |
	(0x0 << DUTY_ON1_BASE) |
	(0x0 << DUTY_OFF1_BASE) |
	(0x0 << DUTY_CLAMP1_BASE) |
	(0x0 << DUTY_A_LSB_BASE);


// ==========================================
// 7.1.10 REF_PROFILES2 (0x96)
// ==========================================
static const uint32_t REG_REF_PROFILES2_DATA =
	(0x0 << DUTY_A_MSB_BASE) |
	(0x0 << DUTY_B_BASE) |
	(0x0 << DUTY_C_BASE) |
	(0x0 << DUTY_D_BASE) |
	(0x0 << DUTY_E_MSB_BASE);


// ==========================================
// 7.1.11 REF_PROFILES3 (0x98)
// ==========================================
static const uint32_t REG_REF_PROFILES3_DATA =
	(0x0 << DUTY_E_LSB_BASE) |
	(0x0 << DUTY_ON2_BASE) |
	(0x0 << DUTY_OFF2_BASE) |
	(0x0 << DUTY_CLAMP2_BASE) |
	DUTY_HYS_0_PER;


// ==========================================
// 7.1.12 REF_PROFILES4 (0x9A)
// ==========================================
static const uint32_t REG_REF_PROFILES4_DATA =
	(0x0 << REF_OFF1_BASE) |
	(0x0 << REF_CLAMP1_BASE) |
	(0x0 << REF_A_BASE) |
	(0x0 << REF_B_MSB_BASE);


// ==========================================
// 7.1.13 REF_PROFILES5 (0x9C)
// ==========================================
static const uint32_t REG_REF_PROFILES5_DATA =
	(0x0 << REF_B_LSB_BASE) |
	(0x0 << REF_C_BASE) |
	(0x0 << REF_D_BASE) |
	(0x0 << REF_E_BASE) |
	MIN_DUTY_1_PER |
	VOLTAGE_MODE_CONFIG_USER_DEF |
	DUTY_COMMAND_FILTER_DIS;


// ==========================================
// 7.1.14 REF_PROFILES6 (0x9E)
// ==========================================
static const uint32_t REG_REF_PROFILES6_DATA =
	(0x0 << REF_OFF2_BASE) |
	(0x0 << REF_CLAMP2_BASE);


// ==========================================
// 7.4.1 INT_ALGO_1 (0xA0)
// ==========================================
static const uint32_t REG_INT_ALGO_1_DATA =
// Active brake speed delta limit exit
//	ACTIVE_BRAKE_SPEED_DELTA_EXIT_2P5_PER |
	ACTIVE_BRAKE_SPEED_DELTA_EXIT_5_PER |
//	ACTIVE_BRAKE_SPEED_DELTA_EXIT_7P5_PER |
//	ACTIVE_BRAKE_SPEED_DELTA_EXIT_10_PER |

// Speed pin glitch filter
//	SPEED_PIN_GLITCH_FILTER_DIS |
	SPEED_PIN_GLITCH_FILTER_0P2_US |
//	SPEED_PIN_GLITCH_FILTER_0P5_US |
//	SPEED_PIN_GLITCH_FILTER_1P0_US |

// Fast ISD enable
//	FAST_ISD_DIS |
	FAST_ISD_EN |

// ISD stop time
//	ISD_STOP_TIME_1_MS |
	ISD_STOP_TIME_5_MS |
//	ISD_STOP_TIME_50_MS |
//	ISD_STOP_TIME_100_MS |

// ISD run time
//	ISD_RUN_TIME_1_MS |
	ISD_RUN_TIME_5_MS |
//	ISD_RUN_TIME_50_MS |
//	ISD_RUN_TIME_100_MS |

// ISD timeout
	ISD_TIMEOUT_500_MS |
//	ISD_TIMEOUT_750_MS |
//	ISD_TIMEOUT_1000_MS |
//	ISD_TIMEOUT_2000_MS |

// Auto handoff minimum BEMF
//	AUTO_HANDOFF_MIN_BEMF_0_MV |
	AUTO_HANDOFF_MIN_BEMF_50_MV |
//	AUTO_HANDOFF_MIN_BEMF_100_MV |
//	AUTO_HANDOFF_MIN_BEMF_250_MV |
//	AUTO_HANDOFF_MIN_BEMF_500_MV |
//	AUTO_HANDOFF_MIN_BEMF_1000_MV |
//	AUTO_HANDOFF_MIN_BEMF_1250_MV |
//	AUTO_HANDOFF_MIN_BEMF_1500_MV |

// Brake current persist time
	BRAKE_CURRENT_PERSIST_50_MS |
//	BRAKE_CURRENT_PERSIST_100_MS |
//	BRAKE_CURRENT_PERSIST_250_MS |
//	BRAKE_CURRENT_PERSIST_500_MS |

// Reverse drive open loop deceleration
	REV_DRV_OPEN_LOOP_DEC_50_PER;
//	REV_DRV_OPEN_LOOP_DEC_60_PER |
//	REV_DRV_OPEN_LOOP_DEC_70_PER |
//	REV_DRV_OPEN_LOOP_DEC_80_PER |
//	REV_DRV_OPEN_LOOP_DEC_90_PER |
//	REV_DRV_OPEN_LOOP_DEC_100_PER |
//	REV_DRV_OPEN_LOOP_DEC_125_PER |
//	REV_DRV_OPEN_LOOP_DEC_150_PER;


// ==========================================
// 7.4.2 INT_ALGO_2 (0xA2)
// ==========================================
static const uint32_t REG_INT_ALGO_2_DATA =
// Flux weakening loop Kp
	(0x0 << FLUX_WEAK_KP_BASE) |

// Flux weakening loop Ki
	(0x0 << FLUX_WEAK_KI_BASE) |

// Flux weakening enable
	FLUX_WEAK_DIS |
//	FLUX_WEAK_EN |

// Closed-loop slow acceleration
//	CL_SLOW_ACC_0P1_HZ_S |
//	CL_SLOW_ACC_1_HZ_S |
//	CL_SLOW_ACC_2_HZ_S |
//	CL_SLOW_ACC_3_HZ_S |
//	CL_SLOW_ACC_5_HZ_S |
	CL_SLOW_ACC_10_HZ_S |
//	CL_SLOW_ACC_20_HZ_S |
//	CL_SLOW_ACC_30_HZ_S |
//	CL_SLOW_ACC_40_HZ_S |
//	CL_SLOW_ACC_50_HZ_S |
//	CL_SLOW_ACC_100_HZ_S |
//	CL_SLOW_ACC_200_HZ_S |
//	CL_SLOW_ACC_500_HZ_S |
//	CL_SLOW_ACC_750_HZ_S |
//	CL_SLOW_ACC_1000_HZ_S |
//	CL_SLOW_ACC_2000_HZ_S |

// Active brake bus current slew rate
//	ACTIVE_BRAKE_BUS_CURRENT_SLEW_10_A_S |
//	ACTIVE_BRAKE_BUS_CURRENT_SLEW_50_A_S |
	ACTIVE_BRAKE_BUS_CURRENT_SLEW_100_A_S |
//	ACTIVE_BRAKE_BUS_CURRENT_SLEW_250_A_S |
//	ACTIVE_BRAKE_BUS_CURRENT_SLEW_500_A_S |
//	ACTIVE_BRAKE_BUS_CURRENT_SLEW_1000_A_S |
//	ACTIVE_BRAKE_BUS_CURRENT_SLEW_5000_A_S |
//	ACTIVE_BRAKE_BUS_CURRENT_SLEW_NO_LIMIT |

// ISD BEMF filter enable
//	ISD_BEMF_FILT_DIS |
	ISD_BEMF_FILT_EN |

// Circular current limit enable
//	CIRCULAR_CURRENT_LIMIT_DIS |
	CIRCULAR_CURRENT_LIMIT_EN |

// IPD high resolution enable
	IPD_HIGH_RESOLUTION_DIS;
//	IPD_HIGH_RESOLUTION_EN;


// ==========================================
// 7.3.1 PIN_CONFIG (0xA4)
// ==========================================
static const uint32_t REG_PIN_CONFIG_DATA =
// PWM dither step
	PWM_DITHER_STEP_1 |
//	PWM_DITHER_STEP_2 |
//	PWM_DITHER_STEP_5 |
//	PWM_DITHER_STEP_10 |

// VDC filter
//	VDC_FILTER_DIS |
	VDC_FILTER_DEFAULT |
//	VDC_FILTER_100_HZ |
//	VDC_FILTER_1000_HZ |

// Lead angle
	LEAD_ANGLE_0_DEG |
//	LEAD_ANGLE_3_DEG |
//	LEAD_ANGLE_6_DEG |
//	LEAD_ANGLE_9_DEG |
//	LEAD_ANGLE_12_DEG |
//	LEAD_ANGLE_15_DEG |
//	LEAD_ANGLE_18_DEG |
//	LEAD_ANGLE_21_DEG |
//	LEAD_ANGLE_24_DEG |
//	LEAD_ANGLE_27_DEG |
//	LEAD_ANGLE_30_DEG |
//	LEAD_ANGLE_33_DEG |
//	LEAD_ANGLE_36_DEG |
//	LEAD_ANGLE_39_DEG |
//	LEAD_ANGLE_42_DEG |
//	LEAD_ANGLE_45_DEG |
//	LEAD_ANGLE_N48_DEG |
//	LEAD_ANGLE_N45_DEG |
//	LEAD_ANGLE_N42_DEG |
//	LEAD_ANGLE_N39_DEG |
//	LEAD_ANGLE_N36_DEG |
//	LEAD_ANGLE_N33_DEG |
//	LEAD_ANGLE_N30_DEG |
//	LEAD_ANGLE_N27_DEG |
//	LEAD_ANGLE_N24_DEG |
//	LEAD_ANGLE_N21_DEG |
//	LEAD_ANGLE_N18_DEG |
//	LEAD_ANGLE_N15_DEG |
//	LEAD_ANGLE_N12_DEG |
//	LEAD_ANGLE_N9_DEG |
//	LEAD_ANGLE_N6_DEG |
//	LEAD_ANGLE_N3_DEG |

// Maximum power
	(0x0 << MAX_POWER_BASE) |

// FG idle configuration
	FG_IDLE_CONFIG_CONTINUE |
//	FG_IDLE_CONFIG_HIGH |
//	FG_IDLE_CONFIG_LOW |

// FG fault configuration
	FG_FAULT_CONFIG_LAST |
//	FG_FAULT_CONFIG_HIGH |
//	FG_FAULT_CONFIG_LOW |
//	FG_FAULT_CONFIG_ACTIVE_TO_BEMF |

// Alarm pin enable
	ALARM_PIN_DIS |
//	ALARM_PIN_EN |

// Brake pin mode
	BRAKE_PIN_MODE_LOW_SIDE |
//	BRAKE_PIN_MODE_ALIGN |

// Align brake angle select
	ALIGN_BRAKE_ANGLE_LAST |
//	ALIGN_BRAKE_ANGLE_CONFIG |

// Brake input override
	BRAKE_INPUT_PIN |
//	BRAKE_INPUT_OVERRIDE_BRAKE |
//	BRAKE_INPUT_OVERRIDE_NO_BRAKE |

// Speed mode
//	SPEED_MODE_ANALOG;
//	SPEED_MODE_PWM;
	SPEED_MODE_I2C;
//	SPEED_MODE_FREQ;


// ==========================================
// 7.3.2 DEVICE_CONFIG1 (0xA6)
// ==========================================
static const uint32_t REG_DEVICE_CONFIG1_DATA =
// DACOUT2 or SOx selection
	DAC_SOX_SEL_DACOUT2 |
//	DAC_SOX_SEL_SOA |
//	DAC_SOX_SEL_SOB |
//	DAC_SOX_SEL_SOC |

// PWM dither mode
	PWM_DITHER_MODE_TRIANGULAR |
//	PWM_DITHER_MODE_RANDOM |

// I2C target address
	(0x02 << I2C_TARGET_ADDR_BASE) |

// EEPROM lock key
	(0x0 << EEPROM_LOCK_KEY_BASE) |

// I2C pin slew rate
	SLEW_RATE_I2C_PINS_4P8_MA |
//	SLEW_RATE_I2C_PINS_3P9_MA |
//	SLEW_RATE_I2C_PINS_1P86_MA |
//	SLEW_RATE_I2C_PINS_30P8_MA |

// Internal pull-up enable for nFAULT and FG
//	PULLUP_DIS |
	PULLUP_EN |

// Maximum DC bus voltage
//	BUS_VOLT_15_V;
//	BUS_VOLT_30_V;
//	BUS_VOLT_40_V;
	BUS_VOLT_NOT_APPLI;


// ==========================================
// 7.3.3 DEVICE_CONFIG2 (0xA8)
// ==========================================
static const uint32_t REG_DEVICE_CONFIG2_DATA =
// Input maximum frequency
	(0x0 << INPUT_MAXIMUM_FREQ_BASE) |

// Sleep entry time
//	SLEEP_ENTRY_TIME_50_US |
//	SLEEP_ENTRY_TIME_200_US |
	SLEEP_ENTRY_TIME_20_MS |
//	SLEEP_ENTRY_TIME_200_MS |

// Dynamic CSA gain
//	DYNAMIC_CSA_GAIN_DIS |
	DYNAMIC_CSA_GAIN_EN |

// Dynamic voltage gain
//	DYNAMIC_VOLTAGE_GAIN_DIS |
	DYNAMIC_VOLTAGE_GAIN_EN |

// Device mode
	DEV_MODE_STANDBY |
//	DEV_MODE_SLEEP |

// PWM dither depth
//	PWM_DITHER_DEPTH_DIS |
//	PWM_DITHER_DEPTH_5_PER |
//	PWM_DITHER_DEPTH_7P5_PER |
	PWM_DITHER_DEPTH_10_PER |

// External clock enable
	EXT_CLK_DIS |
//	EXT_CLK_EN |

// External clock frequency
	EXT_CLK_CONFIG_8_KHZ |
//	EXT_CLK_CONFIG_16_KHZ |
//	EXT_CLK_CONFIG_32_KHZ |
//	EXT_CLK_CONFIG_64_KHZ |
//	EXT_CLK_CONFIG_128_KHZ |
//	EXT_CLK_CONFIG_256_KHZ |
//	EXT_CLK_CONFIG_512_KHZ |
//	EXT_CLK_CONFIG_1024_KHZ |

// External watchdog enable
	EXT_WDT_DIS |
//	EXT_WDT_EN |

// External watchdog timing
	EXT_WDT_CONFIG_100MS_1S |
//	EXT_WDT_CONFIG_200MS_2S |
//	EXT_WDT_CONFIG_500MS_5S |
//	EXT_WDT_CONFIG_1000MS_10S |

// External watchdog input mode
	EXT_WDT_INPUT_MODE_I2C |
//	EXT_WDT_INPUT_MODE_GPIO |

// External watchdog fault mode
	EXT_WDT_FAULT_MODE_REPORT_ONLY;
//	EXT_WDT_FAULT_MODE_LATCH_HIZ;


// ==========================================
// 7.3.4 PERI_CONFIG1 (0xAA)
// ==========================================
static const uint32_t REG_PERI_CONFIG1_DATA =
// Spread spectrum modulation disable
	SPREAD_SPECTRUM_MOD_EN |
//	SPREAD_SPECTRUM_MOD_DIS |

// No-motor fault in closed loop
	NO_MTR_FLT_CLOSEDLOOP_EN |
//	NO_MTR_FLT_CLOSEDLOOP_DIS |

// Abnormal BEMF persistent time
//	ABNORMAL_BEMF_PERSISTENT_2_CYCLE |
	ABNORMAL_BEMF_PERSISTENT_500_MS |
//	ABNORMAL_BEMF_PERSISTENT_1000_MS |
//	ABNORMAL_BEMF_PERSISTENT_2000_MS |

// Flux weakening reference
//	FLUX_WEAK_REF_70_PER |
//	FLUX_WEAK_REF_80_PER |
	FLUX_WEAK_REF_90_PER |
//	FLUX_WEAK_REF_95_PER |

// Input reference window
	INPUT_REFERENCE_WINDOW_DIS |
//	INPUT_REFERENCE_WINDOW_5_PER |
//	INPUT_REFERENCE_WINDOW_10_PER |
//	INPUT_REFERENCE_WINDOW_15_PER |

// Bus power limit enable
	BUS_POWER_LIMIT_DIS |
//	BUS_POWER_LIMIT_EN |

// Direction input override
	DIR_INPUT_PIN |
//	DIR_INPUT_CW |
//	DIR_INPUT_CCW |

// Direction change mode
	DIR_CHANGE_MODE_STOP |
//	DIR_CHANGE_MODE_REVERSE_DRIVE |

// Speed limit enable
	SPEED_LIMIT_DIS |
//	SPEED_LIMIT_EN |

// Active brake speed delta limit entry
//	ACTIVE_BRAKE_SPEED_DELTA_ENTRY_NOT_APPLI |
//	ACTIVE_BRAKE_SPEED_DELTA_ENTRY_5_PER |
	ACTIVE_BRAKE_SPEED_DELTA_ENTRY_10_PER |
//	ACTIVE_BRAKE_SPEED_DELTA_ENTRY_15_PER |
//	ACTIVE_BRAKE_SPEED_DELTA_ENTRY_20_PER |
//	ACTIVE_BRAKE_SPEED_DELTA_ENTRY_25_PER |
//	ACTIVE_BRAKE_SPEED_DELTA_ENTRY_30_PER |
//	ACTIVE_BRAKE_SPEED_DELTA_ENTRY_35_PER |
//	ACTIVE_BRAKE_SPEED_DELTA_ENTRY_40_PER |
//	ACTIVE_BRAKE_SPEED_DELTA_ENTRY_45_PER |
//	ACTIVE_BRAKE_SPEED_DELTA_ENTRY_50_PER |
//	ACTIVE_BRAKE_SPEED_DELTA_ENTRY_60_PER |
//	ACTIVE_BRAKE_SPEED_DELTA_ENTRY_70_PER |
//	ACTIVE_BRAKE_SPEED_DELTA_ENTRY_80_PER |
//	ACTIVE_BRAKE_SPEED_DELTA_ENTRY_90_PER |
//	ACTIVE_BRAKE_SPEED_DELTA_ENTRY_100_PER |

// Active brake modulation index limit
//	ACTIVE_BRAKE_MOD_INDEX_LIMIT_0_PER |
//	ACTIVE_BRAKE_MOD_INDEX_LIMIT_40_PER |
//	ACTIVE_BRAKE_MOD_INDEX_LIMIT_50_PER |
//	ACTIVE_BRAKE_MOD_INDEX_LIMIT_60_PER |
//	ACTIVE_BRAKE_MOD_INDEX_LIMIT_70_PER |
//	ACTIVE_BRAKE_MOD_INDEX_LIMIT_80_PER |
//	ACTIVE_BRAKE_MOD_INDEX_LIMIT_90_PER |
	ACTIVE_BRAKE_MOD_INDEX_LIMIT_100_PER |

// Speed range selection
	SPEED_RANGE_SEL_LOW |
//	SPEED_RANGE_SEL_HIGH |

// Input reference mode
	INPUT_REFERENCE_MODE_SPEED |
//	INPUT_REFERENCE_MODE_POWER |
//	INPUT_REFERENCE_MODE_TORQUE |
//	INPUT_REFERENCE_MODE_MOD_INDEX |

// EEPROM lock mode
	EEPROM_LOCK_MODE_NO_PASS;
//	EEPROM_LOCK_MODE_PASS |
//	EEPROM_LOCK_MODE_READ_PASS_WRITE_LOCK |
//	EEPROM_LOCK_MODE_LOCK;


// ==========================================
// 7.3.5 GD_CONFIG1 (0xAC)
// ==========================================
static const uint32_t REG_GD_CONFIG1_DATA =
// Slew rate
//	SLEW_RATE_NOT_APPLI_0 |
//	SLEW_RATE_NOT_APPLI_1 |
//	SLEW_RATE_125_V_US |
	SLEW_RATE_200_V_US |

// Overvoltage level
//	OVP_SEL_34_V |
	OVP_SEL_22_V |

// Overvoltage protection enable
//	OVP_DIS |
	OVP_EN |

// Overtemperature warning reporting
//	OTW_REP_DIS |
	OTW_REP_EN |

// OCP deglitch time
//	OCP_DEG_0P2_US |
	OCP_DEG_0P6_US |
//	OCP_DEG_1P2_US |
//	OCP_DEG_1P6_US |

// OCP level
	OCP_LVL_24_A |
//	OCP_LVL_16_A |

// OCP fault mode
	OCP_MODE_LATCH |
//	OCP_MODE_RETRY |
//	OCP_MODE_NOT_APPLI_2 |
//	OCP_MODE_NOT_APPLI_3 |

// Current sense amplifier gain
	CSA_GAIN_0P15_V_A;
//	CSA_GAIN_0P3_V_A |
//	CSA_GAIN_0P6_V_A |
//	CSA_GAIN_1P2_V_A;


// ==========================================
// 7.3.6 GD_CONFIG2 (0xAE)
// ==========================================
static const uint32_t REG_GD_CONFIG2_DATA =
// Buck power sequencing
	BUCK_PS_EN |
//	BUCK_PS_DIS |

// Buck current limit
	BUCK_CL_600_MA |
//	BUCK_CL_150_MA |

// Buck output voltage
	BUCK_SEL_3P3_V |
//	BUCK_SEL_5P0_V |
//	BUCK_SEL_4P0_V |
//	BUCK_SEL_5P7_V |

// Buck disable
	BUCK_EN |
//	BUCK_DIS |

// Minimum ON time
//	MIN_ON_TIME_0_US |
	MIN_ON_TIME_AUTO;
//	MIN_ON_TIME_0P5_US |
//	MIN_ON_TIME_0P75_US |
//	MIN_ON_TIME_1_US |
//	MIN_ON_TIME_1P25_US |
//	MIN_ON_TIME_1P5_US |
//	MIN_ON_TIME_2_US;


// ============================================================================
// 8.3.1 ALGO_CTRL1 (0xEA)
// ============================================================================
static const uint32_t REG_ALGO_CTRL1_DATA =
// EEPROM write command
	(0x0 << EEPROM_WRT_BASE) |
//	EEPROM_WRT |

// EEPROM read command
	(0x0 << EEPROM_READ_BASE) |
//	EEPROM_READ |

// Clear faults
	(0x0 << CLR_FLT_BASE) |
//	CLR_FLT |

// Clear fault retry count
	(0x0 << CLR_FLT_RETRY_COUNT_BASE) |

// EEPROM write access key
	EEPROM_WRITE_ACCESS_KEY |

// Forced align angle
	(0x0 << FORCED_ALIGN_ANGLE_BASE) |

// Watchdog tickle
	(0x0 << WATCHDOG_TICKLE_BASE);
//	WATCHDOG_TICKLE;


// ============================================================================
// 8.4.1 ALGO_DEBUG1 (0xEC)
// ============================================================================
static const uint32_t REG_ALGO_DEBUG1_DATA =
// Reference input override
	OVERRIDE_PIN_SPEED_CMD |
//	OVERRIDE_DIGITAL_SPEED_CTRL |

// Digital speed command
	(0x0 << DIGITAL_SPEED_CTRL_BASE) |

// Closed loop disable
	CLOSED_LOOP_EN |
//	CLOSED_LOOP_DIS |

// Force align
	FORCE_ALIGN_DIS |
//	FORCE_ALIGN_EN |

// Force slow first cycle
	FORCE_SLOW_FIRST_CYCLE_DIS |
//	FORCE_SLOW_FIRST_CYCLE_EN |

// Force IPD
	FORCE_IPD_DIS |
//	FORCE_IPD_EN |

// Force ISD
	FORCE_ISD_DIS |
//	FORCE_ISD_EN |

// Force align angle source
	FORCE_ALIGN_ANGLE_SRC_CONFIG;
//	FORCE_ALIGN_ANGLE_SRC_FORCED;


// ============================================================================
// 8.4.2 ALGO_DEBUG2 (0xEE)
// ============================================================================
static const uint32_t REG_ALGO_DEBUG2_DATA =
// Force recirculate stop sector
	FORCE_RECIRCULATE_STOP_LAST |
//	FORCE_RECIRCULATE_STOP_SECTOR1 |
//	FORCE_RECIRCULATE_STOP_SECTOR2 |
//	FORCE_RECIRCULATE_STOP_SECTOR3 |
//	FORCE_RECIRCULATE_STOP_SECTOR4 |
//	FORCE_RECIRCULATE_STOP_SECTOR5 |
//	FORCE_RECIRCULATE_STOP_SECTOR6 |

// Force recirculate stop enable
	FORCE_RECIRCULATE_STOP_EN |
//	FORCE_RECIRCULATE_STOP_DIS |

// Current loop disable
	CURRENT_LOOP_EN |
//	CURRENT_LOOP_DIS |

// Forced Vd when current loop is disabled
	(0x0 << FORCE_VD_CURRENT_LOOP_DIS_BASE) |

// Forced Vq when current loop is disabled
	(0x0 << FORCE_VQ_CURRENT_LOOP_DIS_BASE) |

// MPET command
//	(0x0 << MPET_CMD_BASE) |
	MPET_CMD|

// MPET resistance measurement
//	(0x0 << MPET_R_BASE) |
	MPET_R |

// MPET inductance measurement
//	(0x0 << MPET_L_BASE) |
	MPET_L |

// MPET BEMF constant measurement
//	(0x0 << MPET_KE_BASE) |
	MPET_KE |

// MPET mechanical parameter measurement
//	(0x0 << MPET_MECH_BASE) |
	MPET_MECH |

// MPET write measured parameters to shadow
//	(0x0 << MPET_WRITE_SHADOW_BASE);
	MPET_WRITE_SHADOW;


// ============================================================================
// 8.4.3 CURRENT_PI (0xF0)
// ============================================================================
static const uint32_t REG_CURRENT_PI_DATA =
// Current loop Ki used
	(0x0 << CURRENT_LOOP_KI_BASE) |

// Current loop Kp used
	(0x0 << CURRENT_LOOP_KP_BASE);


// ============================================================================
// 8.4.4 SPEED_PI (0xF2)
// ============================================================================
static const uint32_t REG_SPEED_PI_DATA =
// Speed loop Ki used
	(0x0 << SPEED_LOOP_KI_BASE) |

// Speed loop Kp used
	(0x0 << SPEED_LOOP_KP_BASE);


// ============================================================================
// 8.4.5 DAC_1 (0xF4)
// ============================================================================
static const uint32_t REG_DAC_1_DATA =
// DACOUT1 enum scaling
	(0x0 << DACOUT1_ENUM_SCALING_BASE) |

// DACOUT1 scaling
	(0x8 << DACOUT1_SCALING_BASE) |

// DACOUT1 polarity
	DACOUT1_BIPOLAR |
//	DACOUT1_UNIPOLAR |

// DACOUT1 variable address
	(0x0 << DACOUT1_VAR_ADDR_BASE);


// ============================================================================
// 8.4.6 DAC_2 (0xF6)
// ============================================================================
static const uint32_t REG_DAC_2_DATA =
// DACOUT2 enum scaling
	(0x0 << DACOUT2_ENUM_SCALING_BASE) |

// DACOUT2 scaling
	(0x8 << DACOUT2_SCALING_BASE) |

// DACOUT2 polarity
	DACOUT2_BIPOLAR |
//	DACOUT2_UNIPOLAR |

// DACOUT2 variable address
	(0x0 << DACOUT2_VAR_ADDR_BASE);


// ============================================================================
// 8.4.7 EEPROM_SECURITY (0xF8)
// ============================================================================
static const uint32_t REG_EEPROM_SECURITY_DATA =
// User EEPROM key
	(0x0 << USER_EEPROM_KEY_BASE);
// @formatter:on

uint8_t mcf_i2c_addr_8bit = MCF8316C_I2C_ADDR_WRITE; // 초기값

void MCF8316C_Read_Faults(MCF8316C_FaultStatus_t *faults) {
    if (faults == NULL) return;
    faults->gate_driver_fault = MCF8316C_ReadReg32(REG_GATE_DRIVER_FAULT_STATUS);
    faults->controller_fault  = MCF8316C_ReadReg32(REG_CONTROLLER_FAULT_STATUS);
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

    // 매크로 대신 mcf_i2c_addr_8bit 변수 사용
	HAL_I2C_Master_Transmit(MCF_I2C, mcf_i2c_addr_8bit, tx_buffer, 7, 100);
}

uint32_t MCF8316C_ReadReg32(uint32_t reg_addr) {
	uint8_t ctrl_buffer[3];
	uint8_t rx_buffer[4] = { 0, 0, 0, 0 };
	uint32_t ctrl_word = (0x01 << 23) | (0x01 << 20) | (reg_addr & 0xFFFF);

	ctrl_buffer[0] = (ctrl_word >> 16) & 0xFF;
	ctrl_buffer[1] = (ctrl_word >> 8) & 0xFF;
	ctrl_buffer[2] = ctrl_word & 0xFF;

    // 매크로 대신 mcf_i2c_addr_8bit 변수 사용
	HAL_I2C_Master_Transmit(MCF_I2C, mcf_i2c_addr_8bit, ctrl_buffer, 3, 100);
	HAL_I2C_Master_Receive(MCF_I2C, mcf_i2c_addr_8bit, rx_buffer, 4, 100);

	return ((uint32_t) rx_buffer[3] << 24) | ((uint32_t) rx_buffer[2] << 16)
			| ((uint32_t) rx_buffer[1] << 8) | rx_buffer[0];
}

void MCF8316C_Config_Manual(void) {
	MCF8316C_WriteReg32(REG_ISD_CONFIG, REG_ISD_CONFIG_DATA);
	MCF8316C_WriteReg32(REG_REV_DRIVE_CONFIG, REG_REV_DRIVE_CONFIG_DATA);
	MCF8316C_WriteReg32(REG_MOTOR_STARTUP1, REG_MOTOR_STARTUP1_DATA);
	MCF8316C_WriteReg32(REG_MOTOR_STARTUP2, REG_MOTOR_STARTUP2_DATA);
	MCF8316C_WriteReg32(REG_CLOSED_LOOP1, REG_CLOSED_LOOP1_DATA);
	MCF8316C_WriteReg32(REG_CLOSED_LOOP2, REG_CLOSED_LOOP2_DATA);
	MCF8316C_WriteReg32(REG_CLOSED_LOOP3, REG_CLOSED_LOOP3_DATA);
	MCF8316C_WriteReg32(REG_CLOSED_LOOP4, REG_CLOSED_LOOP4_DATA);
	MCF8316C_WriteReg32(REG_FAULT_CONFIG1, REG_FAULT_CONFIG1_DATA);
	MCF8316C_WriteReg32(REG_FAULT_CONFIG2, REG_FAULT_CONFIG2_DATA);
	MCF8316C_WriteReg32(REG_REF_PROFILES1, REG_REF_PROFILES1_DATA);
	MCF8316C_WriteReg32(REG_REF_PROFILES2, REG_REF_PROFILES2_DATA);
	MCF8316C_WriteReg32(REG_REF_PROFILES3, REG_REF_PROFILES3_DATA);
	MCF8316C_WriteReg32(REG_REF_PROFILES4, REG_REF_PROFILES4_DATA);
	MCF8316C_WriteReg32(REG_REF_PROFILES5, REG_REF_PROFILES5_DATA);
	MCF8316C_WriteReg32(REG_REF_PROFILES6, REG_REF_PROFILES6_DATA);
	MCF8316C_WriteReg32(REG_INT_ALGO_1, REG_INT_ALGO_1_DATA);
	MCF8316C_WriteReg32(REG_INT_ALGO_2, REG_INT_ALGO_2_DATA);
	MCF8316C_WriteReg32(REG_PIN_CONFIG, REG_PIN_CONFIG_DATA);

    // ==============================================================
    // 핵심 버그 수정: DEVICE_CONFIG1을 쓸 때 현재 I2C 주소를 무조건 유지시킴
    // ==============================================================
    uint32_t dev_cfg1 = REG_DEVICE_CONFIG1_DATA;
    dev_cfg1 &= ~(0x7F << I2C_TARGET_ADDR_BASE);                   // 데이터에 적힌 주소값 삭제
    dev_cfg1 |= ((mcf_i2c_addr_8bit >> 1) << I2C_TARGET_ADDR_BASE); // 스캔된 실제 7-bit 주소 강제 삽입
	MCF8316C_WriteReg32(REG_DEVICE_CONFIG1, dev_cfg1);

	MCF8316C_WriteReg32(REG_DEVICE_CONFIG2, REG_DEVICE_CONFIG2_DATA);
	MCF8316C_WriteReg32(REG_PERI_CONFIG1, REG_PERI_CONFIG1_DATA);
	MCF8316C_WriteReg32(REG_GD_CONFIG1, REG_GD_CONFIG1_DATA);
	MCF8316C_WriteReg32(REG_GD_CONFIG2, REG_GD_CONFIG2_DATA);
}

uint8_t MCF8316C_Check_Connection(void) {
	if (HAL_I2C_IsDeviceReady(MCF_I2C, MCF8316C_I2C_ADDR_WRITE, 3, 100) == HAL_OK)
		return 1;
	return 0;
}

void MCF8316C_Emergency_Recovery(void) {
    // 구현 필요 시 작성
}

void MCF8316C_Config_MPET(void) {
	// 매크로상에 MPET 측정을 위한 플래그가 포함되어 있으므로 Manual Config 호출
	MCF8316C_Config_Manual();
}

void MCF8316C_Start_MPET(void) {
	uint32_t algo_debug2_val = REG_ALGO_DEBUG2_DATA;
	MCF8316C_WriteReg32(REG_ALGO_DEBUG2, algo_debug2_val);
}

void MCF8316C_Read_MPET_Results(MCF8316C_MotorParams_t *params) {
	if(params == NULL) return;

	uint32_t mtr_params = MCF8316C_ReadReg32(REG_MTR_PARAMS);
	params->resistance_hex = (mtr_params >> 24) & 0xFF;
	params->bemf_const_hex = (mtr_params >> 16) & 0xFF;
	params->inductance_hex = (mtr_params >> 8) & 0xFF;

	uint32_t curr_pi = MCF8316C_ReadReg32(REG_CURRENT_PI);
	params->curr_loop_ki = (curr_pi >> 16) & 0xFFFF;
	params->curr_loop_kp = curr_pi & 0xFFFF;

	uint32_t spd_pi = MCF8316C_ReadReg32(REG_SPEED_PI);
	params->spd_loop_ki = (spd_pi >> 16) & 0xFFFF;
	params->spd_loop_kp = spd_pi & 0xFFFF;
}

void MCF8316C_Clear_Faults(void) {
	uint32_t val = MCF8316C_ReadReg32(REG_ALGO_CTRL1);
	// 29번 비트: CLR_FLT, 28번 비트: CLR_FLT_RETRY_COUNT
	val |= (1 << 29) | (1 << 28);
	MCF8316C_WriteReg32(REG_ALGO_CTRL1, val);
}

extern TIM_HandleTypeDef htim8;

void MCF8316C_Set_Speed(float speed_percent) {
	if (speed_percent < 0.0f) speed_percent = 0.0f;
	if (speed_percent > 100.0f) speed_percent = 100.0f;

	// MCF8316C의 내부 디지털 속도 지령은 15-bit 해상도를 사용합니다. (0 ~ 32767)
	// 100% = 32767
	uint32_t speed_cmd = (uint32_t)((speed_percent / 100.0f) * 32767.0f);

	// 1. ALGO_DEBUG1 (0xEC) 레지스터 읽기
	uint32_t algo_debug1 = MCF8316C_ReadReg32(REG_ALGO_DEBUG1);

	// 2. 기존 속도 지령 비트(30:16) 및 오버라이드 비트(31) 마스킹(초기화)
	algo_debug1 &= ~((0x7FFF << DIGITAL_SPEED_CTRL_BASE) | (1UL << OVERRIDE_BASE));

	// 3. I2C 속도 오버라이드 활성화 및 새로운 15-bit 속도 지령 삽입
	algo_debug1 |= OVERRIDE_DIGITAL_SPEED_CTRL;           // Bit 31 Set
	algo_debug1 |= (speed_cmd << DIGITAL_SPEED_CTRL_BASE); // Bit 30:16 Set

	// 4. 레지스터 업데이트
	MCF8316C_WriteReg32(REG_ALGO_DEBUG1, algo_debug1);
}
