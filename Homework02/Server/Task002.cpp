// UDPServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define _WINSOCK_DEPRECATED_NO_WARNINGS
//#define SERVER_PORT 5500
#define SERVER_ADDR "127.0.0.1"
#define BUFF_SIZE 2048
#define SERVER_EXE "server.exe"

#include <SDKDDKVer.h>
#include <stdio.h>
#include <conio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <tchar.h>

#pragma comment(lib, "Ws2_32.lib")

struct ResolveResult {
	int status;
	int type;
	addrinfo* addr;
	char* hostName;
};

//check name is domain name or IP
//return true if name is domain name and return false if name is IP
int IsDomainName(char* name) {
	int result = 0, index = 0, sum = 0, countPoint = 0;
	char element = name[0];
	while (element != '\0') {
		if ('0' <= element && element <= '9') {
			sum = sum * 10 + (element - '0');
			element = name[++index];
		}
		else if (element == '.') {
			countPoint++;
			if (sum > 255 || countPoint > 3) { result = 1; break; }
			else {
				sum = 0; element = name[++index];
			}
		}
		else {
			result = 1; break;
		}
	}
	return result;
}

ResolveResult ResolverDomainName(char* name) {
	addrinfo* infoHostName = NULL, *temp = NULL;
	ResolveResult result;
	result.type = 1; result.hostName = NULL;

	int check = getaddrinfo(name, "http", NULL, &infoHostName);
	if (check) { //get address infomation fall
		result.status = 2;
		result.addr = NULL;
		return result;
	}
	temp = infoHostName;
	if (temp == NULL) { //not found infomation
		freeaddrinfo(infoHostName);
		result.status = 3;
		result.addr = NULL;
		return result;
	}
	else
	{
		freeaddrinfo(infoHostName);
		result.status = 1;
		result.addr = temp;
		return result;
	}
}

ResolveResult ResolverIPAddress(char* ip) {
	char servInfo[NI_MAXSERV], hostName[NI_MAXHOST];
	ResolveResult result; result.addr = NULL; result.type = 0;
	sockaddr_in addr;
	u_short port = 1;

	addr.sin_addr.s_addr = inet_addr(ip);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	int check = getnameinfo((sockaddr*)&addr, sizeof(sockaddr), hostName, NI_MAXHOST,
		servInfo, NI_MAXSERV, NI_NUMERICSERV);
	if (check) {
		result.hostName = NULL;
		result.status = 2;
	}
	if (ip == NULL || !strcmp(ip, hostName)) {
		result.hostName = NULL;
		result.status = 3;
	}
	else {
		result.hostName = hostName;
		result.status = 1;
	}
	return result;
}

int SendDomainName(SOCKET s, ResolveResult buff, int flag, const sockaddr* to, int tolen) {
	int status = buff.status;
	if (status == 1) {
		addrinfo* addr = buff.addr;
		char* c = new char[3]{ "11" }; sendto(s, c, strlen(c), flag, to, tolen);
		while (addr != NULL) {
			sockaddr_in* addrTemp = (sockaddr_in*)addr->ai_addr;
			char* cTemp = inet_ntoa(addrTemp->sin_addr);
			sendto(s, cTemp, strlen(cTemp), flag, to, tolen);
			addr = addr->ai_next;
		}
		c = new char[2]{ "0" }; sendto(s, c, strlen(c), flag, to, tolen);
	}
	else if (status == 2) {
		char* cTemp = new char[3]{ "21" };
		sendto(s, cTemp, strlen(cTemp), flag, to, tolen);
	}
	else if (status == 3) {
		char* cTemp = new char[3]{ "31" };
		sendto(s, cTemp, strlen(cTemp), flag, to, tolen);
	}
	return 0;
}

int SendIP(SOCKET s, ResolveResult buff, int flag, const sockaddr* to, int tolen) {
	int status = buff.status;
	if (status == 1) {
		char* c = new char[3]{ "10" }; sendto(s, c, strlen(c), flag, to, tolen);
		char* hostName = buff.hostName;
		sendto(s, hostName, strlen(hostName), flag, to, tolen);
	}
	else if (status == 2) {
		char* cTemp = new char[3]{ "20" };
		sendto(s, cTemp, strlen(cTemp), flag, to, tolen);
	}
	else if (status == 3) {
		char* cTemp = new char[3]{ "30" };
		sendto(s, cTemp, strlen(cTemp), flag, to, tolen);
	}
	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	WSADATA wsaData;
	WORD version = MAKEWORD(2, 2);
	int check = WSAStartup(version, &wsaData);
	if (check) {
		printf("WSAStartup fall");
		return 0;
	}
	SOCKET server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	u_short serverPort = 0;
	moc1: printf("%s ", SERVER_EXE);
	scanf_s("%d", &serverPort);
	if (serverPort < 1) {
		printf("wrong port\n"); goto moc1;
	}

	sockaddr_in serverAddr;
	serverAddr.sin_addr.s_addr = inet_addr(SERVER_ADDR);
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(serverPort);
	check = bind(server, (sockaddr*)&serverAddr, sizeof(serverAddr));
	if (check) {
		printf("can not bind this address");
		return 0;
	}
	printf("SERVER START\n");

	sockaddr_in clientAddr;
	char buff[BUFF_SIZE];
	int ret, clientAddrLen = sizeof(clientAddr);
	while (true) {
		ret = recvfrom(server, buff, BUFF_SIZE, 0, (sockaddr*)&clientAddr, &clientAddrLen);
		if (ret == SOCKET_ERROR) printf("SOCKET ERROR:%d", WSAGetLastError());
		else if (strlen(buff) > 0) {
			buff[ret] = 0;
			printf("Receive from client[%s: %d] %s\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), buff);

			if (IsDomainName(buff)) {
				ResolveResult result = ResolverDomainName(buff);
				ret = SendDomainName(server, result, 0, (sockaddr*)&clientAddr, sizeof(clientAddr));
				if (ret == SOCKET_ERROR) printf("SOCKET ERROR:%d", WSAGetLastError());
			}
			else {
				ResolveResult result = ResolverIPAddress(buff);
				ret = SendIP(server, result, 0, (sockaddr*)&clientAddr, sizeof(clientAddr));
				if (ret == SOCKET_ERROR) printf("SOCKET ERROR:%d", WSAGetLastError());
			}
		}
	}
	closesocket(server);
	WSACleanup();
}
