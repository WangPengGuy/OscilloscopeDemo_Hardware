#ifndef __DAC_H
#define __DAC_H

#define DAC_DHR8R1_Address      0x40007410
#define DAC_DHR12R1_Address		0x40007408

#define WAVE_SIN 1
#define WAVE_RECT 2



/**************************º¯ÊýÉùÃ÷***************************/
void Dac_Init(void);
void DAC_Mode_Init(void);
void DAC_GPIO_Init(void);

#endif
