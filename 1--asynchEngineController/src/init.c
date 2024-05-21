#include "stm32f10x.h"
#include "asyncEngine.h"

#define ADC1_DR_BASEADDRESS     ((uint32_t)0x4001244C)

void RCC_Configuration(void)
{
   RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO | RCC_APB2Periph_TIM1 | RCC_APB2Periph_ADC1, ENABLE);
}

void GPIO_init(void)
{
   GPIOA->ODR = 0;
   GPIOA->CRL = 0x24022220;
   GPIOA->CRH = 0x22222BBB;

   GPIOB->ODR = 0;
   GPIOB->CRL = 0x24222222;
   GPIOB->CRH = 0xBBB22222;
}

void EXTI_init(void)
{
   /* Interrupt from line 6 is not masked */
   EXTI->IMR |= EXTI_IMR_MR6;

   /* Falling trigger enabled */
   EXTI->FTSR |= EXTI_FTSR_TR6;
}

void TIM1_init(void)
{
   timerARR = SystemCoreClock / PWM_FREQUENCY;

   /* See PWM_FREQUENCY */
   TIM1->ARR = timerARR - 1;

   /* 110: PWM mode 1 - In upcounting, channel 1 is active as long as TIMx_CNT<TIMx_CCR1
    * else inactive. In downcounting, channel 1 is inactive as long as
    * TIMx_CNT>TIMx_CCR1 else active.
    * */
   TIM1->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2;
   TIM1->CCMR1 |= TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2;
   TIM1->CCMR2 |= TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2;

   TIM1->CCER = 0;

   /* On - OC1-3 signal is output on the corresponding output pin */
   TIM1->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E;

   /* Complementary output enable */
   TIM1->CCER |= TIM_CCER_CC1NE | TIM_CCER_CC2NE | TIM_CCER_CC3NE;

   /* Setting up deadtime (1,2us) */
   TIM1->BDTR = (TIM_BDTR_DTG & 0x5C);

   /* Update interrupt enable */
   TIM1->DIER |= TIM_DIER_UIE;

   /* Reinitialize the counter and generates an update of the registers */
   TIM1->EGR |= TIM_EGR_UG;

   /* Enable counter */
   TIM1->CR1 |= TIM_CR1_CEN;
}

void TIM4_init(void)
{
   TIM4->ARR = 2880;
   TIM4->PSC = 1000;
   TIM4->EGR |= TIM_EGR_UG;
   TIM4->SR &= ~TIM_SR_UIF;
   TIM4->DIER |= TIM_DIER_UIE;
   TIM4->DIER |= TIM_DIER_TIE;
   TIM4->CR1 |= TIM_CR1_CEN;
}

void DMA_ADC1_Configuration(void)
{
   ADC_InitTypeDef ADC_InitStructure;
   DMA_InitTypeDef DMA_InitStructure;

   DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_BASEADDRESS;
   DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)adcToDma;
   DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
   DMA_InitStructure.DMA_BufferSize = ADC_TO_DMA_BUFF_SIZE;
   DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
   DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
   DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
   DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
   DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
   DMA_InitStructure.DMA_Priority = DMA_Priority_High;
   DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

   DMA_Init(DMA1_Channel1, &DMA_InitStructure);
   DMA_Cmd(DMA1_Channel1, ENABLE);

   ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
   ADC_InitStructure.ADC_ScanConvMode = ENABLE;
   ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
   ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
   ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
   ADC_InitStructure.ADC_NbrOfChannel = ADC_TO_DMA_BUFF_SIZE;
   ADC_Init(ADC1, &ADC_InitStructure);

   ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_239Cycles5);
   ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 2, ADC_SampleTime_239Cycles5);

   ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);
   ADC_DMACmd(ADC1, ENABLE);
   ADC_Cmd(ADC1, ENABLE);

   ADC_ResetCalibration(ADC1);
   while (ADC_GetResetCalibrationStatus(ADC1));

   ADC_StartCalibration(ADC1);
   while (ADC_GetCalibrationStatus(ADC1));
}

void ADC1_init(void)
{
   /* Wake up from powerdown state */
   ADC1->CR2 |= ADC_CR2_ADON;

   /* Start calibration */
   ADC1->CR2 |= ADC_CR2_CAL;

   /* Wait until calibration is done */
   while ((ADC1->CR2 & ADC_CR2_CAL) > 0)
   {
   }

   /* Channel 5 of ADC as input (maybe unimportant to do) */
   ADC1->CR1 |= 0x05;

   /* Injected sequence length - 2 conversion */
   ADC1->JSQR |= ADC_JSQR_JL_0;

   /* Interrupt enable for injected channels */
   ADC1->CR1 |= ADC_CR1_JEOCIE;

   /* Conversion on external event enabled */
   ADC1->CR2 |= ADC_CR2_JEXTTRIG;

   /* External event select for injected group - 111: JSWSTART */
   ADC1->CR2 |= ADC_CR2_JEXTSEL;

   /* Channel x Sample time selection - 111: 239.5 cycles*/
   ADC1->SMPR2 |= ADC_SMPR2_SMP0;
   ADC1->SMPR2 |= ADC_SMPR2_SMP5;

   /* sequence - 5 0 */
   ADC1->JSQR |= ADC_JSQR_JSQ3_0 | ADC_JSQR_JSQ3_2;

   /* Scan mode enabled */
   ADC1->CR1 |= ADC_CR1_SCAN;

   /* Start conversion */
   ADC1->CR2 |= ADC_CR2_ADON;
}

void NVIC_init(void)
{
   working_mode = (GPIOA->IDR & GPIO_IDR_IDR1) >> 1;

   NVIC_SetPriority(EXTI9_5_IRQn, 0);
   NVIC_SetPriority(ADC1_2_IRQn, 1);
   NVIC_SetPriority(TIM1_UP_IRQn, 2);

   if (working_mode)
   {
      for (u8 c = 0; c < SPEED_ARR_LENGTH; c++)
         speed[c] = 410;
   }

   NVIC_EnableIRQ(EXTI9_5_IRQn);
   NVIC_EnableIRQ(ADC1_2_IRQn);
   NVIC_EnableIRQ(TIM4_IRQn);
}

void IWGD_Configuration(void)
{
   /* IWDG timeout equal to 280 ms (the timeout may vary due to LSI frequency
      dispersion) */
   /* Enable write access to IWDG_PR and IWDG_RLR registers */
   IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

   /* IWDG counter clock: 40KHz(LSI) / 32 = 1.25 KHz */
   IWDG_SetPrescaler(IWDG_Prescaler_32);

   /* Set counter reload value to 349 */
   IWDG_SetReload(125);

   /* Reload IWDG counter */
   IWDG_ReloadCounter();

   /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
   IWDG_Enable();
}

void InitVariables(void)
{
   desired_frequency = 10;
   current_frequency = 9;

   /* For 12 kHz PWM and 360 points sine*/
   pointerChangeSpeedCoef = 3;

   /* For 1 second acceleration and 5 seconds deceleration */
   rumpUpDivider = 24;
   slowDownCoeff = 36;
   globalSpeedChangeCoeff = 2;
}

void init(void)
{
	RCC_Configuration();
	GPIO_init();
	EXTI_init();
	DMA_ADC1_Configuration();
	TIM1_init();
	TIM4_init();
	NVIC_init();
	InitVariables();
}
