// NameResolver.cpp : Defines the entry point for the console application.
//

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define HOST_NAME "soict.hust.edu.vn"
#undef UNICODE

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <conio.h>
#include <SDKDDKVer.h>
#include <stdio.h>
#include <tchar.h>

// link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")


int main()
{

	//-----------------------------------------
	// Declare and initialize variables
	WSADATA wsaData;
	int iResult;
	INT iRetval;

	DWORD dwRetval;

	struct addrinfo *result = NULL;
	struct addrinfo *ptr = NULL;
	struct addrinfo hints;

	struct sockaddr_in  *sockaddr_ipv4;
	//struct sockaddr_in6 *sockaddr_ipv6;
	LPSOCKADDR sockaddr_ip;

	char ipstringbuffer[46];
	DWORD ipbufferlength = 46;

	// Validate the parameters

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		printf("Error code:", WSAGetLastErrorr());
		return 1;
	}

	//--------------------------------
	// Setup the hints address info structure
	// which is passed to the getaddrinfo() function
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	printf("Calling getaddrinfo with following parameters:\n");
	printf("\tnodename = %s\n", HOST_NAME);
	printf("\tservname (or port) = %s\n\n", "http");

	//--------------------------------
	// Call getaddrinfo(). If the call succeeds,
	// the result variable will hold a linked list
	// of addrinfo structures containing response
	// information
	dwRetval = getaddrinfo(HOST_NAME, "http", &hints, &result);
	if (dwRetval != 0) {
		printf("getaddrinfo failed with error: %d\n", dwRetval);
		WSACleanup();
		return 1;
	}

	printf("getaddrinfo returned success\n");

	// Retrieve first address and print out
	
	sockaddr_ipv4 = (struct sockaddr_in *) result->ai_addr;
	printf("\tIPv4 address %s\n",
	inet_ntoa(sockaddr_ipv4->sin_addr));
	printf("\tLength of this sockaddr: %d\n", result->ai_addrlen);

	freeaddrinfo(result);
	
	WSACleanup();
	_getch();
	return 0;
}
