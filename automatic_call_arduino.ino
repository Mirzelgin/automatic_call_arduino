#include <RTClib.h>

#include "LiquidCrystal_I2C.h"
#include <Wire.h>
#include <EEPROM.h>
//#include <LiquidCrystal.h>

#include "Button.h"
#include "Schedule.h"
using namespace tools;

Button bLeftOrDown(A0); //кнопка "влево/вниз"
Button bRightOrUp(A1);  //кнопка "вправо/вверх"
Button bOkOrSave(A2);   //кнопка "да/сохранить"
Button bNoOrExit(A3);   //кнопка "нет/выход"

#define relay A4
byte relay_state = LOW;

byte l_m = 0;       //Хранит текущий уровень отображения меню
byte l_sm_1 = 30; //Используется для настройки различных занятий
bool l_sm_2 = true;  //Используется для настроки одного занятия
byte sel_s = 0;  //Выбраное расписание, используется при настройке

//LiquidCrystal lcd(13, 12, 6, 5, 4, 3); //Pins used for RS,E,D4,D5,D6,D7
LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS1307 rtc;

#define number_of_type 2    //Количество различных типов расписаний
#define max_lessons 5       //Максимум занятий в расписании
Schedule s[number_of_type]; //Массив с расписаниями

void setup() {
  Serial.begin(9600);
  rtc.begin();

  //Serial.println("Инициализация часов реального времени...");
  Serial.println(rtc.isrunning());

  //Конфигурируем реле
  pinMode(relay, OUTPUT);
  digitalWrite(relay, LOW);

  //Инициализируем дисплей
  lcd.begin(16, 2);
  lcd.backlight();
  lcd.setCursor(0, 0);

  //Инициализируем модуль RTC
  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");

    //Задаём RTC модулю дату и время компиляции скетча
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

    //Serial.print(now.second());
  }
}

DateTime prev;
long prevMillis = millis();
int span = 500;
void loop() {
  DateTime now(rtc.now() + TimeSpan(0, 0, 0, 6));

  Serial.print(l_m);
  Serial.print(" ");
  Serial.print(l_sm_1);
  Serial.print(" ");
  Serial.println(l_sm_2);

  switch (l_m) {
  case 0: //Включение устройства, ожидание ответа на предложение о настройке
    if (millis() - prevMillis > span) {
        prevMillis = millis();
        //Очищаем дисплей и предлагаем заполнить расписание
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(" ENTER ALL INFO ");
        lcd.setCursor(0, 1);
        lcd.print(" YES*       NO# ");
      }
      if (bOkOrSave.changeButtonStatus() == 1) {
        l_m = 1;
      } else if (bNoOrExit.changeButtonStatus() == 1) {
        l_m = 2;
      }
      break;

    case 1:
      //Мастер настройки расписания

      /**************************************************/
      /******Выбираем количество занятий в расписании****/
      /**************************************************/
      if (l_sm_1 == 30) {
        if (millis() - prevMillis > span) {
          prevMillis = millis();
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
        if (bLeftOrDown.changeButtonStatus() == 1) {
          if (s[sel_s].count > 1) s[sel_s].count--;
        } else if (bRightOrUp.changeButtonStatus() == 1) {
          if (s[sel_s].count < max_lessons) s[sel_s].count++;
        }

        //Принимаем изменения
        else if (bOkOrSave.changeButtonStatus() == 1) {

          //Применяем выбранное кол-во занятий
          s[sel_s].begin();

          //Переходим к настройке занятий
          l_sm_1 = 0;
        }
      }

      /**************************************************/
      /************Настройка времени занятий*************/
      /**************************************************/
      else if (l_sm_1 >= 0 && l_sm_1 < s[sel_s].count) {
        if (millis() - prevMillis > span) {
          prevMillis = millis();
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Lesson #");
          lcd.print(l_sm_1 + 1);
        }

        if (l_sm_2) {   //Настраиваем время начала занятия
          lcd.setCursor(0, 1);
          lcd.print("start: ");
          lcd.print(s[sel_s].schedule[l_sm_1].time_start.hour());
          lcd.print(":");
          lcd.print(s[sel_s].schedule[l_sm_1].time_start.minute());

          //Настраиваем часы начала занятия
          if (bLeftOrDown.changeButtonStatus() == 1) {       //кнопка bLeftOrDown изменяет часы
            s[sel_s].schedule[l_sm_1].time_start = s[sel_s].schedule[l_sm_1].time_start + TimeSpan(0, 1, 0, 0);
            Serial.println(s[sel_s].schedule[l_sm_1].time_start.second());
          } else if (bRightOrUp.changeButtonStatus() == 1) { //кнопка bRightOrUp изменяет минуты
            s[sel_s].schedule[l_sm_1].time_start = s[sel_s].schedule[l_sm_1].time_start + TimeSpan(0, 0, 1, 0);
          }

          //По нажатию ОК переходим к настроке времени конца занятия
          else if (bOkOrSave.changeButtonStatus() == 1) {
            l_sm_2 = false;
          }

          //По нажатию Назад
          else if (bNoOrExit.changeButtonStatus() == 1) {
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
          if (bLeftOrDown.changeButtonStatus() == 1) {       //кнопка bLeftOrDown изменяет часы
            s[sel_s].schedule[l_sm_1].time_end = s[sel_s].schedule[l_sm_1].time_end + TimeSpan(0, 1, 0, 0);
          } else if (bRightOrUp.changeButtonStatus() == 1) { //кнопка bRightOrUp изменяет минуты
            s[sel_s].schedule[l_sm_1].time_end = s[sel_s].schedule[l_sm_1].time_end + TimeSpan(0, 0, 1, 0);
          }

          //По нажатию ОК
          if (bOkOrSave.changeButtonStatus() == 1) {
            if (l_sm_1 < s[sel_s].count - 1) l_sm_1++;
            else l_sm_1 = 40;
            l_sm_2 = true;
          }
        }
      }

      /**************************************************/
      /************Закончить настройку или***************/
      /**********настроить другое расписание*************/
      /**************************************************/
      else if (l_sm_1 == 40) {
        if (sel_s < number_of_type - 1) {
          sel_s++;
          l_sm_1 = 30;
        }
        else {
          l_m = 2;
        }
      }
      break;

    case 2:
      if (now.second() != prev.second()) {
        prev = now;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(now.hour());
        lcd.print(":");
        lcd.print(now.minute());
        lcd.print(":");
        lcd.print(now.second());

        /*
              Serial.print(now.year(), DEC);
              Serial.print('/');
              Serial.print(now.month(), DEC);
              Serial.print('/');
              Serial.print(now.day(), DEC);
              Serial.print(" (");
              Serial.print(") ");
              Serial.print(now.hour(), DEC);
              Serial.print(':');
              Serial.print(now.minute(), DEC);
              Serial.print(':');
              Serial.print(now.second(), DEC);
              Serial.println();
        */
      }

      if (bNoOrExit.changeButtonStatus() == 3) l_m = 0;
      break;
  }
}
