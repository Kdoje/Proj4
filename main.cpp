/*
 * main.cpp
 *
 *  Created on: Oct 4, 2018
 *      Author: kl
 */
#include <stdio.h>
#include<stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/mman.h>
#include"Mailbox.h"
#include "main.h"
int main(int argc, char *argv[]){
	//initialization vars
	if (argc >= 3) {
		struct stat sb;
		char *_startingFile = argv[1];
		char *_search = stripNull(argv[2]);
		int searchSize=strlen(argv[2]);
		int blockSize = 1024;
		int fileToRead = open(_startingFile, O_RDONLY);

		//finds the size of the file
		if (fstat(fileToRead, &sb) < 0) {
			perror("Could not read file to obtain its size");
			return -1;
		}
		int sizeBytes = sb.st_size;
		printf("file size = %d bytes\n", sizeBytes);

		//part 1 of the project
		if (argc == 3) {
			if (fileToRead < 0) {
				printf("invalid file name");
				return -1;
			}

			//finds the size of the file
			if (fstat(fileToRead, &sb) < 0) {
				perror("Could not read file to obtain its size");
				return -1;
			}
			int sizeBytes = sb.st_size;
			printf("file size = %d bytes\n", sizeBytes);

			//find the number of occuranceses of the search term
			int occurences = findOccurances(fileToRead, blockSize, _search, searchSize);
			printf("Occurrences of \"%s\" is: %d", _search, occurences);

			if (close(fileToRead) < 0)
				return -1;

			return 0;
		}
		//using mmap or threads
		else if(argc==4){
			if(atoi(argv[3])){
				blockSize=atoi(argv[3]);
				int occurences = findOccurances(fileToRead, blockSize, _search, searchSize);
				printf("Occurrences of \"%s\" is: %d", _search, occurences);
				return 0;
			}
			char num[3];
			strcpy(num, &argv[3][1]);
			int threads = atoi(num);
			if(threads==0){
				if(strcmp(argv[3], "mmap")==0){
					printf("using mmap\n");
					char *_mappedFile = (char *)( mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fileToRead, 0));
					if((long)_mappedFile==-1){
						printf("couldn't map file\n");
						return -1;
					}
					return 0;
				}
				else{
					printf("invalid arg 3\n");
					return -1;
				}
			}
			else if(threads>=1 && threads<=16){
				if (argv[3][0] == 'p') {
					printf("using %d threads\n", threads);
				}
				else{
					printf("invalid arg 3\n");
					return -1;
				}
				return 0;
			}
			else{
				printf("invalid arg 3");
				return -1;
			}
		}
	}
	else{
		printf("error, please check argument count and validity");
		return -1;
	}
}

char *stripNull(char *_stringToStrip){
	int length=strlen(_stringToStrip);
	char *_finalString = (char *)malloc(length*sizeof(char));  // allocates enough for an array of 4 int
	for(int i=0; i<length; i++){
		_finalString[i]=_stringToStrip[i];
	}
	return _finalString;
}


int findOccurances(int fileDesc, int blockSize, const char *_toFind, int findSize){
	int count=0;
	unsigned char _curBlock[blockSize];
	int readSize=0;
	readSize=read(fileDesc, _curBlock, blockSize);
	while(readSize>0){
		count+=findInBlock(_curBlock, readSize, _toFind, findSize);
		readSize=read(fileDesc, _curBlock, blockSize);
	}
	return count;
}

int findInBlock(unsigned char *_block, int blockSize, const char *_toFind,  int findSize){
	int count =0;
	for(int i=0; i<blockSize; i++){
		bool found=true;
		for(int j=0; j<findSize; j++){
			if(_block[i+j]!=_toFind[j]){
				found=false;
			}
		}
		if(found){
			count++;
		}
	}
	return count;
}

