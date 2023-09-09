/*!
    \file    app.c
    \brief   the app file
*/

#include "app.h" 
#include "lcd.h"
#include "buzzer.h"
#include "can_msg.h"
#include "pcs_msg.h"
// #include "STK3311.h"
#include "alarm.h"
#include "wifi.h"
#include "ad_calc.h"
#include "pack_ctrl.h"

/* 空载待机时间，单位秒 */
const int32_t StandbyTime[T_VAL_SUM] = T_VAL_LIST1;
const int32_t NoloadTime[T_VAL_SUM] = T_VAL_LIST2;
standby_param_st HomeStdby = {0, 0, 0, 0};

mc_state_et mc_state;
CfgInfo_st cfg_param = {0};
app_param_st app_param, app_param_shadow;
uint16_t WakeUp_source = NO_WKUP;
// static uint16_t dc12_power = 0;

/***********************************************************************************************************************
* Description  : Config the app parameter 
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void Config_Default_App_Param(app_param_st *pInfo)
{
	pInfo->button_beep = true;
	pInfo->lcd_stdby_md = T_MODE1;
	pInfo->home_stdby_md = T_MODE5;
	pInfo->usb_stdby_md = T_MODE1;
	pInfo->dc_stdby_md = T_MODE1;
	pInfo->ac_stdby_md = T_MODE1;
}

/***********************************************************************************************************************
* Description  : Print syetem parameter for debug
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/ 
void Print_System_Param(void)
{
    LOG_PRINT("PACK: %.1fV, %dmA, %d%%, %d℃, %d℃, %dmV, %dmV, Sta=0x%02x, Err1=0x%02x, Err2=0x%02x, Err3=0x%02x \n", 
    ((float)PackMsg[WorkPackNum].pack_vol/10), PackMsg[WorkPackNum].pack_curr*10, PackMsg[WorkPackNum].BmsSt.pack_soc, 
    PackMsg[WorkPackNum].BmsSt.pack_tem_max, PackMsg[WorkPackNum].BmsSt.pack_tem_min, PackMsg[WorkPackNum].cell_vol_max, PackMsg[WorkPackNum].cell_vol_min, 
	PackMsg[WorkPackNum].BmsSt.BmsSta.BmsStaByte, PackMsg[WorkPackNum].BmsSt.BmsErr1.ErrByte, PackMsg[WorkPackNum].BmsSt.BmsErr2.ErrByte, PackMsg[WorkPackNum].BmsSt.BmsErr3.ErrByte);

	LOG_PRINT("PCS: Vac=%dV, Pac=%dW, Pbat=%dW, %d℃, Err1=0x%02x, Err2=0x%02x, EnSta=%d, KeySta=%d, RelaySta=%d \n",
    pcs_msg.vac/10, pcs_msg.pac, pcs_msg.pbat, pcs_msg.tpcb, pcs_msg.err_8026, pcs_msg.err_1615, PCS_Switch_Action, PCS_KEY_STATE, AC_RELAY_STATE);

	LOG_PRINT("MC: SM=%02d, Wkup=0x%04x, WkA=%d, WkB=%d, RlyA=%d, RlyB=%d, WP_SN=%02d, ErrCode=%02d \n\n", 
	mc_state, WakeUp_source, WKUP_A_STATE, WKUP_B_STATE, RELAY_A_STATE, RELAY_B_STATE, WorkPackNum, ErrCode);
	//mcu_get_wifi_work_state()
}

int16_t FW_ver_conversion(void)
{
	int16_t ver_num = 0;
	uint8_t ascii_buff[] = MCU_VER;
	uint8_t ge,shi,bai;

	bai = (ascii_buff[0] & 0x0f);
	shi = (ascii_buff[2] & 0x0f);
	ge = (ascii_buff[4] & 0x0f);
	ver_num = bai*100 + shi*10 + ge;
	// Debug_Print("firmware version: %d \n", ver_num);
	return ver_num;
}

static int16_t get_dsg_power(void)
{
	if ((WakeUp_source & USB_WKUP) == 0)
	{
		PvdcMsg.DC30V_Power = 0;
	}
	if ((WakeUp_source & DC_WKUP) == 0)
	{
		PvdcMsg.DC12V_Power = 0;
	}
	if ((WakeUp_source & AC_WKUP) == 0)
	{
		pcs_msg.pac = 0;
	}

	return (PvdcMsg.DC12V_Power + PvdcMsg.DC30V_Power + pcs_msg.pac + LightMsg.light_power);
}

/***********************************************************************************************************************
 * @description: 计算剩余时间，单位：分钟
 * @param [None]
 * @return [RemainTime]:
 ***********************************************************************************************************************/
static uint16_t get_remain_time(void)
{
	uint16_t RemainTime = 0;
	uint32_t temp_value = 0;

	// if (PackMsg[WorkPackNum].BmsSt.BmsSta.BmsStaBit.charging == 1)
	// {
	// 	for (uint8_t i = 0; i < PACK_SUM; i++)
	// 	{
	// 		/* 剩余充电时长 */
	// 		temp_value += (uint32_t)(PackMsg[i].BmsSt.pack_AH * (100 - PackMsg[i].BmsSt.pack_soc));
	// 	}
	// }
	// else
	// {
		for (uint8_t i = 0; i < PACK_SUM; i++)
		{
			/* 剩余放电时长 */
			temp_value += (uint32_t)(PackMsg[i].BmsSt.pack_AH * PackMsg[i].BmsSt.pack_soc);
		}
	// }
	if (PackMsg[WorkPackNum].pack_curr >= 0)
	{
		RemainTime = 65535;
	}
	else
	{
		RemainTime = temp_value * 60 / abs(PackMsg[WorkPackNum].pack_curr);
	}
	// Debug_Print("temp_value=%d, RemainTime=%d \n", temp_value, RemainTime);
	return RemainTime;
}

/***********************************************************************************************************************
 * @description: 将剩余时间进行单位转换
 * @param [uint16_t] minutes: 需要转换的分钟数
 * @return [None]
 ***********************************************************************************************************************/
static int16_t trans_remain_time(uint16_t minutes)
{
	int16_t temp;

	/* 超过60分钟，以小时显示，放大10倍 */
	if (minutes > 59)
	{
		if (minutes > 5940)
		{
			temp = 990;	// Max display number: 99
		}
		else
		{
			temp = minutes/6;
		}
		Hours_Sign_On;
		Mins_Sign_Off;

		return temp;
	}
	/* 小于60分钟，以分钟显示，放大10倍 */
	else
	{
		Mins_Sign_On;
		Hours_Sign_Off;
		return (minutes*10);
	}
}

/***********************************************************************************************************************
 * @description: 配置APP参数
 * @param [None]
 * @return [None]
 ***********************************************************************************************************************/
void config_app_param(void)
{
	switch (cfg_param.PageNum)
	{
		case 0:
			Bottom_Show_Symbol(Sign_P, Num_0);
			Left_Show_Symbol(Blank, Sign_U, Sign_E, Sign_r);
			Right_Show_Num(FW_ver_conversion());
			break;

		case 1:
			Bottom_Show_Symbol(Sign_P, Num_1);
			Left_Show_Symbol(Sign_t, Sign_L, Sign_C, Sign_d);
			if (T_NoTime == app_param_shadow.lcd_stdby_md)
			{
				Right_Show_Symbol(Blank, Num_0, Sign_F, Sign_F);
			}
			else
			{
				Right_Show_Num(StandbyTime[app_param_shadow.lcd_stdby_md]);
			}
			break;

		case 2:
			Bottom_Show_Symbol(Sign_P, Num_2);
			Left_Show_Symbol(Sign_t, Sign_S, Sign_t, Sign_d);
			if (T_NoTime == app_param_shadow.home_stdby_md)
			{
				Right_Show_Symbol(Blank, Num_0, Sign_F, Sign_F);
			}
			else
			{
				Right_Show_Num(StandbyTime[app_param_shadow.home_stdby_md]);
			}
			break;

		case 3:
			Bottom_Show_Symbol(Sign_P, Num_3);
			Left_Show_Symbol(Sign_t, Sign_P, Sign_n, Sign_L);
			if (T_NoTime == app_param_shadow.dc_stdby_md)
			{
				Right_Show_Symbol(Blank, Num_0, Sign_F, Sign_F);
			}
			else
			{
				Right_Show_DecNum(NoloadTime[app_param_shadow.dc_stdby_md] / 360);
			}
			break;

		// case 4:
		// 	Bottom_Show_Symbol(Sign_P, Num_4);
		// 	Left_Show_Symbol(Sign_A, Sign_C, Sign_n, Sign_L);
		// 	if (T_NoTime == app_param_shadow.ac_stdby_md)
		// 	{
		// 		Right_Show_Symbol(Blank, Num_0, Sign_F, Sign_F);
		// 	}
		// 	else
		// 	{
		// 		Right_Show_DecNum(NoloadTime[app_param_shadow.ac_stdby_md] / 360);
		// 	}
		// 	break;

		// case 4:
		// 	Bottom_Show_Symbol(Sign_P, Num_4);
		// 	Left_Show_Symbol(Sign_A, Sign_C, Sign_o, Sign_L);
		// 	Right_Show_Symbol(Blank, Num_0, Sign_F, Sign_F);
		// 	break;

		case 4:
			Bottom_Show_Symbol(Sign_P, Num_4);
			Left_Show_Symbol(Sign_b, Sign_E, Sign_E, Sign_P);
			if (true == app_param_shadow.button_beep)
			{
				Right_Show_Symbol(Blank, Num_0, Sign_N, Blank);
			}
			else
			{
				Right_Show_Symbol(Blank, Num_0, Sign_F, Sign_F);
			}
			break;

		case 5:
			Bottom_Show_Symbol(Sign_P, Num_5);
			Left_Show_Symbol(Sign_C, Sign_S, Sign_o, Sign_C);
			Right_Show_Num(Custom_Full_SOC);
			break;
		
		default:
			break;
	}
	cfg_param.time_count++;
	// Debug_Print("cfg_time_count=%d\n", cfg_param.time_count);
	if (cfg_param.time_count > 20)
	{
		cfg_param.Exit_CfgMode = true;
	}
	if (true == cfg_param.Exit_CfgMode)
	{
		memset(&cfg_param, 0, sizeof(cfg_param));
		/* 参数有变化，存储到Flash */
		if ((memcmp(&app_param, &app_param_shadow, sizeof(app_param_st)) != 0))
		{
			Debug_Print("App param changes, write to falsh.\n");
			app_param_shadow.ac_stdby_md = app_param_shadow.dc_stdby_md;
			app_param_shadow.usb_stdby_md = app_param_shadow.dc_stdby_md;
			Write_AppParam(&app_param_shadow, sizeof(app_param_st));
		}
		Watt_Sign_On;
		Gear_Sign_Off;
	}
}

/***********************************************************************************************************************
 * @description: 数字符号显示内容
 * @param [None]
 * @return [None]
 ***********************************************************************************************************************/
static void digit_symbol_show(void)
{
	/* 常规显示 */
	Top_Show_Num(PackMsg[WorkPackNum].BmsSt.pack_soc);
	if (cfg_param.cfg_mode == false)
	{
		if (PackMsg[WorkPackNum].BmsSt.BmsSta.BmsStaBit.pre_dsg_fail == 1)
		{
			Bottom_Show_Symbol(Sign_d, Sign_d);
		}
		else
		{
			Bottom_Show_Time(trans_remain_time(get_remain_time()));
		}

		uint32_t temp_symbol = Get_Err_Symbol();
		
		if (0 == temp_symbol)
		{
			int16_t temp;

			temp = (int16_t)(PackMsg[WorkPackNum].pack_vol * PackMsg[WorkPackNum].pack_curr / 1000);
			if (temp < 0)
			{
				temp = 0;
			}
			Left_Show_Num(temp);
			// Left_Show_Num(123);
		}
		else
		{
			Left_Show_Symbol((uint8_t)(temp_symbol>>24), (uint8_t)(temp_symbol>>16), (uint8_t)(temp_symbol>>8), (uint8_t)temp_symbol);
		}
		
		Right_Show_Num(get_dsg_power());
		// Right_Show_Num(4567);
		// Right_Show_Num(pvdc_param.pvdc_tem);
	}
	/* 配置模式显示 */
	else
	{
		config_app_param();
	}
	
	if ((WakeUp_source & CHG_AC_WKUP) != 0 && (WakeUp_source & AC_WKUP) != 0)
	{
		UPS_Sign_On;
	}
	else
	{
		UPS_Sign_Off;
	}
	// Top_Show_Num(169);
	// Bottom_Show_Num(96);
	// Left_Show_Num(1234);
	// Right_Show_Num(5678);
	// Left_Show_Symbol(Sign_A, Sign_b, Sign_C, Sign_d);
}

/***********************************************************************************************************************
* Description  : bat bar show charging or soc
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
static void bat_bar_show(void)
{
	uint8_t BaseBar;
	static uint8_t time = 0, FlowBar = 0;

	if (ErrCode != BAT_SOC_LOW && ErrCode != CELL_VOL_UV && ErrCode != BAT_VOL_UV)
	{
		BaseBar = (PackMsg[PackA].BmsSt.pack_soc + 4)/10;
		if (PackMsg[PackA].BmsSt.BmsSta.BmsStaBit.charging)
		{
			if (time++ > 1)	// flowing rate
			{
				time = 0;
				FlowBar++;
				if ((BaseBar + FlowBar) > 10)
				{
					FlowBar = 0;
				}
			}
		}
		else
		{
			time = 0;
			FlowBar = 0;
		}
		// Bat_bar_show_num(BaseBar + FlowBar);
		if (true == PackMsg[PackA].PackSta.plug_in)
		{
			Bat_bar_show_num(BaseBar + FlowBar);
		}
		else
		{
			BAT_Bar_All_Off;
		}
	}
}

/***********************************************************************************************************************
 * @description: 风机图标显示
 * @param [None]
 * @return [None]
 ***********************************************************************************************************************/
static void fan_icon_show(void)
{
	// uint8_t Timer = 0;
	static uint8_t ii = 0, jj = 0;

	if (0 == Get_Fan_PWM())
	{
		ii = 0;
		jj = 0;
		Fan_Sign_All_Off;
	}
	else
	{
		ii++;
		if (ii % 5 == 0)
		{
			switch (jj)
			{
				case 0:
					Fan_Up_Sign_On;
					Fan_Right_Sign_On;
					Fan_Down_Sign_On;
					Fan_Left_Sign_Off;
					break;
				case 1:
					Fan_Up_Sign_Off;
					Fan_Right_Sign_On;
					Fan_Down_Sign_On;
					Fan_Left_Sign_On;
					break;

				case 2:
					Fan_Up_Sign_On;
					Fan_Right_Sign_Off;
					Fan_Down_Sign_On;
					Fan_Left_Sign_On;
					break;

				case 3:
					Fan_Up_Sign_On;
					Fan_Right_Sign_On;
					Fan_Down_Sign_Off;
					Fan_Left_Sign_On;
					break;
				
				default:
					break;
			}
			jj++;
			if (jj >= 4)
			{
				ii = 0;
				jj = 0;
			}
			
		}
	}
}

/***********************************************************************************************************************
 * @description: 显示信息，置于10mS周期中
 * @param [None]
 * @return [None]
 ***********************************************************************************************************************/
void Display_Info(void)
{
	static uint8_t counter = 0;

	if (mc_state != MC_INIT)
	{
		counter++;
		/* 100mS更新一次 */
		if (counter%10 == 0)
		{
			/* code */
			bat_bar_show();
		}
		/* 1S更新一次 */
		if (counter >= 100)
		{
			counter = 0;
			digit_symbol_show();
		}
		fan_icon_show();
	}
	LCD_refresh();
}

bool Wifi_Heart_Monitor(FlagStatus state)
{
	static uint8_t secend = 0;

	if (state == SET)
	{
		if (secend++>25) 
		{
			secend = 0;
			wifi_work_state = WIFI_SATE_UNKNOW;
			return false;
		}
	}
	else
	{
		secend = 0;
	}
	return true;
}

void Wifi_LED_Monitor(void)
{
	static uint8_t count = 0;

	switch(mcu_get_wifi_work_state())
	{
		case SMART_CONFIG_STATE:
		case SMART_AND_AP_STATE:
			/* 处于 Smart 配置状态，LED快闪：250mS */
			if (count++ >= 5)
			{
				Wifi_Sign_Tog;
				count = 0;
			}
		break;

		case AP_STATE:
			/* 处于 AP 配置状态，LED慢闪：1500mS */
			if (count++ >= 30)
			{
				Wifi_Sign_Tog;
				count = 0;
			}
		break;

		case WIFI_NOT_CONNECTED:
		case WIFI_LOW_POWER:
			/* Wi-Fi 配置完成，正在连接路由器，LED常暗 */
			Wifi_Sign_Off;
		break;

		case WIFI_CONNECTED:
		case WIFI_CONN_CLOUD:
			/* 路由器连接成功，LED常亮 */
			Wifi_Sign_On;
		break;

		default:
			Wifi_Sign_Off;
		break;
	}
	// Debug_Print("wifi_work_state: %d\n", mcu_get_wifi_work_state());
}

void Upload_All_DP(void)
{
	static struct 
	{
		uint16_t soc[2];
		uint16_t re_min[2];
		uint16_t battery_info[2][3];
		bool usb_state[2];
		bool ac_out_state[2];
		bool dc_out_state[2];
		light_mode_et light_state[2];
	}dp_param;

	// dp_param.soc[0] = bms_param.pack_soc/10;
	// dp_param.re_min[0] = ((bms_param.pack_remain_time>5940)?5940:bms_param.pack_remain_time);
	// dp_param.battery_info[0][0] = bms_param.pack_vol;
	// dp_param.battery_info[0][1] = ((bms_param.pack_curr<0)?-(bms_param.pack_curr/10):0);
	// dp_param.battery_info[0][2] = (bms_param.pack_vol*dp_param.battery_info[0][1])/10;
	// dp_param.usb_state[0] = pvdc_state.dc30V_living;
	dp_param.ac_out_state[0] = pcs_msg.pcs_state.pcs_working;
	// dp_param.dc_out_state[0] = pvdc_state.dc12V_living;
	dp_param.light_state[0] = LightMsg.lighting_mode;
	/* upload soc */ 
	if (dp_param.soc[1] != dp_param.soc[0])
	{
		dp_param.soc[1] = dp_param.soc[0];
		mcu_dp_value_update(DPID_BATTERY_PERCENTAGE, dp_param.soc[1]);
	}
	/* upload remain_time */ 
	if (dp_param.re_min[1] != dp_param.re_min[0])
	{
		dp_param.re_min[1] = dp_param.re_min[0];
		mcu_dp_value_update(DPID_REMAIN_TIME, dp_param.re_min[1]);
	}
	/* upload battery parameters */ 
	if (dp_param.battery_info[1][0] != dp_param.battery_info[0][0]
	 || dp_param.battery_info[1][1] != dp_param.battery_info[0][1]
	 || dp_param.battery_info[1][2] != dp_param.battery_info[0][2])
	{
		uint8_t bat_info_buff[8] = {0};

		dp_param.battery_info[1][0] = dp_param.battery_info[0][0];
		dp_param.battery_info[1][1] = dp_param.battery_info[0][1];
		dp_param.battery_info[1][2] = dp_param.battery_info[0][2];

		bat_info_buff[0] = (uint8_t)(dp_param.battery_info[1][0] >> 8);
        bat_info_buff[1] = (uint8_t)(dp_param.battery_info[1][0] & 0xff);
        bat_info_buff[2] = (uint8_t)(dp_param.battery_info[1][1] >> 8);
        bat_info_buff[3] = (uint8_t)(dp_param.battery_info[1][1] & 0xff);
        bat_info_buff[4] = (uint8_t)(dp_param.battery_info[1][2] >> 8);
        bat_info_buff[5] = (uint8_t)(dp_param.battery_info[1][2] & 0xff);
        bat_info_buff[6] = (uint8_t)(RATED_OUT_POWER >> 8);
        bat_info_buff[7] = (uint8_t)(RATED_OUT_POWER & 0xff);

		mcu_dp_raw_update(DPID_BATTERY_PARAMETERS, bat_info_buff, 8);
	}
	/* upload usb state */ 
	if (dp_param.usb_state[1] != dp_param.usb_state[0])
	{
		uint8_t usb_state_buff[7] = {0};

		dp_param.usb_state[1] = dp_param.usb_state[0];
        usb_state_buff[0] = dp_param.usb_state[1];
		for (uint8_t i = 1; i < 7; i++)
		{
			usb_state_buff[i] = 0xff;
		}
		mcu_dp_raw_update(DPID_USB_MANAGEMENT, usb_state_buff, 7);
	}
	/* upload ac state */ 
	if (dp_param.ac_out_state[1] != dp_param.ac_out_state[0])
	{
		dp_param.ac_out_state[1] = dp_param.ac_out_state[0];
		mcu_dp_bool_update(DPID_SWITCH_1, dp_param.ac_out_state[1]);
	}
	/* upload dc state */ 
	if (dp_param.dc_out_state[1] != dp_param.dc_out_state[0])
	{
		dp_param.dc_out_state[1] = dp_param.dc_out_state[0];
		mcu_dp_enum_update(DPID_CAR_PORT_CONTROL, dp_param.dc_out_state[1]);
	}
	/* upload light mode */ 
	if (dp_param.light_state[1] != dp_param.light_state[0])
	{
		dp_param.light_state[1] = dp_param.light_state[0];
		mcu_dp_enum_update(DPID_LIGHT_MODE, dp_param.light_state[1]);
	}
}

/***********************************************************************************************************************
 * @description: 熄屏监测
 * @param [None]
 * @return [None]
 ***********************************************************************************************************************/
void Screen_Sleep_Monitor(void)
{
	static uint16_t off_timer = 0;

	/* 判断是否有事件复位熄屏计时 */
	if(true == lcd_param.reset_off_timer)
	{
		off_timer = 0;
		if(false == lcd_param.screen_en_sta)
		{
			LCD_module_on();
		}
		lcd_param.reset_off_timer = false;
	}
	
	if(true == lcd_param.screen_en_sta)// && ErrCode == NO_ERROR)
	{
		if(T_NoTime != app_param.lcd_stdby_md && cfg_param.cfg_mode == false)
		{
			off_timer++;
			if(off_timer > StandbyTime[app_param.lcd_stdby_md])
			{
				/* 如果有遮挡或有报警，不再熄屏 */
				if(/*RESET == PS_FLAG ||*/ ErrCode == NO_ERROR)
				{
					LCD_module_off();
				}
				off_timer = 0;
			}
		}
	}
	else
	{
		off_timer = 0;
	}
}

/***********************************************************************************************************************
 * @description: Home键待机监测
 * @param [None]
 * @return [None]
 ***********************************************************************************************************************/
void Home_Standby_Monitor(void)
{
	if (true == HomeStdby.reset)
	{
		HomeStdby.time_count = 0;
		HomeStdby.reset = false;
	}
	if (true == HomeStdby.start)
	{
		if(T_NoTime != app_param.home_stdby_md && cfg_param.cfg_mode == false)
		{
			HomeStdby.time_count++;
			if (HomeStdby.time_count > StandbyTime[app_param.home_stdby_md])
			{
				HomeStdby.time_count = 0;
				HomeStdby.time_out = true;
			}
		}
	}
}

/***********************************************************************************************************************
 * @description: 端口待机监测
 * @param [None]
 * @return [None]
 ***********************************************************************************************************************/
void Port_Standby_Monitor(void)
{
	static uint8_t full_chagre_wait = 0;
	static uint8_t usb_delay_count = 0, dc_delay_count = 0, ac_delay_count;
	static uint32_t usb_noload_count = 0, dc_noload_count = 0, ac_noload_count = 0;

	if (cfg_param.cfg_mode == false)
	{
		/* USB端口监测 */
		if ((WakeUp_source & USB_WKUP) != 0 && app_param.usb_stdby_md != T_NoTime)
		{
			if (PvdcMsg.DC30V_Power >= 1) // 1W以下作为空载判断
			{
				if (usb_delay_count++ > 4)
				{
					usb_delay_count = 0;
					usb_noload_count = 0;
				}
			}
			else
			{
				usb_delay_count = 0;
				usb_noload_count++;
				if (usb_noload_count > NoloadTime[app_param.usb_stdby_md])
				{
					usb_noload_count = 0;
					WakeUp_source &= (~USB_WKUP);
					USB_Sign_Off;
					TypeC_Sign_Off;
					// DC30V_OFF;
					CAN_Ctrl_PVDC(CMD_SW_DC30V, 0);
					Debug_Print("Disable USB Output.\n");
				}
			}
		}
		else
		{
			usb_delay_count = 0;
			usb_noload_count = 0;
		}
		/* DC端口监测 */
		if ((WakeUp_source & DC_WKUP) != 0 && app_param.dc_stdby_md != T_NoTime)
		{
			if (PvdcMsg.DC12V_Power >= 1) // 1W以下作为空载判断
			{
				if (dc_delay_count++ > 4)
				{
					dc_delay_count = 0;
					dc_noload_count = 0;
				}
			}
			else
			{
				dc_delay_count = 0;
				dc_noload_count++;
				if (dc_noload_count > NoloadTime[app_param.dc_stdby_md])
				{
					dc_noload_count = 0;
					WakeUp_source &= (~DC_WKUP);
					DC_Sign_Off;
					DC5521_Sign_Off;
					// DC12V_OFF;
					CAN_Ctrl_PVDC(CMD_SW_DC12V, 0);
					Debug_Print("Disable DC12V Output.\n");
				}
			}
		}
		else
		{
			dc_delay_count = 0;
			dc_noload_count = 0;
		}
		/* AC端口监测 */
		if ((WakeUp_source & AC_WKUP) != 0 && app_param.ac_stdby_md != T_NoTime)
		{
			if (pcs_msg.pac >= 5) // 5W以下作为空载判断
			{
				if (ac_delay_count++ > 4)
				{
					ac_delay_count = 0;
					ac_noload_count = 0;
				}
			}
			else
			{
				ac_delay_count = 0;
				ac_noload_count++;
				if (ac_noload_count > NoloadTime[app_param.ac_stdby_md])
				{
					ac_noload_count = 0;
					WakeUp_source &= (~AC_WKUP);
					DE_AC_RELAY;
					if ((WakeUp_source & CHG_AC_WKUP) == 0)
					{
						PCS_Param_Clear(&pcs_msg);
					}
					AC_Sign_Off;
					Debug_Print("Disable AC Output.\n");
				}
			}
		}
		else
		{
			ac_delay_count = 0;
			ac_noload_count = 0;
		}
		/* 充电端口 */
		if (WakeUp_source == CHG_AC_WKUP || WakeUp_source == CHG_PV_WKUP || WakeUp_source == (CHG_AC_WKUP + CHG_PV_WKUP))
		{
			if (true == PackMsg[WorkPackNum].PackSta.full_charge)
			{
				// uint8_t i = 0, j = 0;
				// for (i = 0; i < PACK_SUM; i++)
				// {
				// 	if (true == PackMsg[i].PackSta.full_charge)
				// 	{
				// 		j++;
				// 	}
				// }
				// /* 所有包都是满电 */
				// if (j == PACK_SUM)
				// {
				// 	Debug_Print("All packs are full chagre, ready to shutdown.\n");
				// 	mc_state = MC_SLEEP;
				// }
				full_chagre_wait++;
				if (full_chagre_wait > 60) // 1 min
				{
					Debug_Print("All packs are full chagre, ready to shutdown.\n");
					mc_state = MC_SLEEP;
				}
			}
			else
			{
				full_chagre_wait = 0;
			}
		}
		else
		{
			full_chagre_wait = 0;
		}
	}
	// Debug_Print("dc_delay_count=%d, dc_noload_count=%d, ac_delay_count=%d, ac_noload_count=%d\n",
	// 			 dc_delay_count, dc_noload_count, ac_delay_count, ac_noload_count);
}

/***********************************************************************************************************************
 * @description: 上电自检
 * @param [None]
 * @return [None]
 ***********************************************************************************************************************/
static bool power_on_check(void)
{
	return true;
}

/***********************************************************************************************************************
 * @description: 主状态机轮询，置于10mS周期中
 * @param [None]
 * @return [None]
 ***********************************************************************************************************************/
void Main_FSM_Monitor(void)
{
	// Debug_Print("mc_state = %d \n", mc_state);
	switch (mc_state)
	{
		case MC_INIT:
			if (LCD_boot_screen(100) == true)
			{
				if (true == power_on_check())
				{
					Watt_Sign_On;
					digit_symbol_show();    // 为保持同步显示，刷新一次
					mc_state = MC_STANDBY;
				}
				else
				{
					mc_state = MC_ALARM;
					Debug_Print("Fault: Self Check Failed! T_T \n");
				}
			}
		break;
		/*************************************** Standby Mode ***************************************/
		case MC_STANDBY:
		{
			// static uint16_t standby_count = 0, count_1s = 0;
			if(WakeUp_source == NO_WKUP)
			{
				HomeStdby.start = true;
				if (true == HomeStdby.time_out)
				{
					memset(&HomeStdby, 0, sizeof(HomeStdby));
					mc_state = MC_SLEEP;
				}
			}
			else
			{
				memset(&HomeStdby, 0, sizeof(HomeStdby));
				mc_state = MC_EXEC;
			}
			if(NO_ERROR != ErrCode)
			{
				memset(&HomeStdby, 0, sizeof(HomeStdby));
				mc_state = MC_ALARM;
			}
		}
		break;
		/*************************************** Execute Mode ***************************************/
		case MC_EXEC:
			if (NO_ERROR != ErrCode)
			{
				mc_state = MC_ALARM;
			}
			else if (WakeUp_source == NO_WKUP)
			{
				mc_state = MC_STANDBY;
			}
			Output_Overload_Monitor();
		break;
		/*************************************** Alarm Mode ***************************************/
		case MC_ALARM:
		{
			static uint8_t count = 0;

			if (count++ >= 50)
			{
				count = 0;
				Alarm_Process();
			}
			// if(NO_ERROR == ErrCode)
			// {
			// 	mc_state = MC_EXEC;
			// 	WakeUp_source &= (~ALARM_WKUP);
			// }
		}
		break;
		/*************************************** Sleep Mode ***************************************/
		case MC_SLEEP: 
			MCU_To_Sleep();
		break;
		
		default: break;
	}
}
