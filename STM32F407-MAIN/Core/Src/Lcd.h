#ifndef __LCD1602_H_
#define __LCD1602_H_

#define DELAY_2N     4

void LCD_Init(void);
void LCD_SetCursorPos(unsigned char line, unsigned char column);
void LCD_Clear(void);
void LCD_PrintChar(unsigned char row, unsigned char column, char c);
void LCD_PrintString(unsigned char row, unsigned char column, char *string);
void LCD_PrintNum(unsigned char row, unsigned char column, long num);

#endif
