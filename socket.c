
/*======================================================*\
  Monday October the 1st 2012
  Arash HABIBI
  socket.c
\*======================================================*/

#include "socket.h"

int S_DOMAIN = AF_INET;

//--------------------------------------------------------

int S_openAndBindSocket(int port)
{
	int connected=0;
	int sock_fd = socket( S_DOMAIN, S_SOCKTYPE, S_PROTOCOL);
	if(sock_fd < 0)
		perror("S_openAndBindSocket : socket");

	else
	{
		socklen_t addrlen;

		struct sockaddr *local_addr;
		struct sockaddr_in  local_addr4;
		struct sockaddr_in6 local_addr6;

		if(S_DOMAIN == AF_INET) // IPv4
		{
			local_addr4.sin_family = AF_INET;
			local_addr4.sin_port   = htons(port);
			local_addr4.sin_addr.s_addr  = INADDR_ANY;
			addrlen = sizeof(struct sockaddr_in);
			local_addr = (struct sockaddr*)&local_addr4;
		}
		else if(S_DOMAIN == AF_INET6) // IPv6
		{
			local_addr6.sin6_family = AF_INET6;
			// local_addr6.sin6_len    = sizeof(struct sockaddr_in6);
			local_addr6.sin6_port   = htons(port);
			local_addr6.sin6_addr   = in6addr_any;
			addrlen = sizeof(struct sockaddr_in6);
			local_addr = (struct sockaddr*)&local_addr6;
		}
		else
			printf("S_DOMAIN=%d, c'est à dire ni AF_INET=%d ni AF_INET6=%d\n",S_DOMAIN,AF_INET,AF_INET6);

		int res = bind(sock_fd, local_addr, addrlen);
		if(res<0)
			perror("S_openAndBindSocket : bind");

		else
			connected = 1;
	}
	if(!connected)
	{
		close(sock_fd);
		return -1;
	}
	else
		return sock_fd;
}

//--------------------------------------------------------

int S_openSocket(void)
{
	int sock_fd;
	sock_fd = socket(S_DOMAIN, S_SOCKTYPE, S_PROTOCOL);

	if(sock_fd<0)
	{	perror("client socket"); exit(1);	}

	return sock_fd;
}

//--------------------------------------------------------

int S_distantAddress(char *IP_address, int port, struct sockaddr **dist_addr)
{
	int res;

	if(S_DOMAIN == AF_INET) // IPv4
	{
		struct sockaddr_in  *dist_addr4 = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));

		dist_addr4->sin_family = AF_INET;
		dist_addr4->sin_port   = htons(port);
		res = inet_pton(AF_INET, IP_address, &dist_addr4->sin_addr);
		*dist_addr = (struct sockaddr*)dist_addr4;
	}
	else if(S_DOMAIN == AF_INET6) // IPv6
	{
		struct sockaddr_in6 *dist_addr6 = (struct sockaddr_in6*)malloc(sizeof(struct sockaddr_in6));
		dist_addr6->sin6_family = AF_INET6;
		dist_addr6->sin6_port   = htons(port);
		res = inet_pton(AF_INET6, IP_address, &dist_addr6->sin6_addr);
		*dist_addr = (struct sockaddr*)dist_addr6;
	}
	return(res);
}

//--------------------------------------------------------

int S_sameAddress(struct sockaddr *address1, struct sockaddr *address2)
{
  if(S_DOMAIN==AF_INET)
    {
      struct sockaddr_in *adr1, *adr2;
      adr1 = (struct sockaddr_in*)address1;
      adr2 = (struct sockaddr_in*)address2;
      return ( (adr1->sin_addr.s_addr == adr2->sin_addr.s_addr) &&
	       (adr1->sin_port        == adr2->sin_port) );
      // fprintf(stderr,"%u -------------- %u \n",ntohs(adr1->sin_port),ntohs(adr2->sin_port));
      // fprintf(stderr,"%u -------------- %u \n",adr1->sin_port,adr2->sin_port);
      // return (adr1->sin_port == adr2->sin_port);
    }
  else
    {
      int same = 1;
      struct sockaddr_in6 *adr1, *adr2;
      adr1 = (struct sockaddr_in6*)address1;
      adr2 = (struct sockaddr_in6*)address2;
      if(adr1->sin6_port == adr2->sin6_port)
	{
	  int i;
	  for(i=0;i<16;i++)
	    if (adr1->sin6_addr.s6_addr[i] != adr2->sin6_addr.s6_addr[i])
	      same=0;
	}
      else
	same = 0;

      return same;
    }
}

//--------------------------------------------------------

void S_humanReadableAddress(struct sockaddr *address, char *ip_address, int *port)
{
  if(S_DOMAIN==AF_INET)
    {
      struct sockaddr_in adr;
      memcpy(&adr, address, sizeof(struct sockaddr_in));
      inet_ntop(AF_INET, &adr.sin_addr, ip_address, S_NAMES);
      *port = ntohs(adr.sin_port);
    }
  else   if(S_DOMAIN==AF_INET6)
    {
      struct sockaddr_in6 adr;
      memcpy(&adr, address, sizeof(struct sockaddr_in6));
      inet_ntop(AF_INET6, &adr.sin6_addr, ip_address, S_NAMES);
      *port = ntohs(adr.sin6_port);
    }
}

//--------------------------------------------------------

int S_sendMessage(int sock_fd, struct sockaddr *adr, char *message, int expected_length)
{
	int actual_message_length=0;

	socklen_t sin_size;
	if(S_DOMAIN == AF_INET) sin_size = sizeof(struct sockaddr_in);
	else 					sin_size = sizeof(struct sockaddr_in6);

	actual_message_length = sendto(sock_fd, message, expected_length, 0, adr, sin_size);
	if(actual_message_length<0)
	{	perror("sendto"); exit(1); 		}

	int debug=0;
	if(debug)
	{
		char real_hostname[100];
		char real_service[100];
		socklen_t l;
		if(S_DOMAIN == AF_INET) l = sizeof(struct sockaddr_in);
		else					l = sizeof(struct sockaddr_in6);
		// getnameinfo(adr, l, real_hostname,100, real_service, 100, NI_NUMERICHOST);
		getnameinfo(adr, l, real_hostname,100, real_service, 100, 0);
		printf("Message sent to host %s port %s\n", real_hostname, real_service);
	}
	return actual_message_length;
}

//----------------------------------------------------------------

int S_receiveMessage(int sock_fd, struct sockaddr *source, char *message, int expected_length)
{
	int actual_message_length=0;

	socklen_t sin_size;
	if(S_DOMAIN == AF_INET)	sin_size = sizeof(struct sockaddr_in);
	else					sin_size = sizeof(struct sockaddr_in6);

	actual_message_length = recvfrom(sock_fd, message, expected_length, 0, source, &sin_size);

	if(actual_message_length<0)
	{	perror("recvfrom"); exit(1); }

	int debug=0;
	if(debug)
	{
		char real_hostname[100];
		char real_service[100];
		getnameinfo(source, sin_size, real_hostname,100, real_service, 100, 0);
		printf("Message received from host %s port %s\n", real_hostname, real_service);
	}

	return actual_message_length;
}


