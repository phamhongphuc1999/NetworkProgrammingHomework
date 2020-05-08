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
struct SESSION {
	int status;
	int type;
	addrinfo* address;
	char* hostName;
};
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

//Packed with login module
char* EncapsulateLogin(char* buff, char* username, char* password) {
	buff[0] = '1'; buff[1] = 0;
	strcat_s(buff, strlen(buff) + strlen(username) + 1, username);
	int length = strlen(buff);
	buff[length] = ' '; buff[length + 1] = 0;
	strcat_s(buff, strlen(buff) + strlen(password) + 1, password);
	return buff;
}

//package with logout module
char* EncapsulateLogout(char* buff, char* username) {
	buff[0] = '2'; buff[1] = 0;
	strcat_s(buff, strlen(buff) + strlen(username) + 1, username);
	return buff;
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
SESSION ResolverDomainName(char* name) {
	addrinfo* infoHostName = NULL, *temp = NULL;
	SESSION session;
	session.type = 1; 
	session.hostName = NULL;

	int check = getaddrinfo(name, "http", NULL, &infoHostName);
	if (check) { //get address infomation fall
		session.status = 2;
		session.address = NULL;
		return session;
	}
	temp = infoHostName;
	if (temp == NULL) { //not found infomation
		freeaddrinfo(infoHostName);
		session.status = 3;
		session.address = NULL;
		return session;
	}
	else
	{
		freeaddrinfo(infoHostName);
		session.status = 1;
		session.address = temp;
		return session;
	}
}

SESSION ResolverIPAddress(char* ip) {
	char serverInfo[NI_MAXSERV], hostName[NI_MAXHOST];
	SESSION session; 
	session.address = NULL; 
	session.type = 0;
	sockaddr_in addr;
	u_short port = 1;

	addr.sin_addr.s_addr = inet_addr(ip);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	int check = getnameinfo((sockaddr*)&addr, sizeof(sockaddr), hostName, NI_MAXHOST,
		serverInfo, NI_MAXSERV, NI_NUMERICSERV);
	if (check) {
		session.hostName = NULL;
		session.status = 2;
	}
	if (ip == NULL || !strcmp(ip, hostName)) {
		session.hostName = NULL;
		session.status = 3;
	}
	else {
		session.hostName = hostName;
		session.status = 1;
	}
	return session;
}
#pragma endregion

int main()
{
    
}

