#include "sender.h"
#include "keyboard.h"

#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

static pthread_t tPID;

static List* s_userMsgList;
static void* machineAddr;
static char* partnerPortNum;

static int socketDescriptor;
struct addrinfo hints, *res;

static bool chatEnded = false;

void* senderThread(void* unused) {
    
    //Initalize addrinfo structs to be copied into from the getaddrinfo calls. 
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    int getAddrInfoResult = getaddrinfo(machineAddr, (void*)partnerPortNum, &hints, &res);
    if(getAddrInfoResult == -1) {
        printf("Error. Getting address information. errno #%d\n", errno);
        
    }

    struct sockaddr_in *partnerSocAddr;
    
    //Verfies the correct connection then reassigns with cast the struct.
    //Prints out error message if anything occurs with the socket call
    if(res->ai_family == AF_INET) {
        partnerSocAddr = (struct sockaddr_in*) res->ai_addr;
        
        socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);
        if(socketDescriptor == -1) {
            printf("Error. Creating socket. errno #%d\n", errno); 
        } else {
            void* addrIpAddr = &partnerSocAddr->sin_addr;
            char ipString[INET6_ADDRSTRLEN];
            inet_ntop(partnerSocAddr->sin_family, addrIpAddr, ipString, sizeof ipString);
            printf("Chatting with %s:%s(%s:%s)\n", (char*) machineAddr, partnerPortNum, ipString, partnerPortNum);
        }
    } else {
        fputs("Error aquiring the address information.\n", stdout);
    }

    while(1) {
        Keyboard_lockMutex();
        Keyboard_checkTurn();
        Keyboard_unlockMutex();


        //If the termination message (!\n) is recieved it will not sent any more messages to the other client.
        if(chatEnded == false) {
            int sendToResult = sendto(socketDescriptor, List_curr(s_userMsgList), 
                strlen(List_curr(s_userMsgList)), 0, (struct sockaddr*) partnerSocAddr, 
                sizeof(struct sockaddr));

            if(sendToResult == -1) {
                printf("Error. Sending packet. errno #%d\n", errno);
            }
        } 

        //Switch turns and checks for the termination message to block further outgoing messages.
        Keyboard_lockMutex();
        Keyboard_switchTurn();
        Keyboard_singalNewMsg();
        if(strcmp((char*) List_curr(s_userMsgList), "!\n") == 0) {
            chatEnded = true;
        }
        Keyboard_unlockMutex();

        
    }
    return NULL;
}

void Sender_init(List* userMsgs, void* partnerPortNumber, void* machineAddress) {
    s_userMsgList = userMsgs;
    partnerPortNum = partnerPortNumber;
    machineAddr = machineAddress;

    pthread_create(&tPID, NULL, senderThread, NULL);
}


//Cancels & joins the thread, then closes the socket for sending and frees the getaddrinfo call.
void Sender_shutdown() {
    pthread_cancel(tPID);
    pthread_join(tPID, NULL);

    close(socketDescriptor);
    freeaddrinfo(res);
}