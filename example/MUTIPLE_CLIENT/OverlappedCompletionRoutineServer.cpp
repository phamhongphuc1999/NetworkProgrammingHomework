// OverlappedCompletionRoutineServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <process.h>
#pragma comment(lib, "Ws2_32.lib")

#define RECEIVE 0
#define SEND 1
#define PORT 5500
#define DATA_BUFSIZE 8192
#define MAX_CLIENT 2

typedef struct _SOCKET_INFORMATION {
      WSAOVERLAPPED overlapped;
      SOCKET sockfd;
      CHAR buff[DATA_BUFSIZE];
      WSABUF dataBuff;
      DWORD sentBytes;
      DWORD recvBytes;
	  DWORD operation;
} SOCKET_INFORMATION, *LPSOCKET_INFORMATION;

void CALLBACK workerRoutine(DWORD error, DWORD transferredBytes, LPWSAOVERLAPPED overlapped, DWORD inFlags);
unsigned __stdcall workerThread(LPVOID lpParameter);

SOCKET acceptSocket;
LPSOCKET_INFORMATION clients[MAX_CLIENT];
int nClients = 0;
CRITICAL_SECTION criticalSection;

int _tmain(int argc, _TCHAR* argv[])
{
	WSADATA wsaData;
	SOCKET listenSocket;
	SOCKADDR_IN serverAddr, clientAddr;
	int clientAddrLen = sizeof(clientAddr);
	INT ret;
	WSAEVENT acceptEvent;

	InitializeCriticalSection(&criticalSection);

	if ((ret = WSAStartup((2,2),&wsaData)) != 0){
		printf("WSAStartup() failed with error %d\n", ret);
		WSACleanup();
		return 1;
	}

	if ((listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET){
		printf("Failed to get a socket %d\n", WSAGetLastError());
		return 1;
	}

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(PORT);
	if (bind(listenSocket, (PSOCKADDR) &serverAddr, sizeof(serverAddr)) == SOCKET_ERROR){
		printf("bind() failed with error %d\n", WSAGetLastError());
		return 1;
	}

	if (listen(listenSocket, 20)){
		printf("listen() failed with error %d\n", WSAGetLastError());
		return 1;
	}

	printf("Server started!\n");

	if ((acceptEvent = WSACreateEvent()) == WSA_INVALID_EVENT){
		printf("WSACreateEvent() failed with error %d\n", WSAGetLastError());
		return 1;
	}

	// Create a worker thread to service completed I/O requests
	_beginthreadex(0, 0, workerThread, (LPVOID)acceptEvent, 0, 0);

	while(TRUE){
		if ((acceptSocket = accept(listenSocket, (PSOCKADDR)&clientAddr, &clientAddrLen)) == SOCKET_ERROR) {
			printf("accept() failed with error %d\n", WSAGetLastError());
			return 1;
		}

		if (WSASetEvent(acceptEvent) == FALSE){
			printf("WSASetEvent() failed with error %d\n", WSAGetLastError());
			return 1;
		}
	}
	return 0;
}

unsigned __stdcall workerThread(LPVOID lpParameter)
{
	DWORD flags;
	WSAEVENT events[1];
	DWORD index;
	DWORD recvBytes;

	// Save the accept event in the event array
	events[0] = (WSAEVENT) lpParameter;
	while(TRUE)
	{
		// Wait for accept() to signal an event and also process workerRoutine() returns
		while(TRUE){
			index = WSAWaitForMultipleEvents(1, events, FALSE, WSA_INFINITE, TRUE);
			if (index == WSA_WAIT_FAILED){
				printf("WSAWaitForMultipleEvents() failed with error %d\n", WSAGetLastError());
				return 1;
			}

			if (index != WAIT_IO_COMPLETION){
				// An accept() call event is ready - break the wait loop
				break;
			}
		}

		WSAResetEvent(events[index - WSA_WAIT_EVENT_0]);

		EnterCriticalSection(&criticalSection);

		if (nClients == MAX_CLIENT) {
			printf("Too many clients.\n");
			closesocket(acceptSocket);
			continue;
		}

		// Create a socket information structure to associate with the accepted socket
		if ((clients[nClients] = (LPSOCKET_INFORMATION) GlobalAlloc(GPTR, sizeof(SOCKET_INFORMATION))) == NULL){
			printf("GlobalAlloc() failed with error %d\n", GetLastError());
			return 1;
		}

		// Fill in the details of our accepted socket
		clients[nClients]->sockfd = acceptSocket;
		ZeroMemory(&(clients[nClients]->overlapped), sizeof(WSAOVERLAPPED));
		clients[nClients]->sentBytes = 0;
		clients[nClients]->recvBytes = 0;
		clients[nClients]->dataBuff.len = DATA_BUFSIZE;
		clients[nClients]->dataBuff.buf = clients[nClients]->buff;
		clients[nClients]->operation = RECEIVE;
		flags = 0;

		if (WSARecv(clients[nClients]->sockfd, &(clients[nClients]->dataBuff), 1, &recvBytes,
					&flags,	&(clients[nClients]->overlapped), workerRoutine) == SOCKET_ERROR){
			if (WSAGetLastError() != WSA_IO_PENDING){
				printf("WSARecv() failed with error %d\n", WSAGetLastError());
				return 1;
			}
		}

		printf("Socket %d got connected...\n", acceptSocket);
		nClients++;
		LeaveCriticalSection(&criticalSection);
	}

	return 0;
}

void CALLBACK workerRoutine(DWORD error, DWORD transferredBytes, LPWSAOVERLAPPED overlapped, DWORD inFlags)
{
	DWORD sendBytes, recvBytes;
	DWORD flags;

	// Reference the WSAOVERLAPPED structure as a SOCKET_INFORMATION structure
	LPSOCKET_INFORMATION sockInfo = (LPSOCKET_INFORMATION) overlapped;

	if (error != 0)
		printf("I/O operation failed with error %d\n", error);

	if (transferredBytes == 0)
		printf("Closing socket %d\n\n", sockInfo->sockfd);

	if (error != 0 || transferredBytes == 0){
		//Find and remove socket
		EnterCriticalSection(&criticalSection);

		int index;
		for (index = 0; index < nClients; index++)
			if (clients[index]->sockfd == sockInfo->sockfd)
				break;

		closesocket(clients[index]->sockfd);
		GlobalFree(clients[index]);

		for (int i = index; i < nClients - 1; i++)
			clients[i] = clients[i + 1];
		nClients--;

		LeaveCriticalSection(&criticalSection);

		return;
	}

	// Check to see if the recvBytes field equals zero. If this is so, then
	// this means a WSARecv call just completed so update the recvBytes field
	// with the transferredBytes value from the completed WSARecv() call
	if (sockInfo->operation == RECEIVE){
		sockInfo->recvBytes = transferredBytes;
		sockInfo->sentBytes = 0;
		sockInfo->operation = SEND;
	}
	else{
		sockInfo->sentBytes += transferredBytes;
	}

	if (sockInfo->recvBytes > sockInfo->sentBytes){
		// Post another WSASend() request.
		// Since WSASend() is not guaranteed to send all of the bytes requested,
		// continue posting WSASend() calls until all received bytes are sent
		ZeroMemory(&(sockInfo->overlapped), sizeof(WSAOVERLAPPED));
		sockInfo->dataBuff.buf = sockInfo->buff + sockInfo->sentBytes;
		sockInfo->dataBuff.len = sockInfo->recvBytes - sockInfo->sentBytes;
		sockInfo->operation = SEND;
		if (WSASend(sockInfo->sockfd, &(sockInfo->dataBuff), 1, &sendBytes, 0, &(sockInfo->overlapped), workerRoutine) == SOCKET_ERROR){
			if (WSAGetLastError() != WSA_IO_PENDING){
				printf("WSASend() failed with error %d\n", WSAGetLastError());
				return;
			}
		}
	}
	else{
		// Now that there are no more bytes to send post another WSARecv() request
		sockInfo->recvBytes = 0;
		flags = 0;
		ZeroMemory(&(sockInfo->overlapped), sizeof(WSAOVERLAPPED));
		sockInfo->dataBuff.len = DATA_BUFSIZE;
		sockInfo->dataBuff.buf = sockInfo->buff;
		sockInfo->operation = RECEIVE;
		if (WSARecv(sockInfo->sockfd, &(sockInfo->dataBuff), 1, &recvBytes, &flags, &(sockInfo->overlapped), workerRoutine) == SOCKET_ERROR){
			if (WSAGetLastError() != WSA_IO_PENDING ){
				printf("WSARecv() failed with error %d\n", WSAGetLastError());
				return;
			}
		}
	}
}
