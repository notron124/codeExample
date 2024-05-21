#ifdef STM32L1XX_MD
#include "stm32l1xx.h"
#else
#include "stm32f10x.h"
#endif

#include "keyapi.h"
#include "init.h"
#include "port.h"

void RCC_Configuration(void)
{
#ifdef STM32L1XX_MD
   RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB, ENABLE);
#else
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO | RCC_APB2Periph_ADC1, ENABLE);
#endif
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
}

/*******************************************************************************
 * Function Name  : GPIO_Configuration
 * Description    : Configures the different GPIO ports.
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void GPIO_Configuration(void)
{
   /* 2: Out 2MHz, Push-Pull general
    * 3: Out 50 MHz, Push-Pull general
    * 4: Input, floating
    * B: Out 50 MHz, Alt. func. Push-Pull
    * 0: Input, analog mode
    * Unused pins treated as PP-low
    */

   GPIOA->ODR = 0x83;
   GPIOB->ODR = 0;
#ifdef STM32L1XX_MD
   GPIOA->AFR[0] = 0x00007700;
   GPIOA->AFR[1] = 0x00000770;
   GPIOA->MODER = 0x556A40A5;
   GPIOA->PUPDR = 0x40000005;

   GPIOB->MODER = 0x55550055;
#else
   AFIO->MAPR = 0x04000000; // JTAG & SW Disable

   /*
      [PA0-PA3] - Out: SND, W_RST, K1, HEATER
      [PA4-PA6] - Analog IN: ColdJunc, TCoupleAmp, Press
      PA7 - Out: K2
   */
   GPIOA->CRL = 0x20002222;

   /*
      PA8 - Out: -
      PA9 - Alt. Func. Push-Pull: Wi-Fi UART_TX
      PA10 - Floating IN: Wi-Fi UART_RX
      PA11 - Out: Wi-Fi LED
      [PA12-PA15] - Out: -
   */
   GPIOA->CRH = 0x222224B2;

   GPIOB->CRL = 0x44442222;
   GPIOB->CRH = 0x22222222;
#endif
}

void NVIC_Configuration(void)
{
   NVIC_InitTypeDef NVIC_InitStructure;

   /* Vector Table Relocation in Internal FLASH. */
   /* 1 bits for pre-emption priority and 3 bits for subpriority */
   NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

   /* Enable the USART1 Interrupt */
   NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);

   NVIC_InitStructure.NVIC_IRQChannel = ADC1_2_IRQn;
   NVIC_Init(&NVIC_InitStructure);
}

void SysTick_Configuration(void)
{
   /* Select HCLK/8 as SysTick clock source */
   SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
#ifdef STM32L1XX_MD
   SysTick_Config(32000); // 32MHz/32000=1000
#else
   SysTick_Config(SystemCoreClock / 1000); // 72MHz/72000=1000
#endif
}

/*
   Sets the total number of conversions that will be done in
   injected channels.
   Min - 1
   Max - 4
   This number equals to amount of channels that will be read.
*/
void SetTotalConversions(ADC_TypeDef *ADCx, uint8_t numberOfConversions)
{
   ADCx->JSQR = (numberOfConversions - 1) << 20;
}

/*
   Sets the conversion sequence. WORKS ONLY FOR CHANNELS 0-9.
   conversionSequence variable should be written like this:
   1234
   For this example the seqience would be JSQ1 - 4, JSQ2 - 3, JSQ3 - 2, JSQ4 - 1.
   Note: If total conversions is less than 4, it will start from JSQ(4 - JL) or else JSQ(4 - (numberOfConversions - 1)).
   Example:
   123
   would be JSQ2 - 3, JSQ3 - 2, JSQ4 - 1.
*/
void SetConversionSequence(ADC_TypeDef *ADCx, uint16_t conversionSequence, uint8_t numberOfConversions)
{
   for (uint8_t i = 4 - numberOfConversions; i < 4; i++)
   {
      ADCx->JSQR |= (conversionSequence % 10) << (5 * i);
      conversionSequence /= 10;
   }
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

   /* Injected sequence length - 3 conversion */
   SetTotalConversions(ADC1, 3);

   /* Select input Channel 5 for watchdog*/
   // ADC1->CR1 |= 0x05;

   /* Interrupt enable for injected channels */
   ADC1->CR1 |= ADC_CR1_JEOCIE;

   /* Conversion on external event enabled */
   ADC1->CR2 |= ADC_CR2_JEXTTRIG;

   /* External event select for injected group - 111: JSWSTART */
   ADC1->CR2 |= ADC_CR2_JEXTSEL;

   /* Channel x Sample time selection - 111: 239.5 cycles*/
   ADC1->SMPR2 |= ADC_SMPR2_SMP0;
   ADC1->SMPR2 |= ADC_SMPR2_SMP5;

   /* sequence - 4 5 6 */
   SetConversionSequence(ADC1, 654, 3);
   // ADC1->JSQR |= ADC_JSQR_JSQ3_0 | ADC_JSQR_JSQ3_2;

   /* Scan mode enabled */
   ADC1->CR1 |= ADC_CR1_SCAN;

   /* Start conversion */
   ADC1->CR2 |= ADC_CR2_ADON;

   RCC_ADCCLKConfig(RCC_CFGR_ADCPRE_DIV6);
}

void USART1_Configuration(u32 speed)
{
   USART_InitTypeDef USART_InitStructure;

   /* USARTy and USARTz configured as follow:
         - BaudRate = 9600 baud
         - Word Length = 8 Bits
         - One Stop Bit
         - No parity
         - Hardware flow control disabled (RTS and CTS signals)
         - Receive and transmit enabled
   */

   USART_Cmd(USART1, DISABLE);

   USART_InitStructure.USART_BaudRate = speed;
   USART_InitStructure.USART_WordLength = USART_WordLength_8b;
   USART_InitStructure.USART_StopBits = USART_StopBits_1;
   USART_InitStructure.USART_Parity = USART_Parity_No;
   USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
   USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

   /* Configure USART1 */
   USART_Init(USART1, &USART_InitStructure);

//   /* Enable USART1 Receive and Transmit interrupts */
//   USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

   /* Enable the USART1 */
   USART_Cmd(USART1, ENABLE);
}

/*******************************************************************************
 * Function Name  : Set_USBClock
 * Description    : Configures USB Clock input (48MHz).
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
void Set_USBClock(void)
{
   /* Enable the USB clock */
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);
}

void IWGD_Configuration(void)
{
   /* IWDG timeout equal to 280 ms (the timeout may vary due to LSI frequency
      dispersion) */
   /* Enable write access to IWDG_PR and IWDG_RLR registers */
   IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

   /* IWDG counter clock: 40KHz(LSI) / 32 = 1.25 KHz */
   IWDG_SetPrescaler(IWDG_Prescaler_128);

   /* Set counter reload value to 349 */
   IWDG_SetReload(0xFF0); //(349);

   /* Reload IWDG counter */
   IWDG_ReloadCounter();

   /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
   IWDG_Enable();
}

void InitAllKeys(void)
{
   Key_Delays delays;
   delays.shortPress = 75;
   delays.longPressEvent = 1000;
   delays.betweenRepeats = 200;
   delays.betweenClicks = 0;

   Key_IDs IDs;
   IDs.longPressEvent = 4;
   IDs.multiClick = 0;

   Key_Flags flags;
   flags.bits.activeLevel = 0;
   flags.bits.autorepeat = 0;
   flags.bits.longPressEvent = 1;
   flags.bits.multiClickEvent = 0;

   for (uint8_t i = 0; i < KEYS_AMOUNT; i++)
   {
      IDs.shortPress = i;
//      static uint16_t key_pin = P_KEY1;
      InitKey(&keys[i], KEY_GPIO, P_KEY1 << i, delays, IDs, flags);
      IDs.longPressEvent++;
//      key_pin = key_pin << 1;
   }
}

void Init(void)
{
   RCC_Configuration();
   GPIO_Configuration();

   // Configure SysTick to generate an interrupt each 1ms
   SysTick_Configuration();
   ADC1_init();
   USART1_Configuration(115200);
   NVIC_Configuration();
   IWGD_Configuration();

   InitAllKeys();
}
