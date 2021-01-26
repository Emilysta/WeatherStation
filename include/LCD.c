#include "LCD.h"

void LCD_Init()
{
    LCD_RS_DDR |= (1 << LCD_RS_PIN);                 //set output for Register Select
    LCD_E_DDR |= (1 << LCD_E_PIN);                   //set output for Enable
    LCD_DATA_DDR |= (1 << LCD_DATA_FIRST_PIN);       //set 1st pin to output,sent data
    LCD_DATA_DDR |= (1 << (LCD_DATA_FIRST_PIN + 1)); //set 2nd pin to output,sent data
    LCD_DATA_DDR |= (1 << (LCD_DATA_FIRST_PIN + 2)); //set 3rd pin to output,sent data
    LCD_DATA_DDR |= (1 << (LCD_DATA_FIRST_PIN + 3)); //set 4th pin to output,sent data
    _delay_ms(100);
    /*Function set*/
    for (int i = 0; i < 3; i++)
    {
        LCD_SendInstruction(RESET_DISPLAY);
        _delay_ms(5);
    }
    LCD_SendInstruction(SET_4BIT_MODE);
    _delay_us(500);

    LCD_SendInstruction(SET_4BIT_MODE);
    LCD_SendInstruction(MODE_MASK | MODE_5x8 | TWO_LINES);
    _delay_us(200);

    /*DISPLAY ON/OFF 0000 1000 Control Display =0 Cursor=0 Blinking=0*/
    LCD_SendInstruction(0x0);
    LCD_SendInstruction(DISPLAY_MASK);

    _delay_us(200);

    /*Clear DISPLAY */
    LCD_Clear();
    /*Entry mode - increment on*/
    LCD_SendInstruction(0x0);
    LCD_SendInstruction(ENTRY_MODE | INCREMENT);
    _delay_us(200);
    /*DISPLAY ON 1100 Control Display =1 Cursor=0 Blinking=0*/
    LCD_SendInstruction(0x0);
    LCD_SendInstruction(DISPLAY_MASK | DISPLAY_ON | CURSOR_OFF);
    _delay_us(200);
}

void LCD_Clear()
{
    /*Clear DISPLAY */
    LCD_SendInstruction(0x0);           //0000
    LCD_SendInstruction(CLEAR_DISPLAY); //0001
    _delay_ms(5);
}

void LCD_SendInstruction(int8_t mask)
{
    LCD_E_PORT &= ~(1 << LCD_E_PIN);               //low
    LCD_RS_PORT &= ~(1 << LCD_RS_PIN);             //low
    LCD_E_PORT |= (1 << LCD_E_PIN);                //high
    LCD_DATA_PORT |= (mask << LCD_DATA_FIRST_PIN); //move mask to send in 4-bit mode
    LCD_E_PORT &= ~(1 << LCD_E_PIN);               //low
    LCD_DATA_PORT &= ~(mask << LCD_DATA_FIRST_PIN);
}

void LCD_WriteText(char *text)
{
    while (*text)
    {
        LCD_Write(*text >> 4);
        LCD_Write(*text);
        text++;
    }
}

void LCD_Write(int8_t symbol)
{
    LCD_RS_PORT |= (1 << LCD_RS_PIN); //high
    LCD_E_PORT |= (1 << LCD_E_PIN);   //high
    LCD_DATA_PORT |= (symbol << LCD_DATA_FIRST_PIN);
    LCD_E_PORT &= ~(1 << LCD_E_PIN); //low
    LCD_DATA_PORT &= ~(symbol << LCD_DATA_FIRST_PIN);
    LCD_RS_PORT &= ~(1 << LCD_RS_PIN); //low
    _delay_us(100);
}

void LCD_WriteInt(int number)
{
    char buffer[50];
    sprintf(buffer, "%d", number);
    int i = 0;
    while (buffer[i] != '\0')
    {
        LCD_Write(buffer[i] >> 4);
        LCD_Write(buffer[i]);
        i++;
    }
}

void LCD_GoTo(int row, int column)
{
    int8_t regist = (row - 1) * 0x40 + (column - 1);
    int8_t command = regist | 0x80;
    LCD_SendInstruction(command >> 4);
    LCD_SendInstruction(command);
    _delay_us(100);
}

void LCD_GoHome()
{
    LCD_SendInstruction(0x0);
    LCD_SendInstruction(RETURN_HOME);
    _delay_ms(5);
}

void LCD_AddSpecialChar(int8_t symbol[8], int where)
{
    int8_t command = 0x40 + where * 8;
    LCD_SendInstruction(command >> 4);
    LCD_SendInstruction(command);
    _delay_us(100);
    for (int i = 0; i < 8; i++)
    {
        LCD_Write(symbol[i] >> 4);
        LCD_Write(symbol[i]);
    }
}