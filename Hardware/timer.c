#include "stm32f10x.h"
#include "timer.h"

/**
  *��ʱ��6 TRGO���� DAC���
	*��� Ұ��STM32 ��Ŀ���ƪ ʾ����
	*/
void TIM6_Init(u16 psc, u16 arr)
{
		/* ����DAC����Ĳ���Ƶ��
		  x = 72000000 / Ƶ��
		  ARR�Զ���װ�ؼĴ������ڵ�ֵ(��ʱʱ�䣩
		*/													  
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
	
		TIM_PrescalerConfig(TIM6, psc, TIM_PSCReloadMode_Update); //Ԥ��Ƶ=0 
		
		TIM_SetAutoreload(TIM6, arr);	            /* 22 ���100KHz�����Ҳ� */
	
		TIM_SelectOutputTrigger(TIM6, TIM_TRGOSource_Update);
}

/**
  *��ʱ��3 TRGO���� ADC����
  */
void TIM3_Init(u16 arr, u16 psc)
{
		TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
		//NVIC_InitTypeDef NVIC_InitStruct;

		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);

		TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
		TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
		TIM_TimeBaseInitStruct.TIM_Period = arr;
		TIM_TimeBaseInitStruct.TIM_Prescaler = psc;
		TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStruct);

		TIM_SelectOutputTrigger(TIM3,TIM_TRGOSource_Update); //ѡ��TIM3��TRGO����ԴΪ��ʱ������ʱ��

// 		TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);
// 		NVIC_InitStruct.NVIC_IRQChannel = TIM3_IRQn;
// 		NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
// 		NVIC_InitStruct.NVIC_IRQChannelSubPriority = 3;
// 		NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
// 		NVIC_Init(&NVIC_InitStruct);
	
		TIM_Cmd(TIM3,ENABLE);
        
}

/**
  *��ʱ��4 ͨ��3(PB8) ���벶�� ��ADCͨ��0�Ĳ���Ƶ��
  */
void TIM4_Init(u16 arr, u16 psc)
{
	TIM_ICInitTypeDef TIM4_ICInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);	//ʹ��TIM4ʱ��
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);  //ʹ��GPIOBʱ��
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_8;  //PA0 ���֮ǰ����  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //PA0 ����  
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOB,GPIO_Pin_8);						 //PA0 ����
	
  //��ʼ����ʱ��4 TIM4	 
	TIM_TimeBaseStructure.TIM_Period = arr; //�趨�������Զ���װֵ 
	TIM_TimeBaseStructure.TIM_Prescaler =psc; 	//Ԥ��Ƶ��   
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
	
	//��ʼ��TIM4���벶�����
	TIM4_ICInitStructure.TIM_Channel = TIM_Channel_3; //CC1S=01 	ѡ������� IC1ӳ�䵽TI1��
	TIM4_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//�����ز���
	TIM4_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //ӳ�䵽TI1��
	TIM4_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //���������Ƶ,����Ƶ 
	TIM4_ICInitStructure.TIM_ICFilter = 0x00;//IC1F=0000 ���������˲��� ���˲�
	TIM_ICInit(TIM4, &TIM4_ICInitStructure);
	
	//�жϷ����ʼ��
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;  //TIM4�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  //��ռ���ȼ�2��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  //�����ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);  //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ��� 
	
	TIM_ITConfig(TIM4,TIM_IT_Update|TIM_IT_CC3,ENABLE);//��������ж� ,����CC1IE�����ж�	
	
  TIM_Cmd(TIM4,ENABLE ); 	//ʹ�ܶ�ʱ��4
}
/*********��ʱ��4���жϴ�����*********************/
u8 TIM4CH3_IC_STA = 0;  //���벶��״̬
u16 TIM4CH3_IC_VAL = 0;  //һ�����ڵĳ���
void TIM4_IRQHandler(void)
{
	if((TIM4CH3_IC_STA&0X80)==0)//��δ�ɹ�����	
	{	  
		if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)		 
		{	    
			if(TIM4CH3_IC_STA&0X40)//�Ѿ�������������
			{
				if((TIM4CH3_IC_STA&0X3F)==0X3F)//�ߵ�ƽ̫����
				{
					TIM4CH3_IC_STA|=0X80;//��ǳɹ�������һ��
					TIM4CH3_IC_VAL=0X0000;
				}else TIM4CH3_IC_STA++;
			}	 
		}
		if (TIM_GetITStatus(TIM4, TIM_IT_CC3) != RESET)//����1���������¼�
		{	
			if(TIM4CH3_IC_STA&0X40)		//���ǰ���Ѿ�����һ��������	
			{	  			
				TIM4CH3_IC_STA|=0X80;		//��ǳɹ�����2��������
				TIM4CH3_IC_VAL=TIM_GetCapture3(TIM4);
			}else  								//��δ��ʼ,��һ�β���������
			{
				TIM4CH3_IC_STA=0;			//���
				TIM4CH3_IC_VAL=0;
				TIM_SetCounter(TIM4,0);
				TIM4CH3_IC_STA|=0X40;		//��ǲ�����������
			}		    
		}			     	    					   
 	}
 
  TIM_ClearITPendingBit(TIM4, TIM_IT_CC3|TIM_IT_Update); //����жϱ�־λ
}

/**
  *��ʱ��2 ͨ��2(PA1) ���벶�� ��ADCͨ��2�Ĳ���Ƶ��
  */
void TIM2_Init(u16 arr, u16 psc)
{
	TIM_ICInitTypeDef TIM2_ICInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);	//ʹ��TIM1ʱ��
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);  //ʹ��GPIOAʱ��
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_1;  //PA1 ���֮ǰ����  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //PA1 ����  
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOA,GPIO_Pin_1);						 //PA1 ����
	
  //��ʼ����ʱ��2 TIM2	 
	TIM_TimeBaseStructure.TIM_Period = arr; //�趨�������Զ���װֵ 
	TIM_TimeBaseStructure.TIM_Prescaler =psc; 	//Ԥ��Ƶ��   
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
	
	//��ʼ��TIM2���벶�����
	TIM2_ICInitStructure.TIM_Channel = TIM_Channel_2; //CC1S=01 	ѡ������� IC1ӳ�䵽TI1��
	TIM2_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//�����ز���
	TIM2_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //ӳ�䵽TI1��
	TIM2_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //���������Ƶ,����Ƶ 
	TIM2_ICInitStructure.TIM_ICFilter = 0x00;//IC1F=0000 ���������˲��� ���˲�
	TIM_ICInit(TIM2, &TIM2_ICInitStructure);
	
	//�жϷ����ʼ��
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;  //TIM2�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  //��ռ���ȼ�2��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  //�����ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);  //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ��� 
	
	TIM_ITConfig(TIM2,TIM_IT_Update|TIM_IT_CC2,ENABLE);//��������ж� ,����CC1IE�����ж�	
	
  TIM_Cmd(TIM2,ENABLE ); 	//ʹ�ܶ�ʱ��4
}
/**********��ʱ��2���жϴ�����****************/
u8 TIM2CH2_IC_STA = 0;  //���벶��״̬
u16 TIM2CH2_IC_VAL = 0;  //һ�����ڵĳ���
void TIM2_IRQHandler(void)
{
	if((TIM2CH2_IC_STA&0X80)==0)//��δ�ɹ�����	
	{	  
		if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)		 
		{	    
			if(TIM2CH2_IC_STA&0X40)//�Ѿ�������������
			{
				if((TIM2CH2_IC_STA&0X3F)==0X3F)//�ߵ�ƽ̫����
				{
					TIM2CH2_IC_STA|=0X80;//��ǳɹ�������һ��
					TIM2CH2_IC_VAL=0X0000;
				}else TIM2CH2_IC_STA++;
			}	 
		}
		if (TIM_GetITStatus(TIM2, TIM_IT_CC2) != RESET)//����1���������¼�
		{	
			if(TIM2CH2_IC_STA&0X40)		//���ǰ���Ѿ�����һ��������	
			{	  			
				TIM2CH2_IC_STA|=0X80;		//��ǳɹ�����2��������
				TIM2CH2_IC_VAL=TIM_GetCapture2(TIM2);
			}else  								//��δ��ʼ,��һ�β���������
			{
				TIM2CH2_IC_STA=0;			//���
				TIM2CH2_IC_VAL=0;
				TIM_SetCounter(TIM2,0);
				TIM2CH2_IC_STA|=0X40;		//��ǲ�����������
			}		    
		}			     	    					   
 	}
 
  TIM_ClearITPendingBit(TIM2, TIM_IT_CC2|TIM_IT_Update); //����жϱ�־λ
}






