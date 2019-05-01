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

    lesson(){
      active      = true;
      time_start  = DateTime(2001, 1, 1, 0, 1);
      time_end    = DateTime(2001, 1, 1, 0, 2);
    }
};

/*
  class time_edit {
  public:
  static void add_hour(DateTime* st) {
     st = add_hour(*st);
  }

  static DateTime add_hour(DateTime st) {
    if (st.hh < 23) st.hh++;
    else st.hh = 0;
    return st;
  }

  static void subtract_hour(t* st) {
     st = subtract_hour(*st);
  }

  static t subtract_hour(t st) {
    if (st.hh > 0) st.hh--;
    else st.hh = 23;
    return st;
  }

  static void add_minutes(t* st) {
     st = add_minutes(*st);
  }

  static t add_minutes(t st) {
    if (st.mm < 59) st.mm++;
    else st.mm = 0;
    return st;
  }

  static void subtract_minutes(t* st) {
     st = subtract_minutes(*st);
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
    int count = 1;
    lesson* schedule;

    Schedule() {
      for (int i = 0; i < 6; i++){
        days_of_action[i] = true;
      }
    }

    void begin() {
      schedule = new lesson[count];
    }
};
}
