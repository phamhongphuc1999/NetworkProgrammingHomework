// Client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <conio.h>
#include <stdio.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <SDKDDKVer.h>

#define CLIENT_EXE "client.exe"
#define SERVER_PORT 5500
#define SERVER_ADDR "127.0.0.1"
#define BUFF_SIZE 2048

#pragma comment(lib, "Ws2_32.lib")

int LENGTH(char* buff) {
	int result = 0, index = 9;
	while (!('0' <= buff[index] && buff[index] <= '9')) { index--; }
	for (int i = index; i >= 0; i--) {
		result = result * 10 + (buff[i] - '0');
	}
	return result;
}

char* AddHeader(char* c, char* buff) {
	int a = strlen(buff);
	int index = 0;
	while (a > 0) {
		int temp = a % 10;
		a = a / 10;
		c[index] = temp + '0';
		index++;
	}
	for (int i = index; i < 10; i++) {
		c[i] = 'e';
	}
	c[10] = 0;
	strcat_s(c, strlen(buff) + strlen(c) + 1, buff);
	return c;
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

int IsDomainName(char* name) {
	int result = 0, index = 0, sum = 0, countPoint = 0;
	char element = name[0];
	while (element != '\0') {
		if ('0' <= element && element <= '9') {
			sum = sum * 10 + (element - '0');
			element = name[++index];
		}
		else if (element == '.') {
			countPoint++;
			if (sum > 255 || countPoint > 3) { result = 1; break; }
			else {
				sum = 0; element = name[++index];
			}
		}
		else {
			result = 1; break;
		}
	}
	return result;
}

bool CheckInput(char* input, u_short* port, char* name) {
	int index = 0, count = 0, length = strlen(input);
	while (input[index++] == ' ' && index < length); index--;
	while (index < length && input[index] != ' ') {
		name[count] = input[index];
		index++; count++;
	}
	name[count++] = 0; u_short i = 0;
	while (input[index++] == ' ' && index < length); index--;
	while (index < length && '0' <= input[index] && '9' >= input[index]) {
		u_short temp = (u_short)(input[index] - '0');
		i = i * 10 + temp;
		index++;
	}
	*port = i;
	return (i > 0 && !IsDomainName(name));
}

int _tmain(int argc, _TCHAR* argv[]) {
	WSADATA wsaData;
	WORD version = MAKEWORD(2, 2);
	if (WSAStartup(version, &wsaData)) {
		printf("version is not supported\n");
		return 0;
	}
	SOCKET client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	int tv = 10000;
	setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (const char*)(&tv), sizeof(int));
	char* temp = new char[BUFF_SIZE];
	char* serverAddrIpv4 = new char[BUFF_SIZE];
moc1:
	u_short serverPort = 0;
	fflush(stdin); printf("%s: ", CLIENT_EXE);
	gets_s(temp, BUFF_SIZE);
	if (!CheckInput(temp, &serverPort, serverAddrIpv4)) {
		printf("Wrong input\n"); goto moc1;
	}

	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(serverPort);
	serverAddr.sin_addr.s_addr = inet_addr(serverAddrIpv4);
	if (connect(client, (sockaddr*)&serverAddr, sizeof(serverAddr))) {
		printf("can not connect server %d", WSAGetLastError());
		return 0;
	}
	printf("connected server\n");
	while (true) {
		char buff[2048], c[2048];
		fflush(stdin); printf("Send to server: ");
		gets_s(buff, BUFF_SIZE);
		if (!strcmp(buff, "")) break;
		char* rec = AddHeader(c, buff);
		int ret = SEND_TCP(client, AddHeader(c, buff), 0);
		if (ret == SOCKET_ERROR) printf("can not send message\n");
		ret = recv(client, buff, BUFF_SIZE, 0);
		if (ret == SOCKET_ERROR) {
			if (WSAGetLastError() == WSAETIMEDOUT) printf("time out");
			else printf("can not receive message\n");
		}
		else if (strlen(buff) > 0) {
			buff[ret] = 0;
			if (buff[0] == '0') printf("Error: String contains number.\n");
			else {
				int result = LENGTH(&buff[1]);
				printf("Number of words: %d\n", result);
			}
		}
	}
	closesocket(client);
	WSACleanup();
}