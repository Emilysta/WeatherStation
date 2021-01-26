#include "i2c.h"
#include <avr/io.h>

void i2c_init()
{
    TWSR = (1 << TWPS1) | (1 << TWPS0);
    TWBR = ((F_CPU / SCL_FREQ) - 16) / (2 * pow(4, (TWSR & ((1 << TWPS0) | (1 << TWPS1)))));
}

int i2c_send_start(char slave_address)
{
    //send start condition
    TWCR = (1 << TWEN) | (1 << TWSTA) | (1 << TWINT); //set enable bit, set start condition bit, set interrupt
    while (!(TWCR & (1 << TWINT)))
        ;                         //wait till end of start
    if ((TWSR & 0xF8) != T_START) //check start condition
        return -1;
    //send slave address
    TWDR = slave_address; //write slave address with R/W bit.
    TWCR = (1 << TWEN) | (1 << TWINT); //set enable bit, set interrupt
    while (!(TWCR & (1 << TWINT)))
        ; //wait for ack received
    //check if ack was received
    if ((TWSR & 0xF8) != TSLAW_R_ACK) //0xF8 - ignore 3least bits
        return -1;
    return 0;
}

void i2c_send_stop()
{
    TWCR = (1 << TWEN) | (1 << TWSTO) | (1 << TWINT); //set enable bit, set stop condition bit, set interrupt
}
int i2c_rw_address(char rw_address)
{
    TWDR = rw_address;                 //set address of place where to save data or read from
    TWCR = (1 << TWINT) | (1 << TWEN); //set interrupt, set enable bit
    while (!(TWCR & (1 << TWINT)))
        ;                             //wait till end...
    if ((TWSR & 0xF8) != TDATA_R_ACK) //0xF8 - ignore 3least bits
        return -1;
    return 0;
}
int i2c_rwspecial_address(char rw_address)
{
    TWDR = rw_address;                 //set address of place where to save data or read from
    TWCR = (1 << TWINT) | (1 << TWEN); //set interrupt, set enable bit
    while (!(TWCR & (1 << TWINT)))
        ; //wait till end...
        //no chceck on ack
    return 0;
}
int i2c_write(char data)
{
    TWDR = data;                       //set data to save
    TWCR = (1 << TWINT) | (1 << TWEN); //set interrupt, set enable bit
    while (!(TWCR & (1 << TWINT)))
        ;                             //wait till end ...
    if ((TWSR & 0xF8) != TDATA_R_ACK) //0xF8 - ignore 3least bits
        return -1;
    return 0;
}
int i2c_send_repeated_start(char slave_address)
{
    //send repeated start condition
    TWCR = (1 << TWEN) | (1 << TWSTA) | (1 << TWINT); //set enable bit, set start condition bit, set interrupt
    while (!(TWCR & (1 << TWINT)))
        ;                          //wait till end...
    if ((TWSR & 0xF8) != T_RSTART) //check repeated start condition
        return -1;
    //send slave address
    TWDR = slave_address; //write slave address with R/W bit.
    TWCR = (1 << TWEN) | (1 << TWINT);
    while (!(TWCR & (1 << TWINT)))
        ; //wait for ack received
    //check if ack was received
    if ((TWSR & 0xF8) != TSLAW_R_ACK) //0xF8 - ignore 3least bits
      return -1;
    return 0;
}
int i2c_read(int end_of_reading)
{
    if (end_of_reading)
    {
        TWCR = (1 << TWINT) | (1 << TWEN); //set interrupt, set enable bit
    }
    else
    {                                                    
        TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA); //set interrupt, set enable bit, set enable ack bit
    }
    while (!(TWCR & (1 << TWINT)))
        ;
    if (((TWSR & 0xF8) == REC_RDATA_R_ACK) || ((TWSR & 0xF8) == REC_RDATA_R_NACK)) //0xF8 - ignore 3least bits
        return TWDR;
    return -1;
}

int to_BCD_format(int input)
{
    int result = 0;
    int shift = 0;
    while (input > 0)
    {
        result |= (input % 10) << shift * 4;
        input /= 10;
        shift++;
    }
    return result;
}

int from_BCD_format(int input)
{
    int result = 0;
    int sign = 0;
    int multip = 1;
    while (input > 0)
    {
        sign = input & 0x0F;
        input = input >> 4;
        result += multip * sign;
        multip *= 10;
    }
    return result;
}

void set_time_in_DS1307(struct Time to_set)
{
    i2c_send_start(SLAVE_ADDRESS_WRITE);
    i2c_rw_address(0x00);
    i2c_write(to_BCD_format(to_set.seconds));
    i2c_write(to_BCD_format(to_set.minutes) & 0x7F); //make 0 at bit7
    i2c_write(to_BCD_format(to_set.hours) & 0x3F);   //make 0 at bit7 and bit6
    i2c_write(to_BCD_format(to_set.day) & 0x07);     //consider only 3 first bits
    i2c_write(to_BCD_format(to_set.date) & 0x3F);    //consider only 6 first bits
    i2c_write(to_BCD_format(to_set.month) & 0x1F);   //consider only 5 first bits
    i2c_write(to_BCD_format(to_set.year) & 0xFF);    //consider all bits
    i2c_send_stop();
}

int get_full_date_from_DS1307(struct Time *to_get)
{
    if (i2c_send_start(SLAVE_ADDRESS_WRITE) == -1)
        return 1;
    if (i2c_rw_address(0x00) == -1) //write start point which is 00h
        return 2;
    if (i2c_send_repeated_start(SLAVE_ADDRESS_READ) == -1) //go to read mode
        return 3;

    int date_arr[7];
    for (int i = 0; i < 6; i++)
    {
        date_arr[i] = i2c_read(0);  //read and send ack
    }
    date_arr[6] = i2c_read(1);      //read and send nack
    i2c_send_stop();                //send stop
    for (int i = 0; i < 7; i++)
    {
        date_arr[i] = from_BCD_format(date_arr[i]);
    }
    to_get->seconds = date_arr[0];
    to_get->minutes = date_arr[1];
    to_get->hours = date_arr[2];
    to_get->day = date_arr[3];
    to_get->date = date_arr[4];
    to_get->month = date_arr[5];
    to_get->year = date_arr[6];
    return 0;
}

#define EEPROM_DS_WRITE 0xA0    //address of eeprom and write - LSB bit 0.
#define EEPROM_DS_READ 0xA1     //address of eeprom and read - LSB bit 1.
#define ZERO_ADDRESS_1 0x00     //first part of memory address in eeprom 
#define ZERO_ADDRESS_2 0x00     //second part of memory address in eeprom 

int save_to_memory(struct Save_elem *to_save, uint8_t place)
{
    int temp = 0;
    temp = to_save->pressure - 900;
    i2c_send_start(EEPROM_DS_WRITE); //save day in eeprom
    i2c_rw_address(ZERO_ADDRESS_1);
    i2c_rw_address(ZERO_ADDRESS_2 + place * 8);
    i2c_write(to_save->time_to_save.date);
    i2c_send_stop();
    _delay_ms(5);
    i2c_send_start(EEPROM_DS_WRITE); //save month in eeprom
    i2c_rw_address(ZERO_ADDRESS_1);
    i2c_rw_address(ZERO_ADDRESS_2 + 1 + place * 8);
    i2c_write(to_save->time_to_save.month); 
    i2c_send_stop();
    _delay_ms(5);
    i2c_send_start(EEPROM_DS_WRITE); //save year in eeprom
    i2c_rw_address(ZERO_ADDRESS_1);
    i2c_rw_address(ZERO_ADDRESS_2 + 2 + place * 8);
    i2c_write(to_save->time_to_save.year); 
    i2c_send_stop();
    _delay_ms(5);
    i2c_send_start(EEPROM_DS_WRITE); //save temperature in eeprom
    i2c_rw_address(ZERO_ADDRESS_1);
    i2c_rw_address(ZERO_ADDRESS_2 + 3 + place * 8);
    i2c_write(to_save->temperature);
    i2c_send_stop();
    _delay_ms(5);
    i2c_send_start(EEPROM_DS_WRITE); //save humidity in eeprom
    i2c_rw_address(ZERO_ADDRESS_1);
    i2c_rw_address(ZERO_ADDRESS_2 + 4 + place * 8);
    i2c_write(to_save->humidity);
    i2c_send_stop();
    _delay_ms(5);
    i2c_send_start(EEPROM_DS_WRITE); //save pressure in eeprom
    i2c_rw_address(ZERO_ADDRESS_1);
    i2c_rw_address(ZERO_ADDRESS_2 + 5 + place * 8);
    i2c_write(temp);
    i2c_send_stop();
    _delay_ms(5);
    return 0;
}
int read_from_memory(struct Save_elem *to_read, uint8_t place)
{
    int date_arr[6];
    i2c_send_start(EEPROM_DS_WRITE);        //send address of eeprom
    i2c_rwspecial_address(ZERO_ADDRESS_1);  //send memory address of eeprom
    i2c_rw_address(ZERO_ADDRESS_2 + place * 8);
    _delay_ms(20);
    i2c_send_repeated_start(EEPROM_DS_READ); 
    for (int i = 0; i < 5; i++)
    {
        date_arr[i] = i2c_read(0); //read and send ack
    }
    date_arr[5] = i2c_read(1); //read and send nack
    i2c_send_stop();
    to_read->time_to_save.date = date_arr[0];
    to_read->time_to_save.month = date_arr[1];
    to_read->time_to_save.year = date_arr[2];
    to_read->temperature = date_arr[3];
    to_read->humidity = date_arr[4];
    to_read->pressure = date_arr[5] + 900;

    return 0;
}
