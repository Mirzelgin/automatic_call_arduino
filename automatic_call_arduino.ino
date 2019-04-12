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

byte level_menu = 0;       //Хранит текущий уровень отображения меню
byte level_submenu_1 = 20; //Используется для настройки различных занятий
byte level_submenu_2 = 0;  //Используется для настроки одного занятия
byte select_schedule = 0;  //Выбраное расписание, используется при настройке

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

  while (level_menu == 0) {
    btn_state_read();

    if (rtc.now().second() != prev.second()) {
      prev = rtc.now();
      //Очищаем дисплей и предлагаем заполнить расписание
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(" ENTER ALL INFO ");
      lcd.setCursor(0, 1);
      lcd.print(" YES*       NO# ");
    }

    if (btn_chk_state(bOkOrSave)) {
      level_menu = 1;
    } else if (btn_chk_state(bNoOrExit)) {
      level_menu = 2;
    }
  }


  //Мастер настройки расписания
  while (level_menu == 1) {
    btn_state_read();

    //Приступаем к настройке
    //Выбор типа расписания
    while (level_submenu_1 == 20) {
      btn_state_read();

      if (rtc.now().second() != prev.second()) {
        prev = rtc.now();
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Schedule type");
        lcd.setCursor(0, 1);
        lcd.print("#");
        lcd.print(select_schedule);
      }

      //Изменяем значение select_schedule
      if (btn_chk_state(bLeftOrDown)) {
        if (select_schedule > 0) select_schedule--;
      } else if (btn_chk_state(bRightOrUp)) {
        if (select_schedule < number_of_type - 1) select_schedule++;
      }

      //Принимаем изменения
      else if (btn_chk_state(bOkOrSave)) {
        level_submenu_1 = 30;
      }
    }


    //Выбираем количество занятий в расписании
    while (level_submenu_1 == 30) {

      btn_state_read();

      if (rtc.now().second() != prev.second()) {
        prev = rtc.now();
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Schedule type #");
        lcd.print(select_schedule);
        lcd.setCursor(0, 1);
        lcd.print("Lessons: ");
        lcd.print(s[select_schedule].count);
      }

      //Изменяем значение s[select_schedule].count
      //в диапазоне от 1 до max_lessons
      if (btn_chk_state(bLeftOrDown)) {
        if (s[select_schedule].count > 1) s[select_schedule].count--;
      } else if (btn_chk_state(bRightOrUp)) {
        if (s[select_schedule].count < max_lessons) s[select_schedule].count++;
      }

      //Принимаем изменения
      else if (btn_chk_state(bOkOrSave)) {

        //Применяем выбранное кол-во занятий
        s[select_schedule].begin();

        //Переходим к настройке занятий
        level_submenu_1 = 0;
      }
    }

    //Настройка занятий
    while (level_submenu_1 >= 0 && level_submenu_1 < s[select_schedule].count) {
      btn_state_read();

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Lesson #");
      lcd.print(level_submenu_1 + 1);

      //Настраиваем время начала и конца выбранного занятия
      while (level_submenu_2 == 0) {
        btn_state_read();

        lcd.setCursor(0, 1);
        lcd.print(s[select_schedule].schedule[level_submenu_1].time_start.hh);
        lcd.print(" hours start");

        //Настраиваем часы начала занятия
        if (btn_chk_state(bLeftOrDown)) {
          time_edit::subtract_hour(&(s[select_schedule].schedule[level_submenu_1].time_start));
        } else if (btn_chk_state(bRightOrUp)) {
          time_edit::add_hour(&(s[select_schedule].schedule[level_submenu_1].time_start));
        }

        //По нажатию ОК
        else if (btn_chk_state(bOkOrSave)) {
          if (level_submenu_2 < 3)
            level_submenu_2++;
          else if (level_submenu_1 < s[select_schedule].count) {
            level_submenu_1++;
          }
        }
      }

      while (level_submenu_2 == 1) {
        btn_state_read();

        lcd.setCursor(0, 1);
        lcd.print(s[select_schedule].schedule[level_submenu_1].time_start.mm);
        lcd.print(" min start");

        //Настраиваем минуты начала занятия
        if (btn_chk_state(bLeftOrDown)) {
          time_edit::subtract_minutes(&(s[select_schedule].schedule[level_submenu_1].time_start));
        } else if (btn_chk_state(bRightOrUp)) {
          time_edit::add_minutes(&(s[select_schedule].schedule[level_submenu_1].time_start));
        }
      }

      while (level_submenu_2 == 2) {
        btn_state_read();

        lcd.setCursor(0, 1);
        lcd.print(s[select_schedule].schedule[level_submenu_1].time_end.hh);
        lcd.print(" hours end");

        //Настраиваем часы конца занятия
        if (btn_chk_state(bLeftOrDown)) {
          time_edit::subtract_hour(&(s[select_schedule].schedule[level_submenu_1].time_end));
        } else if (btn_chk_state(bRightOrUp)) {
          time_edit::add_hour(&(s[select_schedule].schedule[level_submenu_1].time_end));
        }
      }

      while (level_submenu_2 == 3) {
        btn_state_read();

        lcd.setCursor(0, 1);
        lcd.print(s[select_schedule].schedule[level_submenu_1].time_end.mm);
        lcd.print(" min end");

        //Настраиваем минуты конца занятия
        if (btn_chk_state(bLeftOrDown)) {
          time_edit::subtract_minutes(&(s[select_schedule].schedule[level_submenu_1].time_end));
        } else if (btn_chk_state(bRightOrUp)) {
          time_edit::add_minutes(&(s[select_schedule].schedule[level_submenu_1].time_end));
        }
      }

      //Принимаем изменения
      if (btn_chk_state(bOkOrSave)) {
        if (level_submenu_2 < 3) level_submenu_2++;
        else if (level_submenu_1 < s[select_schedule].count - 1);
      }
    }
  }

  while (level_menu == 2) {
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
    if (btn_chk_state(bNoOrExit)) level_menu = 1;
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
  if ((btn_state[btn] == !btn_prev[btn]) && btn_state[btn] == is_pressed) {
    return true;
  }
  else return false;
}

void printInfo() {
  Serial.print(level_menu);
  Serial.print(" ");
  Serial.print(level_submenu_1);
  Serial.print(" ");
  Serial.print(level_submenu_2);
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
