#ifndef __LCD_H__
#define __LCD_H__

#include "common.h"
#include "HT1621.h"

#define LCD_BL_ON(pwm)	timer_channel_output_pulse_value_config(TIMER0,TIMER_CH_1,(pwm));
#define LCD_BL_OFF		timer_channel_output_pulse_value_config(TIMER0,TIMER_CH_1,0);

#define	SEG_LEN		    (SEG_NUM/2)
#define CLEAR_NUM       (uint8_t)0x08 // 清除8字形的每个段
typedef enum
{
	Blank = 0x00,
    Num_0 = 0xF5,Num_1 = 0x05,Num_2 = 0xD3,Num_3 = 0x97,Num_4 = 0x27,Num_5 = 0xB6,Num_6 = 0xF6,Num_7 = 0x15,Num_8 = 0xF7,Num_9 = 0xB7,
	Sign_A = 0x77,Sign_b = 0xE6,Sign_C = 0xF0,Sign_d = 0xC7,Sign_E = 0xF2,Sign_F = 0x72,Sign_H = 0x67,Sign_J = 0x85,Sign_L = 0xE0,
	Sign_n = 0x46,Sign_N = 0x75,Sign_o = 0xC6,Sign_P = 0x73,Sign_q = 0x37,Sign_r = 0x42,Sign_S = 0xB6,Sign_t = 0xE2,Sign_U = 0xE5,
	Sign_Minus = 0x02,Sign_UnderLine = 0x80
}lcd_symbol_et;

#define		LCD_Border_On		lcd_seg[10] |= (1<<0)
#define		LCD_Border_Off		lcd_seg[10] &= ~(1<<0)
#define		LCD_Border_Tog		lcd_seg[10] ^= (1<<0)
#define		Input_Sign_On		lcd_seg[15] |= (1<<4)
#define		Input_Sign_Off		lcd_seg[15] &= ~(1<<4)
#define		Input_Sign_Tog		lcd_seg[15] ^= (1<<4)
#define		Output_Sign_On		lcd_seg[6] |= (1<<4)
#define		Output_Sign_Off		lcd_seg[6] &= ~(1<<4)
#define		Output_Sign_Tog		lcd_seg[6] ^= (1<<4)
#define		Watt_Sign_On		Input_Sign_On; Output_Sign_On
#define		Watt_Sign_Off		Input_Sign_Off; Output_Sign_Off

#define		Wifi_Sign_On		lcd_seg[15] |= (1<<0)
#define		Wifi_Sign_Off		lcd_seg[15] &= ~(1<<0)
#define		Wifi_Sign_Tog		lcd_seg[15] ^= (1<<0)
#define		BLE_Sign_On		    lcd_seg[15] |= (1<<1)
#define		BLE_Sign_Off		lcd_seg[15] &= ~(1<<1)
#define		BLE_Sign_Tog		lcd_seg[15] ^= (1<<1)
#define		Sound_Sign_On		lcd_seg[15] |= (1<<2)
#define		Sound_Sign_Off		lcd_seg[15] &= ~(1<<2)
#define		Sound_Sign_Tog		lcd_seg[15] ^= (1<<2)
#define		Gear_Sign_On		lcd_seg[15] |= (1<<5)
#define		Gear_Sign_Off		lcd_seg[15] &= ~(1<<5)
#define		Gear_Sign_Tog		lcd_seg[15] ^= (1<<5)
#define		Loop_Sign_On		lcd_seg[15] |= (1<<6)
#define		Loop_Sign_Off		lcd_seg[15] &= ~(1<<6)
#define		Loop_Sign_Tog		lcd_seg[15] ^= (1<<6)
#define		Pack_Sign_On		lcd_seg[15] |= (1<<7)
#define		Pack_Sign_Off		lcd_seg[15] &= ~(1<<7)
#define		Pack_Sign_Tog		lcd_seg[15] ^= (1<<7)

#define		Fan_Up_Sign_On	    lcd_seg[6] |= (1<<3)
#define		Fan_Up_Sign_Off	    lcd_seg[6] &= ~(1<<3)
#define		Fan_Up_Sign_Tog	    lcd_seg[6] ^= (1<<3)
#define		Fan_Down_Sign_On	lcd_seg[7] |= (1<<4)
#define		Fan_Down_Sign_Off	lcd_seg[7] &= ~(1<<4)
#define		Fan_Down_Sign_Tog	lcd_seg[7] ^= (1<<4)
#define		Fan_Left_Sign_On	lcd_seg[7] |= (1<<5)
#define		Fan_Left_Sign_Off	lcd_seg[7] &= ~(1<<5)
#define		Fan_Left_Sign_Tog	lcd_seg[7] ^= (1<<5)
#define		Fan_Right_Sign_On	lcd_seg[6] |= (1<<2)
#define		Fan_Right_Sign_Off	lcd_seg[6] &= ~(1<<2)
#define		Fan_Right_Sign_Tog	lcd_seg[6] ^= (1<<2)
#define		Fan_Sign_All_Off	Fan_Up_Sign_Off;Fan_Down_Sign_Off;Fan_Left_Sign_Off;Fan_Right_Sign_Off
#define		Hours_Sign_On	    lcd_seg[7] |= (1<<6)
#define		Hours_Sign_Off	    lcd_seg[7] &= ~(1<<6)
#define		Hours_Sign_Tog	    lcd_seg[7] ^= (1<<6)
#define		Mins_Sign_On	    lcd_seg[9] |= (1<<3)
#define		Mins_Sign_Off	    lcd_seg[9] &= ~(1<<3)
#define		Mins_Sign_Tog	    lcd_seg[9] ^= (1<<3)

#define		BAT_Sign_On		    lcd_seg[6] |= (1<<5)
#define		BAT_Sign_Off		lcd_seg[6] &= ~(1<<5)
#define		BAT_Sign_Tog		lcd_seg[6] ^= (1<<5)
#define		Hot_Sign_On		    lcd_seg[6] |= (1<<6)
#define		Hot_Sign_Off		lcd_seg[6] &= ~(1<<6)
#define		Hot_Sign_Tog		lcd_seg[6] ^= (1<<6)
#define		Cold_Sign_On		lcd_seg[6] |= (1<<7)
#define		Cold_Sign_Off		lcd_seg[6] &= ~(1<<7)
#define		Cold_Sign_Tog		lcd_seg[6] ^= (1<<7)
#define		Warn_Sign_On		lcd_seg[6] |= (1<<0)
#define		Warn_Sign_Off		lcd_seg[6] &= ~(1<<0)
#define		Warn_Sign_Tog		lcd_seg[6] ^= (1<<0)
#define		Overload_Sign_On	lcd_seg[5] |= (1<<3)
#define		Overload_Sign_Off	lcd_seg[5] &= ~(1<<3)
#define		Overload_Sign_Tog	lcd_seg[5] ^= (1<<3)

#define		BAT_Bar_Border_On	lcd_seg[2] |= (1<<3)
#define		BAT_Bar_Border_Off	lcd_seg[2] &= ~(1<<3)
#define		BAT_Bar_Border_Tog	lcd_seg[2] ^= (1<<3)
#define		BAT_Bar_No1_On	    lcd_seg[14] |= (1<<3)
#define		BAT_Bar_No1_Off	    lcd_seg[14] &= ~(1<<3)
#define		BAT_Bar_No1_Tog	    lcd_seg[14] ^= (1<<3)
#define		BAT_Bar_No2_On	    lcd_seg[13] |= (1<<3)
#define		BAT_Bar_No2_Off	    lcd_seg[13] &= ~(1<<3)
#define		BAT_Bar_No2_Tog	    lcd_seg[13] ^= (1<<3)
#define		BAT_Bar_No3_On	    lcd_seg[12] |= (1<<3)
#define		BAT_Bar_No3_Off	    lcd_seg[12] &= ~(1<<3)
#define		BAT_Bar_No3_Tog	    lcd_seg[12] ^= (1<<3)
#define		BAT_Bar_No4_On	    lcd_seg[11] |= (1<<3)
#define		BAT_Bar_No4_Off	    lcd_seg[11] &= ~(1<<3)
#define		BAT_Bar_No4_Tog	    lcd_seg[11] ^= (1<<3)
#define		BAT_Bar_No5_On	    lcd_seg[0] |= (1<<3)
#define		BAT_Bar_No5_Off	    lcd_seg[0] &= ~(1<<3)
#define		BAT_Bar_No5_Tog	    lcd_seg[0] ^= (1<<3)
#define		BAT_Bar_No6_On	    lcd_seg[1] |= (1<<3)
#define		BAT_Bar_No6_Off	    lcd_seg[1] &= ~(1<<3)
#define		BAT_Bar_No6_Tog	    lcd_seg[1] ^= (1<<3)
#define		BAT_Bar_No7_On	    lcd_seg[7] |= (1<<3)
#define		BAT_Bar_No7_Off	    lcd_seg[7] &= ~(1<<3)
#define		BAT_Bar_No7_Tog	    lcd_seg[7] ^= (1<<3)
#define		BAT_Bar_No8_On	    lcd_seg[7] |= (1<<7)
#define		BAT_Bar_No8_Off	    lcd_seg[7] &= ~(1<<7)
#define		BAT_Bar_No8_Tog	    lcd_seg[7] ^= (1<<7)
#define		BAT_Bar_No9_On	    lcd_seg[4] |= (1<<3)
#define		BAT_Bar_No9_Off	    lcd_seg[4] &= ~(1<<3)
#define		BAT_Bar_No9_Tog	    lcd_seg[4] ^= (1<<3)
#define		BAT_Bar_No10_On	    lcd_seg[3] |= (1<<3)
#define		BAT_Bar_No10_Off	lcd_seg[3] &= ~(1<<3)
#define		BAT_Bar_No10_Tog	lcd_seg[3] ^= (1<<3)
#define		BAT_Bar_All_Off 	BAT_Bar_No1_Off;BAT_Bar_No2_Off;BAT_Bar_No3_Off;BAT_Bar_No4_Off;BAT_Bar_No5_Off;\
								BAT_Bar_No6_Off;BAT_Bar_No7_Off;BAT_Bar_No8_Off;BAT_Bar_No9_Off;BAT_Bar_No10_Off

#define		AC_CHG_Sign_On	    lcd_seg[10] |= (1<<4)
#define		AC_CHG_Sign_Off	    lcd_seg[10] &= ~(1<<4)
#define		AC_CHG_Sign_Tog	    lcd_seg[10] ^= (1<<4)
#define		DC_CHG_Sign_On	    lcd_seg[10] |= (1<<5)
#define		DC_CHG_Sign_Off	    lcd_seg[10] &= ~(1<<5)
#define		DC_CHG_Sign_Tog	    lcd_seg[10] ^= (1<<5)
#define		Thunder1_Sign_On	lcd_seg[10] |= (1<<6); lcd_seg[10] &= ~(1<<7)
#define		Thunder1_Sign_Off	lcd_seg[10] &= ~(1<<6); lcd_seg[10] &= ~(1<<7)
#define		Thunder1_Sign_Tog	lcd_seg[10] ^= (1<<6); lcd_seg[10] &= ~(1<<7)
#define		Thunder2_Sign_On	lcd_seg[10] |= (1<<6); lcd_seg[10] |= (1<<7)
#define		Thunder2_Sign_Off	lcd_seg[10] &= ~(1<<6); lcd_seg[10] &= ~(1<<7)
#define		Thunder2_Sign_Tog	lcd_seg[10] ^= (1<<6); lcd_seg[10] ^= (1<<7)
#define		UPS_Sign_On	        lcd_seg[6] |= (1<<1)
#define		UPS_Sign_Off	    lcd_seg[6] &= ~(1<<1)
#define		UPS_Sign_Tog	    lcd_seg[6] ^= (1<<1)

#define		DC5521_Sign_On	    lcd_seg[10] |= (1<<3)
#define		DC5521_Sign_Off	    lcd_seg[10] &= ~(1<<3)
#define		DC5521_Sign_Tog	    lcd_seg[10] ^= (1<<3)
#define		DC_Sign_On	        lcd_seg[10] |= (1<<2)
#define		DC_Sign_Off	        lcd_seg[10] &= ~(1<<2)
#define		DC_Sign_Tog	        lcd_seg[10] ^= (1<<2)
#define		USB_Sign_On	        lcd_seg[10] |= (1<<1)
#define		USB_Sign_Off	    lcd_seg[10] &= ~(1<<1)
#define		USB_Sign_Tog	    lcd_seg[10] ^= (1<<1)
#define		TypeC_Sign_On	    lcd_seg[7] |= (1<<0)
#define		TypeC_Sign_Off	    lcd_seg[7] &= ~(1<<0)
#define		TypeC_Sign_Tog	    lcd_seg[7] ^= (1<<0)
#define		AC50Hz_Sign_On	    lcd_seg[7] |= (1<<1);lcd_seg[7] &= ~(1<<2)
#define		AC50Hz_Sign_Tog	    lcd_seg[7] ^= (1<<1);lcd_seg[7] &= ~(1<<2)
#define		AC60Hz_Sign_On	    lcd_seg[7] |= (1<<1); lcd_seg[7] |= (1<<2)
#define		AC_Sign_Off	    	lcd_seg[7] &= ~(1<<1); lcd_seg[7] &= ~(1<<2)
#define		ACHZ_Sign_Tog		lcd_seg[7] |= (1<<1);lcd_seg[7] ^= (1<<2)

#define		Digit_Point_On	    lcd_seg[8] |= (1<<3)
#define		Digit_Point_Off	    lcd_seg[8] &= ~(1<<3)

#define		Digit_Num1_On		lcd_seg[15] |= (1<<3)
#define		Digit_Num1_Off		lcd_seg[15] &= ~(1<<3)

#define		Digit_Num8_1		lcd_seg[0]
#define		Digit_Num8_2		lcd_seg[1]
#define		Digit_Num8_3		lcd_seg[11]
#define		Digit_Num8_4		lcd_seg[12]
#define		Digit_Num8_5		lcd_seg[13]
#define		Digit_Num8_6		lcd_seg[14]
#define		Digit_Num8_7		lcd_seg[2]
#define		Digit_Num8_8		lcd_seg[3]
#define		Digit_Num8_9		lcd_seg[4]
#define		Digit_Num8_10		lcd_seg[5]
#define		Digit_Num8_11		lcd_seg[8]
#define		Digit_Num8_12		lcd_seg[9]


/* LCD状态 */
typedef struct
{
    bool refresh_en_sta;    // 刷新使能状态
    bool screen_en_sta;     // 屏幕开启状态
    bool reset_off_timer;
    int8_t bkl_pwm;         // 背光PWM
}lcd_param_st;

extern lcd_param_st lcd_param;
extern uint8_t lcd_seg[SEG_LEN];
extern uint8_t lcd_seg_buff[SEG_LEN];

extern bool LCD_boot_screen(uint16_t soc);
extern void LCD_module_init(void);
extern void LCD_module_off(void);
extern void LCD_module_on(void);
// extern void LCD_BackLight_set(int8_t pwm_value);
#ifdef ENABLE_STK3311
extern void LCD_BackLight_SelfAdaption(void);
#endif
extern void LCD_refresh(void);
extern void LCD_clear_buff(void);

extern void Bat_bar_show_num(uint8_t num);
extern int8_t Top_Show_Num(uint8_t number);
// extern int8_t Top_Show_Time(int16_t number);
extern void Top_Show_Symbol(uint8_t s1, uint8_t s2);
extern int8_t Bottom_Show_Time(int16_t number);
extern void Bottom_Show_Symbol(uint8_t s1, uint8_t s2);
extern int8_t Left_Show_Num(int16_t number);
extern void Left_Show_Symbol(uint8_t s1, uint8_t s2, uint8_t s3, uint8_t s4);
extern int8_t Right_Show_Num(int16_t number);
extern void Right_Show_DecNum(uint16_t val);
extern void Right_Show_Symbol(uint8_t s1, uint8_t s2, uint8_t s3, uint8_t s4);

#endif
