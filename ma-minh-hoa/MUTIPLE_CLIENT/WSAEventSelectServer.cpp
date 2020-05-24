// WSAEventSelectServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <conio.h>
#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")

#define PORT 5500
#define BUFF_SIZE 2048

int Receive(SOCKET, char *, int, int);
int Send(SOCKET, char *, int, int);

int _tmain(int argc, _TCHAR* argv[])
{
	DWORD		nEvents = 0;
	DWORD		index;
	SOCKET		socks[WSA_MAXIMUM_WAIT_EVENTS];
	WSAEVENT	events[WSA_MAXIMUM_WAIT_EVENTS];
	WSANETWORKEVENTS sockEvent;
	
	//Step 1: Initiate WinSock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData))
		printf("Version is not supported\n");

	//Step 2: Construct LISTEN socket	
	SOCKET listenSock;
	listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//Step 3: Bind address to socket
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	socks[0] = listenSock;
	events[0] = WSACreateEvent(); //create new events
	nEvents++;

	// Associate event types FD_ACCEPT and FD_CLOSE
	// with the listening socket and newEvent   
	WSAEventSelect(socks[0], events[0], FD_ACCEPT | FD_CLOSE);
	

	if (bind(listenSock, (sockaddr *)&serverAddr, sizeof(serverAddr)))
	{
		printf("Error! Cannot bind this address.");
		_getch();
		return 0;
	}
		
	//Step 4: Listen request from client
	if (listen(listenSock, 10)) {
		printf("Error! Cannot listen.");
		_getch();
		return 0;
	}

	printf("Server started!\n");

	char sendBuff[BUFF_SIZE], recvBuff[BUFF_SIZE];
	SOCKET connSock;
	sockaddr_in clientAddr;
	int clientAddrLen = sizeof(clientAddr);
	int ret, i;
	
	for (i = 1; i < WSA_MAXIMUM_WAIT_EVENTS; i++) {
		socks[i] = 0;
	}
	while (1) {
		//wait for network events on all socket
		index = WSAWaitForMultipleEvents(nEvents, events, FALSE, WSA_INFINITE, FALSE);
		if (index == WSA_WAIT_FAILED) {
			printf("WSAWaitForMultipleEvents() failed with error %d\n", WSAGetLastError());
			break;
		}

		index = index - WSA_WAIT_EVENT_0;
		WSAEnumNetworkEvents(socks[index], events[index], &sockEvent);

		if (sockEvent.lNetworkEvents & FD_ACCEPT) {
			if (sockEvent.iErrorCode[FD_ACCEPT_BIT] != 0) {
				printf("FD_ACCEPT failed with error %d\n", sockEvent.iErrorCode[FD_READ_BIT]);
				break;
			}

			if ((connSock = accept(socks[index], (sockaddr *) &clientAddr, &clientAddrLen)) == SOCKET_ERROR) {
				printf("accept() failed with error %d\n", WSAGetLastError());
				break;
			}

			//Add new socket into socks array
			int i;
			if (nEvents == WSA_MAXIMUM_WAIT_EVENTS) {
				printf("\nToo many clients.");
				closesocket(connSock);
			}
			else
				for (i = 1; i < WSA_MAXIMUM_WAIT_EVENTS; i++)
					if (socks[i] == 0) {
						socks[i] = connSock;
						events[i] = WSACreateEvent();
						WSAEventSelect(socks[i], events[i], FD_READ | FD_CLOSE);
						nEvents++;
						break;
					}
			
			//reset event
			WSAResetEvent(events[index]);
		}

		if (sockEvent.lNetworkEvents & FD_READ) {
			//Receive message from client
			if (sockEvent.iErrorCode[FD_READ_BIT] != 0) {
				printf("FD_READ failed with error %d\n", sockEvent.iErrorCode[FD_READ_BIT]);
				break;
			}

			ret = Receive(socks[index], recvBuff, BUFF_SIZE, 0);

			//Release socket and event if an error occurs
			if (ret < 0) {
				closesocket(socks[index]);
				socks[index] = 0;
				WSACloseEvent(events[index]);
				nEvents--;
			}
			else {									//echo to client
				memcpy(sendBuff, recvBuff, ret);
				Send(socks[index], sendBuff, ret, 0);

				//reset event
				WSAResetEvent(events[index]);
			}
		}

		if (sockEvent.lNetworkEvents & FD_CLOSE) {
			if (sockEvent.iErrorCode[FD_CLOSE_BIT] != 0) {
				printf("FD_READ failed with error %d\n", sockEvent.iErrorCode[FD_CLOSE_BIT]);
				break;
			}
			//Release socket and event
			closesocket(socks[index]);
			socks[index] = 0;
			WSACloseEvent(events[index]);
			nEvents--;
		}
	}
	return 0;
}

/* The recv() wrapper function */
int Receive(SOCKET s, char *buff, int size, int flags) {
	int n;

	n = recv(s, buff, size, flags);
	if (n == SOCKET_ERROR)
		printf("Error: %d", WSAGetLastError());

	return n;
}

/* The send() wrapper function*/
int Send(SOCKET s, char *buff, int size, int flags) {
	int n;

	n = send(s, buff, size, flags);
	if (n == SOCKET_ERROR)
		printf("Error: %d", WSAGetLastError());

	return n;
}