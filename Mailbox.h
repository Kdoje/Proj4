/*
 * Message.h
 *
 *  Created on: Sep 16, 2018
 *      Author: kl
 */

#ifndef MAILBOX_H_
#define MAILBOX_H_

#define RANGE 1
#define ALLDONE 2
#define NOTDONE 3
#define EMPTY -1
#include<semaphore.h>
#include<stdlib.h>
struct contents{
	int val1;
	int val2;
	int iSender;
	int type;
};
typedef struct contents contents;
const contents NULL_CONTENTS ={
		.val1=0,
		.val2=0,
		.iSender=EMPTY,
		.type=EMPTY,
};

bool operator ==(const contents& a, const contents& b);


class Mailbox {
	sem_t recSem, sendSem;
	contents msgContents;
	contents retval;
private:
	void ClearMsg();
public:
	Mailbox();
	void SendMsg(contents msgContents);
	contents RecvMsg(int iSender);
	virtual ~Mailbox();
};

#endif /* MAILBOX_H_ */
