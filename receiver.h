#include "list.h"

#ifndef _RECEIVER_H_
#define _RECEIVER_H_

void Receiver_init(List* partnerMsgs, void* userPortNUmber);
void Receiver_shutdown(void);
void Receiver_lockMutex(void);
void Receiver_unlockMutex(void);
void Receiver_wait(void);
void Receiver_switchTurn(void);
void Receiver_checkTurn(void);
void Receiver_signalNewMsg(void);

#endif