// Task2_Server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <conio.h>
#include <stdio.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <SDKDDKVer.h>
#include <iostream>
#include <process.h>

#define SERVER_ADDR "127.0.0.1"
#define BUFF_SIZE 2048
#define SERVER_EXE "Task2_Server.exe"
#define MAX_CLIENT 1024
using namespace std;

#pragma comment(lib, "Ws2_32.lib")

#pragma region COMMON
/*
    type: 1-domain name, 0-ip
	status: 1-success, 2-get address infomation fall, 3-not found infomation
*/
struct SESSION {
	int status;
	int type;
	addrinfo* address;
	char* hostName;
	SOCKET connSock;
};

int isThreadFull;

void InitiateSession(SESSION* session) {
	session->status = -1;
	session->type = -1;
	session->address = NULL;
	session->hostName = new  char[BUFF_SIZE];
	session->connSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	session->connSock = 0;
}
#pragma endregion

#pragma region ENCAPSULATION
//return the message size
int LENGTH(char* buff) {
	int result = 0, index = 9;
	while (!('0' <= buff[index] && buff[index] <= '9')) { index--; }
	for (int i = index; i >= 0; i--) {
		result = result * 10 + (buff[i] - '0');
	}
	return result;
}

//add message size in the top of message
char* AddHeader(char* dest, char* source) {
	int a = strlen(source);
	int index = 0;
	while (a > 0) {
		int temp = a % 10;
		a = a / 10;
		dest[index] = temp + '0';
		index++;
	}
	for (int i = index; i < 10; i++) {
		dest[i] = 'e';
	}
	dest[10] = 0;
	strcat_s(dest, strlen(source) + strlen(dest) + 1, source);
	return dest;
}
#pragma endregion

#pragma region STREAM TCP
int RECEIVE_TCP(SOCKET s, char* buff, int flag) {
	int index = 0, ret, result = 0;
	char* temp = new char[10];
	ret = recv(s, temp, 10, flag);
	if (ret == 0) return 0;
	else if (ret == SOCKET_ERROR) return SOCKET_ERROR;
	else {
		int length = LENGTH(temp);
		while (length > 0) {
			ret = recv(s, &buff[index], length, 0);
			if (ret == SOCKET_ERROR) return SOCKET_ERROR;
			else result += ret;
			index += ret;
			length -= ret;
		}
		return result;
	}
}

int SEND_TCP(SOCKET s, char* buff, int flag) {
	int nLeft = strlen(buff), index = 0, ret, result = 0;
	while (nLeft > 0) {
		ret = send(s, &buff[index], nLeft, flag);
		if (ret == SOCKET_ERROR) return SOCKET_ERROR;
		else result += ret;
		nLeft -= ret; index += ret;
	}
	return result;
}

int SendDomainName(SESSION session, int flag) {
	int status = session.status, ret;
	char dest[BUFF_SIZE];
	if (status == 1) {
		addrinfo* addr = session.address;
		char* c = new char[3]{ "11" };
		ret = SEND_TCP(session.connSock, AddHeader(dest, c), flag);
		if (ret == SOCKET_ERROR) return SOCKET_ERROR;
		while (addr != NULL) {
			sockaddr_in* addrTemp = (sockaddr_in*)addr->ai_addr;
			char* cTemp = inet_ntoa(addrTemp->sin_addr);
			ret = SEND_TCP(session.connSock, AddHeader(dest, cTemp), flag);
			if (ret == SOCKET_ERROR) return SOCKET_ERROR;
			addr = addr->ai_next;
		}
		c = new char[2]{ "0" };
		ret = SEND_TCP(session.connSock, AddHeader(dest, c), flag);
		if (ret == SOCKET_ERROR) return SOCKET_ERROR;
	}
	else if (status == 2) {
		char* cTemp = new char[3]{ "21" };
		ret = SEND_TCP(session.connSock, AddHeader(dest, cTemp), flag);
		if (ret == SOCKET_ERROR) return SOCKET_ERROR;
	}
	else if (status == 3) {
		char* cTemp = new char[3]{ "31" };
		ret = SEND_TCP(session.connSock, AddHeader(dest, cTemp), flag);
		if (ret == SOCKET_ERROR) return SOCKET_ERROR;
	}
	return 0;
}

int SendIP(SESSION session, int flag) {
	int status = session.status, ret;
	char dest[BUFF_SIZE];
	if (status == 1) {
		char* c = new char[3]{ "10" };
		ret = SEND_TCP(session.connSock, AddHeader(dest, c), flag);
		if (ret == SOCKET_ERROR) return SOCKET_ERROR;
		ret = SEND_TCP(session.connSock, AddHeader(dest, session.hostName), flag);
		if (ret == SOCKET_ERROR) return SOCKET_ERROR;
	}
	else if (status == 2) {
		char* cTemp = new char[3]{ "20" };
		ret = SEND_TCP(session.connSock, AddHeader(dest, cTemp), flag);
		if (ret == SOCKET_ERROR) return SOCKET_ERROR;
	}
	else if (status == 3) {
		char* cTemp = new char[3]{ "30" };
		ret = SEND_TCP(session.connSock, AddHeader(dest, cTemp), flag);
		if (ret == SOCKET_ERROR) return SOCKET_ERROR;
	}
	return 0;
}
#pragma endregion

#pragma region CHECK MESSAGE FROM CLIENT
//Check value that client send
//return 0 if value is IP, 1 if value is domain name
int IsDomainNameOrIP(char* value) {
	int result = 0, index = 0, sum = 0, countPoint = 0;
	char element = value[0];
	while (element != '\0') {
		if ('0' <= element && element <= '9') {
			sum = sum * 10 + (element - '0');
			element = value[++index];
		}
		else if (element == '.') {
			countPoint++;
			if (sum > 255 || countPoint > 3) { result = 1; break; }
			else {
				sum = 0; element = value[++index];
			}
		}
		else {
			result = 1; break;
		}
	}
	return result;
}
#pragma endregion

#pragma region RESOLVER
void ResolverDomainName(char* name, SESSION* session) {
	addrinfo* infoHostName = NULL;
	session->type = 1; 
	session->hostName = NULL;

	int check = getaddrinfo(name, "http", NULL, &infoHostName);
	if (check) { //get address infomation fall
		session->status = 2;
		session->address = NULL;
	}
	if (infoHostName == NULL) { //not found infomation
		session->status = 3;
		session->address = NULL;
		freeaddrinfo(infoHostName);
	}
	else
	{
		session->status = 1;
		session->address = infoHostName;
		freeaddrinfo(infoHostName);
	}
}

void ResolverIPAddress(char* ip, SESSION* session) {
	char serverInfo[NI_MAXSERV], hostName[NI_MAXHOST];
	session->address = NULL; 
	session->type = 0;
	sockaddr_in addr;
	u_short port = 1;

	addr.sin_addr.s_addr = inet_addr(ip);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	int check = getnameinfo((sockaddr*)&addr, sizeof(sockaddr), hostName, NI_MAXHOST,
		serverInfo, NI_MAXSERV, NI_NUMERICSERV);
	if (check) {
		session->hostName = NULL;
		session->status = 2;
	}
	if (ip == NULL || !strcmp(ip, hostName)) {
		session->hostName = NULL;
		session->status = 3;
	}
	else {
		session->hostName = hostName;
		session->status = 1;
	}
}
#pragma endregion

#pragma region HANDLE MULTIPLE CLIENT
unsigned _stdcall Handler(void* param) {
	SOCKET listenSocket = (SOCKET)param;
	sockaddr_in clientAddr;
	char buff[BUFF_SIZE];
	int ret, numberOfClient = 0, nEvents, clientAddrLen;
	SESSION client[FD_SETSIZE];
	SOCKET connSock;
	for (int i = 0; i < FD_SETSIZE; i++) InitiateSession(&client[i]);

	fd_set readfds, writefds;
	FD_ZERO(&readfds); FD_ZERO(&writefds);
	timeval timeoutInterval;
	timeoutInterval.tv_sec = 10;
	timeoutInterval.tv_usec = 0;

	while (true)
	{
		FD_SET(listenSocket, &readfds);
		for (int i = 0; i < FD_SETSIZE; i++) {
			if (client[i].connSock > 0) FD_SET(client[i].connSock, &readfds);
		}
		writefds = readfds;
		nEvents = select(0, &readfds, 0, 0, 0);
		if (nEvents < 0) {
			printf("ERROR: cannot poll socket: %d\n", WSAGetLastError());
			break;
		}
		if (FD_ISSET(listenSocket, &readfds)) {
			clientAddrLen = sizeof(clientAddr);
			connSock = accept(listenSocket, (sockaddr*)&clientAddr, &clientAddrLen);
			int i;
			for (i = 0; i < FD_SETSIZE; i++) {
				if (client[i].connSock <= 0) {
					client[i].connSock = connSock;
					break;
				}
			}
			if (i == FD_SETSIZE) isThreadFull = 1;
			if (--nEvents <= 0) continue;
		}
		for (int i = 0; i < FD_SETSIZE; i++) {
			if (client[i].connSock <= 0) continue;
			ret = RECEIVE_TCP(client[i].connSock, buff, 0);
			if (ret <= 0) {
				if (ret == SOCKET_ERROR) printf("Connection shutdown\n");
				else if (ret == 0) printf("Client close connection\n");
				closesocket(client[i].connSock);
				InitiateSession(&client[i]);
				continue;
			}
			else if (ret > 0) {
				buff[ret] = 0;
				if (IsDomainNameOrIP(buff)) {
					ResolverDomainName(buff, &client[i]);
					ret = SendDomainName(client[i], 0);
					if (ret == SOCKET_ERROR) printf("cannot send\n");
				}
				else {
					ResolverIPAddress(buff, &client[i]);
					ret = SendIP(client[i], 0);
					if (ret == SOCKET_ERROR) printf("cannot send\n");
				}
			}
		}
	}
	return 0;
}
#pragma endregion

int main()
{
	WSADATA wsaData;
	WORD version = MAKEWORD(2, 2);
	if (WSAStartup(version, &wsaData)) {
		printf("version is not supported\n");
		return 0;
	}
	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	u_short serverPort = 0;
node1:
	printf("%s ", SERVER_EXE);
	scanf_s("%d", &serverPort);
	if (serverPort < 1) {
		printf("wrong port\n"); goto node1;
	}
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(serverPort);
	serverAddr.sin_addr.s_addr = inet_addr(SERVER_ADDR);
	if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr))) {
		printf("can not bind this address"); return 0;
	}
	if (listen(listenSocket, MAX_CLIENT)) {
		printf("can not listen"); return 0;
	}
	printf("SERVER START\n");
	isThreadFull = 1;
	while (true) {
		if (isThreadFull == 1) {
			_beginthreadex(0, 0, Handler, (void*)listenSocket, 0, 0);
			isThreadFull = 0;
		}
	}
}

