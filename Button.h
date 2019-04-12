#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

class Button {
  public:
    Button(byte pin, byte wait) {
      this->pin  = pin;
      this->wait = wait;
      current   = false;
      previous  = false;
      pressed   = false;

      pinMode(pin, INPUT_PULLUP);
    }

    void check() {
      static unsigned long prewMillis; // задержка для гашения дребезга
      previous = current;
      if (digitalRead(pin) == LOW) { // если кнопка нажата
        if (!previous) { // если предыдущее значение кнопки "не нажата"
          prewMillis = millis(); // засечь время
          current = true; // установить предыдущее значение кнопки в "нажата"
        }
      }
      else { // если кнопка не нажата
        if (millis() - prewMillis > wait) { // и если вышло время ожидания окончания дребезга
          current = false; // сброс значения текущего состояния кнопки
        }
      }
    }

    bool isPressed() {
      return current && !previous;
    }

    bool currentState() {
      return current;
    }

  private:
    byte pin;
    byte wait;
    bool current;
    bool previous;
    bool pressed;
};
/*
  Button::Button(byte pin, byte wait) {
  this->pin  = pin;
  this->wait = wait;
  current   = false;
  previous  = false;
  pressed   = false;

  pinMode(pin, INPUT_PULLUP);
  }

  void Button::check() {
  static unsigned long prewMillis; // задержка для гашения дребезга
  previous = current;
  if (digitalRead(pin) == LOW) { // если кнопка нажата
    if (!previous) { // если предыдущее значение кнопки "не нажата"
      prewMillis = millis(); // засечь время
      current = true; // установить предыдущее значение кнопки в "нажата"
    }
  }
  else { // если кнопка не нажата
    if (millis() - prewMillis > wait) { // и если вышло время ожидания окончания дребезга
      current = false; // сброс значения текущего состояния кнопки
    }
  }
  }

  bool Button::isPressed() {
  return current && !previous;
  }

  bool Button::currentState() {
  return current;
  }
*/
