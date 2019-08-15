#include "gmday.h"
/* In this time file, years months and days are those conventionally
 * used: January is month 1, the first day of the month is day 1 etc
 *
 * In the year0 routines,  the first day of the year            is day 0.
 * In the ep2000 routines, the first day of the third millenium is day 0.
 * The inverse routines work back to March in the Year 1.
 */
static int monthly_excess[12]
  ={306,337,  0, 31, 61, 92,122,153,184,214,245,275};

long gm_refday(int year,int month,int day)
/* converts a date in ymd specification to a number with
 * the property that consecutive days in the Gregorian 
 * calender are consecutive numbers. Also the number mod 7
 * is the day of the week (0=Sun...6=Sat). The result is 
 * unreliable if improper values for y m d are given.
 */
{
if(month<3) --year;
return ((long)(day + monthly_excess[--month])
            + 365L*year + year/4L - year/100L + year/400L + 2L);
}

void gm_refdayr (long refday, int *year_p,int *month_p,int *day_p)
{
int n; long residual=refday; long y=0,m=0,d=0;

while (residual>366)
    {y += residual/366L;residual = refday - (365*y + y/4 -y/100 + y/400) -2;}
if (residual>365 && (((y+1)%4 != 0) || ((y+1)%100 == 0)) && ((y+1)%400 != 0))
    {residual -= 365;y++;}
for (n=0;n<12;n++) {m=11-(n+10)%12; if (residual>monthly_excess[m]) break;}

d = residual-monthly_excess[m]; ++m<3?y++:y;
*year_p = y; *month_p = m; *day_p = d;
}

int gm_day_y0(int year,int month,int day)
/* return day of year : 1st January is Day 0. */
{return (gm_refday(year,month,day) - gm_refday(year,1,1));}

void gm_day_y0r (int y0_day, int *year_p,int *month_p,int *day_p)
{
long refday;
refday = y0_day + gm_refday(*year_p,1,1);
gm_refdayr (refday, year_p, month_p, day_p);
}

long gm_day_e2(int year,int month,int day)
/* return day relative to the year 2000. 1st January 2000 is Day 0. */
{return (gm_refday(year,month,day) - gm_refday(2000,1,1));}

void gm_day_e2r (long e2_day, int *year_p,int *month_p,int *day_p)
{
long refday;
refday = e2_day + gm_refday(2000,1,1);
gm_refdayr (refday, year_p, month_p, day_p);
}

