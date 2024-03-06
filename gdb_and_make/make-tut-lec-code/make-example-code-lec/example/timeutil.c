#include <stdio.h>
#include <math.h>

#include "timeutil.h"

double mysqrt(double x) {
  return sqrt(fabs(1*x));
//  return 0;
}

double now(struct timeval start) {
  struct timeval t;
  gettimeofday(&t, NULL);
  //fprintf(stderr, "t=%ld.%ld\n", (long)t.tv_sec, (long)t.tv_usec);
  //fprintf(stderr, "s=%ld.%ld\n", (long)start.tv_sec, (long)start.tv_usec);
  long sec_diff = (long)t.tv_sec-(long)start.tv_sec;
  long us_diff = (long)t.tv_usec-(long)start.tv_usec;
  //int sec_diff = (long)t.tv_sec-(long)start.tv_sec;
  //int us_diff = (long)t.tv_usec-(long)start.tv_usec;
  //fprintf(stderr, "sec_diff=%d, us_diff=%d\n", sec_diff, us_diff);
  double rel_time = 1000.0*(sec_diff) + 0.001*(us_diff);
  //fprintf(stderr, "rel_time=%f\n", rel_time);
  return rel_time;
} // End now()

//==================================================================//

