// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

// ham tang PC
void IncreaseProgramCounter()
{
	// Cau c: Tang gia tri bien program counter
	
	int counter = machine->ReadRegister(PCReg);

	// Vi tri truoc do gan cho vi tri hien tai
   	machine->WriteRegister(PrevPCReg, counter);
	
    counter = machine->ReadRegister(NextPCReg);

	// Vi tri hien tai gan cho vi tri tiep theo
    machine->WriteRegister(PCReg, counter);

	// Vi tri tiep theo cong 4 byte
   	machine->WriteRegister(NextPCReg, counter + 4);
}

/* 
Input: - User space address (int) 
 - Limit of buffer (int) 
Output:- Buffer (char*) 
Purpose: Copy buffer from User memory space to System memory space 
*/
char* User2System(int address,int limit) {
	char* buff = NULL;

	buff = new char[limit +1]; //Cap phat 
	if (buff == NULL)
		return buff;

	memset(buff,0,limit+1);

	int c;
	for (int i = 0 ; i < limit ;i++) {
		machine->ReadMem(address + i, 1, &c);
		buff[i] = (char)c;
		if (c == 0)	//Neu gap ki tu ket thuc chuoi thi dung lai.
			break;
	}
	return buff;
}


/* 
Input: - User space address (int) 
 - Limit of buffer (int) 
 - Buffer (char[]) 
Output:- Number of bytes copied (int) 
Purpose: Copy buffer from System memory space to User memory space 
*/
int System2User(int address,int limit, char* buff)
{
	if (limit < 0) 
		return -1;
	if (limit == 0)
		return limit;

	int i = 0, c = 0;
	do {
		c= (int) buff[i];
		machine->WriteMem(address + i,1,c);
		i++;
	}while(i < limit && c != 0);
	return i;
}

void Exception_CreateFile()
{
    int addr;
    char *fileName;

    addr = machine->ReadRegister(4);

    fileName = User2System(addr, 255);

    if (fileName == NULL)
    {
        printf("\nKhong du bo nho");
        DEBUG('a', "\nKhong du bo nho");
        machine->WriteRegister(2, -1); // Nếu lỗi thì ghi vào thanh ghi số 2 giá trị -1
    }

    if (strlen(fileName) == 0)
    {
        printf("\nTen file khong duoc trong");
        DEBUG('a', "\nTen file khong duoc trong");
        machine->WriteRegister(2, -1); // Nếu lỗi thì ghi vào thanh ghi số 2 giá trị -1
    }

    // Create file with size = 0
    // Dùng đối tượng fileSystem của lớp OpenFile để tạo file
    if (fileSystem->Create(fileName, 0) == -1)
    {
        printf("\nXay ra loi khi tao file (%s)", fileName);
        machine->WriteRegister(2, -1);
    }

    machine->WriteRegister(2, 0); // Nếu không có lỗi ghi vào thanh ghi số 2
    delete[] fileName;            // Giải phóng vùng nhớ cho filename
}

void Exception_Open()
{
    int addr;
    int type; 
    char* filename;

    //Lấy các tham số từ thanh ghi
    addr = machine->ReadRegister(4); 
    type = machine->ReadRegister(5); 
    filename = User2System(addr, 255);

    int freeSlot = fileSystem->FindFreeSlot();
    
    if (freeSlot == -1){
        machine->WriteRegister(2, -1); //Trả lỗi về cho người dùng
    }

    switch (type){
    case 0: 
        fileSystem->openFileTable[freeSlot] = fileSystem->Open(filename, type);
        if (fileSystem->openFileTable[freeSlot] != NULL)
        {
            machine->WriteRegister(2, freeSlot); //trả về vị trí còn trống
        }
        break;
    case 1:
        fileSystem->openFileTable[freeSlot] = fileSystem->Open(filename, type);
        if (fileSystem->openFileTable[freeSlot] != NULL) //Mo file thanh cong
        {
            machine->WriteRegister(2, freeSlot); //trả về vị trí còn trống
        }
        break;
    case 2:
        machine->WriteRegister(2, 0);
        break;
    case 3:
        machine->WriteRegister(2, 1);
        break;
    }
 
    delete[] filename;
}

void Exception_Close()
{
    int fileId;
    fileId = machine->ReadRegister(4); 
    if (fileId >= 0 && fileId <= 10) 
    {
        if (fileSystem->openFileTable[fileId]) 
        {
            machine->WriteRegister(2, 0);   //Trả kết quả thành công
            delete[] fileSystem->openFileTable[fileId]; 
        }
    }
    machine->WriteRegister(2, -1);  // Trả lỗi
}

void Exception_Read()
{
    int addr;
    int charCount;
    int id;

    int firstPositionInFile;
    int lastPositionInFile;
    char *buffer;

    //Lấy giá trị tham số từ thanh ghi
    addr = machine->ReadRegister(4);
    charCount = machine->ReadRegister(5); 
    id = machine->ReadRegister(6);  

    // Nếu nằm ngoài bảng mô tả thì trả lỗi
    if (id < 0 || id > 14)
    {
        printf("\nFile ID nam ngoai bang mo ta");
        machine->WriteRegister(2, -1);
        return;
    }

    //Kiểm tra file tồn tại
    if (fileSystem->openFileTable[id] == NULL)
    {
        printf("\nFile khong ton tai.");
        machine->WriteRegister(2, -1);
        return;
    }

    if (fileSystem->openFileTable[id]->type == 3) 
    {
        printf("\nKhong the read file stdout.");
        machine->WriteRegister(2, -1);
        return;
    }

    buffer = User2System(addr, charCount); 
    //file stdin 
    if (fileSystem->openFileTable[id]->type == 2)
    {
        // Số byte thực sự đọc được
        int size = gSynchConsole->Read(buffer, charCount); 
        System2User(addr, size, buffer); 
        machine->WriteRegister(2, size); 
        delete[] buffer;
        return;
    }

    firstPositionInFile = fileSystem->openFileTable[id]->GetCurrentPos();
    fileSystem->openFileTable[id]->Read(buffer, charCount);
    lastPositionInFile = fileSystem->openFileTable[id]->GetCurrentPos();

    //Số byte thực sự = lastPositionInFile - firstPositionInFile
    int size = lastPositionInFile - firstPositionInFile;

    //Trường hợp file khác rỗng
    if (size > 0){
        System2User(addr, size, buffer); 
        machine->WriteRegister(2, size);
        delete[] buffer;
        return;
    }

    //Trường hợp còn lại file null trả về -2
    machine->WriteRegister(2, -2); 
    delete[] buffer;
}

void Exception_Write()
{
    int addr;
    int charCount;
    int id;

    int firstPositionInFile;
    int lastPositionInFile;
    char *buffer;

    //Lấy giá trị tham số từ thanh ghi
    addr = machine->ReadRegister(4);
    charCount = machine->ReadRegister(5); 
    id = machine->ReadRegister(6);  

    // Nếu nằm ngoài bảng mô tả thì trả lỗi
    if (id < 0 || id > 14)
    {
        printf("\nFile ID nam ngoai bang mo ta");
        machine->WriteRegister(2, -1);
        return;
    }

    //Kiểm tra file tồn tại
    if (fileSystem->openFileTable[id] == NULL)
    {
        printf("\nFile khong ton tai.");
        machine->WriteRegister(2, -1);
        return;
    }

    if (fileSystem->openFileTable[id]->type == 1 || fileSystem->openFileTable[id]->type == 2) 
    {
        printf("\nKhong the viet file stdin hoac file chi doc.");
        machine->WriteRegister(2, -1);
        return;
    }

    firstPositionInFile = fileSystem->openFileTable[id]->GetCurrentPos();
    fileSystem->openFileTable[id]->Write(buffer, charCount);
    lastPositionInFile = fileSystem->openFileTable[id]->GetCurrentPos();

    //Số byte thực sự = lastPositionInFile - firstPositionInFile
    int size = lastPositionInFile - firstPositionInFile;

    //Xét với file chỉ đọc và viết thì trả về số byte thật sự
    if (fileSystem->openFileTable[id]->type == 0)
    {
        if (size > 0)
        {
            machine->WriteRegister(2, size);   //Trả về số byte thật sự
            delete[] buffer;
            return;
        }
    }

    //Với file stdout
    if (fileSystem->openFileTable[id]->type == 3) 
    {
        int i;
        for (i = 0; buffer[i] != '\0'; i++)
        {
            gSynchConsole->Write(buffer + i, 1);
        }
        machine->WriteRegister(2, i - 1); // Tra ve so byte thuc su write duoc
        delete[] buffer;
    }
}

void Exception_Exec()
{
    int addr;
    addr = machine->ReadRegister(4);	// doc dia chi ten chuong trinh tu thanh ghi r4
    char* name;
    name = User2System(addr, 255); // Lay ten chuong trinh, nap vao kernel

    if (name == NULL)
    {
        DEBUG('a', "\nKhong du bo nho");
        printf("\nKhong du bo nho");
        machine->WriteRegister(2, -1);
        return;
    }

    OpenFile *openFile = fileSystem->Open(name);
    if (openFile == NULL)
    {
        printf("\n Khong mo duoc file chuong trinh.");
        machine->WriteRegister(2,-1);
        return;
    }

    delete openFile;

    // id của tiểu trình
    int id = pTab->ExecUpdate(name); 
    machine->WriteRegister(2, id);

    delete[] name;
}

void Exception_Join()
{
	int id = machine->ReadRegister(4);
		
	int res = pTab->JoinUpdate(id);
	
	machine->WriteRegister(2, res);
}

void Exception_Exit()
{
    
}
// Ham xu ly ngoai le runtime Exception va system call

void ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);
	int i;
	char *buff = NULL;

	// Bien cua readInt	
	int numlen, num, sign, result;

	// Bien cua printInt
	int n, lengthStr, temp;
	const int minInt = -2147483648;
	int len, iIndex;
	char tmp;

	// Bien cua readChar va printChar
	char c;

	// Bien cua readString va printString
	int addr, length, size;
	switch (which) {
		case NoException:
			return;
		case PageFaultException: {
			printf("No valid translation found\n");
			interrupt->Halt();			
			break;
		}
		case ReadOnlyException: {
			printf("Write attempted to page marked 'read-only'\n");
			interrupt->Halt();			
			break;
		}
		case BusErrorException: {
			printf("Translation resulted in an invalid physical address\n");
			interrupt->Halt();			
			break;

		}
		case AddressErrorException: {
			printf("Unaligned reference or one that was beyond the end of the address space\n");
			interrupt->Halt();			
			break;
		}
		case OverflowException: {
			printf("Integer overflow in add or sub\n");
			interrupt->Halt();			
			break;
		}
		case IllegalInstrException: {
			printf("Unimplemented or reserved instr\n");
			interrupt->Halt();			
			break;
		}
		case NumExceptionTypes: {	
			printf("Error\n");
			interrupt->Halt();			
			break;
		}
		case SyscallException: {
			switch(type) {
				case SC_Halt: {
					DEBUG('a', "Shutdown, initiated by user program.\n");
					interrupt->Halt();	
					break;
				}
				
				case SC_Exit: {
					int exitStatus = machine->ReadRegister(4); // doc exit status cua tien trinh da join tu thanh ghi r4
					int exitCode = ptable->ExitUpdate(exitStatus); // tra ve ket qua va luu vao exitCode
					machine->WriteRegister(2, exitCode); // ghi gia tri exitCode vao thanh ghi r2
					break;
				}
				case SC_Exec: {
					int address = machine->ReadRegister(4); // doc dia chi tu thanh ghi r4
					char *namefile = User2System(address, 255); // chuyen tu user space sang kernel space
					if (fileSystem->Open(namefile) == NULL){ // neu loi
						gsc->Write("\nKhong mo duoc file", 19); // thong bao khong mo duoc  file
						machine->WriteRegister(2, -1); // gan -1 vao thanh ghi r2
						break;
					}
					int id = ptable->ExecUpdate(namefile); // chay chuong trinh
					machine->WriteRegister(2, id); // ghi ket qua thuc thi vao thanh ghi r2
				
					break;
				}
				case SC_Join: {
					int id = machine->ReadRegister(4); // doc id cua tien trinh can join tu thanh ghi r4
					int exitCode = ptable->JoinUpdate(id); // goi thuc hien va tra ve exitCode
					machine->WriteRegister(2, exitCode); // ghi exitCode vao thanh ghi r2
					break;
				}
				case SC_CreateFile: 
					Exception_CreateFile();
					break;
				
				

				case SC_Open:
					Exception_Open();
					break;
				

				case SC_Close:
					Exception_Close();
					break;

				case SC_Read:
					Exception_Read();
					break;
				case SC_Write:
					Exception_Write();
					break;
		
				case SC_Exec:
					Exception_Exec();
					break;
				case SC_Join:       
					Exception_Join();
					break;

				case SC_Exit:
				{
					//void Exit(int status);
					// Input: status code
					int exitStatus = machine->ReadRegister(4);

					if(exitStatus != 0)
					{
						Increase_ProgramCounter();
						return;
				
					}			
			
					int res = pTab->ExitUpdate(exitStatus);

					currentThread->FreeSpace();
					currentThread->Finish();
					break;
				}
				case SC_CreateSemaphore:
				{
					int virtAddr = machine->ReadRegister(4);
					int semval = machine->ReadRegister(5);

					char *name = User2System(virtAddr, MaxFileLength + 1);
					if(name == NULL)
					{
						DEBUG('a', "\n Not enough memory in System");
						printf("\n Not enough memory in System");
						machine->WriteRegister(2, -1);
						delete[] name;
						break;
					}
			
					int res = semTab->Create(name, semval);

					if(res == -1)
					{
						DEBUG('a', "\n Khong the khoi tao semaphore");
						printf("\n Khong the khoi tao semaphore");
						machine->WriteRegister(2, -1);
						delete[] name;
						break;		
					}
			
					delete[] name;
					machine->WriteRegister(2, res);
					break;
				}

				case SC_Wait:			
				{
					// int Wait(char* name)
					int virtAddr = machine->ReadRegister(4);

					char *name = User2System(virtAddr, MaxFileLength + 1);
					if(name == NULL)
					{
						DEBUG('a', "\n Not enough memory in System");
						printf("\n Not enough memory in System");
						machine->WriteRegister(2, -1);
						delete[] name;
						break;
					}
			
					int res = semTab->Wait(name);

					if(res == -1)
					{
						DEBUG('a', "\n Khong ton tai ten semaphore nay!");
						printf("\n Khong ton tai ten semaphore nay!");
						machine->WriteRegister(2, -1);
						delete[] name;
						break;		
					}
			
					delete[] name;
					machine->WriteRegister(2, res);
					break;
				}
				case SC_Signal:		
				{
					// int Signal(char* name)
					int virtAddr = machine->ReadRegister(4);

					char *name = User2System(virtAddr, MaxFileLength + 1);
					if(name == NULL)
					{
						DEBUG('a', "\n Not enough memory in System");
						printf("\n Not enough memory in System");
						machine->WriteRegister(2, -1);
						delete[] name;
						break;
					}
			
					int res = semTab->Signal(name);

					if(res == -1)
					{
						DEBUG('a', "\n Khong ton tai ten semaphore nay!");
						printf("\n Khong ton tai ten semaphore nay!");
						machine->WriteRegister(2, -1);
						delete[] name;
						break;	
					}
			
					delete[] name;
					machine->WriteRegister(2, res);
					break;
				case SC_ReadInt: {
							buff = new char[20];
					
							// doc toi da 20 ki tu va tra ve so ki tu doc duoc
							numlen = gsc->Read(buff,20);
							// So nguyen duoc nhap o dang so duong
							num = 0;
							// Bien bieu dien dau (+): duong hoac (-): am cua so
							sign = 1;
											
							for(i = 0; i < numlen; i++)
							{
								if(i == 0)
								{
									if(buff[i] == '-')
									{
										sign = sign * (-1);
										continue;			
									}
									else if(buff[i] == '+')
									{
										sign = sign * 1;
										continue;			
									}
									else if(buff[i] == ' ')
										continue;
								}
								if((buff[i] >= '0') && (buff[i] <= '9'))
									num = (num * 10) + (buff[i] - '0');
								else 
								{
									num = 0;
									break;
								}
							}
			
							if(num != 0)
								result = num * sign;
							else
								result = num;

							machine->WriteRegister(2,result);
						
							break;
						}
				case SC_PrintInt: {
					n = machine->ReadRegister(4);
					
					// Chuyen so thanh chuoi
					buff = new char[255];
					if (n == 0) {
						buff = "0";
						lengthStr = 1;	// Cap nhat lai length
					}
					else if (n <= minInt) { // Neu n la so nho hon so nho nhat cua kieu int	
						buff = "-2147483648";
						lengthStr = 11;	// Cap nhat lai length
					}
					else {
						sign = 0;
						i = 0, temp = 0;

						// kiem tra n la so duong hay so am, neu am thi chuyen thanh duong
						if (n >= 0) {
							temp = n;
						}
						else {
							temp = n * -1;
							i = 1;
							sign = 1;
						}

						// Chuyen so duong thanh chuoi nhung bi dao nguoc
						while (temp != 0) {
							buff[i] = temp % 10 + 48;
							temp /= 10;
							i++;
						}
						buff[i] = '\0';
						

						// su dung bien len de lay 1 nua do dai cua chuoi
						lengthStr = i;
						len = 0; iIndex = 0;
						if (sign) {
							len = (float)lengthStr / 2.0 + 0.5;
							iIndex = 0;
						}
						else {
							len = lengthStr / 2;
							iIndex = 1;
						}
						i = sign; // gan sign cho i, neu so do la so am thi chua phan tu dau tien ra 

						// dao chuoi de duoc chuoi can tim
						while (i < len) {
							tmp = buff[lengthStr - i - iIndex];
							buff[lengthStr - i - iIndex] = buff[i];
							buff[i] = tmp;
							i++;
						}
						buff[lengthStr] = '\0';
						if (sign)
							buff[0] = '-';
					}

					gsc->Write(buff,lengthStr);
					
					break;
					
				}
				case SC_ReadChar: {
					buff = new char[2];	// khai bao mang str
					gsc->Read(buff,2);		// doc ki tu duoc nhap vao mang mang str 
					
					c = buff[0];		// ki tu dau cua mang str la ki tu char duoc doc vao
					machine->WriteRegister(2,c);
					
					break;
				}
				case SC_PrintChar: {
					char ch = machine->ReadRegister(4);
					gsc->Write(&ch, 1);
					break;
				}
				case SC_ReadString: {
					//Lay dia chi ki tu dau tien cua chuoi
					addr = machine->ReadRegister(4);
					//Lay do dai chuoi cho phep
					length = machine->ReadRegister(5);

					buff = new char[length +1]; //Cap phat bo nho dua tren do dai chuoi cho phep
					if (buff == NULL)
						break;
					memset(buff,0,length + 1);	// khoi tao gia tri 0 cho chuoi buff

					size = gsc->Read(buff, length);     //Lay so byte thuc su doc duoc
					System2User(addr, size, buff);  //copy chuoi tu vung nho system space sang user space co do dai la so byte thuc su cua chuoi nguoi dung nhap
					
					break;
				}
				case SC_PrintString: {
					int len = 0;
					int address = machine->ReadRegister(4);
					char* str = User2System(address, 255);
					for (int j = 0;;j++) {
						len++;
						if (str[j] == 0)
							break;
					}
					gsc->Write(str,len);
					if (str != NULL)
						delete[] str;

					break;
				}

			}
	
			// Goi ham tang bien program counter
			IncreaseProgramCounter();
			break;
		}
		default:
			printf("Unexpected user mode exception %d %d\n", which, type);
			ASSERT(FALSE);
	}	
}
