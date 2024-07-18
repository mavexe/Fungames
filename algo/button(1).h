/*  button.h
 *  Создан: 13 апреля 2022 г.
 *  Автор: Damil
*/

#ifndef BUTTON_H_
#define BUTTON_H_

#include "typedef.h"

#define BUTTON_LOW_COUNT		40	// мс
#define BUTTON_HIGH_COUNT		40	// мс
#define BUTTON_LONG_PRESS_MS	40	// * BUTTON_HIGH_COUNT = 1600 мс

typedef enum enumButtonPress
{
	BUTTON_NO_PRESS = 0,	// 0 - кнопка не нажата
	BUTTON_SHORT_PRESS,		// 1 - короткое нажатие
	BUTTON_LONG_PRESS		// 2 - долгое нажатие
} cButtonPress;

typedef struct sButton
{
	ui32 countLow;	// счетчик низкого уровня для определения дребезга
	ui32 countHigh;	// счетчик высокого уровня для определения дребезга
	bool is;		// появился новый импульс(!)
	bool isHigh;	// подтвердить высокий сигнал, когда countHigh станет больше определённого значения
	bool isLow;		// подтвердить низкий сигнал, когда countLow станет больше определённого значения
	bool isPress;	// кнопка нажата, можно обработать
	bool isLong;	// длительное нажатие
	bool beginTimerLong;
	//ui32 iTimerLong;// таймер для определения длительности нажатия
	ui32 timerHighLong;	// счетчик для определения длительности нажатия
} cButton;


// -------------------------------- Кнопки -------------------------------------

inline void cButtonInit(cButton* button)
{
	button->countLow = 0;
	button->countHigh = 0;
	button->is = false;
	button->isHigh = false;
	button->isLow = true;
	button->isPress = false;
	button->isLong = false;
	button->beginTimerLong = false;
	button->timerHighLong = 0;
}

inline cButtonPress cButtonIsPressed(cButton* button)
{
	cButtonPress result = BUTTON_NO_PRESS;
	if (button->isPress)
	{
		if (button->isLong)  result = BUTTON_LONG_PRESS;
		else				 result = BUTTON_SHORT_PRESS;

		button->isPress = false;
		//button->isLong = false;
	}
	return  result;
}

inline void cButtonProcess(cButton* button, bool level)
{
	if (level == 0) // низкий уровень - кнопка нажата, высокий - отпущена
	{
		if (!button->is)
		{
			button->is = true;
			button->countHigh = 0;
		}
		button->countHigh++;
		button->countLow = 0;
	}
	else
	{
		button->countLow++;
		button->countHigh = 0;
	}
	if (button->is) // есть прерывание
	{
		if (button->countHigh >= BUTTON_HIGH_COUNT) // подтвердили сигнал (т.е. это не дребезг)
		{
			button->isHigh = true; // кнопка нажата
			button->isLow = false;
			button->countHigh = 0;
			if (!button->beginTimerLong)
			{
				button->timerHighLong = 0;
				button->beginTimerLong = true;
			}
			else if (!button->isLong)
			{
				button->timerHighLong++;
				if (button->timerHighLong >= BUTTON_LONG_PRESS_MS)
				{
					button->isLong = true;
					button->isPress = true;
				}
			}
		}

		if (button->countLow > BUTTON_LOW_COUNT) // перестаём отслеживать сигнал
		{
			button->is = false; // закончить отслеживать сигнал с кнопки

			if (button->isHigh) // если кнопка нажата
			{
				button->isLow = true; // кнопка отпущена
				button->isHigh = false;
				if (!button->isLong)  button->isPress = true;
				button->isLong = false;
				button->beginTimerLong = false;
			}
		}
	}
}

#endif /* BUTTON_H_ */
