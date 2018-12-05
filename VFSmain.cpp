#define _CRT_SECURE_NO_WARNINGS

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<io.h>
#include<Windows.h>

#define BUFFER_SIZE 200

#define MAXINODE 50				// maximum inode size is 50

#define READ 1
#define WRITE 2

#define MAXFILESIZE 1024		// maimum file size of our file is 1024 byte(1kb)

#define REGULAR 1				// Regular file
#define SPECIAL 2				// Special file

#define START 0
#define CURRENT 1
#define END 2

// structure of super block
typedef struct superblock
{
	int TotalInodes;
	int FreeInode;
}SUPERBLOCK, * PSUPERBLOCK;

// structure of inode
typedef struct inode
{
	char FileName[50];
	int InodeNumber;
	int FileSize;
	int FileActualSize;
	int FileType;
	char * Buffer;
	int LinkCount;
	int ReferenceCount;
	int Permission;
	struct inode * next;
}INODE , * PINODE, **PPINODE;

// structure of file table
typedef struct filetable
{
	int readoffset;
	int writeoffset;
	int count;
	int mode;
	PINODE ptrinode;
}FILETABLE,* PFILETABLE;

// structure of User File Descriptor Table
typedef struct ufdt
{
	PFILETABLE ptrfiletable;
}UFDT;

UFDT UFDTArr[MAXINODE];		// array of typr UFDT (User file descriptor)

SUPERBLOCK SUPERBLOCKobj;	// Object of Super block

PINODE head = NULL;			// head is pointer of type of PINODE

int LogIn(char *,  char *);
void InitialiseSuperBlock();
void CreateDILB();
void ls_file();
void closeAllFile();
void DisplayHelp();
int stat_file(char * );
int fstat_file(int);
int CloseFileByName(char *);
int GetFDFromName(char *);
void CloseFileByName(int);
int WriteFile(int, char *, int);
int truncate_File(char *);
int CreateFile(char *, int);
PINODE Get_Inode(char *);
int OpenFile(char *, int);
int ReadFile(int, char *, int);
int LseekFile(int, int, int);
void CreateBackUp();

int LogIn(char * str, char * pass)
{
	if (str == NULL)
	{
		return -1;
	}
	if (_stricmp(str, pass) == 0)
	{
		return 1;
	}
	else
	{
		return -1;
	}
}

void InitialiseSuperBlock()
{
	int i = 0;
	while (i < MAXINODE)
	{
		UFDTArr[i].ptrfiletable = NULL;	// Initialise all inode from INODE table
		i++;
	}
	SUPERBLOCKobj.TotalInodes = MAXINODE;	// total number of inode is 50
	SUPERBLOCKobj.FreeInode = MAXINODE;		//total number of free inode is 50
}

void CreateDILB()
{
	int i = 1;
	PINODE newn = NULL;
	PINODE temp = head;

	while (i <= MAXINODE)
	{
		newn = (PINODE)malloc(sizeof(INODE));		//allocate memory for every element of DILB block

		newn->LinkCount = 0;
		newn->ReferenceCount = 0;
		newn->FileType = 0;
		newn->FileSize = 0;
		newn->Buffer = NULL;
		newn->next = NULL;
		newn->InodeNumber = i;
		
		if (temp == NULL)		//if link list is empty controal goes in if section and allocate comming node as first node of link list
		{
			head = newn;
			temp = head;
		}
		else					// control goes  else part if link list contains atleast one node
		{
			temp->next = newn;
			temp = temp->next;
		}
		i++;
	}
	//printf("DILB created Successfully");
}

void ls_file()
{
	int i = 0;
	PINODE temp = head;

	if (SUPERBLOCKobj.FreeInode == MAXINODE)
	{
		printf("Error : There are no files.\n");
		return;
	}
	printf("----------------------------------------------------------------\n");
	printf("File Name\t Inode number\t File size\t Link count\n");
	printf("----------------------------------------------------------------\n");
	while (temp != NULL)
	{
		if (temp ->FileType != 0)
		{
			printf("%s\t\t  %d\t\t  %d\t\t  %d\n", temp->FileName, temp->InodeNumber, temp->FileActualSize, temp->LinkCount);
		}
		temp = temp->next;
	}
	printf("----------------------------------------------------------------\n");
}

void closeAllFile()
{
	int i = 0;
	while (i < 50)
	{
		if (UFDTArr[i].ptrfiletable != NULL)
		{
			UFDTArr[i].ptrfiletable->readoffset = 0;		// readoffset set to 0 from FileTable 
			UFDTArr[i].ptrfiletable->writeoffset = 0;		// writeoffset set to 0 from FileTable 
			(UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)--;		//reduced ReferenceCount by one from IIT table
			break;
		}
		i++;
	}
}

void DisplayHelp()
{
	printf("\tls : To list out all files.\n");
	printf("\tclear : To clear console.\n");
	printf("\topen : To open the file.\n");
	printf("\tclose : To close the file.\n");
	printf("\tcloseall : To close all opened file.\n");
	printf("\tread : To read the contents from file.\n");
	printf("\twrite : To write the contents into file.\n");
	printf("\texit : To Terminate the file system.\n");
	printf("\tstat : To Display information of file using name.\n");
	printf("\tfstat : To Dispaly information of file using name.\n");
	printf("\ttruncate : To Remove all data from file.\n");
	printf("\trm : To delete the file.\n");
}

int stat_file(char * name)
{
	PINODE temp = head;
	int  i = 0;
	if (name == NULL)
	{
		return -1;
	}

	while (temp != NULL)
	{
		if (strcmp(name,temp->FileName) == 0)
		{
			break;
		}
		temp = temp->next;
	}
	if (temp == NULL)
	{
		return -2;
	}

	printf("\n-------------Statistical Information about file-------------\n");
	printf("File name : %s\n", temp->FileName);
	printf("Inode number : %d\n", temp->InodeNumber);
	printf("File size : %d\n", temp->FileSize);
	printf("Actual file size : %d\n", temp->FileActualSize);
	printf("Link count : %d\n", temp->LinkCount);
	printf("Reference count : %d\n", temp->ReferenceCount);

	if (temp->Permission == 1)
	{
		printf("File permission : Read only\n");
	}
	else if (temp->Permission == 2)
	{
		printf("File permission : Write\n");
	}
	else if (temp->Permission == 3)
	{
		printf("File permission : Read & Write\n");
	}
	printf("---------------------------------------------------------------\n\n");
	return 0;
}

int fstat_file(int fd)
{
	PINODE temp = head;
	int i = 0;
	if (fd < 0)
	{
		return -1;
	}

	if (UFDTArr[i].ptrfiletable == NULL)
	{
		return -2;
	}

	temp = UFDTArr[i].ptrfiletable->ptrinode;

	printf("\n-------------Statistical Information about file-------------\n");
	printf("File name : %s\n", temp->FileName);
	printf("Inode number : %d\n", temp->InodeNumber);
	printf("File size : %d\n", temp->FileSize);
	printf("Actual file size : %d\n", temp->FileActualSize);
	printf("Link count : %d\n", temp->LinkCount);
	printf("Reference count : %d\n", temp->ReferenceCount);

	if (temp->Permission == 1)
	{
		printf("File permission : Read only\n");
	}
	else if (temp->Permission == 2)
	{
		printf("File permission : Write\n");
	}
	else if (temp->Permission == 3)
	{
		printf("File permission : Read & Write\n");
	}
	printf("---------------------------------------------------------------\n\n");
	return 0;
}

int CloseFileByName(char * name)
{
	int i = 0;
	i = GetFDFromName(name);
	if (i == -1)
	{
		return -1;
	}

	UFDTArr[i].ptrfiletable->readoffset = 0;
	UFDTArr[i].ptrfiletable->writeoffset = 0;
	(UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)--;

	return 0;
}

int GetFDFromName(char * name)
{
	int i = 0;
	while (i < 50)
	{
		if (UFDTArr[i].ptrfiletable != NULL)
		{
			if (_stricmp((UFDTArr[i].ptrfiletable->ptrinode->FileName),name) == 0)
			{
				break;
			}
		}
		i++;
	}
	if (i == 50)
	{
		return -1;
	}
	else
	{
		return i;
	}
}

void CloseFileByName(int fd)
{
	UFDTArr[fd].ptrfiletable->readoffset = 0;
	UFDTArr[fd].ptrfiletable->writeoffset = 0;
	(UFDTArr[fd].ptrfiletable->ptrinode->ReferenceCount)--;
}

int rm_file(char * name)
{
	int fd = 0;
	fd = GetFDFromName(name);
	if (fd == -1)
	{
		return -1;
	}

	(UFDTArr[fd].ptrfiletable->ptrinode->LinkCount)--;		// Decrement the link count by 1 from IIT(inode info table) table.

	if (UFDTArr[fd].ptrfiletable->ptrinode->LinkCount == 0)
	{
		((UFDTArr[fd].ptrfiletable->ptrinode->FileType) == 0);
		free(UFDTArr[fd].ptrfiletable);
	}
	UFDTArr[fd].ptrfiletable = NULL;
	(SUPERBLOCKobj.FreeInode)++;							//Increment freeInode by 1
}

void man(char * name)
{
	if (name == NULL)
	{
		return;
	}

	if (_stricmp(name,"create") == 0)
	{
		printf("Description : Used to create new regular file\n");
		printf("Usage : create File_name Permission\n");
	}
	else if (_stricmp(name, "read") == 0)
	{
		printf("Description : Used to read data from regular file\n");
		printf("Usage : read File_name\n After this enter the data that we want to write\n");
	}
	else if (_stricmp(name, "write") == 0)
	{
		printf("Description : Used to write into regular file\n");
		printf("Usage : write File_name Permission\n");
	}
	else if (_stricmp(name, "ls") == 0)
	{
		printf("Description : Used to list all information of files\n");
		printf("Usage : ls\n");
	}
	else if (_stricmp(name, "stat") == 0)
	{
		printf("Description : Used to display information of files\n");
		printf("Usage : stat File_name \n");
	}
	else if (_stricmp(name, "fstat") == 0)
	{
		printf("Description : Used to display information of files\n");
		printf("Usage : stat File_Descriptor\n");
	}
	else if (_stricmp(name, "truncate") == 0)
	{
		printf("Description : Used to remove data from file\n");
		printf("Usage : truncate File_name \n");
	}
	else if (_stricmp(name, "open") == 0)
	{
		printf("Description : Used to open exiting file\n");
		printf("Usage : open File_name mode\n");
	}
	else if (_stricmp(name, "close") == 0)
	{
		printf("Description : Used to closed opened file\n");
		printf("Usage : close File_name\n");
	}
	else if (_stricmp(name, "closeall") == 0)
	{
		printf("Description : Used to close opened files\n");
		printf("Usage : closeall\n");
	}
	else if (_stricmp(name, "lseek") == 0)
	{
		printf("Description : Used to change file offset\n");
		printf("Usage : lseek File_name ChangeInOffSet StartPoint\n");
	}
	else if (_stricmp(name, "rm") == 0)
	{
		printf("Description : Used to delete the file\n");
		printf("Usage : rm File_name \n");
	}
	else
	{
		printf("ERROR : No manual entry available.\n");
	}


}

int WriteFile(int fd, char * arr, int iSize)
{
	if (((UFDTArr[fd].ptrfiletable->mode) != WRITE) && ((UFDTArr[fd].ptrfiletable->mode) != READ + WRITE))
	{
		return -1;
	}
	if (((UFDTArr[fd].ptrfiletable->ptrinode->Permission) != WRITE) && ((UFDTArr[fd].ptrfiletable->ptrinode->Permission) != READ + WRITE))
	{
		return -1;
	}
	if ((UFDTArr[fd].ptrfiletable->writeoffset) == MAXFILESIZE)
	{
		return -2;
	}
	if ((UFDTArr[fd].ptrfiletable->ptrinode->FileType) != REGULAR)
	{
		return -3;
	}

	strncpy((UFDTArr[fd].ptrfiletable->ptrinode->Buffer) + (UFDTArr[fd].ptrfiletable->writeoffset), arr, iSize);
	(UFDTArr[fd].ptrfiletable->writeoffset) = (UFDTArr[fd].ptrfiletable->writeoffset) + iSize;
	(UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + iSize;
	return iSize;
}

int truncate_File(char * name)
{
	int fd = GetFDFromName(name);
	if (fd == -1)
	{
		return -1;
	}
	memset(UFDTArr[fd].ptrfiletable->ptrinode->Buffer, 0, 1024);

	UFDTArr[fd].ptrfiletable->readoffset = 0;
	UFDTArr[fd].ptrfiletable->writeoffset = 0;
	UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize = 0;
}

PINODE Get_Inode(char * name)
{
	int i = 0;
	PINODE temp = head;

	if (name == NULL)
	{
		return NULL;
	}

	while (temp != NULL)
	{
		if (strcmp(name, temp->FileName) == 0)
		{
			break;
		}
		temp = temp->next;
	}
	return temp;
}

int CreateFile(char * name, int permission)
{
	int i = 0;
	PINODE temp = head;

	if ((name == NULL) ||(permission == 0) ||(permission > 3))
	{
		return -1;
	}

	if (SUPERBLOCKobj.FreeInode == 0)
	{
		return -2;
	}
	if (Get_Inode(name) != NULL)
	{
		return -3;
	}

	(SUPERBLOCKobj.FreeInode)--;

	while (temp != NULL)
	{
		if (temp->FileType == 0)
		{
			break;
		}
		temp = temp->next;
	}
	while (i < MAXINODE)
	{
		if (UFDTArr[i].ptrfiletable == NULL)
		{
			break;
		}
		i++;
	}

	UFDTArr[i].ptrfiletable = (PFILETABLE)malloc(sizeof(FILETABLE));
	if (UFDTArr[i].ptrfiletable == NULL)
	{
		return -4;
	}

	UFDTArr[i].ptrfiletable->count = 1;
	UFDTArr[i].ptrfiletable->mode = permission;
	UFDTArr[i].ptrfiletable->readoffset = 0;
	UFDTArr[i].ptrfiletable->writeoffset = 0;

	UFDTArr[i].ptrfiletable->ptrinode = temp;
	strcpy_s(UFDTArr[i].ptrfiletable->ptrinode->FileName, name);
	UFDTArr[i].ptrfiletable->ptrinode->FileType = REGULAR;
	UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount = 1;
	UFDTArr[i].ptrfiletable->ptrinode->LinkCount = 1;
	UFDTArr[i].ptrfiletable->ptrinode->FileSize = MAXFILESIZE;
	UFDTArr[i].ptrfiletable->ptrinode->FileActualSize = 0;
	UFDTArr[i].ptrfiletable->ptrinode->Permission = permission;
	UFDTArr[i].ptrfiletable->ptrinode->Buffer = (char *)malloc(MAXFILESIZE);

	memset(UFDTArr[i].ptrfiletable->ptrinode->Buffer, 0, 1024);
	return 1;

}

int OpenFile(char * name, int mode)
{
	int i = 0;
	PINODE temp = NULL;
	if (name == NULL || mode <= 0)
	{
		return -1;
	}
	temp = Get_Inode(name);
	if (temp == NULL)
	{
		return -2;
	}
	if (temp->Permission < mode)
	{
		return -3;
	}

	while (i < MAXINODE)
	{
		if (UFDTArr[i].ptrfiletable == NULL)
		{
			break;
		}
		i++;
	}

	UFDTArr[i].ptrfiletable = (PFILETABLE)malloc(sizeof(FILETABLE));

	if (UFDTArr[i].ptrfiletable == NULL)
	{
		return -1;
	}
	UFDTArr[i].ptrfiletable->count = 1;
	UFDTArr[i].ptrfiletable->mode = mode;

	if (mode == READ + WRITE)
	{
		UFDTArr[i].ptrfiletable->readoffset = 0;
		UFDTArr[i].ptrfiletable->writeoffset = 0;
	}
	else if (mode == READ)
	{
		UFDTArr[i].ptrfiletable->readoffset = 0;
	}
	else if (mode == WRITE)
	{
		UFDTArr[i].ptrfiletable->writeoffset = 0;
	}
	UFDTArr[i].ptrfiletable->ptrinode = temp;
	(UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)++;

	return i;
}

int ReadFile(int fd, char * arr,int iSize)
{
	int read_size = 0;

	if (UFDTArr[fd].ptrfiletable == NULL)
	{
		return -1;
	}
	if ((UFDTArr[fd].ptrfiletable->mode != READ )&&(UFDTArr[fd].ptrfiletable->mode != READ+WRITE))
	{
		return -2;
	}
	if ((UFDTArr[fd].ptrfiletable->ptrinode->Permission != READ) && (UFDTArr[fd].ptrfiletable->ptrinode->Permission != READ + WRITE))
	{
		return -2;
	}
	if ((UFDTArr[fd].ptrfiletable->readoffset) == (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
	{
		return -3;
	}
	if (UFDTArr[fd].ptrfiletable->ptrinode->FileType != REGULAR)
	{
		return -4;
	}

	read_size = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) - (UFDTArr[fd].ptrfiletable->readoffset);
	if (read_size < iSize)
	{
		strncpy(arr, (UFDTArr[fd].ptrfiletable->ptrinode->Buffer) + (UFDTArr[fd].ptrfiletable->readoffset), read_size);
		(UFDTArr[fd].ptrfiletable->readoffset) = (UFDTArr[fd].ptrfiletable->readoffset) + read_size;
	}
	else
	{
		strncpy(arr, (UFDTArr[fd].ptrfiletable->ptrinode->Buffer) + (UFDTArr[fd].ptrfiletable->readoffset), iSize);
		(UFDTArr[fd].ptrfiletable->readoffset) = (UFDTArr[fd].ptrfiletable->readoffset) + iSize;
	}
	return iSize;
}

int LseekFile(int fd, int size, int from)
{
	if((fd < 0) || (from > 2))
	{
		return -1;
	}
	if (UFDTArr[fd].ptrfiletable == NULL)
	{
		return -1;
	}

	if ((UFDTArr[fd].ptrfiletable->mode == READ) || (UFDTArr[fd].ptrfiletable->mode == READ + WRITE))
	{
		if (from == CURRENT)
		{
			if (((UFDTArr[fd].ptrfiletable->readoffset) + size) > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
			{
				return -1;
			}
			if (((UFDTArr[fd].ptrfiletable->readoffset) + size) < 0)
			{
				return -1;
			}
			(UFDTArr[fd].ptrfiletable->readoffset) = (UFDTArr[fd].ptrfiletable->readoffset) + size;
		}
		else if (from == START)
		{
			if (size > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
			{
				return -1;
			}
			if (size < 0)
			{
				return -1;
			}
			(UFDTArr[fd].ptrfiletable->readoffset) = size;
		}
		else if (from == END)
		{
			if ((UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + size > MAXFILESIZE)
			{
				return -1;
			}
			if (((UFDTArr[fd].ptrfiletable->readoffset) + size) < 0)
			{
				return -1;
			}
			(UFDTArr[fd].ptrfiletable->readoffset) = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + size;
		}
	}
	else if (UFDTArr[fd].ptrfiletable->mode == WRITE)
	{
		if (from == CURRENT)
		{
			if (((UFDTArr[fd].ptrfiletable->writeoffset) + size) > MAXFILESIZE)
			{
				return -1;
			}
			if (((UFDTArr[fd].ptrfiletable->writeoffset) + size) < 0)
			{
				return -1;
			}
			if (((UFDTArr[fd].ptrfiletable->writeoffset) + size) > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
			{
				(UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) = (UFDTArr[fd].ptrfiletable->writeoffset) + size;
				(UFDTArr[fd].ptrfiletable->writeoffset) = (UFDTArr[fd].ptrfiletable->writeoffset) + size;
			}
			
		}
		else if (from == START)
		{
			if (size > MAXFILESIZE)
			{
				return -1;
			}
			if (size < 0)
			{
				return 0;
			}
			if (size > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
			{
				(UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) = size;
				(UFDTArr[fd].ptrfiletable->writeoffset) = size;
			}
		}
		else if (from == END)
		{
			if ((UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + size > MAXFILESIZE)
			{
				return -1;
			}
			if (((UFDTArr[fd].ptrfiletable->writeoffset) + size) < 0)
			{
				return -1;
			}
			(UFDTArr[fd].ptrfiletable->writeoffset) = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + size;
		}
	}
}

void CreateBackUp()
{

	int i = 0;
	char * FileName = NULL; 
	char * FileName2 = NULL;
	int FileInodeNo = 0;
	FILE * fptr = NULL;

	while (i < MAXINODE)
	{
		if (UFDTArr[i].ptrfiletable != NULL)
		{
			FileName = UFDTArr[i].ptrfiletable->ptrinode->FileName;
			FileName2 = strcat(UFDTArr[i].ptrfiletable->ptrinode->FileName, ".txt");
			fptr = fopen(FileName2, "w+");
			fprintf(fptr, "%s", UFDTArr[i].ptrfiletable->ptrinode->Buffer);
		}
		i++;
	}
}

int main()
{
	
	char * ptr = NULL;				// ptr is pointer of type charater
	int ret = 0;
	int fd = 0;
	int count = 0;
	char command[4][80];			// command is two-dimentional array, which hoalds 4 one-dimentional array, each one-dimentional array contains 80 element of type charactor.
	char str[80];					// str is one-dimentional array, which hoalds 80 elements of type chractor. (command maximum accepted upto 80 chractor thats why we declre size of str array is 80 )
	char arr[1024];

	InitialiseSuperBlock();			// call for initialiseing Super Block

	CreateDILB();					// call for Disk Inode link Block(DILB)

	printf("\nPlease enter password for farther access :");
	int flag = 0;
	char pass[] = "sudarshan";
	char str2[20];
	int i = 0;
	
	while (1)
	{
		printf("\nMarvellous VFS :>");
		scanf("%s", str2);
		fflush(stdin);
		flag = LogIn(str2, pass);

		if (flag == 1)
		{
			//printf("\nstring matched\n");
			break;
		}
		else {
			printf("pass is incorrect!\n");
			continue;
		}

	}
	//GetCurrentDirectory(1024, head->Buffer);
	fflush(stdin);
			while (1)
			{
				
				fflush(stdin);				// fluash the standard input
				strcpy_s(str, "");	
				// copy the command from srt chractor array
				printf("\nMarvellous VFS :>");
				fgets(str, 80, stdin);		// get(copy/read) the input upto 80 chracter from stdin(standard input) to str

				count = sscanf(str, "%s %s %s %s", command[0], command[1], command[2], command[3]);	//scan the input by space saprated and stored into command array.
				if (i == 0) //&& (count == 0))
				{
					i++;
					system("cls");
					continue;

				}
				
				if (count == 1)
				{
					if (_stricmp(command[0], "ls") == 0)
					{
						ls_file();
					}
					else if (_stricmp(command[0], "closeall") == 0)
					{
						closeAllFile();
						printf("All file closed successfully\n");
					}
					else if (_stricmp(command[0], "clear") == 0)
					{
						system("cls");
						continue;
					}
					else if (_stricmp(command[0], "help") == 0)
					{
						DisplayHelp();
						continue;
					}
					else if (_stricmp(command[0], "exit") == 0)
					{
						printf("terminating the marvellous Virtual File System.\n");
						break;
					}
					else
					{
						printf("\nERROR : Command not found !!!\n");
						continue;
					}
				}

				else if (count == 2)
				{
					if (_stricmp(command[0], "stat") == 0)
					{
						ret = stat_file(command[1]);
						if (ret == -1)
						{
							printf("ERROR : Incorrect parameters\n");
						}
						if (ret == -2)
						{
							printf("ERROR : There is no such file\n");
						}
						continue;
					}
					else if (_stricmp(command[0], "fstat") == 0)
					{
						ret = fstat_file(atoi(command[1]));
						if (ret == -1)
						{
							printf("ERROR : Incorrect parameters\n");
						}
						if (ret == -2)
						{
							printf("ERROR : There is no such file\n");
						}
						continue;
					}
					else if (_stricmp(command[0], "close") == 0)
					{
						ret = CloseFileByName(command[1]);
						if (ret == -1)
						{
							printf("ERROR : There is no such file\n");
							continue;
						}
					}
					else if (_stricmp(command[0], "rm") == 0)
					{
						ret = rm_file(command[1]);
						if (ret == -1)
						{
							printf("ERROR : There is no such file\n");
							continue;
						}
					}
					else if (_stricmp(command[0], "man") == 0)
					{
						man(command[1]);
					}
					else if (_stricmp(command[0], "write") == 0)
					{
						fd = GetFDFromName(command[1]);
						if (fd == -1)
						{
							printf("Error : Incorrect parameter\n");
							continue;
						}
						printf("Enter the Data : \n");
						scanf("%[^\n]s", arr);

						ret = strlen(arr);
						if (ret == 0)
						{
							printf("Error : Incorrect parameter\n");
							continue;
						}
						ret = WriteFile(fd, arr, ret);

						if (ret == -1)
						{
							printf("ERROR : Permission denied\n");
						}
						if (ret == -2)
						{
							printf("ERROR : There is no sufficient memory to write\n");
						}
						if (ret == -3)
						{
							printf("ERROR : It is not regular file\n");
						}
					}
					else if (_stricmp(command[0], "truncate") == 0)
					{
						ret = truncate_File(command[1]);
						if (ret == -1)
						{
							printf("Error : Incorrect parameter\n");
						}
					}
					else
					{
						printf("\nERROR : Command not fount\n");
						continue;
					}
				}

				else if (count == 3)
				{
					if (_stricmp(command[0], "create") == 0)
					{
						ret = CreateFile(command[1], atoi(command[2]));
						if (ret >= 0)
						{
							printf("File is successfully created with file descriptor : %d\n", ret);
						}
						if (ret == -1)
						{
							printf("ERROR : Incorrect parameters\n");
						}
						if (ret == -2)
						{
							printf("ERROR : There is no Inode\n");
						}
						if (ret == -3)
						{
							printf("ERROR : file already exists\n");
						}
						if (ret == -4)
						{
							printf("ERROR : Memory allocation faliure\n");
						}
						continue;
					}
					else if (_stricmp(command[0], "open") == 0)
					{
						ret = OpenFile(command[1], atoi(command[2]));
						if (ret >= 0)
						{
							printf("File is successfully opened with file descriptor : %d\n", ret);
						}
						if (ret == -1)
						{
							printf("ERROR : Incorrect parameters\n");
						}
						if (ret == -2)
						{
							printf("ERROR : File not present\n");
						}
						if (ret == -3)
						{
							printf("ERROR : Permission denied\n");
						}
						continue;
					}
					else if (_stricmp(command[0], "read") == 0)
					{
						ret = GetFDFromName(command[1]);
						if (fd == -1)
						{
							printf("Error : Incorect parameter\n");
							continue;
						}

						ptr = (char *)malloc(sizeof(atoi(command[2])) + 1);
						if (ptr == NULL)
						{
							printf("Error : Memory allocation failure\n");
							continue;
						}

						ret = ReadFile(fd, ptr, atoi(command[2]));
						if (ret == -1)
						{
							printf("ERROR : File not existing\n");
						}
						if (ret == -2)
						{
							printf("ERROR : Permission denied\n");
						}
						if (ret == -3)
						{
							printf("ERROR : Reached at end of file\n");
						}
						if (ret == -4)
						{
							printf("ERROR : It is not regular file\n");
						}
						if (ret == 0)
						{
							printf("ERROR : File empty\n");
						}
						if (ret > 0)
						{
							_write(2, ptr, ret);
						}
						continue;
					}
					else
					{
						printf("\nError : Command not fount !!!\n");
						continue;
					}
				}

				else if (count == 4)
				{
					if (_stricmp(command[0], "lseek") == 0)
					{
						fd = GetFDFromName(command[1]);
						if (fd == -1)
						{
							printf("Error : Incorrect parameter\n");
							continue;
						}

						ret = LseekFile(fd, atoi(command[2]), atoi(command[3]));
						if (ret == -1)
						{
							printf("Error : Unable to perform lseek\n");
						}
					}
					else
					{
						printf("\nError : Command not fount !!!\n");
						continue;
					}
				}

				else
				{
					printf("\nError : Command not fount !!!\n");
					continue;
				}
			}
	
	CreateBackUp();
	return 0;
}