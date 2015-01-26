#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <ctype.h>
#include <tchar.h>
#include <sys/types.h>
#include <windows.h>

#include "data.h"
#include "timer.h"

/* static struct answer answ; */
struct answer answ;

unsigned short	portnr = 50000;
char			*server_port = "50000";

SOCKET ConnSocket;

// Declare socket address "remote" as static
static struct sockaddr_in6 remoteAddr;
struct in6_addr anyaddr = IN6ADDR_ANY_INIT;


//void initClient(char *name)
//{
//	int val, err, res;
//	u_long on = 1;						// on!=0, non-blocking mode is enabled.
//	struct in6_addr loopback = IN6ADDR_LOOPBACK_INIT;
//	WSADATA wsaData;
//	WORD wVersionRequested;
//
//	wVersionRequested = MAKEWORD(2, 1);
//	if (WSAStartup(wVersionRequested, &wsaData) == SOCKET_ERROR)
//	{
//		printf("SERVER: WSAStartup() failed!\n");
//		printf("        error code: %d\n", WSAGetLastError());
//		exit(-1);
//	}
//	ConnSocket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);		// set socket
//	memset(&remoteAddr, 0, sizeof (remoteAddr));				// configure socket
//	remoteAddr.sin6_family = AF_INET6;
//	remoteAddr.sin6_flowinfo = 0;
//	if (name != 0)
//	{
//		if (_istalpha(name[0]))		 // name
//		{
//			val = sizeof(remoteAddr);
//			err = WSAStringToAddressA(name, AF_INET6, 0, (struct sockaddr *)&remoteAddr, &val);
//
//			if (err==SOCKET_ERROR)
//				printf("Fehler in Addressaufloesung! (Name) %s\n", name);
//		}
//		else						// number
//		{
//			val = sizeof(remoteAddr);
//			err = WSAStringToAddressA(name, AF_INET6, 0, (struct sockaddr *)&remoteAddr, &val);
//
//			if (err==SOCKET_ERROR)
//				printf("Fehler in Addressaufloesung! (Adresse) %s\n", name);
//		}
//	}
//	else							// no address set as program argument
//	{
//		remoteAddr.sin6_addr=loopback;
//	}
//	remoteAddr.sin6_port = htons(portnr);
//	res = ioctlsocket(ConnSocket, FIONBIO, &on);	// make socket non blocking
//}

//struct timeouts *sendData(struct request *req, struct timeouts *listptr, int *sqnr_counter, int *fail, struct request *temp, int *ende)
//{
//	int w, res;
//	struct timeval timer;
//	fd_set rfds;
//
//	timer.tv_sec = 0;
//	timer.tv_usec = 300000; // 300 msek
//	FD_ZERO(&rfds);
//	FD_SET(ConnSocket, &rfds);
//	if ((req->ReqType == ReqHello) && ((*fail) != 1))				// for hello package
//	{
//		
//		printf("-SENDING HELLO PACKET\tSeqNr: %d | Type: %c\n", req->SeNr, req->ReqType);
//		w = sendto(ConnSocket, (const char *)req, sizeof(*req), 0, (struct sockaddr *)&remoteAddr, sizeof(remoteAddr));
//		if (w == SOCKET_ERROR)			// if sending failed because of socket problems
//		{
//			fprintf(stderr, "send() failed: error %d\n", WSAGetLastError());
//		}
//	}
//	if (listptr == NULL && ((req->ReqType == ReqData) || (req->ReqType == ReqClose)) && ((*fail) != 1))		// send data-packages if timeout-list is empty
//	{
//		req->SeNr = (*sqnr_counter);
//		(*sqnr_counter)++;
//		printf("-SENDING DATA PACKET\tSeqNr: %d | Type: %c\n", req->SeNr, req->ReqType);
//		w = sendto(ConnSocket, (const char *)req, sizeof(*req), 0, (struct sockaddr *)&remoteAddr, sizeof(remoteAddr));
//		if (w == SOCKET_ERROR)			// if sending failed because of socket problems
//		{
//			fprintf(stderr, "send() failed: error %d\n", WSAGetLastError());
//		}
//	}
//	if (((*fail) == 1) && (listptr->timer == 0)) // for re-sending timeout package
//	{
//		(*fail) = 0;
//		(*req) = (*temp);
//		printf("-TIMEOUT!\n-RE-SENDING MISSING PACKET\tSeqNr: %d | Type: %c\n", req->SeNr, req->ReqType);
//		w = sendto(ConnSocket, (const char *)req, sizeof(*req), 0, (struct sockaddr *)&remoteAddr, sizeof(remoteAddr));
//		if (w == SOCKET_ERROR)			// if sending failed because of socket problems
//		{
//			fprintf(stderr, "send() failed: error %d\n", WSAGetLastError());
//		}
//		listptr = del_timer(listptr, req->SeNr);
//	}
//	res = select(ConnSocket + 1, &rfds, NULL, NULL, &timer);		// check for reading on socket in the next 100ms (waiting for answer)
//	if (listptr != NULL)
//		decrement_timer(listptr);
//	if (res == 0)	// no answer received in 100 ms
//	{
//		if((*fail) == 0)
//		{
//			printf("Packet Nr.%d send failure\n\n", req->SeNr);
//			listptr = add_timer(listptr, 3, req->SeNr);		// add a new timeout to the timeout-list
//			(*fail) = 1;										// indicate send-fail
//			(*temp) = (*req);							// save not send package
//		}
//	}
//	else			// TEST
//	{
//		(*fail) = 0;
//		if (listptr != NULL)
//			listptr = NULL;
//	}
//	return (listptr);									// return timeout-list
//}

//struct answer *recvanswer()
//{
//	int recvcc;						/* Length of message */
//	int remoteAddrSize = sizeof(struct sockaddr_in6);
//
//	/* Receive a message from a socket */
//	printf("-RECEIVING ANSWER\t");
//	recvcc = recvfrom(ConnSocket, (char *)&answ, sizeof (answ), 0, (struct sockaddr *) &remoteAddr, &remoteAddrSize); // receive answer
//	if (recvcc == SOCKET_ERROR)				// if receiving failed because of socket problems
//	{
//		printf("... NO SUCCESS ... trying again\n");
//	
//	}
//	if (recvcc == 0)						// if server closed connection
//	{
//		printf("Server closed connection\n");
//		closesocket(ConnSocket);
//		WSACleanup();
//		exit(-1);
//	}
//	return (&answ);
//}

void exitClient()
{
	

	closesocket(ConnSocket);			//close the socket
	printf("in exit client\n");

	if (WSACleanup() == SOCKET_ERROR)	// terminates socket operations
	{
		printf("Client: WSACleanup() failed!\n");
		printf("        error code: %d\n", WSAGetLastError());
		exit(-1);
	}
}