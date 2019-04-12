#ifndef button_coin_h
#define button_coin_h

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif 

class button_coin {
public:
    int pin; // номер пина кнопки, задается при инициализации
    bool reg; // режим работы кнопки, задается при инициализации
                  // -0- если нормально разомкнутая кнопка соеденена с минусом
                  // -1- если нормально разомкнутая кнопка соеденена с плюсом через резистор 1К
    int wait; // время задержки антидребезга
    bool cur; // текущее состояние кнопки
    bool prev; // предыдущее состояние кнопки
    bool trigger_push; // триггер на нажатие кнопки
    bool trigger_let; // триггер на отпускание кнопки
    int coint; // счетчик нажатий
    bool con; // момент нажатия кнопки
    bool discon; // момент отпускания кнопки
  
    // Конструктор класса
    button_coin (int a, boolean b, int c);
    
    // Нажатие кнопки
    bool but ();

private:
	bool knopka();

	void triger();

	int coin ();

	void connect_but ();

	void discon_but ();
};

#endif

