#include "Button.h"

static void    debounce(Button_st* _button, uint32_t _currentTime);
static void    update(Button_st* _button, uint32_t _currentTime);
static void    processEvent(Button_st* _button, uint32_t _currentTime);
static uint8_t isEventActive(Button_st* _button);
static void    resetEvent(Button_st* _button);
static void    sendEvent(Button_st* _button, Button_Event_et _event, uint8_t _counter);

void Button_Init(Button_st* _button, Button_Descriptor_st* _descriptor){
	_button->descriptor = *_descriptor;

	resetEvent(_button);

	_button->stateDescriptor.currentState  = BUTTON_STATE_RELEASED;
	_button->stateDescriptor.previousState = BUTTON_STATE_RELEASED;
	_button->stateDescriptor.changeTime    = 0;

	_button->debounceDescriptor.currentState  = BUTTON_STATE_RELEASED;
	_button->debounceDescriptor.previousState = BUTTON_STATE_RELEASED;
	_button->debounceDescriptor.changeTime    = 0;
}

void Button_Run(Button_st* _button){
	uint32_t currentTime = _button->descriptor.getTick();

	debounce(_button, currentTime);
	update(_button, currentTime);
	processEvent(_button, currentTime);
}

void Button_SetEventCallback(Button_st* _button, Button_EventCallback _eventCallback, void* _context){
	_button->descriptor.eventCallback = _eventCallback;
	_button->descriptor.context       = _context;
}

static void debounce(Button_st* _button, uint32_t _currentTime){
	Button_StateDescriptor_st* debounceDescriptor = &_button->debounceDescriptor;
	Button_StateDescriptor_st* stateDescriptor    = &_button->stateDescriptor;

	debounceDescriptor->currentState = _button->descriptor.getState(_button);

	if(debounceDescriptor->currentState != debounceDescriptor->previousState){
		debounceDescriptor->previousState = debounceDescriptor->currentState;
		debounceDescriptor->changeTime    = _currentTime;
	}
	else if((_currentTime - debounceDescriptor->changeTime) >= BUTTON_DEBOUNCE_TIME_MS
			&& stateDescriptor->currentState != debounceDescriptor->currentState
			&& stateDescriptor->changeTime != debounceDescriptor->changeTime){
		stateDescriptor->currentState = debounceDescriptor->currentState;
		stateDescriptor->changeTime   = debounceDescriptor->changeTime;

		if(stateDescriptor->currentState == BUTTON_STATE_PRESSED){
			sendEvent(_button, BUTTON_EVENT_PRESS, 1);
		}
		else{
			sendEvent(_button, BUTTON_EVENT_RELEASE, 1);
		}
	}
}

static void update(Button_st* _button, uint32_t _currentTime){
	Button_EventDescriptor_st* eventDescriptor = &_button->eventDescriptor;
	Button_StateDescriptor_st* stateDescriptor = &_button->stateDescriptor;

	if(stateDescriptor->previousState != stateDescriptor->currentState){
		if(stateDescriptor->currentState == BUTTON_STATE_PRESSED && eventDescriptor->eventFinish == 0){
			eventDescriptor->pressCount++;
		}
		else if(stateDescriptor->currentState == BUTTON_STATE_RELEASED && eventDescriptor->eventFinish == 0){
			eventDescriptor->releaseCount++;
		}

		stateDescriptor->previousState = stateDescriptor->currentState;
	}

	if(stateDescriptor->currentState == BUTTON_STATE_RELEASED && eventDescriptor->eventFinish == 1){
		resetEvent(_button);
	}

	if((_currentTime - stateDescriptor->changeTime) >= BUTTON_EVENT_FINISH_TIMEOUT_MS && isEventActive(_button)){
		eventDescriptor->eventFinish = 1;
	}
}

static void processEvent(Button_st* _button, uint32_t _currentTime){
	Button_EventDescriptor_st* eventDescriptor = &_button->eventDescriptor;

	if(eventDescriptor->eventFinish == 0){
		return;
	}

	if(eventDescriptor->pressCount == eventDescriptor->releaseCount
			&& eventDescriptor->pressCount > 0
			&& eventDescriptor->eventCount == 0){
		eventDescriptor->eventCount = eventDescriptor->pressCount;
		sendEvent(_button, BUTTON_EVENT_CLICK, eventDescriptor->eventCount);
	}
	else if(eventDescriptor->pressCount > eventDescriptor->releaseCount){
		eventDescriptor->eventCount++;
		sendEvent(_button, BUTTON_EVENT_HOLD, eventDescriptor->eventCount);

		_button->stateDescriptor.changeTime = _currentTime;
		eventDescriptor->eventFinish        = 0;
	}
	else{
		resetEvent(_button);
	}
}

static uint8_t isEventActive(Button_st* _button){
	Button_EventDescriptor_st* eventDescriptor = &_button->eventDescriptor;

	return eventDescriptor->pressCount   != 0
		|| eventDescriptor->releaseCount != 0
		|| eventDescriptor->eventFinish  != 0
		|| eventDescriptor->eventCount   != 0;
}

static void resetEvent(Button_st* _button){
	_button->eventDescriptor.pressCount   = 0;
	_button->eventDescriptor.releaseCount = 0;
	_button->eventDescriptor.eventFinish  = 0;
	_button->eventDescriptor.eventCount   = 0;
}

static void sendEvent(Button_st* _button, Button_Event_et _event, uint8_t _counter){
	if(_button->descriptor.eventCallback != NULL){
		_button->descriptor.eventCallback(_button, _event, _counter);
	}
}
