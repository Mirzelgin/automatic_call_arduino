#include <RTClib.h>

class lesson
{
  public:
    bool active;
    DateTime time_start;
    DateTime time_end;

    lesson() {
      active      = false;
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
    static const int maxCount = 10;
    int count = 1;
    lesson schedule[maxCount];

    Schedule() {
      for (int i = 0; i < 6; i++) {
        days_of_action[i] = true;
      }
    }

    void begin() {
      for (int i = 0; i < count; i++) {
        schedule[i].active = true;
      }
    }
};
