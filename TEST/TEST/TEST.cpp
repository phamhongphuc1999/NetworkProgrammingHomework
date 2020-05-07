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
#include <stdlib.h>
#include <time.h>
#include <list>
#include <vector>

#define SERVER_ADDR "127.0.0.1"
#define BUFF_SIZE 5
#define SERVER_EXE "Task2_Server.exe"
#define MAX_CLIENT 1024
using namespace std;

#pragma comment(lib, "Ws2_32.lib")

int main()
{
	ofstream i; i.open("D:/Documents/ac.txt", ios::in);
	string line = "chien tranh giua cac vi sao";
	i.seekp(10);
	int ip = i.tellp();
	i << line;
	cout << line;
	system("pause");
}

