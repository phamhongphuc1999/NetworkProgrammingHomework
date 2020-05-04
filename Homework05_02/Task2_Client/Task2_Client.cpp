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

#pragma region INPUT AND DATA
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
#pragma endregion

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

#pragma region FILE
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

int main()
{
	WSADATA wsaData;
	WORD version = MAKEWORD(2, 2);
	if (WSAStartup(version, &wsaData)) {
		printf("version is not supported\n");
		return 0;
	}
	//SOCKET client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	SOCKET client = socket(AF_INET, SOCK_DGRAM, AF_NETBIOS);
	
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
	printf("============================== press 0 to encryption =========================\n");
	printf("============================== press 1 to decryption =========================\n");
	while (true) {
		int opcode; int key; string path;
		fflush(stdin);
	node1:
		printf("choice function: ");
		scanf_s("%d", &opcode);
		if (opcode == 0) {
		node0_1:
			printf("enter key to encryption: ");
			cin >> key;
			if (key <= 0) {
				printf("require key is unsigned integer\n");
				goto node0_1;
			}
			printf("enter path of file: ");
			cin >> path;
			if (path == "") break;
		}
		else if (opcode == 1) {
		node1_1:
			printf("enter key to decryption: ");
			cin >> key;
			if (key <= 0) {
				printf("require key is unsigned integer\n");
				goto node1_1;
			}
			printf("enter path of file: ");
			cin >> path;
			if (path == "") break;
		}
		else {
			printf("Wrong function\n");
			goto node1;
		}
		//vector<string> payloadList = CreatePayload(path);
		byte* keyChar = ConvertIntToBytes(key);
		SEND_TCP(client, AddHearer(keyChar, 4, opcode), 4, 0);

		////vector<string> payloadList = CreatePayload(path);
		//byte* keyChar = ConvertIntToBytes(key);
		//printf("%d\n\n\n\n", strlen(AddHearer(keyChar, 4, opcode)));
		//SEND_TCP(client, AddHearer(keyChar, 4, opcode), 0);

		////*int ret = send(client, buff, strlen(buff), 0);
		//if (ret == SOCKET_ERROR) printf("can not send message\n");
		//ret = recv(client, buff, BUFF_SIZE, 0);
		//if (ret == SOCKET_ERROR) {
		//printf("ERROR: %d", WSAGetLastError());
		//break;
		//}
		//buff[ret] = 0;
		//printf("%s\n", buff);*/
	}
	closesocket(client);
	WSACleanup();
}