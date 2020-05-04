// client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <conio.h>
#include <stdio.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <SDKDDKVer.h>
#include <process.h>
#include <fstream>
#include <string>
#include <list>
#include <vector>
#include <iostream>

#define BUFF_SIZE 2048
#define CLIENT_EXE "Task2_Client.exe"
using namespace std;

#pragma comment(lib, "Ws2_32.lib")

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
char* AddHeader(char* dest, char* source, char* opcode) {
	bool check = !strcmp(opcode, "0") || !strcmp(opcode, "1") || !strcmp(opcode, "2") || !strcmp(opcode, "3");
	if (check) dest[0] = opcode[0];
	else {
		dest[0] = 0;
		return dest;
	}
	int a = strlen(source);
	int index = 1;
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

#pragma region WORK WITH FILE
vector<string> CreatePayload(string path) {
	ifstream file; file.open(path);
	string temp = "", line;
	vector<string> result;
	while (!file.eof()) {
		getline(file, line);
		temp += line + "\n";
		while (temp.length() > BUFF_SIZE) {
			result.push_back(temp.substr(0, BUFF_SIZE));
			temp = temp.substr(BUFF_SIZE);
		}
	}
	file.close();
	return result;
}
#pragma endregion

#pragma region STREAM TCP
int RECEIVE_TCP(SOCKET s, char* buff, int* opcode, int flag) {
	int index = 0, ret, result = 0;
	char* temp = new char[10];
	ret = recv(s, temp, 10, flag);
	if (ret == 0) return 0;
	else if (ret == SOCKET_ERROR) return SOCKET_ERROR;
	else {
		*opcode = temp[0] - '0';
		int length = LENGTH(&temp[1]);
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

#pragma region CONVERT
char* ConvertStringToChars(string input, char* output) {
	int length = input.length();
	char* result = new char[length];
	for (int i = 0; i < length; i++) {
		output[i] = input[i];
		result[i] = input[i];
	}
	output[length] = 0;
	result[length] = 0;
	return result;
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
	printf("======================== press 0 to encryption =============================\n");
	printf("======================== press 1 to decryption =============================\n");
	char buff[BUFF_SIZE], dest[BUFF_SIZE], opcode[10];
	int ret, key; string path;
	while (true) {
		fflush(stdin);
	node1:
		printf("choice function: ");
		gets_s(opcode, 10);
		if (!strcmp(opcode, "0")) {
		node2:
			printf("enter key to encryption: ");
			cin >> key;
			if (key <= 0) {
				printf("requere key is unsigned integer\n");
				goto node2;
			}
			printf("enter path of file: ");
			cin >> path;
		}
		else if (!strcmp(opcode, "1")) {
		node3:
			printf("enter key to decryption: ");
			cin >> key;
			if (key <= 0) {
				printf("require key is unsigned integer\n");
				goto node3;
			}
			printf("enter path of file: ");
			cin >> path;
		}
		else {
			printf("wrong function\n");
			goto node1;
		}
		vector<string> payload = CreatePayload(path);
		int length = payload.size();
		ret = SEND_TCP(client, AddHeader(dest, key, opcode), 0);
		if (ret == SOCKET_ERROR) printf("can not send key\n");
		for (int i = 0; i < length; i++) {
			ConvertStringToChars(payload[i], buff);
			ret = SEND_TCP(client, AddHeader(dest, buff, new char[2]{ "2" }), 0);
			if (ret == SOCKET_ERROR) printf("can not send file\n");
		}
		ret = SEND_TCP(client, AddHeader(dest, new char[1]{}, new char[2]{ "2" }), 0);
		if (ret == SOCKET_ERROR) printf("can not send file\n");
		/*int ret = SEND_TCP(client, AddHeader(dest, buff), 0);
		if (ret == SOCKET_ERROR) printf("can not send message\n");*/

		/*ret = RECEIVE_TCP(client, buff, 0);
		if (ret == SOCKET_ERROR) {
			printf("ERROR: %d", WSAGetLastError());
			break;
		}
		buff[ret] = 0;
		printf("%s\n", buff);*/
	}
	closesocket(client);
	WSACleanup();
}
