#include "Button.h"

static void Debounce(Button_st* button){
	button->debounceDescriptor.currentState = button->userFunctions.getPinLevel(button);

	if(button->debounceDescriptor.currentState != button->debounceDescriptor.previousState){
		button->debounceDescriptor.previousState = button->debounceDescriptor.currentState;
		button->debounceDescriptor.changeTime = button->userFunctions.getTick();
	}

	else if(button->debounceDescriptor.currentState == button->debounceDescriptor.previousState &&
			button->stateDescriptor.changeTime != button->debounceDescriptor.changeTime &&
			button->userFunctions.getTick() > button->debounceDescriptor.changeTime + BUTTON_DEBOUNCE_TIME_MS){
		button->stateDescriptor.currentState = button->debounceDescriptor.currentState;
		button->stateDescriptor.changeTime = button->debounceDescriptor.changeTime;
	}
}

static void UpdateDescriptors(Button_st* button){
	if(button->stateDescriptor.previousState != button->stateDescriptor.currentState){
		if(button->stateDescriptor.currentState == BUTTON_STATE_PRESSED){
			button->eventDescriptor.pressCount++;
			button->eventDescriptor.pressReady = 1;
		}
		else if(button->stateDescriptor.currentState == BUTTON_STATE_RELEASED){
			button->eventDescriptor.releaseCount++;
			button->eventDescriptor.releaseReady = 1;
		}
		button->stateDescriptor.previousState = button->stateDescriptor.currentState;
	}

	if(button->stateDescriptor.currentState == BUTTON_STATE_RELEASED &&
			button->userFunctions.getTick() > button->stateDescriptor.changeTime + BUTTON_EVENT_DESCRIPTOR_RESET_TIMEOUT){
		*(uint16_t*)&button->eventDescriptor = 0;
	}

	if(button->eventDescriptor.pressCount == 1 &&
			button->eventDescriptor.releaseCount == 0 &&
			button->eventDescriptor.holdCompleted == 0 &&
			button->userFunctions.getTick() > button->stateDescriptor.changeTime + BUTTON_HOLD_TIMEOUT_MS){
		button->eventDescriptor.holdReady = 1;
	}

	if(button->eventDescriptor.pressCount == 1 &&
			button->eventDescriptor.releaseCount == 0 &&
			button->eventDescriptor.holdCompleted == 1 &&
			button->userFunctions.getTick() > button->stateDescriptor.changeTime + BUTTON_HOLD_TIMEOUT_MS + BUTTON_HOLDED_TIMEOUT_MS){
		button->eventDescriptor.holdedReady = 1;
	}

	if(button->eventDescriptor.pressCount == 1 &&
			button->eventDescriptor.releaseCount == 1 &&
			button->eventDescriptor.clickCompleted == 0 &&
			button->userFunctions.getTick() > button->stateDescriptor.changeTime + BUTTON_CLICK_TIMEOUT_MS){
		button->eventDescriptor.clickReady = 1;
	}

	if(button->eventDescriptor.pressCount == 2 &&
			button->eventDescriptor.releaseCount == 2 &&
			button->eventDescriptor.doubleClickCompleted == 0){
		button->eventDescriptor.doubleClickReady = 1;
	}
}

static void MakeEvent(Button_st* button){
	if(button->eventDescriptor.pressReady){
		button->userFunctions.eventCallback(button, BUTTON_EVENT_PRESSED);
		button->eventDescriptor.pressReady = 0;
		button->eventDescriptor.pressCompleted = 1;
	}

	else if(button->eventDescriptor.releaseReady){
		button->userFunctions.eventCallback(button, BUTTON_EVENT_RELEASED);
		button->eventDescriptor.releaseReady = 0;
		button->eventDescriptor.releaseCompleted = 1;
	}

	else if(button->eventDescriptor.holdReady){
		button->userFunctions.eventCallback(button, BUTTON_EVENT_HOLD);
		button->eventDescriptor.holdReady = 0;
		button->eventDescriptor.holdCompleted = 1;
	}

	else if(button->eventDescriptor.holdedReady){
		button->userFunctions.eventCallback(button, BUTTON_EVENT_HOLDED);
	}

	else if(button->eventDescriptor.clickReady){
		button->userFunctions.eventCallback(button, BUTTON_EVENT_CLICK);
		button->eventDescriptor.clickReady = 0;
		button->eventDescriptor.clickCompleted = 1;
	}

	else if(button->eventDescriptor.doubleClickReady){
		button->userFunctions.eventCallback(button, BUTTON_EVENT_DOUBLE_CLICK);
		button->eventDescriptor.doubleClickReady = 0;
		button->eventDescriptor.doubleClickCompleted = 1;
	}
}


void Button_Init(Button_st* button, ButtonUserFunctions_st* userFunctions){
	button->userFunctions.getPinLevel = userFunctions->getPinLevel;
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
	UpdateDescriptors(button);
	MakeEvent(button);
}
