#include "calendar.h"
#include "csvutils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int START_YEAR = 1901;
const int MONTH_DAY_BIT = 12;
const int MONTH_NUM_BIT = 13;
const int SECOND_OF_DAY = 86400;

extern const char* HAN_DANWEI[];
extern const int MONTH_DAYS[];
extern const char* HAN_NUMBER[];
extern const char* HAN_KESHU[];
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

void _convert_number_to_chinese(int num, char *buf, int size)
{
    int bs = 0;
    char bytes[20];
    if (size < 4) {
        return;
    }

    // 0
    if (num == 0) {
        memcpy(buf, HAN_NUMBER[0], 4);
    }
    // 取出 num 的各位上的数值
    while (num) {
        bytes[bs++] = num % 10;
        num /= 10;
    }

    char *pbuf = buf;
    if (bs < 6) {
        for (int i = bs - 1; i >= 0; --i) {
            // 最高位是十位且最高位是1,不输出一十
            if (bytes[i] != 0) {
                if (!(i == bs - 1 && bs == 2 && bytes[i] == 1)) {
                    if (pbuf - buf > size - 3) {
                        return;
                    }
                    memcpy(pbuf, HAN_NUMBER[bytes[i]], 4);
                    pbuf += 3;
                }
                // 输出值非0位的单位
                if (i != 0) {
                    if (pbuf - buf > size - 3) {
                        return;
                    }
                    memcpy(pbuf, HAN_DANWEI[i], 4);
                    pbuf += 3;
                }
            }
            else {
                // 找到第一个非零位
                int c = i;
                while (c >= 0 && bytes[c] == 0) {
                    --c;
                } 
                if (c != -1) {
                    if (pbuf - buf > size - 3) {
                        return;
                    }
                    memcpy(pbuf, HAN_NUMBER[0], 4);
                    pbuf += 3;
                    i = c + 1;
                }
            }
        }
    }
}

// 获取指定年月日的时间戳(毫秒数，time得到的是秒数)
long long get_timestamp(const int year, const int month, const int day)
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
    return days * SECOND_OF_DAY;
}


void get_solar_date(const time_t *timestamp, SolarDate *solarDate) 
{
    struct tm *t = localtime(timestamp);
    solarDate->year = t->tm_year + 1900;    // tm_year = Year - 1900
    solarDate->month = t->tm_mon + 1;
    solarDate->day = t->tm_mday;
    solarDate->hour = t->tm_hour;
    solarDate->min = t->tm_min;
    solarDate->sec = t->tm_sec;
    solarDate->week = t->tm_wday == 0 ? 7 : t->tm_wday;
}

void get_shichen(const SolarDate *solarDate, ShiChen *shichen) 
{
    shichen->shi = ((solarDate->hour + 1) / 2) % 12;
    shichen->is_chu = (solarDate->hour % 2 != 0);
    shichen->duan = shichen->shi;

    const int secsOfHour = solarDate->min * 60 + solarDate->sec;
    const int secsOfDay = solarDate->hour * 3600 + secsOfHour;
    shichen->shike = secsOfHour / 864;
    shichen->ke = secsOfDay / 864 + 1;

    const int jingshu = (shichen->shi + 2) % 12;
    if (jingshu >= 1 && jingshu <= 5) {
        shichen->jing = jingshu;
    }
    else {
        shichen->jing = 0;
    }
}

void convert_shichen_to_chinese(const ShiChen *shiChen, ChineseShiChen *chineseShiChen)
{
    sprintf(chineseShiChen->shi, "%s%s", HAN_DIZHI[shiChen->shi], shiChen->is_chu ? "初" : "正");
    strcpy(chineseShiChen->duan, HAN_DUAN[shiChen->shi]);
    sprintf(chineseShiChen->shike, "%s刻", HAN_KESHU[shiChen->shike]);
    if (shiChen->jing) {
        sprintf(chineseShiChen->jing, "%s更", HAN_NUMBER[shiChen->jing]);
    }
    else {
        memset(chineseShiChen->jing, 0, sizeof(chineseShiChen->jing));
    }
    char buf[13];
    _convert_number_to_chinese(shiChen->ke, buf, sizeof(buf));
    sprintf(chineseShiChen->ke, "%s", buf);
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
void get_lunar_date(const int year, const int month, const int day, LunarDate *lunarDate)
{
    int lunar_year = year, lunar_month = 1, lunar_day = 1;
    BOOL is_leap = FALSE;
    // 获取year年春节的公历信息
    const unsigned short data = G_LUNAR_YEAR_DAY[year - START_YEAR];
    const unsigned short chunjie_month = (data >> 5) & 0x3;
    const unsigned short chunjie_day = data & 0x1f;
    LeapMonthInfo leapMonthInfo;
    get_leap_month_info(lunar_year, &leapMonthInfo);

    int span_days = (get_timestamp(year, month, day) - get_timestamp(year, chunjie_month, chunjie_day)) / SECOND_OF_DAY;
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
}


// 获取农历日期的中文表示
void convert_lunar_to_chinese(const LunarDate *lunarDate, ChineseLunarDate* chineseLunarDate)
{
    char xy_year[5];
    sprintf(xy_year, "%d", lunarDate->lunar_xy_year);
    int i;
    for (i = 0; i < strlen(xy_year); ++i) {
        strcpy(chineseLunarDate->c_lunar_xy_year + 3 * i, HAN_NUMBER[xy_year[i] - '0']);
    }
    strcpy(chineseLunarDate->c_lunar_xy_year + 3 * i, "年");

    int ad_year = lunarDate->lunar_ad_year;
    const int year_tiangan = (ad_year - 3) % 10 - 1;
	const int year_dizhi = (ad_year - 3) % 12 - 1;
    char ganzhi[7];
    strcpy(chineseLunarDate->c_lunar_year, _convert_to_ganzhi(year_tiangan, year_dizhi, ganzhi));

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
void get_ganzhi_date(const int year, const int month, const int day, const int hour, const int min, GanZhiInfo *ganZhiInfo)
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
	R %= 60;
	// 以上步骤得到日柱（日天干地支的次序数：1~60，下面将起点转化为0）
	R = (R == 0) ? 59 : R - 1;
	const int day_tiangan = R % 10;
	const int day_dizhi = R % 12;

	// 时天干与月天干类似，与日天干有循环对应关系
	const int shi_tiangan = (((R % 10) % 5) * 2 + (((hour + 1) / 2))) % 10;
	const int shi_dizhi = ((hour + 1) / 2) % 12;

	// 计算星次和星座
	const int xingcishu = (ganzhi_month == 12 ? 0 : ganzhi_month);
	const int this_month_zhongqi = 15 + (data[month - 1] & 0xf);
	const int xingzuoshu = (day < this_month_zhongqi ? month - 1 : month) % 12;

    char ganzhi[7];
    strcpy(ganZhiInfo->year_ganzhi, _convert_to_ganzhi(year_tiangan, year_dizhi, ganzhi));
    strcpy(ganZhiInfo->month_ganzhi, _convert_to_ganzhi(month_tiangan, month_dizhi, ganzhi));
    strcpy(ganZhiInfo->day_ganzhi, _convert_to_ganzhi(day_tiangan, day_dizhi, ganzhi));
    strcpy(ganZhiInfo->shi_ganzhi, _convert_to_ganzhi(shi_tiangan, shi_dizhi, ganzhi));
    strcpy(ganZhiInfo->xingci, HAN_XINGCI[xingcishu]);
    strcpy(ganZhiInfo->xingzuo, HAN_XINGZUO[xingzuoshu]);
}

// 获取(当前节气名 持续天数 下一节气名 下一节气月 下一节气日 下一节气还有多少天)
void get_jieqi_info(const int year, const int month, const int day, JieQiInfo *jieqiInfo)
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
	const long long t_now = get_timestamp(year, month, day);
	const long long tl_zhongqi = get_timestamp(l_year, l_month, l_zhongqi);
	const long long t_jieqi = get_timestamp(year, month, jieqi);
	const long long t_zhongqi = get_timestamp(year, month, zhongqi);
	const long long tn_jieqi = get_timestamp(n_year, n_month, n_jieqi);
	
	// 上月中气(l_year, l_month, l_zhongqi) 本月节气(year, month, jieqi) 本月中气(year, month, zhingqi) 下月节气(n_year, n_month, n_jieqi)
	if (day < jieqi) {
		// 上月中气 ~ 本月节气前
        strcpy(jieqiInfo->this_jieqi, HAN_JIEQI[month == 1 ? 23 : (month - 2) * 2 + 1]);
        strcpy(jieqiInfo->next_jieqi, HAN_JIEQI[(month - 1) * 2]);

		jieqiInfo->next_jieqi_month = month;
		jieqiInfo->next_jieqi_day = jieqi;
		jieqiInfo->this_jieqi_days = (t_now - tl_zhongqi) / SECOND_OF_DAY + 1;
		jieqiInfo->next_jieqi_days = (t_jieqi - t_now) / SECOND_OF_DAY;
	}
	else if (day == jieqi || day < zhongqi) {
        strcpy(jieqiInfo->this_jieqi, HAN_JIEQI[(month - 1) * 2]);
        strcpy(jieqiInfo->next_jieqi, HAN_JIEQI[(month - 1) * 2 + 1]);

		jieqiInfo->next_jieqi_month = month;
		jieqiInfo->next_jieqi_day = zhongqi;
		jieqiInfo->this_jieqi_days = (t_now - t_jieqi) / SECOND_OF_DAY + 1;
		jieqiInfo->next_jieqi_days = (t_zhongqi - t_now) / SECOND_OF_DAY;
	}
	else {
        jieqiInfo->next_jieqi_month = month == 12 ? 1 : month + 1;
        jieqiInfo->next_jieqi_day = n_jieqi;
		jieqiInfo->this_jieqi_days = (t_now - t_zhongqi) / SECOND_OF_DAY + 1;
		jieqiInfo->next_jieqi_days = (tn_jieqi - t_now) / SECOND_OF_DAY;
        strcpy(jieqiInfo->this_jieqi, HAN_JIEQI[(month - 1) * 2 + 1]);
        strcpy(jieqiInfo->next_jieqi, HAN_JIEQI[(jieqiInfo->next_jieqi_month - 1) * 2]);
	}
}

void display_solar_date(const SolarDate *solarDate) {
    char week[4];
    if (solarDate->week == 7) {
        strcpy(week, "日");
    }
    else {
        strcpy(week, HAN_NUMBER[solarDate->week]);
    }
    printf("[公历]: %d/%d/%d 星期%s %02d:%02d:%02d\n",
        solarDate->year,
        solarDate->month,
        solarDate->day, week,
        solarDate->hour,
        solarDate->min,
        solarDate->sec 
    );
}

void display_shichen(const ChineseShiChen *chineseShiChen)
{
    printf("[时辰]: %s(%s%s%s) %s\n",
        chineseShiChen->shi,
        chineseShiChen->duan,
        (strlen(chineseShiChen->jing) != 0) ? " " : "",
        chineseShiChen->jing,
        chineseShiChen->shike
    );
    printf("[刻制]: %s/一百\n", chineseShiChen->ke);
}

void display_lunar_date(const ChineseLunarDate *chineseLunarDate)
{
    printf("[农历]: 开元%s %s年 %s %s\n",
        chineseLunarDate->c_lunar_xy_year,
        chineseLunarDate->c_lunar_year,
        chineseLunarDate->c_lunar_month,
        chineseLunarDate->c_lunar_day
    );
}


void display_ganzhi_date(const GanZhiInfo *ganZhiInfo) 
{
    printf("[干支]: %s %s %s %s\n",
        ganZhiInfo->year_ganzhi,
        ganZhiInfo->month_ganzhi,
        ganZhiInfo->day_ganzhi,
        ganZhiInfo->shi_ganzhi
    );
    printf("[星次]: %s\n", ganZhiInfo->xingci);
    printf("[星座]: %s\n", ganZhiInfo->xingzuo);
}


void display_jieqi_info(const JieQiInfo *jieQiInfo)
{
    if (jieQiInfo->this_jieqi_days == 1) {
        printf("[节气]: 今天是\033[1m%s\033[0m, \033[1m%s\033[0m在%d天后\n",
            jieQiInfo->this_jieqi,
            jieQiInfo->next_jieqi,
            jieQiInfo->next_jieqi_days
        );
    }
    else {
        if (jieQiInfo->next_jieqi_days == 1 || jieQiInfo->next_jieqi_days == 2) {
            printf("[节气]: \033[1m%s\033[0m第%d天, %s天是\033[1m%s\033[0m\n",
                jieQiInfo->this_jieqi,
                jieQiInfo->this_jieqi_days,
                (jieQiInfo->next_jieqi_days == 1) ? "明" : "后",
                jieQiInfo->next_jieqi
            );
        }
        else {
            printf("[节气]: \033[1m%s\033[0m第%d天, \033[1m%s\033[0m在%d天后\n",
                jieQiInfo->this_jieqi,
                jieQiInfo->this_jieqi_days,
                jieQiInfo->next_jieqi,
                jieQiInfo->next_jieqi_days
            );
        }
    }
}

static void parse_solar_date(int year, char *date, int *month, int *day)
{
    *month = 0;
    *day = 0;
    int pos = 0;
    while (date[pos] && date[pos] != '/') {
        ++pos;
    }
    *month = atoi(date);
    if (date[pos + 1] == 'w') { // 1日 三
        int n = date[pos + 2] - '0';
        int w = date[pos + 3] - '0';
        SolarDate solarDate;
        time_t now = get_timestamp(year, *month, 1) - 8 * 3600;
        get_solar_date(&now, &solarDate);
        if (solarDate.week <= w) {
            *day = (w - solarDate.week + 7 * (n - 1) + 1);
        }
        else {
            *day = (w + 7 - solarDate.week) + 7 * (n - 1) + 1;
        }
    }
    else {
        *day = atoi(date + pos + 1);
    }
}

static void parse_lunar_date(int year, char *date, int *month, int *day)
{
    // char *pos = strstr(date, "月");
    // printf("%s\n", pos);
    *month = 0;
    *day = 0;
    for (size_t i = 0; i < 12; ++i)
    {
        char *pos = strstr(date, HAN_YUEFEN[i]);
        if (pos) {
            *month = i + 1;
            break;
        }
    }
    for (size_t i = 0; i < 30; ++i)
    {
        char *pos = strstr(date, HAN_RIQI[i]);
        if (pos) {
            *day = i + 1;
            break;
        }
    }
    char *pos = strstr(date, "晦日");
    if (pos) {
        *day = get_lunar_month_days(year, *month);
    }
}

void display_solar_festival(const SolarDate *solarDate)
{
    FILE *sfp = fopen("/usr/local/share/calendar/source/solar_festival.csv", "r");
    if (!sfp) {
        return;
    }
    char table[200][2][64];
    int lines, fields;
    csv_read((char ***)table, 20, 2, 64, &lines, &fields, sfp);
    
    for (size_t i = 0; i < lines; ++i)
    {
        int month, day;
        parse_solar_date(solarDate->year, table[i][1], &month, &day);
        if (solarDate->month == month && solarDate->day == day) {
            printf("[节日]: %s\n", table[i][0]);
            break;
        }
    }

    fclose(sfp);
}

void display_lunar_festival(const LunarDate *lunarDate)
{
    FILE *lfp = fopen("/usr/local/share/calendar/source/lunar_festival.csv", "r");
    if (!lfp) {
        return;
    }

    char table[200][2][64];
    int lines, fields;

    csv_read((char ***)table, 20, 2, 64, &lines, &fields, lfp);
    for (size_t i = 0; i < lines; ++i) {
        int month = 0, day = 0;
        parse_lunar_date(lunarDate->lunar_ad_year, table[i][1], &month, &day);
        if (!lunarDate->is_leap && lunarDate->lunar_month == month && lunarDate->lunar_day == day) {
            printf("[节日]: %s\n", table[i][0]);
            break;
        }
    }

    fclose(lfp);
}
