/*
* TestRT_3.cpp
*
*  Created on: May 15, 2017
*      Author: Mikey and Vincent
*/

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
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<glib-2.0/glib.h>
#include <mutex>

//constants
MAX_PULSE = 30000
PROBE_STORAGE_SIZE = 20000;             // the arbitrary size of stored
                                        // probe's storage
//prototypes
void *interruptThread(void *ptr);
void *taskThread(void *ptr);
void *probingThread(void *ptr);
void initCounter(void);
void counter(int channelSig);
static gboolean EventA( GIOChannel *channel, GIOCondition condition, gpointer user_data );
static gboolean EventB( GIOChannel *channel, GIOCondition condition, gpointer user_data );


//global variables
int state = 0;                              //state of channels
int netAngleIncrement = 0;                  //storage for temporary netAngleIncrement, to copy in to netAngleIncrement
int RealNetAngleIncrement = 0;              //storage for actual netAngleIncrement, used while being probed
const int INTERVAL =1000000;                //in nanosecond
std::mutex mtx;                             //probingThread mutex
std::mutex dataMtx;                         //actual mutex to prevent data corruption

int index = 0;

double probeAngleDeg[PROBE_STORAGE_SIZE];   // the strorage space for probed data
double outputNetAngle[MAX_PULSE];           // the storage space for the netAngle debug
int outputState[MAX_PULSE];                 // the storage space for the state debug
int outputNetIncrement[MAX_PULSE];          //Store the value at each interrupt

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


void counter(int channelSig){
    //TODO store actual netAngleIncrement and temp netAngleIncrement seperately
    // and protect the actual one with a mutex so that the probe reads a safe
    // data. every if statement update temp and copy to actual but surround actual
    // with mutex CONDITION VARIABLE MAYBE
    init++;

    if(init>2 && indexOutput<MAX_PULSE){
        if(state==1){
            if(nb_signal==2){
                netAngleIncrement++;
                RealNetAngleIncrement = netAngleIncrement;
                state=2;
            }else if(nb_signal==1){
                netAngleIncrement--;
                RealNetAngleIncrement = netAngleIncrement;
                state=4;
            }else{
                failInt++;
                //cout << "problem with the counter in case 1" << endl;
            }
        }

        else if(state==2){
            if(nb_signal==1){
                netAngleIncrement++;
                RealNetAngleIncrement = netAngleIncrement;
                state=3;
            }else if(nb_signal==2){
                state=1;
                netAngleIncrement--;
                RealNetAngleIncrement = netAngleIncrement;
            }else{
                failInt++;
                //cout << "problem with the counter in case 1" << endl;
            }
        }

        else if(state==3){
            if(nb_signal==2){
                netAngleIncrement++;
                RealNetAngleIncrement = netAngleIncrement;
                state=4;
            }else if(nb_signal==1){
                netAngleIncrement--;
                RealNetAngleIncrement = netAngleIncrement;
                state=2;
            }else{
                failInt++;
                //cout << "problem with the counter in case 1" << endl;
            }
        }

        else if(state==4){
            if(nb_signal==1){
                netAngleIncrement++;
                RealNetAngleIncrement = netAngleIncrement;
                state=1;
            }else if(nb_signal==2){
                netAngleIncrement--;
                RealNetAngleIncrement = netAngleIncrement;
                state=3;
            }else{
                failInt++;
                //cout << "problem with the counter in case 1" << endl;
            }

        }
        else{
            cout<<"state is fucked" << endl;
        }

        netAngleDegree=double(netAngleIncrement)/PULSE_PER_DEGREE;

        outputNetIncrement[indexOutput]=netAngleIncrement;
        outputNetAngle[indexOutput]=netAngleDegree;
        outputState[indexOutput] = state;
        indexOutput++;

        if(indexOutput+1==MAX_PULSE){
            printOutData();
        }
    }
}

/*
Purpose: thread for spwaning a probing thread every millisecond
         only launches a thread if previously spawned thread has Finished
         Otherwise, program breaks because of control discrepancy
*/
void *taskThread(void *ptr){
    char *message;
    message = (char *) ptr;
    struct timespec t_taskThread;                   //struct for keeping time (not actual monotonic clock)



    clock_gettime(CLOCK_MONOTONIC,&t_taskThread);   //get the current time and store in the timespec struct

    t_taskThread.tv_sec++;                          //increment the timespec struct time by one full second so that
                                                    //we can get a delay in the next step

    while(true) {
        /* wait until next shot */
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t_taskThread, NULL);   //delay until time stored in the timespec

        pthread_t probingThread;
        const char *message1 = "probingThread";
        int  iret1;

        pthread_attr_t attr;
        struct sched_param parm;
        pthread_attr_init(&attr);

        /* Create independent thread which will execute function */
        pthread_attr_getschedparam(&attr, &parm);                               // put the scheduling param of att to parm
        parm.sched_priority = sched_get_priority_min(SCHED_FIFO);               //return the minimum priority
        pthread_attr_setschedpolicy(&attr, SCHED_FIFO);                         //set the scheduling policy of attr1 as FIFIO
        pthread_attr_setschedparam(&attr, &parm);                               //set the scheduling parameter of attr1 as parm1

        //Creation of the probingThread
        iret1 = pthread_create(&probingThread, &attr, probingThread,(void*) message1);      //create a thread that launch the print_message_function with the arguments  message1
        pthread_setschedparam(probingThread, SCHED_FIFO, &parm);                            // sets the scheduling and parameters of thread1 with SCHED_FIFO and parm1
                                                                                            // if it fails, return not 0
        //set RT-Preempt thread priorities
        pthread_setschedprio(probingThread, 45);

        //check if threads created correctly, if 0 then ok if. if not then wtf
        printf("pthread_create() for probingThread returns: %d\n",iret1);

        //check if the previous thread completed or not
        if(!mtx.try_lock()){
            //launch new probingThread
            pthread_join(probingThread, NULL);
        }
        else{
            //if previous probingThread still has mutex, program is too slow
            cout<<"PROBINGTHREAD IS TOO SLOW"<<endl;
        }
        /* calculate next shot */
        t_taskThread.tv_nsec += INTERVAL;

        while (t_taskThread.tv_nsec >= NSEC_PER_SEC) {
            t_taskThread.tv_nsec -= NSEC_PER_SEC;
            t_taskThread.tv_sec++;
        }
    }

    return (void*) NULL;
}

/*
Purpose: every millisecond, forcibly take mutex protecting encoder data
         and do complete processing of that data within 1 millisecond.
*/
void *probingThread(void *ptr){
    //do stuff on mutex protected data
    probeAngleDeg[index] = netAngleDegree; //store current netAngleDegree
    probeIncrement[index] = indexOutput;
    index++;                            //increment index

    if(index > PROBE_STORAGE_SIZE || indexOutput-1 == MAX_PULSE){               //if index is past storage limit, print
        cout << "number of failure: " << failInt << endl;
        printProbe();
        return (void*) NULL;
    }
    //release mutex if finished
    mtx.unlock();
    return (void*) NULL;
}

/*
Purpose: constantly be interrupted by encoder pulses found through GPIO
         in linux filesystem 'value' files. Upon receiving interrupt from either
         channel, call EventA or EventB functions.
*/
void *interruptThread(void *ptr){
    char *message;
    message = (char *) ptr;

    while(true) {

        //initialize the looping for interrupt handling of both channels
        GMainLoop* loopA = g_main_loop_new(0, 0);
        GMainLoop* loopB = g_main_loop_new(0, 0);

        int fdA = open( "/sys/class/gpio/gpio66/value", O_RDONLY | O_NONBLOCK );
        GIOChannel* channelA = g_io_channel_unix_new(fdA);
        GIOCondition condA = GIOCondition(G_IO_PRI);
        guint idA = g_io_add_watch(channelA, condA, EventA, 0);

        int fdB = open( "/sys/class/gpio/gpio69/value", O_RDONLY | O_NONBLOCK );
        GIOChannel* channelB = g_io_channel_unix_new(fdB);
        GIOCondition condB = GIOCondition(G_IO_PRI);
        guint idB = g_io_add_watch(channelB, condB, EventB, 0);

        g_main_loop_run( loopA );
        g_main_loop_run( loopB );

    }

    return (void*) NULL;
}

/*
Purpose: initialize the data for the encoder counter and state
*/
void initCounter(void){

    cout << "Initialization Counter" << endl;
    //Declaration of the initial state of the encoder signal//
    int initA;
    int initB;

    //Setup for A - signal A
    int fd = open( "/sys/class/gpio/gpio66/value", O_RDONLY | O_NONBLOCK );
    GIOChannel* channel = g_io_channel_unix_new(fd);
    GError *error = 0;
    gsize bytes_read = 0;
    const int buf_sz = 1024;
    gchar buf[buf_sz] = {};
    g_io_channel_seek_position( channel, 0, G_SEEK_SET, 0 );
    GIOStatus rc = g_io_channel_read_chars( channel, buf,buf_sz - 1,&bytes_read,&error );

    //check starting value of channel A
    if(buf[0]=='0'){
        initA=0;
    }else if(buf[0]=='1'){
        initA=1;
    }else{
        cout << "problem with the signal A init" << endl;
    }



    //Setup for C - signal B
    fd = open( "/sys/class/gpio/gpio69/value", O_RDONLY | O_NONBLOCK );
    channel = g_io_channel_unix_new(fd);
    bytes_read = 0;
    g_io_channel_seek_position( channel, 0, G_SEEK_SET, 0 );
    rc = g_io_channel_read_chars( channel, buf,buf_sz - 1,&bytes_read,&error );

    //check starting value of channel B
    if(buf[0]=='0'){
        initB=0;
    }else if(buf[0]=='1'){
        initB=1;
    }else{
        cout << "probleme with the signal B init" << endl;
    }


    //Determine starting state based on starting value of channel A and B
    if((initA==0)&&(initB==1)){
        state=1;
        cout << "Initialize counter done, init state: " << state << endl;
    }else if((initA==0)&&(initB==0)){
        state=2;
        cout << "Initialize counter done, init state: " << state << endl;
    }else if((initA==1)&&(initB==0)){
        state=3;
        cout << "Initialize counter done, init state: " << state << endl;
    }else if((initA==1)&&(initB==1)){
        state=4;
        cout << "Initialize counter done, init state: " << state << endl;
    }else{
        cout << "Problem with the counter init" << endl;
    }
}

/*
Purpose: constantly be listening for interrupt coming from channelA
         if interrupted, then call counter function with argument
         indicating channelA was the interrupted channel
*/
static gboolean EventA( GIOChannel *channel, GIOCondition condition, gpointer user_data )
{
    const int channelSig=1;
    GError *error = 0;
    gsize bytes_read = 0;
    const int buf_sz = 1024;
    gchar buf[buf_sz] = {};
    g_io_channel_seek_position( channel, 0, G_SEEK_SET, 0 );
    GIOStatus rc = g_io_channel_read_chars( channel, buf,buf_sz - 1,&bytes_read,&error );
    counter(channelSig);
    return 1;
}

/*
Purpose: constantly be listening for intterupt coming from channelB
         if interruped, then call counter functino with argument
         indicating channelB was the interrupted channel
*/
static gboolean EventB( GIOChannel *channel, GIOCondition condition, gpointer user_data )
{
    const int channelSig=2;
    GError *error = 0;
    gsize bytes_read = 0;
    const int buf_sz = 1024;
    gchar buf[buf_sz] = {};
    g_io_channel_seek_position( channel, 0, G_SEEK_SET, 0 );
    GIOStatus rc = g_io_channel_read_chars( channel, buf,buf_sz - 1,&bytes_read,&error );
    counter(channelSig);
    return 1;
}

/*
Purpose: print out data collected from the encoders including:
         index, interrupt number, calculated angle of encoder, and state
*/
void printOutData(void){
    cout << "Printing of the output starts" << endl;

    int i=0;
    FILE *fj1=fopen("outputEncoder.dat","w");

    fprintf(fj1,"indexOutput;Net Increment;Net Angle (degrees);State;\r\n");

    while(i<MAX_PULSE){
        fprintf(fj1,"%d;%d;%f;%d;%d;%d;\r\n", i+1, outputNetIncrement[i], outputNetAngle[i],outputState[i]);

        if(i==MAX_PULSE-1){
            fclose(fj1);
        }
        i++ ;
    }

    cout << "Printing of the output is done" << endl;

}
