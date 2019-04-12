#include "button_coin.h"

//
// конструктор - вызывается всегда при создании экземпляра класса RoboCraft
//

    // Конструктор класса
    button_coin::button_coin (int a, boolean b, int c) {
      pin = a;
      reg = b;
      wait = c;
      cur = 0;
      prev = 0;
      trigger_push = 0;
      trigger_let = 0;
      coint = 0;
      con = 0;
      discon = 0;
      pinMode(pin, INPUT_PULLUP);
    }
    
    // Нажатие кнопки
    bool button_coin::but () {
      knopka(); // антидребезг
      coin (); // счетчик нажатий
      triger(); // триггер
      connect_but (); // нажатие кнопки
      discon_but (); // отпускание кнопки
      return cur; // вернуть значение
    }

// Антидребезг
    bool button_coin::knopka() {
    static bool prew; // предыдущее состояние кнопки !!только для антидребезга!!
    static unsigned long prewMillis; // задержка для гашения дребезга

      prev = cur;
      if (digitalRead(pin) == LOW) { // если кнопка нажата
        if (!prew) { // если предыдущее значение кнопки "не нажата"
          prewMillis = millis(); // засечь время
          prew = true; // установить предыдущее значение кнопки в "нажата"
        }
        if (millis() - prewMillis > wait) { // если вышло время ожидания окончания дребезга
          cur = 1; // установить значение кнопки "нажата"
        }
      }
      else { // если кнопка не нажата
        if (millis() - prewMillis > wait) { // и если вышло время ожидания окончания дребезга
          cur = false; // сброс значения текущего состояния кнопки
          prew = false; // сброс значения предыдущего состояния кнопки
        }
      }
      return cur;
    };

    // Триггер
    void button_coin::triger() {    
      if (cur && !prev) // если кнопка нажата
        trigger_push = !trigger_push;
  
      if (!cur && prev) // если кнопка отжата
        trigger_let = !trigger_let;
    }
  
    // Счетчик нажатий
    int button_coin::coin () {
      if (cur && !prev)
        coint++;
    }
    
    // Нажатие кнопки
    void button_coin::connect_but () {
      if (cur && !prev) con = 1;
      else con = 0;
    }
    
    // Отпускание кнопки
    void button_coin::discon_but () {
      if (!cur && prev) discon = 1;
      else discon = 0;
    }
