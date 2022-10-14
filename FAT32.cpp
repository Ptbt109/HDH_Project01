#include"FAT32.h"
#include"Util.h"

int readSector(LPCWSTR  drive, int readPoint, BYTE sector[512])
{
	int retCode = 0;
	DWORD bytesRead;
	HANDLE device = NULL;

	device = CreateFile(drive,  // Drive to open
		GENERIC_READ,           // Access mode
		FILE_SHARE_READ | FILE_SHARE_WRITE,        // Share Mode
		NULL,                   // Security Descriptor
		OPEN_EXISTING,          // How to create
		0,                      // File attributes
		NULL);                  // Handle to template

	if (device == INVALID_HANDLE_VALUE) // Open Error
	{
		printf("CreateFile: %u\n", GetLastError());
		return 1;
	}

	SetFilePointer(device, readPoint, NULL, FILE_BEGIN);//Set a Point to Read

	if (!ReadFile(device, sector, 512, &bytesRead, NULL))
	{
		printf("ReadFile: %u\n", GetLastError());
	}
	else
	{
		printf("Success!\n");
	}
}

void initBootsector(BYTE* sector, FAT32& fat32) {
	fat32.fatCategory = ReadtoString(sector, "52", 8);// //offset 52 - 8 bytes
	std::stringstream ss;
	ss << std::hex;
	ss << setw(2) << setfill('0') << (int)sector[21];
	ss >> fat32.volumeType;
	fat32.bytePerSector = hexToDecimal(sector, 11, 2);
	fat32.sectorPerCluster = hexToDecimal(sector, 13, 1);
	fat32.reservedSector = hexToDecimal(sector, 14, 2);
	fat32.numFat = hexToDecimal(sector, 16, 1);
	fat32.rdetEntry = hexToDecimal(sector, 17, 2);
	fat32.totalSector = hexToDecimal(sector, 32, 4);
	fat32.sectorPerFAT = hexToDecimal(sector, 36, 4);
	fat32.rootCluster = hexToDecimal(sector, 44, 4);
	fat32.sectorsPerTrack = ReadIntReverse(sector, "18", 2); 
	fat32.headsCount = ReadIntReverse(sector, "1A", 2); 
	fat32.hiddenSectors = ReadIntReverse(sector, "1C", 4); 
	fat32.secondaryInfoSector = ReadIntReverse(sector, "30", 2); 
	fat32.bootCopySector = ReadIntReverse(sector, "32", 2); 

}

void readInformationBootSector(FAT32 fat32)
{
	cout << "Thong tin phan vung FAT32" << endl;
	cout << "Loai FAT: " << fat32.fatCategory << endl;
	cout << "Loai volume: " << fat32.volumeType << endl;
	cout << "So Byte cho 1 sector: " << fat32.bytePerSector <<" bytes" << endl;
	cout << "So Sector cho 1 cluster (SC): " << fat32.sectorPerCluster<<" sectors" << endl;
	cout << "So Sector cua BootSector (SB): " << fat32.reservedSector <<" sectors" << endl;
	cout << "So bang FAT (NF): " << fat32.numFat << endl;
	cout << "So sector cua track: " << fat32.sectorsPerTrack << " sectors" << endl;
	cout << "So luong dau doc: " << fat32.headsCount << endl;
	cout << "So sector an truoc volume: " << fat32.hiddenSectors << " sectors" << endl;
	cout << "Tong so sector, kich thuoc Volume (SV): " << fat32.totalSector << " sectors" << endl;
	cout << "Kich thuoc moi bang FAT: " << fat32.sectorPerFAT<<" sectors" << endl;
	cout << "So Sector cho bang RDET: " << fat32.rdetEntry << endl;
	cout << "Cluster bat dau cua RDET: " << fat32.rootCluster << endl;
	cout << "Sector chua thong tin phu (ve cluster trong): " << fat32.secondaryInfoSector << endl;
	cout << "Sector chua ban sao cua Bootsector: " << fat32.bootCopySector << endl;
	cout << "Sector dau tien FAT1: " << fat32.reservedSector << endl;

	int firstSectorDATA = fat32.reservedSector + fat32.numFat * fat32.sectorPerFAT;
	cout << "Sector dau tien cua vung RDET: " << firstSectorDATA << endl;
	cout << "Sector dau tien cua vung DATA: " << firstSectorDATA << endl;
}

void initFAT(int*& FAT, FAT32 fat32, LPCWSTR drive1)
{
	//There are 2 FAT tables
	//FAT32 -- 32 bits (4 Bytes) but only 28 bits are used and high 4 bits are reserved	
	int fatTableSize = fat32.sectorPerFAT * fat32.bytePerSector;
	BYTE* readBytes = new BYTE[fatTableSize];
	fstream diskStream(drive1, std::ios::in);
	char* a = new char[fatTableSize];
	diskStream.seekg(fat32.reservedSector * fat32.bytePerSector, 0);
	diskStream.read(a, fatTableSize);

	readBytes = (BYTE*)a;
	int numberOfFATEntries = fatTableSize / 4; //FAT_RECORD_SIZE;
	int count = 0;
	//8946
	for (int i = 0; i < numberOfFATEntries; i++)
	{
		if (hexToDecimal(readBytes, 4 * i, 4) == 0)
		{
			count += 1;
			if (count >= 200)
				break;
		}
		FAT[i] = hexToDecimal(readBytes, 4 * i, 4);
	}
}

int firstSectorIndexOfCluster(int clusIndex, FAT32 fat32)
{
	int dataFirstSector = fat32.reservedSector + fat32.numFat * fat32.sectorPerFAT;
	return ((clusIndex - 2) * fat32.sectorPerCluster) + dataFirstSector;
}


DIRECTORY* readDirectory(int firstEntryIndex, int clusIndex, int* FAT, FAT32 fat32, LPCWSTR drive1, string space)
{
	int clusSize = fat32.sectorPerCluster * fat32.bytePerSector;
	//Root directory entries có kích thước 32 Bytes 
	//và số lượng các entries bị giới hạn bởi kích thước của cluster
	//và kích thước dựa trên FAT entries

	int totalEntries = (fat32.sectorPerCluster * fat32.bytePerSector) / 32;
	char* a = new char[clusSize];
	int firstSector = firstSectorIndexOfCluster(clusIndex, fat32);
	fstream diskStream(drive1, std::ios::in);

	diskStream.seekg(firstSector * fat32.bytePerSector, SEEK_SET);
	diskStream.read(a, clusSize);
	BYTE* readBytes = new BYTE[clusSize];
	readBytes = (BYTE*)a;

	//đọc từng entry
	DIRECTORY* dirHeadNode, * dirTempNode, * dirTailNode;
	dirHeadNode = dirTempNode = dirTailNode = NULL;

	//holder to store longer named
	int byteHolder[13];
	int name[256];
	int holderIndex = -1;
	int nameIndex = -1;

	for (int i = firstEntryIndex; i < totalEntries; i++)
	{
		//cuối bảng ghi
		if (readBytes[32 * i] == 0x0) break;

		//tập tin không được sử dụng do bị xóa
		if (readBytes[32 * i] == 0xE5) continue;

		int status = hexToDecimal(32 * i + readBytes, 11, 1);

		//LONG FILE NAME ENTRY
		if (status == 0xF)
		{
			//Code để lấy được Long file name, đọc lần lượt 2 Bytes để lấy cái kí tự
			//1 -- 10
			for (int j = 1; j < 10; j += 2)
			{
				if (hexToDecimal(32 * i + readBytes, j, 2) == 0xffff) break; //kết thúc file name
				byteHolder[++holderIndex] = hexToDecimal(32 * i + readBytes, j, 2);
			}
			//14 -- 25
			for (int j = 14; j < 25; j += 2)
			{
				if (hexToDecimal(32 * i + readBytes, j, 2) == 0xffff) break; //kết thúc file name
				byteHolder[++holderIndex] = hexToDecimal(32 * i + readBytes, j, 2);
			}
			//28 -- 31
			for (int j = 28; j < 31; j += 2)
			{
				if (hexToDecimal(32 * i + readBytes, j, 2) == 0xffff) break; //kết thúc file name
				byteHolder[++holderIndex] = hexToDecimal(32 * i + readBytes, j, 2);
			}
			//POP VÀ PUSH VÀO LONG NAME STACK
			while (holderIndex != -1)
				name[++nameIndex] = byteHolder[holderIndex--];
			continue; //Bởi vì Long file name entry được xác định nên bỏ qua
		}
		//Chỉ có các tập tin và thư mục được xét
		//và các tập tin liên quan đến hệ thống được bỏ qua dựa vào Attr

		if (status == 0x10 || status == 0x20)
		{
			dirTempNode = new DIRECTORY;
			dirTempNode->next = dirTempNode->dir = NULL;

			if (dirHeadNode == NULL)
				dirHeadNode = dirTempNode;
			else
				dirTailNode->next = dirTempNode;

			dirTailNode = dirTempNode;

			int count = 0; //sử dụng để đếm các ký tự
			for (int j = 0; j < 11; j++)
				dirTempNode->Name[count++] = readBytes[32 * i + j];

			if (nameIndex != -1)
			{
				count = 0;
				for (int j = nameIndex; j > -1; j--)
					dirTempNode->Name[count++] = (char)name[j];

				holderIndex = nameIndex = -1;
			}
			dirTempNode->Name[count] = '\0';
			dirTempNode->Attr = status;
			dirTempNode->FileSize = hexToDecimal(32 * i + readBytes, 28, 4);
			dirTempNode->StartCluster = (hexToDecimal(32 * i + readBytes, 20, 2)
				* pow(2, 16)) + hexToDecimal(32 * i + readBytes, 26, 2);


			//to read directories in directories

			cout << space <<"| "<< "Ten file: " << dirTempNode->Name << space<<space << (dirTempNode->Attr == 0x10 ? "<Directory>" : "<Archive>")<<" |" << endl;
			cout << space <<"| " << "Kich thuoc: " << dirTempNode->FileSize << " B" << " |" << endl;
			cout << space << "| " << "Cluster bat dau: " << dirTempNode->StartCluster << " |" << endl;
			if (status == 0x10)
			{
				space = space + "     ";
				cout << endl;
				dirTempNode->dir = readDirectory(2, dirTempNode->StartCluster, FAT, fat32, drive1, space); //2 to skip . and .. dir entries
				space = space.substr(0, space.length() - 5);
			}
			else if (status == 0x20)
			{
				int len = strlen(dirTempNode->Name) - 3;
				char string[4];
				strcpy(string, dirTempNode->Name + len);

				if (strcmp(string, "txt") == 0 || strcmp(string, "TXT") == 0)
				{
					cout << space << "| " << "Noi dung file TXT: ";
					readContentOfFile(fat32, dirTempNode->StartCluster, drive1, space);
				}
				else
					cout << space << "--> " << "Dung phan mem tuong thich de doc noi dung!"<< endl;
				cout << "-------------------------------------------------------------------" << endl;
			}
			
			if (FAT[clusIndex] != 0x0fffffff)
				dirTailNode->next = readDirectory(firstEntryIndex, FAT[clusIndex], FAT, fat32, drive1, space);
		}
	}

	return dirHeadNode;
}

void readContentOfFile(FAT32 fat32, int clusIndex, LPCWSTR drive1, string space) {
	int content_sector = firstSectorIndexOfCluster(clusIndex, fat32);
	int clusterSizeInBytes = fat32.sectorPerCluster * fat32.bytePerSector;

	char* a = new char[clusterSizeInBytes];
	fstream G(drive1, std::ios::in);
	G.seekg(content_sector * fat32.bytePerSector, 0);
	G.read(a, clusterSizeInBytes);

	char* content = new char[clusterSizeInBytes];
	int i = 0;
	while (a[i] != '\x00')
	{
		content[i] = a[i];
		i++;
		if (i == 4095) break;
	}
	content[i] = '\0';

	cout << space << content << endl;
}

void freeDirEntries(DIRECTORY* dir)
{
	DIRECTORY* temp;
	while (dir)
	{
		if (dir->dir)
			freeDirEntries(dir->dir);

		temp = dir;
		dir = dir->next;
		delete temp;
	}
}


