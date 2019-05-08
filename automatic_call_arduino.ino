#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <EEPROM.h>
//#include <LiquidCrystal.h>

#include "Button.h"
#include "Schedule.h"

Button bLeftOrDown(A0); //кнопка "влево/вниз"
Button bRightOrUp(A1);  //кнопка "вправо/вверх"
Button bOkOrSave(A2);   //кнопка "да/сохранить"
Button bNoOrExit(A3);   //кнопка "нет/выход"
unsigned int g = 0;
const int p = 250;

#define relay A4
bool relayState = LOW;
const int timeIsOn = 5000;
long timeOn = 0;

byte l_m = 0;       //Хранит текущий уровень отображения меню
byte l_sm_1 = 30; //Используется для выбора различных переменных меню
bool l_sm_2 = true;  //Используется для настроки одного занятия
byte sel_s = 0;  //Выбраное расписание

//LiquidCrystal lcd(13, 12, 6, 5, 4, 3); //Pins used for RS,E,D4,D5,D6,D7
LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS1307 rtc;

#define number_of_type 2    //Количество различных типов расписаний
#define max_lessons 5       //Максимум занятий в расписании
Schedule s[number_of_type]; //Массив с расписаниями

String days_of_the_week[6] = { "SAT", "MON", "TUE", "WED", "THU", "FRI" };

void setup() {
  Serial.begin(9600);
  rtc.begin();

  //Конфигурируем реле
  pinMode(relay, OUTPUT);
  digitalWrite(relay, LOW);

  //Инициализируем дисплей
  lcd.begin(16, 2);
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.createChar(1, bell);

  //Инициализируем модуль RTC
  if (!rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
  }

  //Задаём RTC модулю дату и время компиляции скетча
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  rtc.adjust(DateTime(2000, 1, 1));
}

DateTime prev;
long prevMillis = millis();
int span = 500;
void loop() {
  DateTime now(rtc.now() + TimeSpan(0, 0, 0, 6));

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
        for (int i = 0; i < number_of_type; i++) {
          EEPROM.get(i * sizeof(s[i]) + 1, s[i]);
        }
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
          //кнопка bLeftOrDown изменяет часы
          if (bLeftOrDown.changeButtonStatus() == 1 || (bLeftOrDown.changeButtonStatus() == 3 && millis() - g > p)) {
            g = millis();
            s[sel_s].schedule[l_sm_1].time_start = s[sel_s].schedule[l_sm_1].time_start + TimeSpan(0, 1, 0, 0);
          }

          //кнопка bRightOrUp изменяет минуты
          if (bRightOrUp.changeButtonStatus() == 1 || (bRightOrUp.changeButtonStatus() == 3 && millis() - g > p)) {
            g = millis();
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
          //кнопка bLeftOrDown изменяет часы
          if (bLeftOrDown.changeButtonStatus() == 1 || (bLeftOrDown.changeButtonStatus() == 3 && millis() - g > p)) {
            g = millis();
            s[sel_s].schedule[l_sm_1].time_end = s[sel_s].schedule[l_sm_1].time_end + TimeSpan(0, 1, 0, 0);
          }

          //кнопка bRightOrUp изменяет минуты
          if (bRightOrUp.changeButtonStatus() == 1 || (bRightOrUp.changeButtonStatus() == 3 && millis() - g > p)) {
            g = millis();
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
      /*********Выбор дней активности расписания*********/
      /**************************************************/
      else if (l_sm_1 >= 40 && l_sm_1 <= 46) {
        int sellDay = l_sm_1 - 40;

        if (millis() - prevMillis > span) {
          prevMillis = millis();
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print(days_of_the_week[sellDay]);
          lcd.print(" - ");
          lcd.print(s[sel_s].days_of_action[sellDay] ? "true" : "false");
        }

        //кнопка bLeftOrDown инвертирует состояние дня
        if (bLeftOrDown.changeButtonStatus() == 1) {
          s[sel_s].days_of_action[sellDay] = !s[sel_s].days_of_action[sellDay];
        }

        //кнопка bRightOrUp изменяет выбранный день недели
        if (bRightOrUp.changeButtonStatus() == 1) {
          if (sellDay < 6 - 1) l_sm_1++;
          else l_sm_1 = 40;
        }

        //По нажатию ОК
        if (bOkOrSave.changeButtonStatus() == 1) {
          l_sm_1 = 50;
        }
      }

      /**************************************************/
      /************Закончить настройку или***************/
      /**********настроить другое расписание*************/
      /**************************************************/
      else if (l_sm_1 == 50) {
        if (sel_s < number_of_type - 1) {
          sel_s++;
          l_sm_1 = 30;
        }
        else {
          for (int i = 0; i < number_of_type; i++) {
            EEPROM.put(i * sizeof(s[i]) + 1, s[i]);
          }
          l_m = 2;
        }
      }
      break;

    case 2:
      static int i = 0;
      if (now.second() != prev.second()) {
        prev = now;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(days_of_the_week[now.dayOfTheWeek()]);
        lcd.print(" ");
        lcd.print(printTime(now));
        lcd.print(" \1 ");
        lcd.print(relayState == LOW ? "off" : "on");
        lcd.setCursor(0, 1);

        int sn = 0;

        /**************************************************/
        /******Определяем какое расписание действует*******/
        /**************************************************/
        if (s[0].days_of_action[now.dayOfTheWeek()]) {
          sn = 0;
        } else if (s[1].days_of_action[now.dayOfTheWeek()]) {
          sn = 1;
        } else {
          lcd.print("Not now!");
          break;
        }

        DateTime ts = s[sn].schedule[i].time_start;
        DateTime te = s[sn].schedule[i].time_end;

        if (compareTime(now, ts) <= 0) {
          if (compareTime(now, ts) == 0) {
            relayOn();
            Serial.print("on " + printTime(now));
          }
          lcd.print("S #" + String(sn) + '.' + String(i + 1) + ' ');
          lcd.print(printTime(ts));
        } else if (compareTime(now, te) <= 0) {
          if (compareTime(now, te) == 0) {
            relayOn();
            Serial.print("on " + printTime(now));
          }
          lcd.print("E #" + String(sn) + '.' + String(i + 1) + ' ');
          lcd.print(printTime(te));
        } else {
          if (i < s[sn].count - 1) i++;
          lcd.print("Not now!");
        }
      }

      /**************************************************/
      /**********Выключаем звонок если включен***********/
      /**************************************************/
      if (millis() - timeOn > timeIsOn && relayState == HIGH) {
        relayState = LOW;
        digitalWrite(relay, relayState);
        Serial.println(" off " + printTime(now));
      }

      if (bNoOrExit.changeButtonStatus() == 3) l_m = 0;
      break;
  }
}

void relayOn() {
  timeOn = millis();
  relayState = HIGH;
  digitalWrite(relay, relayState);
}

int compareTime(DateTime a, DateTime b) {
  long as = ((a.hour() * 60) + a.minute()) * 60 + a.second();
  long bs = ((b.hour() * 60) + b.minute()) * 60 + b.second();

  if (as < bs) return -1;
  else if (as == bs) return 0;
  else return 1;
}

String printTime(DateTime t) {
  return String(t.hour()) + ':' + String(t.minute()) + ':' + String(t.second());
}

void toString(Schedule sch) {
  Serial.println("*********BEGIN*********");
  Serial.println("days_of_action");
  for (int i = 0; i < 6; i++) {
    Serial.print(' ' + String(sch.days_of_action[i]));
  }
  Serial.println();
  for (int i = 0; i < sch.count; i++) {
    Serial.println("schedule[" + String(i) + "]");
    Serial.println(sch.schedule[i].toString());
  }
  Serial.println("*********END*********");
}
