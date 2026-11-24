#include "Button.h"

static void Debounce(Button_st* button){
	button->debounceDescriptor.currentState = button->descriptor.getState(button);

	if(button->debounceDescriptor.currentState != button->debounceDescriptor.previousState){
		button->debounceDescriptor.previousState = button->debounceDescriptor.currentState;
		button->debounceDescriptor.changeTime = button->descriptor.getTick();
	}

	else if(button->debounceDescriptor.currentState == button->debounceDescriptor.previousState
			&& button->descriptor.getTick() > button->debounceDescriptor.changeTime + BUTTON_DEBOUNCE_TIME_MS
			&& button->stateDescriptor.currentState != button->debounceDescriptor.currentState
			&& button->stateDescriptor.changeTime != button->debounceDescriptor.changeTime){
		button->stateDescriptor.currentState = button->debounceDescriptor.currentState;
		button->stateDescriptor.changeTime = button->debounceDescriptor.changeTime;

		if(button->stateDescriptor.currentState == BUTTON_STATE_PRESSED){
			button->descriptor.eventCallback(button, BUTTON_EVENT_PRESS, 1);
		}
		else{
			button->descriptor.eventCallback(button, BUTTON_EVENT_RELEASE, 1);
		}
	}
}

static void Update(Button_st* button){
	if(button->stateDescriptor.previousState != button->stateDescriptor.currentState){
		if(button->stateDescriptor.currentState == BUTTON_STATE_PRESSED && button->eventDescriptor.eventFinish == 0){
			button->eventDescriptor.pressCount++;
		}
		else if(button->stateDescriptor.currentState == BUTTON_STATE_RELEASED && button->eventDescriptor.eventFinish == 0){
			button->eventDescriptor.releaseCount++;
		}
			
		button->stateDescriptor.previousState = button->stateDescriptor.currentState;
	}

	if(button->stateDescriptor.currentState == BUTTON_STATE_RELEASED && button->eventDescriptor.eventFinish == 1){
		*(uint16_t*)&button->eventDescriptor = 0;
	}

	if((button->descriptor.getTick() > button->stateDescriptor.changeTime + BUTTON_EVENT_FINISH_TIMEOUT) && *(uint16_t*)&button->eventDescriptor != 0){
		button->eventDescriptor.eventFinish = 1;
	}
}

static void Event(Button_st* button){
	if(button->eventDescriptor.eventFinish){
		if(button->eventDescriptor.pressCount == button->eventDescriptor.releaseCount
				&& button->eventDescriptor.pressCount > 0
				&& button->eventDescriptor.releaseCount > 0
				&& button->eventDescriptor.eventCount == 0){
			button->eventDescriptor.eventCount = button->eventDescriptor.pressCount;
			button->descriptor.eventCallback(button, BUTTON_EVENT_CLICK, button->eventDescriptor.eventCount);
		}
		else if(button->eventDescriptor.pressCount > button->eventDescriptor.releaseCount){
			button->eventDescriptor.eventCount++;
			button->descriptor.eventCallback(button, BUTTON_EVENT_HOLD, button->eventDescriptor.eventCount);
			button->stateDescriptor.changeTime = button->descriptor.getTick();
			button->eventDescriptor.eventFinish = 0;
		}
		else{
			*(uint16_t*)&button->eventDescriptor = 0;
		}
	}
}


void Button_Init(Button_st* button, ButtonDescriptor_st* descriptor){
	button->descriptor.getState = descriptor->getState;
	button->descriptor.getTick = descriptor->getTick;
	button->descriptor.eventCallback = descriptor->eventCallback;
	button->stateDescriptor.currentState = BUTTON_STATE_RELEASED;
	button->stateDescriptor.previousState = BUTTON_STATE_RELEASED;
	button->debounceDescriptor.currentState = BUTTON_STATE_RELEASED;
	button->debounceDescriptor.previousState = BUTTON_STATE_RELEASED;
	*(uint16_t*)&button->eventDescriptor = 0;
}

void Button_SetEventCallback(Button_st* button, ButtonEventCallback eventCallback){
	button->descriptor.eventCallback = eventCallback;
}

void Button_Run(Button_st* button){
	Debounce(button);
	Update(button);
	Event(button);
}
