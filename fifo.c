
/*======================================================*\
  Monday October the 1st 2012
  Arash HABIBI
  fifo.c

  typedef struct ff
  {
  char _message[S_MAX_MESSAGE];
  struct sockaddr *_destination;
  struct ff *_next;
  } fifo;

\*======================================================*/

#include "fifo.h"
#include "socket.h" // seulement pour #define S_MAX_MESSAGE 1024


//----------------------------------------------------------

fifo* FF_new(void)
{
	return NULL;
}
//----------------------------------------------------------

int FF_isEmpty(fifo *ff)
{
	return (ff==NULL);
}

//----------------------------------------------------------
// Introduit un nouvel élément en queue de la file.

fifo* FF_append(fifo *ff, char *message, int id, int length, double arrival_time, struct sockaddr *destination)
{
	fifo *newElt=(fifo*)malloc(sizeof(fifo));

	if(newElt==NULL)
		fprintf(stderr,"FF_append : malloc failed.\n");

	else
	{
		newElt->_message = (char*)malloc(length+1);
		strncpy(newElt->_message, message, length);
		newElt->_id = id;
		newElt->_length = length;
		newElt->_arrival_time = arrival_time;
		newElt->_destination = destination;

		if(FF_isEmpty(ff))
			newElt->_next = newElt;

		else
		{
			fifo *first = ff->_next;
			ff->_next = newElt;
			newElt->_next = first;
		}
	}
	return newElt;
}

//----------------------------------------------------------
// Enlève l'élément qui est en tete de la liste et donne sa
// valeur.

fifo* FF_behead(fifo *ff)
{
	if(FF_isEmpty(ff))
		printf("FF_behead : empty fifo.\n");

	else
	{
		fifo *first = ff->_next;
		if(ff->_next!=ff) // plus d'un élément
		{
			fifo *second=first->_next;
			ff->_next = second;
		}
		else // un seul element
			ff = NULL;

		free(first->_message);
		free(first);
	}
	return ff;
}

//----------------------------------------------------------

int FF_head(fifo *ff, char *message, int *id, int *length, double *arrival_time, struct sockaddr **destination)
{
	if(FF_isEmpty(ff))
	{
		printf("FF_head : empty fifo.\n");
		return -1;
	}
	else
	{
		fifo *head = ff->_next;
		strcpy(message,head->_message);
		*length = head->_length;
		*id = head->_id;
		*arrival_time = head->_arrival_time;
		*destination = head->_destination;
	}
	return 0;
}

//----------------------------------------------------------

void FF_print(fifo *ff, char *message)
{
	fprintf(stderr,"-------------- check fifo (%s) ---------------\n",message);
	fifo *f;
	char host[S_NAMES], port[S_NAMES];

	if(FF_isEmpty(ff))
		fprintf(stderr,"Empty fifo.\n");

	else
	{
		int finished=0;
		int nb_elems=0;
		socklen_t sinsize = sizeof(struct sockaddr_in);

		f=ff->_next;
		while(!finished)
		{
			int diag=getnameinfo(f->_destination, sinsize,
								 host, S_NAMES, port, S_NAMES, 0);
			if(diag<0)
				fprintf(stderr,"getnameinfo : %s\n",gai_strerror(diag));

			fprintf(stderr,"\x1b[34mDatagram number %d : \x1b[37m\t arrived at time %f, Destination host : %s\t port %s \t message (%d) : %s\n",
					nb_elems, f->_arrival_time, host, port, f->_length, f->_message);

			if(f==ff) finished=1;
			else f=f->_next;
			nb_elems++;
		}
		printf("Total : %d elements.\n",nb_elems);
	}
}

//----------------------------------------------------------

