/*
* main.cpp
*
*  Created on: May 15, 2017
*      Author: Mikey and Vincent
*/

//user includes
#include "common.h"
#include "taskThread.h"
#include "interruptThread.h"
#include "initCounter.h"

/*external variables located in common.h need declaration in project so we
  can simply declare them all here and they will be seen by the other files
*/
#define NSEC_PER_SEC    (1000000000)           /* The number of nsecs per sec. */
#define NSEC_PER_MSEC   (1000000)              //number of nsecs in milliseconds


//global variables
int state = 0;                              //state of channels
int netAngleIncrement = 0;                  //storage for temporary netAngleIncrement, to copy in to netAngleIncrement
int RealNetAngleIncrement = 0;              //storage for actual netAngleIncrement, used while being probed

std::mutex mtx;                             //probingThread mutex
std::mutex dataMtx;                         //mutex to protect data

int indexProbe = 0;
int indexOutput = 0;

double probeAngleDeg[PROBE_STORAGE_SIZE];      // the strorage space for probed data
int probeIncrement[PROBE_STORAGE_SIZE];     // the storage space for the probed incremental data
double outputNetAngle[MAX_PULSE];              // the storage space for the netAngle debug
int outputState[MAX_PULSE];                 // the storage space for the state debug
int outputNetIncrement[MAX_PULSE];          //Store the value at each interrupt
double netAngleDegree=0;                       //the net angle in degree

int failInt = 0;
int init = 0;

/*
Purpose: Entry point thread/function
*/
int main(int argc, char const *argv[]) {
    //initialize counter
    initCounter();

    //setup interrupt and task threads
    pthread_t theTaskThread;
    pthread_t theInterruptThread;
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

    //TODO something up with the pthread_setschedparam function arguements. there are too many refer to other file
    //Creation of the taskThread
    iret1 = pthread_create(&theTaskThread, &attr, taskThread,(void*) message1);    //create a thread that launch the print_message_function with the arguments  message1
    pthread_setschedparam(theTaskThread, SCHED_FIFO, &parm);                       // sets the scheduling and parameters of thread1 with SCHED_FIFO and parm1
                                                                                // if it fails, return not 0
    //Creation of the interruptThread
    iret2 = pthread_create(&theInterruptThread, &attr, interruptThread, (void*) message2);
    pthread_setschedparam(theInterruptThread, SCHED_FIFO, &parm);

    //set RT-Preempt thread priorities
    pthread_setschedprio(theTaskThread, 40);
    pthread_setschedprio(theInterruptThread, 45);

    //check if threads created correctly, if 0 then ok if. if not 9 then wtf
    printf("pthread_create() for taskThread returns: %d\n",iret1);
    printf("pthread_create() for interruptThread returns: %d\n",iret2);

    //launch threads
    pthread_join( theTaskThread, NULL);
    pthread_join( theInterruptThread, NULL);

    printf("Finished: %d\n");

    return 0;
}
