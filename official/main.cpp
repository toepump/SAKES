/*
* main.cpp
*
*  Created on: May 15, 2017
*      Author: Mikey and Vincent
*/

//system includes TODO: move neceary includes to the auxillary header files
#include <iostream>
#include <unistd.h>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sched.h>
#include <sys/mman.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <glib-2.0/glib.h>
#include <mutex>

//user includes
#include "common.h"
#include "taskThread.h"
#include "interruptThread.h"
#include "probingThread.h"
#include "counter.h"
#include "initCounter.h"
#include "printOutData.h"

#define NSEC_PER_SEC    (1000000000)           /* The number of nsecs per sec. */
#define NSEC_PER_MSEC   (1000000)              //number of nsecs in milliseconds

/*
Purpose: Entry point thread/function
*/
int main(int argc, char const *argv[]) {
    //initialize counter
    initCounter();

    //setup interrupt and task threads
    pthread_t taskThread;
    pthread_t interruptThread;
    const char *message1 = "taskThread";
    const char *message2 = "interruptThread";
    int  iret1;
    int  iret2;

    /*set attribute */
    pthread_attr_t attr;
    struct sched_param parm;
    pthread_attr_init(&attr);   //Initialize the thread attributes with default attribute

    /* Create independent thread which will execute function */
    pthread_attr_getschedparam(&attr, &parm);                   // put the scheduling param of att to parm
    parm.sched_priority = sched_get_priority_min(SCHED_FIFO);   //return the minimum priority
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);             //set the scheduling policy of attr1 as FIFIO
    pthread_attr_setschedparam(&attr, &parm);                   //set the scheduling parameter of attr1 as parm1

    //Creation of the taskThread
    iret1 = pthread_create(&taskThread, &attr, taskThread,(void*) message1);    //create a thread that launch the print_message_function with the arguments  message1
    pthread_setschedparam(taskThread, SCHED_FIFO, &parm);                       // sets the scheduling and parameters of thread1 with SCHED_FIFO and parm1
                                                                                // if it fails, return not 0
    //Creation of the interruptThread
    iret2 = pthread_create(&interruptThread, &attr, interruptThread, (void*) message2);
    pthread_attr_setschedparam(taskThread, SCHED_FIFO, &parm);

    //set RT-Preempt thread priorities
    pthread_setschedprio(taskThread, 40);
    pthread_setschedprio(interruptThread, 45);

    //check if threads created correctly, if 0 then ok if. if not 9 then wtf
    printf("pthread_create() for taskThread returns: %d\n",iret1);
    printf("pthread_create() for interruptThread returns: %d\n",iret2);

    //launch threads
    pthread_join( taskThread, NULL);
    pthread_join( interruptThread, NULL);

    printf("Finished: %d\n");

    return 0;
}
