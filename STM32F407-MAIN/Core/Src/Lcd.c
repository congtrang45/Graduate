#include "main.h"
#include "Lcd.h"
#include "stdio.h"
#include "string.h"
char CLR[17] = "                M";
void LCD_Delay (int cnt)
{
    cnt <<= DELAY_2N;
    while (cnt--);
}
void LCD_Data_out(unsigned char data)
{
	if ((data & 0x08) == 0x08)
		HAL_GPIO_WritePin(LCDD7_GPIO_Port,LCDD7_Pin, 1);
	else
		HAL_GPIO_WritePin(LCDD7_GPIO_Port,LCDD7_Pin, 0);

	if ((data & 0x04) == 0x04)
		HAL_GPIO_WritePin(LCDD6_GPIO_Port,LCDD6_Pin, 1);
	else
		HAL_GPIO_WritePin(LCDD6_GPIO_Port,LCDD6_Pin, 0);

	if ((data & 0x02) == 0x02)
		HAL_GPIO_WritePin(LCDD5_GPIO_Port,LCDD5_Pin, 1);
	else
		HAL_GPIO_WritePin(LCDD5_GPIO_Port,LCDD5_Pin, 0);

	if ((data & 0x01) == 0x01)
		HAL_GPIO_WritePin(LCDD4_GPIO_Port,LCDD4_Pin, 1);
	else
		HAL_GPIO_WritePin(LCDD4_GPIO_Port,LCDD4_Pin, 0);
}

void LCD_Write4bit(unsigned char c)
{
	HAL_GPIO_WritePin(LCDRS_GPIO_Port,LCDRW_Pin,0);
	HAL_GPIO_WritePin(LCDEN_GPIO_Port,LCDEN_Pin,1);
    LCD_Data_out(c&0x0F);
    LCD_Delay(35);
    HAL_GPIO_WritePin(LCDEN_GPIO_Port,LCDEN_Pin,0);
    LCD_Delay(35);
}

void LCD_WriteCMD(unsigned char c)
{
    LCD_Delay(35);
    HAL_GPIO_WritePin(LCDRS_GPIO_Port,LCDRS_Pin,0);
    LCD_Write4bit(c>>4);
    LCD_Write4bit(c);
}

void LCD_SetCursorPos(unsigned char line, unsigned char column)
{
    uint8_t  address;
    switch(line)
    {
        case 0: //Starting address of 1st line
            address = 0x80;
            break;
        case 1: //Starting address of 2nd line
            address = 0xC0;
            break;
        default:
            break;
    }
    address = address + column;
    LCD_WriteCMD(address);
}

void LCD_Clear(void)
{
	LCD_PrintString(0, 0, CLR);
	LCD_PrintString(1, 0, CLR);
}

void LCD_WriteData(unsigned char c)
{
    LCD_Delay(35);
    HAL_GPIO_WritePin(LCDRS_GPIO_Port,LCDRS_Pin,1);
    LCD_Write4bit(c>>4);
    LCD_Write4bit(c);
}

void LCD_Init(void)
{
    LCD_Delay(15000);
    HAL_GPIO_WritePin(LCDRS_GPIO_Port,LCDRS_Pin,0);
    LCD_Write4bit(0x3);  /* Select 4-bit interface             */
    LCD_Delay(15000);
    LCD_Write4bit(0x3);
    LCD_Delay(15000);
    LCD_Write4bit(0x3);
    LCD_Delay(15000);
    LCD_Write4bit(0x2);
    LCD_Delay(15000);

    LCD_WriteCMD(0x28);  /* 2 lines, 5x8 character matrix      */
    LCD_Delay(15000);
    LCD_WriteCMD(0x0C);  /* Display ctrl:Disp=ON,Curs/Blnk=OFF */
    LCD_Delay(15000);
    LCD_WriteCMD(0x06);  /* Entry mode: Move right, no shift   */
    LCD_Delay(15000);

    LCD_WriteCMD(0x80);  /* Set DDRAM address counter to 0     */
    LCD_Delay(15000);
}

void LCD_PrintChar(unsigned char row, unsigned char column, char c)
{
    LCD_SetCursorPos(row, column);
    LCD_WriteData(c);
}

void LCD_PrintString(unsigned char row, unsigned char column, char *string)
{
    LCD_SetCursorPos(row, column);
    while (*string)
    {
        LCD_WriteData(*string++);
    }
}

void LCD_PrintNum(unsigned char row, unsigned char column, long num)
{
    char sNum[17];
    sprintf(sNum, "%6d", num);

    LCD_PrintString(row, column, sNum);
}

