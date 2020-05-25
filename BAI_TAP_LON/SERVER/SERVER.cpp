// SERVER.cpp : Defines the entry point for the console application.
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
#include <iostream>

#define SERVER_ADDR "127.0.0.1"
#define BUFF_SIZE 2048
#define SERVER_EXE "Server.exe"
#define MAX_CLIENT 64
using namespace std;

#pragma comment(lib, "Ws2_32.lib")

int main()
{
	
}

