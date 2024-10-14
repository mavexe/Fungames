//���������� ������������ ���� � �������� ��������� ����������������
#include "stm32f10x.h"
#include "stm32f10x_conf.h"

#include "main.h"

//������� ����������, ������� ����� ������� ���������� ���������� ���������� �������
unsigned int LED_timer;


//�������,���������� �� �������-����������� ���������� ���������� �������
void SysTick_Timer_main(void)
{
//���� ���������� LED_timer ��� �� ����� �� 0,
if (LED_timer)
	{
	//��������� �� ��������, ���� ��� ������ 1500 ������� ���������
	if (LED_timer>1500) GPIOC->BSRR= GPIO_BSRR_BS9;
	//����� ���� ������ ��� ����� 1500 �� ��������
	else GPIOC->BSRR= GPIO_BSRR_BR9;
	//���������� ��������� ���������� LED_timer
	LED_timer--;
	}
//��� �� �������� ���������� ����� �� ����, ������� ����� �������� 2000
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