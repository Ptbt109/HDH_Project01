#include"FAT32.h"
#include"NTFS.h"
#include"Util.h"

int main()
{
	int choice = 0;
	string nameDisk;
	FAT32 fat32;
	
	do {
		menu();
		cout << "Nhap lua chon: ";
		cin >> choice;
		switch (choice)
		{
		case 1://Thong tin FAT32
		{
			cout << "Nhap ten o dia: ";
			cin >> nameDisk;
			BYTE sector[512];
			string a = "\\\\.\\" + nameDisk + ":";
			CString str(a.c_str());
			CStringW strw(str);
			LPCWSTR drive1 = strw;


			readSector(drive1, 0, sector);
			initBootsector(sector, fat32);

			//Đọc các thông tin được mô tả trong Boot Sector
			cout << "++++++++++++++++++    THONG TIN BOOT SECTOR     ++++++++++++++++++\n" << endl;
			readInformationBootSector(fat32);

			cout << "\n=====================  CAY THU MUC  =======================\n" << endl;
			// Đọc cây thư mục				
			struct DIRECTORY* DIR = NULL;
			int numberOfFATEntries = (fat32.sectorPerFAT * fat32.bytePerSector) / 4;
			int* FAT = new int[numberOfFATEntries * sizeof(int)];
			initFAT(FAT, fat32, drive1);
			// hiển thị cây thư mục 
			DIR = readDirectory(0, fat32.rootCluster, FAT, fat32, drive1, " ");

			freeDirEntries(DIR);
			DIR = NULL;
			system("pause");
			system("cls");
			break;
		}
		case 2://Thong tin NTFS
		{
			vector<vector<string>> sector;
			HANDLE disk = NULL;
			int flag = ReadSector(disk, sector);
			NTFS drive(disk, sector);
			cout << "++++++++++++++++++    THONG TIN PARTITION BOOT SECTOR     ++++++++++++++++++\n" << endl;
			drive.readBoot_Sector();
			cout << "\n=====================  CAY THU MUC  ==============================\n" << endl;
			drive.read_MFT(find_table(disk, drive.get_first_sector_MFT() * 512), drive);
			drive.findSubDirectory();
			system("pause");
			system("cls");
			break;

		}
		default:
			break;
		}
	} while (choice != 3);

	return 0;
}