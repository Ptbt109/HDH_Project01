#pragma once
#ifndef _NTFS_H_
#define _NTFS_H_

#include"Util.h"

class MFT_ENTRY {
public:
	int recordID;
	int parentID;
	string type;
	string name;
	string data;
	int size;
	void printEntry(int numTab = 0);
};

class NTFS
{
private:
	HANDLE device;
	vector<vector<string>> BootSector;
	int bytes_per_sector; // Số bytes trên mỗi sector
	int sectors_per_cluster; // Sc : Số bytes trên mỗi cluster
	int begin_MFT; // cluster bắt đầu MFT

	vector<MFT_ENTRY> MFTEntries;
public:
	NTFS(HANDLE disk, vector<vector<string>> sector);

	void readBoot_Sector();

	void read_MFT(vector<vector<string>>, NTFS);

	int get_first_sector_MFT();


	string get_type_file(string);

	void pushToMFTEntries(MFT_ENTRY entry);

	void findSubDirectory(int parentID = 5, int numTab = -1);

};

#endif