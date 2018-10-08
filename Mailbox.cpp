/*
 * Message.cpp
 *
 *  Created on: Sep 16, 2018
 *      Author: kl
 */

#include "Mailbox.h"

bool operator ==(const contents& a, const contents& b){
	return(a.val1==b.val1)&&(a.val2==b.val2)&&(a.iSender==b.iSender)&&(a.type==b.type);
}

Mailbox::Mailbox() {
	sem_init(&recSem, 0 ,0);
	sem_init(&sendSem, 0 ,1);
	this->msgContents=NULL_CONTENTS;
}

//a thread can send a message from the get-go. But only
//one message can be in the mailbox at a time.
void Mailbox::SendMsg(contents msgContents){
	sem_wait(&sendSem);
	this->msgContents=msgContents;
	sem_post(&recSem);
}

//because of the way the semaphores are set up, this won't let
//the caller read until it has a valid message
contents Mailbox::RecvMsg(int iSender){
	sem_wait(&recSem);
	retval = msgContents;
	ClearMsg();
	sem_post(&sendSem);
	return retval;
}

void Mailbox::ClearMsg() {
	msgContents=NULL_CONTENTS;
}


Mailbox::~Mailbox() {
	sem_destroy(&sendSem);
	sem_destroy(&recSem);
}

//returns the summation of val1 from all messages recieved
int waitForMessages(int messagesNeeded, Mailbox mailboxes[]) {
	int messages = messagesNeeded;
	int sum =0;
	while (messages > 0) {
		contents msgContents;
		msgContents = mailboxes[0].RecvMsg(1);
		//if at least one thread isn't done then make isDone false
		sum+=msgContents.val1;
		//printf("      read message from %d\n", msgContents.iSender);
		messages--;
	}
	return sum;
}

