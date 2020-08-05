// Server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <conio.h>
#include <stdio.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <SDKDDKVer.h>

#define SERVER_ADDR "127.0.0.1"
#define BUFF_SIZE 2048
#define SERVER_EXE "server.exe"

#pragma comment(lib, "Ws2_32.lib")

int COUNT(char* input) {
	int index = 0;
	while (input[index] == ' ') index++;
	char a = input[++index];
	if ('0' <= a && a <= '9') return -1;
	else {
		int length = strlen(input), count = 0;
		while (index < length) {
			if ('0' <= input[index] && input[index] <= '9') return -1;
			else {
				if (input[index - 1] == ' ' && input[index] != ' ') count++;
				index++;
			}
		}
		return ++count;
	}
}

int AddBody(char* c, int begin, int number) {
	int index = begin;
	if (number == 0) {
		c[index] = '0';
		index++;
	}
	else {
		while (number > 0) {
			int temp = number % 10;
			number = number / 10;
			c[index] = temp + '0';
			index++;
		}
	}
	c[index] = 0;
	return 0;
}

int LENGTH(char* buff) {
	int result = 0, index = 9;
	while (!('0' <= buff[index] && buff[index] <= '9')) { index--; }
	for (int i = index; i >= 0; i--) {
		result = result * 10 + (buff[i] - '0');
	}
	return result;
}

int RECEIVE_TCP(SOCKET s, char* buff, int flag) {
	int index = 0, ret, result = 0;
	char* temp = new char[10];
	ret = recv(s, temp, 10, flag);
	int length = LENGTH(temp);
	while (length > 0) {
		ret = recv(s, &buff[index], length, 0);
		if (ret == SOCKET_ERROR) return SOCKET_ERROR;
		else result = ret;
		index += ret;
		length -= ret;
	}
	return result;
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

int _tmain(int argc, _TCHAR* argv[])
{
	WSADATA wsaData;
	WORD version = MAKEWORD(2, 2);
	if (WSAStartup(version, &wsaData)) {
		printf("version is not supported\n");
		return 0;
	}
	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	u_short serverPort = 0;
moc1:
	printf("%s ", SERVER_EXE);
	scanf_s("%d", &serverPort);
	if (serverPort < 1) {
		printf("wrong port\n"); goto moc1;
	}

	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(serverPort);
	serverAddr.sin_addr.s_addr = inet_addr(SERVER_ADDR);
	if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr))) {
		printf("can not bind this address"); return 0;
	}
	if (listen(listenSocket, 10)) {
		printf("can not listen"); return 0;
	}
	printf("SERVER START\n");
	sockaddr_in clientAddr;
	char buff[BUFF_SIZE], buffSend[BUFF_SIZE];
	int ret, clientAddrLen = sizeof(clientAddr);
moc2:	
	SOCKET connSock = accept(listenSocket, (sockaddr*)&clientAddr, &clientAddrLen);
	while (true) {
		ret = RECEIVE_TCP(connSock, buff, 0);
		if (ret == SOCKET_ERROR) {
			printf("error: %d", WSAGetLastError());
			break;
		}
		else if (strlen(buff) > 0) {
			buff[ret] = 0;
			if (!strcmp(buff, "")) {
				closesocket(connSock);
				goto moc2;
			}
			printf("Client send: %s\n", buff);
			char* rec; int count = COUNT(buff);
			if (count < 0) {
				printf("Send to client: String contain number\n");
				rec = new char[20];
				AddBody(rec, 0, 0);
			}
			else {
				printf("Send to client: Number of words: %d\n", count);
				rec = new char[20];
				rec[0] = '1';
				AddBody(rec, 1, count);
			}
			ret = SEND_TCP(connSock, rec, 0);
			printf("%s\n", rec);
			if (ret == SOCKET_ERROR) printf("error: %d\n", WSAGetLastError());
		}
	}
	closesocket(listenSocket);
	WSACleanup();
}
