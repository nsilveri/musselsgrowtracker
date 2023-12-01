#pragma once

#include <Arduino.h>

struct DateTime {
    int year;
    int month;
    int day;
    int hours;
    int minutes;
    int seconds;
};

class DateTimeConverter {
public:
    static uint32_t convertDateToUnixTime(const uint16_t &year, const uint8_t &month, const uint8_t &day,
                                          const uint8_t &hour, const uint8_t &minute, const uint8_t &second);

    static DateTime convertUnixTimeToDate(long unixTime);
};

extern DateTimeConverter DateConverter;