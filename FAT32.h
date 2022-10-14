#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cmath>
#include <windows.h>
#include <fstream>
#include<sstream>
#include<iomanip>
#include<string>
#include <string.h>
#include<vector>
#include<string>
#include <atlstr.h>
#include <fcntl.h>
#include"Util.h"

using namespace std;

struct FAT32 {
    string fatCategory;// //offset 52 - 8 bytes
    string volumeType;//offset 15 - 1 byte, Loại volume được nhận biết bằng chuỗi thập lục phân (f8 là đĩa cứng)
    int bytePerSector;          // 	Số byte của một Sector      (B - 2)
    int sectorPerCluster;       // 	Số Sector của một Cluster   (D - 1) SC
    int reservedSector;         // 	Số Sector của BootSector    (E - 2) SB
    int numFat;                 // 	Số bảng FAT                 (10 - 1) NF
    int rdetEntry;              // 	Số Entry của RDET           (11 - 2) SRDET
    int totalSector;            //  Kích thước Volume           (32 - 4) Sv
    int sectorPerFAT;           //  Số Sector của một bảng FAT  (24 - 4) Sf
    int rootCluster;            //  Cluster bắt đầu của RDET    (2C - 4) 

    int sectorsPerTrack; //Số sector của track (offset 18 - 2 byte)
    int headsCount; //Số lượng đầu đọc (offset 1A - 2 bytes)
    int hiddenSectors; // Số sector ẩn trước volume (offset 1C - 4 bytes)
    int secondaryInfoSector; //Sector chứa thông tin phụ (về cluster trống) (offset 30 - 2 bytes)
    int bootCopySector; //Sector chứa bản sao của Bootsector (offset 32 - 2 bytes)
};

struct DIRECTORY {
    char Name[256];             // Tên thư mục/ tập tin
    int Attr;                   // Thuộc tính (thường là thư mục/ tập tin)
    int StartCluster;           // Cluster bắt đầu
    int FileSize;               // Kích cỡ (tính theo byte)
    DIRECTORY* next;            // Trỏ đến thư mục/ tập tin tiếp theo
    DIRECTORY* dir;             // Trỏ đến thư mục con
};


int readSector(LPCWSTR  drive, int readPoint, BYTE sector[512]);
void initBootsector(BYTE* sector, FAT32& fat32);
void readInformationBootSector(FAT32 fat32);

void initFAT(int*& FAT, FAT32 fat32, LPCWSTR drive1);
int firstSectorIndexOfCluster(int N, FAT32 fat32);

DIRECTORY* readDirectory(int firstRecordIndex, int clusIndex, int* entryList, FAT32 fat32, LPCWSTR drive1, string space);
void readContentOfFile(FAT32 fat32, int clusIndex, LPCWSTR drive1, string space);
void freeDirEntries(DIRECTORY* dir);