#pragma once

#include <time.h>

#define TRUE 1
#define FALSE 0

typedef int BOOL;


typedef struct SolarDateStruct {
    int year;
    int month;
    int day;
    int hour;
    int min;
    int sec;
    int week;
} SolarDate;


typedef struct LeapMonthInfoStruct {
    int lunar_month;
    int leap_month_days;
} LeapMonthInfo;


typedef struct LunarDateStruct {
    int lunar_ad_year;
    int lunar_xy_year;
    int lunar_month;
    int lunar_day;
    BOOL is_leap;
} LunarDate;


typedef struct ChineseLunarDateStruct {
    char c_lunar_ad_year[16];
    char c_lunar_xy_year[16];
    char c_lunar_month[10];
    char c_lunar_day[7];
} ChineseLunarDate;


typedef struct GanzhiInfoStruct {
    char year_ganzhi[7];
    char month_ganzhi[7];
    char day_ganzhi[7];
    char shi_ganzhi[7];
    char shichen[7];
    char keshu[7];
    char xingci[7];
    char xingzuo[10];
} GanZhiInfo;


typedef struct JieqiInfoStruct {
    char this_jieqi[7];
    int this_jieqi_days;
    char next_jieqi[7];
    int next_jieqi_month;
    int next_jieqi_day;
    int next_jieqi_days;
} JieQiInfo;


BOOL display_gregorian_calendar_date();
unsigned long long get_timestamp(const int year, const int month, const int day);
BOOL get_solar_date(const time_t *timestamp, SolarDate *solarDate);
// 公历闰年返回TRUE，否则返回FALSE
BOOL is_solar_leap_year(const int year);
// 获取(leap_month, leap_month_day)
BOOL get_leap_month_info(const int lunar_year, LeapMonthInfo *leapMonthInfo); 
// 返回指定农历年月的天数
int get_lunar_month_days(const int lunar_year, const int lunar_month);
// 获取(lunar_year, lunar_month, lunar_day, is_leap)
BOOL get_lunar_date(const int year, const int month, const int day, LunarDate *lunarDate);
BOOL display_lunar_date(const ChineseLunarDate *chineseLunarDate);


// 获取农历日期的中文表示
BOOL convert_lunar_to_chinese(const LunarDate *lunarDate, ChineseLunarDate* lunarDateChinese);
// 获取干支的中文表示：天干地支序号从0开始
const char * _convert_to_ganzhi(const int tiangan, const int dizhi, char ganzhi[]);


// 获取(干支年 月 日 时 地支时 刻数 星次:以中气分割 星座:以节气分割)
BOOL get_ganzhi_date(const int year, const int month, const int day, const int hour, const int min, GanZhiInfo *ganZhiInfo);
// 获取(当前节气名 持续天数 下一节气名 下一节气月 下一节气日 下一节气还有多少天)
BOOL get_jieqi_info(const int year, const int month, const int day, JieQiInfo *jieqiInfo);
BOOL display_ganzhi_date(const GanZhiInfo *ganZhiInfo);
BOOL display_jieqi_info(const JieQiInfo *jieQiInfo);
BOOL display_solar_date(const SolarDate *solarDate);

