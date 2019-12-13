/**
 * Derived from the Oak OLED_minimal example with additional
 * inspiration from the Adafruit GFX SSD1306 driver.
 *
 * OLED_minimal cites https://github.com/costonisp/ESP8266-I2C-OLED as
 * a major source as well.
 *
 * The MIT License (MIT)
 * Copyright (c) 2016 Brian Taylor
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#ifndef OAKOLED_H
#define OAKOLED_H

#include <Wire.h>

#include <Adafruit_GFX.h>

#define OLED_address  0x3c
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define SSD1306_CLOCK 2800000L
#define FAST_WRITE
#define OLED_BUFFER_SIZE (OLED_WIDTH * OLED_HEIGHT / 8)

#define SSD1306_COLUMNADDR 0x21
#define SSD1306_PAGEADDR   0x22
#define SSD1306_SETCONTRAST 0x81

#define VERT 0b00000001
#define HORI 0b00000000
#define PAGE 0b00000010

class OakOLED : public Adafruit_GFX {
  public:

  OakOLED();
  virtual void drawPixel(int16_t, int16_t, uint16_t);
  virtual void invertDisplay(bool);
  virtual void fillScreen(uint16_t);

  void display();

  void CommandPromPr(String);
  
  void CommandPromReset();
  
  void begin();

  void resetDisplay(void);

  void displayOn(void);

  void displayOff(void);

  void clearDisplay(void);
  
  void Contrast(unsigned char);
  
  void Phase(unsigned char, unsigned char);

  void Clock(unsigned char, unsigned char);

  void Vcom(unsigned char);

  void OffsetY(char);

  void Mirror(bool);
  
  void ScrollSetupX(bool, unsigned char, unsigned char, unsigned char);
  
  void ScrollSetupXY(bool, bool, unsigned char, unsigned char, unsigned char, unsigned char);

  void ScrollAc(bool);
  
  void sendcommand(unsigned char com);

  uint8_t buffer[OLED_BUFFER_SIZE];
#ifdef FAST_WRITE
  bool buffer_diff[OLED_HEIGHT/8][OLED_WIDTH+1];
#endif
  unsigned char cmd_line_num=OLED_HEIGHT-8;
  
  private:

  //==========================================================//
  // Inits oled and draws logo at startup
  void init_OLED(void);
  void SetMemColumn(unsigned char, unsigned char);
  void SetMemPage(unsigned char, unsigned char);
  void SetMemStartPage(unsigned char);
  void SetMemStartAddr(unsigned char);
  void SetMemMode(unsigned char);

};

#endif
