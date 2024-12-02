#ifndef BUTTON_H_
#define BUTTON_H_

#include <stdio.h>
#include <string.h>

#define BUTTON_DEBOUNCE_TRESHOLD_MS          25
#define BUTTON_CLICK_TRESHOLD_MS             50
#define BUTTON_CLICK_TIMEOUT_MS              250
#define BUTTON_HOLD_TRESHOLD_MS              500

#define CLICK_EVENT_COMBINATION_CLICK        0b00100101
#define CLICK_EVENT_COMBINATION_DOUBLE_CLICK 0b00001010
#define CLICK_EVENT_COMBINATION_HOLD         0b00010001

enum ButtonEvent_e{
	BUTTON_EVENT_CLICK,
	BUTTON_EVENT_DOUBLE_CLICK,
	BUTTON_EVENT_HOLD,
	BUTTON_EVENT_PRESSED,
	BUTTON_EVENT_RELEASED
};

enum ButtonState_e{
	BUTTON_STATE_PRESSED,
	BUTTON_STATE_RELEASED
};

typedef uint8_t ButtonEvent_t;
typedef uint8_t ButtonState_t;

typedef ButtonState_t (*GetLevel)();
typedef uint32_t (*GetTick)();

typedef struct{
	uint8_t pressChange : 2;
	uint8_t releaseChange: 2;
	uint8_t waitHoldTreshold : 1;
	uint8_t waitClickTimeout : 1;
	uint8_t reserved : 2;
}ButtonEventDescriptor_st;

typedef union
{
    ButtonEventDescriptor_st InStruct;
    uint8_t InInt;
}ButtonEventDescriptor_u;

typedef struct{
	GetLevel getLevel;
	GetTick getTick;
	
	ButtonState_t currentState;
	ButtonState_t previousState;
	uint32_t lastChangedStateTime;
	
	ButtonState_t currentDebounceState;
	ButtonState_t previousDebounceState;
	uint32_t lastChangedDebounceStateTime;
	
	ButtonEventDescriptor_u buttonEventDescriptor;
}Button_st;

void Button_Init(Button_st* button, GetLevel getLevel, GetTick getTick);
void Button_Run(Button_st* button);

void Button_Callback(Button_st* button, ButtonEvent_t event);

#endif