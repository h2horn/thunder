#include "dma.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 32000000UL
#include <util/delay.h>
#include "enc28j60.h"
#include "spi.h"

void dma_spi_init(void);
void spi_init_channel(USART_t *usart, PORT_t *port, uint8_t clk, uint8_t dat);
void dma_init_channel(DMA_CH_t *dma, uint16_t src, uint16_t dest, uint8_t trigsrc);

uint8_t buffer[480];

void dma_init() {
	dma_spi_init();

	// Enable DMA Controller
	// DMA.CTRL = DMA_CH_ENABLE_bm;
// &SPIC.DATA
	// dma_init_channel(&DMA.CH0, (uint16_t)&SPIC.DATA, (uint16_t)&USARTC0.DATA, DMA_CH_TRIGSRC_USARTC0_DRE_gc);
	// dma_init_channel(&DMA.CH1, (uint16_t)&SPIC.DATA, (uint16_t)&USARTD0.DATA, DMA_CH_TRIGSRC_USARTD0_DRE_gc);
	// dma_init_channel(&DMA.CH2, (uint16_t)&SPIC.DATA, (uint16_t)&USARTD1.DATA, DMA_CH_TRIGSRC_USARTD1_DRE_gc);
	// dma_init_channel(&DMA.CH3, (uint16_t)&SPIC.DATA, (uint16_t)&USARTE0.DATA, DMA_CH_TRIGSRC_USARTE0_DRE_gc);

	// dma_start(&DMA.CH0);
	// dma_start(&DMA.CH1);
	// dma_start(&DMA.CH2);
	// dma_start(&DMA.CH3);
}

/*!\brief Init DMA Channel
 * \param dma 			DMA Channel
 * \param src 			Source Address
 * \param dest 			Destination Address
 * \param trigsrc		Trigger Source
 */
void dma_init_channel(DMA_CH_t *dma, uint16_t src, uint16_t dest, uint8_t trigsrc) {
	dma->CTRLA = DMA_CH_BURSTLEN_1BYTE_gc | DMA_CH_SINGLE_bm;
	// Modus Transaktion, increase und decrease, Burst
	dma->ADDRCTRL = DMA_CH_SRCRELOAD_BLOCK_gc | DMA_CH_SRCDIR_INC_gc | 
				   DMA_CH_DESTRELOAD_NONE_gc | DMA_CH_DESTDIR_FIXED_gc;
	// Trigger Source
	dma->TRIGSRC = trigsrc;
	// Buffer 160 * 3
	dma->TRFCNT = 480;
	// Source
	dma->SRCADDR0  = (src >> 0) & 0xFF;
	dma->SRCADDR1  = (src >> 8) & 0xFF;
	dma->SRCADDR2  = 0;
	// Destination
	dma->DESTADDR0 = (dest >> 0) & 0xFF;
	dma->DESTADDR1 = (dest >> 8) & 0xFF;
	dma->DESTADDR2 = 0;
	// Enable Interrupt
	dma->CTRLB = DMA_CH_TRNINTLVL_LO_gc;
}

void dma_spi_init() {
	spi_init_channel(&USARTC0, &PORTC, 1, 3);
	spi_init_channel(&USARTD0, &PORTD, 1, 3);
	spi_init_channel(&USARTD1, &PORTD, 5, 7);
	spi_init_channel(&USARTE0, &PORTE, 1, 3);
}

/*!\brief Init USART SPI Channel
 * \param usart 		USART Register
 * \param port 			Port des USARTs
 * \param clk 			Clock Pin
 * \param dat 			Data Pin
 */
void spi_init_channel(USART_t *usart, PORT_t *port, uint8_t clk, uint8_t dat) {
	// Set MOSI and SCK as Output
	port->DIRSET |= (1 << clk) | (1 << dat);
	// 0 = 16 Mhz / 8 = 2Mhz SPI
	usart->BAUDCTRLA = 0;
	usart->BAUDCTRLB = 0;
	// No Interrupts
	usart->CTRLA = 0;
	// TX 
	usart->CTRLB = USART_TXEN_bm;
	// MSB, SPI Mode 0
	usart->CTRLC = USART_CMODE_MSPI_gc;
}

/*!\brief Start DMA Transfer
 * \param dma 			DMA Channel
 */
void dma_start(DMA_CH_t *dma) {
	// Buffer 160 * 3
	// dma.TRFCNT = 480;
	dma->CTRLA |= DMA_CH_ENABLE_bm;
}

//Interruptroutine wird aufgerufen wenn Transfer fertig
ISR (DMA_CH0_vect) {
	// Write Latch ((160 + 63) / 64) * 3
	USARTC0.DATA = 0x00;
	while(!(USARTC0.STATUS & USART_DREIF_bm));

	_delay_us( 1 );

	ENC28J60_CONTROL_PORT.OUTSET = (1<<ENC28J60_CONTROL_CS);

	enc28j60PacketFinished();

	// Clear Interrupt Flags
	DMA.CH0.CTRLB |= DMA_CH_TRNIF_bm | DMA_CH_ERRIF_bm;
}

//Interruptroutine wird aufgerufen wenn Transfer fertig
ISR (DMA_CH1_vect) {
	// Write Latch ((160 + 63) / 64) * 3
	USARTD0.DATA = 0x00;
	while(!(USARTD0.STATUS & USART_DREIF_bm));

	// enc28j60PacketFinished();

	// Clear Interrupt Flags
	DMA.CH1.CTRLB |= DMA_CH_TRNIF_bm | DMA_CH_ERRIF_bm;
}

//Interruptroutine wird aufgerufen wenn Transfer fertig
ISR (DMA_CH2_vect) {
	// Write Latch ((160 + 63) / 64) * 3
	USARTD1.DATA = 0x00;
	while(!(USARTD1.STATUS & USART_DREIF_bm));

	// enc28j60PacketFinished();

	// Clear Interrupt Flags
	DMA.CH2.CTRLB |= DMA_CH_TRNIF_bm | DMA_CH_ERRIF_bm;
}

//Interruptroutine wird aufgerufen wenn Transfer fertig
ISR (DMA_CH3_vect) {
	// Write Latch ((160 + 63) / 64) * 3
	USARTE0.DATA = 0x00;
	while(!(USARTE0.STATUS & USART_DREIF_bm));

	// enc28j60PacketFinished();

	// Clear Interrupt Flags
	DMA.CH3.CTRLB |= DMA_CH_TRNIF_bm | DMA_CH_ERRIF_bm;
}