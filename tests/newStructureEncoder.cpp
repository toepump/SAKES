/*
* TestRT_3.cpp
*
*  Created on: May 15, 2017
*      Author: Mikey and Vincent
*/

//prototypes
void *interruptThread(void *ptr);
void *taskThread(void *ptr);
void *probingThread(void *ptr);
void initCounter(void);


//global variables
int state = 0;

/*
Purpose: Entry thread/function for input commands and launching program threads
*/
int main(int argc, char const *argv[]) {
    //TODO: if you want to add any input commands add them here

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

/*
Purpose: thread for spwaning a probing thread every millisecond
         only launches a thread if previously spawned thread has Finished
         Otherwise, program breaks because of control discrepancy
*/
void *taskThread(void *ptr){

}

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
