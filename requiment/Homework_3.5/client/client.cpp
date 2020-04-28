// client.cpp : Defines the entry point for the console application.
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
#define CLIENT_EXE "client.exe"

#pragma comment(lib, "Ws2_32.lib")


int LENGTH(char* buff) {
	int result = 0, index = 9;
	while (!('0' <= buff[index] && buff[index] <= '9')) { index--; }
	for (int i = index; i >= 0; i--) {
		result = result * 10 + (buff[i] - '0');
	}
	return result;
}

char* AddHeader(char* c, char* buff, int function) {
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
	c[10] = function + '0'; c[11] = 0;
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

bool CheckConnect(char* input, u_short* port, char* name) {
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
	return (i > 0);
}

int CheckInput(char* input, char* method, char* username, char* password) {
	int index = 0, count = 0, length = strlen(input), result;
	while (input[index++] == ' ' && index < length); index--;
	while (index < length && input[index] != ' ') {
		method[count] = input[index];
		index++; count++;
	}
	method[count] = 0; count = 0;
	if (!strcmp(method, "login")) result = 1;
	else if (!strcmp(method, "logout")) result = 2;
	else if (!strcmp(method, "exit")) result = 3;
	else return 0;
	while (input[index++] == ' ' && index < length); index--;
	while (index < length && input[index] != ' ') {
		username[count] = input[index];
		index++; count++;
	}
	username[count] = 0; count = 0;
	while (input[index++] == ' ' && index < length); index--;
	while (index < length && input[index] != ' ') {
		password[count] = input[index];
		index++; count++;
	}
	password[count] = 0;
	if (result == 1 && (!strcmp(username, "") || !strcmp(password, ""))) return -1;
	else if (result == 2 && !strcmp(username, "")) return -2;
	else return result;

}

char* LOGIN(char* buff, char* username, char* password) {
	buff[0] = 0;
	strcat_s(buff, strlen(buff) + strlen(username) + 1, username);
	int length = strlen(buff);
	buff[length] = ' '; buff[length + 1] = 0;
	strcat_s(buff, strlen(buff) + strlen(password) + 1, password);
	return buff;
}

char* LOGOUT(char* buff, char* username) {
	strcpy_s(buff, strlen(username) + 1, username);
	return buff;
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

int main()
{
	WSADATA wsaData;
	WORD version = MAKEWORD(2, 2);
	if (WSAStartup(version, &wsaData)) {
		printf("version is not supported\n");
		return 0;
	}
	SOCKET client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	int tv = 10000;
	setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (const char*)(&tv), sizeof(int));
moc1:
	char* temp = new char[BUFF_SIZE];
	char* serverAddrIpv4 = new char[BUFF_SIZE];
	u_short serverPort = 0;
	fflush(stdin); printf("%s: ", CLIENT_EXE);
	gets_s(temp, BUFF_SIZE);
	if (!CheckConnect(temp, &serverPort, serverAddrIpv4)) {
		printf("Wrong input\n"); goto moc1;
	}
	else if (IsDomainName(serverAddrIpv4)) {
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
		char temp[2048], buff[2048], c[2048], method[2048], username[2048], password[2048];
		fflush(stdin);
	moc2:
		gets_s(temp, BUFF_SIZE);
		if (!strcmp(temp, "")) break;
		int function = CheckInput(temp, method, username, password);
		switch (function)
		{
		case 0:
			printf("ERROR: wrong input\n");
			goto moc2;
		case -1:
			printf("ERROR: login username password\n");
			goto moc2;
		case -2:
			printf("ERROR: logout username\n");
			goto moc2;
		case 1: LOGIN(buff, username, password); break;
		case 2: LOGOUT(buff, username); break;
		case 3: buff[0] = 0; break;
		}
		int ret = SEND_TCP(client, AddHeader(c, buff, function), 0);
		if (ret == SOCKET_ERROR) printf("can not send message\n");
	}
	closesocket(client);
	WSACleanup();
}
