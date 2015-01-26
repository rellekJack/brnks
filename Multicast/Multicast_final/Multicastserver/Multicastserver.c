#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>      /* for fprintf() */
#include <winsock2.h>   /* for socket(), connect(), send(), and recv() */
#include <ws2tcpip.h>   /* for ip options */
#include <stdlib.h>     /* for atoi() and exit() */

#include "data.h"
#include "timer.h"
#include "ClientSy.h"

#if defined(_MSC_VER)
#pragma comment(lib, "ws2_32.lib")
#endif


static void DieWithError(char* errorMessage)
{
    fprintf(stderr, "%s: %d\n", errorMessage, WSAGetLastError());
	getchar();
    exit(EXIT_FAILURE);
}

//int printAnswer(struct answer *answPtr) // print received answer
//{
//	switch (answPtr->AnswType)
//	{
//	case AnswHello:
//		printf("Answer Hello");
//		break;
//	case AnswOk:
//		printf("Answer Ok : Packet acknowledged No: %u ", answPtr->SeNo);
//		break;
//	case AnswNACK:
//		printf("Answer NACK: Packet negativ acknowledged No: %u ", answPtr->SeNo);
//		break;
//	case AnswClose:
//		printf("Answer Close");
//		break;
//	case AnswErr:
//		printf("Answer Error: %s", errorTable[answPtr->ErrNo]);
//		break;
//	default:
//		printf("Illegal Answer");
//		break;
//	};							/* switch */
//	puts("\n");
//	return answPtr->AnswType;
//}

void createRequestStack(char* file, struct request **stack, int *length){
	FILE *fp;
	int i;
	char c = '\0';
	int seqNr = 1;
	if ((fp = fopen(file, "rt")) == 0)		// open file
	{
		DieWithError("FAILED TO OPEN FILE");
	}
	while (c != EOF && (c = fgetc(fp)) != EOF){
		i = 0;
		(*stack) = realloc(*stack, (++(*length)) * sizeof(struct request));
		(*stack)[*length - 1].ReqType = ReqData;
		(*stack)[*length - 1].SeNr = seqNr++;
		strcpy((*stack)[*length - 1].fname, file);
		do{
			(*stack)[*length - 1].name[i++] = c;
		} while ((c = fgetc(fp)) != '\n' && c != EOF);
		(*stack)[*length - 1].name[i] = '\0';
		(*stack)[*length - 1].FlNr = i;

	}
	fclose(fp);
}

//void sendRequest(SOCKET ConnSocket, struct request *req, struct timeouts **timeouts, int *sqnr_counter, struct request *temp,  struct sockaddr_in6 *remote, int *sizeOfRemote)
//{
//	int w, res;
//	struct timeval timer;
//	fd_set rfds;
//
//	timer.tv_sec = 0;
//	timer.tv_usec = 300000; // 300 msek
//	FD_ZERO(&rfds);
//	FD_SET(ConnSocket, &rfds);
//	if ((req->ReqType == ReqHello))				// for hello package
//	{
//
//		printf("-SENDING HELLO PACKET\tSeqNr: %d | Type: %c\n", req->SeNr, req->ReqType);
//		w = sendto(ConnSocket, (const char *)req, sizeof(*req), 0, (struct sockaddr *)remote, &sizeOfRemote);
//		if (w == SOCKET_ERROR)			// if sending failed because of socket problems
//		{
//			fprintf(stderr, "send() failed: error %d\n", WSAGetLastError());
//		}
//	}
//	if (*timeouts == NULL && ((req->ReqType == ReqData) || (req->ReqType == ReqClose)))		// send data-packages if timeout-list is empty
//	{
//		req->SeNr = (*sqnr_counter);
//		(*sqnr_counter)++;
//		printf("-SENDING DATA PACKET\tSeqNr: %d | Type: %c\n", req->SeNr, req->ReqType);
//		w = sendto(ConnSocket, (const char *)req, sizeof(*req), 0, (struct sockaddr *)remote, &sizeOfRemote);
//		if (w == SOCKET_ERROR)			// if sending failed because of socket problems
//		{
//			fprintf(stderr, "send() failed: error %d\n", WSAGetLastError());
//		}
//	}
//	while (1){
//		res = select(ConnSocket + 1, &rfds, NULL, NULL, &timer);		// check for writing on socket in the next 100ms (waiting for answer)
//
//		if (*timeouts != NULL)
//			decrement_timer(*timeouts);
//		if (res == 0)	// no answer received in 100 ms
//		{
//			if ((*fail) == 0)
//			{
//				printf("Packet Nr.%d send failure\n\n", req->SeNr);
//				(*timeouts) = add_timer(*timeouts, 3, req->SeNr);		// add a new timeout to the timeout-list
//				(*fail) = 1;										// indicate send-fail
//				(*temp) = (*req);							// save not send package
//			}
//		}
//		else			// TEST
//		{
//			(*fail) = 0;
//			if (*timeouts != NULL)
//				(*timeouts) = NULL;
//		}
//		return;
//	}
//	
//}

struct answer *recvanswer(SOCKET ConnSocket, struct sockaddr_in6 *remote, int *sizeOfRemote)
{
	int recvcc;						/* Length of message */
	int remoteAddrSize = sizeof(struct sockaddr_in6);
	struct answer answ;

	/* Receive a message from a socket */
	printf("-RECEIVING ANSWER\t");
	recvcc = recvfrom(ConnSocket, (char *)&answ, sizeof(answ), 0, (struct sockaddr *)remote, &sizeOfRemote); // receive answer
	if (recvcc == SOCKET_ERROR)				// if receiving failed because of socket problems
	{
		printf("... NO SUCCESS ... trying again\n");

	}
	if (recvcc == 0)						// if server closed connection
	{
		printf("Server closed connection\n");
		closesocket(ConnSocket);
		WSACleanup();
		exit(-1);
	}
	return (&answ);
}

struct timeval newTimeout(long msecLength){
	return (struct timeval){ 0, msecLength * 1000 };
}

int main(int argc, char *argv[]){
    SOCKET    sock;                   /* Socket */
    WSADATA   wsaData;                /* For WSAStartup */
    char*     multicastIP;            /* Arg: IP Multicast address */
    char*     multicastPort;          /* Arg: Server port */
    char*     filename	;             /* Arg: File to multicast */
	int		  windowSize;
    DWORD     multicastTTL;           /* Arg: TTL of multicast packets */
    ADDRINFO* multicastAddr;          /* Multicast address */
    ADDRINFO  hints          = { 0 }; /* Hints for name lookup */

    if ( WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        DieWithError("WSAStartup() failed");
    }
	
    if ( argc < 5 || argc > 6 )
    {
        fprintf(stderr, "Usage:  %s <Multicast Address> <Port> <Filename> <WindowSize> [<TTL>]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
	

    multicastIP   = argv[1];             /* First arg:   multicast IP address */
    multicastPort = argv[2];             /* Second arg:  multicast port */
	filename    = argv[3];             /* Third arg:   String to multicast */
	if (filename == NULL || filename == "") DieWithError("Kein Dateiname angegeben");
	windowSize = (argc >= 5 ? atoi(argv[4]):1);
	if (windowSize < 1 || windowSize > 10) DieWithError("WindowSize muss zwischen 1 und 10 liegen");
    multicastTTL  = (argc == 6 ?         /* Fith arg:  If supplied, use command-line */
                     atoi(argv[5]) : 1); /* specified TTL, else use default TTL of 1 */

   
	


    /* Resolve destination address for multicast datagrams */
    hints.ai_family   = PF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags    = AI_NUMERICHOST;
    if ( getaddrinfo(multicastIP, multicastPort, &hints, &multicastAddr) != 0 )
    {
        DieWithError("getaddrinfo() failed");
    }

    printf("Using %s\n", multicastAddr->ai_family == PF_INET6 ? "IPv6" : "IPv4");

    /* Create socket for sending multicast datagrams */
    if ( (sock = socket(multicastAddr->ai_family, multicastAddr->ai_socktype, 0)) == INVALID_SOCKET )
    {
        DieWithError("socket() failed");
    }

    /* Set TTL of multicast packet */
    if ( setsockopt(sock,
                    multicastAddr->ai_family == PF_INET6 ? IPPROTO_IPV6        : IPPROTO_IP,
                    multicastAddr->ai_family == PF_INET6 ? IPV6_MULTICAST_HOPS : IP_MULTICAST_TTL,
                    (char*) &multicastTTL, sizeof(multicastTTL)) != 0 )
    {
        DieWithError("setsockopt() failed");
    }

	

	struct timeval timer;
	fd_set rfds;
	

	int i = 0, w, receiverCount = 0, trysCount;

	while (1) /* Run forever */
	{
		int sqnr_counter = 1, fpos = 0, fail = 0;
		int recvStringLen;
		char recvString[500];
		struct sockaddr_in6 receiver[MAX_MC_RECEIVER];
		struct request request;
		int responseCount;

		/*SENDING HELLO*/
		while (1){
			request.SeNr = 0;
			request.ReqType = ReqHello;
			strcpy(request.fname, filename);
			receiverCount = 0;

			printf("-SENDING HELLO PACKET\tSeqNr: %d | Type: %c\n", 0, ReqHello);
			w = sendto(sock, (const char *)&request, sizeof(request), 0, multicastAddr->ai_addr, multicastAddr->ai_addrlen);
			if (w == SOCKET_ERROR)			// if sending failed because of socket problems
			{
				DieWithError("HELLO SEND FAILED");
			}

			for (trysCount = 0; receiverCount < MAX_MC_RECEIVER && trysCount < TIMEOUT; trysCount++){
				while (1){
					FD_ZERO(&rfds);
					FD_SET(sock, &rfds);
					struct timeval timer = newTimeout(TIMEOUT_INT);
					struct answer helloAnswer;
					w = select(0, &rfds, NULL, NULL, &timer);
					if (w == -1){
						DieWithError("Fehler bei select()");
					}

					if (w == 0){
						break;
					}

					if ((recvStringLen = recvfrom(sock, (char*)&helloAnswer, sizeof(helloAnswer), 0,NULL, 0) < 0))
					{
						DieWithError("recvfrom() failed");
					}

					printf("++ GOT ANSWER: %c ++\n", helloAnswer.AnswType);

					if (helloAnswer.AnswType == AnswHello){
						receiverCount++;
					}
				}
			}

			if (receiverCount > 0) break;
		}

		request.ReqType = ReqData;
		struct request *requests = NULL;
		int length = 0;
		int position = 0;

		createRequestStack(filename, &requests, &length);
		/*SENDING DATA*/
		int isFinished = 0;
		while (receiverCount > 0 && !isFinished){
			int receivedNACK;
			do{
				responseCount = 0;
				receivedNACK = 0;
				for (int c = 0; c < windowSize && !isFinished; c++){
					request = requests[position++];
					if (position == length){
						isFinished = 1;
					}
					
					//if (windowSize >= 3 && request.SeNr == 2) request.SeNr = 3; // NACK FALL
					printf("-SENDING DATA PACKET\tSeqNr: %d | Type: %c\n", request.SeNr, request.ReqType);
					w = sendto(sock, (const char *)&request, sizeof(request), 0, multicastAddr->ai_addr, multicastAddr->ai_addrlen);
					if (w == SOCKET_ERROR)			// if sending failed because of socket problems
					{
						fprintf(stderr, "send() failed: error %d\n", WSAGetLastError());
					}
				}
				//if (isFinished) break;
				
				for (trysCount = 0; trysCount < TIMEOUT && responseCount < receiverCount * windowSize && !receivedNACK; trysCount++){
					while (1){
						FD_ZERO(&rfds);
						FD_SET(sock, &rfds);
						struct timeval timer = newTimeout(TIMEOUT_INT);
						struct answer dataAnswer;
						w = select(0, &rfds, NULL, NULL, &timer);
						if (w < 0){
							DieWithError("ERROR DURING SENDING DATA");
						}
						if (w == 0){
							break;
						}
						if ((recvStringLen = recvfrom(sock, (const char*)&dataAnswer, sizeof(dataAnswer), 0, NULL,0) < 0))
						{
							DieWithError("recvfrom() failed");
						}

						printf("++ GOT ANSWER %c FOR #%d++\n", dataAnswer.AnswType, dataAnswer.SeNo);

						if (dataAnswer.AnswType == AnswOk){
							responseCount++;
						}

						if (dataAnswer.AnswType == AnswNACK){
							request = requests[dataAnswer.SeNo - 1];
							printf("-RESENDING DATA PACKET\tSeqNr: %d | Type: %c\n", request.SeNr, request.ReqType);
							w = sendto(sock, (const char *)&request, sizeof(request), 0, multicastAddr->ai_addr, multicastAddr->ai_addrlen);
							if (w == SOCKET_ERROR)			// if sending failed because of socket problems
							{
								DieWithError("ERROR WHILE RESENDING PACKET #%d", request.SeNr);
							}
							trysCount = 0;
							responseCount = 0;
						}
					}
				}
				
				if (trysCount == TIMEOUT && responseCount < receiverCount) receiverCount = responseCount; // AFTER 3 TRYS SET receiverCount to responseCount
			} while (receivedNACK || responseCount < receiverCount || !isFinished);
		}

		/*SENDING CLOSE*/
		responseCount = 0;
		while (receiverCount > 0 && responseCount < receiverCount){
			responseCount = 0;
			request.SeNr = length + 1;
			request.ReqType = ReqClose;
			trysCount = 0;
			printf("-SENDING CLOSE PACKET\tSeqNr: %d | Type: %c\n", request.SeNr, request.ReqType);
			w = sendto(sock, (const char *)&request, sizeof(request), 0, multicastAddr->ai_addr, multicastAddr->ai_addrlen);
			if (w == SOCKET_ERROR)			// if sending failed because of socket problems
			{
				fprintf(stderr, "send() failed: error %d\n", WSAGetLastError());
			}
			for (trysCount = 0; trysCount < TIMEOUT && responseCount < receiverCount; trysCount++){
				while (1){
					FD_ZERO(&rfds);
					FD_SET(sock, &rfds);
					struct timeval timer = newTimeout(TIMEOUT_INT);
					struct answer closeAnswer;
					w = select(0, &rfds, NULL, NULL, &timer);
					if (w < 0){
						DieWithError("ERROR DURING SENDING DATA");
					}
					if (w == 0){
						break;
					}
					if ((recvStringLen = recvfrom(sock, (const char*)&closeAnswer, sizeof(closeAnswer), 0, NULL, 0) < 0))
					{
						DieWithError("recvfrom() failed");
					}

					printf("++ GOT ANSWER: %c ++\n", closeAnswer.AnswType);

					if (closeAnswer.AnswType == AnswClose){
						responseCount++;
					}
				}
			}
		}
	}

	printf("CLIENT TERMINATED");

    /* NOT REACHED */
    freeaddrinfo(multicastAddr);
    closesocket(sock);
    return 0;
}