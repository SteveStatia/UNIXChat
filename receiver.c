#include "receiver.h"
#include "list.h"
#include "sender.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <netdb.h>
#include <string.h>	
#include <unistd.h>			
#include <pthread.h>
#include <errno.h>

#define MAX_MSG_RX_LEN 512

static pthread_t tPID;
static pthread_mutex_t newMsgRxMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t isNewMsgRxCond = PTHREAD_COND_INITIALIZER;

static List* s_partnerMsgList;
static bool turn = true;
static char* msgRx;

static int socketDescriptor;
static void* s_userPortNum;


void Receiver_lockMutex() {
    pthread_mutex_lock(&newMsgRxMutex);
}

void Receiver_unlockMutex() {
    pthread_mutex_unlock(&newMsgRxMutex);
}

void Receiver_wait() {
    pthread_cond_wait(&isNewMsgRxCond, &newMsgRxMutex);
}

void Receiver_signalNewMsg() {
    pthread_cond_signal(&isNewMsgRxCond);
}

void Receiver_switchTurn() {
    turn = true;
}

void Receiver_checkTurn() {
    while(turn!= false) {
        pthread_cond_wait(&isNewMsgRxCond, &newMsgRxMutex);
    }
}

void* recieveThread(void* unused) {

    //Initilize structs for the getaddrinfo call, as well as the socketaddr_in to store that info
    struct sockaddr_in *rxSocket;
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; 
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    //Makes the call to get the information regarding the machine address given the address and port
    //Since this address is for receiving from their end we dont need an address just our port in order to receive messages
    int getAddrInfoResult = getaddrinfo(NULL, s_userPortNum, &hints, &res);
    if(getAddrInfoResult == -1) {
       printf("Error. Getting the address information. errno #%d\n", errno);
    } else {
        if(res->ai_family == AF_INET) {
            rxSocket = (struct sockaddr_in*) res->ai_addr;
        }
    }

    //Create the socket that will be used
    socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);
    if(socketDescriptor == -1) {
        printf("Error. Creating socket. errno #%d\n", errno);
    } 

    //Bind the socket
    int bindResult = bind(socketDescriptor, (struct sockaddr*) rxSocket, sizeof(struct sockaddr));
    if(bindResult == -1) {
        printf("Error. Binding socket. errno #%d\n", errno);
    }

    //Free the addrinfo from the getaddrinfo call and init variables for reciving messages
    freeaddrinfo(res);
    char buffer[MAX_MSG_RX_LEN];
    unsigned int rxSocketLen = sizeof(rxSocket);
    msgRx = malloc(MAX_MSG_RX_LEN);

    while(1) {
        Receiver_lockMutex();
        while(turn != true) {
            Receiver_wait();
        }
        Receiver_unlockMutex();

        int bytesRx = recvfrom(socketDescriptor, buffer, MAX_MSG_RX_LEN, 0, 
            (struct sockaddr *) &rxSocket, &rxSocketLen);

        if(bytesRx == -1) {
            printf("Error. Receiving packet. errno #%d\n", errno);
        } else if(bytesRx >= 0) {

            //Terminate code taken from the the workshop powerpoint
            int nullTerminate = (bytesRx < MAX_MSG_RX_LEN) ? bytesRx : MAX_MSG_RX_LEN - 1;
            buffer[nullTerminate] = 0;

            strncpy(msgRx, buffer, MAX_MSG_RX_LEN);

            int result = List_add(s_partnerMsgList, msgRx);
            if(result != 0) { //if the list is full no more message will be added
                fputs("Error. Message could not be received, list full.\n", stdout);
            } 
        }

        //Switch turns, and check if the end message was received
        Receiver_lockMutex();
        turn = false;
        Receiver_signalNewMsg();
        if(strncmp(buffer, "!\n", MAX_MSG_RX_LEN) == 0) {
            Receiver_unlockMutex();
            return NULL;
        }
        Receiver_unlockMutex();
    }

    return NULL;
}


void Receiver_init(List* partnerMsgList, void* userPortNumber) {
    s_partnerMsgList = partnerMsgList;
    s_userPortNum = userPortNumber;
    
    pthread_create(&tPID, NULL, recieveThread, NULL);
}

//Cancels & joins the thread then closes the open socket for receivng and frees memory, mutexs, and cond variables
void Receiver_shutdown(void) {
    pthread_cancel(tPID);
    pthread_join(tPID, NULL);

    close(socketDescriptor);

    if(msgRx) {
        free(msgRx);
        msgRx = NULL;
    }

    pthread_cond_signal(&isNewMsgRxCond);
    pthread_mutex_unlock(&newMsgRxMutex);
    pthread_mutex_destroy(&newMsgRxMutex);
    pthread_cond_destroy(&isNewMsgRxCond);
}



