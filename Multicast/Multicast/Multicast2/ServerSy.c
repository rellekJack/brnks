#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <ctype.h>

#include "data.h"

struct request req;
/*Deklaration socket descripor*/
SOCKET ConnSocket;
/*Deklaration socket address local static*/
static struct sockaddr_in6 localaddr;
//struct in6_addr anyaddr = IN6ADDR_ANY_INIT;	 //aus beleg
struct sockaddr *sockaddr_ip6_local=NULL;
/*Deklaration socket address remote static*/
static struct sockaddr_in6 remoteaddr;
//int remoteaddrlen=sizeof(remoteaddr);		 //aus beleg
//static struct addrinfo info;				 //aus beleg

int initServer(char *MCAddress, char *Port)
{
		int truevalue = 1, loopback = 0;
		int val, i=0;
		int addr_len;
		struct ipv6_mreq mreq;
		struct addrinfo *resultlocaladdress = NULL, *resultmulticastaddress = NULL, *ptr = NULL, hints;

	WSADATA wsadata;
	WORD wversionrequested;

	wversionrequested = MAKEWORD(2,1);
	if( WSAStartup( wversionrequested,&wsadata) == SOCKET_ERROR)
	{
		printf("Server: WSAStartup() failed! \n");
		printf("        error code:  %d\n",WSAGetLastError());
		exit(-1);
	}
	/*create socket*/
	ConnSocket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);

	if(ConnSocket < 0)
	{
		fprintf(stderr, "Client: Error Opening Socket: Error: %d\n", WSAGetLastError());
		WSACleanup();
		exit(-1);
	}

	//memset( &localaddr,0,sizeof(localaddr));		//aus beleg
	//memset( &remoteaddr,0, remoteaddrlen);			//aus beleg

	/*intialize socket*/
	setsockopt(ConnSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&truevalue, sizeof(truevalue));

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family	= AF_INET6;
	hints.ai_flags = AI_NUMERICHOST;
	if( getaddrinfo(MCAddress, NULL, &hints, &resultmulticastaddress) != 0)
	{
		fprintf(stderr, "getaddrinfo MCAdress failed with error: %d\n", WSAGetLastError());
		WSACleanup();
		exit(-1);
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family	= AF_INET6;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;

	hints.ai_flags = AI_PASSIVE;

	val = getaddrinfo(NULL, Port, &hints, &resultlocaladdress);

	if( val != 0)
	{
		printf("getaddrinfo LocalAdress failed with error: %d\n", val);
		WSACleanup();
		exit(-1);
	}

	for(ptr = resultlocaladdress; ptr != NULL; ptr=ptr->ai_next)
	{
		printf("getaddrinfo response %d\n", i++);
		printf("\tFlags: 0x%x\n", ptr->ai_flags);
		printf("\tFamily: ");
		switch (ptr->ai_family)
		{
			case AF_UNSPEC:
				printf("Unspecified\n");
				break;
			case AF_INET:
				printf("AF_INET (IPv4)\n");
				break;
			case AF_INET6:
				printf("AF_INET6 (IPv6)\n");

				sockaddr_ip6_local = (struct sockaddr *) ptr->ai_addr;
				addr_len = ptr->ai_addrlen;
				break;
			default:
				printf("Other %ld\n", ptr->ai_family);
				break;
		}

	}

	printf("in bind\n");

	if (bind(ConnSocket, sockaddr_ip6_local, addr_len) == SOCKET_ERROR)
	{
		fprintf(stderr, "bind() failed: error %d\n", WSAGetLastError());
		WSACleanup();
		exit(-1);
	}

	memcpy(&mreq.ipv6mr_multiaddr, &((struct sockaddr_in6*)(resultmulticastaddress->ai_addr))->sin6_addr, sizeof(mreq.ipv6mr_multiaddr));

	mreq.ipv6mr_interface = 3;

	if( setsockopt(ConnSocket, IPPROTO_IPV6, IPV6_JOIN_GROUP, (char*) &mreq, sizeof(mreq)) != 0)
	{
		fprintf(stderr,"setsockopt(IPV6_JOIN_GROUP) failed %d\n", WSAGetLastError());
		WSACleanup();
		exit(-1);
	}

	freeaddrinfo(resultlocaladdress);
	freeaddrinfo(resultmulticastaddress);
	return(0);
	}

struct request *getrequest()
	{
		static long seq_number = 0;
		int recvcc;
		int remoteaddrsize = sizeof(struct sockaddr_in6);

		recvcc = recvfrom(ConnSocket, (char *)&req, sizeof(req), 0, (struct sockaddr *) &remoteaddr, &remoteaddrsize);

			if(recvcc = SOCKET_ERROR)
			{
				fprintf(stderr,"recv() failed: error %d\n", WSAGetLastError());
				closesocket(ConnSocket);
				WSACleanup();
				exit(-1);
			}
		return(&req);
	}

void sendanswer(struct answer *answ)
	{
		int w;

		w = sendto(ConnSocket, (const char *)answ, sizeof(*answ), 0, (struct sockaddr *)&remoteaddr, sizeof(remoteaddr));
		if ( w == SOCKET_ERROR)
		{
			fprintf(stderr,"send() failed: error &d\n", WSAGetLastError());
		}
	}

int exitServer()
	{
		closesocket(ConnSocket);
		printf("in exit Server\n");
		
		if(WSACleanup()==SOCKET_ERROR)
		{
			printf("Server: WSACleanup() failed!\n");
			printf("        error code:  %d\n", WSAGetLastError());
			exit(-1);
		}
		return(0);
	}
	





