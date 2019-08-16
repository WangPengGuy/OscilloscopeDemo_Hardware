#include "delay.h"
#include "led.h"
#include "sys.h"
#include "usart.h"
#include "timer.h"
#include "adc.h"
#include "dac.h"
#include "dma.h"
#include "dac_wave_maker.h"

#define DATA_LENGTH 1000  //ADC�Ĳ������ݳ���
u16 data_value_u16[DATA_LENGTH];   //�洢��·ADC��ȫ������ֵ��ż��λ��ADC_Channel_0������λ��ADC_Channel_2
u8 data_to_send[DATA_LENGTH*2];    //�洢��·ADC�Ĳ���ֵ ��16bit�ֽ�Ϊ����8bit ��ֵ
                                   //ż��λ�洢ADC����ֵ��8λ����ʵֻ�õ���4λ��������λ�洢��8λ
																	 //ż��λ ��8bit����7����λ��Ϊ0����ʾ��ADC����ֵ�ĸ�8λ
																	 //               ��6����λ�洢ADC����ֵ�ĵ�8λ����λ������7����λ��
																	 //               ��5����λ�洢ADC����ֵ�ĵ�8λ�ĵ�4����λ
																	 //               ��4����λ��ʾ����ֵ����һͨ���Ĳ���ֵ��0 --> ADC_Channel_0
																	 //                                                      1 --> ADC_Channel_2
																	 //����λ ��7����λ��Ϊ1����ʾ��ADC����ֵ�ĵ�8λ��
																	 //       ��4����λ��ʾ����ֵ����һͨ���Ĳ���ֵ��0 --> ADC_Channel_0
																	 //                                              1 --> ADC_Channel_2
																	 //       �������ûḲ��ԭ����ֵ�����Խ�ԭ����ֵ�洢��ż��λ�ĵ�6����λ\��5����λ
																	 //����������Ҫ��Ϊ�˷�ֹ����������������ݵĶ�ʧ
u16 data_value_Channel0_Channel2[2];  //�洢ÿ��ADCת����Ĳ���ֵ, ��һλ��ADC_Channel_0���ڶ�λ��ADC_Channel_2
extern u8 TIM4CH3_IC_STA;
extern u16 TIM4CH3_IC_VAL;
extern u8 TIM2CH2_IC_STA;
extern u16 TIM2CH2_IC_VAL;

void setADC_SampleTime(u8 sp)
{
	if(sp >= 9)
		sp=9;
	if(sp <= 1)
		sp=1;
	switch(sp)
	{
		case 1: TIM3->ARR = 9583; TIM3->PSC = 0; break;  //20ms/div
		case 2: TIM3->ARR = 4791; TIM3->PSC = 0; break;  //10ms/div
		case 3: TIM3->ARR = 2405; TIM3->PSC = 0; break;  //5ms/div
		case 4: TIM3->ARR = 959; TIM3->PSC = 0; break;   //2ms/div
		case 5: TIM3->ARR = 490; TIM3->PSC = 0; break;   //1ms/div
		case 6: TIM3->ARR = 250; TIM3->PSC = 0; break;   //500us/div
// 		case 7: TIM3->ARR = 50; TIM3->PSC = 0; break;    //200us/div
// 		case 8: TIM3->ARR = 155; TIM3->PSC = 1; break;   //100us/div
// 		case 9: TIM3->ARR = 35; TIM3->PSC = 1; break;    //50us/div
		default: TIM3->ARR = 9583; TIM3->PSC = 0; break;
	}
// 	switch(sp)
// 	{
// 		case 1: TIM3->ARR = 47915; break;
// 		case 2: TIM3->ARR = 23915; break;
// 		case 3: TIM3->ARR = 11915; break;
// 		case 4: TIM3->ARR = 4715; break;
// 		case 5: TIM3->ARR = 2315; break;
// 		case 6: TIM3->ARR = 1115; break;
// 		case 7: TIM3->ARR = 395; break;
// 		case 8: TIM3->ARR = 155; break;
// 		case 9: TIM3->ARR = 35; break;
// 		default: TIM3->ARR = 47915; break;
// 	}
}

int main(void)
{
	/****************************��������*****************************/
	u16 i = 0;
	u16 t = 0;
	u16 low_8_bit_7 = 0;      //�洢ADC����ֵ��8λ�ĵ�7����λ
	u16 low_8_bit_4 = 0;      //�洢ADC����ֵ��8λ�ĵ�4����λ
	u16 high_8_bit_7654 = 0;
	u16 low_8_bit_7_4 = 0;
	u16 TIM4CH3_IC_VAL_old = 0;
	u16 TIM2CH2_IC_VAL_old = 0;
  u32 temp1 = 0, temp2 = 0;
	/****************************�����ʼ��*******************************/
	NVIC_Configuration();
	delay_init();
	LED_Init();
	uart_init(9600);
	Adc_Init();
	Dac_Init();
	TIM2_Init(0XFFFF, 710);  //��1MHz��Ƶ�ʼ���
	TIM4_Init(0XFFFF, 710);
	TIM6_Init(0, 112);  // 72MHz/(200*72)=5KHz, ��DAC����ź�Ƶ��
	TIM3_Init(250,0);  //����ADC�Ĳ�������
	DMA_DAC_Init(); 
	DMA_ADC_Init();
	DMA_Usart1_TX_Init((u32)&data_to_send, DATA_LENGTH*2); 
	Adc_Start();
	SetupWaveType(WAVE_RECT);

	
// 	for(t=0; t<128; t++)
// 	{
// 		printf("%d\t",g_Wave[t]);
// 	}

	//DMA_Start(DMA1_Channel4);//��ʼһ��DMA����
	
	while(1)
	{ 
		if(TIM4CH3_IC_STA&0X80)
		{
			temp1 = TIM4CH3_IC_STA & 0X3F;
			temp1 *=65536;			
			temp1 += TIM4CH3_IC_VAL;		
			TIM4CH3_IC_VAL_old = temp1;			
// 		  printf("ͨ��1���ڣ�%d \r\n\r\n", TIM4CH3_IC_VAL_old);
			TIM4CH3_IC_STA = 0;
		}
		
		if(TIM2CH2_IC_STA&0X80)
		{
			temp2 = TIM2CH2_IC_STA & 0X3F;
			temp2 *= 65536;
			temp2 += TIM2CH2_IC_VAL;
			TIM2CH2_IC_VAL_old = temp2;
// 			printf("ͨ��2���ڣ�%d \r\n\r\n", TIM2CH2_IC_VAL_old);
			TIM2CH2_IC_STA = 0;
		}
			
		
		for(i=0; i<(DATA_LENGTH-6); i++)
		{
			while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC )){} //�ȴ�ת������
	    while(DMA_GetFlagStatus(DMA1_FLAG_TC1)==RESET);//�ȴ�������ɷ����һλ�������׶�ʧ 
			DMA_ClearFlag(DMA1_FLAG_TC1); //���ͨ�� 1 ������ɱ�־
			data_value_u16[i] = data_value_Channel0_Channel2[0];
//				printf("%d  ",data_value_Channel0_Channel2[0]);
			data_value_u16[++i] = data_value_Channel0_Channel2[1];
//				printf("%d  ",data_value_Channel0_Channel2[1]);
// 			Adc_value = Adc_GetValue();
// 			data_value_u16[i] = Adc_value;
// 			data_to_send[i*2] =(u8)(Adc_value>>8);
// 			data_to_send[i*2+1] = (u8) Adc_value;
			LED0 = !LED0;
		}
		//printf("\r\n");
		//Adc_voltage = Adc_value*3.3/4096;
		if(i>=(DATA_LENGTH-6))
		{
			for(t=0; t<(DATA_LENGTH-6); t++ )
			{
				low_8_bit_7 = ( data_value_u16[i] & (0x0080)) >> 7;  //0x0080=0000000010000000,�õ���8λ����λ
				low_8_bit_4 = ( data_value_u16[i] & (0x0010)) >> 4;  //0x0010=0000000000010000,�õ���8λ�ĵ�4λ
				if(t%2 == 0) 
				{
					high_8_bit_7654 = (0<<7)|(low_8_bit_7<<6)|(low_8_bit_4<<5)|(0<<4);
					               // ��7λ��0������Ϊ��8λ
												 //         ��6λ�洢��8λ����λ
												 //                         ��5λ�洢��8λ�ĵ�4λ
					               //                                          ��4λ��0��������ADC_Channel_0�Ĳ���ֵ
					low_8_bit_7_4  = (1<<7)|(0<<4);
					               // ��7λ��1������Ϊ��8λ
					               //       ��4λ��0��������ADC_Channel_0�Ĳ���ֵ
				}
				else
				{
					high_8_bit_7654 = (0<<7)|(low_8_bit_7<<6)|(low_8_bit_4<<5)|(1<<4);
					               // ��7λ��0������Ϊ��8λ
												 //         ��6λ�洢��8λ����λ
												 //                         ��5λ�洢��8λ�ĵ�4λ
					               //                                          ��4λ��1��������ADC_Channel_2�Ĳ���ֵ					
					low_8_bit_7_4  = (1<<7)|(1<<4);
										     // ��7λ��1������Ϊ��8λ
					               //       ��4λ��0��������ADC_Channel_2�Ĳ���ֵ
				}
				data_to_send[t*2] = (u8) ((data_value_u16[t]>>8)|high_8_bit_7654);  
				data_to_send[t*2+1] = (u8) ((data_value_u16[t]&0x6f)|low_8_bit_7_4);    //0x6f=01101111
			}
		  //��Ƶ��ֵ�ӽ�ȥ
			data_to_send[DATA_LENGTH-6] = 0xff;
			data_to_send[DATA_LENGTH-5] = 0xff;
			data_to_send[DATA_LENGTH-4] = TIM4CH3_IC_VAL_old >> 8;
			data_to_send[DATA_LENGTH-3] = TIM4CH3_IC_VAL_old;
			data_to_send[DATA_LENGTH-2] = TIM2CH2_IC_VAL_old >> 8;
			data_to_send[DATA_LENGTH-1] = TIM2CH2_IC_VAL_old;
			
			DMA_ADvalue_to_Usart_Start();//��ʼһ��DMA����			
			i = 0;
			
// 			printf("\r\n");
			
// 			for(i=0;i<DATA_LENGTH;i++)
// 			{
// 				printf("%d\t",data_value_u16[i]);				
// 			}
			
// 			printf("\r\n");
// 			for(i=0;i<DATA_LENGTH*2;i++)
// 			{
// 				printf("%d\t",data_to_send[i]);				
// 			}
			//while(1);
		} 
	
		//printf("%f\r\n", Adc_voltage);
// 		if((TIM4CH3_IC_STA&0X80))
// 		{
// 			TIM4CH3_IC_STA = 0;
// 		}
// 		if((TIM2CH2_IC_STA&0X80))
// 		{
// 			TIM2CH2_IC_STA = 0;
// 		}
		LED0 = !LED0;
		//delay_ms(50);
	}
}
