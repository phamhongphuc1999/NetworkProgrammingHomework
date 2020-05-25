#pragma once
#include "CONST.h"
#include <fstream>
#include <list>
#include <string>
using namespace std;

bool IsFileExistOrValid(string pathToFile);
list<string> CreatePayload(string pathToFile);
