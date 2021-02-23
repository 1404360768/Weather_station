#ifndef __LCD_NOKIA_5110_H
#define __LCD_NOKIA_5110_H

#include "lcd_config.h"
#include "fonts.h" //需要的字符集

#define NOKIA_5110_WIDE     84
#define NOKIA_5110_HIGH     48


// /CS 片选引脚  低电平有效,本例最简单方法直接接地
// LCD				ATmage8
// -RST:复位,低脉冲即可  <--->  	PB2
// CE:片选,低电平有效	<--->	这里直接接地了
// Din:LCD的数据输入 	<---> 	PB3 (MOSI/OC2)
// Clk:同步信号 		<---> 	PB5 (SCK)
// Vcc: 电源
// Gnd:地
// BL:背光 高电瓶亮,低电平瓶没,就这样


void Nokia_Init(void);

void Nokia_Write_Char_nMemory(uint8_t c);
void Nokia_Draw_Point_nMemory(unsigned char y, unsigned char x);

int32_t Nokia_Request_Buff(void);
void Nokia_RefreshMemory(void);
void Nokia_Draw_Point(uint8_t x, uint8_t y, uint8_t t);
void Nokia_Write_Char(uint8_t x,uint8_t y,uint8_t c);
void Nokia_Write_Byte(uint8_t dat, uint8_t cmd);







#endif
