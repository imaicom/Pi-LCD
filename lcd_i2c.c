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
#define LCD_BACKLIGHT  0x00  // Off

#define ENABLE  0b00000100 // Enable bit

// Timing constants
#define E_PULSE  0.0005
#define E_DELAY  0.0005

int lcd_byte(int bits, int mode) {
  // Send byte to data pins
  // bits = the data
  // mode = 1 for data
  //        0 for command

  bits_high = mode | (bits & 0xF0) | LCD_BACKLIGHT
  bits_low = mode | ((bits<<4) & 0xF0) | LCD_BACKLIGHT

  # High bits
  bus.write_byte(I2C_ADDR, bits_high)
  lcd_toggle_enable(bits_high)

  # Low bits
  bus.write_byte(I2C_ADDR, bits_low)
  lcd_toggle_enable(bits_low)
};

