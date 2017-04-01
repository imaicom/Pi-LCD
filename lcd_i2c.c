// cc -o lcd_i2c lcd_i2c.c -lwiringPi
#include <stdio.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>

// Define some device parameters
#define I2C_ADDR   0x27 // I2C device address
#define LCD_WIDTH  16   // Maximum characters per line

// Define some device constants
#define LCD_CHR  1 // Mode - Sending data
#define LCD_CMD  0 // Mode - Sending command

#define LCD_LINE_1  0x80 // LCD RAM address for the 1st line
#define LCD_LINE_2  0xC0 // LCD RAM address for the 2nd line
#define LCD_LINE_3  0x94 // LCD RAM address for the 3rd line
#define LCD_LINE_4  0xD4 // LCD RAM address for the 4th line

#define LCD_BACKLIGHT  0x08  // On

#define ENABLE  0b00000100 // Enable bit

// Timing constants
#define E_PULSE  0.5
#define E_DELAY  0.5

void lcd_toggle_enable(int bits) {
  int fd;
  fd = wiringPiI2CSetup(I2C_ADDR);
  // Toggle enable
  Delay(E_DELAY);
  wiringPiI2CWrite(fd, (bits | ENABLE));
  Delay(E_PULSE);
  wiringPiI2CWrite(fd,(bits & ~ENABLE));
  Delay(E_DELAY);
};

void lcd_byte(int bits, int mode) {
  int fd;
  fd = wiringPiI2CSetup(I2C_ADDR);
  // Send byte to data pins 
  // bits = the data
  // mode = 1 for data
  //        0 for command
  int bits_high = mode | (bits & 0xF0)      | LCD_BACKLIGHT;
  int bits_low  = mode | ((bits<<4) & 0xF0) | LCD_BACKLIGHT;

  // High bits
  wiringPiI2CWrite(fd, bits_high);
  lcd_toggle_enable(bits_high);

  // Low bits
  wiringPiI2CWrite(fd, bits_low);
  lcd_toggle_enable(bits_low);
};

void lcd_init() {
  // Initialise display
  lcd_byte(0x33,LCD_CMD); // 110011 Initialise
  lcd_byte(0x32,LCD_CMD); // 110010 Initialise
  lcd_byte(0x06,LCD_CMD); // 000110 Cursor move direction
  lcd_byte(0x0C,LCD_CMD); // 001100 Display On,Cursor Off, Blink Off 
  lcd_byte(0x28,LCD_CMD); // 101000 Data length, number of lines, font size
  lcd_byte(0x01,LCD_CMD); // 000001 Clear display
  Delay(E_DELAY);
};

void main (void){
	
	
}
