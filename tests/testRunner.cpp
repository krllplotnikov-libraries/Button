#include <gtest/gtest.h>

#include "Button.h"



/*--------------------------------------------------------------------------------
                        Local data types
--------------------------------------------------------------------------------*/
typedef enum{
    NORMAL_OPEN,
    NORMAL_CLOSE
}ButtonType_et;

typedef enum{
    PULL_DOWN,
    PULL_UP
}ButtonPull_et;

typedef struct{
    ButtonState_et state;
    ButtonType_et type;
    ButtonPull_et pull;
}EntityButton_st;

typedef struct{
    uint32_t tick;
    uint32_t pinState;
}EntityMCU_st;

typedef struct{
    Button_st* button;
    ButtonEvent_et event;
    uint8_t counter;
}Result_st;

EntityButton_st entityButton0 = {
    .state = BUTTON_STATE_RELEASED,
    .type = NORMAL_OPEN,
    .pull = PULL_UP
};

EntityButton_st entityButton1 = {};
EntityMCU_st MCU = {};
Result_st result = {};



/*--------------------------------------------------------------------------------
                        Helper functions prototypes
--------------------------------------------------------------------------------*/
void MCU_IncTick(EntityMCU_st* mcu, uint32_t ticks);
void MCU_SetPinState(EntityMCU_st* mcu, uint32_t pin, uint8_t state);
uint32_t MCU_GetTick(EntityMCU_st* mcu);
uint8_t MCU_GetPinState(EntityMCU_st* mcu, uint32_t pin);

void Simulate_Init(EntityButton_st* entityButton, EntityMCU_st* mcu, Button_st* button, uint32_t duration);
void Simulate_Press(EntityButton_st* entityButton, EntityMCU_st* mcu, Button_st* button, uint32_t duration);
void Simulate_Release(EntityButton_st* entityButton, EntityMCU_st* mcu, Button_st* button, uint32_t duration);



/*--------------------------------------------------------------------------------
                        Library
--------------------------------------------------------------------------------*/
void Button_EventCallback(Button_st* button, ButtonEvent_et event, uint8_t counter);
ButtonState_et Button_GetState(Button_st* button);
uint32_t Button_GetTick();

Button_st button0 = {0};
ButtonUserFunctions_st buttonUserFunctions = {
    .getState = Button_GetState,
    .getTick = Button_GetTick,    
    .eventCallback = Button_EventCallback
};

void Button_EventCallback(Button_st* button, ButtonEvent_et event, uint8_t counter){
    result.button = button;
    result.event = event;
    result.counter = counter;
}

ButtonState_et Button_GetState(Button_st* button){
    if(button == &button0){
        return (ButtonState_et)MCU_GetPinState(&MCU, 0);
    }
    return BUTTON_STATE_RELEASED;
}

uint32_t Button_GetTick(){
    return MCU_GetTick(&MCU);
}



/*--------------------------------------------------------------------------------
                        Tests
--------------------------------------------------------------------------------*/



/*--------------------------------------------------------------------------------
                        Init Tests
--------------------------------------------------------------------------------*/
TEST(InitTest, InitTest){
    Button_Init(&button0, &buttonUserFunctions);

    ASSERT_EQ(button0.userFunctions.getTick, Button_GetTick);
    ASSERT_EQ(button0.userFunctions.getState, Button_GetState);
    ASSERT_EQ(button0.userFunctions.eventCallback, Button_EventCallback);
}

/*--------------------------------------------------------------------------------
                        Event Tests
--------------------------------------------------------------------------------*/
TEST(PressTest, EventTest){
    Button_Init(&button0, &buttonUserFunctions);

    Simulate_Init(&entityButton0, &MCU, &button0, BUTTON_DEBOUNCE_TIME_MS + 5);
    Simulate_Press(&entityButton0, &MCU, &button0, BUTTON_DEBOUNCE_TIME_MS + 5);

    ASSERT_EQ(result.button, &button0);
    ASSERT_EQ(result.event, BUTTON_EVENT_PRESS);
    ASSERT_EQ(result.counter, 1);
}

TEST(ReleaseTest, EventTest){
    Button_Init(&button0, &buttonUserFunctions);

    Simulate_Init(&entityButton0, &MCU, &button0, BUTTON_DEBOUNCE_TIME_MS + 5);
    Simulate_Press(&entityButton0, &MCU, &button0, BUTTON_DEBOUNCE_TIME_MS + 5);
    Simulate_Release(&entityButton0, &MCU, &button0, BUTTON_DEBOUNCE_TIME_MS + 5);

    ASSERT_EQ(result.button, &button0);
    ASSERT_EQ(result.event, BUTTON_EVENT_RELEASE);
    ASSERT_EQ(result.counter, 1);
}

TEST(ClickTest, EventTest)
{
    Button_Init(&button0, &buttonUserFunctions);

    Simulate_Init(&entityButton0, &MCU, &button0, BUTTON_DEBOUNCE_TIME_MS + 5);
    Simulate_Press(&entityButton0, &MCU, &button0, BUTTON_DEBOUNCE_TIME_MS + 5);
    Simulate_Release(&entityButton0, &MCU, &button0, BUTTON_EVENT_FINISH_TIMEOUT + 5);

    ASSERT_EQ(result.button, &button0);
    ASSERT_EQ(result.event, BUTTON_EVENT_CLICK);
    ASSERT_EQ(result.counter, 1);
}

TEST(HoldTest, EventTest)
{
    Button_Init(&button0, &buttonUserFunctions);

    Simulate_Init(&entityButton0, &MCU, &button0, BUTTON_DEBOUNCE_TIME_MS + 5);
    Simulate_Press(&entityButton0, &MCU, &button0, BUTTON_EVENT_FINISH_TIMEOUT + 5);

    ASSERT_EQ(result.button, &button0);
    ASSERT_EQ(result.event, BUTTON_EVENT_HOLD);
    ASSERT_EQ(result.counter, 1);
}

/*--------------------------------------------------------------------------------
                        Integration Tests
--------------------------------------------------------------------------------*/
TEST(MulticlickTest, IntegrationTest)
{
    Button_Init(&button0, &buttonUserFunctions);

    Simulate_Init(&entityButton0, &MCU, &button0, BUTTON_DEBOUNCE_TIME_MS + 5);

    uint8_t outLoopClicks = 0;
    uint8_t inLoopClicks = 0;

    for(outLoopClicks = 1; outLoopClicks < BUTTON_MAX_CLICK_COUNT; outLoopClicks++){
        for(inLoopClicks = 0; inLoopClicks < outLoopClicks; inLoopClicks++){
            Simulate_Press(&entityButton0, &MCU, &button0, BUTTON_DEBOUNCE_TIME_MS + 5);
            Simulate_Release(&entityButton0, &MCU, &button0, BUTTON_DEBOUNCE_TIME_MS + 5);
        }

        Simulate_Release(&entityButton0, &MCU, &button0, BUTTON_EVENT_FINISH_TIMEOUT + 5);

        ASSERT_EQ(result.button, &button0);
        ASSERT_EQ(result.event, BUTTON_EVENT_CLICK);
        ASSERT_EQ(result.counter, inLoopClicks);
    }
}

TEST(LongHoldTest, IntegrationTest)
{
    Button_Init(&button0, &buttonUserFunctions);

    Simulate_Init(&entityButton0, &MCU, &button0, BUTTON_DEBOUNCE_TIME_MS + 5);
    Simulate_Press(&entityButton0, &MCU, &button0, BUTTON_DEBOUNCE_TIME_MS + 5);

    uint8_t holdEvents = 0;
    for(holdEvents = 1; holdEvents < BUTTON_MAX_EVENTS_COUNT; holdEvents++){
        Simulate_Press(&entityButton0, &MCU, &button0, BUTTON_EVENT_FINISH_TIMEOUT + 1);

        ASSERT_EQ(result.button, &button0);
        ASSERT_EQ(result.event, BUTTON_EVENT_HOLD);
        ASSERT_EQ(result.counter, holdEvents);
    }
}

/*
TEST(DebounceTest, IntegrationTest)
{
    Button_Init(&button0, &buttonUserFunctions);

    Simulate_Init(&entityButton0, &MCU, &button0, BUTTON_DEBOUNCE_TIME_MS - 1);

    Simulate_Press(&entityButton0, &MCU, &button0, BUTTON_DEBOUNCE_TIME_MS + BUTTON_DEBOUNCE_TIME_MS - 1);
    Simulate_Release(&entityButton0, &MCU, &button0, BUTTON_DEBOUNCE_TIME_MS + BUTTON_DEBOUNCE_TIME_MS - 1);

    ASSERT_EQ(result.button, &button0);
    ASSERT_EQ(result.event, BUTTON_EVENT_NONE);
    ASSERT_EQ(result.counter, 0);
}
*/

TEST(HoldBeforeTurnOnTest, IntegrationTest)
{
    Button_Init(&button0, &buttonUserFunctions);

    Simulate_Press(&entityButton0, &MCU, &button0, 1);
    Simulate_Init(&entityButton0, &MCU, &button0, 1);

    Simulate_Press(&entityButton0, &MCU, &button0, BUTTON_EVENT_FINISH_TIMEOUT * 2);

    ASSERT_EQ(result.button, &button0);
    ASSERT_EQ(result.event, BUTTON_EVENT_HOLD);
    ASSERT_EQ(result.counter, 2);
}

/*--------------------------------------------------------------------------------
                        Boundary Tests
--------------------------------------------------------------------------------*/
TEST(MaxClicksTest, BoundaryTest)
{
    Button_Init(&button0, &buttonUserFunctions);

    Simulate_Init(&entityButton0, &MCU, &button0, BUTTON_DEBOUNCE_TIME_MS + 5);

    uint8_t clicks = 0;

    for(clicks = 0; clicks < BUTTON_MAX_CLICK_COUNT + 2; clicks++){ //15 clicks -> 16 owerflow == 0, 17 owerflow == 1
        Simulate_Press(&entityButton0, &MCU, &button0, BUTTON_DEBOUNCE_TIME_MS + 5);
        Simulate_Release(&entityButton0, &MCU, &button0, BUTTON_DEBOUNCE_TIME_MS + 5);
    }

    Simulate_Release(&entityButton0, &MCU, &button0, BUTTON_EVENT_FINISH_TIMEOUT + 5);

    ASSERT_EQ(result.button, &button0);
    ASSERT_EQ(result.event, BUTTON_EVENT_CLICK);
    ASSERT_EQ(result.counter, 1);
}



/*--------------------------------------------------------------------------------
                        Main
--------------------------------------------------------------------------------*/
int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}



/*--------------------------------------------------------------------------------
                        Helper functions definition
--------------------------------------------------------------------------------*/
void MCU_IncTick(EntityMCU_st* mcu, uint32_t ticks){
    mcu->tick += ticks;
}

void MCU_SetPinState(EntityMCU_st* mcu, uint32_t pin, uint8_t state){
    if(state){
        mcu->pinState |= (1 << pin);
    }
    else{
        mcu->pinState &= ~(1 << pin);
    }
}

uint32_t MCU_GetTick(EntityMCU_st* mcu){
    return mcu->tick;
}

uint8_t MCU_GetPinState(EntityMCU_st* mcu, uint32_t pin){
    return mcu->pinState & (1 << pin);
}


void Simulate_Init(EntityButton_st* entityButton, EntityMCU_st* mcu, Button_st* button, uint32_t duration){
    for(uint32_t i = 0; i < 50; i++){
        Button_Run(button);
        MCU_IncTick(mcu, 1);
    }
}

void Simulate_Press(EntityButton_st* entityButton, EntityMCU_st* mcu, Button_st* button, uint32_t duration){
    entityButton->state = BUTTON_STATE_PRESSED;
    if(entityButton->type == NORMAL_OPEN && entityButton->pull == PULL_UP){
        if(entityButton == &entityButton0){
            MCU_SetPinState(mcu, 0, 1);
        }
    }
    for(uint32_t i = 0; i < duration; i++){
        Button_Run(button);
        MCU_IncTick(mcu, 1);
    }
}

void Simulate_Release(EntityButton_st* entityButton, EntityMCU_st* mcu, Button_st* button, uint32_t duration){
    entityButton->state = BUTTON_STATE_RELEASED;
    if(entityButton->type == NORMAL_OPEN && entityButton->pull == PULL_UP){
        if(entityButton == &entityButton0){
            MCU_SetPinState(mcu, 0, 0);
        }
    }
    for(uint32_t i = 0; i < duration; i++){
        Button_Run(button);
        MCU_IncTick(mcu, 1);
    }
}