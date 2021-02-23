#include "keyboard.h"
#include "list.h"

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>			// for strncmp()
#include <unistd.h>			// for close()
#include <pthread.h>


#define MAX_MSG_LEN 512

static pthread_mutex_t s_newMsgToSendMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t isNewMessageToSend = PTHREAD_COND_INITIALIZER;
static pthread_t tPID;

static List* s_userMsgList;
static char* msgFromKeyboard;
static bool turn = true;
static bool chatEnded = false;


/*  The following mutex and wait functions are used between the Keyboard and Sender thread
*   The Keyboard file was chosen to manage the mutex because the sender is dependant on a 
*   new message being available to send.
*/
void Keyboard_lockMutex() {
    pthread_mutex_lock(&s_newMsgToSendMutex);
}

void Keyboard_unlockMutex() {
    pthread_mutex_unlock(&s_newMsgToSendMutex);
}

void Keyboard_wait() {
    pthread_cond_wait(&isNewMessageToSend, &s_newMsgToSendMutex);
}

void Keyboard_singalNewMsg() {
    pthread_cond_signal(&isNewMessageToSend);
    
}

void Keyboard_checkTurn() {
    while(turn != false) {
        pthread_cond_wait(&isNewMessageToSend, &s_newMsgToSendMutex);
    }
}

void Keyboard_switchTurn() {
    turn = true;
}


void* keyboardThread(void* unused) {

    char buffer[MAX_MSG_LEN];
    msgFromKeyboard = malloc(MAX_MSG_LEN);

    while(1) {
        pthread_mutex_lock(&s_newMsgToSendMutex);
        while(turn != true) {
            pthread_cond_wait(&isNewMessageToSend, &s_newMsgToSendMutex);
        }
        pthread_mutex_unlock(&s_newMsgToSendMutex);

        if(chatEnded == false) {
            fgets(buffer, MAX_MSG_LEN, stdin);
            if(strncmp(buffer, "!\n", MAX_MSG_LEN) == 0) {
                chatEnded = true;
            }
            List_add(s_userMsgList, buffer);
        }

        pthread_mutex_lock(&s_newMsgToSendMutex);
        turn = false;
        pthread_cond_signal(&isNewMessageToSend);
        pthread_mutex_unlock(&s_newMsgToSendMutex);
    }

    return NULL;
}


//Initilize Keyboard thread and save the user message list
void Keyboard_init(List* userMsgList) {
    s_userMsgList = userMsgList;

    pthread_create(&tPID, NULL, keyboardThread, NULL);
}

//Cancel and join the Keybard thread, frees the malloc memory if it has been malloc'd
void Keyboard_shutdown(void) {
    pthread_mutex_unlock(&s_newMsgToSendMutex);
    pthread_cancel(tPID);
    pthread_join(tPID, NULL);

    if(msgFromKeyboard) {
        free(msgFromKeyboard);
        msgFromKeyboard = NULL;
    }

    pthread_cond_signal(&isNewMessageToSend);
    pthread_cond_destroy(&isNewMessageToSend);
    pthread_mutex_unlock(&s_newMsgToSendMutex);
    pthread_mutex_destroy(&s_newMsgToSendMutex);
}
