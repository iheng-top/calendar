#include "calendar.h"
#include "config.h"

#include <stdio.h>
#include <time.h>

int main()
{
    printf("Calendar Version %d.%d\n\n", CALENDAR_VERSION_MAJOR, CALENDAR_VERSION_MINOR);
    
    SolarDate solarDate;
    time_t now = time(NULL);
    get_solar_date(&now, &solarDate);
    display_solar_date(&solarDate);

    LunarDate lunarDate;
    ChineseLunarDate chineseLunarDate;
    get_lunar_date(solarDate.year, solarDate.month, solarDate.day, &lunarDate);
    convert_lunar_to_chinese(&lunarDate, &chineseLunarDate);
    display_lunar_date(&chineseLunarDate);

    GanZhiInfo ganZhiInfo;
    get_ganzhi_date(solarDate.year, solarDate.month, solarDate.day, solarDate.hour, solarDate.min, &ganZhiInfo);
    display_ganzhi_date(&ganZhiInfo);

    JieQiInfo jieQiInfo;
    get_jieqi_info(solarDate.year, solarDate.month, solarDate.day, &jieQiInfo);
    display_jieqi_info(&jieQiInfo);
}