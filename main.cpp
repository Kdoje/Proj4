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
#include <pthread.h>
#include"Mailbox.h"
#include "main.h"
const int MAX_THREAD=16;
Mailbox mailbox[MAX_THREAD+1];
char *_mappedFile, *_search;
int searchSize;
void *findThread(void *threadid) {

	//get the thread id to an int
	long tidInit;
	tidInit = (long) threadid;
	int tid = (int) tidInit;

	//read then send the message
	contents recieved;
	recieved = mailbox[tid].RecvMsg(0);
	contents toSend;
	int startInd=recieved.val1;
	int endInd=recieved.val2;
	toSend.val1=findInRange((unsigned char *)_mappedFile, startInd, endInd,
			_search, searchSize);
	toSend.iSender=tid;
	toSend.type=ALLDONE;
//	printf("sending from %d\n", tid);
	mailbox[0].SendMsg(toSend);
//	printf("message sent\n");
	pthread_exit(NULL);
}

int main(int argc, char *argv[]){
	//initialization vars
	if (argc >= 3) {
		struct stat sb;
		char *_startingFile = argv[1];
		_search = stripNull(argv[2]);
		searchSize=strlen(argv[2]);
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
				printf("invalid file name\n");
				return -1;
			}

			//finds the size of the file
			if (fstat(fileToRead, &sb) < 0) {
				perror("Could not read file to obtain its size\n");
				return -1;
			}
			int sizeBytes = sb.st_size;
			printf("file size = %d bytes\n", sizeBytes);

			//find the number of occuranceses of the search term
			int occurences = findOccurances(fileToRead, blockSize, _search, searchSize);
			printf("Occurrences of \"%s\" is: %d\n", _search, occurences);

			if (close(fileToRead) < 0)
				return -1;

			return 0;
		}
		//using mmap or threads
		else if(argc==4){
			if(atoi(argv[3])){
				blockSize=atoi(argv[3]);
				int occurances = findOccurances(fileToRead, blockSize, _search, searchSize);
				printf("Occurrences of \"%s\" is: %d\n", _search, occurances);
				return 0;
			}
			char num[3];
			strcpy(num, &argv[3][1]);
			int threads = atoi(num);
			//set up the mapped file as soon as the program determines it will be used
		    _mappedFile = (char *) (mmap(NULL, sb.st_size, PROT_READ,
					MAP_SHARED, fileToRead, 0));

			if ((long) _mappedFile == -1) {
				printf("couldn't map file\n");
				return -1;
			}

			//if the arg is mmap don't use threads
			if(threads==0){
				if(strcmp(argv[3], "mmap")==0){
					printf("using mmap\n");
					int occurances=findInBlock((unsigned char *)_mappedFile, (int) sb.st_size, _search, searchSize);
					printf("Occurrences of \"%s\" is: %d\n", _search, occurances);
					return 0;
				}
				else{
					printf("invalid arg 3\n");
					return -1;
				}
			}

			//otherwise check to see if it can, then use threads
			else if(threads>=1 && threads<=16){
				if (argv[3][0] == 'p') {
					printf("using %d threads\n", threads);
					int threadError=0;
					for (int i = 0; i < threads; i++) {
						//printf("main() creating thread %d\n", i + 1);
						pthread_t threads[threads];
						threadError = pthread_create(&threads[i], NULL, findThread,
								(void *) (i + 1)); //this sets the thread id's to be 1-> range
						if (threadError) {
							printf("couldn't make threads \n");
							exit(-1);
						}
					}

					//sends the start messages to the
					int charCount= (int) sb.st_size;
					int prevVal = 0;
					int step = charCount / threads;
					for (int i = 1; i <= threads - 1; i++) {
						contents toSend;
						toSend.val1 = prevVal + 1;
						toSend.val2 = prevVal += step;
						toSend.type = RANGE;
						toSend.iSender = 0;
						//printf("val1: %d, val2: %d\n", toSend.val1, toSend.val2);
						mailbox[i].SendMsg(toSend);
					}

					//send the last message to go through the range (fixes divisibility problem)
					contents toSend;
					toSend.val1 = prevVal + 1;
					toSend.val2 = charCount;
					toSend.type = RANGE;
					toSend.iSender = 0;
					mailbox[threads].SendMsg(toSend);
					//get messages from all the threads
					int occurances = waitForMessages(threads, mailbox);
					printf("Occurrences of \"%s\" is: %d\n", _search, occurances);
				}

				//otherwise the arg is invalid
				else{
					printf("invalid arg 3\n");
					return -1;
				}
				return 0;
			}
			else{
				printf("invalid arg 3\n");
				return -1;
			}
		}
	}
	else{
		printf("error, please check argument count and validity\n");
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

int findInRange(unsigned char *_block, int startInd, int endInd, const char *_toFind,  int findSize){
	int count =0;
	for(int i=startInd; i<endInd; i++){
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

