#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

class Button {
  public:
    Button() {}

    Button(byte pin) {
      buttonPin = pin;
      pinMode(pin, INPUT_PULLUP);
    }

    void begin(byte pin) {
      buttonPin = pin;
      pinMode(pin, INPUT_PULLUP);
    }

    int getStatus() {
      return event;
    }

    int changeButtonStatus() {
      event = 0;

      // Текущее состояние кнопки
      int currentButtonClick = digitalRead(buttonPin) == isPressed ? 1 : 0;

      // Текущее время
      unsigned long timeButton = millis();

      switch (currentButtonStatus) {

        case 0:
          // В настоящий момент кнопка не нажималась

          if (currentButtonClick) {
            // Зафиксировали нажатие кнопки

            currentButtonStatus = 1;
            currentButtonStatusStart1 = millis();


          } else {
            // Кнопка не нажата
            // Ничего не происходит
          }
          break;

        case 1:
          // В настоящий момент кнопка на этапе первого нажатия

          if (currentButtonClick) {
            // Кнопка все еще нажата

            if (timeButton - currentButtonStatusStart1 >= delayLongSingleClick) {
              // Кнопка в нажатом состоянии уже дольше времени, после которого фиксируем длительное одинарное нажатие на кнопку

              // Событие длительного давления на кнопку - продолжаем давить
              event = 3;

            }

          } else {
            // Кнопку отжали обратно

            if (timeButton - currentButtonStatusStart1 < delayFalse) {
              // Время, которое была кнопка нажата, меньше минимального времени регистрации клика по кнопке
              // Скорее всего это были какие то флюктуации
              // Отменяем нажатие
              currentButtonStatus = 0;
              event = 0;

            } else if (timeButton - currentButtonStatusStart1 < delayLongSingleClick) {
              // Время, которое была кнопка нажата, меньше времени фиксации долгого нажатия на кнопку
              // Значит это первое одноразовое нажатие
              // Дальше будем ожидать второго нажатия
              currentButtonStatus = 2;
              currentButtonStatusStart2 = millis();
            } else {
              // Время, которое была нажата кнопка, больше времени фиксации долгого единоразового нажатия
              // Значит это завершение длительного нажатия
              currentButtonStatus = 0;
              event = 4;

            }

          }

          break;

        case 2:
          // Мы находимся в фазе отжатой кнопки в ожидании повторного ее нажатия для фиксации двойного нажатия
          // или, если не дождемся - значит зафиксируем единичное нажатие


          if (currentButtonClick) {
            // Если кнопку снова нажали

            // Проверяем, сколько времени кнопка находилась в отжатом состоянии
            if (timeButton - currentButtonStatusStart2 < delayFalse) {
              // Кнопка была в отжатом состоянии слишком мало времени
              // Скорее всего это была какая то флюктуация дребезга кнопки
              // Возвращаем обратно состояние на первичное нажатие кнопки
              currentButtonStatus = 1;

            } else {
              // Кнопка была достаточно долго отжата, чтобы зафиксировать начало второго нажатия
              // Фиксируем
              currentButtonStatus = 3;
              currentButtonStatusStart3 = millis();
            }

          } else {
            // Если кнопка все еще отжата

            // Проверяем, не достаточно ли она уже отжата, чтобы зафиксировать разовый клик
            if (timeButton - currentButtonStatusStart2 > delayDeltaDoubleClick) {
              // Кнопка в отжатом состоянии слишком долго
              // Фиксируем одинарный клие
              currentButtonStatus = 0;
              event = 1;
            }

          }

          break;

        case 3:
          // Мы на этапе второго нажатия
          // Для подтверждения факта двойного нажатия

          if (currentButtonClick) {
            // Кнопка все еще зажата
            // Ничего не происходит, ждем, когда отожмут

          } else {
            // Кнопку отжали

            // Проверям, действительно ли отжали, или это дребезг кнопки
            if (timeButton - currentButtonStatusStart3 < delayFalse) {
              // Кнопку отжали слишком рано
              // Скорре всего это дребезг
              // Гинорируем его

            } else {
              // Кнопка была в нажатом состоянии уже достаточно длительное время
              // Это завершение цикла фиксации двойного нажатия
              // Сообщаем такое событие
              event = 2;
              currentButtonStatus = 0;
            }
          }

          break;

      }

      return event;
    }

  private:
    int event = 0;
    int isPressed = LOW;

    int buttonPin = A0;                  // Пин с кнопкой

    int currentButtonStatus = 0;              // 0 - Кнопка не нажата
    // 1 - Кнопка нажата первый раз
    // 2 - Кнопка отжата после нажатия
    // 3 - Кнопка нажата во второй раз

    unsigned long currentButtonStatusStart1;  // Кол-во милисекунд от начала работы программы, когда начался статус 1
    unsigned long currentButtonStatusStart2;  // Кол-во милисекунд от начала работы программы, когда начался статус 2
    unsigned long currentButtonStatusStart3;  // Кол-во милисекунд от начала работы программы, когда начался статус 3



    const int delayFalse = 10;                // Длительность, меньше которой не регистрируется единоразовый клик
    const int delayLongSingleClick = 1000;    // Длительность зажатия кнопки для выхода в режим увеличения громкости
    const int delayDeltaDoubleClick = 800;    // Длительность между кликами, когда будет зафиксирован двойной клик
};

class Keypad {
  public:
    Button* buttons;

    Keypad(int* pins) {
      count = sizeof(pins) / sizeof(typeof(pins));
      buttons = new Button[count];
      for (int i = 0; i < count; i++)
      {
        (*(buttons + i)).begin(pins[i]);
      }
    }

    void checkButtonStatus() {
      for (int i = 0; i < count; i++)
      {
        (*(buttons + i)).changeButtonStatus();
      }
    }

    int getBtnStatus(int i) {
      return buttons[i].getStatus();
    }

  private:
    byte count = 1;
};
