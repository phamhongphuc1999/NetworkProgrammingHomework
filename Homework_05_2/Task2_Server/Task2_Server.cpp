// Task2_Server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <conio.h>
#include <stdio.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <SDKDDKVer.h>
#include <process.h>

#define BUFF_SIZE 2048
#define CLIENT_EXE "Task2_Client.exe"

#pragma comment(lib, "Ws2_32.lib")


int main()
{
	byte f[100];
	f[0] = '0';
	f[1] = 'a';
	f[2] = 0;
	printf("%s", f);
	system("pause");
}

