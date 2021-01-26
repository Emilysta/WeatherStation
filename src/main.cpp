#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>

#include "LCD.c"
#include "dht.c"
#include "i2c.c"
#include "spi.c"

int page = 0;
int menu_page = 1;
int space_table[5];
int block_keyboard = 0;

uint8_t scan_keyboard() {
  uint8_t r, c;
  for (r = 0; r < 4; r++) {
    PORTA &= ~(1 << r);  // set low
    _delay_ms(1);
    for (c = 0; c < 4; c++) {
      if (!(PINA & (1 << (c + 4)))) {  // check if on PORTC is sth
        PORTA |= (1 << r);
        _delay_ms(100);
        return r * 4 + (c + 1);
      }
    }
    PORTA |= (1 << r);  // set high
  }
  _delay_ms(100);
  return 99;  // nothing pressed
}

void print_parameters(struct Save_elem* to_read_with_data) {
  char date[9];
  sprintf(date, "%i.%i.%i", to_read_with_data->time_to_save.date,
          to_read_with_data->time_to_save.month,
          to_read_with_data->time_to_save.year);
  LCD_Clear();
  LCD_GoHome();
  LCD_WriteText(date);
  LCD_GoTo(1, 10);
  LCD_WriteInt(to_read_with_data->temperature);
  LCD_Write(0);
  LCD_Write(2);
  LCD_GoTo(2, 1);
  LCD_WriteInt(to_read_with_data->humidity);
  LCD_WriteText("%");
  LCD_GoTo(2, 8);
  LCD_WriteInt(to_read_with_data->pressure);
  LCD_WriteText("hPa");
}

void menu() {                           // first page of menu
  TCCR1B |= (1 << CS12) | (1 << CS10);  // 0b00000101 - start counting
  menu_page = 1;
  LCD_Clear();
  LCD_GoHome();
  LCD_WriteText("1-temperatura");
  LCD_GoTo(2, 1);
  LCD_WriteText("2-wilgotno");
  LCD_Write(0);
  LCD_Write(3);
  LCD_Write(0);
  LCD_Write(4);
}
void menu2() {  // second page of menu
  menu_page = 2;
  LCD_Clear();
  LCD_GoHome();
  LCD_WriteText("3-ci");
  LCD_Write(0);
  LCD_Write(3);
  LCD_WriteText("nienie");
  LCD_GoTo(2, 1);
  LCD_WriteText("4-data");
}
void menu3() {  // third page of menu
  menu_page = 3;
  LCD_Clear();
  LCD_GoHome();
  LCD_WriteText("5-zapis");
  LCD_GoTo(2, 1);
  LCD_WriteText("6-wy");
  LCD_Write(0);
  LCD_Write(3);
  LCD_WriteText("_zapisanych");
}

void timers() {                           // start settings of timers
  TIMSK |= (1 << TOIE1) | (1 << OCIE1A);  // 328p - TIMSK1 32-TIMSK
  TCNT1 = 0;                              // start value to 0.
  OCR1A = 32500;                          // value to compare
  sei();                                  // enable interrupts
}

ISR(TIMER1_COMPA_vect) {
  if (menu_page == 1) {  // changing pages
    menu2();
  } else if (menu_page == 2) {
    menu3();
  } else {
    menu();
  }
  TCNT1 = 0;
}

ISR(TIMER1_OVF_vect)  // interrupt
{
  TCNT1 = 0;               // set start value to 0.
  page = 0;                // move to page 0 - main menu
  menu();                  // display main menu
  TIMSK |= (1 << OCIE1A);  // enable comparing
  block_keyboard = 0;      // unlock keyboard
}

void page2(int SL) {  // subpage od 3rd page in main menu
  /*stop timer*/
  TCCR1B &= ~(1 << CS12);  // 0b00000001
  TCCR1B &= ~(1 << CS10);  // 0b00000000
  LCD_Clear();
  LCD_GoHome();
  if (SL) {
    LCD_WriteText("Miejsce odczytu:");
  } else {
    LCD_WriteText("Miejsce zapisu:");
  }
  LCD_GoTo(2, 1);
  // p means free space,
  // u means used space
  for (int i = 0; i < 5; i++) {
    LCD_GoTo(2, i * 3 + 1);
    LCD_WriteInt(i + 1);
    if (space_table[i] == 1) {
      LCD_WriteText("u");
    } else {
      LCD_WriteText("p");
    }
  }
}

int main(void) {
  int8_t humidity = 0;
  int8_t temperature = 0;
  int kb = 99;

  int8_t special[8] = {0b00011000, 0b00011011, 0b00000100, 0b00000100,  //°C
                       0b00000100, 0b00000100, 0b00000100, 0b00000011};
  int8_t polish_s[8] = {0b00000010, 0b00000100, 0b00001110, 0b00010000,  //ś
                        0b00001110, 0b00000001, 0b00011110, 0b00000000};
  int8_t polish_c[8] = {0b00000010, 0b00000100, 0b00001110, 0b00010000,  //ć
                        0b00010000, 0b00010001, 0b00001110, 0b00000000};

  for (int i = 0; i < 5; i++) {
    space_table[i] = 0;  // zero means free space to save in this run
  }

  DDRA |= (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3);  // ustawienie wyjść
  PORTA |= (1 << 0) | (1 << 1) | (1 << 2) |
           (1 << 3);  // ustawienie wartości wyjść stany wysokie
  PORTA |= (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7);  // ustawienie wejść

  timers();                         // set timers
  LCD_Init();                       // init LCD cursor off
  LCD_Clear();                      // clear lcd
  LCD_GoHome();                     // set cursor to home
  LCD_AddSpecialChar(special, 2);   // add °C to memory
  LCD_AddSpecialChar(polish_s, 3);  // add ś to memory
  LCD_AddSpecialChar(polish_c, 4);  // add ć to memory
  menu();                           // display menu
  spi_init();                       // init spi commmunication
  i2c_init();                       // init i2c commmunication
  _delay_ms(10);
  /*Time to_set;                    // this was to set date in ds1307, once it
  is to_set.seconds = 0;               // once it is set, you can comment out
  to_set.minutes = 20;
  to_set.hours = 18;
  to_set.day = 1;
  to_set.date = 25;
  to_set.month = 1;
  to_set.year = 21;
  set_time_in_DS1307(to_set);*/
  _delay_ms(100);
  int pressure;
  char date[9];
  char time[9];
  struct Time time_to_get;
  while (1) {
    switch (page) {
      case 0: {  // main menu
        kb = scan_keyboard();
        if (!block_keyboard) {
          switch (kb) {
            case 1: {  // show temperature
              block_keyboard = 1;
              TCNT1 = 0;
              TIMSK &= ~(1 << OCIE1A);
              dht_gettemperaturehumidity(&temperature, &humidity);
              LCD_Clear();
              LCD_GoHome();
              LCD_WriteText("Obecna");
              LCD_GoTo(2, 1);
              LCD_WriteText("temperatura:");
              LCD_WriteInt(temperature);
              LCD_Write(0);
              LCD_Write(2);
              break;
            }
            case 2: {  // show humidity
              block_keyboard = 1;
              TCNT1 = 0;
              TIMSK &= ~(1 << OCIE1A);
              dht_gettemperaturehumidity(&temperature, &humidity);
              LCD_Clear();
              LCD_GoHome();
              LCD_WriteText("Obecna");
              LCD_GoTo(2, 1);
              LCD_WriteText("wilgotno");
              LCD_Write(0);
              LCD_Write(3);
              LCD_Write(0);
              LCD_Write(4);
              LCD_WriteText(": ");
              LCD_WriteInt(humidity);
              LCD_WriteText("%");
              break;
            }
            case 3: {  // show pressure
              block_keyboard = 1;
              TCNT1 = 0;
              TIMSK &= ~(1 << OCIE1A);
              pressure = spi_download_press();
              LCD_Clear();
              LCD_GoHome();
              LCD_WriteText("Obecne ci");
              LCD_Write(0);
              LCD_Write(3);
              LCD_WriteText("nienie");
              LCD_GoTo(2, 1);
              LCD_WriteInt(pressure);
              LCD_WriteText("hPa");
              break;
            }
            case 4: {  // show date
              block_keyboard = 1;
              TCNT1 = 0;
              TIMSK &= ~(1 << OCIE1A);
              for (int i = 0; i < 8; i++) {
                date[i] = '\0';
                time[i] = '\0';
              }
              LCD_Clear();
              for (int i = 0; i < 5; i++) {
                get_full_date_from_DS1307(&time_to_get);
                sprintf(date, "%i.%i.%i", time_to_get.date, time_to_get.month,
                        time_to_get.year);
                sprintf(time, "%i:%i:%i", time_to_get.hours,
                        time_to_get.minutes, time_to_get.seconds);
                _delay_ms(2);
                LCD_GoHome();
                LCD_WriteText(time);
                LCD_GoTo(2, 1);
                LCD_WriteText(date);
                _delay_ms(995);
              }
              break;
            }
            case 5: {  // go to submenu to save
              page = 1;
              TCNT1 = 0;
              TIMSK &= ~(1 << OCIE1A);
              _delay_ms(100);
              page2(0);
              break;
            }
            case 6: {  // go to submenu to read
              page = 2;
              TCNT1 = 0;
              TIMSK &= ~(1 << OCIE1A);
              _delay_ms(100);
              page2(1);
              break;
            }
          }
        }
        break;
      }
      case 1: {  // submenu to save data to eeprom
        kb = scan_keyboard();
        if (!block_keyboard) {
          if (kb != 99) {
            struct Save_elem to_save_with_data;
            dht_gettemperaturehumidity(&to_save_with_data.temperature,
                                       &(to_save_with_data.humidity));
            to_save_with_data.pressure = spi_download_press();
            get_full_date_from_DS1307(&to_save_with_data.time_to_save);

            switch (kb) {
              case 1: {
                block_keyboard = 1;
                TCCR1B |=
                    (1 << CS12) | (1 << CS10);  // 0b00000101 - start counting
                save_to_memory(&to_save_with_data, 1);
                LCD_Clear();
                LCD_GoHome();
                LCD_WriteText("Zapisano na");
                LCD_GoTo(2, 1);
                LCD_WriteText("pozycji: 1");
                space_table[0] = 1;
                break;
              }
              case 2: {
                block_keyboard = 1;
                TCCR1B |=
                    (1 << CS12) | (1 << CS10);  // 0b00000101 - start counting
                save_to_memory(&to_save_with_data, 2);
                LCD_Clear();
                LCD_GoHome();
                LCD_WriteText("Zapisano na");
                LCD_GoTo(2, 1);
                LCD_WriteText("pozycji: 2");
                space_table[1] = 1;
                break;
              }
              case 3: {
                block_keyboard = 1;
                TCCR1B |=
                    (1 << CS12) | (1 << CS10);  // 0b00000101 - start counting
                save_to_memory(&to_save_with_data, 3);
                LCD_Clear();
                LCD_GoHome();
                LCD_WriteText("Zapisano na");
                LCD_GoTo(2, 1);
                LCD_WriteText("pozycji: 3");
                space_table[2] = 1;
                break;
              }
              case 4: {
                block_keyboard = 1;
                TCCR1B |=
                    (1 << CS12) | (1 << CS10);  // 0b00000101 - start counting
                save_to_memory(&to_save_with_data, 4);
                LCD_Clear();
                LCD_GoHome();
                LCD_WriteText("Zapisano na");
                LCD_GoTo(2, 1);
                LCD_WriteText("pozycji: 4");
                space_table[3] = 1;
                break;
              }
              case 5: {
                block_keyboard = 1;
                TCCR1B |=
                    (1 << CS12) | (1 << CS10);  // 0b00000101 - start counting
                save_to_memory(&to_save_with_data, 5);
                LCD_Clear();
                LCD_GoHome();
                LCD_WriteText("Zapisano na");
                LCD_GoTo(2, 1);
                LCD_WriteText("pozycji: 5");
                space_table[4] = 1;
                break;
              }
              case 16: {
                block_keyboard = 1;
                TCCR1B |=
                    (1 << CS12) | (1 << CS10);  // 0b00000101 - start counting
                LCD_Clear();
                LCD_GoHome();
                LCD_WriteText("Anuluje.");
                _delay_ms(500);
                LCD_WriteText(".");
                _delay_ms(500);
                LCD_WriteText(".");
                break;
              }
            }
          }
        }
        break;
      }
      case 2: {  // submenu to read data from eeprom
        kb = scan_keyboard();
        if (!block_keyboard) {
          if (kb != 99) {
            struct Save_elem to_read_with_data;
            switch (kb) {
              case 1: {
                block_keyboard = 1;
                read_from_memory(&to_read_with_data, 0);
                TCCR1B |=
                    (1 << CS12) | (1 << CS10);  // 0b00000101 - start counting
                print_parameters(&to_read_with_data);
                break;
              }
              case 2: {
                block_keyboard = 1;
                read_from_memory(&to_read_with_data, 1);
                TCCR1B |=
                    (1 << CS12) | (1 << CS10);  // 0b00000101 - start counting
                print_parameters(&to_read_with_data);
                break;
              }
              case 3: {
                block_keyboard = 1;
                read_from_memory(&to_read_with_data, 2);
                TCCR1B |=
                    (1 << CS12) | (1 << CS10);  // 0b00000101 - start counting
                print_parameters(&to_read_with_data);
                break;
              }
              case 4: {
                block_keyboard = 1;
                read_from_memory(&to_read_with_data, 3);
                TCCR1B |=
                    (1 << CS12) | (1 << CS10);  // 0b00000101 - start counting
                print_parameters(&to_read_with_data);
                break;
              }
              case 5: {
                block_keyboard = 1;
                read_from_memory(&to_read_with_data, 4);
                TCCR1B |=
                    (1 << CS12) | (1 << CS10);  // 0b00000101 - start counting
                print_parameters(&to_read_with_data);
                break;
              }
              case 16: {
                block_keyboard = 1;
                TCCR1B |=
                    (1 << CS12) | (1 << CS10);  // 0b00000101 - start counting
                LCD_Clear();
                LCD_GoHome();
                LCD_WriteText("Anuluje.");
                _delay_ms(500);
                LCD_WriteText(".");
                _delay_ms(500);
                LCD_WriteText(".");
                break;
              }
            }
          }
        }
        break;
      }
    }
  }
}