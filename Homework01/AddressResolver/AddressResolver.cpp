// AddressResolver.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define ADDRESS_RESOLVER "AddressReSolver.exe"
#undef UNICODE

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <conio.h>

#pragma comment(lib, "Ws2_32.lib")

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

int main()
{
moc1: printf("press exit() to close program");
	// Initialize Winsock
	WSADATA wsaData;
	WORD version = MAKEWORD(2, 2);
	int check = WSAStartup(version, &wsaData);
	if (check) {
		printf("WSAStartup fall: %d", check);
		goto moc1;
	}
	char hostName[1000], servInfo[NI_MAXSERV], hostNameIP[NI_MAXHOST];
	addrinfo* infoHostName = NULL, *temp = NULL;
	sockaddr_in* addr_ipv4;
	sockaddr_in addr_ipv4_ip;
	u_short port = 1;
	bool isFree = true;
	while (true) {
	moc2: printf("\n%s ", ADDRESS_RESOLVER);
		gets_s(hostName);
		if (!strcmp(hostName, "exit()")) break;
		check = IsDomainName(hostName);
		if (check) { //hostName is domain name
		    /*
		    Call getaddrinfo(). If the call succeeds, the result variable will hold a linked list
		    of addrinfo structures containing response infomation
		    */
			check = getaddrinfo(hostName, "http", NULL, &infoHostName);
			if (check) {
				printf("\nGet address infomation fall");
				goto moc2;
			}
			temp = infoHostName;
			isFree = false;
			if (temp == NULL) printf("\nNot found information");
			else {
				printf("\nList of IPs:");
				while (temp != NULL)
				{
					addr_ipv4 = (sockaddr_in*)temp->ai_addr;
					printf("\n    %s", inet_ntoa(addr_ipv4->sin_addr));
					temp = temp->ai_next;
				}
			}
			freeaddrinfo(infoHostName);
			isFree = true;
		}
		else { //hostName is IP
			//Setup IP address which is passed to the getnameinfo() function
			addr_ipv4_ip.sin_addr.s_addr = inet_addr(hostName);
			addr_ipv4_ip.sin_family = AF_INET;
			addr_ipv4_ip.sin_port = htons(port);
			check = getnameinfo((sockaddr*)&addr_ipv4_ip, sizeof(sockaddr), hostNameIP,
				NI_MAXHOST, servInfo, NI_MAXSERV, NI_NUMERICSERV);
			if (check) {
				printf("\nGet IP infomation fall");
				goto moc2;
			}
			if (hostName == NULL || !strcmp(hostName, hostNameIP)) printf("\nNot found information");
			else {
				printf("\nList of name:");
				printf("\n    %s", hostNameIP);
			}
		}
	}
	if (!isFree) freeaddrinfo(infoHostName);
	WSACleanup();
	printf("Press any key to close window");
	_getch();
	return 1;
}
