#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>			// for strncmp()
#include <unistd.h>			// for close()
#include <pthread.h>
#include <stdbool.h>
#include <string.h>

#include "list.h"
#include "keyboard.h"
#include "printer.h"
#include "receiver.h"
#include "sender.h"

#define TERMINATE_MSG "!\n"

static List* userMsgs;
static List* partnerMsgs;

static void* USER_PORT;
static void* PARTNER_PORT;
static void* MACHINE_NAME;


int main(int argCount, char** args) {
    fputs("Welcome to s-talk!\n", stdout);

    //Handle the machine addres and cast
    USER_PORT = args[1];
    MACHINE_NAME = args[2];
    PARTNER_PORT = args[3];

    if(args[1] == NULL || args[2] == NULL || args[3] == NULL) {
        fputs("Error. Missing connection information.\n", stdout);
        return EXIT_FAILURE;
    }
    

    //Initilize the lists used by the threads
    if((userMsgs = List_create()) == NULL) {
        fputs("Error. List could not be created\n", stdout);
        return EXIT_FAILURE;
    }
    
    if((partnerMsgs = List_create()) == NULL) {
        fputs("Error. List could not be created\n", stdout);
        return EXIT_FAILURE;
    }
    
    //Initilization of the 4 threads in their modules that will be running the program
    Keyboard_init(userMsgs);
    Sender_init(userMsgs, PARTNER_PORT, MACHINE_NAME);
    Receiver_init(partnerMsgs, USER_PORT);
    Printer_init(partnerMsgs);

    //Monitors the two lists to see when the termination message is sent/received
    while(1) {
        
        if(List_last(userMsgs) != NULL) {
            if(strcmp(List_curr(userMsgs), TERMINATE_MSG) == 0) {
                break;
            }
        }

        if(List_last(partnerMsgs) != NULL) {
            if(strcmp(List_curr(partnerMsgs), TERMINATE_MSG) == 0) {
                break;
            }
        }
    }

    //Shutdown the 4 threads in the individual modules
    Sender_shutdown();
    Printer_shutdown();
    Keyboard_shutdown();
    Receiver_shutdown();
    

    //Remove the pointers and data from the two lists
    if(userMsgs != NULL && partnerMsgs != NULL) {
        List_free(userMsgs, NULL);
        List_free(partnerMsgs, NULL);
        userMsgs = NULL;
        partnerMsgs = NULL;
    }

    printf("Chat ended.\n");

    return 0;
}
