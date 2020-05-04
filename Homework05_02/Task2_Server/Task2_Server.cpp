// server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <SDKDDKVer.h>
#include <iostream>
#include <fstream>
#include <string>
#include <process.h>

#define SERVER_ADDR "127.0.0.1"
#define BUFF_SIZE 2048
#define SERVER_EXE "Task2_Server.exe"
#define MAX_CLIENT 1024
using namespace std;

#pragma comment(lib, "Ws2_32.lib")

#pragma region CONVERT
byte* ConvertIntToBytes(int value) {
	byte* result = new byte[4];
	for (int i = 0; i < 4; i++)
		result[i] = (value >> (i * 8)) & 0xff;
	return result;
}

int ConvertBytesToInt(byte* value, int length) {
	int result = 0;
	if (length > 4) length = 4;
	else if (length < 0) return -1;
	for (int i = length - 1; i >= 0; i--) {
		result <<= 8;
		result |= value[i];
	}
	return result;
}

byte* ConvertStringToBytes(string value, int* lenByte) {
	int length = value.length();
	*lenByte = length;
	byte* result = new byte[length];
	for (int i = 0; i < length; i++) {
		result[i] = value[i];
	}
	return result;
}

string ConvertBytesToString(byte* value, int length) {
	string result;
	for (int i = 0; i < length; i++) {
		result += value[i];
	}
	return result;
}
#pragma endregion

#pragma region ENCAPSULATION
byte* AddHearer(byte* payload, int lenPayload, int opcode) {
	byte* result = new byte[lenPayload + 3];
	byte* opcodeByte = ConvertIntToBytes(opcode);
	byte* lenPayloadByte = ConvertIntToBytes(lenPayload);
	result[0] = opcodeByte[0];
	result[1] = lenPayloadByte[0];
	result[2] = lenPayloadByte[1];
	for (int i = 0; i < lenPayload; i++)
		result[i + 3] = payload[i];
	return result;
}
#pragma endregion

#pragma region STREAM TCP
int RECEIVE_TCP(SOCKET s, byte* buff, int* opcode, int flag) {
	int index = 0, ret, result = 0;
	byte* temp = new byte[3];
	ret = recv(s, (char*)temp, 3, flag);
	if (ret == 0) return 0;
	else if (ret == SOCKET_ERROR) return SOCKET_ERROR;
	else {
		*opcode = ConvertBytesToInt(temp, 1);
		int length = ConvertBytesToInt(&temp[1], 2);
		while (length > 0) {
			ret = recv(s, (char*)(&buff[index]), length, 0);
			if (ret == SOCKET_ERROR) return SOCKET_ERROR;
			else result += ret;
			index += ret;
			length -= ret;
		}
		return result;
	}
}

int SEND_TCP(SOCKET s, byte* buff, int length, int flag) {
	int nLeft = length, index = 0, ret, result = 0;
	while (nLeft > 0) {
		ret = send(s, (char*)(&buff[index]), nLeft, flag);
		if (ret == SOCKET_ERROR) return SOCKET_ERROR;
		else result += ret;
		nLeft -= ret; index += ret;
	}
	return result;
}
#pragma endregion

#pragma region EN-DECRYPTION
byte* Encryption(byte* payload, int length, int opcode) {
	byte* result = new byte[length];
	for (int i = 0; i < length; i++) {
		result[i] = payload[i] + opcode;
	}
	return result;
}

byte* Decryption(byte* payload, int length, int opcode) {
	byte* result = new byte[length];
	for (int i = 0; i < length; i++) {
		result[i] = payload[i] - opcode;
	}
	return result;
}
#pragma endregion

#pragma region WORK WITH FILE
string CreateRamdomFileName() {
	int length = rand() % 6 + 5;
	string result = "";
	srand((int)time(0));
	for (int i = 0; i < length; i++) {
		int element = rand() % 26 + 97;
		result += (char)element;
	}
	return result;
}
#pragma endregion

#pragma region HANDLER MULTIPLE CLIENT
unsigned _stdcall Handler(void* param) {
	SOCKET connSock = (SOCKET)param;
	byte buff[BUFF_SIZE], buffSend[BUFF_SIZE];
	char username[2048], password[2048];
	char* result = new char[10];
	int ret, opcode = 0;

	while (true) {
		//ret = recv(connSock, buff, BUFF_SIZE, 0);
		ret = RECEIVE_TCP(connSock, buff, &opcode, 0);
		if (ret == SOCKET_ERROR) {
			printf("Connection shutdown\n");
			break;
		}
		else if (ret > 0) {
			/*buff[ret] = 0;
			printf("%s\n", buff);*/
			string bu = ConvertBytesToString(buff, ret);
			cout << bu << endl;
			/*ret = send(connSock, buff, strlen(buff), 0);
			if (ret == SOCKET_ERROR) printf("can not send message\n");*/
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
	while (true) {
		SOCKET connSock = accept(listenSocket, (sockaddr*)&clientAddr, &clientAddrLen);
		_beginthreadex(0, 0, Handler, (void*)connSock, 0, 0);
	}
}