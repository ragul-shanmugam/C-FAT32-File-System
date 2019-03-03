/*

Name: Ragul Shanmugam
ID  : 1001657250

*/

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>
#include <ctype.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 10    // Mav shell only supports ten arguments

struct __attribute__((__packed__)) DirectoryEntry
{
  char DIR_Name[11];
  uint8_t DIR_Attr;
  uint8_t Unused1[8];
  uint16_t DIR_FirstClusterHigh;
  uint8_t Unused2[4];
  uint16_t DIR_FirstClusterLow;
  uint32_t DIR_FileSize;
};
struct DirectoryEntry dir[16];

FILE *fp;
  char BS_OEMName[8];
  int16_t BPB_BytsPerSec;
  int8_t BPB_SecPerClus;
  int16_t BPB_RsvdSecCnt;
  int8_t BPB_NumFATS;
  int16_t BPB_RootEntCnt;
  char BS_VolLab[11];
  int32_t BPB_FATSz32;
  int32_t BPB_RootClus;
  int32_t RootDirSectors=0;
  int32_t FirstDataSector=0;
  int32_t FirstSectorofCluster=0;

int LBAToOffset(int32_t sector)
{
    return ((sector-2) * BPB_BytsPerSec) + (BPB_BytsPerSec * BPB_RsvdSecCnt)+(BPB_NumFATS * BPB_FATSz32 * BPB_BytsPerSec);
}

int16_t NextLB(uint32_t sector)
{
  uint32_t FATAddress= (BPB_BytsPerSec * BPB_RsvdSecCnt) + (sector*4);
  u_int16_t val;
  fseek(fp, FATAddress, SEEK_SET);
  fread(&val,2,1,fp);
  return val;
}

char* filestatus(char* fn);

char* filestatus(char *fn)
{
	int i;
	char *dn= dir[i].DIR_Name;
	int index=0;
	int len=0;
	char tempstr[11];
	for(i = 0; fn[i]; i++)
	{
	len++;
	fn[i] = toupper(fn[i]);
	if(fn[i] == '.')
		index = i;
	}
	if(index != 0)
	{
		for(i = 0; i < index; i++)
			tempstr[i] = fn[i];
		for(i; i < 8; i++)
			tempstr[i] = ' ';
		for(i; i < 11; i++)
			tempstr[i] = fn[(index + 1) + (i - 8)];
	}
	else if(strcmp(fn, ".") == 0)
	{
		for(i = 0; i < 11; i++)
			tempstr[i] = dn[i];
	}
	else
	{
		for(i = 0; i < (len - 1); i++)
			tempstr[i] = fn[i];
		for (i; i < 11; i++)
			tempstr[i] = ' ';
		tempstr[11] = '\0';
	}
	if(strcmp(tempstr, dn) == 0)
	{
	for(i=0;i<11;i++)
	{
	  return (char *)tempstr[i];
	}
	}
	return "Not Found";
}

int main(int argc, char* argv[])
{
  int offset;
  int filestat=0;
  int rdir;
  int i;
  int flag;
  char file[12];
  char * usr_str = (char*) malloc( MAX_COMMAND_SIZE );

  while( 1 )
  {
    // Print out the mfs prompt
    printf ("mfs> ");


    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (usr_str, MAX_COMMAND_SIZE, stdin) );


    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];


    int   token_count = 0;

        // Pointer to point to the token
        // parsed by strsep
        char *arg_ptr;
        char *working_str  = strdup( usr_str );

        // We are going to move the working_str pointer so
        // keep track of its original value so we can deallocate
        // the correct amount at the end
        char *working_root = working_str;

        // Tokenize the input strings with whitespace used as the delimiter
        while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL)
                                        && (token_count<MAX_NUM_ARGUMENTS))
        {
            token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
            if( strlen( token[token_count] ) == 0 )
            {
                token[token_count] = NULL;
            }
            token_count++;
        }

        if( token[0] == '\0')
        {
            continue;
        }
		
		/*
        Comparing the user input for open, close, info, stat, cd, ls, get, read,
		volume and exit or quit
        and perform the corresponding operations
        */
		if(strcmp(token[0],"open")== 0)
        {
          fp = fopen(token[1], "r");
          if(token[1]=='\0')
        	{
        		printf("Error: File system image not found.\n");
        		continue;
        	}
          else if(filestat==1)
          {
            printf("Error: File system image already open.\n");
        		continue;
          }
          else
          {
            filestat=1;
            printf("The File has been opened\n");
            fseek(fp, 11, SEEK_SET);
        		fread(&BPB_BytsPerSec,1,2,fp);

        		fseek(fp, 13, SEEK_SET);
        		fread(&BPB_SecPerClus,1,1,fp);

        		fseek(fp, 14, SEEK_SET);
        		fread(&BPB_RsvdSecCnt,1,2,fp);

        		fseek(fp, 16, SEEK_SET);
        		fread(&BPB_NumFATS,1,1,fp);

            fseek(fp, 17, SEEK_SET);
            fread(&BPB_RootEntCnt,1,2,fp);

        		fseek(fp, 36, SEEK_SET);
        		fread(&BPB_FATSz32,1,4,fp);

            fseek(fp, 44, SEEK_SET);
            fread(&BPB_RootClus,1,4,fp);

            rdir=(BPB_NumFATS * BPB_FATSz32 * BPB_BytsPerSec) + (BPB_RsvdSecCnt * BPB_BytsPerSec);
            fseek(fp, rdir, SEEK_SET);
            memset(&dir,0,16*sizeof(struct DirectoryEntry));
            for(i = 0; i < 16; i++)
        		{
        			fread(&dir[i],1,32,fp);
        		}

         }
        }
		else if(strcmp(token[0],"close") == 0 )
        {
          if(token[1]=='\0')
          {
            printf("Type an input image file to be closed.\n");
          }
          else if(token[1]==0)
          {
            printf("Error: File system image must be opened first.\n");
          }
          else
          {
            filestat = 0;
            printf("File system is closed.\n");
            fclose(fp);
          }
        }
		
		else if((strcmp(token[0],"info"))==0)
        {
          if(filestat== 1)
          {
            printf("BPB_BytsPersec: \t%d\t%x\n", BPB_BytsPerSec,BPB_BytsPerSec);
            printf("BPB_SecPerClus: \t%d\t%x\n", BPB_SecPerClus,BPB_SecPerClus);
            printf("BPB_NumFATS:  \t%d\t%x\n", BPB_NumFATS);
            printf("BPB_RsvdSecCnt: \t%d\t%x\n", BPB_RsvdSecCnt);
            printf("BPB_FATSz32:  \t%d\t%x\n", BPB_FATSz32);
          }
          else
          {
            printf("File System is closed.\n");
          }
        }
		
		else if(strcmp(token[0], "stat") == 0)
		{
		    if (fp!=NULL)
			{
			for (i = 0; i < 16; i++)
				{
				if (strcmp(filestatus(token[1]), dir[i].DIR_Name)==0)
					{
					  printf("Name:%s\nAttributes: %d\nLow Cluster Number: %d\nSize: %d\n", dir[i].DIR_Name, dir[i].DIR_Attr, dir[i].DIR_Name, dir[i].DIR_FileSize);
					  break;
					}
				else
					{
					printf("Error: File not found.\n");
					}
				}
			}
			else
			{
			  printf("Error: File system image must be opened first.\n");
			}
		}
	
		else if((strcmp(token[0], "ls")) == 0)
		{
		  if(fp != NULL)
		  {
			for(i = 0; i < 16; i++)
			{
			  if((dir[i].DIR_Attr == 0x01) || (dir[i].DIR_Attr == 0x10) || (dir[i].DIR_Attr == 0x20))
			  {
				char name[12];
				memcpy( name, dir[i].DIR_Name, 11);
				name[11] = '\0';
				printf("%s \t %u \t %d\n", name, dir[i].DIR_Attr, dir[i].DIR_FileSize);
			  }

			}
		  }
			else
			{
			  printf("Error: File system image must be opened first.\n");
			}
		  }
	  
	  	else if ((strcmp(token[0], "volume")) == 0)
        {
		  if(fp != NULL)
		  {
			fseek(fp, 71, SEEK_SET);
				fread(BS_VolLab, 1, 11, fp);
			if( BS_VolLab != NULL)
			{
			  printf("Volume is : '%s \n", BS_VolLab);
			}
			else
			{
			 printf("%s\n", "Error: volume name not found.");
			}
		  }
		  else
		  {
			printf("Error: File system image must be opened first.\n");
		  }

		}

		else if ( (strcmp(token[0],"quit") == 0) || (strcmp(token[0],"exit") == 0) )
        {
          exit(0);
		  fflush(NULL);
        }
		
		free( working_root );		
  }
  return 0;
}