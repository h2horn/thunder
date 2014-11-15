#ifndef _DMA_H
	#define _DMA_H

	#include <avr/io.h>

	void dma_init(void);
	void dma_start(DMA_CH_t *dma);

#endif /* _DMA_H */