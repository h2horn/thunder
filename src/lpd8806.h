#ifndef __LED_TEST_H__
#define __LED_TEST_H__

#include <avr/io.h>

void writeLatch(void);
void spi_send(uint8_t b);
void showLed(void);
void setPixelRGB(uint16_t n, uint8_t r, uint8_t g, uint8_t b);
void setPixelColor(uint16_t n, uint32_t c);
uint32_t wheel(uint32_t WheelPos);
uint32_t color(uint8_t r, uint8_t g, uint8_t b);
void rainbow(void);

#endif // __LED_TEST_H__