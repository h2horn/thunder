#include <avr/io.h>
#define F_CPU 32000000UL
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "lpd8806.h"
#include "ethernet.h"
#include "dma.h"
#include <stddef.h>
#include <avr/pgmspace.h> 

// #define _DEBUG_
void uart_init(void);
void adc_init(void);

#define BAUD_VAL 115200						// baudrate fuer usart-uebertragung
#define BSEL_VAL ((F_CPU/(16*BAUD_VAL))-1)	// berechnung fuer baudraten-registerwert

#ifdef _DEBUG_
	static int uart_write_char(char c, FILE *stream);
	static FILE mystdout = FDEV_SETUP_STREAM (uart_write_char, NULL, _FDEV_SETUP_WRITE);
#endif

// Prototypes
void clock_init(void);

// Vars
uint8_t buffer[480];
uint8_t state = 0;

int main(void) {
	PORTA.DIR = 0xFF;
	clock_init();

	// Interrupt init
	PMIC.CTRL |= PMIC_HILVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_LOLVLEN_bm;
	sei();

	dma_init();
	writeLatch();

#ifdef _DEBUG_
	uart_init();    
	stdout = &mystdout;
#endif

	ethernet_init();

#ifdef _DEBUG_
	printf("Hello, world!\n");
#endif

	// adc_init();

	PORTA.DIR = 0xFF;
	PORTA.OUT = 0xFF;
	// PORTA.OUTTGL = 0xFF;
	// writeLatch();

	// USARTD0.DATA = 0x00;
	// while(!(USARTD0.STATUS & USART_DREIF_bm));

	// showLed();
	// writeLatch();
	// showLed();
	// writeLatch();
	// _delay_ms(2000);
	// showLed();
	// writeLatch();

	while (1) {
		check_ethernet();
		// PORTA.OUTTGL = 0xFF;
		// printf("T %u\n", ADCA.CH0RES);
		// printf("Temp %u C\n", ADCA.CH0RES * 85 / 2642);
		// _delay_ms(5000);

	}
}

void clock_init() {
	// 32Mhz
	OSC.CTRL |= OSC_RC32MEN_bm;
	// Warten bis Oszillator
	while(!(OSC.STATUS & OSC_RC32MRDY_bm));

	//Configuration Change Protection Register
	CCP = 0xD8;
	//Select 32Mhz OSC
	CLK.CTRL = (CLK.CTRL & ~CLK_SCLKSEL_gm) | CLK_SCLKSEL_RC32M_gc;
}

void uart_init() {
	PORTC.OUT = (0<<PIN2) | (1<<PIN3);	// laut datenblatt TX-pin auf high setzen
	PORTC.DIR = (0<<PIN2) | (1<<PIN3);	// RX-pin als eingang, TX-pin als ausgang
	
	USARTC0.BAUDCTRLB = 0;			// wert passt in 8bit-register, also B-register und skalierungsfaktor 0
	USARTC0.BAUDCTRLA = BSEL_VAL;	// baudratenwert in register schreiben
	
	USARTC0.CTRLA = 0;		// keine interrupt oder sonstiges
	USARTC0.CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc | USART_CHSIZE_8BIT_gc;	// modus: asynchron | parity: keines | stop-bits: 1 | character-size: 8bit
	USARTC0.CTRLB = USART_RXEN_bm | USART_TXEN_bm;	// empfaenger und transmitter aktivieren
	
	return;
}

#ifdef _DEBUG_
static int uart_write_char(char c, FILE *stream) {
	if (c == '\n')
        uart_write_char('\r', stream);

	while(!(USARTC0.STATUS & USART_DREIF_bm)) {}	// warten, bis das datenregister leer ist
	USARTC0.DATA = c;		// daten senden	

	return 0;
}
#endif

void adc_init() {
	ADCA.CTRLB = ADC_CONMODE_bm | ADC_RESOLUTION_12BIT_gc | ADC_FREERUN_bm;
	ADCA.REFCTRL = ADC_REFSEL_INT1V_gc | ADC_TEMPREF_bm;
	ADCA.EVCTRL = ADC_SWEEP_0_gc;
	ADCA.PRESCALER = ADC_PRESCALER_DIV512_gc;
	ADCA.CAL = 0x01FF; /* ADCACAL0 : ADCACAL1 */
	ADCA.CH0.CTRL = ADC_CH_GAIN_1X_gc | ADC_CH_INPUTMODE_INTERNAL_gc;
	ADCA.CH0.MUXCTRL = ADC_CH_MUXINT_TEMP_gc;
	ADCA.CTRLA = ADC_ENABLE_bm;
	ADCA.CH0.CTRL = ADC_CH_START_bm;

#ifdef _DEBUG_
	int tcal;
	NVM_CMD = NVM_CMD_READ_CALIB_ROW_gc;
	tcal = pgm_read_byte(offsetof(NVM_PROD_SIGNATURES_t, TEMPSENSE1)) << 8;
	tcal |= pgm_read_byte(offsetof(NVM_PROD_SIGNATURES_t, TEMPSENSE0));
	NVM_CMD = NVM_CMD_NO_OPERATION_gc;
	printf("Temp Cal Val %u \n", tcal);
#endif
}
