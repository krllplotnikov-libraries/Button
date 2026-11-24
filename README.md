# Button library

## Содержание
1. [Описание](#Описание)
2. [Особенности](#Особенности)
3. [Типы данных](#Типы-данных)
4. [Использование](#Использование)
5. [Примеры](#Примеры)
6. [Запуск тестов](#Запуск-тестов)
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
- Корректная работа при переполнении счётчика тиков
- Не используются прерывания
- Не используются weak функции
____

## Типы данных
```C
//структура экземпляра кнопки
struct Button_st;

//дескриптор кнопки: аппаратные функции, callback и context
struct Button_Descriptor_st;

//перечисление состояний кнопки
enum Button_State_et;

//перечисление событий взаимодействия с кнопкой
enum Button_Event_et;

//указатель на функцию обратного вызова на событие взаимодействия с кнопкой
//Button_st* _button      - указатель на экземпляр кнопки
//Button_Event_et _event  - событие взаимодействия с кнопкой
//uint8_t _counter        - количество повторений события _event
typedef void (*Button_EventCallback)(Button_st* _button, Button_Event_et _event, uint8_t _counter);

//указатель на функцию для получения состояния кнопки
//Button_st* _button - указатель на экземпляр кнопки
typedef Button_State_et (*Button_GetState)(Button_st* _button);

//указатель на функцию для получения времени в миллисекундах с момента запуска
typedef uint32_t (*Button_GetTick)(void);
```

Поле `context` дескриптора — непрозрачный указатель на объект-подписчик. Библиотека хранит его, но не разыменовывает; обработчик события получает его через `_button->descriptor.context`.
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
4) Определить три функции: получение времени в миллисекундах с момента запуска; получение состояния кнопки; обратный вызов на событие взаимодействия с кнопкой
```C
static uint32_t getTick(void){
	//Необходимо вернуть значение времени в миллисекундах
	return HAL_GetTick();
}

static Button_State_et getButtonState(Button_st* _button){
	if(_button == &buttonLeft){
		//Необходимо вернуть текущее состояние кнопки, в зависимости от логического уровня на пине, к которому подключена кнопка
		return (Button_State_et)!HAL_GPIO_ReadPin(BUTTON_LEFT_GPIO_Port, BUTTON_LEFT_Pin);
	}

	return BUTTON_STATE_RELEASED;
}

static void buttonEvent(Button_st* _button, Button_Event_et _event, uint8_t _counter){
	switch(_event){
	case BUTTON_EVENT_CLICK:{
		if(_counter == 1){
			//Одиночный клик
		}
		else if(_counter == 2){
			//Двойной клик
		}
		break;
	}

	case BUTTON_EVENT_HOLD:{
		if(_counter == 4){
			//Удержание кнопки 1 секунду
		}
		else if(_counter == 10){
			//Удержание кнопки 2.5 секунды
		}
		break;
	}

	default: break;
	}
}
```
5) Создать дескриптор Button_Descriptor_st и присвоить ранее объявленные функции
```C
Button_Descriptor_st buttonDescriptor = {
	.getState      = getButtonState,
	.getTick       = getTick,
	.eventCallback = buttonEvent,
	.context       = NULL,
};
```
6) Вызвать функцию инициализации кнопки Button_Init(), аргументами передать указатель на экземпляр кнопки и указатель на дескриптор
```C
Button_Init(&buttonLeft, &buttonDescriptor);
```
7) В цикле вызывать функцию Button_Run(), аргументом передать указатель на экземпляр кнопки
```C
while(1){
	Button_Run(&buttonLeft);
}
```
8) В buttonEvent() обрабатывать полученные события

Если кнопка создана одним слоем (например, `Board`), а события обрабатывает другой (например, модуль), обработчик и `context` назначаются после инициализации:
```C
Button_SetEventCallback(&buttonLeft, buttonEvent, &moduleButton);
```

**ВАЖНО!**
- Событие BUTTON_EVENT_CLICK имеет ограничение в 15 нажатий подряд
- Событие BUTTON_EVENT_HOLD вызывается каждые 250 миллисекунд (BUTTON_EVENT_FINISH_TIMEOUT_MS). Для определения длительности удержания кнопки необходимо значение _counter умножить на 250
- Если eventCallback равен NULL, события не передаются
____

## Примеры
- STM32
```C
#include "Button.h"

static uint32_t        getTick(void);
static Button_State_et getButtonState(Button_st* _button);
static void            buttonEvent(Button_st* _button, Button_Event_et _event, uint8_t _counter);

Button_st buttonLeft  = {0};
Button_st buttonRight = {0};

Button_Descriptor_st buttonDescriptor = {
	.getState      = getButtonState,
	.getTick       = getTick,
	.eventCallback = buttonEvent,
	.context       = NULL,
};

static uint32_t getTick(void){
	return HAL_GetTick();
}

static Button_State_et getButtonState(Button_st* _button){
	if(_button == &buttonLeft){
		return (Button_State_et)!HAL_GPIO_ReadPin(BUTTON_LEFT_GPIO_Port, BUTTON_LEFT_Pin);
	}
	else if(_button == &buttonRight){
		return (Button_State_et)!HAL_GPIO_ReadPin(BUTTON_RIGHT_GPIO_Port, BUTTON_RIGHT_Pin);
	}

	return BUTTON_STATE_RELEASED;
}

static void buttonEvent(Button_st* _button, Button_Event_et _event, uint8_t _counter){
	if(_button == &buttonLeft){
		if(_event == BUTTON_EVENT_CLICK && _counter == 2){
			//Двойной клик левой кнопки
		}
	}
	else if(_button == &buttonRight){
		if(_event == BUTTON_EVENT_HOLD && _counter == 4){
			//Удержание правой кнопки 1 секунду
		}
	}
}

int main(void){
	Button_Init(&buttonLeft, &buttonDescriptor);
	Button_Init(&buttonRight, &buttonDescriptor);

	while(1){
		Button_Run(&buttonLeft);
		Button_Run(&buttonRight);
	}
}
```

- ESP32
```C
#include "driver/gpio.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "Button.h"

#define BUTTON_LEFT_GPIO_NUM  GPIO_NUM_9
#define BUTTON_RIGHT_GPIO_NUM GPIO_NUM_10

static uint32_t        getTick(void);
static Button_State_et getButtonState(Button_st* _button);
static void            buttonEvent(Button_st* _button, Button_Event_et _event, uint8_t _counter);
static void            buttonTask(void* _parameters);

gpio_config_t buttonGpioConfig = {
	.mode         = GPIO_MODE_INPUT,
	.pull_up_en   = GPIO_PULLUP_ENABLE,
	.pin_bit_mask = ((1ULL << BUTTON_LEFT_GPIO_NUM) | (1ULL << BUTTON_RIGHT_GPIO_NUM)),
};

Button_st buttonLeft  = {0};
Button_st buttonRight = {0};

Button_Descriptor_st buttonDescriptor = {
	.getState      = getButtonState,
	.getTick       = getTick,
	.eventCallback = buttonEvent,
	.context       = NULL,
};

void app_main(void){
	gpio_config(&buttonGpioConfig);

	Button_Init(&buttonLeft, &buttonDescriptor);
	Button_Init(&buttonRight, &buttonDescriptor);

	xTaskCreate(buttonTask, "button", 2048, NULL, 1, NULL);
}

static uint32_t getTick(void){
	return (uint32_t)(esp_timer_get_time() / 1000);
}

static Button_State_et getButtonState(Button_st* _button){
	if(_button == &buttonLeft){
		return (Button_State_et)!gpio_get_level(BUTTON_LEFT_GPIO_NUM);
	}
	else if(_button == &buttonRight){
		return (Button_State_et)!gpio_get_level(BUTTON_RIGHT_GPIO_NUM);
	}

	return BUTTON_STATE_RELEASED;
}

static void buttonEvent(Button_st* _button, Button_Event_et _event, uint8_t _counter){
	if(_button == &buttonLeft){
		if(_event == BUTTON_EVENT_CLICK && _counter == 1){
			//Одиночный клик левой кнопки
		}
	}
	else if(_button == &buttonRight){
		if(_event == BUTTON_EVENT_HOLD && _counter == 10){
			//Удержание правой кнопки 2.5 секунды
		}
	}
}

static void buttonTask(void* _parameters){
	while(1){
		Button_Run(&buttonLeft);
		Button_Run(&buttonRight);
		vTaskDelay(pdMS_TO_TICKS(1));
	}
}
```
____

## Запуск тестов

```sh
# Если не установлен Docker, установите его:
sudo apt update && sudo apt install docker.io

# Создайте образ и запустите контейнер с тестами:
docker build -t button_test .
docker run --rm button_test
```
