/*  main.cpp
 *  Создан: 13 марта 2023 г.
 *  Автор: Damil
*/

#define DEBUG  0 // отладка
#define DEBUG_I2C  0 // отладка i2c

#include "typedef.h"

#include "stm32f0xx.h"
#include "stm32f0xx_adc.h"
#include "stm32f0xx_exti.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_i2c.h"
#include "stm32f0xx_misc.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_syscfg.h"
#include "system_stm32f0xx.h"
#include "arlan.h"
#include "button.h"
#include "switcher.h"
#include "i2c.h"

#define getBit(x, y)  ( (x)->IDR & (y) )
//#define getBit(x, y)  GPIO_ReadInputDataBit(x, y)
#define set_0(x, y)  GPIO_ResetBits(x, y)
#define set_1(x, y)  GPIO_SetBits(x, y)

typedef enum eTask {TASK_WAIT_PRES = 0, TASK_WAIT_AC_OK, TASK_PWR_ON, TASK_WAIT_PW_OK, TASK_ON} cTask;

ui64 Timer = 0;
ui64 TimerLED = 0;
ui64 TimerDelay = 0;
ui64 TimerPWR1 = 0;
ui64 TimerPWR2 = 0;
cButton buttonFront, buttonBack;
cSwitcher swPres1, swPres2, swAcOK1, swAcOK2, swPwOK1, swPwOK2;

ui8 i2c_cmd = 0xFF;
bool is_i2c_cmd = false;

void doTask();
bool adc();
bool initialization();	// Начальная настройка периферии и инициализация переменных
void setStartState();
bool sysTimerInitialization();	// Начальная настройка системного таймера
void port_Initialization();		// Начальная настройка портов ввода-вывода
void adc_Initialization();		// Начальная настройка АЦП
void i2c_Initialization();		// Начальная настройка I2C
ui16 getVoltage(ui32 adc_channel); // Считывание напряжения

int main()
{
	if (!initialization())  return 0;

	__enable_irq();

	while (1)
	{
		doTask();
	}

	return 0;
}

#if (DEBUG == 1)
bool doPow1 = false, doPow2 = false;
#endif
bool powOK = false;
volatile ui16 V5, V3_3, V1_8, V1_5, V1, V0_9;

void doTask()
{
	static ui16 task = 0;
	static ui16 task1 = TASK_WAIT_PRES;
	static ui16 task2 = TASK_WAIT_PRES;
	static bool buttonState = true;

	if ( (cButtonIsPressed(&buttonFront) != BUTTON_NO_PRESS) || (cButtonIsPressed(&buttonBack) != BUTTON_NO_PRESS) )
	{
		if (buttonState)
		{
			set_1(PORT_LED_BTN, PIN_LED1);
			set_1(PORT_LED_BTN, PIN_LED2);
		}
		else
		{
			set_0(PORT_LED_BTN, PIN_LED1);
			set_0(PORT_LED_BTN, PIN_LED2);
		}
		buttonState = !buttonState;
	}

	if (is_i2c_cmd)
	{
		switch (i2c_cmd)
		{
			case I2C_CMD_TURN_OFF_LEDS: // откл.
				set_0(PORT_LED_BTN, PIN_LED1);
				set_0(PORT_LED_BTN, PIN_LED2);
				break;
			case I2C_CMD_TURN_ON_LEDS: // вкл.
				set_1(PORT_LED_BTN, PIN_LED1);
				set_1(PORT_LED_BTN, PIN_LED2);
				break;
		}
		is_i2c_cmd = false;
	}

// ------------------------------------------------------------------------------

	if (task1 > TASK_WAIT_PRES)
	{
		if (swPres1.state == SWITCHER_OFF)
		{
			set_1(PORT_PWR, PIN_PWR1_ON); // 1 - откл. PWR1
			#if (DEBUG == 1)
			doPow1 = false;
			doPow2 = true;
			#endif
			task1 = TASK_WAIT_PRES;
		}
		else if (task1 > TASK_PWR_ON)
		{
			if (swAcOK1.state == SWITCHER_OFF)
			{
				set_1(PORT_PWR, PIN_PWR1_ON); // 1 - откл. PWR1
				#if (DEBUG == 1)
				doPow1 = false;
				doPow2 = true;
				#endif
				task1 = TASK_WAIT_AC_OK;
			}
			else if (task1 > TASK_WAIT_PW_OK)
			{
				if (swPwOK1.state == SWITCHER_OFF)
				{
					if (swPwOK2.state == SWITCHER_OFF)
					{
						set_0(PORT_SW_RST, PINS_SW_PG_OUT | PIN_RST_TCA);
						set_0(PORT_ADC, PINS_V_EN);
						set_0(PORT_LED_BTN, PINS_LEDS);
					}
					task1 = TASK_WAIT_PW_OK;
				}
			}
		}
	}

	switch (task1)
	{
		case TASK_WAIT_PRES:
			if (swPres1.state == SWITCHER_ON)
				task1++;
			break;
		case TASK_WAIT_AC_OK:
			if (swAcOK1.state == SWITCHER_ON)
			{
				TimerPWR1 = Timer;
				task1++;
			}
			break;
		case TASK_PWR_ON:
			if (Timer - TimerPWR1 >= TIMER_PWR) // 1 сек
			{
				set_0(PORT_PWR, PIN_PWR1_ON);
				#if (DEBUG == 1)
				if (!doPow2)  doPow1 = true;
				#endif
				task1++;
			}
			break;
		case TASK_WAIT_PW_OK:
			if (swPwOK1.state == SWITCHER_ON)
			{
				powOK = true;
				#if (DEBUG == 1)
				if (!doPow2)
				#endif
					TimerDelay = Timer;
				task1++;
			}
			break;
		case TASK_ON:
			break;
	}

// ------------------------------------------------------------------------------

	if (task2 > TASK_WAIT_PRES)
	{
		if (swPres2.state == SWITCHER_OFF)
		{
			set_1(PORT_PWR, PIN_PWR2_ON); // 1 - откл. PWR2
			#if (DEBUG == 1)
			doPow2 = false;
			doPow1 = true;
			#endif
			task2 = TASK_WAIT_PRES;
		}
		else if (task2 > TASK_PWR_ON)
		{
			if (swAcOK2.state == SWITCHER_OFF)
			{
				set_1(PORT_PWR, PIN_PWR2_ON); // 1 - откл. PWR2
				#if (DEBUG == 1)
				doPow2 = false;
				doPow1 = true;
				#endif
				task2 = TASK_WAIT_AC_OK;
			}
			else if (task2 > TASK_WAIT_PW_OK)
			{
				if (swPwOK2.state == SWITCHER_OFF)
				{
					if (swPwOK1.state == SWITCHER_OFF)
					{
						set_0(PORT_SW_RST, PINS_SW_PG_OUT | PIN_RST_TCA);
						set_0(PORT_ADC, PINS_V_EN);
						set_0(PORT_LED_BTN, PINS_LEDS);
					}
					task2 = TASK_WAIT_PW_OK;
				}
			}
		}
	}

	switch (task2)
	{
		case TASK_WAIT_PRES:
			if (swPres2.state == SWITCHER_ON)
				task2++;
			break;
		case TASK_WAIT_AC_OK:
			if (swAcOK2.state == SWITCHER_ON)
			{
				TimerPWR2 = Timer;
				task2++;
			}
			break;
		case TASK_PWR_ON:
			if (Timer - TimerPWR2 >= TIMER_PWR) // 1 сек
			{
				set_0(PORT_PWR, PIN_PWR2_ON);
				#if (DEBUG == 1)
				if (!doPow1)  doPow2 = true;
				#endif
				task2++;
			}
			break;
		case TASK_WAIT_PW_OK:
			if (swPwOK2.state == SWITCHER_ON)
			{
				powOK = true;
				#if (DEBUG == 1)
				if (!doPow1)
				#endif
					TimerDelay = Timer;
				task2++;
			}
			break;
		case TASK_ON:
			break;
	}


// ------------------------------------------------------------------------------

	if (powOK)
	{
		switch (task)
		{
			case 0:
				if ( Timer - TimerDelay >= (TIMER_ADC << 3) ) // 10*(2^3) = 80 мс
				{
					set_1(PORT_ADC, PIN_5V_EN);
					TimerDelay = Timer;
					task++;
				}
				break;
			case 1:
				if (Timer - TimerDelay >= TIMER_ADC) // 10 мс
				{
					V5 = getVoltage(ADC_Channel_5V);
					V5 *= V5_DIV;
					TimerDelay = Timer;
					if (V5 < ADC5V_MIN || V5 > ADC5V_MAX)
					{
						set_0(PORT_ADC, PINS_V_EN);
						task = 0;
						break;
					}
					set_1(PORT_ADC, PIN_3V3_EN);
					task++;
				}
				break;
			case 2:
				if (Timer - TimerDelay >= TIMER_ADC) // 10 мс
				{
					V3_3 = getVoltage(ADC_Channel_3V3);
					V3_3 *= V3_3_DIV;
					TimerDelay = Timer;
					if (V3_3 < ADC3V3_MIN || V3_3 > ADC3V3_MAX)
					{
						set_0(PORT_ADC, PINS_V_EN);
						task = 0;
						break;
					}
					set_1(PORT_ADC, PIN_1V8_EN);
					task++;
				}
				break;
			case 3:
				if (Timer - TimerDelay >= TIMER_ADC) // 10 мс
				{
					V1_8 = getVoltage(ADC_Channel_1V8);
					TimerDelay = Timer;
					if (V1_8 < ADC1V8_MIN || V1_8 > ADC1V8_MAX)
					{
						set_0(PORT_ADC, PINS_V_EN);
						task = 0;
						break;
					}
					set_1(PORT_ADC, PIN_1V5_EN);
					task++;
				}
				break;
			case 4:
				if (Timer - TimerDelay >= TIMER_ADC) // 10 мс
				{
					V1_5 = getVoltage(ADC_Channel_1V5);
					TimerDelay = Timer;
					if (V1_5 < ADC1V5_MIN || V1_5 > ADC1V5_MAX)
					{
						set_0(PORT_ADC, PINS_V_EN);
						task = 0;
						break;
					}
					set_1(PORT_ADC, PIN_1V_EN);
					task++;
				}
				break;
			case 5:
				if (Timer - TimerDelay >= TIMER_ADC) // 10 мс
				{
					V1   = getVoltage(ADC_Channel_1V);
					TimerDelay = Timer;
					if (V1 < ADC1V_MIN || V1 > ADC1V_MAX)
					{
						set_0(PORT_ADC, PINS_V_EN);
						task = 0;
						break;
					}
					set_1(PORT_ADC, PIN_0V9_EN);
					task++;
				}
				break;
			case 6:
				if (Timer - TimerDelay >= TIMER_ADC) // 10 мс
				{
					V0_9 = getVoltage(ADC_Channel_0V9);
					TimerDelay = Timer;
					if (V0_9 < ADC0V9_MIN || V0_9 > ADC0V9_MAX)
					{
						set_0(PORT_ADC, PINS_V_EN);
						task = 0;
						break;
					}
					task++;
				}
				break;
			case 7:
				set_1(PORT_SW_RST, PINS_SW_PG_OUT);
				TimerDelay = Timer;
				task++;
				break;
			case 8:
				if (Timer - TimerDelay >= 10)
				{
					set_1(PORT_SW_RST, PIN_RST_TCA);
					TimerDelay = Timer;
					task++;
				}
				break;
			case 9:
				if ( Timer - TimerDelay >= (TIMER_ADC << 6) ) // 10*(2^6) = 640 мс
				{
					if ( !adc() )
					{
						set_0(PORT_ADC, PINS_V_EN);
						set_0(PORT_SW_RST, PINS_SW_PG_OUT | PIN_RST_TCA);
						task = 0;
					}
					TimerDelay = Timer;
				}
				break;
		}
	}

}

bool adc()
{
	V5 = getVoltage(ADC_Channel_5V);
	V5 *= V5_DIV;
	if (V5 < ADC5V_MIN || V5 > ADC5V_MAX)		return  false;

	V3_3 = getVoltage(ADC_Channel_3V3);
	V3_3 *= V3_3_DIV;
	if (V3_3 < ADC3V3_MIN || V3_3 > ADC3V3_MAX)	return  false;

	V1_8 = getVoltage(ADC_Channel_1V8);
	if (V1_8 < ADC1V8_MIN || V1_8 > ADC1V8_MAX)	return  false;

	V1_5 = getVoltage(ADC_Channel_1V5);
	if (V1_5 < ADC1V5_MIN || V1_5 > ADC1V5_MAX)	return  false;

	V1   = getVoltage(ADC_Channel_1V);
	if (V1 < ADC1V_MIN || V1 > ADC1V_MAX)		return  false;

	V0_9 = getVoltage(ADC_Channel_0V9);
	if (V0_9 < ADC0V9_MIN || V0_9 > ADC0V9_MAX)	return  false;

	return  true;
}

#ifdef __cplusplus
extern "C"
{
#endif

ui8 i2cBuf[I2C_BUF_LEN + 2];

void I2C1_IRQHandler()
{
	static ui16 I2C_Direction = I2C_Direction_Transmitter;
	static ui8 iData;
	ui8 data;
        uint8_t address;
         uint8_t blocked_addr = (0x40 << 1);
	// приняли свой адрес (очистака I2C_IT_ADDR чтением SR1 и SR2)
	if ( I2C_GetITStatus(I2C1, I2C_IT_ADDR) == SET )
	{
		I2C_ClearITPendingBit(I2C1, I2C_ICR_ADDRCF);
		I2C_Direction = I2C_GetTransferDirection(I2C1);
		if (I2C_Direction == I2C_Direction_Receiver)
		{
			for (ui8 i = 0; i < I2C_BUF_LEN; i++)  i2cBuf[i] = 0;
			iData = 0;
		}
		#if (DEBUG_I2C == 1)
		//set_1(PORT_LED_BTN, PIN_LED1);
		#endif
	}
	else if ( I2C_GetITStatus(I2C1, I2C_IT_STOPF) == SET )
	{
		// сбрасываем I2C_IT_STOPF
		I2C_ClearITPendingBit(I2C1, I2C_ICR_STOPCF);
		I2C_Direction = I2C_Direction_Transmitter;
		iData = 0;
	}
	else if ( I2C_GetITStatus(I2C1, I2C_IT_RXNE) == SET )
	{
		data = I2C_ReceiveData(I2C1);
		if (iData <= I2C_BUF_LEN)
		{
			i2cBuf[iData] = data;
			iData++;
			if (iData == I2C_BUF_LEN + 1)
			{
				is_i2c_cmd = read_i2c_buffer(i2cBuf + 1);
				iData = 0;
			}
		}
       /*          uint8_t address =  & 0xFE; // Mask out LSB for 7-bit address */
        address = I2C_GetAddressMatched(I2C1);
        if (address == blocked_addr) {
            // Address match, do not acknowledge the address
            I2C_AcknowledgeConfig(I2C1, DISABLE);
            I2C_GenerateSTOP(I2C1, ENABLE);
            // You can also clear the ADDR flag to prevent further interrupts
            I2C_ClearFlag(I2C1, I2C_FLAG_ADDR);           
        }
		#if (DEBUG_I2C == 1)
		//set_1(PORT_LED_BTN, PIN_LED2);
		#endif
                
	}
}

void SysTick_Handler() // 1 мс
{
	Timer++;

	cButtonProcess( &buttonFront, (bool)getBit(PORT_LED_BTN, PIN_BUTTON1) );
	cButtonProcess( &buttonBack,  (bool)getBit(PORT_LED_BTN, PIN_BUTTON2) );

	cSwitcherProcess( &swPres1, (bool)getBit(PORT_PWR, PIN_PWR1_PRESENT) );
	cSwitcherProcess( &swPres2, (bool)getBit(PORT_PWR, PIN_PWR2_PRESENT) );
	cSwitcherProcess( &swAcOK1, (bool)getBit(PORT_PWR, PIN_PWR1_AC_OK) );
	cSwitcherProcess( &swAcOK2, (bool)getBit(PORT_PWR, PIN_PWR2_AC_OK) );
	cSwitcherProcess( &swPwOK1, (bool)getBit(PORT_PWR, PIN_PWR1_PW_OK) );
	cSwitcherProcess( &swPwOK2, (bool)getBit(PORT_PWR, PIN_PWR2_PW_OK) );

	#if (DEBUG == 1)
	static bool ledState = false;
	if (Timer - TimerLED >= 500)
	{
		if (doPow1)
		{
			ledState ? set_1(PORT_LED_BTN, PIN_LED1) : set_0(PORT_LED_BTN, PIN_LED1);
			set_0(PORT_LED_BTN, PIN_LED2);
		}
		if (doPow2)
		{
			ledState ? set_1(PORT_LED_BTN, PIN_LED2) : set_0(PORT_LED_BTN, PIN_LED2);
			set_0(PORT_LED_BTN, PIN_LED1);
		}
		ledState = !ledState;
		TimerLED = Timer;
	}
	#endif
}

//void EXTI2_3_IRQHandler() // обработчик прерывания PIN_PWR1_AC_OK (PIN_PB02)
//{
//	//if (EXTI_GetITStatus(EXTI_Line2) != RESET)
//	if ( (EXTI->PR & EXTI_Line2) != RESET )
//	{
//		//checkPow1();
//		//offPow1();
//		if (task1 > 1)
//		{
//			//if (getBit(PORT_PWR, PIN_PWR1_AC_OK) == 0)
//			if ((PORT_PWR->IDR & PIN_PWR1_AC_OK) == 0)
//			{
//				//set_0(PORT_PWR, PIN_PWR1_ON); // 1 - откл. PWR1
//				PORT_PWR->BSRR = PIN_PWR1_ON;
//				//set_1(PORT_PWR, PIN_PWR2_ON); // 0 - вкл. PWR2
//				PORT_PWR->BRR = PIN_PWR2_ON;
//				doPow1 = false;
//				//pow_OK = false;
//				//off1 = true;
//				//setStartState();
//				//set_1(PORT_SW_RST, PINS_SW_PG_OUT | PIN_RST_TCA);
//				//set_1(PORT_ADC, PINS_V_EN);
//				//set_1(PORT_LED_BTN, PINS_LEDS);
//				task1 = 1;
//			}
//		}
//
//		EXTI_ClearITPendingBit(EXTI_Line2); // очистка флага прерывания
//		//EXTI->PR = EXTI_Line2;
//	}
//}
//
//void EXTI4_15_IRQHandler() // обработчик прерывания PIN_PWR2_AC_OK (PIN_PB10)
//{
//	//if (EXTI_GetITStatus(EXTI_Line10) != RESET)
//	if ( (EXTI->PR & EXTI_Line10) != RESET )
//	{
//		//checkPow2();
//		//offPow2();
//		if (task2 > 1)
//		{
//			//if (getBit(PORT_PWR, PIN_PWR2_AC_OK) == 0)
//			if ((PORT_PWR->IDR & PIN_PWR2_AC_OK) == 0)
//			{
//				//set_0(PORT_PWR, PIN_PWR2_ON); // 1 - откл. PWR2
//				PORT_PWR->BSRR = PIN_PWR2_ON;
//				//set_1(PORT_PWR, PIN_PWR1_ON); // 0 - вкл. PWR1
//				PORT_PWR->BRR = PIN_PWR1_ON;
//				doPow2 = false;
//				//pow_OK = false;
//				//off2 = true;
//				//setStartState();
//				//set_1(PORT_SW_RST, PINS_SW_PG_OUT | PIN_RST_TCA);
//				//set_1(PORT_ADC, PINS_V_EN);
//				//set_1(PORT_LED_BTN, PINS_LEDS);
//				task2 = 1;
//			}
//		}
//
//		EXTI_ClearITPendingBit(EXTI_Line10); // очистка флага прерывания
//		//EXTI->PR = EXTI_Line10;
//	}
//}

#ifdef __cplusplus
}
#endif

bool initialization() // Начальная настройка периферии и инициализация переменных
{
	cButtonInit(&buttonFront);
	cButtonInit(&buttonBack);

	cSwitcherInit(&swPres1, false);
	cSwitcherInit(&swPres2, false);
	cSwitcherInit(&swAcOK1, true);
	cSwitcherInit(&swAcOK2, true);
	cSwitcherInit(&swPwOK1, true);
	cSwitcherInit(&swPwOK2, true);

	Timer = 0;
	TimerDelay = 0;
	TimerLED = 0;

	if (!sysTimerInitialization())  return  false;	// Системный таймер
	port_Initialization();		// Порты ввода-вывода
	adc_Initialization(); 		// АЦП
	i2c_Initialization();		// I2C

	setStartState();

	return  true;
}

void setStartState()
{
	set_0(PORT_SW_RST, PINS_SW_PG_OUT | PIN_RST_TCA);
	set_0(PORT_ADC, PINS_V_EN);
	set_0(PORT_LED_BTN, PINS_LEDS);
	set_1(PORT_PWR, PINS_PWR_OUT);
}

bool sysTimerInitialization() // Начальная настройка системного таймера
{
//	RCC_PLLConfig(RCC_PLLSource_HSI_Div2, RCC_PLLMul_6);
//	RCC_PLLCmd(ENABLE);
//	while ( (RCC->CR) & RCC_CR_PLLRDY == 0) {};

	// Setup SysTick Timer for 1 msec interrupts: 48 000 000 - 1 сек, 48 000 - 1 мс, 48 - 1 мкс
	if (   SysTick_Config( SystemCoreClock / 1000)   ) // 1 мс
	{
		/* Capture error */
		return  false;
	}

//	GPIO_InitTypeDef port;
//	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);	// Тактуем порт
//	port.GPIO_Pin = GPIO_Pin_8;
//	port.GPIO_Speed = GPIO_Speed_50MHz;
//	port.GPIO_Mode = GPIO_Mode_AF;
//	GPIO_Init(GPIOA, &port);			// Конфигурируем вывод MCO
//
//	RCC_MCOConfig(RCC_MCOSource_PLLCLK_Div2, RCC_MCOPrescaler_1);
	return  true;
}

void port_Initialization() // Начальная настройка портов ввода-вывода
{
	GPIO_InitTypeDef port;
//	EXTI_InitTypeDef exti;
//	NVIC_InitTypeDef nvic;

	// Включаем тактирование порта GPIOA
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB | RCC_AHBPeriph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	GPIO_StructInit(&port);

	port.GPIO_Pin = PINS_PWR_OUT;
	port.GPIO_Mode = GPIO_Mode_OUT;
	port.GPIO_OType = GPIO_OType_PP;
	port.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(PORT_PWR, &port);

	port.GPIO_Pin = PINS_PWR_IN;
	port.GPIO_Mode = GPIO_Mode_IN;
	port.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(PORT_PWR, &port);

//	// Настраиваем прерывание для PIN_PWR1_AC_OK (PIN_PB02)
//	// Connect EXTI Line to GPIO Pin
//	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource2);
//	// Configure Button EXTI line
//	exti.EXTI_Line = EXTI_Line2;
//	exti.EXTI_Mode = EXTI_Mode_Interrupt;
//	exti.EXTI_Trigger = EXTI_Trigger_Falling;
//	exti.EXTI_LineCmd = ENABLE;
//	EXTI_Init(&exti);
//	// Enable and set Button EXTI Interrupt to the lowest priority
//	nvic.NVIC_IRQChannel = EXTI2_3_IRQn;
//	nvic.NVIC_IRQChannelPriority = 3;
//	nvic.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_Init(&nvic);
//
//	// Настраиваем прерывание для PIN_PWR2_AC_OK (PIN_PB10)
//	// Connect EXTI Line to GPIO Pin
//	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource10);
//	// Configure Button EXTI line
//	exti.EXTI_Line = EXTI_Line10;
//	exti.EXTI_Mode = EXTI_Mode_Interrupt;
//	exti.EXTI_Trigger = EXTI_Trigger_Falling;
//	exti.EXTI_LineCmd = ENABLE;
//	EXTI_Init(&exti);
//	// Enable and set Button EXTI Interrupt to the lowest priority
//	nvic.NVIC_IRQChannel = EXTI4_15_IRQn;
//	nvic.NVIC_IRQChannelPriority = 3;
//	nvic.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_Init(&nvic);

	port.GPIO_Pin = PINS_SW_PG_OUT | PIN_RST_TCA;
	port.GPIO_Mode = GPIO_Mode_OUT;
	port.GPIO_OType = GPIO_OType_PP;
	port.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(PORT_SW_RST, &port);

	port.GPIO_Pin = PINS_BUTTONS;
	port.GPIO_Mode = GPIO_Mode_IN;
	port.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(PORT_LED_BTN, &port);

	port.GPIO_Pin = PINS_LEDS;
	port.GPIO_Mode = GPIO_Mode_OUT;
	port.GPIO_OType = GPIO_OType_PP;
	port.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(PORT_LED_BTN, &port);
}

void adc_Initialization() // Начальная настройка АЦП
{
	GPIO_InitTypeDef port;
	ADC_InitTypeDef adc;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

	GPIO_StructInit(&port);
	port.GPIO_Pin = PINS_V;
	port.GPIO_Mode = GPIO_Mode_AN;
	GPIO_Init(PORT_ADC, &port);

	port.GPIO_Pin = PINS_V_EN;
	port.GPIO_Mode = GPIO_Mode_OUT;
	port.GPIO_OType = GPIO_OType_PP;
	port.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(PORT_ADC, &port);

	ADC_StructInit(&adc);
	ADC_Init(ADC1, &adc);

	ADC_ClockModeConfig(ADC1, ADC_ClockMode_AsynClk);

	ADC_GetCalibrationFactor(ADC1);
}

void i2c_Initialization() // Начальная настройка I2C
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE); // Включаем тактирование
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
        
	I2C_InitTypeDef i2c;
        I2C_TypeDef HI2C;
	GPIO_InitTypeDef port;
	NVIC_InitTypeDef nvic;

	port.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7;
	port.GPIO_Speed = GPIO_Speed_50MHz;		// Speed 50 MHz
	port.GPIO_Mode = GPIO_Mode_AF;			// Альтернативная функция
	port.GPIO_PuPd = GPIO_PuPd_NOPULL;		// Без подтяжки
	port.GPIO_OType = GPIO_OType_OD;		// Открытый сток
	GPIO_Init(GPIOB, &port);

	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_1);

	nvic.NVIC_IRQChannel = I2C1_IRQn;
	nvic.NVIC_IRQChannelPriority = 0;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic);

	//I2C_DeInit(I2C1);
	I2C_Cmd(I2C1, DISABLE);

	I2C_StructInit(&i2c);
	i2c.I2C_Mode = I2C_Mode_I2C;					// Режим работы (I2C)
	i2c.I2C_OwnAddress1 = (I2C_SLAVE_ADDR) << 1;	// Адрес (не важен, если устройство - мастер)
        i2c.I2C_SlaveAddressConfig = (I2C_SLAVE_ADDR) << 1;
        
        address = I2C_GetAddressMatched(I2C1);
	i2c.I2C_Timing = 								// Значение для записи в регистр I2C_TIMINGR   // todo
			(0xB  << I2C_OFFSET_TIMINGR_PRESC)|\
			(0x13 << I2C_OFFSET_TIMINGR_SCLL)|\
			(0xF  << I2C_OFFSET_TIMINGR_SCLH)|\
			(0x4  << I2C_OFFSET_TIMINGR_SCLDEL)|\
			(0x2  << I2C_OFFSET_TIMINGR_SDADEL);
	i2c.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
       
	//I2C_ITConfig(I2C_IT_TCI | I2C_IT_ADDRI | I2C_IT_RXI | I2C_IT_TXI, ENABLE);
	I2C_ITConfig(I2C1, I2C_IT_ADDRI | I2C_IT_RXI | I2C_IT_STOPI, ENABLE);

	I2C_Init(I2C1, &i2c);
	I2C_Cmd(I2C1, ENABLE);
	while ((I2C1->CR1 & I2C_CR1_PE)==0) {};	// Ждём пока включится
}

ui16 getVoltage(ui32 adc_channel) // Считывание напряжения
{
	ui16 V = 0; // Переменная для хранения результата преобразования ADC

	ADC_ChannelConfig(ADC1, adc_channel, ADC_SampleTime_239_5Cycles);
	ADC_Cmd(ADC1, ENABLE);	// Включаем ADC
	while ( ADC_GetFlagStatus(ADC1, ADC_FLAG_ADRDY) == RESET ); // Ждём готовности ADC

	ADC_StartOfConversion(ADC1);		// Запускаем преобразование
	while ( ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET ); // Ждём готовности ADC
	V = ADC_GetConversionValue(ADC1);	// Считываем результат преобразования
	ADC_StopOfConversion(ADC1);
	V = ( (ui32)V*3300 ) / 4096;		// Масштабируем

	ADC_Cmd(ADC1, DISABLE);	// Отключаем ADC

	return  V;
}
