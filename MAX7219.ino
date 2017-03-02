/*
  One MAX7219 connected to an 8x8 LED matrix.

  Inspiration - https://halfbyteblog.wordpress.com/2015/11/29/using-a-max7219-8x8-led-matrix-with-an-attiny85-like-trinket-or-digispark/
  and http://www.gammon.com.au/forum/?id=11516 

  P0 -> DIN
  P1 -> CS
  P2 -> CLK
  
 */

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "font.h"

#define CLK_HIGH()  PORTB |= (1<<PB2)
#define CLK_LOW()   PORTB &= ~(1<<PB2)
#define CS_HIGH()   PORTB |= (1<<PB1)
#define CS_LOW()    PORTB &= ~(1<<PB1)
#define DATA_HIGH() PORTB |= (1<<PB0)
#define DATA_LOW()  PORTB &= ~(1<<PB0)
#define INIT_PORT() DDRB |= (1<<PB0) | (1<<PB1) | (1<<PB2)

#define delay_long   400
#define delay_short  10
#define delay_scroll 35

void spi_send(uint8_t data)
{
    uint8_t i;

    for (i = 0; i < 8; i++, data <<= 1)
    {
  CLK_LOW();
  if (data & 0x80)
      DATA_HIGH();
  else
      DATA_LOW();
  CLK_HIGH();
    }
    
}

void max7219_writec(uint8_t high_byte, uint8_t low_byte)
{
    CS_LOW();
    spi_send(high_byte);
    spi_send(low_byte);
    CS_HIGH();
}

void max7219_clear(void)
{
    uint8_t i;
    for (i = 0; i < 8; i++)
    {
  max7219_writec(i+1, 0);
    }
}

void max7219_init(void)
{
    INIT_PORT();
    // Decode mode: none
    max7219_writec(0x09, 0);
    // Intensity: 3 (0-15)
    max7219_writec(0x0A, 5);
    // Scan limit: All "digits" (rows) on
    max7219_writec(0x0B, 7);
    // Display test: off
    max7219_writec(0x0F, 0);
    max7219_clear();
    // Shutdown register: Display on
    max7219_writec(0x0C, 1);
}


uint8_t display[16];

void update_display(void)
{
    uint8_t i;

    for (i = 0; i < 8; i++)
    {
  max7219_writec(i+1, display[i]);
    }
}

void letter(uint8_t ch)
{
    uint8_t i;

    for (i = 0; i < 8; i++)
      display[i] = pgm_read_byte (&p437_font [ch] [i]);
    update_display();
}

void preload(uint8_t ch)
{
    uint8_t i;

    for (i = 0; i < 8; i++)
      display[i+8] = pgm_read_byte (&p437_font [ch] [i]);
    update_display();
}

void scroll_left()
{
    uint8_t i;
    uint8_t j;

    for (j = 0; j < 8; j++) {
    for (i = 0; i < 15; i++)
      display[i] = display[i+1];
      update_display();
      _delay_ms(delay_scroll);
    }
}

void showstring (const char * s)
{
  char c;
  while (c = *s++)
    {
    letter (c); 
     if (*s)
        preload (*s);
      else
        preload (' ');
    _delay_ms(delay_long);
    scroll_left();
//    letter (' ');  // brief gap between letters
//    _delay_ms(delay_short);
    }
}  // end of showString

void set_pixel(uint8_t r, uint8_t c, uint8_t value)
{
    switch (value)
    {
    case 0: // Clear bit
  display[r] &= (uint8_t) ~(0x80 >> c);
  break;
    case 1: // Set bit
  display[r] |= (0x80 >> c);
  break;
    default: // XOR bit
  display[r] ^= (0x80 >> c);
  break;
    }
}


int main(void)
{
    uint8_t i;
    
    max7219_init();

    while(1)
    {
      showstring(" ATTiny85 lichtkrant \x1A\x1B Jan-Willem Ruys \x1A\x1B ABC 123 \x1A\x1B");
    }
}
