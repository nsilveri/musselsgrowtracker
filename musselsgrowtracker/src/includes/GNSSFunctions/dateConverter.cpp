#include "dateConverter.h"
#include <ctime>

uint32_t DateTimeConverter::convertDateToUnixTime(const uint16_t &year, const uint8_t &month, const uint8_t &day,
                                                  const uint8_t &hour, const uint8_t &minute, const uint8_t &second)
{
   uint32_t t;

	//Year
	uint16_t y(year);
	//Month of year
	uint8_t m(month);
	//Day of month
	uint8_t d(day);

	//January and February are counted as months 13 and 14 of the previous year
	if( m <= 2 )
	{
		m += 12;
		y -= 1;
	}

	//Convert years to days
	t = (365 * y) + (y / 4) - (y / 100) + (y / 400);
	//Convert months to days
	t += (30 * m) + (3 * (m + 1) / 5) + d;
	//Unix time starts on January 1st, 1970
	t -= 719561;
	//Convert days to seconds
	t *= 86400;
	//Add hours, minutes and seconds
	t += (3600 * hour) + (60 * minute) + second;

	//Return Unix time
	return t;
}

DateTime DateTimeConverter::convertUnixTimeToDate(long unixTime) {
    DateTime dateTime;
    
    time_t rawTime = unixTime;
    struct tm *timeInfo;
    
    timeInfo = localtime(&rawTime);
    
    dateTime.year = timeInfo->tm_year + 1900;
    dateTime.month = timeInfo->tm_mon + 1;
    dateTime.day = timeInfo->tm_mday;
    dateTime.hours = timeInfo->tm_hour;
    dateTime.minutes = timeInfo->tm_min;
    dateTime.seconds = timeInfo->tm_sec;
    
    return dateTime;
}

DateTimeConverter DateConverter;