#include <RTClib.h>
#include <time.h>


namespace tools {
char* days_of_the_week[] { "SAT", "MON", "TUE", "WED", "THU", "FRI" };

struct lesson
{
  bool active;
  DateTime time_start;
  DateTime time_end;
};

/*
class time_edit {
public:
  static void add_hour(DateTime* st) {
    *st = add_hour(*st);
  }

  static DateTime add_hour(DateTime st) {
    if (st.hh < 23) st.hh++;
    else st.hh = 0;
    return st;
  }

  static void subtract_hour(t* st) {
    *st = subtract_hour(*st);
  }

  static t subtract_hour(t st) {
    if (st.hh > 0) st.hh--;
    else st.hh = 23;
    return st;
  }

  static void add_minutes(t* st) {
    *st = add_minutes(*st);
  }

  static t add_minutes(t st) {
    if (st.mm < 59) st.mm++;
    else st.mm = 0;
    return st;
  }

  static void subtract_minutes(t* st) {
    *st = subtract_minutes(*st);
  }

  static t subtract_minutes(t st) {
    if (st.mm > 0) st.mm--;
    else st.mm = 59;
    return st;
  }
};
*/

class Schedule
{
  public:
    bool days_of_action[6];
    int count;
    lesson* schedule;

    Schedule() {
      count = 1;
      for (int i = 0; i < count; i++)
      {
        (*(schedule + i)).active = false;
      }
    }

    Schedule(int count) {
      this->count = count;
      schedule = new lesson[count];
      for (int i = 0; i < count; i++)
      {
        (*(schedule + i)).active = false;
      }
    }

    void begin() {
      schedule = new lesson[count];
    }
};
}
