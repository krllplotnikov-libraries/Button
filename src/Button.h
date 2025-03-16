#ifndef BUTTON_H_
#define BUTTON_H_

#include <stdint.h>

#define BUTTON_DEBOUNCE_TIME_MS     5
#define BUTTON_EVENT_FINISH_TIMEOUT 250

typedef enum ButtonEvent ButtonEvent_et;
typedef enum ButtonState ButtonState_et;

typedef struct Button                Button_st;
typedef struct ButtonEventDescriptor ButtonEventDescriptor_st;
typedef struct ButtonStateDescriptor ButtonStateDescriptor_st;
typedef struct ButtonUserFunctions   ButtonUserFunctions_st;

typedef void           (*ButtonEventCallback) (Button_st* button, ButtonEvent_et event, uint8_t counter);
typedef ButtonState_et (*ButtonGetState)      (Button_st* button);
typedef uint32_t       (*ButtonGetTick)       ();

enum ButtonEvent{
	BUTTON_EVENT_NONE,
	BUTTON_EVENT_CLICK,
	BUTTON_EVENT_HOLD,
};

enum ButtonState{
	BUTTON_STATE_RELEASED,
	BUTTON_STATE_PRESSED
};

struct ButtonEventDescriptor{
	uint16_t pressCount           : 4;
	uint16_t releaseCount         : 4;
	uint16_t eventFinish          : 1;
	uint16_t eventCount           : 7;
};

struct ButtonStateDescriptor{
	ButtonState_et currentState;
	ButtonState_et previousState;
	uint32_t       changeTime;
};

struct ButtonUserFunctions{
	ButtonGetState      getState;
	ButtonGetTick       getTick;
	ButtonEventCallback eventCallback;
};

struct Button{
	ButtonUserFunctions_st   userFunctions;
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
