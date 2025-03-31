# Button library

## Содержание
1. [Описание](#Описание)
2. [Особенности](#Особенности)
3. [Типы данных](#Типы-данных)
4. [Использование](#Использование)
5. [Примеры](#Примеры)
____

## Описание
Библиотека для обработки событий нажатия и удержания кнопок
____

## Особенности
- Аппаратно независимая реализация
- Программный антидребезг
- Работа с любым типом подтяжки
- Работа с любым типом кнопки
- Обработка событий через callback функцию
- Обработка произвольного количества нажатий
- Обработка удержания любой длительности
- Не используются прерывания
____

## Типы данных
```C
//структура для создания экземпляра кнопки
struct Button_st;

//структура для определения пользовательских функций
struct ButtonUserFunctions_st;

//перечесление состояний кнопки
enum ButtonState_et; 

//перечесление событий взаимодейсвия с кнопкой
enum ButtonEvent_et; 

//указатель на функцию обратного вызова на событие взаимодейсвия с кнопкой
//Button_st* button - указатель на экземпляра структуры кнопки
//ButtonEvent_et event - событие взаимодейсвия с кнопкой
//uint8_t counter - количество повторений события event
void (*ButtonEventCallback) (Button_st* button, ButtonEvent_et event, uint8_t counter); 

//указатель на функцию для получения состояния кнопки
//Button_st* button - указатель на экземпляра структуры кнопки
ButtonState_et (*ButtonGetState) (Button_st* button); 

//указатель на функцию для получения времени в миллисекундах с момента запуска
uint32_t (*ButtonGetTick) (); 
```
____

## Использование
1) Добавить файлы Button.h и Button.c в проект
2) Подключить файл Button.h
```C
#include "Button.h"
```
3) Создать экземпляр структуры Button_st
```C
Button_st buttonLeft = {0};
```
4) Определить три функции: получение времени в миллисекундах с момента запуска; получение состояния кнопки; обратный вызов на событие взаимодейсвия с кнопкой
```C
uint32_t GetTick_Button(){
	//Необходимо вернуть значение времени в миллисекундах
	return HAL_GetTick();
}

ButtonState_et GetState_Button(Button_st* button){
	if(button == &buttonLeft){
		//Необходимо вернуть текущее состояния кнопки, в зависимости от логического уровня на пине к которому подключена кнопка
		return (ButtonState_et)!HAL_GPIO_ReadPin(BUTTON_LEFT_GPIO_Port, BUTTON_LEFT_Pin);
	}

	return BUTTON_STATE_RELEASED;
}

void EventCallback_Button(Button_st* button, ButtonEvent_et event, uint8_t counter){
	if(button == &buttonLeft){
		if(event == BUTTON_EVENT_CLICK){
			if(counter == 1){
				//Одиночный клик
			}
			if(counter == 2){
				//Двойной клик
			}
		}
		if(event == BUTTON_EVENT_HOLD){
			if(counter == 4){
				//удержание кнопки 1 секунду
			}
			if(counter == 10){
				//удержание кнопки 2.5 секунды
			}
		}
	}
}
```
5) Создать экземпляр структуры ButtonUserFunctions_st и присвоить ранее объявленные функции
```C
ButtonUserFunctions_st buttonUserFunctions = {
		.getState = GetState_Button,
		.getTick = GetTick_Button,
		.eventCallback = EventCallback_Button
};
```
6) Вызвать функцию инициализации кнопки Button_Init(), аргументами передать ссылку на экземпляр структуры кнопки и ссылку на структуру с указателями на функции
```C
void main(){
	Button_Init(&buttonLeft, &buttonUserFunctions);
}
```
7) В цикле вызывать функцию Button_Run(), аргументами передать ссылку на экземпляр структуры кнопки
```C
while(1){
	Button_Run(&buttonLeft);
}
```
8) В EventCallback_Button() обрабатывать полученные события

ВАЖНО! 

Событие BUTTON_EVENT_CLICK имеет ограничение в 15 нажатий подряд

Событие BUTTON_EVENT_HOLD вызывается каждые 250 миллисекунд. Т.е. для определения длительности удержания кнопки необходимо значение counter умножить на 250
____

## Примеры
- STM32
```C
#include "Button.h"

ButtonState_et GetState_Button(Button_st* button);
uint32_t GetTick_Button();
void EventCallback_Button(Button_st* button, ButtonEvent_et event, uint8_t counter);

Button_st buttonLeft = {0};
Button_st buttonRight = {0};

ButtonUserFunctions_st buttonUserFunctions = {
		.getState = GetState_Button,
		.getTick = GetTick_Button,
		.eventCallback = EventCallback_Button
};

ButtonState_et GetState_Button(Button_st* button){
	if(button == &buttonLeft){
		return (ButtonState_et)!HAL_GPIO_ReadPin(BUTTON_LEFT_GPIO_Port, BUTTON_LEFT_Pin);
	}

	else if(button == &buttonRight){
		return (ButtonState_et)!HAL_GPIO_ReadPin(BUTTON_RIGHT_GPIO_Port, BUTTON_RIGHT_Pin);
	}

	return BUTTON_STATE_RELEASED;
}

uint32_t GetTick_Button(){
	return HAL_GetTick();
}

void EventCallback_Button(Button_st* button, ButtonEvent_et event, uint8_t counter){
	if(button == &buttonLeft){
		if(event == BUTTON_EVENT_CLICK){
			if(counter == 1){
				//Одиночный клик
			}
			if(counter == 2){
				//Двойной клик
			}
		}
		if(event == BUTTON_EVENT_HOLD){
			if(counter == 4){
				//удержание кнопки 1 секунду
			}
			if(counter == 10){
				//удержание кнопки 2.5 секунды
			}
		}
	}

	if(button == &buttonRight){
		if(event == BUTTON_EVENT_CLICK){
			if(counter == 1){
				//Одиночный клик
			}
			if(counter == 2){
				//Двойной клик
			}
		}
		if(event == BUTTON_EVENT_HOLD){
			if(counter == 4){
				//удержание кнопки 1 секунду
			}
			if(counter == 10){
				//удержание кнопки 2.5 секунды
			}
		}
	}
}

void main(){
	Button_Init(&buttonLeft, &buttonUserFunctions);
	Button_Init(&buttonRight, &buttonUserFunctions);

	while(1){
		Button_Run(&buttonLeft);
		Button_Run(&buttonRight);
	}
}
```
____