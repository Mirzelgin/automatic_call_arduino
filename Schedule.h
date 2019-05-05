#include <RTClib.h>
#include <time.h>


namespace tools {
char* days_of_the_week[] { "SAT", "MON", "TUE", "WED", "THU", "FRI" };

byte bell[8] =
{
  B00000,
  B00100,
  B01010,
  B01010,
  B01010,
  B11111,
  B00100,
  B00000,
};

class lesson
{
  public:
    bool active;
    DateTime time_start;
    DateTime time_end;

    lesson() {
      active      = true;
      time_start  = DateTime(2001, 1, 1, 0, 1);
      time_end    = DateTime(2001, 1, 1, 0, 2);
    }

    String toString() {
      String str = "";
      str = str + "active: " + String(active) + "\n";
      str = str + "time_start: " + String(time_start.hour()) + ':' + String(time_start.minute()) + "\n";
      str = str + "time_end: " + String(time_end.hour()) + ':' + String(time_end.minute()) + "\n";
      return str;
    }
};

class Schedule
{
  public:
    bool days_of_action[6];
    int count = 1;
    lesson* schedule;

    Schedule() {
      for (int i = 0; i < 6; i++) {
        days_of_action[i] = true;
      }
    }

    void begin() {
      schedule = new lesson[count];
    }

    String toString() {
      String str = "*****************\n";
      str = str + "days_of_action" + "\n";
      for (int i = 0; i < 6; i++) {
        str = str + ' ' + String(days_of_action[i]);
      }
      for (int i = 0; i < count; i++) {
        str = str + "\n" + "schedule[" + String(i) + "]\n";
        str = str + "active: " + String(schedule[i].active) + "\n";
        str = str + "time_start: " + String(schedule[i].time_start.hour()) + ':' + String(schedule[i].time_start.minute()) + "\n";
        str = str + "time_end: " + String(schedule[i].time_end.hour()) + ':' + String(schedule[i].time_end.minute()) + "\n";
      }
      str = str + "*****************";
      return str;
    }
};
}
