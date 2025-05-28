#ifndef __OLED_H
#define __OLED_H

void OLED_Init(void);
void OLED_Clear(void);
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char);

void OLED_ClearLine(u8 line1,u8 line2);					//自己添加
void OLED_ClearColumn(u8 column1,u8 column2);		//自己添加

void OLED_ShowString(uint8_t Line, uint8_t Column, char *String);
void OLED_ShowString_FromLineToLine(u8 startline,char *str,u8 endline);							//自己添加
void OLED_ShowString_FromPointToLine(u8 Line,u8 Column,char *String,u8 endline);		//自己添加

void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length);
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);

#endif
