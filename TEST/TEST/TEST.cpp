#include <stdio.h>
#include <conio.h>
#include "stdafx.h"
#define _CRT_SECURE_NO_WARNINGS


using namespace std;

int main() {
	FILE* f = fopen("file.txt", "w");
	if (f == NULL) {
		printf("aaaaaaaaaaaaaaaaaaaaaaa");
	}
	fprintf(f, "%s", "abc");
	fclose(f);
}