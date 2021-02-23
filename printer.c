#include "printer.h"
#include "receiver.h"

#include <pthread.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>	
#include <stdlib.h>

#define MAX_MSG_LEN 512

static pthread_t tPID;
static List* s_partnerMsgList;
static char* rxMsgToPrint;


void* printerThread(void* unused) {
    
    rxMsgToPrint = malloc(MAX_MSG_LEN);

    while(1) {
        Receiver_lockMutex();
        Receiver_checkTurn();
        Receiver_unlockMutex();

        //Copy the message into memory
        if(List_last(s_partnerMsgList) != NULL) {
            char *buffer = List_curr(s_partnerMsgList);
            
            strcpy(rxMsgToPrint, buffer);

            //Print message to the screen
            fputs(rxMsgToPrint, stdout);

        } 

        //Relinquish mutex
        Receiver_lockMutex();
        Receiver_switchTurn();
        Receiver_signalNewMsg();
        Receiver_unlockMutex();
    }

    return NULL;
}

void Printer_init(List* partnerMsgList) {
    s_partnerMsgList = partnerMsgList;

    pthread_create(&tPID, NULL, printerThread, NULL);
}


void Printer_shutdown() {
    pthread_cancel(tPID);
    pthread_join(tPID, NULL);    

    if(rxMsgToPrint) {
        free(rxMsgToPrint);
        rxMsgToPrint = NULL;
    }
}