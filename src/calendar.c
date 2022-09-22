#include "calendar.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int START_YEAR = 1901;
const int MONTH_DAY_BIT = 12;
const int MONTH_NUM_BIT = 13;
const int MILLISECOND_OF_YEAR = 86400000;

extern const int MONTH_DAYS[];
extern const char* HAN_NUMBER[];
extern const char* HAN_YUEFEN[];
extern const char* HAN_RIQI[]; 
extern const char* HAN_XINGQI[];
extern const char* HAN_TIANGAN[];
extern const char* HAN_DIZHI[];
extern const char* HAN_SHENGXIAO[];
extern const char* HAN_JIEQI[];
extern const char* HAN_DUAN[];
extern const char* HAN_XINGZUO[];
extern const char* HAN_XINGCI[];
extern const unsigned int G_LUNAR_MONTH_DAY[];
extern const unsigned short G_LUNAR_YEAR_DAY[];
extern const unsigned char G_LUNAR_JIEQI[];

// 获取指定年月日的时间戳
unsigned long long get_timestamp(const int year, const int month, const int day)
{
    unsigned long long days = 0;
    for (int i = 1970; i < year; ++i) {
        if (is_solar_leap_year(i)) {
            days += 366;
        }
        else {
            days += 365;
        }
    }
    for (int i = 0; i < month - 1; ++i) {
        days += MONTH_DAYS[i];
    }
    if (month > 2 && is_solar_leap_year(year)) {
        days += 1;
    }
    days += day - 1;
    // int * int -> int，结果会溢出
    return days * MILLISECOND_OF_YEAR;
}


BOOL get_solar_date(const time_t *timestamp, SolarDate *solarDate) {
    struct tm *t = localtime(timestamp);
    solarDate->year = t->tm_year + 1900;    // tm_year = Year - 1900
    solarDate->month = t->tm_mon + 1;
    solarDate->day = t->tm_mday;
    solarDate->hour = t->tm_hour;
    solarDate->min = t->tm_min;
    solarDate->sec = t->tm_sec;
    solarDate->week = t->tm_wday == 0 ? 7 : t->tm_wday;

    return TRUE;
}


BOOL display_solar_date(const SolarDate *solarDate) {
    char week[4];
    if (solarDate->week == 7) {
        strcpy(week, "日");
    }
    else {
        strcpy(week, HAN_NUMBER[solarDate->week]);
    }
    printf("[公历]: %d/%d/%d 星期%s %02d:%02d:%02d\n", solarDate->year, solarDate->month, solarDate->day, week, solarDate->hour, solarDate->min, solarDate->sec);
    return TRUE;
}


// 公历闰年返回TRUE，否则返回FALSE
BOOL is_solar_leap_year(const int year)
{
    return ((year % 4 == 0) && (year % 100 != 0) || (year % 400 == 0));
}

// 获取(leap_month, leap_month_day)
BOOL get_leap_month_info(const int lunar_year, LeapMonthInfo *leapMonthInfo)
{
    const unsigned int data = G_LUNAR_MONTH_DAY[lunar_year - START_YEAR];
    leapMonthInfo->lunar_month = (data >> MONTH_NUM_BIT) & 0xf;
    if (leapMonthInfo->lunar_month) {
        if (data & (1 << MONTH_DAY_BIT)) {
            leapMonthInfo->leap_month_days = 30;
        }
        else {
            leapMonthInfo->leap_month_days = 29;
        }
        return TRUE;
    }
    return FALSE;
}

// 返回指定农历年月的天数
int get_lunar_month_days(const int lunar_year, const int lunar_month)
{
    return (G_LUNAR_MONTH_DAY[lunar_year - START_YEAR] & (1 << (lunar_month - 1))) ? 30 : 29;
}

// 获取(lunar_year, lunar_month, lunar_day, is_leap)
BOOL get_lunar_date(const int year, const int month, const int day, LunarDate *lunarDate)
{
    int lunar_year = year, lunar_month = 1, lunar_day = 1;
    BOOL is_leap = FALSE;
    // 获取year年春节的公历信息
    const unsigned short data = G_LUNAR_YEAR_DAY[year - START_YEAR];
    const unsigned short chunjie_month = (data >> 5) & 0x3;
    const unsigned short chunjie_day = data & 0x1f;
    LeapMonthInfo leapMonthInfo;
    get_leap_month_info(lunar_year, &leapMonthInfo);

    int span_days = (get_timestamp(year, month, day) - get_timestamp(year, chunjie_month, chunjie_day)) / MILLISECOND_OF_YEAR;
    if (span_days >= 0) {
        int month_days = get_lunar_month_days(lunar_year, lunar_month);
        while (span_days >= month_days) {
            span_days -= month_days;
            // 闰月
            if (lunar_month == leapMonthInfo.lunar_month) {
                if (span_days < leapMonthInfo.leap_month_days) {
                    is_leap = TRUE;
                    break;
                }
                span_days -= leapMonthInfo.leap_month_days;
            }
            lunar_month += 1;
            month_days = get_lunar_month_days(lunar_year, lunar_month);
        }
        lunar_day += span_days;
    }
    else {
        lunar_month = 12;
        lunar_year -= 1;
        int month_days = get_lunar_month_days(lunar_year, lunar_month);
        while (abs(span_days) >= month_days) {
            span_days += month_days;
            lunar_month -= 1;
            if (lunar_month == leapMonthInfo.lunar_month) {
                if (abs(span_days) < month_days) {
                    is_leap = TRUE;
                    break;
                }
                span_days += leapMonthInfo.leap_month_days;
            }
            month_days = get_lunar_month_days(lunar_year, lunar_month);
        }
        lunar_day += (month_days + span_days);
    }

    lunarDate->is_leap = is_leap;
    lunarDate->lunar_ad_year = lunar_year;
    lunarDate->lunar_xy_year = lunar_year + 2697;
    lunarDate->lunar_month = lunar_month;
    lunarDate->lunar_day = lunar_day;
    return TRUE;
}


// 获取农历日期的中文表示
BOOL convert_lunar_to_chinese(const LunarDate *lunarDate, ChineseLunarDate* chineseLunarDate)
{
    char ad_year[5];
    char xy_year[5];
    sprintf(ad_year, "%d", lunarDate->lunar_ad_year);
    sprintf(xy_year, "%d", lunarDate->lunar_xy_year);
    int i;
    for (i = 0; i < strlen(ad_year); ++i) {
        strcpy(chineseLunarDate->c_lunar_ad_year + 3 * i, HAN_NUMBER[ad_year[i] - '0']);
    }
    strcpy(chineseLunarDate->c_lunar_ad_year + 3 * i, "年");
    for (i = 0; i < strlen(xy_year); ++i) {
        strcpy(chineseLunarDate->c_lunar_xy_year + 3 * i, HAN_NUMBER[xy_year[i] - '0']);
    }
    strcpy(chineseLunarDate->c_lunar_xy_year + 3 * i, "年");

    if (lunarDate->is_leap) {
        strcpy(chineseLunarDate->c_lunar_month, "闰");
        strcpy(chineseLunarDate->c_lunar_month + 3, HAN_YUEFEN[lunarDate->lunar_day - 1]);
    }
    else {
        strcpy(chineseLunarDate->c_lunar_month, HAN_YUEFEN[lunarDate->lunar_month - 1]);
    }

    strcpy(chineseLunarDate->c_lunar_day, HAN_RIQI[lunarDate->lunar_day - 1]);
}


// 获取干支的中文表示：天干地支序号从0开始
const char * _convert_to_ganzhi(const int tiangan, const int dizhi, char ganzhi[])
{
    sprintf(ganzhi, "%s%s", HAN_TIANGAN[tiangan], HAN_DIZHI[dizhi]);
    return ganzhi;
}


// 获取(干支年 月 日 时 地支时 刻数 星次:以中气分割 星座:以节气分割)
BOOL get_ganzhi_date(const int year, const int month, const int day, const int hour, const int min, GanZhiInfo *ganZhiInfo)
{
    // 截取year年节气日期
	const unsigned char * data = &G_LUNAR_JIEQI[12 * (year - 1900)]; // 12 * (year - 1900 + 1)

	// 计算干支对应公元纪年(ganzhi_year)和在干支纪月的第几个月份(ganzhi_month)
	// 元旦对应干支的上一年第11月
	int ganzhi_year = year - 1, ganzhi_month = 11, m = 1;
	for (; data < (unsigned char *)(G_LUNAR_JIEQI + (12 * (year - 1900 + 1))); ++data) {
		// 节气日是干支月变化的日期
		const unsigned char jieqi_date = (15 - (*data >> 4));
		if (month == m) {
			if (day >= jieqi_date) {
				ganzhi_month += 1;
			}
			break;
		}
		else {
			m += 1;
			ganzhi_month += 1;
		}
	}
	// 以上计数时12月以后按13月计算，接下来换回正常计数
	if (ganzhi_month > 12) {
		ganzhi_year += 1;
		ganzhi_month -= 12;
	}
	const int year_tiangan = (ganzhi_year - 3) % 10 - 1;
	const int year_dizhi = (ganzhi_year - 3) % 12 - 1;


	// 计算月天干次序：冬月对应地支为“子”，天干与年份有循环对应关系
	int month_tiangan = (ganzhi_year % 5 - 2) * 2 - 1;
	month_tiangan = (month_tiangan < 0) ? (month_tiangan + 10) : month_tiangan;
	month_tiangan += (ganzhi_month - 1) - 1;
	month_tiangan %= 10;
	const int month_dizhi = (ganzhi_month + 1) % 12;

	// 使用高氏日柱公式计算日天干次序（R: 1~60，甲子~癸亥）
	const int _month[] = {0, 31, -1, 30, 0, 31, 1, 32, 3, 33, 4, 34}; // 月基数
	const int _year[] = {3, 47, 31, 15, 0, 44, 28, 12, 57, 41}; // 世纪常数（17 ~ 26世纪）
	const int S = year % 100 - 1;
	const int U = S % 4;
	const int M = _month[month - 1];
	int R = (S / 4) * 6 + 5 * ((S / 4) * 3 + U) + M + day + _year[((year - 1) / 100) + 1 - 17];
	if (is_solar_leap_year(year) && month > 2) {
		R += 1;
	}
	R = R == 60 ? R : R % 60;
	// 以上步骤得到日柱（日天干地支的次序数：1~60，下面将起点转化为0）
	R -= 1;
	const int day_tiangan = R % 10;
	const int day_dizhi = R % 12;

	// 时天干与月天干类似，与日天干有循环对应关系
	const int shi_tiangan = (((R % 10) % 5) * 2 + (((hour + 1) / 2))) % 10;
	const int shi_dizhi = (hour + 1) / 2;

	// 计算多少刻：正点过后加4刻
	const int keshu = (hour % 2 != 0) ? min / 15 + 1 : (min / 15) + 1 + 5;

	// 计算星次和星座
	const int xingcishu = (ganzhi_month == 12 ? 0 : ganzhi_month);
	const int this_month_zhongqi = 15 + (data[month - 1] & 0xf);
	const int xingzuoshu = (day < this_month_zhongqi ? month - 1 : month) % 12;

    char ganzhi[7];
    strcpy(ganZhiInfo->year_ganzhi, _convert_to_ganzhi(year_tiangan, year_dizhi, ganzhi));
    strcpy(ganZhiInfo->month_ganzhi, _convert_to_ganzhi(month_tiangan, month_dizhi, ganzhi));
    strcpy(ganZhiInfo->day_ganzhi, _convert_to_ganzhi(day_tiangan, day_dizhi, ganzhi));
    strcpy(ganZhiInfo->shi_ganzhi, _convert_to_ganzhi(shi_tiangan, shi_dizhi, ganzhi));
    sprintf(ganZhiInfo->shichen, "%s时", HAN_DIZHI[shi_dizhi]);
    sprintf(ganZhiInfo->keshu, "%s刻", HAN_NUMBER[keshu]);
    strcpy(ganZhiInfo->xingci, HAN_XINGCI[xingcishu]);
    strcpy(ganZhiInfo->xingzuo, HAN_XINGZUO[xingzuoshu]);

	return TRUE;
}

// 获取(当前节气名 持续天数 下一节气名 下一节气月 下一节气日 下一节气还有多少天)
BOOL get_jieqi_info(const int year, const int month, const int day, JieQiInfo *jieqiInfo)
{
    const int last_month = G_LUNAR_JIEQI[12 * (year - 1900) + month - 2];
	const int this_month = G_LUNAR_JIEQI[12 * (year - 1900) + month - 1];
	const int next_month = G_LUNAR_JIEQI[12 * (year - 1900) + month];
	const int l_jieqi = 15 - (last_month >> 4), l_zhongqi = 15 + (last_month & 0xf);
	const int jieqi = 15 - (this_month >> 4), zhongqi = 15 + (this_month & 0xf);
	const int n_jieqi = 15 - (next_month >> 4), n_zhongqi = 15 + (next_month & 0xf);
	// let this_jieqi = '', next_jieqi = '', next_jieqi_month = 0, next_jieqi_day = 0, l_num = 0, num = 0;

	const int l_month = month == 1 ? 12 : month - 1, l_year = month == 1 ? year - 1 : year;
	const int n_month = month == 12 ? 1 : month + 1, n_year = month == 12 ? year + 1 : year;
	const unsigned long long t_now = get_timestamp(year, month, day);
	const unsigned long long tl_zhongqi = get_timestamp(l_year, l_month, l_zhongqi);
	const unsigned long long t_jieqi = get_timestamp(year, month, jieqi);
	const unsigned long long t_zhongqi = get_timestamp(year, month, zhongqi);
	const unsigned long long tn_jieqi = get_timestamp(n_year, n_month, n_jieqi);
	
	// 上月中气(l_year, l_month, l_zhongqi) 本月节气(year, month, jieqi) 本月中气(year, month, zhingqi) 下月节气(n_year, n_month, n_jieqi)
	if (day < jieqi) {
		// 上月中气 ~ 本月节气前
        strcpy(jieqiInfo->this_jieqi, HAN_JIEQI[month == 1 ? 23 : (month - 2) * 2 + 1]);
        strcpy(jieqiInfo->next_jieqi, HAN_JIEQI[(month - 1) * 2]);

		jieqiInfo->next_jieqi_month = month;
		jieqiInfo->next_jieqi_day = jieqi;
		jieqiInfo->this_jieqi_days = (t_now - tl_zhongqi) / MILLISECOND_OF_YEAR;
		jieqiInfo->next_jieqi_days = (t_jieqi - t_now) / MILLISECOND_OF_YEAR;
	}
	else if (day == jieqi || day < zhongqi) {
        strcpy(jieqiInfo->this_jieqi, HAN_JIEQI[(month - 1) * 2]);
        strcpy(jieqiInfo->next_jieqi, HAN_JIEQI[(month - 1) * 2 + 1]);

		jieqiInfo->next_jieqi_month = month;
		jieqiInfo->next_jieqi_day = zhongqi;
		jieqiInfo->this_jieqi_days = (t_now - t_jieqi) / MILLISECOND_OF_YEAR;
		jieqiInfo->next_jieqi_days = (t_zhongqi - t_now) / MILLISECOND_OF_YEAR;
	}
	else {
        jieqiInfo->next_jieqi_month = month == 12 ? 1 : month + 1;
        jieqiInfo->next_jieqi_day = n_jieqi;
		jieqiInfo->this_jieqi_days = (t_now - t_zhongqi) / MILLISECOND_OF_YEAR;
		jieqiInfo->next_jieqi_days = (tn_jieqi - t_now) / MILLISECOND_OF_YEAR;
        strcpy(jieqiInfo->this_jieqi, HAN_JIEQI[(month - 1) * 2 + 1]);
        strcpy(jieqiInfo->next_jieqi, HAN_JIEQI[(jieqiInfo->next_jieqi_month - 1) * 2]);
	}
	return TRUE;
}

BOOL display_gregorian_date()
{
    // printf("[公历]: %d/%d/%d %s %d:%d:%d\n");
    return FALSE;
}

BOOL display_lunar_date(const ChineseLunarDate *chineseLunarDate)
{
    printf("[农历]: 公元%s(开元%s) %s %s\n", chineseLunarDate->c_lunar_ad_year, chineseLunarDate->c_lunar_xy_year, chineseLunarDate->c_lunar_month, chineseLunarDate->c_lunar_day);
    return TRUE;
}


BOOL display_ganzhi_date(const GanZhiInfo *ganZhiInfo) 
{
    printf("[干支历]: %s %s %s %s\n[时辰]: %s %s\n[星次]: %s\n[星座]: %s\n", ganZhiInfo->year_ganzhi, ganZhiInfo->month_ganzhi, ganZhiInfo->day_ganzhi, ganZhiInfo->shi_ganzhi, ganZhiInfo->shichen, ganZhiInfo->keshu, ganZhiInfo->xingci, ganZhiInfo->xingzuo);
    return TRUE;
}


BOOL display_jieqi_info(const JieQiInfo *jieQiInfo)
{
    printf("[节气]: 距上一个节气[%s]已经过了%d天, 距离下一个节气[%s(%d月%d日)]还有%d天\n", jieQiInfo->this_jieqi, jieQiInfo->this_jieqi_days, jieQiInfo->next_jieqi, jieQiInfo->next_jieqi_month, jieQiInfo->next_jieqi_day, jieQiInfo->next_jieqi_days);
    return TRUE;
}
