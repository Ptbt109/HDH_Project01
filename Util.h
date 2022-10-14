#pragma once
#ifndef _UTIL_H_
#define _UTIL_H_

#include<iostream>
#include<cstdlib>
#include<stdio.h>
#include<vector>
#include<windows.h>
#include<iomanip>
#include<sstream>
using namespace std;

void menu();
int hexToDecimal(BYTE* byte, int offset, int count);
string hexToString(string str);
int hexCharToInt(char a);
uint64_t ReadIntReverse(uint8_t* byte, string offsetHex, unsigned int count);
string ReadtoString(BYTE* data, string offsetHex, unsigned int bytes);

//chuyen bang sector thanh vector 2d string
vector<vector<string>> to_vector(BYTE sector[512]);

//doc string hex tu bang sector
string to_hexstr(vector<vector<string>> sector, int col, int row, int num_byte, bool flag);

//chuyen tu chuyen hexstr sang chuoi string
string hexstr_tostr(string hex);

//chuyen tu hexstr sang int
int hexstr_to_int(string hexstr);

//chuyen tu chuoi hex sang chuoi bin
string to_binstr(int number);

//ket hop 2 vector
vector<vector<string>> combine_table(vector<vector<string>> v1, vector<vector<string>> v2);

//kiem tra vector nay co chua gia tri khong
bool is_end(vector<vector<string>> vec);

// tìm sector của ổ đĩa theo vị trí sector đầu vào
vector<vector<string>> find_table(HANDLE device, int readPont);

//in bang
void print_table(vector<vector<string>> vec);

//in tab ra man hinh
void print_Tab(int k);

string getOpener(string ext);

vector<string> split_File_Name(string file_name, char ch);

string get_ext_file(string file_name);

int ReadSector(HANDLE& device, vector<vector<string>>& sector);

#endif // !_UTIL_H_
