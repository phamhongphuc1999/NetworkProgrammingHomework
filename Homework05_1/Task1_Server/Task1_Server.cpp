// server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <conio.h>
#include <stdio.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <SDKDDKVer.h>
#include <fstream>
#include <string>
#include <list>
#include <process.h>

#define SERVER_ADDR "127.0.0.1"
#define BUFF_SIZE 2048
#define SERVER_EXE "Task2_Server.exe"
#define MAX_CLIENT 1024
using namespace std;

#pragma comment(lib, "Ws2_32.lib")

#pragma region COMMON
/*  type: 0 - already logined, 1 - not logined, 2 - session is locked
location: location of line in the file
numberOfError: number of incorrect password attempts
*/
struct ACCOUNT
{
	char* username;
	char* password;
	int numberOfError;
	int type;
	int location;
};

//position: the pointer point in the location in listSession
struct SESSION
{
	ACCOUNT account;
	list<SESSION*>::iterator position;
};

struct PARAM {
	char* username;
	char* password;
	SESSION* session;
	char* result;
};

//list of session
list<SESSION*> listSession;

int lockSession;

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

#pragma region HANDLE LOGIN
char* CHECK_CURRENT_SESSION(SESSION* session, char* username, char* password) {
	bool cmpUser = strcmp(session->account.username, username);
	bool cmpPass = strcmp(session->account.password, password);
	if (session->account.type == 0) return new char[4]{ "101" };
	else if (cmpUser) return new char[4]{ "110" };
	else if (!cmpUser && session->account.type == 1) {
		if (cmpPass) {
			session->account.numberOfError += 1;
			if (session->account.numberOfError > 3) {
				session->account.type = 3;
				session->account.numberOfError = 0;
				ofstream file; file.open("account.txt", ios::in);
				file.seekp(session->account.location);
				file << "1"; file.close();
				return new char[4]{ "113" };
			}
			else return new char[4]{ "111" };
		}
		else {
			session->account.numberOfError = 0;
			session->account.type = 0;
			return new char[4]{ "100" };
		}
	}
	else return new char[4]{ "112" };
}

char* CHECK_OTHER_SESSION(SESSION* session, char* username) {
	for (list<SESSION*>::iterator item = listSession.begin(); item != listSession.end(); item++) {
		SESSION* otherSession = *item;
		if (otherSession != session && otherSession->account.type == 0) {
			if (!strcmp(otherSession->account.username, username)) return new char[4]{ "102" };
		}
	}
	return new char[4]{ "110" };
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
	else if (line[lenLine - 1] == '1') return 3; //session is locked
	else return 0; //login success
}

char* LOGIN(SESSION* session, char* username, char* password) {
	string line;
	ifstream file; file.open("account.txt", ios::out);
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
		session->account.location = index + strlen(username) + strlen(password) + 2;
		strcpy_s(session->account.username, strlen(username) + 1, username);
		strcpy_s(session->account.password, strlen(password) + 1, password);
		switch (check)
		{
		case 0:
			session->account.type = 0;
			session->account.numberOfError = 0;
			return new char[4]{ "100" };
		case 2:
			session->account.type = 1;
			session->account.numberOfError += 1;
			return new char[4]{ "111" };
		case 3:
			session->account.type = 3;
			session->account.numberOfError = 0;
			return new char[4]{ "112" };
		}
	}
}
#pragma endregion

#pragma region HANDLE LOGOUT
char* LOGOUT(SESSION* session, char* username) {
	if (session->account.type != 0) return new char[4]{ "210" };
	else {
		if (!strcmp(session->account.username, username)) {
			session->account.username = new char[BUFF_SIZE];
			session->account.password = new char[BUFF_SIZE];
			session->account.numberOfError = 0;
			session->account.type = 1;
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
			lockSession = 0;
			break;
		}
	}
	return 0;
}

unsigned _stdcall AsynchronousLogin(void* param) {
	while (true) {
		if (lockSession == 0) {
			lockSession = 1;
			PARAM* temp = (PARAM*)param;
			temp->result = CHECK_CURRENT_SESSION(temp->session, temp->username, temp->password);
			if (!strcmp(temp->result, "110")) {
				temp->result = CHECK_OTHER_SESSION(temp->session, temp->username);
				if (!strcmp(temp->result, "110"))
					temp->result = LOGIN(temp->session, temp->username, temp->password);
			}
			lockSession = 0;
			break;
		}
	}
	return 0;
}

unsigned _stdcall AsynchronousLogout(void* param) {
	while (true)
	{
		if (lockSession == 0) {
			lockSession = 1;
			PARAM* temp = (PARAM*)param;
			temp->result = LOGOUT(temp->session, temp->username);
			lockSession = 0;
			break;
		}
	}
	return 0;
}

unsigned _stdcall Handler(void* param) {
	SOCKET connSock = (SOCKET)param;
	char buff[BUFF_SIZE], buffSend[BUFF_SIZE];
	char username[2048], password[2048];
	char* result = new char[10];
	int ret;

	SESSION session;
	session.account.username = new char[BUFF_SIZE];
	session.account.password = new char[BUFF_SIZE];
	session.account.numberOfError = 0;
	session.account.type = 1;
	session.account.location = -1;
	PARAM temp;
	temp.username = new char[BUFF_SIZE];
	temp.password = new char[BUFF_SIZE];
	temp.session = &session;
	temp.result = result;
	HANDLE hCrSession = (HANDLE)_beginthreadex(0, 0, CreateSession, (void*)&session, 0, 0);
	WaitForSingleObject(hCrSession, INFINITE);
	while (true) {
		ret = RECEIVE_TCP(connSock, buff, 0);
		if (ret <= 0) {
			if(ret == 0) printf("client close connection\n");
			else if (ret == SOCKET_ERROR) printf("connection shutdown\n");
			if (session.account.type == 0) {
				printf("The username: %s has not logged out, will perform automatic logout\n", session.account.username);
			}
			HANDLE hRelease = (HANDLE)_beginthreadex(0, 0, ReleaseSession, (void*)&session, 0, 0);
			WaitForSingleObject(hRelease, INFINITE);
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
				temp.session = &session;
				if (buff[0] == '1') {
					Slip(&buff[1], username, password);
					printf("Request: Login[username: %s, password: %s]\n", username, password);
					strcpy_s(temp.username, strlen(username) + 1, username);
					strcpy_s(temp.password, strlen(password) + 1, password);
					HANDLE hLogin = (HANDLE)_beginthreadex(0, 0, AsynchronousLogin, (void*)&temp, 0, 0);
					WaitForSingleObject(hLogin, INFINITE);

				}
				else if (buff[0] == '2') {
					printf("Request: Logout[username: %s]\n", &buff[1]);
					strcpy_s(temp.username, strlen(&buff[1]) + 1, &buff[1]);
					HANDLE hLogout = (HANDLE)_beginthreadex(0, 0, AsynchronousLogout, (void*)&temp, 0, 0);
					WaitForSingleObject(hLogout, INFINITE);
				}
				int ret = SEND_TCP(connSock, AddHeader(buffSend, temp.result), 0);
				if (ret == SOCKET_ERROR) printf("can not send message\n");
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
	sockaddr_in clientAddr;
	int clientAddrLen = sizeof(clientAddr);
	lockSession = 0;
	while (true) {
		SOCKET connSock = accept(listenSocket, (sockaddr*)&clientAddr, &clientAddrLen);
		_beginthreadex(0, 0, Handler, (void*)connSock, 0, 0);
	}
}
