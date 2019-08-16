#ifndef __DMA_H
#define __DMA_H

void DMA_Usart1_TX_Init(u32 memory_base_addr, u16 buffer_size);

void DMA_ADvalue_to_Usart_Start(void);

void DMA_DAC_Init(void);

void DMA_ADC_Init(void);

#endif
