
/*======================================================*\
  Monday October the 1st 2012
  Arash HABIBI
  timer.c
\*======================================================*/

#include "timer.h"

static struct timeval _start;

//----------------------------------------------------------

void T_init(void)
{
	gettimeofday(&_start,NULL);
}

//----------------------------------------------------------

double T_substract(struct timeval t1, struct timeval t2)
{
	double diff;
	diff = t1.tv_sec - t2.tv_sec;
	diff += (1e-6)*(t1.tv_usec - t2.tv_usec);
	return diff;
}

//----------------------------------------------------------

double T_get(void)
{
	struct timeval current;
	gettimeofday(&current,NULL);
	return T_substract(current,_start);
}

//----------------------------------------------------------

struct timeval T_timeval(double duree)
{
	struct timeval res;

	double duree_sec = floor(duree);
	res.tv_sec  = (long)duree_sec;
	res.tv_usec = (long)(1000000*(duree - duree_sec));
	return res;
}


