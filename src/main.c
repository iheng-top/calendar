#include "calendar.h"
#include "config.h"

#include <stdio.h>

int main()
{
    printf("Calendar Version %d.%d\n\n", CALENDAR_VERSION_MAJOR, CALENDAR_VERSION_MINOR);

    LunarDate lunarDate;
    ChineseLunarDate chineseLunarDate;
    get_lunar_date(2022, 9, 18, &lunarDate);
    convert_lunar_to_chinese(&lunarDate, &chineseLunarDate);
    display_lunar_date(&chineseLunarDate);

    GanZhiInfo ganZhiInfo;
    get_ganzhi_date(2022, 9, 18, 17, 55, &ganZhiInfo);
    display_ganzhi_date(&ganZhiInfo);

    JieQiInfo jieQiInfo;
    get_jieqi_info(2022, 9, 18, &jieQiInfo);
    display_jieqi_info(&jieQiInfo);
}