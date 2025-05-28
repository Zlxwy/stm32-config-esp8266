#ifndef __OLED_H
#define __OLED_H
#include "stm32f4xx.h"                  // Device header
#include "delay.h"
#include "sys.h"

//如果要更改软件IIC使用的引脚，只需在这里改就行了
#define  RCC_GPIO_OLED_PORT_Periph   RCC_AHB1Periph_GPIOF
#define  GPIO_OLED_PORT              GPIOF
#define  GPIO_OLED_SDA_PIN           GPIO_Pin_0
#define  GPIO_OLED_SCL_PIN           GPIO_Pin_1
#define  OLED_SDA                    PFout(0)
#define  OLED_SDA_READ               PFin(0)
#define  OLED_SCL                    PFout(1)

/**OLED的IIC引脚电平的操作宏定义**/
#define  OLED_SDA_SET                OLED_SDA=1
#define  OLED_SCL_SET                OLED_SCL=1
#define  OLED_SDA_CLR                OLED_SDA=0
#define  OLED_SCL_CLR                OLED_SCL=0

void OLED_WriteCommand(u8 command);
void OLED_WriteData(u8 data);
void OLED_SetCursor(u8 Y, u8 X);
void OLED_Clear(void);
void OLED_Init(void);

void OLED_ShowChar(u8 Line, u8 Column, char Char);
void OLED_ClearLine(u8 line1, u8 line2);
void OLED_ClearColumn(u8 column1,u8 column2);
void OLED_ShowString(u8 Line, u8 Column, char *String);
void OLED_ShowString_FromLineToLine(u8 startline,char *str,u8 endline);
void OLED_ShowString_FromPointToLine(u8 Line,u8 Column,char *String,u8 endline);
void OLED_ShowNum(u8 Line,u8 Column,u32 Number,u8 Length);
void OLED_ShowSignedNum(u8 Line,u8 Column,s32 Number,u8 Length);
void OLED_ShowHexNum(u8 Line,u8 Column,u32 Number,u8 Length);
void OLED_ShowBinNum(u8 Line,u8 Column,u32 Number,u8 Length);

#endif
