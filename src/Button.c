#include "Button.h"

static void Debounce(Button_st* button){
	button->debounceDescriptor.currentState = button->userFunctions.getState(button);

	if(button->debounceDescriptor.currentState != button->debounceDescriptor.previousState){
		button->debounceDescriptor.previousState = button->debounceDescriptor.currentState;
		button->debounceDescriptor.changeTime = button->userFunctions.getTick();
	}

	else if(button->debounceDescriptor.currentState == button->debounceDescriptor.previousState
			&& button->userFunctions.getTick() > button->debounceDescriptor.changeTime + BUTTON_DEBOUNCE_TIME_MS
			&& button->stateDescriptor.currentState != button->debounceDescriptor.currentState
			&& button->stateDescriptor.changeTime != button->debounceDescriptor.changeTime){
		button->stateDescriptor.currentState = button->debounceDescriptor.currentState;
		button->stateDescriptor.changeTime = button->debounceDescriptor.changeTime;
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

	if((button->userFunctions.getTick() > button->stateDescriptor.changeTime + BUTTON_EVENT_FINISH_TIMEOUT) && *(uint16_t*)&button->eventDescriptor != 0){
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
			button->userFunctions.eventCallback(button, BUTTON_EVENT_CLICK, button->eventDescriptor.eventCount);
		}
		else if(button->eventDescriptor.pressCount > button->eventDescriptor.releaseCount){
			button->eventDescriptor.eventCount++;
			button->userFunctions.eventCallback(button, BUTTON_EVENT_HOLD, button->eventDescriptor.eventCount);
			button->stateDescriptor.changeTime = button->userFunctions.getTick();
			button->eventDescriptor.eventFinish = 0;
		}
		else{
			*(uint16_t*)&button->eventDescriptor = 0;
		}
	}
}


void Button_Init(Button_st* button, ButtonUserFunctions_st* userFunctions){
	button->userFunctions.getState = userFunctions->getState;
	button->userFunctions.getTick = userFunctions->getTick;
	button->userFunctions.eventCallback = userFunctions->eventCallback;
	button->stateDescriptor.currentState = BUTTON_STATE_RELEASED;
	button->stateDescriptor.previousState = BUTTON_STATE_RELEASED;
	button->debounceDescriptor.currentState = BUTTON_STATE_RELEASED;
	button->debounceDescriptor.previousState = BUTTON_STATE_RELEASED;
	*(uint16_t*)&button->eventDescriptor = 0;
}

void Button_Run(Button_st* button){
	Debounce(button);
	Update(button);
	Event(button);
}
