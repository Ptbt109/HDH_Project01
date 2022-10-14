#include"NTFS.h"

NTFS::NTFS(HANDLE disk, vector<vector<string>> sector)
{
	device = disk;
	BootSector = sector;
}

void NTFS::readBoot_Sector()
{
	cout << "Thong tin phan vung NTFS:" << endl;

	bytes_per_sector = hexstr_to_int(to_hexstr(BootSector, 11, 0, 2, 1));
	cout << "- So byte cua sector: " << bytes_per_sector <<" bytes" << endl;

	sectors_per_cluster = hexstr_to_int(to_hexstr(BootSector, 13, 0, 1, 1));
	cout << "- So sector tren cluster: " << sectors_per_cluster <<" sectors" << endl;

	cout << "- Media descriptor: " << to_hexstr(BootSector, 5, 1, 1, 1) << " hard disk" << endl;

	cout << "- Tong so sector NTFS: " << hexstr_to_int(to_hexstr(BootSector, 8, 2, 8, 1)) <<" sectors" << endl;

	begin_MFT = hexstr_to_int(to_hexstr(BootSector, 0, 3, 8, 1));
	cout << "- Cluster bat dau cua MFT: " << begin_MFT << endl;

	cout << "- MFT mirror bat dau tai: " << hexstr_to_int(to_hexstr(BootSector, 8, 3, 8, 1)) << endl;
}

int NTFS::get_first_sector_MFT()
{
	return begin_MFT * sectors_per_cluster + 78;
}


string NTFS::get_type_file(string flag_type)
{
	string type_file;
	if (flag_type == "10000000")
	{
		type_file = "Folder";
	}
	if (flag_type == "00000020")
	{
		type_file = "File";
	}
	return type_file;
}

void NTFS::pushToMFTEntries(MFT_ENTRY entry)
{
	this->MFTEntries.push_back(entry);
}



void NTFS::findSubDirectory(int parentID, int numTab)
{

	for (int i = 0; i < MFTEntries.size(); i++)
	{
		MFT_ENTRY currentEntry = MFTEntries[i];
		if (currentEntry.parentID == parentID)
		{
			currentEntry.printEntry(numTab + 1);
			if (currentEntry.type == "Folder")
				findSubDirectory(currentEntry.recordID, numTab + 1);
		}
	}
}

void NTFS::read_MFT(vector<vector<string>> k, NTFS drive)
{
	//vector<vector<string>> k = find_table(disk, drive.get_first_sector_MFT() * 512);

	vector<vector<string>> sector;
	int total_sector = k.size() / 64;


	for (int j = 0; j < total_sector; j++)
	{
		MFT_ENTRY newEntry;
		for (int i = 0 + 64 * j; i < 64 + 64 * j; i++)
		{
			sector.push_back(k[i]);
		}
		int id_record = hexstr_to_int(to_hexstr(sector, 12, 2, 4, 1));
		newEntry.recordID = id_record;

		int start_offset_$standard_info = hexstr_to_int(to_hexstr(sector, 4, 1, 2, 1));

		int size_$standard_info = hexstr_to_int(to_hexstr(sector, (start_offset_$standard_info + 4) % 16, (start_offset_$standard_info + 4) / 16, 4, 1));

		int start_offset_data_$standard_info = hexstr_to_int(to_hexstr(sector, (start_offset_$standard_info + 20) % 16, (start_offset_$standard_info + 20) / 16, 2, 1))
			+ start_offset_$standard_info;

		int start_offset_$file_name = start_offset_$standard_info + size_$standard_info;

		int size_$file_name = hexstr_to_int(to_hexstr(sector, (start_offset_$file_name + 4) % 16, (start_offset_$file_name + 4) / 16, 4, 1));

		int start_offset_data_$file_name = hexstr_to_int(to_hexstr(sector, (start_offset_$file_name + 20) % 16, (start_offset_$file_name + 20) / 16, 2, 1))
			+ start_offset_$file_name;

		int parent_id = hexstr_to_int(to_hexstr(sector, start_offset_data_$file_name % 16, start_offset_data_$file_name / 16, 6, 1));

		newEntry.parentID = parent_id;

		string flag_type = to_hexstr(sector, (start_offset_data_$file_name + 56) % 16, (start_offset_data_$file_name + 56) / 16, 4, 1);
		string type = drive.get_type_file(flag_type);
		newEntry.type = type;

		int len_file_name_attribute = hexstr_to_int(to_hexstr(sector, (start_offset_data_$file_name + 64) % 16, (start_offset_data_$file_name + 64) / 16, 1, 1));

		int start_offset_file_name_attribute = start_offset_data_$file_name + 66;

		int modun = start_offset_file_name_attribute % 16;

		string file_name;

		//doc ten file
		if ((modun + len_file_name_attribute * 2) > 16) {
			file_name = hexstr_tostr(to_hexstr(sector, start_offset_file_name_attribute % 16, start_offset_file_name_attribute / 16,
				16 - modun, 1));
			reverse(file_name.begin(), file_name.end());
			int remain_size_file_name = len_file_name_attribute * 2 - (16 - modun);
			int now_offset = start_offset_file_name_attribute + (16 - modun);
			int q = remain_size_file_name / 16;
			int p = remain_size_file_name % 16;
			string name;
			for (int i = 0; i < q; i++)
			{
				name = hexstr_tostr(to_hexstr(sector, (now_offset + i * 16) % 16, (now_offset + i * 16) / 16,
					16, 1));
				reverse(name.begin(), name.end());
				file_name += name;
			}
			now_offset += q * 16;
			name = hexstr_tostr(to_hexstr(sector, now_offset % 16, now_offset / 16,
				p, 1));
			reverse(name.begin(), name.end());
			file_name += name;
		}
		else {
			file_name = hexstr_tostr(to_hexstr(sector, start_offset_file_name_attribute % 16, start_offset_file_name_attribute / 16,
				len_file_name_attribute * 2, 1));
			reverse(file_name.begin(), file_name.end());
		}
		newEntry.name = file_name;
		int start_offset_next_attribute = start_offset_$file_name + size_$file_name;
		int next_attribute, size_next_attribute;
		int start_offset_$data;
		int size_$data, size_of_file, start_offset_data_of_txt;
		if (flag_type == "00000020")
		{

			string ext = get_ext_file(file_name);

			if (ext == "txt" or ext == "TXT")
			{
				next_attribute = hexstr_to_int(to_hexstr(sector, start_offset_next_attribute % 16, start_offset_next_attribute / 16, 4, 1));

				if (next_attribute != 128)
				{
					size_next_attribute = hexstr_to_int(to_hexstr(sector, (start_offset_next_attribute + 4) % 16,
						(start_offset_next_attribute + 4) / 16, 4, 1));
					start_offset_$data = start_offset_next_attribute + size_next_attribute;
				}
				else
				{
					start_offset_$data = start_offset_next_attribute;
				}
				size_of_file = hexstr_to_int(to_hexstr(sector, (start_offset_next_attribute + 4) % 16,
					(start_offset_next_attribute + 4) / 16, 4, 1));
				size_$data = hexstr_to_int(to_hexstr(sector, (start_offset_next_attribute + 16) % 16,
					(start_offset_next_attribute + 16) / 16, 4, 1));
				start_offset_data_of_txt = hexstr_to_int(to_hexstr(sector, (start_offset_next_attribute + 20) % 16,
					(start_offset_next_attribute + 20) / 16, 2, 1)) + start_offset_$data;
				newEntry.size = size_$data;

				//xu li doc du lieu data file txt
				string data;
				int data_row = start_offset_data_of_txt / 16;
				int data_col = start_offset_data_of_txt % 16;
				int count = 0;
				while (count < size_$data)
				{
					data += sector[data_row][data_col];
					data_col++;
					if (data_col == 16)
					{
						data_col = 0;
						data_row += 1;
					}
					count++;
				}

				string datafile = hexstr_tostr(data);
				if (datafile.length() > 0)
					newEntry.data = datafile;
			}
			else
			{
				int size_file = hexstr_to_int(to_hexstr(sector, (start_offset_$file_name + 64) % 16,
					(start_offset_$file_name + 64) / 16, 4, 1));

				newEntry.size = size_file;
			}
		}

		if (newEntry.type.length() > 0)
		{
			this->pushToMFTEntries(newEntry);
		}
		sector.clear();
	}
}

void MFT_ENTRY::printEntry(int numTab) {

	print_Tab(numTab);
	cout << "| " << "ID Record : " << this->recordID << " | " << endl;
	print_Tab(numTab);
	cout << "| " << "ID Parent: " << this->parentID << " | " << endl;
	print_Tab(numTab);
	cout << "| " << "Type of Entry: " << this->type << " | " << endl;
	print_Tab(numTab);
	cout << "| " << "Entry name: " << this->name << " | " << endl;
	if (this->type == "File")
	{
		string opener = getOpener(get_ext_file(this->name));
		if (opener.length() > 0)
		{
			print_Tab(numTab);
			cout << "| " << opener << " | " << endl;
		}
	}

	if (this->size >= 0)
	{
		print_Tab(numTab);
		cout << "| " << "Entry size: " << this->size << " bytes" << " | " << endl;
	}

	if (this->data.length() > 0)
	{
		print_Tab(numTab);
		cout << "| " << "Data: " << this->data << " | " << endl;
	}
	print_Tab(numTab);
	cout << "--------------------------------------" << endl;
}

