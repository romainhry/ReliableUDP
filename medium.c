
/*======================================================*\
  Monday October the 1st 2012
  Arash HABIBI
  medium.c
\*======================================================*/

#include "socket.h"
#include "fifo.h"
#include "timer.h"

extern int S_DOMAIN;

//----------------------------------------------------------------

int main(int argc, char **argv)
{
	if ( argc!=9 )
	{
		printf("\nUsage \t : %s <IPversion> <local_port> <host1> <port1> <host2> <port2> <error_rate> <delay>\n\n",argv[0]);
		return 1;
	}

	// printf("in=%d in6=%d x=%d\n",sizeof(struct sockaddr_in), sizeof(struct sockaddr_in6), sizeof(struct sockaddr));
	// return 0;

	//---------------- exploitation des arguments ---------------

	int local_port;
	local_port = atoi(argv[2]);

	int version = atoi(argv[1]);
	if(version==4)	S_DOMAIN = AF_INET;
	else 			S_DOMAIN = AF_INET6;

	int distant_port1, distant_port2;
	distant_port1 = atoi(argv[4]);
	distant_port2 = atoi(argv[6]);

	char distant_host1[S_NAMES], distant_host2[S_NAMES];
	strcpy(distant_host1,argv[3]);
	strcpy(distant_host2,argv[5]);

	float error_rate = atof(argv[7]);
	if(error_rate<0) error_rate = 0;
	else if(error_rate>1) error_rate = 1;

	float delay = atof(argv[8]);

	printf("Simulated medium between host %s port %d", distant_host1, distant_port1);
	printf(" and host %s port %d\n", distant_host2, distant_port2);
	printf("buffer_length \t: %d\n", S_MAX_MESSAGE);
	printf("error rate \t: %f\n",error_rate);
	printf("delay \t\t: %f\n",delay);

	//---------------- creation et attachement de socket --------

	int server_sock_fd;
	server_sock_fd = S_openAndBindSocket(local_port);

	//------------- Trouver l'adresse de destination ------------

	struct sockaddr *distant_address1, *distant_address2;
	S_distantAddress(distant_host1, distant_port1, &distant_address1);
	S_distantAddress(distant_host2, distant_port2, &distant_address2);

	//--------- Boucle de traitement des evenements -------------

	char message[S_MAX_MESSAGE];
	memset(message,0,S_MAX_MESSAGE);

	int diag=0, pack_nb_from_host1=0, pack_nb_from_host2=0, packet_number;
	struct timeval duree_timeout = T_timeval(0.0);
	fifo *window=FF_new();
	T_init();

	fd_set readset;

	while(1)
	{
		int debug = 1;
		FD_ZERO(&readset);
		FD_SET(server_sock_fd, &readset);
		int nfds = server_sock_fd+1;

		float now = T_get(); // now est le nombre de secondes depuis le lancement du programme

		int res;
		if(FF_isEmpty(window))	res = select(nfds, &readset, NULL, NULL, NULL);
		else					res = select(nfds, &readset, NULL, NULL, &duree_timeout);

		if(res<0)
		{	perror("select"); exit(1); 	}

		if(FD_ISSET(server_sock_fd,&readset)) // arrivee d'un message
		{
			// --------- reception ---------------

			struct sockaddr *destination_address, *source_address;
			struct sockaddr_in source4;
			struct sockaddr_in6 source6;

			if(S_DOMAIN==AF_INET)	source_address = (struct sockaddr*)&source4;
			else					source_address = (struct sockaddr*)&source6;

			int nb_bytes_received = S_receiveMessage(server_sock_fd, source_address, message, S_MAX_MESSAGE);
			if(nb_bytes_received<0)
			{	perror("received");	exit(1); }

			// --------- identification de l'expéditeur -------------

			// int known_host=1, from_one_to_two=0;
			int known_host=1;

			if(S_sameAddress(source_address, distant_address1))
			{
				destination_address = distant_address2;
				pack_nb_from_host1++;
				packet_number = pack_nb_from_host1;
				// from_one_to_two = 1;
			}
			else if(S_sameAddress(source_address, distant_address2))
			{
				destination_address = distant_address1;
				pack_nb_from_host2++;
				packet_number = pack_nb_from_host2;
				// from_one_to_two = 0;
			}
			else known_host = 0;

			if(known_host)
			{
				message[nb_bytes_received]='\0';

				if(debug)
				{
					char actual_host[S_NAMES], actual_port[S_NAMES];
					size_t adr_length;
					if(S_DOMAIN==AF_INET) adr_length = sizeof(struct sockaddr_in);
					else                  adr_length = sizeof(struct sockaddr_in6);
					getnameinfo((struct sockaddr *)source_address, adr_length,
								actual_host,S_NAMES,actual_port,S_NAMES,0);
					printf("\x1b[34mMedium : Received message number %d (%d bytes) coming from host %s port %s :\x1b[37m \n%s\n",
						   packet_number, nb_bytes_received, actual_host, actual_port, message);
				}

				// ---- Decision : retransmission ou perte ? ----------------

				float alea = drand48();
				if(alea > error_rate)
				{
					// Mettre le message dans la file d'attente pour la retransmission
					window = FF_append(window, message, packet_number, nb_bytes_received, now, destination_address);
				}
				else
					if(debug)
						printf("\x1b[34mMedium : Lost message number %d.\x1b[37m\n", packet_number);
			}
			else
			  {
			    int port;
			    char adresse_ip[S_NAMES];
			    S_humanReadableAddress(source_address, adresse_ip, &port);
			    printf("%s port %d : Unknown host\n",adresse_ip,port);
			  }
		}

		// ---- Retransmission de tous les messages échus -----------------

		int id, length, finished=0;
		double arrival_time, departure_time;
		struct sockaddr *destination_address;

		while(!finished)
		{
			if(FF_isEmpty(window)) finished=1;
			else
			{
				FF_head(window, message, &id, &length, &arrival_time, &destination_address);

				departure_time = arrival_time + delay;
				if(departure_time > now)
				{
					finished=1;
					duree_timeout = T_timeval(departure_time - now);
				}
				else // departure_time < now : le message aurait du partir
				{
					diag=S_sendMessage(server_sock_fd, destination_address, message, length);
					if(diag<0)
					{	perror("sendto");	exit(1);	}

					else
						if(debug)
						{
							char actual_host[S_NAMES], actual_port[S_NAMES];
							getnameinfo((struct sockaddr *)destination_address, sizeof(struct sockaddr_in),
										actual_host,S_NAMES,actual_port,S_NAMES,0);
							printf("\x1b[34mMedium : Sent message number %d (%d bytes) to host %s port %s :\x1b[37m \n%s\n",
								   id, length, actual_host, actual_port, message);
						}
					window = FF_behead(window);  // On enlève le premier élément
				}
			}
		}
	}

	close(server_sock_fd);

	return 0;
}
