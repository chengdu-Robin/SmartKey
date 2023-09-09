
#ifndef __ALARM_H__
#define __ALARM_H__

#include "common.h"

/* 错误状态 */
typedef enum
{
    NO_ERROR = 0x00,	// 无错误

	DC12V_OFF_BASE_VALUE, // 报警后需要关DC12V的故障基值
	DC12V_OV,
	// DC5521_OC,
	DC12V_OC,
	DC12V_SC,
	DC12V_OFF_TOP_VALUE,
	
	DC30V_OFF_BASE_VALUE,
	DC30V_OV,
	DC30V_OC,
	DC30V_SC,
	DC30V_OFF_TOP_VALUE,
	PV_OFF_BASE_VALUE,
	PV_IN_OV,
	PV_IN_UV,
	PV_IN_OC,
	PV_OUT_OV,
	PV_OUT_OC,
	PVDC_OT,
	PVDC_BREAK,
	PV_OFF_TOP_VALUE,

	PCS_RAF_ERR, // read ac freq error
	PCS_CAF_ERR, // change ac freq error
	PCS_WR_ERR, // write param error
	PCS_OFF_BASE_VALUE, // 报警后需要关PCS的故障基值
	PCS_SetCurr_ERR,
	PCS_ON_OFF_ERR,
	PCS8026_OC,
    PCS8026_SC,
    PCS8026_OutUV,
    PCS8026_BusOV,
    PCS8026_BusUV,
    PCS8026_ZoErr,
    PCS8026_OT,
    PCS8026_OutOV,
    PCS8026_InErr,
    PCS8026_PinOff,
    PCS8026_UartOff,
    PCS1615_SC,
    PCS1615_OC,
    PCS1615_BatUV,
    PCS1615_BatOV,
    PCS1615_OT,
    PCS1615_LVEN,
    PCS1615_HVEN,
    PCS1615_UartErr,
	PCS_BREAK,
	PCS_OFF_TOP_VALUE,
	/* 以下故障定义为报警后关机 */
	POWER_OFF_BASE_VALUE, // 报警后需要关机的故障基值
	BAT_SOC_LOW,
	BAT_DSG_OT,
	BAT_DSG_UT,
	BAT_VOL_UV,
	CELL_VOL_UV,
	BMS_BREAK,
	CHG_MOS_OT,
	DSG_MOS_OT,
	NTC_BREAK,
	BAT_VOL_OV,
	CELL_VOL_OV,
	CELL_EMPTY,
	BAT_CHG_OC,
	BAT_DSG_OC,
	BAT_DSG_SC,
	PreDSG_ERR,
	PreCHG_ERR,
	BMS_AFE_ERR,
	FLASH_ERR,
	HEAT_ERR,
	DSG_RUNAWAY,
	CHG_RUNAWAY,
	VOL_BREAK,
	CURR_BREAK,
	ILLEGAL_ACT,
	SET_ID_ERR,
	MOS_CTRL_FAIL,
	RELAY_CTRL_FAIL,
	ERR_NUM
}alarm_err_et;

/* 总报警时长定义，计算周期500mS */
typedef enum
{
	ALARM_10S = 20,
	ALARM_15S = 30,
	ALARM_30S = 60,
	ALARM_60S = 120,
	ALARM_120S = 240
}alert_time;

/* 报警频率，计算周期10mS，在buzzer里调用 */
typedef enum
{
	BEE_1HZ = 50,
	BEE_2HZ = 25,
	BEE_3HZ = 17
}beep_freq;

/* 错误结构体 */
typedef struct
{
	alarm_err_et state;		// 故障状态
	uint8_t priority;		// 优先级:0~255，由高到低
	bool beep_en;			// 是否使能蜂鸣器
	beep_freq bee_freq;		// 蜂鸣器报警频率（周期）
	uint8_t bee_times;		// 蜂鸣器报警次数
	alert_time alarm_time;	// 总报警时长
}alarm_info_st;


extern alarm_err_et ErrCode;
extern alarm_info_st err_source[ERR_NUM];

extern void Alarm_Info_Init(void);
extern void Output_Overload_Monitor(void);
// extern void Alarm_Source_Monitor(void);
extern void Alarm_Process(void);
extern uint32_t Get_Err_Symbol(void);


#endif
