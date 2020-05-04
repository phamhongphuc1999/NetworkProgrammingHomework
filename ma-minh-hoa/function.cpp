#pragma region CONVERT
byte* ConvertIntToBytes(int value) {
	byte* result = new byte[4];
	for (int i = 0; i < 4; i++)
		result[i] = (value >> (i * 8)) & 0xff;
	return result;
}

int ConvertBytesToInt(byte* value, int length) {
	int result = 0;
	if (length > 4) length = 4;
	else if (length < 0) return -1;
	for (int i = length - 1; i >= 0; i--) {
		result <<= 8;
		result |= value[i];
	}
	return result;
}

byte* ConvertStringToBytes(string value, int* lenByte) {
	int length = value.length();
	*lenByte = length;
	byte* result = new byte[length];
	for (int i = 0; i < length; i++) {
		result[i] = value[i];
	}
	return result;
}

string ConvertBytesToString(byte* value, int length) {
	string result;
	for (int i = 0; i < length; i++) {
		result += value[i];
	}
	return result;
}

char* ConvertStringToChars(string input, char* output) {
	int length = input.length();
	char* result = new char[length];
	for (int i = 0; i < length; i++) {
		output[i] = input[i];
		result[i] = input[i];
	}
	output[length] = 0;
	result[length] = 0;
	return result;
}

char* ConvertIntToChars(char* dest, int value) {
	int index = 0;
	while (value > 0) {
		int temp = value % 10;
		value = value / 10;
		dest[index] = temp + '0';
		index++;
	}
	dest[index] = 0;
	return dest;
}

int ConvertCharsToInt(char* value) {
	int length = strlen(value);
	int result = 0;
	for (int i = length - 1; i >= 0; i--) {
		result = result * 10 + (value[i] - '0');
	}
	return result;
}

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
#pragma endregion

#pragma region WORK WITH FILE
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

bool DELETE_FILE(string file_path)
{
	char* file_path_char = new char[300];
	file_path_char = ConvertStringToChars(file_path, file_path_char);
	int ret = remove(file_path_char);
	bool is_ok = (ret == 0) ? true : false;
	return ret;
}

string GetFileName(string path_file) {
	WIN32_FIND_DATA fileName;
	wchar_t *path_file_full = StringToWchar(path_file);
	HANDLE hFind = FindFirstFile(path_file_full, &fileName);
	return WcharToString(fileName.cFileName);
}
#pragma endregion