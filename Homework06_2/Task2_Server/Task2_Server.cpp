// server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <conio.h>
#include <stdio.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <SDKDDKVer.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <list>
#include <process.h>

#define SERVER_ADDR "127.0.0.1"
#define BUFF_SIZE 2048
#define SERVER_EXE "Task2_Server.exe"
#define MAX_CLIENT 1024
using namespace std;

#pragma comment(lib, "Ws2_32.lib")

#pragma region COMMON
//the infomation of resolved result
/*
status: 1-success, 2-get infomation fall, 3-not found infomation
type: 0-damain name, 1-IP
*/
struct INFO
{
	int status;
	int type;
	list<char*> address;
	char* hostName;
};

struct SESSION
{
	INFO info;
	SOCKET connSock;
	list<SESSION*>::iterator position;
};

//list of session
list<SESSION*> listSession;

int lockSession, isThreadFull;

//initiate session
void InitiateSession(SESSION* session) {
	session->connSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	session->connSock = 0;
	session->info.type = -1;
	session->info.status = -1;
	session->info.address.clear();
	session->info.hostName = new char[BUFF_SIZE];
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

int SEND_IP(SESSION session, int flag) {
	int ret; char* dest = new char[BUFF_SIZE];
	if (session.info.status == 1) {
		char* c = new char[3]{ "11" };
		ret = SEND_TCP(session.connSock, AddHeader(dest, c), flag);
		if (ret == SOCKET_ERROR) return SOCKET_ERROR;
		list<char*>::iterator pointer = session.info.address.begin();
		for (; pointer != session.info.address.end(); pointer++) {
			ret = SEND_TCP(session.connSock, AddHeader(dest, *pointer), flag);
			if (ret == SOCKET_ERROR) return SOCKET_ERROR;
		}
		c = new char[2]{ "0" };
		ret = SEND_TCP(session.connSock, AddHeader(dest, c), flag);
	}
	else if (session.info.status == 2) {
		char* temp = new char[3]{ "21" };
		ret = SEND_TCP(session.connSock, AddHeader(dest, temp), flag);
		if (ret == SOCKET_ERROR) return SOCKET_ERROR;
	}
	else if (session.info.status == 3) {
		char* temp = new char[3]{ "31" };
		ret = SEND_TCP(session.connSock, AddHeader(dest, temp), flag);
		if (ret == SOCKET_ERROR) return SOCKET_ERROR;
	}
	return ret;
}

int SEND_DOMAIN_NAME(SESSION session, int flag) {
	int ret; char* dest = new char[BUFF_SIZE];
	if (session.info.status == 1) {
		char* c = new char[3]{ "10" };
		ret = SEND_TCP(session.connSock, AddHeader(dest, c), flag);
		if (ret == SOCKET_ERROR) return SOCKET_ERROR;
		ret = SEND_TCP(session.connSock, AddHeader(dest, session.info.hostName), flag);
		if (ret == SOCKET_ERROR) return SOCKET_ERROR;
	}
	else if (session.info.status == 2) {
		char* temp = new char[3]{ "20" };
		ret = SEND_TCP(session.connSock, AddHeader(dest, temp), flag);
		if (ret == SOCKET_ERROR) return SOCKET_ERROR;
	}
	else if (session.info.status == 3) {
		char* temp = new char[3]{ "30" };
		ret = SEND_TCP(session.connSock, AddHeader(dest, temp), flag);
		if (ret == SOCKET_ERROR) return SOCKET_ERROR;
	}
	return ret;
}
#pragma endregion

#pragma region CHECK DATA FROM CLIENT
//Check the ip that the user entered
//return 0 if IP is corect, 1 if IP is incorrect
int CheckIP(char* IP) {
	int result = 0, index = 0, sum = 0, countPoint = 0;
	char element = IP[0];
	while (element != '\0') {
		if ('0' <= element && element <= '9') {
			sum = sum * 10 + (element - '0');
			element = IP[++index];
		}
		else if (element == '.') {
			countPoint++;
			if (sum > 255 || countPoint > 3) { result = 1; break; }
			else {
				sum = 0; element = IP[++index];
			}
		}
		else {
			result = 1; break;
		}
	}
	return result;
}
#pragma endregion

#pragma region RESOLVE
void ResolverDomainName(char* name, SESSION* session) {
	addrinfo* infoHostName = NULL;

	int check = getaddrinfo(name, "http", NULL, &infoHostName);
	if (check) { //get address infomation fall
		session->info.status = 2;
		session->info.address.clear();
	}
	if (infoHostName == NULL) { //not found infomation
		session->info.status = 3;
		session->info.address.clear();
		freeaddrinfo(infoHostName);
	}
	else
	{
		session->info.status = 1;
		while (infoHostName != NULL) {
			sockaddr_in* addrTemp = (sockaddr_in*)infoHostName->ai_addr;
			char* temp = inet_ntoa(addrTemp->sin_addr);
			session->info.address.push_back(temp);
			infoHostName = infoHostName->ai_next;
		}
		freeaddrinfo(infoHostName);
	}
}

void ResolverIPAddress(char* ip, SESSION* session) {
	char servInfo[NI_MAXSERV], hostName[NI_MAXHOST];
	sockaddr_in addr;
	u_short port = 1;

	addr.sin_addr.s_addr = inet_addr(ip);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	int check = getnameinfo((sockaddr*)&addr, sizeof(sockaddr), hostName, NI_MAXHOST,
		servInfo, NI_MAXSERV, NI_NUMERICSERV);
	if (check) {
		session->info.hostName = NULL;
	    session->info.status = 2;
	}
	if (ip == NULL || !strcmp(ip, hostName)) {
		session->info.hostName = NULL;
		session->info.status = 3;
	}
	else {
		strcpy_s(session->info.hostName, strlen(hostName) + 1, hostName);
		session->info.status = 1;
	}
}
#pragma endregion

#pragma region HANDLER MULTIPLE CLIENT
unsigned _stdcall CreateSession(void* param) {
	while (true)
	{
		if (lockSession == 0) {
			lockSession = 1;
			SESSION* session = (SESSION*)param;
			listSession.push_back(session);
			session->position = --listSession.end();
			lockSession = 0;
			break;
		}
	}
	return 0;
}

unsigned _stdcall ReleaseSession(void* param) {
	while (true)
	{
		if (lockSession == 0) {
			lockSession = 1;
			SESSION* session = (SESSION*)param;
			listSession.erase(session->position);
			closesocket(session->connSock);
			InitiateSession(session);
			lockSession = 0;
			break;
		}
	}
	return 0;
}

unsigned _stdcall Handler(void* param) {
	SOCKET listenSocket = (SOCKET)param;
	sockaddr_in clientAddr;
	char buff[BUFF_SIZE];
	char* result = new char[10];
	int ret, numberOfClient = 0, nEvents, clientAddrLen;

	SESSION client[FD_SETSIZE];
	SOCKET connSock;
	for (int i = 0; i < FD_SETSIZE; i++) InitiateSession(&client[i]);

	fd_set readfds, writefds;
	FD_ZERO(&readfds); FD_ZERO(&writefds);
	timeval timeoutInterval;
	timeoutInterval.tv_sec = 10;
	timeoutInterval.tv_usec = 0;

	while (true) {
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
			if (connSock != INVALID_SOCKET) {
				for (i = 0; i < FD_SETSIZE; i++) {
					if (client[i].connSock <= 0) {
						client[i].connSock = connSock;
						HANDLE hCrSession = (HANDLE)_beginthreadex(0, 0, CreateSession, (void*)&client[i], 0, 0);
						WaitForSingleObject(hCrSession, INFINITE);
						break;
					}
				}
			}
			else continue;
			if (i == FD_SETSIZE) isThreadFull = 1;
			if (--nEvents <= 0) continue;
		}
		for (int i = 0; i < FD_SETSIZE; i++) {
			if (client[i].connSock <= 0) continue;
			if (FD_ISSET(client[i].connSock, &readfds)) {
				ret = RECEIVE_TCP(client[i].connSock, buff, 0);
				if (ret <= 0) {
					if (ret == SOCKET_ERROR) printf("Connection shutdown\n");
					else if (ret == 0) printf("Client close connection\n");

					HANDLE hRelease = (HANDLE)_beginthreadex(0, 0, ReleaseSession, (void*)&client[i], 0, 0);
					WaitForSingleObject(hRelease, INFINITE);
					continue;
				}
				else if (ret > 0) {
					buff[ret] = 0;
					if (CheckIP(buff)) { //domain name
						ResolverDomainName(buff, &client[i]);
						ret = SEND_IP(client[i], 0);
						if (ret == SOCKET_ERROR) printf("can not send to client\n");
					}
					else { //ip
						ResolverIPAddress(buff, &client[i]);
						SEND_DOMAIN_NAME(client[i], 0);
						if (ret == SOCKET_ERROR) printf("can not send to client\n");
					}
				}
			}
		}
	}
	return 0;
}
#pragma endregion

int main() {
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

	lockSession = 0; isThreadFull = 1;
	while (true) {
		if (isThreadFull == 1) {
			_beginthreadex(0, 0, Handler, (void*)listenSocket, 0, 0);
			isThreadFull = 0;
		}
	}
}
