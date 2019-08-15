#if !defined(TRUE)
  #define FALSE 0
  #define TRUE  !FALSE
#endif

long gm_refday  (            int year,   int month,   int day);
void gm_refdayr (long refday,int *year_p,int *month_p,int *day_p);
int  gm_day_y0  (            int year,   int month,   int day);
void gm_day_y0r (int  y0_day,int *year_p,int *month_p,int *day_p);
long gm_day_e2  (            int year,   int month,   int day);
void gm_day_e2r (long e2_day,int *year_p,int *month_p,int *day_p);

