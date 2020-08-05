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

#define SERVER_ADDR "127.0.0.1"
#define BUFF_SIZE 2048
#define SERVER_EXE "server.exe"
using namespace std;

#pragma comment(lib, "Ws2_32.lib")

struct HandlerLockAccount {
	string line;
	int count;
};

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
	int length = LENGTH(temp) + 1;
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

int CHECK_LOGIN(char* buff, string line) {
	int space = 0, index = 0;
	int lenbuff = strlen(buff);
	int lenline = line.length();
	while (index < lenbuff && index < lenline) {
		if (buff[index] != line[index]) break;
		if (buff[index] == ' ') space = 1;
		index++;
	}
	if (space == 0) return 0; //username not equa
	else {
		if ((lenline - lenbuff != 2) || (index < lenbuff)) return 2; //password not correct
		else return 1; //login success
	}
}

char* LOGIN(char* buff, HandlerLockAccount* h) {
	fstream f; f.open("account.txt", ios::in && ios::out);
	int index = 0, result = 0;
	bool username = false, password = false;
	string line;
	while (!f.eof())
	{
		getline(f, line);
		result = CHECK_LOGIN(buff, line);
		if (result != 0) break;
	}f.close();
	if (result == 0) return "100"; //not found username
	else if (result == 2) {
		if (h->line != line) {
			h->line = line; h->count = 0;
		}
		else {
			h->count += 1;
			if (h->count == 3) line[line.length() - 1] = '1';
		}
		return "102";
	} //password is not correct
	else {
		if (line[line.length() - 1] == '0') return "101"; //login success
		else return "103"; //account is locked
	}
}

//char* LOGOUT(char* buff) {
//
//}
//
//char* EXIT(char* buff) {
//
//}

char* Handler(char* buff, HandlerLockAccount h) {
	if (buff[0] == '1') return LOGIN(&buff[1], &h);
	else if (buff[0] == '2') return "0";
	else if (buff[0] == '3') return "0";
	else return "000";
}

int main() {
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
	HandlerLockAccount h; h.count = 0; h.line = "";
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
			printf("Client send: %s\n", Handler(buff, h));
		}
	}
	closesocket(listenSocket);
	WSACleanup();
}
