
/*======================================================*\
  Monday October the 1st 2012
  Arash HABIBI
  timer.h
\*======================================================*/

#ifndef __TIMER_H__
#define __TIMER_H__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <sys/time.h>

void   T_init(void);
double T_get(void);
double T_substract(struct timeval t1, struct timeval t2);
struct timeval T_timeval(double duree);

#endif

