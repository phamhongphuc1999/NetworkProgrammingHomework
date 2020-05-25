#include "InteractFile.h"
#include "stdafx.h"

list<string> CreatePayload(string path) {
	ifstream file; file.open(path);
	string temp = "", line;
	list <string> result;
	while (!file.eof()) {
		getline(file, line);
		temp += line + '\n';
		while (temp.length() > BUFF_SIZE) {
			result.push_back(temp.substr(0, BUFF_SIZE));
			temp = temp.substr(BUFF_SIZE);
		}
	}
	file.close();
	return result;
}