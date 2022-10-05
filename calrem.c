#if 0
set -e; [ "$0" -nt "$0.bin" ] &&
gcc -Wall -Wextra -pedantic -std=c99 "$0" -o "$0.bin"
exec "$0.bin" "$@"
#endif

#define _POSIX_C_SOURCE 202210L
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#define LEN(x) (sizeof(x)/sizeof((x[0])))

enum {
	JAN=1, FEB, MAR, APR,
	MAY, JUN, JUL, AUG,
	SEP, OCT, NOV, DEC,
};

#define FIRST (1U)
#define HOLIDAY (1U << 1)
#define ZJAZD (1U << 2)

struct reminder {
	int8_t day_of_month;
	int8_t month;
	int8_t week_day;
	int year;
	int class;
	const char *name;
};

enum {
	EASTER,
	EASTER_MONDAY,
	PENTECOST,
	CORPUS_CHRISTI,
};

struct date {
	int8_t day_of_month;
	int8_t month;
	int8_t week_day;
	bool leap_year;
	//int16_t year_day;
	int year;
};

const char *class_str[] = {
	"first",
	"holiday",
	"zjazd",
};

const char *weekday_str[] = {
	"Nd",
	"Pn",
	"Wt",
	"Śr",
	"Cz",
	"Pt",
	"Sb",
};

const char *month_str[] = {
	"Sty", "Lut", "Mar", "Kwi",
	"Maj", "Cze", "Lip", "Sie",
	"Wrz", "Paź", "Lis", "Gru",
};

const char *month_str_eng[] = {
	"Jan", "Feb", "Mar", "Apr",
	"May", "Jun", "Jul", "Aug",
	"Sep", "Oct", "Nov", "Dec",
};

const int month_len[] = {
	31, 28, 31, 30,
	31, 30, 31, 31,
	30, 31, 30, 31,
};

struct reminder reminders[] = {
	[EASTER] = {.class = HOLIDAY, .name = "Wielkanoc"},
	[EASTER_MONDAY] = {.class = HOLIDAY, .name = "Poniedziałek Wielkanocny"},
	[PENTECOST] = {.class = HOLIDAY, .name = "Zesłanie Ducha Świętego"},
	[CORPUS_CHRISTI] = {.class = HOLIDAY, .name = "Boże Ciało"},
	{1, 1, 0, 0, HOLIDAY, "Nowy Rok"},
	{6, 1, 0, 0, HOLIDAY, "Święto Trzech Króli"},
	{1, 5, 0, 0, HOLIDAY, "Święto Pracy"},
	{3, 5, 0, 0, HOLIDAY, "Święto Narodowe Trzeciego Maja"},
	{5, 8, 0, 0, HOLIDAY, "Święto Wojska Polskiego"},
	{1, 11, 0, 0, HOLIDAY, "Wszystkich Świętych"},
	{11, 11, 0, 0, HOLIDAY, "Narodowe Święto Niepodległości"},
	{25, 12, 0, 0, HOLIDAY, "Boże Narodzenie"},
	{26, 12, 0, 0, HOLIDAY, "Boże Narodzenie"},

	{8, 10, 0, 2022, ZJAZD, "Zjazd 1"},
	{22, 10, 0, 2022, ZJAZD, "Zjazd 2"},
	{5, 11, 0, 2022, ZJAZD, "Zjazd 3"},
	{19, 11, 0, 2022, ZJAZD, "Zjazd 4 zdalny"},
	{26, 11, 0, 2022, ZJAZD, "Zjazd? 5"},
	{3, 12, 0, 2022, ZJAZD, "Zjazd 6"},
	{17, 12, 0, 2022, ZJAZD, "Zjazd 7 zdalny"},
	{14, 1, 0, 2023, ZJAZD, "Zjazd 8 zdalny"},
	{28, 1, 0, 2023, ZJAZD, "Zjazd 9"},
	{11, 2, 0, 2023, ZJAZD, "Zjazd 10 zdalny"},
	{25, 2, 0, 2023, ZJAZD, "Zjazd 11"},
	{4, 3, 0, 2023, ZJAZD, "Zjazd? 12 zdalny"},
	{11, 3, 0, 2023, ZJAZD, "Zjazd 13"},
	{25, 3, 0, 2023, ZJAZD, "Zjazd 14"},
	{1, 4, 0, 2023, ZJAZD, "Zjazd? 15"},
	{15, 4, 0, 2023, ZJAZD, "Zjazd 16 zdalny"},
	{29, 4, 0, 2023, ZJAZD, "Zjazd 17"},
	{13, 5, 0, 2023, ZJAZD, "Zjazd 18"},
	{20, 5, 0, 2023, ZJAZD, "Zjazd? 19"},
	{3, 6, 0, 2023, ZJAZD, "Zjazd 20"},
	{17, 6, 0, 2023, ZJAZD, "Zjazd 21"},
	{24, 6, 0, 2023, ZJAZD, "Zjazd 22"},
};

void move_date(struct date *date, int days);

void
reminder_from_date(struct reminder *reminder, struct date *date)
{
	reminder->year = date->year;
	reminder->month = date->month;
	reminder->day_of_month = date->day_of_month;
}

bool
reminder_match_date(struct reminder *reminder, struct date *date)
{
	return
		(reminder->year == 0 || reminder->year == date->year) &&
		(reminder->month == 0 || reminder->month == date->month) &&
		(reminder->day_of_month == 0 || reminder->day_of_month == date->day_of_month) &&
		(reminder->week_day == 0 || reminder->week_day == date->week_day);
}

bool
is_leap_year(int y)
{
	if(y % 4) {
		return false;
	} else if(y % 100) {
		return true;
	} else if(y % 400) {
		return false;
	} else {
		return true;
	}
}

int
get_month_len(struct date *date)
{
	if(date->month == FEB) {
		return month_len[date->month - 1] + date->leap_year;
	}
	return month_len[date->month - 1];
}

void
get_easter_date(struct date *date)
{
	int a = date->year % 19;
	int b = date->year / 100;
	int c = date->year % 100;

	int d = b / 4;
	int e = b % 4;

	int f = (b + 8) / 25;
	int g = (b - f + 1) / 3;

	int h = (19 * a + b - d - g + 15) % 30;

	int i = c / 4;
	int k = c % 4;

	int l = (32 + 2 * e + 2 * i - h - k) % 7;
	int m = (a + 11 * h + 22 * l) / 451;
	int p = (h + l - 7 * m + 114) % 31;

	date->month = (h + l - 7 * m + 114) / 31;
	date->day_of_month = p + 1;
	date->week_day = 7;
}


// TODO: take reminders as input
void
movable_reminders(struct date *date)
{
	date->leap_year = is_leap_year(date->year);

	struct date easter = {.year = date->year};
	get_easter_date(&easter);
	reminder_from_date(&reminders[EASTER], &easter);

	struct date easter_monday = easter;
	move_date(&easter_monday, 1);
	reminder_from_date(&reminders[EASTER_MONDAY], &easter_monday);

	struct date pentecost = easter;
	move_date(&pentecost, 49);
	reminder_from_date(&reminders[PENTECOST], &pentecost);

	struct date corpus_christi = easter;
	move_date(&corpus_christi, 60);
	reminder_from_date(&reminders[CORPUS_CHRISTI], &corpus_christi);
}

void
move_date(struct date *date, int days)
{
	date->day_of_month += days;
	date->week_day = (date->week_day + days) % 7;

	if(days > 0) {
		while(date->day_of_month > get_month_len(date)) {
			date->day_of_month -= get_month_len(date);
			date->month++;
			if(date->month > DEC) {
				date->year++;
				movable_reminders(date);
				date->month = JAN;
			}
		}
	} else {
		while(date->day_of_month < 1) {
			date->month--;
			if(date->month < JAN) {
				date->year--;
				movable_reminders(date);
				date->month = DEC;
			}
			date->day_of_month += get_month_len(date);
		}
	}
}

int firstday = 1;

int
main(int argc, char *argv[])
{
	time_t now = time(NULL);
	struct tm today = {0};
	localtime_r(&now, &today);

	struct date date = {
		.day_of_month = today.tm_mday,
		.month = today.tm_mon + 1,
		.year = today.tm_year + 1900,
		.week_day = today.tm_wday,
	};

	puts(
		"<!DOCTYPE html>\n"
		"<html lang=pl>\n"
		"<meta charset=utf-8>\n"
		"<meta name=viewport content=\"width=device-width,initial-scale=1\">\n"
		"<style>\n"
		"body{\n"
		"font:menu;font-size:1.125em;\n"
		"padding:0 0.5em 90vh 0.5em;\n"
		"}\n"
		"table, tbody {\n"
		"border-collapse: collapse;\n"
		"display: block;\n"
		"width: 100%;\n"
		"}\n"
		"td:nth-child(6) {\n"
		"color: gray;\n"
		"}\n"
		"td:nth-child(7),td.holiday {\n"
		"color: red;\n"
		"}\n"
		"tr, td {\n"
		"border: 0 solid #DDD;\n"
		"}\n"
		"tr {\n"
		"border-top-width: 1px;\n"
		"}\n"
		"tr:last-child {\n"
		"border-bottom-width: 1px;\n"
		"}\n"
		"tr:first-child, tr.next, tr.next + tr {\n"
		"border-top-width: 0;\n"
		"}\n"
		"\n"
		"tr:first-child > td {\n"
		"height: unset;\n"
		"font-weight: bold;\n"
		"}\n"
		"td {\n"
		"vertical-align: top;\n"
		"width: 14vw;\n"
		"height: 5em;\n"
		"border-left-width: 1px;\n"
		"}\n"
		"td:last-child {\n"
		"border-right-width: 1px;\n"
		"}\n"
		"tr.next > td {\n"
		"border-top-width: 1px;\n"
		"border-bottom-width: 1px;\n"
		"border-bottom-color: #888;\n"
		"}\n"
		"tr.next > td.first {\n"
		"border-left-color: #888;\n"
		"}\n"
		"tr.next > td.first, tr.next > td.first ~ td {\n"
		"border-top-width: 1px;\n"
		"border-top-color: #888;\n"
		"border-bottom-width: 1px;\n"
		"border-bottom-color: #DDD;\n"
		"}\n"
		"\n"
		"ul {\n"
		"color: black;\n"
		"padding-left: 20px;\n"
		"font-size: 0.5em;\n"
		"}\n"
		"</style>\n"
		"<title>Calendar</title>\n"
		"<table>"
	);

	puts("<tr>");
	for(int i = 0; i < LEN(weekday_str); i++) {
		printf("<td>%s\n", weekday_str[(i + firstday) % LEN(weekday_str)]);
	}
	move_date(&date, -date.week_day + firstday - 7 * 1);
	movable_reminders(&date);
	for(int i = 0; i < 32; i++) {
		printf("<tr");
		if(date.day_of_month + 7 > get_month_len(&date)) {
			printf(" class=next");
		}
		printf(">\n");
		for(int j = 0; j < 7; j++) {
			unsigned class = 0;
			for(int r = 0; r < LEN(reminders); r++) {
				if(reminder_match_date(&reminders[r], &date)) {
					class |= reminders[r].class;
				}
			}
			if(date.day_of_month == 1) {
				class |= FIRST;
			}

			printf("<td");
			if(class != 0) {
				printf(" class=\"");
				for(int c = 0; c < LEN(class_str); c++) {
					if(class & (1 << c)) {
						printf(" %s", class_str[c]);
					}
				}
				printf("\"");
			}
			printf(">%d", date.day_of_month);
			if(date.day_of_month == 1) {
				printf(" %s\n", month_str[date.month-1]);
			} else {
				puts("");
			}

			bool rems = false;
			for(int r = 0; r < LEN(reminders); r++) {
				if(reminder_match_date(&reminders[r], &date)) {
					//printf("y %d m %d d %d w %d\n", reminders[r].year, reminders[r].month, reminders[r].day_of_month, reminders[r].week_day);
					if(!rems) {
						puts("<ul>");
						rems = true;
					}
					printf("\t<li>%s\n", reminders[r].name);
				}
			}
			if(rems) {
				puts("</ul>");
			}
			move_date(&date, 1);
		}
	}
	puts("</table>");

	return 0;
}
