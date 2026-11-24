#ifndef BUTTON_H_
#define BUTTON_H_

#include <stddef.h>
#include <stdint.h>

#define BUTTON_DEBOUNCE_TIME_MS        5
#define BUTTON_EVENT_FINISH_TIMEOUT_MS 250
#define BUTTON_MAX_EVENTS_COUNT        0x3D
#define BUTTON_MAX_CLICK_COUNT         0x0F

enum Button_Event{
	BUTTON_EVENT_NONE,
	BUTTON_EVENT_CLICK,
	BUTTON_EVENT_HOLD,
	BUTTON_EVENT_PRESS,
	BUTTON_EVENT_RELEASE
};

enum Button_State{
	BUTTON_STATE_RELEASED,
	BUTTON_STATE_PRESSED
};

typedef enum Button_Event Button_Event_et;
typedef enum Button_State Button_State_et;

typedef struct Button_EventDescriptor Button_EventDescriptor_st;
typedef struct Button_StateDescriptor Button_StateDescriptor_st;
typedef struct Button_Descriptor      Button_Descriptor_st;
typedef struct Button                 Button_st;

typedef void            (*Button_EventCallback) (Button_st* _button, Button_Event_et _event, uint8_t _counter);
typedef Button_State_et (*Button_GetState)      (Button_st* _button);
typedef uint32_t        (*Button_GetTick)       (void);

struct Button_EventDescriptor{
	uint16_t pressCount   : 4;
	uint16_t releaseCount : 4;
	uint16_t eventFinish  : 1;
	uint16_t eventCount   : 7;
};

struct Button_StateDescriptor{
	Button_State_et currentState;
	Button_State_et previousState;
	uint32_t        changeTime;
};

struct Button_Descriptor{
	Button_GetState      getState;
	Button_GetTick       getTick;
	Button_EventCallback eventCallback;
	void*                context;
};

struct Button{
	Button_Descriptor_st      descriptor;
	Button_EventDescriptor_st eventDescriptor;
	Button_StateDescriptor_st stateDescriptor;
	Button_StateDescriptor_st debounceDescriptor;
};

#ifdef __cplusplus
extern "C" {
#endif

void Button_Init(Button_st* _button, Button_Descriptor_st* _descriptor);
void Button_Run(Button_st* _button);

void Button_SetEventCallback(Button_st* _button, Button_EventCallback _eventCallback, void* _context);

#ifdef __cplusplus
}
#endif

#endif
