#include "list.h"

#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

void Keyboard_init(List *userMsgs);
void Keyboard_shutdown(void);
void Keyboard_singalNewMsg(void);
void Keyboard_wait(void);
void Keyboard_lockMutex(void);
void Keyboard_unlockMutex(void);
void Keyboard_switchTurn(void);
void Keyboard_checkTurn(void);

#endif