// TEST.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <conio.h>
#include <stdio.h>
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

byte* ConvertStringToBytes(string value, int* lenByte) {
	int length = value.length();
	*lenByte = length;
	byte* result = new byte[length];
	for (int i = 0; i < length; i++) {
		result[i] = value[i];
	}
	return result;
}

byte* ConvertIntToBytes(int value) {
	byte* result = new byte[4];
	for (int i = 0; i < 4; i++)
		result[i] = (value >> (i * 8)) & 0xff;
	return result;
}

string ConvertBytesToString(byte* value, int length) {
	string result;
	for (int i = 0; i < length; i++) {
		result += value[i];
	}
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

int main()
{
	int abc = 0;
	string s = "chien tranh giua cac vi\n sao";
	byte* b = ConvertStringToBytes(s , &abc);
	byte* b1 = Encryption(b, abc, 10);
	string s1 = ConvertBytesToString(b1, abc);
	cout << s1 << endl;
	byte* b2 = Decryption(b1, abc, 10);
	string s2 = ConvertBytesToString(b2, abc);
	cout << s2 << endl;
	system("pause");
}

