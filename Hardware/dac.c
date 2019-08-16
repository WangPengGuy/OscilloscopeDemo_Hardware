#include "stm32f10x.h"
#include "dac.h"

void DAC_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIOA, GPIO_Pin_4);
}

void DAC_Mode_Init(void)
{
	DAC_InitTypeDef DAC_InitType;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);
	
	DAC_InitType.DAC_Trigger = DAC_Trigger_T6_TRGO;   //��ʱ��6 TRGO����
	DAC_InitType.DAC_WaveGeneration = DAC_WaveGeneration_None;	   //�������ǲ�����ʹ�ܹر�
	DAC_InitType.DAC_LFSRUnmask_TriangleAmplitude = DAC_LFSRUnmask_Bit0;  //����/��ֵѡ����
	DAC_InitType.DAC_OutputBuffer = DAC_OutputBuffer_Disable;  //DAC1�������ر�
	DAC_Init(DAC_Channel_1, &DAC_InitType);
		/* Enable DAC Channel1: Once the DAC channel1 is enabled, PA.04 is
	 automatically connected to the DAC converter. */
	 DAC_Cmd(DAC_Channel_1, ENABLE);		 //DACͨ��1ʹ��
	 /* Enable DMA for DAC Channel1 */
	 DAC_DMACmd(DAC_Channel_1, ENABLE);		 //DACͨ��1DMA����
	DAC_SetChannel1Data(DAC_Align_12b_R,0);
}

void Dac_Init(void)
{
	DAC_GPIO_Init();
	DAC_Mode_Init();
}



