#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>

/**
*  développeurs : HENRY Romain - LANG Jordan
*/

#define TAILLE_DG 1024
#define TAILLE_TETE 2
#define TAILLE_DATA 1022
#define MSEC 999999
#define SEC 0
#define TAILLE_FENETRE 4

struct datagramme {
	char num_seq;
	char num_ack;
	char message[TAILLE_DATA];
};

int main(int argc, char *argv[]) {
	int port_local;
	struct datagramme* dg_send = malloc(sizeof(struct datagramme));
	struct datagramme* dg_recv = malloc(sizeof(struct datagramme));
	char* adresse_dest = NULL;
	char* file_send = NULL;
	char* file_recv = NULL;
	struct sockaddr_in local;
	struct sockaddr_in expediteur;

	int eSeq2,send_next=0;

	// création de la socket
	int fd = socket(AF_INET,SOCK_DGRAM,0);
	if(errno != 0) {
	printf("Code de l'erreur : %d\n", errno);
	perror("Erreur");
	}

	//packaging
	if(argc < 4) {
		printf("Usage : ./serveur  <fichier_a_envoyer> <fichier_recu> <port_local>");
		exit(1);
	}
	file_send = argv[1];
	file_recv = argv[2];
	port_local = atoi(argv[3]);

	// création de la structure sockaddr_in
	local.sin_family = AF_INET;
	local.sin_port = htons(port_local);
	local.sin_addr.s_addr = htonl(INADDR_ANY);

	// on bind le socket créé avec la structure qui contient les paramètres
	bind(fd, (struct sockaddr*) &local, sizeof(local));
	if(errno != 0) {
		printf("Code de l'erreur : %d\n", errno);
		perror("Erreur");
	}

	//***************************************  DEBUT MEME CODE QUE CLIENT ***************************************

	// ouverture du fichier à envoyer en lecture
	int input_fd = open(file_send, O_RDONLY);
	if(input_fd<0) { 
		perror("open input"); 
		exit(1);     
	}

	// ouverture du fichier de sauvegarde en ecriture
	int output_fd = open(file_recv, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
	if(output_fd<0) { 
		perror("open output"); 
		exit(1);   
	}

	//initiation de l'envoi du fichier et reception d'un fichier
	socklen_t exp = sizeof(expediteur);
	int send_finish = 0, recv_finish = 0, finished = 0;
	int reSeq = 0, reAck = 0, eSeq = 1, eAck = 0, reSeqInter=-1, eSeqEnd=-1, last_eAck;
	int ack_lastsend =0;
	int rcvLost = 0;
	int send_new = 1;
	int nb_lu = 0, nb_carac = 0;
	int i, init = 1, k, firstsend = 1;
	char fenetre[TAILLE_FENETRE][TAILLE_DATA];
	char* stringInter;
	struct timeval timev;
	int nb_fds;
	int res_select, res_select2;
	fd_set rdfs;
	
	while(!finished) {
		nb_carac = 0;
		timev.tv_sec = SEC;
		timev.tv_usec = MSEC;
	    FD_ZERO(&rdfs);
	    FD_SET(fd, &rdfs);
		nb_fds = fd+1;
		
		send_next=0;
		send_new =1;
		
		if(init == 0) {
			printf("Start timer : %ld\n", timev.tv_sec);
			res_select = select(nb_fds, &rdfs, NULL, NULL, &timev);

		} else {
			res_select = 1;
			init = 0;
		}

		if (res_select == 0) 
		{
			printf("Timeout\n");
			send_new = 0;
		} 
		else 
		{
			
			printf("Stop timer\n");
			rcvLost = 0;
			res_select2=1;
			for(k =0;k<TAILLE_FENETRE && res_select2;k++)
			{
				timev.tv_sec = SEC;
				timev.tv_usec = MSEC/TAILLE_FENETRE;
			    FD_ZERO(&rdfs);
			    FD_SET(fd, &rdfs);
				nb_fds = fd+1;
				nb_carac = 0;
				
				nb_carac = recvfrom(fd, dg_recv, TAILLE_DG, 0, (struct sockaddr*) &expediteur, &exp);
				nb_carac -= TAILLE_TETE;

				printf("seq reçu : %c\n", dg_recv->num_seq);
				printf("ack reçu : %c\n\n", dg_recv->num_ack);

				reSeq = dg_recv->num_seq-'0';
				reAck = dg_recv->num_ack-'0';
				
				if(reSeq == TAILLE_FENETRE+1)
				{
					last_eAck = eAck;
					recv_finish = 1;
				}
				else
					recv_finish =0;


				if(reAck==TAILLE_FENETRE)
					send_new=1;
				else
					send_new =0;


				
				if(send_finish)
				{
					if(reAck==(eSeqEnd+1)%TAILLE_FENETRE && !ack_lastsend)
						ack_lastsend =1;
					else if (!ack_lastsend)
						send_finish=0;
				
				}

				if(!recv_finish && eAck==TAILLE_FENETRE +1)
				{
					eAck=last_eAck;
				}


				if(!send_finish && reAck !=TAILLE_FENETRE+1)
				{

					eSeq=reAck%TAILLE_FENETRE;
					if(eSeq!=eSeq2)
					{
						send_next= (int)fabs(eSeq-eSeq2);
						printf("send next : %d \n",send_next);
					}
				}
				else
					eSeq=eSeq2;

				if(reSeq != eAck%TAILLE_FENETRE && !rcvLost && !recv_finish)
				{
					
				}
				else
				{
					if(!recv_finish)
					{
						eAck=(reSeq+1)%TAILLE_FENETRE;
						if(k==TAILLE_FENETRE-1)
							eAck=TAILLE_FENETRE;
						printf("ecriture copytiti.txt : %s\n", dg_recv->message);
						write(output_fd, dg_recv->message, nb_carac);
					}
					else if(k==0)
					{
						eAck=TAILLE_FENETRE+1;
					}

				}

				if(k<TAILLE_FENETRE-1)
					res_select2 = select(nb_fds, &rdfs, NULL, NULL, &timev);
			}
			if(res_select2==0)
			{
				printf("why ?\n");
				send_new=0;
			}
		}

		printf("send : %d, recv : %d\n", send_finish, recv_finish);
		
		//si l'envoi et la réception ont terminés on arrête le programme
		if(send_finish && recv_finish) {
			finished = 1;
		}
		eSeq2= eSeq;
		for(k =eSeq2;k<eSeq2+TAILLE_FENETRE;k++)
		{ 
			if(!send_finish)
			{
				dg_send->num_seq=eSeq+'0';
				if(((k>=TAILLE_FENETRE-send_next+eSeq2 && send_next != 0) || send_new || firstsend))
				{
					if(firstsend && k==TAILLE_FENETRE-1)
						firstsend =0;
					printf("je suis la\n");
					nb_lu = read(input_fd, dg_send->message, TAILLE_DATA);

					printf("nb lu :%d\n",nb_lu);
					strcpy(fenetre[k%TAILLE_FENETRE],dg_send->message);
					printf("message : %s\n",dg_send->message);
					printf("message : %s\n",fenetre[k%TAILLE_FENETRE]);

					if(nb_lu < TAILLE_DATA) {
						eSeqEnd=eSeq;
						send_finish = 1;
					}
				}
				else
				{
					printf("je suis ici eSeqend : %d\n",eSeqEnd);
					strcpy(dg_send->message,fenetre[k%TAILLE_FENETRE]);
					if(k%TAILLE_FENETRE==eSeqEnd)
					{
						send_finish=1;
						send_new = 1;
					}
					printf("message : %s\n",dg_send->message);
				}
			}
			else if(send_new || firstsend)
			{
				if(firstsend && k==TAILLE_FENETRE-1)
						firstsend =0;
				dg_send->num_seq=TAILLE_FENETRE+'0'+1;
			}
				
			dg_send->num_ack=eAck+'0';

			printf("seq : %c\n", dg_send->num_seq);
			printf("ack : %c\n", dg_send->num_ack);

			// envoi du paquet
			nb_carac = sendto(fd, dg_send, nb_lu+TAILLE_TETE, 0, (struct sockaddr*) &expediteur, exp);
			if(k+1<eSeq2+TAILLE_FENETRE)
				eSeq = (eSeq+1)%TAILLE_FENETRE;
		}
		printf("eSeqEnd : %d\n",eSeq2);
	}

	free(dg_send);
	free(dg_recv);
	close(input_fd);
	close(output_fd);

	//***************************************  FIN MEME CODE QUE CLIENT ***************************************

	return EXIT_SUCCESS;
}