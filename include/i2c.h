#define SLAVE_ADDRESS_WRITE 0xD0
#define SLAVE_ADDRESS_READ 0xD1
#define SCL_FREQ 100000

#define T_START 0x08     // start transmitted
#define T_RSTART 0x10    // repeated start transmitted
#define ARBIT_LOST 0x38  // arbitration lost
/*MASTER Sender Mode*/
#define TSLAW_R_ACK 0x18   // slave address write transmitted, received ACK
#define TSLAW_R_NACK 0x20  // slave address write transmitted, received NACK
#define TDATA_R_ACK 0x28   // data transmitted, received ACK
#define TDATA_R_NACK 0x30  // data transmitted, received NACK
/*MASTER Receiver Mode*/
#define REC_TSLAR_R_ACK 0x40   // slave address read transmitted, send ACK
#define REC_TSLAR_R_NACK 0x48  // slave address read transmitted, send NACK
#define REC_RDATA_R_ACK 0x50   // data received, send ACK
#define REC_RDATA_R_NACK 0x58  // data received, send NACK

struct Time {
  int seconds;
  int minutes;
  int hours;
  int day;
  int date;
  int month;
  int year;
};

struct Save_elem {
  int8_t temperature;
  int8_t humidity;
  int pressure;
  struct Time time_to_save;
};

/*initialize i2c */
void i2c_init();
/*Send start to slave_address*/
int i2c_send_start(char slave_address);
/*Send stop (slave address not needed) */
void i2c_send_stop();
/*Send (read/write) address of memory*/
int i2c_rw_address(char rw_address);
/*Send (read/write) address of memory without checking ACK*/
int i2c_rwspecial_address(char rw_address);
/*Write data to slave */
int i2c_write(char data);
/*Send reapeted start to slave_address*/
int i2c_send_repeated_start(char slave_address);
/*Read from slave
end_of_reading is to set wheter to send ack, or nack bit
1 - means end reading, send nack bit
0 - means that there is sth to read there*/
int i2c_read(int end_of_reading);

/*additional functions*/
/*Converts decimal to BCD format*/
int to_BCD_format(int input);

/*Converts BCD format to decimal*/
int from_BCD_format(int input);

/*Set time specified by arguments
-seconds
-minutes
-hours
-day - day of week
-date - day in date
-month
-year*/
void set_time_in_DS1307(struct Time to_set);

/*Get full date from DS1307
-seconds
-minutes
-hours
-day - day of week
-date - day in date
-month
-year */
int get_full_date_from_DS1307(struct Time *to_get);

/*Save to memory
-date - day in date
-month
-year
-pressure
-temperature
-humidity */
int save_to_memory(struct Save_elem *to_save, uint8_t place);

/*Read from memory
-date - day in date
-month
-year
-pressure
-temperature
-humidity
*/
int read_from_memory(struct Save_elem *to_read, uint8_t place);
