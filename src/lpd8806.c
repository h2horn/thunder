#include "lpd8806.h"
#include <avr/io.h>
#define F_CPU 32000000UL
#include <util/delay.h>

uint8_t buffer[480];
uint16_t leds = 160;

void setPixelRGB(uint16_t n, uint8_t r, uint8_t g, uint8_t b) {
	uint8_t *p = &buffer[n * 3];
	*p++ = g | 0x80;
	*p++ = r | 0x80;
	*p++ = b | 0x80;
}

void setPixelColor(uint16_t n, uint32_t c) {
	uint8_t *p = &buffer[n * 3];
	*p++ = (c >> 16) | 0x80;
	*p++ = (c >> 8) | 0x80;
	*p++ = c | 0x80;
}

uint32_t wheel(uint32_t WheelPos) {
  uint8_t r = 0, g = 0, b = 0;
  switch(WheelPos / 128)
  {
    case 0:
      r = 127 - WheelPos % 128; //Red down
      g = WheelPos % 128; // Green up
      b = 0; //blue off
      break;
    case 1:
      g = 127 - WheelPos % 128; //green down
      b = WheelPos % 128; //blue up
      r = 0; //red off
      break;
    case 2:
      b = 127 - WheelPos % 128; //blue down
      r = WheelPos % 128; //red up
      g = 0; //green off
      break;
  }
  return(color(r,g,b));
}

uint32_t color(uint8_t r, uint8_t g, uint8_t b) {
	return 0x808080 | ((uint32_t)g << 16) | ((uint32_t)r << 8) | (uint32_t)b;
}

void showLed() {
  for(uint16_t i = 0; i < 480; i++) {
    spi_send(buffer[i]);
  }
  writeLatch();
  _delay_ms(1);
}

void writeLatch() {
  for(uint16_t n = ((160 + 63) / 64) * 3; n > 0; n--) {
    spi_send(0x00);
  }
}

void spi_send(uint8_t b) {
  USARTC0.DATA = b;
  USARTD0.DATA = b;
  USARTD1.DATA = b;
  USARTE0.DATA = b;
  while(!(USARTC0.STATUS & USART_DREIF_bm));
  while(!(USARTD0.STATUS & USART_DREIF_bm));
  while(!(USARTD1.STATUS & USART_DREIF_bm));
  while(!(USARTE0.STATUS & USART_DREIF_bm));
}

void rainbow() { 
  for (int i = 0; i < leds * 3; ++i) {
    setPixelRGB(i, 0, 0, 0);
  }
  showLed();
  for (int i = 0; i < 384; ++i) {
    for (int j = 0; j < leds; ++j) {
      setPixelColor(i, wheel(((j * 384 / leds) + i) % 384));
    }
    showLed();
    //    delay(1);
  }
}