// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define _WINSOCK_DEPRECATED_NO_WARNINGS
//#define SERVER_PORT 5500
#define SERVER_ADDR "127.0.0.1"
#define BUFF_SIZE 2048
#define SERVER_EXE "server.exe"

#include <stdio.h>
#include <conio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <tchar.h>

#pragma comment(lib, "Ws2_32.lib")

//type: 1-client send IP to server, 0-client send domain to server
//status:1-correct, 2-get information fall, 3-not found information
