#include "common.h"
#include "taskThread.h"
#include "probingThread.h"

using namespace std;

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

        pthread_t theProbingThread;
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
        iret1 = pthread_create(&theProbingThread, &attr, probingThread,(void*) message1);      //create a thread that launch the print_message_function with the arguments  message1
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

            indexProbe++;
            //if index > than PROBE_STORAGE_SIZE or indexOutput > MAX_PULSE
            if(indexProbe-1 > PROBE_STORAGE_SIZE || indexOutput >= MAX_PULSE-1){
                //printProbe
                printProbe();
            }
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

void printProbe(void){
    cout << "Printing of the probe starts" << endl;

    int i=0;
    FILE *fj2=fopen("probeCheck.data","w");

    fprintf(fj2, "Time (ms); Net Angle (degree); net Increment;\n");

    while(i<PROBE_STORAGE_SIZE){
        fprintf(fj2,  "%d;%f;%d;\r\n", i, probeAngleDeg[i], probeIncrement[i]);
        i++ ;
    }

    cout << "Printing of the output is done" << endl;

}
