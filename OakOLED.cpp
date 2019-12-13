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

#include "OakOLED.h"
#include <Arduino.h>

OakOLED::OakOLED()
  : Adafruit_GFX(OLED_WIDTH, OLED_HEIGHT) {
  memset(buffer, OLED_BUFFER_SIZE, 0);
#ifdef FAST_WRITE
  memset(buffer_diff, 0, sizeof(buffer_diff[0][0]) * ((OLED_HEIGHT/8) * (OLED_WIDTH+1)));
  memset(buffer_diff, 1, sizeof(buffer_diff[0][0]) * ((OLED_HEIGHT/8) * (OLED_WIDTH+1)));
#endif
}

void OakOLED::drawPixel(int16_t x, int16_t y, uint16_t c) {
  if(x > OLED_WIDTH-2 || y > OLED_HEIGHT-2)
	return;
  uint16_t idx = x + (y / 8) * OLED_WIDTH;
  uint8_t bit = y % 8;

#ifdef FAST_WRITE
  bool old_st = (buffer[idx] >> bit) & 1u;
#endif  
  if(c == 2)
	  buffer[idx] |= ((!bool((buffer[idx] >> bit) & 1u)) << bit);
  else if(c) {
    buffer[idx] |= (1 << bit);
  } else {
    buffer[idx] &= ~(1 << bit);
  }
  
#ifdef FAST_WRITE
if(old_st != bool(c)) {
  if(y > 0 && y <= 8)
	  buffer_diff[0][x]=1;
  else if(y > 8 && y <= 16)
	  buffer_diff[1][x]=1;
  else if(y > 16 && y <= 24)
	  buffer_diff[2][x]=1;
  else if(y > 24 && y <= 32)
	  buffer_diff[3][x]=1;
  else if(y > 32 && y <= 40)
	  buffer_diff[4][x]=1;
  else if(y > 40 && y <= 48)
	  buffer_diff[5][x]=1;
  else if(y > 48 && y <= 56)
	  buffer_diff[6][x]=1;
  else if(y > 56 && y <= 64)
	  buffer_diff[7][x]=1;
}
#endif
}

void OakOLED::SetMemMode(unsigned char mode) {
  sendcommand(0b00100000);   
  sendcommand(mode);   
} 

void OakOLED::SetMemColumn(unsigned char start, unsigned char end) {
  sendcommand(0b00100001);   
  sendcommand(start);   
  sendcommand(end);   
}

void OakOLED::SetMemPage(unsigned char start, unsigned char end) {
  sendcommand(0b00100010);   
  sendcommand(start);   
  sendcommand(end);   
}

void OakOLED::SetMemStartPage(unsigned char start) {
  sendcommand(0b10110000 | start);   
}

void OakOLED::SetMemStartAddr(unsigned char start) {
  sendcommand(0b00000000 | start & 0x0F);   
  sendcommand(0b00010000 | ((start & 0xF0) >> 4));   
}

//void OakOLED::SetMemHighStartAddr(unsigned char start) {
//  sendcommand(0b00010000 | start);   
//}

void OakOLED::CommandPromPr(String str)
{
    fillRect(0,cmd_line_num,OLED_WIDTH,8,0);
    setCursor(0,cmd_line_num);
    print(str);
    display();
    OffsetY(cmd_line_num);
    if(cmd_line_num==0)
      cmd_line_num=OLED_HEIGHT;
    cmd_line_num=cmd_line_num-8;
}

void OakOLED::CommandPromReset()
{
    fillScreen(0);
    setCursor(0,0);
    OffsetY(0);
    display();
    cmd_line_num=OLED_HEIGHT-8;
}

void OakOLED::invertDisplay(bool i)
{
	sendcommand(0b10100110 | i);
}

void OakOLED::display() {
#ifdef FAST_WRITE
  unsigned char i=0;
  for(unsigned char ii = 0; ii < OLED_HEIGHT/8; ii++)
  {
	  for(unsigned char jj = 0; jj < OLED_WIDTH; jj++)
	  {
		  if(buffer_diff[ii][jj] == 1)
		  {
			SetMemStartPage(ii); //Y
			SetMemStartAddr(jj); //X
			Wire.beginTransmission(OLED_address);
			Wire.write(0b01000000);
			for(i =0; ; i++)	
			{
			  buffer_diff[ii][jj] == 0;
			  Wire.write(buffer[(ii*OLED_WIDTH) + jj]);
			  if(buffer_diff[ii][jj+1] != 1 || i == 30 || jj == OLED_WIDTH-1)
				  break;
			  jj++;
			}
			Wire.endTransmission();
			//Serial.println("x="+String(jj,DEC)+"y="+String(ii,DEC));
			//delay(20);
		  }
	  }
  }
  
  #if defined(ESP8266)
  yield();
  #endif

  //SetMemMode(HORI);
#else
  sendcommand(SSD1306_COLUMNADDR);
  sendcommand(0); // start at column == 0
  sendcommand(OLED_WIDTH - 1); // end at column == 127

  sendcommand(SSD1306_PAGEADDR);
  sendcommand(0); // start at page == 0
  sendcommand(7); // end at page == 7

  #if defined(ESP8266)
  yield();
  #endif
  // we can go by 16s
  for(uint16_t ii = 0; ii < OLED_BUFFER_SIZE; ii+=16) {
    Wire.beginTransmission(OLED_address);
    Wire.write(0x40);
    for(uint16_t jj = 0; jj < 16; ++jj) {
      Wire.write(buffer[ii + jj]);
    }

    Wire.endTransmission();
  }
#endif
}

void OakOLED::begin() {
  Wire.begin();
  Wire.setClock(SSD1306_CLOCK);
  init_OLED();
  resetDisplay();
  clearDisplay();
}

void OakOLED::ScrollSetupX(bool direct = 0, unsigned char ps = 0b00000000, unsigned char pe  = 0b00000111, unsigned char ss = 0b00000000) {
	sendcommand(0b00100110 | direct);
	sendcommand(0b00000000);
	sendcommand(ps);
	sendcommand(ss);
	sendcommand(pe);
	sendcommand(0b00000000);
	sendcommand(0b11111111);
}

void OakOLED::ScrollSetupXY(bool direct1 = 0, bool direct2 = 0, unsigned char ps = 0b00000000, unsigned char pe  = 0b00000111, unsigned char ss = 0b00000000, unsigned char ee = 0b00000000) {
	sendcommand((0b00101000 | direct1) | direct2 << 1);
	sendcommand(0b00000000);
	sendcommand(ps);
	sendcommand(ss);
	sendcommand(pe);
	sendcommand(0b00000000);
	sendcommand(ee);
}

void OakOLED::ScrollAc(bool state = true)
{
	sendcommand(0b00101110 | state);
}

void OakOLED::Contrast(unsigned char contr) {
  sendcommand(0x81);   
  sendcommand(contr);  //0-255 
} 

void OakOLED::Phase(unsigned char ph1, unsigned char ph2) {
  sendcommand(0xD9);   
  sendcommand((ph1 << 4) | ph2);   //1-15
}

void OakOLED::Clock(unsigned char clock, unsigned char delim) {
  sendcommand(0xD5);   
  sendcommand(delim | (clock << 4));
}

void OakOLED::Vcom(unsigned char contr) {
  sendcommand(0xDB);   
  sendcommand(contr << 4);   
}

void OakOLED::OffsetY(char offset) {
  sendcommand(0xD3);   
  sendcommand(offset);   
}

void OakOLED::Mirror(bool st) {
	sendcommand(0b10100000 | st);
}

void OakOLED::fillScreen(uint16_t color) {
  memset(buffer, bool(color), OLED_BUFFER_SIZE);
#ifdef FAST_WRITE
  memset(buffer_diff, 1, sizeof(buffer_diff[0][0]) * ((OLED_HEIGHT/8) * OLED_WIDTH));
#endif
}

//==========================================================//
// Resets display depending on the actual mode.
void OakOLED::resetDisplay(void)
{
  displayOff();
  clearDisplay();
  displayOn();
}

//==========================================================//
// Turns display on.
void OakOLED::displayOn(void)
{
  sendcommand(0xaf);        //display on
}

//==========================================================//
// Turns display off.
void OakOLED::displayOff(void)
{
  sendcommand(0xae);    //display off
}

//==========================================================//
// Clears the display by sendind 0 to all the screen map.
void OakOLED::clearDisplay(void)
{
  memset(buffer, 0, OLED_BUFFER_SIZE);
#ifdef FAST_WRITE
  memset(buffer_diff, 1, sizeof(buffer_diff[0][0]) * ((OLED_HEIGHT/8) * (OLED_WIDTH+1)));
#endif
}

//==========================================================//
// Used to send commands to the display.
void OakOLED::sendcommand(unsigned char com)
{
  Wire.beginTransmission(OLED_address);     //begin transmitting
  Wire.write(0x80);                          //command mode
  Wire.write(com);
  Wire.endTransmission();                    // stop transmitting
}

//==========================================================//
// Inits oled and draws logo at startup
void OakOLED::init_OLED(void)
{
  sendcommand(0xae);    //display off
  sendcommand(0xa6);            //Set Normal Display (default)

  // Adafruit Init sequence for 128x64 OLED module
  sendcommand(0xAE);             //DISPLAYOFF
  sendcommand(0xD5);            //SETDISPLAYCLOCKDIV
  sendcommand(0x80);            // the suggested ratio 0x80
  sendcommand(0xA8);            //SSD1306_SETMULTIPLEX
  sendcommand(0x3F);
  sendcommand(0xD3);            //SETDISPLAYOFFSET
  sendcommand(0x0);             //no offset
  sendcommand(0x40 | 0x0);      //SETSTARTLINE
  sendcommand(0x8D);            //CHARGEPUMP
  sendcommand(0x14);
  sendcommand(0x20);             //MEMORYMODE
  sendcommand(0x00);             //0x0 act like ks0108

  //sendcommand(0xA0 | 0x1);      //SEGREMAP   //Rotate screen 180 deg
  sendcommand(0xA0);

  //sendcommand(0xC8);            //COMSCANDEC  Rotate screen 180 Deg
  sendcommand(0xC0);

  sendcommand(0xDA);            //0xDA
  sendcommand(0x12);           //COMSCANDEC
  sendcommand(0x81);           //SETCONTRAS
  sendcommand(0xCF);           //
  sendcommand(0xd9);          //SETPRECHARGE
  sendcommand(0xF1);
  sendcommand(0xDB);        //SETVCOMDETECT
  sendcommand(0x40);
  sendcommand(0xA4);        //DISPLAYALLON_RESUME
  sendcommand(0xA6);        //NORMALDISPLAY

  clearDisplay();
  sendcommand(0x2e);            // stop scroll
  //----------------------------REVERSE comments----------------------------//
  sendcommand(0xa0);    //seg re-map 0->127(default)
  sendcommand(0xa1);    //seg re-map 127->0
  sendcommand(0xc8);
  delay(1000);
  //----------------------------REVERSE comments----------------------------//
  // sendcommand(0xa7);  //Set Inverse Display
  // sendcommand(0xae);   //display off
#ifdef FAST_WRITE
  SetMemMode(PAGE);
#else
  SetMemMode(HORI);
#endif
  //sendcommand(0x20);            //Set Memory Addressing Mode
  //sendcommand(0x00);            //Set Memory Addressing Mode ab Horizontal addressing mode
  //  sendcommand(0x02);         // Set Memory Addressing Mode ab Page addressing mode(RESET)

  sendcommand(SSD1306_SETCONTRAST);
  sendcommand(0xff);
}
