#ifndef ___HOLIDAY_H__
#define ___HOLIDAY_H__

struct Holiday {
    int year;
    int month;
    int holidays[50];
    int length;
};

bool getHolidays(Holiday& result, int year, int month);

#endif
