#include "calendar.h"
#include "config.h"

#include <stdio.h>
#include <time.h>

int main()
{
    printf("Calendar Version %s\n\n", CALENDAR_VERSION);
    
    SolarDate solarDate;
    time_t now = time(NULL);
    get_solar_date(&now, &solarDate);

    LunarDate lunarDate;
    ChineseLunarDate chineseLunarDate;
    get_lunar_date(solarDate.year, solarDate.month, solarDate.day, &lunarDate);
    convert_lunar_to_chinese(&lunarDate, &chineseLunarDate);

    GanZhiInfo ganZhiInfo;
    get_ganzhi_date(solarDate.year, solarDate.month, solarDate.day, solarDate.hour, solarDate.min, &ganZhiInfo);

    ShiChen shiChen;
    ChineseShiChen chineseShiChen;
    get_shichen(&solarDate, &shiChen);
    convert_shichen_to_chinese(&shiChen, &chineseShiChen);

    JieQiInfo jieQiInfo;
    get_jieqi_info(solarDate.year, solarDate.month, solarDate.day, &jieQiInfo);

    display_solar_date(&solarDate);
    display_lunar_date(&chineseLunarDate, &ganZhiInfo);
    display_shichen(&chineseShiChen);
    display_ganzhi_date(&ganZhiInfo);
    display_jieqi_info(&jieQiInfo);
}
