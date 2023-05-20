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
#include <vector>
#include <list>

#define SERVER_ADDR "127.0.0.1"
#define BUFF_SIZE 2048
#define SERVER_EXE "Task2_Server.exe"
#define MAX_CLIENT 1024
using namespace std;

#pragma comment(lib, "Ws2_32.lib")

#pragma region COMMON
struct NewFile
{
	vector<string> payload;
	string file_name;
};

struct ENCRYPTION_FILE
{
	string path;
	int key;
	int mode;
	vector<string> payload;
};

int lockData;
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

string ConvertCharsToString(char* value) {
	int length = strlen(value);
	string result = "";
	for (int i = 0; i < length; i++) {
		result += value[i];
	}
	return result;
}

char* ConvertIntToChars(char* dest, int value) {
	int index = 0;
	while (value > 0) {
		int temp = value % 10;
		value = value / 10;
		dest[index] = temp + '0';
		index++;
	}
	dest[index] = 0;
	return dest;
}

int ConvertCharsToInt(char* value) {
	int length = strlen(value);
	int result = 0;
	for (int i = length - 1; i >= 0; i--) {
		result = result * 10 + (value[i] - '0');
	}
	return result;
}

string WcharToString(wchar_t* wchar_str)
{
	string str = "";
	int index = 0;
	while (wchar_str[index] != 0)
	{
		str += (char)wchar_str[index];
		++index;
	}
	return str;
}

wchar_t* StringToWchar(string str)
{
	int index = 0;
	int count = str.size();
	wchar_t *ws_str = new wchar_t[count + 1];
	while (index < str.size())
	{
		ws_str[index] = (wchar_t)str[index];
		index++;
	}
	ws_str[index] = 0;
	return ws_str;
}
#pragma endregion

#pragma region EN_DECRYPTION
string En_Or_decryption(string value, int key, int mode) {
	string result = "";
	int length = value.length();
	for (int i = 0; i < length; i++) {
		if (mode == 0) result += value[i] + key;
		else result += value[i] - key;
	}
	return result;
}

vector<string> En_Or_decryptionFileData(string path, int key, int mode) {
	ifstream file; file.open(path);
	string temp = "", line;
	vector<string> result;
	while (!file.eof()) {
		getline(file, line);
		temp += line + "\n";
		while (temp.length() > BUFF_SIZE) {
			string encrypString = En_Or_decryption(temp.substr(0, BUFF_SIZE), key, mode);
			result.push_back(encrypString);
			temp = temp.substr(BUFF_SIZE);
		}
	}
	file.close();
	return result;
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

vector<string> ListFileInFolder(string path_folder)
{
	WIN32_FIND_DATA find_file_data;

	vector<string> list_file;
	wchar_t* path_folder_full = StringToWchar(path_folder);

	HANDLE hFind = FindFirstFile((LPCWSTR)&(*path_folder_full), &find_file_data);
	list_file.push_back(WcharToString((wchar_t*)find_file_data.cFileName));
	while (FindNextFile(hFind, &find_file_data))
	{
		list_file.push_back(WcharToString((wchar_t*)find_file_data.cFileName));
	}
	return list_file;
}

bool DELETE_FILE(string file_path)
{
	char* file_path_char = new char[300];
	file_path_char = ConvertStringToChars(file_path, file_path_char);
	int ret = remove(file_path_char);
	bool is_ok = (ret == 0) ? true : false;
	return ret;
}

bool CheckFileName(string fileName){
	vector<string> fileList = ListFileInFolder("data/*");
	int length = fileList.size();
	for (int i = 0; i < length; i++) {
		if (fileName == fileList[i]) return false;
	}
	return true;
}

bool WriteNewFile(vector<string> payloadList, string* file_name) {
	try {
	node1:
		string fileName = CreateRamdomFileName();
		if (!CheckFileName(fileName)) goto node1;
		*file_name = fileName;
		ofstream file; file.open("data/" + fileName, ios::out);
		int length = payloadList.size();
		for (int i = 0; i < length; i++) {
			file << payloadList[i];
		}
		file.close();
		return true;
	}
	catch (exception) {
		return false;
	}
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

#pragma region HANDLER MULTIPLE CLIENT
unsigned _stdcall WriteAsynchronousNewFile(void* param) {
	while (true)
	{
		if (lockData == 0) {
			lockData = 1;
			NewFile* file = (NewFile*)param;
			vector<string> payload = file->payload;
			WriteNewFile(payload, &file->file_name);
			lockData = 0;
			break;
		}
	}
	return 0;
}

unsigned _stdcall DeleteAsynchronousFile(void* param) {
	while (true) {
		if (lockData == 0) {
			lockData = 1;
			string* path = (string*)param;
			DELETE_FILE(*path);
			lockData = 0;
			break;
		}
	}
	return 0;
}

unsigned _stdcall En_Or_DecryAsynchromousFile(void* param) {
	while (true)
	{
		if (lockData == 0) {
			lockData = 1;
			ENCRYPTION_FILE* temp = (ENCRYPTION_FILE*)param;
			temp->payload = En_Or_decryptionFileData(temp->path, temp->key, temp->mode);
			lockData = 0;
			break;
		}
	}
	return 0;
}

unsigned _stdcall Handler(void* param) {
	SOCKET connSock = (SOCKET)param;
	char buff[BUFF_SIZE + 1], buffSend[BUFF_SIZE];
	char username[2048], password[2048];
	char* result = new char[10];
	int ret, opcode = 0, key;
	list<pair<string, int>> fileInfo;
	pair<string, int> item;
	while (true) {
		ret = RECEIVE_TCP(connSock, buff, &opcode, 0);
		if (ret == SOCKET_ERROR) {
			printf("Connection shutdown\n");
			closesocket(connSock);
			break;
		}
		else if (ret == 0) {
			printf("client close connection\n");
			closesocket(connSock);
			break;
		}
		else if (ret > 0) {
			buff[ret] = 0;
			if (opcode == 1 || opcode == 0) {
				buff[ret] = 0;
				key = ConvertCharsToInt(buff);
				item.second = opcode;
			}
			vector<string> payloadList;
			while (true) {
				ret = RECEIVE_TCP(connSock, buff, &opcode, 0);
				if (ret == SOCKET_ERROR) {
					printf("can not receive from client\n");
					break;
				}
				else if (opcode == 2) {
					buff[ret] = 0;
					if (!strcmp(buff, "")) {
						printf("receive fineshed\n");
						break;
					}
					payloadList.push_back(ConvertCharsToString(buff));
				}
			}
			NewFile fInfo; fInfo.payload = payloadList;
			HANDLE hWrite = (HANDLE)_beginthreadex(0, 0, WriteAsynchronousNewFile, (void*)&fInfo, 0, 0);
			WaitForSingleObject(hWrite, INFINITE);
			item.first = fInfo.file_name;
			fileInfo.push_back(item);

			list<pair<string, int>>::iterator pointer = fileInfo.begin();
			ENCRYPTION_FILE param;
			param.path = "data/" + pointer->first;
			param.key = key; param.mode = pointer->second;
			HANDLE hEn = (HANDLE)_beginthreadex(0, 0, En_Or_DecryAsynchromousFile, (void*)&param, 0, 0);
			WaitForSingleObject(hEn, INFINITE);
			int length = param.payload.size();
			for (int i = 0; i < length; i++) {
				ConvertStringToChars(param.payload[i], buff);
				ret = SEND_TCP(connSock, AddHeader(buffSend, buff, new char[2]{ "2" }), 0);
				if (ret == SOCKET_ERROR) printf("can not send file\n");
			}
			ret = SEND_TCP(connSock, AddHeader(buffSend, new char[1]{ 0 }, new char[2]{ "2" }), 0);
			if (ret == SOCKET_ERROR) printf("can not send file\n");
			string path = "data/" + pointer->first;
			HANDLE delete_file = (HANDLE)_beginthreadex(0, 0, DeleteAsynchronousFile, (void*)&path, 0, 0);
			WaitForSingleObject(delete_file, INFINITE);
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
	lockData = 0;
	while (true) {
		SOCKET connSock = accept(listenSocket, (sockaddr*)&clientAddr, &clientAddrLen);
		_beginthreadex(0, 0, Handler, (void*)connSock, 0, 0);
	}
}
