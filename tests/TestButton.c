#include "MicroTest.h"
#include "Button.h"

//-----Types-----
enum ButtonType{
	BUTTON_TYPE_NORMAL_OPEN,
	BUTTON_TYPE_NORMAL_CLOSE
};

enum ButtonPull{
	BUTTON_PULL_DOWN,
	BUTTON_PULL_UP
};

typedef enum ButtonType ButtonType_et;
typedef enum ButtonPull ButtonPull_et;

typedef struct EntityButton EntityButton_st;
typedef struct EntityMCU    EntityMCU_st;
typedef struct EventResult  EventResult_st;

struct EntityButton{
	Button_State_et state;
	ButtonType_et   type;
	ButtonPull_et   pull;
};

struct EntityMCU{
	uint32_t tick;
	uint32_t pinState;
};

struct EventResult{
	Button_st*      button;
	Button_Event_et event;
	uint8_t         counter;
};
//-----Types-----



//-----Variables-----
static EntityButton_st entityButton = {
	.state = BUTTON_STATE_RELEASED,
	.type  = BUTTON_TYPE_NORMAL_OPEN,
	.pull  = BUTTON_PULL_UP,
};

static EntityMCU_st mcu = {
	.tick     = 0xFFFFFF00, // tick counter overflow is covered by tests
	.pinState = 0,
};

static EventResult_st eventResult = {0};
static Button_st      button      = {0};
//-----Variables-----



//-----Function prototypes-----
static void            fakeEventCallback(Button_st* _button, Button_Event_et _event, uint8_t _counter);
static Button_State_et fakeGetState(Button_st* _button);
static uint32_t        fakeGetTick(void);

static void            mcuIncTick(EntityMCU_st* _mcu, uint32_t _ticks);
static void            mcuSetPinState(EntityMCU_st* _mcu, uint32_t _pin, uint8_t _state);
static uint32_t        mcuGetTick(EntityMCU_st* _mcu);
static uint8_t         mcuGetPinState(EntityMCU_st* _mcu, uint32_t _pin);

static void            runButton(Button_st* _button, EntityMCU_st* _mcu, uint32_t _duration);
static void            simulateInit(EntityMCU_st* _mcu, Button_st* _button);
static void            simulatePress(EntityButton_st* _entityButton, EntityMCU_st* _mcu, Button_st* _button, uint32_t _duration);
static void            simulateRelease(EntityButton_st* _entityButton, EntityMCU_st* _mcu, Button_st* _button, uint32_t _duration);
static void            assertEvent(Button_Event_et _event, uint8_t _counter);

static void            testInit(void);
static void            testEventPress(void);
static void            testEventRelease(void);
static void            testEventClick(void);
static void            testEventHold(void);
static void            testIntegrationMulticlick(void);
static void            testIntegrationLongHold(void);
static void            testIntegrationHoldBeforeTurnOn(void);
static void            testIntegrationDebounce(void);
static void            testBoundaryMaxClicks(void);
//-----Function prototypes-----



int main(void){
	MICROTEST_BEGIN();

	MICROTEST_RUN_TEST(testInit);

	MICROTEST_RUN_TEST(testEventPress);
	MICROTEST_RUN_TEST(testEventRelease);
	MICROTEST_RUN_TEST(testEventClick);
	MICROTEST_RUN_TEST(testEventHold);

	MICROTEST_RUN_TEST(testIntegrationMulticlick);
	MICROTEST_RUN_TEST(testIntegrationLongHold);
	MICROTEST_RUN_TEST(testIntegrationHoldBeforeTurnOn);
	MICROTEST_RUN_TEST(testIntegrationDebounce);

	MICROTEST_RUN_TEST(testBoundaryMaxClicks);

	return (int)MICROTEST_END();
}



//-----Function definitions-----
void MicroTest_SetUp(void){
	Button_Descriptor_st buttonDescriptor = {
		.getState      = fakeGetState,
		.getTick       = fakeGetTick,
		.eventCallback = fakeEventCallback,
		.context       = NULL,
	};

	eventResult.button  = NULL;
	eventResult.event   = BUTTON_EVENT_NONE;
	eventResult.counter = 0;

	Button_Init(&button, &buttonDescriptor);
}

//-----Init tests-----
static void testInit(void){
	MICROTEST_ASSERT_EQUAL_PTR(fakeGetTick, button.descriptor.getTick);
	MICROTEST_ASSERT_EQUAL_PTR(fakeGetState, button.descriptor.getState);
	MICROTEST_ASSERT_EQUAL_PTR(fakeEventCallback, button.descriptor.eventCallback);
	MICROTEST_ASSERT_NULL(button.descriptor.context);
}

//-----Event tests-----
static void testEventPress(void){
	simulateInit(&mcu, &button);
	simulatePress(&entityButton, &mcu, &button, BUTTON_DEBOUNCE_TIME_MS + 5);

	assertEvent(BUTTON_EVENT_PRESS, 1);
}

static void testEventRelease(void){
	simulateInit(&mcu, &button);
	simulatePress(&entityButton, &mcu, &button, BUTTON_DEBOUNCE_TIME_MS + 5);
	simulateRelease(&entityButton, &mcu, &button, BUTTON_DEBOUNCE_TIME_MS + 5);

	assertEvent(BUTTON_EVENT_RELEASE, 1);
}

static void testEventClick(void){
	simulateInit(&mcu, &button);
	simulatePress(&entityButton, &mcu, &button, BUTTON_DEBOUNCE_TIME_MS + 5);
	simulateRelease(&entityButton, &mcu, &button, BUTTON_EVENT_FINISH_TIMEOUT_MS + 5);

	assertEvent(BUTTON_EVENT_CLICK, 1);
}

static void testEventHold(void){
	simulateInit(&mcu, &button);
	simulatePress(&entityButton, &mcu, &button, BUTTON_EVENT_FINISH_TIMEOUT_MS + 5);

	assertEvent(BUTTON_EVENT_HOLD, 1);
}

//-----Integration tests-----
static void testIntegrationMulticlick(void){
	simulateInit(&mcu, &button);

	for(uint8_t clicks = 1; clicks < BUTTON_MAX_CLICK_COUNT; clicks++){
		for(uint8_t click = 0; click < clicks; click++){
			simulatePress(&entityButton, &mcu, &button, BUTTON_DEBOUNCE_TIME_MS + 5);
			simulateRelease(&entityButton, &mcu, &button, BUTTON_DEBOUNCE_TIME_MS + 5);
		}

		simulateRelease(&entityButton, &mcu, &button, BUTTON_EVENT_FINISH_TIMEOUT_MS + 5);

		assertEvent(BUTTON_EVENT_CLICK, clicks);
	}
}

static void testIntegrationLongHold(void){
	simulateInit(&mcu, &button);
	simulatePress(&entityButton, &mcu, &button, BUTTON_DEBOUNCE_TIME_MS + 5);

	for(uint8_t holdEvents = 1; holdEvents < BUTTON_MAX_EVENTS_COUNT; holdEvents++){
		simulatePress(&entityButton, &mcu, &button, BUTTON_EVENT_FINISH_TIMEOUT_MS + 1);

		assertEvent(BUTTON_EVENT_HOLD, holdEvents);
	}
}

static void testIntegrationHoldBeforeTurnOn(void){
	simulatePress(&entityButton, &mcu, &button, 1);
	simulateInit(&mcu, &button);

	simulatePress(&entityButton, &mcu, &button, BUTTON_EVENT_FINISH_TIMEOUT_MS * 2);

	assertEvent(BUTTON_EVENT_HOLD, 2);
}

static void testIntegrationDebounce(void){
	MICROTEST_IGNORE_MESSAGE("debounce scenario is not finalized yet");

	simulateInit(&mcu, &button);
	simulatePress(&entityButton, &mcu, &button, BUTTON_DEBOUNCE_TIME_MS + BUTTON_DEBOUNCE_TIME_MS - 1);
	simulateRelease(&entityButton, &mcu, &button, BUTTON_DEBOUNCE_TIME_MS + BUTTON_DEBOUNCE_TIME_MS - 1);

	assertEvent(BUTTON_EVENT_NONE, 0);
}

//-----Boundary tests-----
static void testBoundaryMaxClicks(void){
	simulateInit(&mcu, &button);

	// 15 clicks -> 16th overflows to 0, 17th overflows to 1
	for(uint8_t clicks = 0; clicks < BUTTON_MAX_CLICK_COUNT + 2; clicks++){
		simulatePress(&entityButton, &mcu, &button, BUTTON_DEBOUNCE_TIME_MS + 5);
		simulateRelease(&entityButton, &mcu, &button, BUTTON_DEBOUNCE_TIME_MS + 5);
	}

	simulateRelease(&entityButton, &mcu, &button, BUTTON_EVENT_FINISH_TIMEOUT_MS + 5);

	assertEvent(BUTTON_EVENT_CLICK, 1);
}

//-----Fakes-----
static void fakeEventCallback(Button_st* _button, Button_Event_et _event, uint8_t _counter){
	eventResult.button  = _button;
	eventResult.event   = _event;
	eventResult.counter = _counter;
}

static Button_State_et fakeGetState(Button_st* _button){
	if(_button == &button){
		if(mcuGetPinState(&mcu, 0)){
			return BUTTON_STATE_PRESSED;
		}
	}

	return BUTTON_STATE_RELEASED;
}

static uint32_t fakeGetTick(void){
	return mcuGetTick(&mcu);
}

//-----Helpers-----
static void mcuIncTick(EntityMCU_st* _mcu, uint32_t _ticks){
	_mcu->tick += _ticks;
}

static void mcuSetPinState(EntityMCU_st* _mcu, uint32_t _pin, uint8_t _state){
	if(_state){
		_mcu->pinState |= (1UL << _pin);
	}
	else{
		_mcu->pinState &= ~(1UL << _pin);
	}
}

static uint32_t mcuGetTick(EntityMCU_st* _mcu){
	return _mcu->tick;
}

static uint8_t mcuGetPinState(EntityMCU_st* _mcu, uint32_t _pin){
	return (_mcu->pinState & (1UL << _pin)) ? 1 : 0;
}

static void runButton(Button_st* _button, EntityMCU_st* _mcu, uint32_t _duration){
	for(uint32_t i = 0; i < _duration; i++){
		Button_Run(_button);
		mcuIncTick(_mcu, 1);
	}
}

static void simulateInit(EntityMCU_st* _mcu, Button_st* _button){
	runButton(_button, _mcu, 50);
}

static void simulatePress(EntityButton_st* _entityButton, EntityMCU_st* _mcu, Button_st* _button, uint32_t _duration){
	_entityButton->state = BUTTON_STATE_PRESSED;
	if(_entityButton->type == BUTTON_TYPE_NORMAL_OPEN && _entityButton->pull == BUTTON_PULL_UP){
		mcuSetPinState(_mcu, 0, 1);
	}

	runButton(_button, _mcu, _duration);
}

static void simulateRelease(EntityButton_st* _entityButton, EntityMCU_st* _mcu, Button_st* _button, uint32_t _duration){
	_entityButton->state = BUTTON_STATE_RELEASED;
	if(_entityButton->type == BUTTON_TYPE_NORMAL_OPEN && _entityButton->pull == BUTTON_PULL_UP){
		mcuSetPinState(_mcu, 0, 0);
	}

	runButton(_button, _mcu, _duration);
}

static void assertEvent(Button_Event_et _event, uint8_t _counter){
	MICROTEST_ASSERT_EQUAL_PTR(&button, eventResult.button);
	MICROTEST_ASSERT_EQUAL_INT(_event, eventResult.event);
	MICROTEST_ASSERT_EQUAL_UINT8(_counter, eventResult.counter);
}
//-----Function definitions-----
