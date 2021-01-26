#include <avr/io.h>
/*Initialization of communication with the sensor*/
void spi_init();
/*Read from address*/
char spi_read(char address);
/*Write data to addresss*/
void spi_write(char address, char data);
/*Download pressure data*/
int32_t spi_download_press();
/*Download temperature data*/
uint8_t spi_download_temp();