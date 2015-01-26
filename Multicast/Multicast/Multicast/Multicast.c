
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>      /* for printf() and fprintf() */
#include <winsock2.h>   /* for socket(), connect(), sendto(), and recvfrom() */
#include <ws2tcpip.h>   /* for ip_mreq */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <time.h>       /* for timestamps */

#include "ServerSy.h"
#include "data.h"


#if defined(_MSC_VER)
#pragma comment(lib, "ws2_32.lib")
#endif


void DieWithError(char* errorMessage)
{
    fprintf(stderr, "%s: %d\n", errorMessage, WSAGetLastError());
	getchar();
    exit(EXIT_FAILURE);
}

void createfile(struct request *reqPtr, int *init_file_create)
{
	FILE *fp;

	if ((fp = fopen(reqPtr->fname, "w+t")) == 0)  // create file
	{
		printf("\nFehler: konnte Datei %s nicht oeffnen\a\n", reqPtr->fname); exit(0);
	}
	(*init_file_create) = 1;			// indicate that file was created
	fclose(fp);
}

int writeFile(char temp[][256], struct request *reqPtr)
{
	FILE *fp;
	int i, j;

	if ((fp = fopen(reqPtr->fname, "a+t")) == 0)		// open file
	{
		printf("\nFehler: konnte Datei %s nicht oeffnen\a\n", reqPtr->fname); exit(0);
	}
	for (j = 0; (j < 100); j++)			// print buffer into file
	{
		for (i = 0; (i < 256) /*&& (lol[j][i] != '\0')*/; i++)
		{
			if (temp[j][i] != '\0')
				fputc(temp[j][i], fp);
			else						//stop if end of file ie reached
			{
				fputc(temp[j][i], fp);
				fclose(fp);
				return 1;
			}
		}
	}
	fclose(fp);
	return 0;
}

struct answer *answreturn(struct request *reqPtr, int *sqnr_counter, int *window_size, int *drop_pack_sqnr)
{
	static struct answer   answ;

	if ((reqPtr->FlNr) == 0)
	{
		answ.AnswType = AnswErr;
		answ.ErrNo = 4;
		return(&answ);
	}
	switch (reqPtr->ReqType)
	{
	case ReqHello:
		answ.AnswType = AnswHello;
		answ.FlNr = reqPtr->FlNr;
		answ.SeNo = reqPtr->SeNr;
		(*window_size) = reqPtr->FlNr;
		return(&answ);
		break;
	case ReqData:
		if ((reqPtr->SeNr != 0))
		{
			if ((reqPtr->SeNr>0) && (reqPtr->name>0) && ((reqPtr->SeNr == (*sqnr_counter)) || (reqPtr->SeNr == (*drop_pack_sqnr))))	// Sqn Nr > 0
			{
				answ.AnswType = AnswOk;
				answ.FlNr = reqPtr->FlNr;
				answ.SeNo = reqPtr->SeNr;
				(*sqnr_counter)++;
				if (reqPtr->SeNr == (*drop_pack_sqnr))
					(*sqnr_counter)--;
				return(&answ);
			}
			else										// Sqn Nr < 0
			{
				answ.AnswType = AnswErr;
				answ.ErrNo = 3;							// Illegal Sqn Number
				return(&answ);
			}
		}
		else											// Sqn Nr = 0
		{
			answ.AnswType = AnswWarn;
			answ.ErrNo = 1;								// Sequence Nr not available
			return(&answ);
		}
	case ReqClose:
		if (reqPtr->SeNr != 0)
		{
			if ((reqPtr->SeNr > 0) /*&& (reqPtr->name > 0)*/ && ((reqPtr->SeNr == (*sqnr_counter)) || (reqPtr->SeNr == (*drop_pack_sqnr))))	// Sqn Nr > 0
			{
				answ.AnswType = AnswClose;
				answ.FlNr = reqPtr->FlNr;
				answ.SeNo = reqPtr->SeNr;
				(*sqnr_counter)++;
				if (reqPtr->SeNr == (*drop_pack_sqnr))
					(*sqnr_counter)--;
				return(&answ);
			}
			else										// Sqn Nr < 0
			{
				answ.AnswType = AnswErr;
				answ.ErrNo = 3;							// Illegal Sqn Number
				return(&answ);
			}
		}
		else											// Sqn Nr = 0
		{
			answ.AnswType = AnswWarn;
			answ.ErrNo = 1;								// Sequence Nr not available
			return(&answ);
		}
	default:
		answ.AnswType = AnswErr;
		answ.ErrNo = 4;									// Illegal Request
		return(&answ);
	}
	answ.AnswType = AnswErr;
	answ.ErrNo = 5;											// Server Internal Error
	return (&answ);
}

int main(int argc, char* argv[])
{
    SOCKET     sock;                     /* Socket */
    WSADATA    wsaData;                  /* For WSAStartup */
    char*      multicastIP;              /* Arg: IP Multicast Address */
    char*      multicastPort;            /* Arg: Port */
	char*	   id;
	
    ADDRINFO*  multicastAddr;            /* Multicast Address */
    ADDRINFO*  localAddr;                /* Local address to bind to */
    ADDRINFO   hints          = { 0 };   /* Hints for name lookup */

    if ( WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        DieWithError("WSAStartup() failed");
    }

    if ( argc != 4 )
    {
        fprintf(stderr,"Usage: %s <Multicast IP> <Multicast Port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    multicastIP   = argv[1];      /* First arg:  Multicast IP address */
    multicastPort = argv[2];      /* Second arg: Multicast port */
	id = argv[3];
	

    /* Resolve the multicast group address */
    hints.ai_family = PF_UNSPEC;
    hints.ai_flags  = AI_NUMERICHOST;
    if ( getaddrinfo(multicastIP, NULL, &hints, &multicastAddr) != 0 )
    {
        DieWithError("getaddrinfo() failed");
    }

    printf("Using %s\n", multicastAddr->ai_family == PF_INET6 ? "IPv6" : "IPv4");

    /* Get a local address with the same family (IPv4 or IPv6) as our multicast group */
    hints.ai_family   = multicastAddr->ai_family;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags    = AI_PASSIVE; /* Return an address we can bind to */
    if ( getaddrinfo(NULL, multicastPort, &hints, &localAddr) != 0 )
    {
        DieWithError("getaddrinfo() failed");
    }

    /* Create socket for receiving datagrams */
    if ( (sock = socket(localAddr->ai_family, localAddr->ai_socktype, 0)) == INVALID_SOCKET )
    {
        DieWithError("socket() failed");
    }
	int yes;

	if ((setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0))
	{
		DieWithError("Reusing ADDR failed");
	}

    /* Bind to the multicast port */
    if ( bind(sock, localAddr->ai_addr, localAddr->ai_addrlen) != 0 )
    {
        DieWithError("bind() failed");
    }

    /* Join the multicast group. We do this seperately depending on whether we
     * are using IPv4 or IPv6. WSAJoinLeaf is supposed to be IP version agnostic
     * but it looks more complex than just duplicating the required code. */
    if ( multicastAddr->ai_family  == PF_INET &&  
         multicastAddr->ai_addrlen == sizeof(struct sockaddr_in) ) /* IPv4 */
    {
        struct ip_mreq multicastRequest;  /* Multicast address join structure */

        /* Specify the multicast group */
        memcpy(&multicastRequest.imr_multiaddr,
               &((struct sockaddr_in*)(multicastAddr->ai_addr))->sin_addr,
               sizeof(multicastRequest.imr_multiaddr));

        /* Accept multicast from any interface */
        multicastRequest.imr_interface.s_addr = htonl(INADDR_ANY);

        /* Join the multicast address */
        if ( setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*) &multicastRequest, sizeof(multicastRequest)) != 0 )
        {
            DieWithError("setsockopt() failed");
        }
    }
    else if ( multicastAddr->ai_family  == PF_INET6 &&
              multicastAddr->ai_addrlen == sizeof(struct sockaddr_in6) ) /* IPv6 */
    {
        struct ipv6_mreq multicastRequest;  /* Multicast address join structure */

        /* Specify the multicast group */
        memcpy(&multicastRequest.ipv6mr_multiaddr,
               &((struct sockaddr_in6*)(multicastAddr->ai_addr))->sin6_addr,
               sizeof(multicastRequest.ipv6mr_multiaddr));

        /* Accept multicast from any interface */
        multicastRequest.ipv6mr_interface = 0;

        /* Join the multicast address */
        if ( setsockopt(sock, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, (char*) &multicastRequest, sizeof(multicastRequest)) != 0 )
        {
            DieWithError("setsockopt() failed");
        }
    }
    else
    {
        DieWithError("Neither IPv4 or IPv6");
    }

    freeaddrinfo(localAddr);
    freeaddrinfo(multicastAddr);

	

    for (;;) /* Run forever */
    {
        time_t timert;
        char   recvString[500];      /* Buffer for received string */
        int    recvStringLen;        /* Length of received string */


		char ipstr[INET6_ADDRSTRLEN];

        /* Receive a single datagram from the server */
		struct sockaddr_in6 remote;
		int len = sizeof(remote);
        if ( (recvStringLen = recvfrom(sock, recvString, sizeof(recvString) - 1, 0, &remote, &len)) < 0 )
		//if ((recvStringLen = recvfrom(sock, recvString, sizeof(recvString) - 1, 0, NULL, 0)) < 0)
        {
            DieWithError("recvfrom() failed");
        }
		
        recvString[recvStringLen] = '\0';

        /* Print the received string */
        time(&timert);  /* get time stamp to print with recieved data */
        printf("Time Received: %.*s : %s\n", strlen(ctime(&timert)) - 1, ctime(&timert), recvString);

		if (sendto(sock, recvString, recvStringLen, 0,
			(struct sockaddr *)&remote, len) != recvStringLen)
		{
			DieWithError("sendto() sent a different number of bytes than expected");
		}

    }

    /* NOT REACHED */
    closesocket(sock);
    exit(EXIT_SUCCESS);
}