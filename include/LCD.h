#include <avr/io.h>
/*Data to set*/
/*RW to GND - writing mode and all data pins are in one row, one by one*/
#define LCD_E_DDR DDRC
#define LCD_E_PORT PORTC
#define LCD_E_PIN 6
#define LCD_RS_DDR DDRC
#define LCD_RS_PORT PORTC
#define LCD_RS_PIN 7
#define LCD_DATA_DDR DDRC
#define LCD_DATA_PORT PORTC
#define LCD_DATA_FIRST_PIN 2

/*FUNCTION SET*/
#define RESET_DISPLAY 0x3  // 0011
#define SET_4BIT_MODE 0x2  // 0010
#define MODE_MASK 0x0      // 0000
#define TWO_LINES 0x8      // 1000
#define ONE_LINE 0x0       // 0000
#define MODE_5x10 0x4      // 0100
#define MODE_5x8 0x0       // 0000

/*DISPLAY CONSTANTS*/
#define DISPLAY_MASK 0x8   // 1000
#define DISPLAY_OFF 0x0    // 0000
#define DISPLAY_ON 0x4     // 0100
#define CURSOR_ON 0x2      // 0010
#define CURSOR_OFF 0x0     // 0000
#define BLINK_ON 0x1       // 0001
#define BLINK_OFF 0x0      // 0000
#define CLEAR_DISPLAY 0x1  // 0001

/*ENTRY MODE CONSTANT*/
#define ENTRY_MODE 0x4  // 0100
#define INCREMENT 0x2   // 0010
#define DECREMENT 0x0   // 0000
#define SHIFT 0x1       // 0001
#define NO_SHIFT 0x0    // 0000

/*OTHER*/
#define RETURN_HOME 0x2  // 0010

/* Functions*/
/*Initialization of LCD*/
void LCD_Init();
/*Clear screen*/
void LCD_Clear();
/*Function to send instruction to LCD*/
void LCD_SendInstruction(int8_t mask);
/*Function to write one char to LCD, use twice to send char = 4bit|4bit*/
void LCD_Write(int8_t symbol);
/*Function to send text to LCD*/
void LCD_WriteText(char *text);
/*Function to send number - int - to LCD*/
void LCD_WriteInt(int number);
/*Move cursor to (row,column)
The top left corner is a point: (1,1)*/
void LCD_GoTo(int row, int column);
/*Move cursor to home (1,1)*/
void LCD_GoHome();
/*Function to add special charachter to memeory*/
void LCD_AddSpecialChar(int8_t symbol[8], int where);