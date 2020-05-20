// UDPClient.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define CLIENT_EXE "client.exe"
#define SERVER_ADDR "127.0.0.1"
#define BUFF_SIZE 3000

#include <SDKDDKVer.h>
#include <stdio.h>
#include <conio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <tchar.h>
#include <iostream>
using namespace std;

#pragma comment(lib, "Ws2_32.lib")

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

int ReceiveFromServer(SOCKET s, int flag, sockaddr_in from_in, int* fromlen) {
	char* buff = new char[BUFF_SIZE];
	sockaddr* from = (sockaddr*)&from_in;
	int ret = recvfrom(s, buff, BUFF_SIZE, flag, from, fromlen);
	buff[ret] = 0;
	if (buff[1] == '1') {
		switch (buff[0])
		{
		case '1':
			while (true) {
				buff = new char[BUFF_SIZE];
				ret = recvfrom(s, buff, BUFF_SIZE, flag, from, fromlen);
				buff[ret] = 0;
				if (!strcmp(buff, "0")) break;
				printf("Receive from server[%s: %d] %s\n", inet_ntoa(from_in.sin_addr),
					ntohs(from_in.sin_port), buff);
			} break;
		case '2':
			printf("get address infomation fall\n");
			break;
		case '3':
			printf("not found infomation\n");
			break;
		}
	}
	else if (buff[1] == '0') {
		switch (buff[0])
		{
		case '1':
			buff = new char[BUFF_SIZE];
			ret = recvfrom(s, buff, BUFF_SIZE, flag, from, fromlen);
			buff[ret] = 0;
			printf("Receive from server[%s: %d] %s\n", inet_ntoa(from_in.sin_addr),
				ntohs(from_in.sin_port), buff);
			break;
		case '2':
			printf("get ip infomation fall\n");
			break;
		case '3':
			printf("not found infomation\n");
			break;
		}
	}
	return ret;
}

int _tmain(int argc, _TCHAR* argv[])
{
	WSADATA wsaData;
	WORD version = MAKEWORD(2, 2);
	int check = WSAStartup(version, &wsaData);
	if (check) {
		printf("WSAStartup fall");
		return 0;
	}
	SOCKET client = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	int tv = 10000;
	setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (const char*)(&tv), sizeof(int));

	sockaddr_in serverAddr;
	u_short serverPort = 0;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(1000);
	serverAddr.sin_addr.s_addr = inet_addr(SERVER_ADDR);
	char* temp = new char[BUFF_SIZE];
	
	printf("press empty string to close window\n");
	while (true) {
		moc1:
		char* buff = new char[BUFF_SIZE];
		
		printf("%s ", CLIENT_EXE);
		gets_s(temp, BUFF_SIZE);
		check = CheckInput(temp, &serverPort, buff);
		if (!strcmp(buff, "")) break;
		if (!check) {
			printf("wrong input string or wrong server port\n");
			goto moc1;
		}
		serverAddr.sin_port = htons(serverPort);
		int serverAddrLen = sizeof(serverAddr);
		int ret = sendto(client, buff, strlen(buff), 0, (sockaddr*)&serverAddr, serverAddrLen);
		if (ret == SOCKET_ERROR) printf("SOCKET ERROR: %d\n", WSAGetLastError());
		ret = ReceiveFromServer(client, 0, serverAddr, &serverAddrLen);
		if (ret == SOCKET_ERROR) {
			if (WSAGetLastError() == WSAETIMEDOUT) printf("time out\n");
			else printf("SOCKET ERROR: %d\n", WSAGetLastError());
		}
	}
	closesocket(client);
	WSACleanup();
	return 0;
}
