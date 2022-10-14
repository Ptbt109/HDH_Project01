#pragma once
#include"Util.h"

void menu()
{
	cout << "************************" << endl;
	cout << "* 1. FAT 32            *" << endl;
	cout << "* 2. NTFS              *" << endl;
	cout << "* 3. EXIT              *" << endl;
	cout << "************************" << endl;
}

int hexToDecimal(BYTE* sector, int offset, int count)
{
	int power, sum;
	power = sum = 0;

	BYTE X;
	for (int i = 0; i < count; i++)
	{
		X = sector[i + offset];
		for (int j = 0; j < 8; j++)
		{
			if (X & 1)
				sum = sum + (int)pow(2, power);

			X = X >> 1;
			power++;
		}
	}

	return sum;
}

int hexCharToInt(char a) {
	if (a >= '0' && a <= '9')
		return(a - 48);
	if (a >= 'A' && a <= 'Z')
		return(a - 55);
	return(a - 87);
}

string hexToString(string str) {
	stringstream HexString;
	for (int i = 0; i < str.length(); i++) {
		char a = str.at(i++);
		char b = str.at(i);
		int x = hexCharToInt(a);
		int y = hexCharToInt(b);
		HexString << (char)((16 * x) + y);
	}
	return HexString.str();
}

uint64_t ReadIntReverse(uint8_t* byte, string offsetHex, unsigned int count)
{
	/*truyền vào toàn bộ mảng, vị trí bắt đầu (hexString), số lượng byte đọc
	   trả về giá trị cần đọc(decimal)*/
	const int offset = stoi(offsetHex, nullptr, 16);

	byte += offset;
	uint64_t result = 0;
	int i;
	for (i = count - 1; i >= 0; i--)
		result = (result << 8) | byte[i];

	return result;
}

string ReadtoString(BYTE* data, string offsetHex, unsigned int bytes)//Tùng sửa tham số offset hàm này
{
	const int offset = stoi(offsetHex, nullptr, 16);

	int len = offset + bytes;
	stringstream ss;
	ss << hex;
	for (int i = offset; i < len; i++)
	{
		ss << setw(2) << setfill('0') << (int)data[i];
		ss << " ";
	}
	string result;
	while (!ss.eof())
	{
		string temp;
		ss >> temp;
		result += hexToString(temp);
	}
	return result;
}

//chuyen bang sector thanh vector 2d string
vector<vector<string>> to_vector(BYTE sector[512]) {
	vector<vector<string>> result;
	int i = 0;

	for (int i = 0; i < 32; i++) {
		vector<string> temp;
		for (int j = 0; j < 16; j++) {
			stringstream ss;
			ss << setfill('0') << setw(2) << uppercase << hex << static_cast<unsigned>(sector[i * 16 + j]);
			temp.push_back(ss.str());
		}
		result.push_back(temp);
	}
	return result;
}

//doc string hex tu bang sector
string to_hexstr(vector<vector<string>> sector, int col, int row, int num_byte, bool flag) {
	string result;
	if (flag)
		for (int i = num_byte - 1; i >= 0; i--)
			result += sector[row][col + i];
	else
		for (int i = 0; i < num_byte; i++)
			result += sector[row][col + i];

	return result;
}

//chuyen tu chuyen hexstr sang chuoi string
string hexstr_tostr(string hex) {
	string ascii = "";
	for (size_t i = 0; i < hex.length(); i += 2)
	{
		string part = hex.substr(i, 2);
		char ch = stoul(part, nullptr, 16);
		ascii += ch;
	}
	return ascii;
}

//chuyen tu hexstr sang int
int hexstr_to_int(string hexstr)
{
	int result;
	char* offset;
	string real_hex = "0x" + hexstr;
	result = strtol(real_hex.c_str(), &offset, 0);
	return result;
}

string to_binstr(int number)
{
	string res;
	while (number != 0)
	{
		res = (number % 2 == 0 ? "0" : "1") + res;
		number /= 2;
	}
	if (res.length() < 8)
	{
		string sub;
		for (int i = 7 - res.length(); i >= 0; i--)
		{
			sub += '0';
		}
		res = sub + res;
	}
	return res;
}

vector<vector<string>> combine_table(vector<vector<string>> v1, vector<vector<string>> v2) {
	/*int begin = v1.size();*/
	for (int i = 0; i < v2.size(); i++) {
		v1.push_back(v2[i]);
	}
	v2.clear();
	return v1;
}

bool is_end(vector<vector<string>> vec) {
	for (int i = 0; i < vec[0].size(); i++) {
		if (vec[vec.size() - 1][i] != "00") return false;
	}
	return true;
}

vector<vector<string>> find_table(HANDLE device, int readPont)
{
	DWORD bytesRead;
	BYTE sector[512];

	long a = 0x0;

	vector<vector<string>> result;
	SetFilePointer(device, readPont, &a, FILE_BEGIN);//Set a Point to Read

	if (!ReadFile(device, sector, 512, &bytesRead, NULL))
	{
		printf("Find sector false at: %u\n", GetLastError());
		return result;
	}

	result = to_vector(sector);

	if (is_end(result)) {
		return result;
	}

	vector<vector<string>> next = find_table(device, readPont + 512);
	return combine_table(result, next);
};

void print_table(vector<vector<string>> vec) {
	for (int i = 0; i < vec.size(); i++) {
		for (int j = 0; j < vec[i].size(); j++) {
			cout << vec[i][j] << " ";
		}
		cout << endl;
	}
	return;
}

void print_Tab(int k)
{
	if (k == 0)
		return;
	if (k >= 1)
	{
		for (int i = 0; i < k; i++)
		{
			cout << "\t";
		}
	}
}

string getOpener(string ext)
{
	string opener = "";
	if (ext == "txt" || ext == "TXT")
		return "";
	if (ext == "doc" || ext == "DOC" || ext == "docx" || ext == "DOCX")
		opener = "Microsoft Word";
	if (ext == "xls" || ext == "XLS" || ext == "xlsx" || ext == "XLSX")
		opener = "Microsoft Excel";
	if (ext == "pptx" || ext == "PPTX")
		opener = "Microsoft Powerpoint";
	if (ext == "pdf" || ext == "PDF")
		opener = "Adobe Reader XI";
	if (ext == "mp3" || ext == "MP3" || ext == "mp4" || ext == "MP4" || ext == "wmv" || ext == "WMV" || ext == "wma" || ext == "WMA")
		opener = "Windows Media Player";
	if (ext == "jpg" || ext == "JPG" || ext == "jpeg" || ext == "JPEG" || ext == "png" || ext == "PNG")
		opener = "Photos";
	if (ext == "accdb" || ext == "ACCDB")
		opener = "Microsoft Access";
	if (ext == "sql" || ext == "SQL")
		opener = "SQL Server or MySQL";
	if (ext == "pub" || ext == "PUB")
		opener = "Microsoft Publisher";
	if (ext == "rar" || ext == "RAR" || ext == "rar4" || ext == "RAR4" || ext == "zip" || ext == "ZIP")
		opener = "WinRAR";
	if (ext == "rtf" || ext == "RTF")
		opener = "WordPad";
	if (opener.length() == 0)
	{
		return "Unknown extension";
	}
	return "Open " + ext + " file by: " + opener;
}

vector<string> split_File_Name(string file_name, char ch)
{
	vector<string> res;
	string fea;
	stringstream line;
	if (file_name != "")
	{
		line << file_name;
		while (getline(line, fea, ch))
			res.push_back(fea);
	}
	return res;
}

string get_ext_file(string file_name)
{
	vector<string> name_fea = split_File_Name(file_name, '.');
	string ext = name_fea[1];
	string ext1;
	for (int i = 0; i < ext.size(); i++)
	{
		if (ext[i] == 'a' or ext[i] == 'b' or ext[i] == 'c' or ext[i] == 'd' or ext[i] == 'e' or ext[i] == 'f' or ext[i] == 'g'
			or ext[i] == 'h' or ext[i] == 'i' or ext[i] == 'j' or ext[i] == 'k' or ext[i] == 'l' or ext[i] == 'm' or ext[i] == 'n'
			or ext[i] == 'o' or ext[i] == 'p' or ext[i] == 'q' or ext[i] == 'r' or ext[i] == 's' or ext[i] == 't' or ext[i] == 'u'
			or ext[i] == 'v' or ext[i] == 'w' or ext[i] == 'x' or ext[i] == 'y' or ext[i] == 'z'
			or ext[i] == 'A' or ext[i] == 'B' or ext[i] == 'C' or ext[i] == 'D' or ext[i] == 'E' or ext[i] == 'F' or ext[i] == 'G'
			or ext[i] == 'H' or ext[i] == 'I' or ext[i] == 'J' or ext[i] == 'K' or ext[i] == 'L' or ext[i] == 'M' or ext[i] == 'N'
			or ext[i] == 'O' or ext[i] == 'P' or ext[i] == 'Q' or ext[i] == 'R' or ext[i] == 'S' or ext[i] == 'T' or ext[i] == 'U'
			or ext[i] == 'V' or ext[i] == 'W' or ext[i] == 'X' or ext[i] == 'Y' or ext[i] == 'Z')
		{
			ext1 += ext[i];
		}
	}
	return ext1;
}

int ReadSector(HANDLE& device, vector<vector<string>>& sector)//(LPCWSTR  drive, int readPoint, BYTE sector[512])
{
	DWORD bytesRead;
	BYTE boot[512];

	wstring disk_name;
	cout << "Nhap ten o dia: ";
	wcin >> disk_name;
	disk_name = L"\\\\.\\" + disk_name + L":";

	LPCWSTR drive = disk_name.c_str();

	device = CreateFile(drive,    // Drive to open
		GENERIC_READ,           // Access mode
		FILE_SHARE_READ | FILE_SHARE_WRITE,        // Share Mode
		NULL,                   // Security Descriptor
		OPEN_EXISTING,          // How to create
		0,                      // File attributes
		NULL);                  // Handle to template

	if (device == INVALID_HANDLE_VALUE) // Open Error
	{
		printf("Open drive false at: %u\n", GetLastError());
		return 0;
	}

	SetFilePointer(device, 0, NULL, FILE_BEGIN);//Set a Point to Read

	if (!ReadFile(device, boot, 512, &bytesRead, NULL))
	{
		printf("Read bootsector false at: %u\n", GetLastError());
		return 0;
	}
	else
	{
		//Dung de luu bang sector duoi dang vector 2 chieu voi gia tri moi element la string
		sector = to_vector(boot);
		printf("Read bootsector success!\n\n");
		return 1;
	}
}