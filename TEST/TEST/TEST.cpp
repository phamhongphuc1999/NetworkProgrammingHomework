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

vector<string> ListFileInFolder(string path_folder)
{
	WIN32_FIND_DATA find_file_data;

	vector<string> list_file;
	wchar_t *path_folder_full = StringToWchar(path_folder);

	HANDLE hFind = FindFirstFile(path_folder_full, &find_file_data);
	list_file.push_back(WcharToString(find_file_data.cFileName));
	while (FindNextFile(hFind, &find_file_data))
	{
		list_file.push_back(WcharToString(find_file_data.cFileName));
	}
	return list_file;
}

int main()
{
	string s = "chien tranh giua cac vi sao";
	string a;
	for (int i = 0; i < s.length(); i++) {
		a += s[i] + 100;
	}
	string b;
	for (int i = 0; i < s.length(); i++)
		b += a[i] - 100;
	cout << a << endl;
	cout << b << endl;
	system("pause");
}

