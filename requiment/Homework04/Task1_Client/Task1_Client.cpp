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

#pragma region COMMON
char** messServer = new char*[10]{ "001", "100", "101", "110", "111", "112", "113", "200", "210", "211" };

char** MESSAGE = new char*[10]{ "something wrong, please send again", "login successful", "already logined",
"username is incorrect", "password is incorrect", "account is locked",
"the account has just been locked because of incorrect login three times",
"logout successful", "not logged in, so cannot log out",
"not logged out because the username is incorrect" };
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
			else result = ret;
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

#pragma region INPUT AND DATA
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
	return (i > 0) && !CheckIP(name);
}

bool IsContainSpaceOrEmpty(char* input) {
	bool result = false;
	int length = strlen(input);
	if (length == 0) return true;
	for (int i = 0; i < length; i++) {
		if (input[i] == ' ') {
			result = true;
			break;
		}
	}
	return result;
}
#pragma endregion

#pragma region CONSOLE
void GetChar(char* input, char* more) {
more:
	printf("%s: ", more);
	gets_s(input, BUFF_SIZE);
	if (IsContainSpaceOrEmpty(input)) {
		printf("%s not contain space or empty\n", more);
		goto more;
	}
}

void GetData(char* username, char* password, char* temp) {
more:
	printf("choice function: ");
	gets_s(temp, BUFF_SIZE);
	if (!strcmp(temp, "1")) {
		GetChar(username, "username");
		GetChar(password, "password");
	}
	else if (!strcmp(temp, "2")) {
		GetChar(username, "username");
	}
	else if (strcmp(temp, "0")) {
		printf("ERROR: wrong input\n");
		goto more;
	}
}

void PrintMessageFromServer(char* message) {
	for (int i = 0; i < 10; i++) {
		if (!strcmp(message, messServer[i])) {
			printf("%s\n", MESSAGE[i]);
			break;
		}
	}
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
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(serverPort);
	serverAddr.sin_addr.s_addr = inet_addr(serverAddrIpv4);
	if (connect(client, (sockaddr*)&serverAddr, sizeof(serverAddr))) {
		printf("can not connect server %d", WSAGetLastError());
		return 0;
	}
	printf("connected server\n");
	printf("=================== press 1 to login ==============================\n");
	printf("=================== press 2 to logout =============================\n");
	printf("================ press 0 to close connection ======================\n");
	while (true) {
		char temp[BUFF_SIZE], buff[BUFF_SIZE], dest[BUFF_SIZE], username[BUFF_SIZE], password[BUFF_SIZE];
		fflush(stdin);
		GetData(username, password, temp);
		if (!strcmp(temp, "1")) EncapsulateLogin(buff, username, password);
		else if (!strcmp(temp, "2")) EncapsulateLogout(buff, username);
		else if (!strcmp(temp, "0")) break;

		int ret = SEND_TCP(client, AddHeader(dest, buff), 0);
		if (ret == SOCKET_ERROR) printf("can not send message\n");

		ret = RECEIVE_TCP(client, buff, 0);
		if (ret == SOCKET_ERROR) {
			printf("ERROR: %d", WSAGetLastError());
			break;
		}
		buff[ret] = 0;
		PrintMessageFromServer(buff);
	}
	closesocket(client);
	WSACleanup();
}
