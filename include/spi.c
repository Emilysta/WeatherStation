#include "spi.h"
#include <util/delay.h>

void spi_write(char address, char data)
{
    address &= 0x7F;
    PORTB &= ~(1 << 4);
    _delay_ms(1);
    SPDR = address;               //SPI Data register
    while (!(SPSR & (1 << SPIF))) //SPIF interrupt flag - end of transmission
        ;
    _delay_ms(10);
    SPDR = data;
    while (!(SPSR & (1 << SPIF))) //SPIF interrupt flag - end of transmission
        ;
    _delay_ms(1);
    PORTB |= (1 << 4);
}
char spi_read(char address)
{
    PORTB &= ~(1 << 4);
    _delay_ms(1);
    address |= 0x80;
    SPDR = address;               //Check Status Register if there is new data
    while (!(SPSR & (1 << SPIF))) //SPIF interrupt flag - end of transmission
        ;
    _delay_ms(10);
    SPDR = 0x55;
    while (!(SPSR & (1 << SPIF))) //SPIF interrupt flag - end of transmission
        ;
    _delay_ms(1);
    PORTB |= (1 << 4);
    return SPDR;
}
void spi_init()
{
    //SCK,MOSI,CS as output from microcontroller
    DDRB |= (1 << 7) | (1 << 5) | (1 << 4);
    PORTB |= (1 << 4);
    /*set eanble, set as master,set f_osc/64 - only spr1 to 1*/
    SPCR |= (1 << SPE) | (1 << MSTR) | (1 << SPR1);
    SPCR |= (1 << CPOL) | (1 << CPHA); //set spi mode=3
    SPCR &= ~(1 << DORD);              //set 0 on  DORD to transmit MSB first
    _delay_ms(5);
    spi_write(0x20, 0x00); //Write to CTRL_REG1 an init bit (bit 7 to 0) to set active status
    spi_write(0x10, 0x02);
    _delay_ms(100);

    spi_write(0x20, 0x00); //Write to CTRL_REG1 an init bit (bit 7 to 0) to set active status
    spi_write(0x20, 0x80); //Write to CTRL_REG1 an init bit (bit 7 to 1) to set active status
}
int32_t spi_download_press()
{
    spi_write(0x21, 0x01); //Write to CTRL_REG2, to set one shot 0x01
    _delay_ms(10);

    uint8_t temp = spi_read(0x27);
    while ((temp & 0x02) == 0)
    {
        _delay_ms(10);
        temp = spi_read(0x27);
    }

    uint8_t pressureL = spi_read(0x28);
    uint8_t pressureM = spi_read(0x29);
    uint8_t pressureH = spi_read(0x2a);

    int32_t press = (int32_t)(int8_t)pressureH << 16 | (uint16_t)pressureM << 8 | pressureL;
    int32_t output = (press) / 4096;
    return output;
}
uint8_t spi_download_temp()
{
    spi_write(0x21, 0x01); //Write to CTRL_REG2 , to set one shot 0x01
    _delay_ms(10);

    uint8_t temp = spi_read(0x27);
    while ((temp & 0x01) == 0)
    {
        _delay_ms(10);
        temp = spi_read(0x27);
    }

    int8_t tempL = spi_read(0x2B);
    int8_t tempH = spi_read(0x2C);
    int16_t temperature = (int16_t)(tempH << 8 | tempL);
    int8_t output = 42.5 + (float)(temperature) / 480;
    return output;
}