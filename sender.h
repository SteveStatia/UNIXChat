#include "list.h"

#ifndef _SENDER_H_
#define _SENDER_H_

void Sender_init(List* userMsgs, void* partnerPortNumber, void* machineAddress);
void Sender_shutdown(void);

#endif