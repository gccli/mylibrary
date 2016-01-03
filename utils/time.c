#include "utils/time.h"

/**
 * cycle counter
 * cycle counter is a 64-bit, unsigned number.
 * IA32 counter is accessed with the rdtsc (for "read time stamp counter") instruction
 */
void cc_query(unsigned int *hi, unsigned int *lo)
{
  asm("rdtsc; movl %%edx, %0; movl %%eax, %1"
    : "=r" (*hi), "=r" (*lo)
    :
  : "%edx", "%eax");
}

double cc_gettime(unsigned int hi, unsigned int lo)
{
  double number_of_cycle;
  unsigned int cyc_hi, cyc_lo;
  unsigned int high, low, borrow;

  cc_query(&cyc_hi, &cyc_lo);

  low  = cyc_lo - lo;
  borrow = low > cyc_lo;
  high = cyc_hi - hi - borrow;

  number_of_cycle = (double)high*(1<<30)*4+low;
  if (number_of_cycle < 0.001) {
    fprintf (stderr, "Error: counter returns net value: %.0f\n", number_of_cycle);
  }

  return number_of_cycle;
}

double ghz(int sleeptime)
{
  double i_number, rate;
  unsigned int high, low;
  cc_query(&high, &low);
  sleep(sleeptime);
  i_number = cc_gettime(high, low);
  rate = i_number/(1e9*sleeptime);
  printf ("The system processor clock rate: %.3f GHz\n", rate);

  return rate;
}

#ifdef _UT_TEST
int main(int argc, char *argv[])
{
    time_t now = time(NULL);
    time_t basetime = get_0_clock(now);
    char tmpstr[DATE_STR_SIZE];

    ghz(1);
    printf("basetime  %s\n", to_datetime(basetime, tmpstr));
    printf("datetime  %s\n", to_datetime(now, tmpstr));
    printf("datetime  %s\n", to_datetime_noyear(now, tmpstr));
    printf("iso8601   %s\n", to_iso_datetime(now, tmpstr));
    printf("date      %s\n", to_date(now, tmpstr));
    printf("date      %s\n", to_date_noyear(now, tmpstr));
    printf("time      %s\n", to_time(now, tmpstr));

    return 0;
}
#endif
