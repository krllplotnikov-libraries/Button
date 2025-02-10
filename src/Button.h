#ifndef BUTTON_H_
#define BUTTON_H_

#include <stdint.h>

#define BUTTON_DEBOUNCE_TIME_MS               25
#define BUTTON_CLICK_TIMEOUT_MS               250
#define BUTTON_HOLD_TIMEOUT_MS                500
#define BUTTON_HOLDED_TIMEOUT_MS              1000
#define BUTTON_EVENT_DESCRIPTOR_RESET_TIMEOUT 300

enum ButtonEvent{
	BUTTON_EVENT_NONE,
	BUTTON_EVENT_CLICK,
	BUTTON_EVENT_DOUBLE_CLICK,
	BUTTON_EVENT_HOLD,
	BUTTON_EVENT_HOLDED,
	BUTTON_EVENT_PRESSED,
	BUTTON_EVENT_RELEASED
};

enum ButtonState{
	BUTTON_STATE_RELEASED,
	BUTTON_STATE_PRESSED
};

typedef enum ButtonEvent ButtonEvent_et;
typedef enum ButtonState ButtonState_et;

typedef struct Button Button_st;
typedef struct ButtonEventDescriptor ButtonEventDescriptor_st;
typedef struct ButtonStateDescriptor ButtonStateDescriptor_st;
typedef struct ButtonDebounceDescriptor ButtonDebounceDescriptor_st;
typedef struct ButtonUserFunctions ButtonUserFunctions_st;

typedef void (*ButtonEventCallback)(Button_st* button, ButtonEvent_et event);
typedef ButtonState_et (*ButtonGetState_et)(Button_st* button);
typedef uint32_t (*GetTick)();



struct ButtonEventDescriptor{
	uint8_t pressCount           : 2;
	uint8_t releaseCount         : 2;
	uint8_t pressReady           : 1;
	uint8_t pressCompleted       : 1;
	uint8_t releaseReady         : 1;
	uint8_t releaseCompleted     : 1;
	uint8_t clickReady           : 1;
	uint8_t clickCompleted       : 1;
	uint8_t doubleClickReady     : 1;
	uint8_t doubleClickCompleted : 1;
	uint8_t holdReady            : 1;
	uint8_t holdCompleted        : 1;
	uint8_t holdedReady          : 1;
	uint8_t holdedCompleted      : 1;
};

struct ButtonStateDescriptor{
	ButtonState_et currentState;
	ButtonState_et previousState;
	uint32_t changeTime;
};

struct ButtonUserFunctions{
	ButtonGetState_et getState;
	GetTick getTick;

	ButtonEventCallback eventCallback;
};

struct Button{
	ButtonUserFunctions_st userFunctions;
	ButtonEventDescriptor_st eventDescriptor;
	ButtonStateDescriptor_st stateDescriptor;
	ButtonStateDescriptor_st debounceDescriptor;
};

#ifdef __cplusplus
extern "C" {
#endif

void Button_Init(Button_st* button, ButtonUserFunctions_st* userFunctions);
void Button_Run(Button_st* button);

#ifdef __cplusplus
}
#endif

#endif
