#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <EEPROM.h>
#include "RTClib.h"
//#include <LiquidCrystal.h>

#define debug

#include "Schedule.h"
using namespace tools;

#define b1 A0 //кнопка "влево/вниз"
#define b2 A1 //кнопка "вправо/вверх"
#define b3 A2 //кнопка "да/сохранить"
#define b4 A3 //кнопка "нет/выход"

#define bLeftOrDown   0
#define bRightOrUp    1
#define bOkOrSave     2
#define bNoOrExit     3

#define is_pressed LOW //Значение при которм кнопка считается нажатой
#define btn_count 4
byte btn_pin[btn_count] = { A0, A1, A2, A3 };
byte btn_prev[btn_count];
byte btn_state[btn_count];

#define relay A4
byte relay_state = LOW;

byte l_m = 0;       //Хранит текущий уровень отображения меню
byte l_sm_1 = 20; //Используется для настройки различных занятий
bool l_sm_2 = true;  //Используется для настроки одного занятия
byte sel_s = 0;  //Выбраное расписание, используется при настройке

//LiquidCrystal lcd(13, 12, 6, 5, 4, 3); //Pins used for RS,E,D4,D5,D6,D7
LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS1307 rtc;

#define number_of_type 2    //Количество различных типов расписаний
#define max_lessons 5       //Максимум занятий в расписании
Schedule s[number_of_type]; //Массив с расписаниями

void setup() {
  Wire.begin();
  rtc.begin();
  Serial.begin(9600);

  //Конфигурируем пины кнопок на вход
  pinMode(b1, INPUT_PULLUP);
  pinMode(b2, INPUT_PULLUP);
  pinMode(b3, INPUT_PULLUP);
  pinMode(b4, INPUT_PULLUP);

  //Конфигурируем реле
  pinMode(relay, OUTPUT);
  digitalWrite(relay, LOW);

  //Инициализируем дисплей
  lcd.begin();
  lcd.setCursor(0, 0);
  lcd.print("Engineers Garage");
  lcd.setCursor(0, 1);
  lcd.print("   TIME TABLE   ");

  delay(1000);

  //Инициализируем модуль RTC
  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");

    //Задаём RTC модулю дату и время компиляции скетча
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

    //Serial.print(rtc.now().second());
  }

  delay(1000);
}

DateTime prev;
int span = 100;
void loop() {
  btn_state_read();

  //return;

  if (l_m == 0) { //Включение устройства, предложение о настройке
    //Очищаем дисплей и предлагаем заполнить расписание
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(" ENTER ALL INFO ");
    lcd.setCursor(0, 1);
    lcd.print(" YES*       NO# ");

    if (btn_chk_state(bOkOrSave)) {
      l_m = 1;
    } else if (btn_chk_state(bNoOrExit)) {
      l_m = 2;
    }
  } 
  /**************************************************/
  else if (l_m == 1) {  //Мастер настройки расписания

    //Приступаем к настройке
    //Выбор типа расписания
    if (l_sm_1 == 20) {
      if (rtc.now().second() != prev.second()) {
        prev = rtc.now();
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Schedule type");
        lcd.setCursor(0, 1);
        lcd.print("#");
        lcd.print(sel_s);
      }

      //Изменяем значение sel_s
      if (btn_chk_state(bLeftOrDown)) {
        if (sel_s > 0) sel_s--;
      } else if (btn_chk_state(bRightOrUp)) {
        if (sel_s < number_of_type - 1) sel_s++;
      }

      //Принимаем изменения
      else if (btn_chk_state(bOkOrSave)) {
        l_sm_1 = 30;
      }
    }

    //Выбираем количество занятий в расписании
    else if (l_sm_1 == 30) {
      btn_state_read();

      if (rtc.now().second() != prev.second()) {
        prev = rtc.now();
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Schedule type #");
        lcd.print(sel_s);
        lcd.setCursor(0, 1);
        lcd.print("Lessons: ");
        lcd.print(s[sel_s].count);
      }

      //Изменяем значение s[sel_s].count
      //в диапазоне от 1 до max_lessons
      if (btn_chk_state(bLeftOrDown)) {
        if (s[sel_s].count > 1) s[sel_s].count--;
      } else if (btn_chk_state(bRightOrUp)) {
        if (s[sel_s].count < max_lessons) s[sel_s].count++;
      }

      //Принимаем изменения
      else if (btn_chk_state(bOkOrSave)) {

        //Применяем выбранное кол-во занятий
        s[sel_s].begin();

        //Переходим к настройке занятий
        l_sm_1 = 0;
      }
    }

    //Настройка занятий
    else if (l_sm_1 >= 0 && l_sm_1 < s[sel_s].count) {
      btn_state_read();

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Lesson #");
      lcd.print(l_sm_1 + 1);

      if (l_sm_2) {   //Настраиваем время начала занятия
        lcd.setCursor(0, 1);
        lcd.print("start: ");
        lcd.print(s[sel_s].schedule[l_sm_1].time_start.hour());
        lcd.print(":");
        lcd.print(s[sel_s].schedule[l_sm_1].time_start.minute());

        //Настраиваем часы начала занятия
        if (btn_chk_state(bLeftOrDown)) {       //кнопка bLeftOrDown изменяет часы
          s[sel_s].schedule[l_sm_1].time_start =+ new TimeSpan(0, 1, 0, 0);
        } else if (btn_chk_state(bRightOrUp)) { //кнопка bRightOrUp изменяет минуты
          s[sel_s].schedule[l_sm_1].time_start =+ new TimeSpan(0, 0, 1, 0);
        }

        //По нажатию ОК переходим к настроке времени конца занятия
        else if (btn_chk_state(bOkOrSave)) {
          l_sm_2 = false;
        }

        //По нажатию Назад
        else if (btn_chk_state(bNoOrExit)) {
          l_sm_2 = true;
          if (l_sm_1 != 0) l_sm_1--;
        }
      }

      else {   //Настраиваем время конца занятия
        lcd.setCursor(0, 1);
        lcd.print("end:   ");
        lcd.print(s[sel_s].schedule[l_sm_1].time_end.hour());
        lcd.print(":");
        lcd.print(s[sel_s].schedule[l_sm_1].time_end.minute());

        //Настраиваем часы начала занятия
        if (btn_chk_state(bLeftOrDown)) {       //кнопка bLeftOrDown изменяет часы
          s[sel_s].schedule[l_sm_1].time_end =+ new TimeSpan(0, 1, 0, 0);
        } else if (btn_chk_state(bRightOrUp)) { //кнопка bRightOrUp изменяет минуты
          s[sel_s].schedule[l_sm_1].time_end =+ new TimeSpan(0, 0, 1, 0);
        }

        //По нажатию ОК
        else if (btn_chk_state(bOkOrSave)) {
          l_sm_2 = true;
          l_sm_1++;
        }

        //По нажатию Назад
        else if (btn_chk_state(bNoOrExit)) {
          l_sm_2 = false;
          if (l_sm_1 != 0) l_sm_1--;
        }
      }
    }
  }

  while (l_m == 2) {
    btn_state_read();

    if (rtc.now().second() != prev.second()) {
      prev = rtc.now();
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(rtc.now().hour());
      lcd.print(":");
      lcd.print(rtc.now().minute());
      lcd.print(":");
      lcd.print(rtc.now().second());
    }
    if (btn_chk_state(bNoOrExit)) l_m = 1;
  }
}

void btn_state_read() {
  for (byte i = 0; i < btn_count; i++) {
    btn_prev[i] = btn_state[i];
    btn_state[i] = digitalRead(btn_pin[i]);
  }
  printInfo();
}


bool btn_chk_state(byte btn) {
  return (btn_state[btn] == !btn_prev[btn]) && (btn_state[btn] == is_pressed) ;
}

void printInfo() {
  Serial.print(l_m);
  Serial.print(" ");
  Serial.print(l_sm_1);
  Serial.print(" ");
  Serial.print(l_sm_2);
  Serial.print(" | ");
  Serial.print(btn_chk_state(0));
  Serial.print(" ");
  Serial.print(btn_chk_state(1));
  Serial.print(" ");
  Serial.print(btn_chk_state(2));
  Serial.print(" ");
  Serial.print(btn_chk_state(3));
  Serial.print("\n");
}
