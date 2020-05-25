#include "stdafx.h"
#include "InteractFile.h"

bool IsFileExistOrValid(string pathToFile){
	fstream file; file.open(pathToFile);
	return file.good();
}

list<string> CreatePayload(string pathToFile) {
	ifstream file; file.open(pathToFile, ios::out);
	string temp = "", line;
	list<string> result;
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