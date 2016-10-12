
/*======================================================*\
  Monday October the 1st 2012
  Arash HABIBI
  fifo.h
\*======================================================*/

#ifndef __FIFO_H__
#define __FIFO_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "socket.h"

typedef struct ff
{
	char *_message;
	int _id;
	int _length;
	double _arrival_time;
	struct sockaddr *_destination;
	struct ff *_next;
} fifo;

fifo* FF_new(void);

int FF_isEmpty(fifo *ff);
fifo* FF_append(fifo *ff, char *message, int id, int  length, double  arrival_time, struct sockaddr *destination);
fifo* FF_behead(fifo *ff);
int   FF_head (fifo *ff, char *message, int *id, int *length, double *arrival_time, struct sockaddr **destination);

void FF_print(fifo *ff, char *message);


#endif

