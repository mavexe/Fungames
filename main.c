//Подключаем заголовочный файл с описание регистров микроконтроллера
#include "stm32f10x.h"
#include "stm32f10x_conf.h"

#include "main.h"

//Объявим переменную, которая будет считать количество прерываний системного таймера
unsigned int LED_timer;


//Функция,вызываемая из функции-обработчика прерываний системного таймера
void SysTick_Timer_main(void)
{
//Если переменная LED_timer еще не дошла до 0,
if (LED_timer)
	{
	//Проверяем ее значение, если оно больше 1500 включим светодиод
	if (LED_timer>1500) GPIOC->BSRR= GPIO_BSRR_BS9;
	//иначе если меньше или равно 1500 то выключим
	else GPIOC->BSRR= GPIO_BSRR_BR9;
	//Произведем декремент переменной LED_timer
	LED_timer--;
	}
//Ели же значение переменной дошло до нуля, зададим новое значение 2000
else LED_timer=2000;
}



void main(void)
{
RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

GPIOA->CRH &=~ GPIO_CRH_MODE9;
GPIOA->CRH |= GPIO_CRH_MODE9_1;
GPIOA->CRH &=~ GPIO_CRH_CNF9;
while(1)
{
    GPIOA->BSRR = GPIO_BSRR_BS2;
    GPIOA->BSRR = GPIO_BSRR_BR2;
    
    NVIC_EnableIRQ
}
}