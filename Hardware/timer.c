#include "stm32f10x.h"
#include "timer.h"

/**
  *定时器6 TRGO触发 DAC输出
	*详见 野火STM32 项目提高篇 示波器
	*/
void TIM6_Init(u16 psc, u16 arr)
{
		/* 决定DAC输出的采样频率
		  x = 72000000 / 频率
		  ARR自动重装载寄存器周期的值(定时时间）
		*/													  
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
	
		TIM_PrescalerConfig(TIM6, psc, TIM_PSCReloadMode_Update); //预分频=0 
		
		TIM_SetAutoreload(TIM6, arr);	            /* 22 输出100KHz的正弦波 */
	
		TIM_SelectOutputTrigger(TIM6, TIM_TRGOSource_Update);
}

/**
  *定时器3 TRGO触发 ADC采样
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

		TIM_SelectOutputTrigger(TIM3,TIM_TRGOSource_Update); //选择TIM3的TRGO触发源为计时器更新时间

// 		TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);
// 		NVIC_InitStruct.NVIC_IRQChannel = TIM3_IRQn;
// 		NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
// 		NVIC_InitStruct.NVIC_IRQChannelSubPriority = 3;
// 		NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
// 		NVIC_Init(&NVIC_InitStruct);
	
		TIM_Cmd(TIM3,ENABLE);
        
}

/**
  *定时器4 通道3(PB8) 输入捕获 测ADC通道0的波形频率
  */
void TIM4_Init(u16 arr, u16 psc)
{
	TIM_ICInitTypeDef TIM4_ICInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);	//使能TIM4时钟
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);  //使能GPIOB时钟
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_8;  //PA0 清除之前设置  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //PA0 输入  
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOB,GPIO_Pin_8);						 //PA0 下拉
	
  //初始化定时器4 TIM4	 
	TIM_TimeBaseStructure.TIM_Period = arr; //设定计数器自动重装值 
	TIM_TimeBaseStructure.TIM_Prescaler =psc; 	//预分频器   
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
	
	//初始化TIM4输入捕获参数
	TIM4_ICInitStructure.TIM_Channel = TIM_Channel_3; //CC1S=01 	选择输入端 IC1映射到TI1上
	TIM4_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//上升沿捕获
	TIM4_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //映射到TI1上
	TIM4_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //配置输入分频,不分频 
	TIM4_ICInitStructure.TIM_ICFilter = 0x00;//IC1F=0000 配置输入滤波器 不滤波
	TIM_ICInit(TIM4, &TIM4_ICInitStructure);
	
	//中断分组初始化
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;  //TIM4中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  //先占优先级2级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  //从优先级0级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器 
	
	TIM_ITConfig(TIM4,TIM_IT_Update|TIM_IT_CC3,ENABLE);//允许更新中断 ,允许CC1IE捕获中断	
	
  TIM_Cmd(TIM4,ENABLE ); 	//使能定时器4
}
/*********定时器4的中断处理函数*********************/
u8 TIM4CH3_IC_STA = 0;  //输入捕获状态
u16 TIM4CH3_IC_VAL = 0;  //一个周期的长度
void TIM4_IRQHandler(void)
{
	if((TIM4CH3_IC_STA&0X80)==0)//还未成功捕获	
	{	  
		if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)		 
		{	    
			if(TIM4CH3_IC_STA&0X40)//已经捕获到上升沿了
			{
				if((TIM4CH3_IC_STA&0X3F)==0X3F)//高电平太长了
				{
					TIM4CH3_IC_STA|=0X80;//标记成功捕获了一次
					TIM4CH3_IC_VAL=0X0000;
				}else TIM4CH3_IC_STA++;
			}	 
		}
		if (TIM_GetITStatus(TIM4, TIM_IT_CC3) != RESET)//捕获1发生捕获事件
		{	
			if(TIM4CH3_IC_STA&0X40)		//如果前面已经捕获到一次上升沿	
			{	  			
				TIM4CH3_IC_STA|=0X80;		//标记成功捕获到2次上升沿
				TIM4CH3_IC_VAL=TIM_GetCapture3(TIM4);
			}else  								//还未开始,第一次捕获上升沿
			{
				TIM4CH3_IC_STA=0;			//清空
				TIM4CH3_IC_VAL=0;
				TIM_SetCounter(TIM4,0);
				TIM4CH3_IC_STA|=0X40;		//标记捕获到了上升沿
			}		    
		}			     	    					   
 	}
 
  TIM_ClearITPendingBit(TIM4, TIM_IT_CC3|TIM_IT_Update); //清除中断标志位
}

/**
  *定时器2 通道2(PA1) 输入捕获 测ADC通道2的波形频率
  */
void TIM2_Init(u16 arr, u16 psc)
{
	TIM_ICInitTypeDef TIM2_ICInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);	//使能TIM1时钟
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);  //使能GPIOA时钟
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_1;  //PA1 清除之前设置  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //PA1 输入  
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOA,GPIO_Pin_1);						 //PA1 下拉
	
  //初始化定时器2 TIM2	 
	TIM_TimeBaseStructure.TIM_Period = arr; //设定计数器自动重装值 
	TIM_TimeBaseStructure.TIM_Prescaler =psc; 	//预分频器   
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
	
	//初始化TIM2输入捕获参数
	TIM2_ICInitStructure.TIM_Channel = TIM_Channel_2; //CC1S=01 	选择输入端 IC1映射到TI1上
	TIM2_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//上升沿捕获
	TIM2_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //映射到TI1上
	TIM2_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //配置输入分频,不分频 
	TIM2_ICInitStructure.TIM_ICFilter = 0x00;//IC1F=0000 配置输入滤波器 不滤波
	TIM_ICInit(TIM2, &TIM2_ICInitStructure);
	
	//中断分组初始化
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;  //TIM2中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  //先占优先级2级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  //从优先级0级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器 
	
	TIM_ITConfig(TIM2,TIM_IT_Update|TIM_IT_CC2,ENABLE);//允许更新中断 ,允许CC1IE捕获中断	
	
  TIM_Cmd(TIM2,ENABLE ); 	//使能定时器4
}
/**********定时器2的中断处理函数****************/
u8 TIM2CH2_IC_STA = 0;  //输入捕获状态
u16 TIM2CH2_IC_VAL = 0;  //一个周期的长度
void TIM2_IRQHandler(void)
{
	if((TIM2CH2_IC_STA&0X80)==0)//还未成功捕获	
	{	  
		if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)		 
		{	    
			if(TIM2CH2_IC_STA&0X40)//已经捕获到上升沿了
			{
				if((TIM2CH2_IC_STA&0X3F)==0X3F)//高电平太长了
				{
					TIM2CH2_IC_STA|=0X80;//标记成功捕获了一次
					TIM2CH2_IC_VAL=0X0000;
				}else TIM2CH2_IC_STA++;
			}	 
		}
		if (TIM_GetITStatus(TIM2, TIM_IT_CC2) != RESET)//捕获1发生捕获事件
		{	
			if(TIM2CH2_IC_STA&0X40)		//如果前面已经捕获到一次上升沿	
			{	  			
				TIM2CH2_IC_STA|=0X80;		//标记成功捕获到2次上升沿
				TIM2CH2_IC_VAL=TIM_GetCapture2(TIM2);
			}else  								//还未开始,第一次捕获上升沿
			{
				TIM2CH2_IC_STA=0;			//清空
				TIM2CH2_IC_VAL=0;
				TIM_SetCounter(TIM2,0);
				TIM2CH2_IC_STA|=0X40;		//标记捕获到了上升沿
			}		    
		}			     	    					   
 	}
 
  TIM_ClearITPendingBit(TIM2, TIM_IT_CC2|TIM_IT_Update); //清除中断标志位
}






