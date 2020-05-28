// server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <conio.h>
#include <stdio.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <SDKDDKVer.h>
#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <process.h>

#define SERVER_ADDR "127.0.0.1"
#define BUFF_SIZE 2048
#define SERVER_EXE "server.exe"

#define RECEIVE 0
#define SEND 1
#define PORT 5500
#define DATA_BUFSIZE 8192
#define MAX_CLIENT 2
using namespace std;

#pragma comment(lib, "Ws2_32.lib")

#pragma region COMMON
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
/*  type: 0 - already logined, 1 - not logined, 2 - account is locked
location: location of line in the file
numberOfError: number of incorrect password attempts
*/
struct Account
{
	char* username;
	char* password;
	int numberOfError;
	int type;
	int location;
};

void Slip(char* input, char* username, char* password) {
	int index = 0, count = 0;
	while (input[index] != ' ') {
		username[count] = input[index];
		index++; count++;
	}
	username[count] = 0; count = 0;
	while (input[++index] != '\0') {
		password[count] = input[index];
		count++;
	}
	password[count] = 0;
}

void Slip(string input, char* username, char* password) {
	int index = 0, count = 0;
	while (input[index] != ' ') {
		username[count] = input[index];
		index++; count++;
	}
	username[count] = 0; count = 0;
	while (input[++index] != ' ') {
		password[count] = input[index];
		count++;
	}
	password[count] = 0;
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

#pragma region HANDLE LOGIN
char* CHECK_CLIENT_LOGIN(Account* account, char* username, char* password) {
	bool cmpUser = strcmp(account->username, username);
	bool cmpPass = strcmp(account->password, password);
	if (account->type == 0) return new char[4]{ "101" };
	else if (cmpUser) return new char[4]{ "110" };
	else if (!cmpUser && account->type == 1) {
		if (cmpPass) {
			account->numberOfError += 1;
			if (account->numberOfError > 3) {
				account->type = 3;
				account->numberOfError = 0;
				ofstream file; file.open("D:/Documents/VisualStudio/Lap_trinh_mang/Homework04/Task1_Server/account.txt", ios::in);
				file.seekp(account->location);
				file << "1"; file.close();
				return new char[4]{ "113" };
			}
			else return new char[4]{ "111" };
		}
		else {
			account->numberOfError = 0;
			account->type = 0;
			return new char[4]{ "100" };
		}
	}
	else return new char[4]{ "112" };
}

int CHECK_SINGE_ACCOUNT(string line, char* username, char* password) {
	int index = 0, count = 0, result = -1;
	int lenLine = line.length(), lenUser = strlen(username), lenPass = strlen(password);
	while (line[index] != ' ' && count < lenUser) {
		if (line[index] != username[count]) break;
		index++; count++;
	}
	if (line[index] != ' ' || count < lenUser) return 1; //username is not correct
	count = 0; index++;
	while (line[index] != ' ' && count < lenPass) {
		if (line[index] != password[count]) break;
		index++; count++;
	}
	if (line[index] != ' ' || count < lenPass) return 2; //password is not correct
	else if (line[lenLine - 1] == '1') return 3; //account is locked
	else return 0; //login success
}

char* LOGIN(Account* account, char* username, char* password) {
	string line; ifstream file; file.open("D:/Documents/VisualStudio/Lap_trinh_mang/Homework04/Task1_Server/account.txt", ios::out);
	int check = 1, index = 0;
	while (!file.eof()) {
		getline(file, line);
		check = CHECK_SINGE_ACCOUNT(line, username, password);
		if (check != 1) { file.close(); break; }
		index += line.length() + 2;
	}
	if (check == 1) return new char[4]{ "110" };
	else {
		Slip(line, username, password);
		account->location = index;
		strcpy_s(account->username, strlen(username) + 1, username);
		strcpy_s(account->password, strlen(password) + 1, password);
		switch (check)
		{
		case 0:
			account->type = 0;
			account->numberOfError = 0;
			return new char[4]{ "100" };
		case 2:
			account->type = 1;
			account->numberOfError += 1;
			return new char[4]{ "111" };
		case 3:
			account->type = 3;
			account->numberOfError = 0;
			return new char[4]{ "112" };
		}
	}
}
#pragma endregion

#pragma region HANDLE LOGOUT
char* LOGOUT(Account* account, char* username) {
	if (account->type != 0) return new char[4]{ "210" };
	else {
		if (!strcmp(account->username, username)) {
			account->username = new char[BUFF_SIZE];
			account->password = new char[BUFF_SIZE];
			account->numberOfError = 0;
			account->type = 1;
			return new char[4]{ "200" };
		}
		else return new char[4]{ "211" };
	}
}
#pragma endregion

#pragma region CHECK DATA FROM CLIENT
int CheckDataFromClient(char* buff) {
	int length = strlen(buff);
	if (buff[0] != '1' && buff[0] != '2') return 1;
	else {
		int count = 0;
		for (int i = 1; i < length; i++) {
			if (buff[i] == ' ') count++;
			if (count > 1) return 1;
		}
		return 0;
	}
}
#pragma endregion

int main() {
	WSADATA wsaData;
	WORD version = MAKEWORD(2, 2);
	WSAEVENT acceptEvent;
	InitializeCriticalSection(&criticalSection);
	if (WSAStartup(version, &wsaData)) {
		printf("version is not supported\n");
		return 0;
	}
	SOCKET listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (listenSocket == INVALID_SOCKET) {
		printf("Failed to get a socket %d\n", WSAGetLastError());
		return 1;
	}
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
	sockaddr_in clientAddr;
	char buff[BUFF_SIZE], buffSend[BUFF_SIZE];
	int ret, clientAddrLen = sizeof(clientAddr);
	if (bind(listenSocket, (PSOCKADDR)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		printf("bind() failed with error %d\n", WSAGetLastError());
		return 1;
	}
	
	if (listen(listenSocket, 10)) {
		printf("can not listen"); return 0;
	}
	printf("SERVER START\n");

	if ((acceptEvent = WSACreateEvent()) == WSA_INVALID_EVENT) {
		printf("WSACreateEvent() failed with error %d\n", WSAGetLastError());
		return 1;
	}

	_beginthreadex(0, 0, workerThread, (LPVOID)acceptEvent, 0, 0);

	while (TRUE) {
		if ((acceptSocket = accept(listenSocket, (PSOCKADDR)&clientAddr, &clientAddrLen)) == SOCKET_ERROR) {
			printf("accept() failed with error %d\n", WSAGetLastError());
			return 1;
		}

		if (WSASetEvent(acceptEvent) == FALSE) {
			printf("WSASetEvent() failed with error %d\n", WSAGetLastError());
			return 1;
		}
	}






	

	/*SOCKET connSock = accept(listenSocket, (sockaddr*)&clientAddr, &clientAddrLen);
	Account account; account.username = new char[BUFF_SIZE];
	account.password = new char[BUFF_SIZE];
	account.numberOfError = 0; account.type = 1; 
	account.location = -1;
	char username[2048], password[2048];
	char* result = new char[10];
	while (true) {
		ret = RECEIVE_TCP(connSock, buff, 0);
		if (ret == SOCKET_ERROR) {
			printf("Connection shutdown\n");
			if (account.type == 0) {
				printf("The username: %s has not logged out, will perform automatic logout\n", account.username);
			}
			closesocket(connSock);
			break;
		}
		else if (ret == 0) {
			printf("client close connection\n");
			if (account.type == 0) {
				printf("The username: %s has not logged out, will perform automatic logout\n", account.username);
			}
			closesocket(connSock);
			break;
		}
		else if (ret > 0) {
			buff[ret] = 0;
			if (CheckDataFromClient(buff)) {
				strcpy_s(result, 4, "001");
				int ret = SEND_TCP(connSock, AddHeader(buffSend, result), 0);
				if (ret == SOCKET_ERROR) printf("can not send message\n");
			}
			else {
				if (buff[0] == '1') {
					Slip(&buff[1], username, password);
					printf("Request: Login[username: %s, password: %s]\n", username, password);
					result = CHECK_CLIENT_LOGIN(&account, username, password);
					if (!strcmp(result, "110")) {
						result = LOGIN(&account, username, password);
					}
				}
				else if (buff[0] == '2') {
					printf("Request: Logout[username: %s]\n", &buff[1]);
					result = LOGOUT(&account, &buff[1]);
				}

				printf("%s\n", result);
				int ret = SEND_TCP(connSock, AddHeader(buffSend, result), 0);
				if (ret == SOCKET_ERROR) printf("can not send message\n");
			}
		}
	}
	closesocket(listenSocket);
	WSACleanup();*/
}
