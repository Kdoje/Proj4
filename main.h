/*
 * main.h
 *
 *  Created on: Oct 4, 2018
 *      Author: kl
 */

#ifndef MAIN_H_
#define MAIN_H_

int findOccurances(int fileDesc, int blockSize, const char *_toFind, int findSize);
char *stripNull(char *_stringToStrip);
int findInBlock(unsigned char *_block, int blockSize, const char *_toFind, int findSize);

#endif /* MAIN_H_ */
