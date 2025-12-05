#include <gtest/gtest.h>
#include <iostream>

#include "Button.h"


uint32_t tick = 0;
ButtonState_et pinState = BUTTON_STATE_RELEASED;
ButtonEvent_et buttonEvent = BUTTON_EVENT_NONE;

void IncTick(uint32_t ticks){
    tick += ticks;
}

uint32_t GetTick(){
    return tick;
}

void SetPinState(ButtonState_et state){
    pinState = state;
}

ButtonState_et GetPinState(){
    return pinState;
}

void Button_EventCallback(Button_st* button, ButtonEvent_et event, uint8_t counter);
ButtonState_et Button_GetState(Button_st* button);
uint32_t Button_GetTick();

Button_st button0 = {0};
ButtonUserFunctions_st button0UserFunctions = {
    .getState = Button_GetState,
    .getTick = Button_GetTick,    
    .eventCallback = Button_EventCallback
};

void Button_EventCallback(Button_st* button, ButtonEvent_et event, uint8_t counter){
    if(button == &button0){
        if(event == BUTTON_EVENT_PRESS){
            buttonEvent = BUTTON_EVENT_PRESS;
        }

        if(event == BUTTON_EVENT_RELEASE){
            buttonEvent = BUTTON_EVENT_RELEASE;
        }  

        if(event == BUTTON_EVENT_CLICK){
            buttonEvent = BUTTON_EVENT_CLICK;
        }

        if(event == BUTTON_EVENT_HOLD){
            buttonEvent = BUTTON_EVENT_HOLD;
        }

        if(event == BUTTON_EVENT_NONE){
            buttonEvent = BUTTON_EVENT_NONE;
        }        
    }
}

ButtonState_et Button_GetState(Button_st* button){
    if(button == &button0){
        return pinState;
    }
    return BUTTON_STATE_RELEASED;
}

uint32_t Button_GetTick(){
    return tick;
}

TEST(ClickTest, BasicTest)
{
    SetPinState(BUTTON_STATE_RELEASED);

    Button_Init(&button0, &button0UserFunctions);
    Button_Run(&button0);

    for(uint32_t i = 0; i < 10; i++){
        IncTick(1);
        Button_Run(&button0);
        ASSERT_EQ(buttonEvent, BUTTON_EVENT_NONE);
    }

    SetPinState(BUTTON_STATE_PRESSED);

    for(uint32_t i = 0; i <= BUTTON_DEBOUNCE_TIME_MS; i++){
        IncTick(1);
        Button_Run(&button0);
        ASSERT_EQ(buttonEvent, BUTTON_EVENT_NONE);
    }   

    IncTick(1);
    Button_Run(&button0);

    ASSERT_EQ(buttonEvent, BUTTON_EVENT_PRESS);

    for(uint32_t i = 0; i < 50; i++){
        IncTick(1);
        Button_Run(&button0);
        ASSERT_EQ(buttonEvent, BUTTON_EVENT_PRESS);
    }       

    SetPinState(BUTTON_STATE_RELEASED);

    for(uint32_t i = 0; i <= BUTTON_DEBOUNCE_TIME_MS; i++){
        IncTick(1);
        Button_Run(&button0);
        ASSERT_EQ(buttonEvent, BUTTON_EVENT_PRESS);
    }   

    IncTick(1);
    Button_Run(&button0);

    ASSERT_EQ(buttonEvent, BUTTON_EVENT_RELEASE);

    for(uint32_t i = 0; i < BUTTON_EVENT_FINISH_TIMEOUT - BUTTON_DEBOUNCE_TIME_MS - 1; i++){
        IncTick(1);
        Button_Run(&button0);
        ASSERT_EQ(buttonEvent, BUTTON_EVENT_RELEASE);
    }   

    IncTick(1);
    Button_Run(&button0);

    ASSERT_EQ(buttonEvent, BUTTON_EVENT_CLICK);

    buttonEvent = BUTTON_EVENT_NONE;
    for(uint32_t i = 0; i < 500; i++){
        IncTick(1);
        Button_Run(&button0);
        ASSERT_EQ(buttonEvent, BUTTON_EVENT_NONE);
    }       
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
