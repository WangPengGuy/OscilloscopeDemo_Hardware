#include "stm32f10x.h"
#include "adc.h"

void Adc_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE); //ʹ��IO��ʱ��
	
	//PA1 ��Ϊģ��ͨ����������                         
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//ģ����������
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void Adc_Mode_Init(void)
{
	ADC_InitTypeDef ADC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE); //ʹ��ADC1ͨ��ʱ��
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);  //����ADC��Ƶ����6 72M/6=12,ADC���ʱ�䲻�ܳ���14M
	
	ADC_DeInit(ADC1);  //��λADC1,������ ADC1 ��ȫ���Ĵ�������Ϊȱʡֵ

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//ADC����ģʽ:ADC1�����ڶ���ģʽ
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;	//��ֹɨ��ģʽ���������ڵ�ͨ��ģʽ
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	//ģ��ת������������ת��ģʽ
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_TRGO ;	//ת���ɶ�ʱ��3��TRGO�¼�����
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADC�����Ҷ���
	ADC_InitStructure.ADC_NbrOfChannel = 2;	//˳����й���ת����ADCͨ������Ŀ
	ADC_Init(ADC1, &ADC_InitStructure);	//����ADC_InitStruct��ָ���Ĳ�����ʼ������ADCx�ļĴ���   
	
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_1Cycles5);  ////����ADC�Ĺ���ͨ���飬ת��˳�򣬲�������
	ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 2, ADC_SampleTime_1Cycles5);  ////����ADC�Ĺ���ͨ���飬ת��˳�򣬲�������
	
	ADC_DMACmd(ADC1, ENABLE); //ʹ��ADC1��DMA���� ��ԭ��ADC��û�еģ�ע�����
	
	ADC_ExternalTrigConvCmd(ADC1,ENABLE);  //ʹ��ADC1���ⲿ����ת����������TIM3��TRGO�¼�����
	
	ADC_Cmd(ADC1, ENABLE);	//ʹ��ָ����ADC1
	
	ADC_ResetCalibration(ADC1);	//ʹ�ܸ�λУ׼  
	 
	while(ADC_GetResetCalibrationStatus(ADC1));	//�ȴ���λУ׼����
	
	ADC_StartCalibration(ADC1);	 //����ADУ׼
 
	while(ADC_GetCalibrationStatus(ADC1));	 //�ȴ�У׼����
}

void Adc_Init()
{
	Adc_GPIO_Init();
	Adc_Mode_Init();
}

void Adc_Start()
{
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);  //�������ADC
}

u16 Adc_GetValue()
{
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));  //�ȴ�ת������
	return ADC_GetConversionValue(ADC1);  //����ת����ֵ
}
