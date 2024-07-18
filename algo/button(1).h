/*  button.h
 *  ������: 13 ������ 2022 �.
 *  �����: Damil
*/

#ifndef BUTTON_H_
#define BUTTON_H_

#include "typedef.h"

#define BUTTON_LOW_COUNT		40	// ��
#define BUTTON_HIGH_COUNT		40	// ��
#define BUTTON_LONG_PRESS_MS	40	// * BUTTON_HIGH_COUNT = 1600 ��

typedef enum enumButtonPress
{
	BUTTON_NO_PRESS = 0,	// 0 - ������ �� ������
	BUTTON_SHORT_PRESS,		// 1 - �������� �������
	BUTTON_LONG_PRESS		// 2 - ������ �������
} cButtonPress;

typedef struct sButton
{
	ui32 countLow;	// ������� ������� ������ ��� ����������� ��������
	ui32 countHigh;	// ������� �������� ������ ��� ����������� ��������
	bool is;		// �������� ����� �������(!)
	bool isHigh;	// ����������� ������� ������, ����� countHigh ������ ������ ������������ ��������
	bool isLow;		// ����������� ������ ������, ����� countLow ������ ������ ������������ ��������
	bool isPress;	// ������ ������, ����� ����������
	bool isLong;	// ���������� �������
	bool beginTimerLong;
	//ui32 iTimerLong;// ������ ��� ����������� ������������ �������
	ui32 timerHighLong;	// ������� ��� ����������� ������������ �������
} cButton;


// -------------------------------- ������ -------------------------------------

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
	if (level == 0) // ������ ������� - ������ ������, ������� - ��������
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
	if (button->is) // ���� ����������
	{
		if (button->countHigh >= BUTTON_HIGH_COUNT) // ����������� ������ (�.�. ��� �� �������)
		{
			button->isHigh = true; // ������ ������
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

		if (button->countLow > BUTTON_LOW_COUNT) // �������� ����������� ������
		{
			button->is = false; // ��������� ����������� ������ � ������

			if (button->isHigh) // ���� ������ ������
			{
				button->isLow = true; // ������ ��������
				button->isHigh = false;
				if (!button->isLong)  button->isPress = true;
				button->isLong = false;
				button->beginTimerLong = false;
			}
		}
	}
}

#endif /* BUTTON_H_ */
