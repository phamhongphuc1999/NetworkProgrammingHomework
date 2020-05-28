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
#include <fstream>
#include <string>
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

SOCKET acceptSocket;
LPSOCKET_INFORMATION clients[MAX_CLIENT];
int nClients = 0;
CRITICAL_SECTION criticalSection;

/*  type: 0 - already logined, 1 - not logined, 2 - account is locked
location: location of line in the file
numberOfError: number of incorrect password attempts
*/
struct SESSION
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
char* CHECK_CLIENT_LOGIN(SESSION* session, char* username, char* password) {
	bool cmpUser = strcmp(session->username, username);
	bool cmpPass = strcmp(session->password, password);
	if (session->type == 0) return new char[4]{ "101" };
	else if (cmpUser) return new char[4]{ "110" };
	else if (!cmpUser && session->type == 1) {
		if (cmpPass) {
			session->numberOfError += 1;
			if (session->numberOfError > 3) {
				session->type = 3;
				session->numberOfError = 0;
				ofstream file; file.open("D:/Documents/VisualStudio/Lap_trinh_mang/Homework04/Task1_Server/account.txt", ios::in);
				file.seekp(session->location);
				file << "1"; file.close();
				return new char[4]{ "113" };
			}
			else return new char[4]{ "111" };
		}
		else {
			session->numberOfError = 0;
			session->type = 0;
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

char* LOGIN(SESSION* session, char* username, char* password) {
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
		session->location = index;
		strcpy_s(session->username, strlen(username) + 1, username);
		strcpy_s(session->password, strlen(password) + 1, password);
		switch (check)
		{
		case 0:
			session->type = 0;
			session->numberOfError = 0;
			return new char[4]{ "100" };
		case 2:
			session->type = 1;
			session->numberOfError += 1;
			return new char[4]{ "111" };
		case 3:
			session->type = 3;
			session->numberOfError = 0;
			return new char[4]{ "112" };
		}
	}
}
#pragma endregion

#pragma region HANDLE LOGOUT
char* LOGOUT(SESSION* session, char* username) {
	if (session->type != 0) return new char[4]{ "210" };
	else {
		if (!strcmp(session->username, username)) {
			session->username = new char[BUFF_SIZE];
			session->password = new char[BUFF_SIZE];
			session->numberOfError = 0;
			session->type = 1;
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

#pragma region Overlapped
void CALLBACK WorkerRoutine(DWORD error, DWORD transferredBytes, LPWSAOVERLAPPED overlapped, DWORD inFlags) {
	DWORD sendBytes, recvBytes;
	DWORD flags;
	LPSOCKET_INFORMATION sockInfo = (LPSOCKET_INFORMATION)overlapped;
	if (error != 0) printf("I/O operation failed with error %d\n", error);
	if (transferredBytes == 0) printf("Closing socket %d\n\n", sockInfo->sockfd);
	if (error != 0 || transferredBytes == 0) {
		//Find and remove socket
		EnterCriticalSection(&criticalSection);
		int index;
		for (index = 0; index < nClients; index++)
			if (clients[index]->sockfd == sockInfo->sockfd) break;

		closesocket(clients[index]->sockfd);
		GlobalFree(clients[index]);

		for (int i = index; i < nClients - 1; i++) clients[i] = clients[i + 1];
		nClients--;
		LeaveCriticalSection(&criticalSection);
		return;
	}
	if (sockInfo->operation == RECEIVE) {
		sockInfo->recvBytes = transferredBytes;
		sockInfo->sentBytes = 0;
		sockInfo->operation = SEND;
	}
	else sockInfo->sentBytes += transferredBytes;
	if (sockInfo->recvBytes > sockInfo->sentBytes) {
		ZeroMemory(&(sockInfo->overlapped), sizeof(WSAOVERLAPPED));
		sockInfo->dataBuff.buf = sockInfo->buff + sockInfo->sentBytes;
		sockInfo->dataBuff.len = sockInfo->recvBytes - sockInfo->sentBytes;
		sockInfo->operation = SEND;
		if (WSASend(sockInfo->sockfd, &(sockInfo->dataBuff), 1, &sendBytes, 0, &(sockInfo->overlapped), WorkerRoutine) == SOCKET_ERROR) {
			if (WSAGetLastError() != WSA_IO_PENDING) {
				printf("WSASend() failed with error %d\n", WSAGetLastError());
				return;
			}
		}
	}
	else {
		sockInfo->recvBytes = 0;
		flags = 0;
		ZeroMemory(&(sockInfo->overlapped), sizeof(WSAOVERLAPPED));
		sockInfo->dataBuff.len = DATA_BUFSIZE;
		sockInfo->dataBuff.buf = sockInfo->buff;
		sockInfo->operation = RECEIVE;
		if (WSARecv(sockInfo->sockfd, &(sockInfo->dataBuff), 1, &recvBytes, &flags, &(sockInfo->overlapped), WorkerRoutine) == SOCKET_ERROR) {
			if (WSAGetLastError() != WSA_IO_PENDING) {
				printf("WSARecv() failed with error %d\n", WSAGetLastError());
				return;
			}
		}
	}
}

unsigned _stdcall WorkerThread(LPVOID lpParameter) {
	DWORD flags;
	WSAEVENT events[1];
	DWORD index;
	DWORD recvBytes;

	events[0] = (WSAEVENT)lpParameter;
	while (true)
	{
		index = WSAWaitForMultipleEvents(1, events, FALSE, WSA_INFINITE, TRUE);
		if (index == WSA_WAIT_FAILED) {
			printf("WSAWaitForMultipleEvents() failed with error %d\n", WSAGetLastError());
			return 1;
		}
		if (index != WAIT_IO_COMPLETION) break;

		WSAResetEvent(events[index - WSA_WAIT_EVENT_0]);
		EnterCriticalSection(&criticalSection);
		if (nClients == MAX_CLIENT) {
			printf("Too many clients.\n");
			closesocket(acceptSocket);
			continue;
		}
		if ((clients[nClients] = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR, sizeof(SOCKET_INFORMATION))) == NULL) {
			printf("GlobalAlloc() failed with error %d\n", GetLastError());
			return 1;
		}

		clients[nClients]->sockfd = acceptSocket;
		ZeroMemory(&(clients[nClients]->overlapped), sizeof(WSAOVERLAPPED));
		clients[nClients]->sentBytes = 0;
		clients[nClients]->recvBytes = 0;
		clients[nClients]->dataBuff.len = DATA_BUFSIZE;
		clients[nClients]->dataBuff.buf = clients[nClients]->buff;
		clients[nClients]->operation = RECEIVE;
		flags = 0;

		if (WSARecv(clients[nClients]->sockfd, &(clients[nClients]->dataBuff), 1, &recvBytes,
			&flags, &(clients[nClients]->overlapped), WorkerRoutine) == SOCKET_ERROR) {
			if (WSAGetLastError() != WSA_IO_PENDING) {
				printf("WSARecv() failed with error %d\n", WSAGetLastError());
				return 1;
			}
		}
		printf("Socket %d got connected...\n", acceptSocket);
		nClients++;
		LeaveCriticalSection(&criticalSection);
	}
	return 0;
}
#pragma endregion


int main() {
	WSADATA wsaData;
	WSAEVENT acceptEvent;
	u_short serverPort = 0;
	WORD version = MAKEWORD(2, 2);
	sockaddr_in clientAddr;
	char buff[BUFF_SIZE], buffSend[BUFF_SIZE];
	int ret, clientAddrLen = sizeof(clientAddr);
	if (WSAStartup(version, &wsaData)) {
		printf("version is not supported\n");
		return 0;
	}
	InitializeCriticalSection(&criticalSection);
	SOCKET listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (listenSocket == INVALID_SOCKET) {
		printf("Failed to get a socket %d\n", WSAGetLastError());
		return 0;
	}
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
	if (bind(listenSocket, (PSOCKADDR)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		printf("can not bind this address"); return 0;
	}
	if (listen(listenSocket, 10)) {
		printf("listen() failed with error %d\n", WSAGetLastError());
		return 0;
	}
	printf("SERVER START\n");

	if ((acceptEvent = WSACreateEvent()) == WSA_INVALID_EVENT) {
		printf("WSACreateEvent() failed with error %d\n", WSAGetLastError());
		return 1;
	}

	_beginthreadex(0, 0, WorkerThread, (LPVOID)acceptEvent, 0, 0);

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
	return 0;
}
