#include "button.h"


void Button_Init(Button_st* button, GetLevel getLevel, GetTick getTick){
	button->getLevel = getLevel;
	button->getTick = getTick;
	button->currentState = BUTTON_STATE_RELEASED;
	button->previousState = BUTTON_STATE_RELEASED;
	button->currentDebounceState = BUTTON_STATE_RELEASED;
	button->previousDebounceState = BUTTON_STATE_RELEASED;
}

void Button_Run(Button_st* button){
	button->currentDebounceState = button->getLevel();
	if(button->currentDebounceState != button->previousDebounceState){
		button->previousDebounceState = button->currentDebounceState;
		button->lastChangedDebounceStateTime = button->getTick();
	}
	
	if(button->currentDebounceState == button->previousDebounceState && button->getTick() > button->lastChangedDebounceStateTime + BUTTON_DEBOUNCE_TRESHOLD_MS){
		button->currentState = button->getLevel();
		if(button->previousState == BUTTON_STATE_RELEASED && button->currentState == BUTTON_STATE_PRESSED){
			button->buttonEventDescriptor.InStruct.pressChange++;
			button->previousState = BUTTON_STATE_PRESSED;
			button->buttonEventDescriptor.InStruct.waitHoldTreshold = 0;
			button->buttonEventDescriptor.InStruct.waitClickTimeout = 0;
			Button_Callback(button, BUTTON_EVENT_PRESSED);
		}
		else if(button->previousState == BUTTON_STATE_PRESSED && button->currentState == BUTTON_STATE_RELEASED){
			button->buttonEventDescriptor.InStruct.releaseChange++;
			button->previousState = BUTTON_STATE_RELEASED;
			button->buttonEventDescriptor.InStruct.waitHoldTreshold = 0;
			button->buttonEventDescriptor.InStruct.waitClickTimeout = 0;
			Button_Callback(button, BUTTON_EVENT_RELEASED);
		}
	}
	
	if(button->getTick() > button->lastChangedDebounceStateTime + BUTTON_HOLD_TRESHOLD_MS && button->currentState == BUTTON_STATE_PRESSED){
		button->buttonEventDescriptor.InStruct.waitHoldTreshold = 1;
	}
	
	if(button->getTick() > button->lastChangedDebounceStateTime + BUTTON_CLICK_TIMEOUT_MS && button->currentState == BUTTON_STATE_RELEASED){
		button->buttonEventDescriptor.InStruct.waitClickTimeout = 1;
	}
	
	switch(button->buttonEventDescriptor.InInt){
		case CLICK_EVENT_COMBINATION_CLICK: Button_Callback(button, BUTTON_EVENT_CLICK); button->buttonEventDescriptor.InInt = 0; break;
		case CLICK_EVENT_COMBINATION_DOUBLE_CLICK: Button_Callback(button, BUTTON_EVENT_DOUBLE_CLICK); button->buttonEventDescriptor.InInt = 0; break;
		case CLICK_EVENT_COMBINATION_HOLD: Button_Callback(button, BUTTON_EVENT_HOLD); button->buttonEventDescriptor.InInt = 0; button->buttonEventDescriptor.InStruct.releaseChange = -1; break;
	}
	
}

__attribute((weak)) void Button_Callback(Button_st* button, ButtonEvent_t event);