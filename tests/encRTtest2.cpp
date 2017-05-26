/*
 * TestRT_3.cpp
 *
 *  Created on: May 15, 2017
 *      Author: Vincent
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

using namespace std;

#define NSEC_PER_SEC    (1000000000) /* The number of nsecs per sec. */
#define NSEC_PER_MSEC   (1000000)   //number of nsecs in milliseconds
const int INTERVAL =1000000; // in nanosecond

const int MAX_PULSE = 50000; //maximum number of pulse recorded
const int PULSE_PER_TURN = 12000; //The number of pulse (interrupt) to complete one turn
const int PROBE_STORAGE_SIZE = 30000; //in ms
const double PULSE_PER_DEGREE = double(PULSE_PER_TURN)/360.0; // The number of pulse (interrupt) to complete one degree
int outputNetIncrement[MAX_PULSE]; //Store the value at each interrupt

double outputNetAngle[MAX_PULSE]; //Store the value at each interrupt
double probeAngleDeg[PROBE_STORAGE_SIZE]; //net angle check for the probing thread
double probeIncrement[PROBE_STORAGE_SIZE]; //net angle check for the probing thread

int outputEncfwd[MAX_PULSE]; //Store the value at each interrupt
int outputEncbwd[MAX_PULSE]; //Store the value at each interrupt
int outputState[MAX_PULSE];
int indexOutput=0; //Incremented at each interrupt


void *testThread1(void *ptr);
void *testThread2(void *ptr);


int netAngleIncrement=0; //the angle in encoder increment unit
int encfwd=0; //number of pulse done in the forward direction
int encbwd=0; //number of pulse done in the backward direction
double netAngleDegree=0; //the net angle in degree
int state=0; //the state of the encoder (1, 2, 3 or 4)
static int init=0;

//prototypes
static gboolean EventA( GIOChannel *channel, GIOCondition condition, gpointer user_data );
static gboolean EventB( GIOChannel *channel, GIOCondition condition, gpointer user_data );
void counter(int nb_signal);
void printOutData(void);
void printProbe(void);

static gboolean EventA( GIOChannel *channel, GIOCondition condition, gpointer user_data )
{
    const int nb_signal=1;
    GError *error = 0;
    gsize bytes_read = 0;
    const int buf_sz = 1024;
    gchar buf[buf_sz] = {};
    g_io_channel_seek_position( channel, 0, G_SEEK_SET, 0 );
    GIOStatus rc = g_io_channel_read_chars( channel, buf,buf_sz - 1,&bytes_read,&error );
    counter(nb_signal);
    return 1;
}

static gboolean EventB( GIOChannel *channel, GIOCondition condition, gpointer user_data )
{
    const int nb_signal=2;
    GError *error = 0;
    gsize bytes_read = 0;
    const int buf_sz = 1024;
    gchar buf[buf_sz] = {};
    g_io_channel_seek_position( channel, 0, G_SEEK_SET, 0 );
    GIOStatus rc = g_io_channel_read_chars( channel, buf,buf_sz - 1,&bytes_read,&error );
    counter(nb_signal);
    return 1;
}

//test counter function to determine which direction encoder is and how much
void counter(int nb_signal) {
    init++;

    if(init>2 && indexOutput<MAX_PULSE){

        if(state==1){
            if(nb_signal==2){
            	netAngleIncrement++;
                //encfwd++;
                state=2;
            }else if(nb_signal==1){
                //encbwd++;
                netAngleIncrement--;
                state=4;
            }else{
                cout << "problem with the counter in case 1" << endl;
            }
        }

        else if(state==2){
            if(nb_signal==1){
            	netAngleIncrement++;
            	//encfwd++;
                state=3;
            }else if(nb_signal==2){
            	state=1;
            	netAngleIncrement--;
            	//encbwd++;
            }else{
                cout << "problem with the counter in case 2" << endl;
            }
        }

        else if(state==3){
            if(nb_signal==2){
            	netAngleIncrement++;
               // encfwd++;
                state=4;
            }else if(nb_signal==1){
            	netAngleIncrement--;
				//encbwd++;
				state=2;
            }else{
                cout << "problem with the counter in case 3" << endl;
            }
        }

        else if(state==4){
            if(nb_signal==1){
                //encfwd++;
                netAngleIncrement++;
                state=1;
            }else if(nb_signal==2){
            	//encbwd++;
            	netAngleIncrement--;
                state=3;
            }else{
                cout << "problem with the counter in case 4" << endl;
            }

        }
        netAngleDegree=double(netAngleIncrement)/PULSE_PER_DEGREE;

        	/*
        outputNetIncrement[indexOutput]=netAngleIncrement;
        outputNetAngle[indexOutput]=netAngleDegree;
        outputEncfwd[indexOutput]=encfwd;
        outputEncbwd[indexOutput]=encbwd;
        outputState[state];
        indexOutput++;
        */
        }

	if(indexOutput%10000==0){
		cout << "10000" << endl;
	}

    	if(indexOutput+1>MAX_PULSE){
    		printOutData();
    	}

}

void printOutData(void){
	cout << "Printing of the output starts" << endl;

	int i=0;
	FILE *fj1=fopen("outputEncoder.dat","w");

	fprintf(fj1,"indexOutput;Net Increment;Net Angle (degrees);State;EncoderForward;EncoderBackward;\r\n");

	while(i<MAX_PULSE){
	    fprintf(fj1,"%d;%d;%d;%d;%d;%d;\r\n", i+1, outputNetIncrement[i], outputNetAngle[i],outputState[i], outputEncfwd[i], outputEncbwd[i]);

	    if(i==MAX_PULSE-1){
	    	fclose(fj1);
	    }
	    i++ ;
	}

	cout << "Printing of the output is done" << endl;

}

void printProbe(void){
	cout << "Printing of the probe starts" << endl;

	int i=0;
	FILE *fj2=fopen("probeCheck.dat","w");

	fprintf(fj2, "Time (ms); Net Angle (degree); net Increment;\n");

	while(i<PROBE_STORAGE_SIZE){
	    fprintf(fj2,  "%d;%f;%d; \r\n", i, probeAngleDeg[i],probeIncrement[i]);
	    i++ ;
	}

	cout << "Printing of the output is done" << endl;

}

void initCounter(void){

    cout << "Initialization Counter" << endl;
    //Declaration of the initial state of the encoder signal//
    int initA;
    int initB;

    //Operation for A - signal A
    int fd = open( "/sys/class/gpio/gpio66/value", O_RDONLY | O_NONBLOCK );
    GIOChannel* channel = g_io_channel_unix_new(fd);
    GError *error = 0;
    gsize bytes_read = 0;
    const int buf_sz = 1024;
    gchar buf[buf_sz] = {};
    g_io_channel_seek_position( channel, 0, G_SEEK_SET, 0 );
    GIOStatus rc = g_io_channel_read_chars( channel, buf,buf_sz - 1,&bytes_read,&error );

    if(buf[0]=='0'){
        initA=0;
    }else if(buf[0]=='1'){
        initA=1;
    }else{
        cout << "probleme with the signal A init" << endl;
    }

    //Operation for C - signal B
    fd = open( "/sys/class/gpio/gpio69/value", O_RDONLY | O_NONBLOCK );
    channel = g_io_channel_unix_new(fd);
    bytes_read = 0;
    g_io_channel_seek_position( channel, 0, G_SEEK_SET, 0 );
    rc = g_io_channel_read_chars( channel, buf,buf_sz - 1,&bytes_read,&error );

    if(buf[0]=='0'){
        initB=0;
    }else if(buf[0]=='1'){
        initB=1;
    }else{
        cout << "probleme with the signal B init" << endl;
    }


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

int main(int argc, char* argv[]){

	//Initialization of the counter
	initCounter();

	//Creation of the thread
	pthread_t thread1;
    pthread_t thread2;
	const char *message1 = "Thread 1";
    const char *message2 = "Thread 2";

	int  iret1;
	int  iret2;
	/*set attribute */

	pthread_attr_t attr1, attr2; //Creation of the variable for the attribute
	struct sched_param parm1, parm2; //Creation of new sched_param
	pthread_attr_init(&attr1); //Initialize the thread attributes with default attribute
	pthread_attr_init(&attr2);

	/* Create independent thread which will execute function */

	pthread_attr_getschedparam(&attr1, &parm1); // put the scheduling param of att to parm
	parm1.sched_priority = sched_get_priority_min(SCHED_FIFO); //return the minimum priority
	pthread_attr_setschedpolicy(&attr1, SCHED_FIFO); //set the scheduling policy of attr1 as FIFIO
	pthread_attr_setschedparam(&attr1, &parm1); //set the scheduling parameter of attr1 as parm1

	iret1 = pthread_create(&thread1, &attr1, testThread1,(void*) message1); //create a thread that launch the print_message_function with the arguments  message1
    pthread_setschedparam(thread1, SCHED_FIFO, &parm1); // sets the scheduling and parameters of thread1 with SCHED_FIFO and parm1
														// if it fails, return not 0

    //===========================================================
    pthread_attr_getschedparam(&attr2, &parm2);
    parm2.sched_priority = sched_get_priority_min(SCHED_FIFO);
    pthread_attr_setschedpolicy(&attr2, SCHED_FIFO);
    pthread_attr_setschedparam(&attr2, &parm2);

    iret1 = pthread_create(&thread1, &attr1, testThread2,(void*) message2);
    pthread_setschedparam(thread2, SCHED_FIFO, &parm2);



	//set priority each thread
	pthread_setschedprio(thread1, 49);
    pthread_setschedprio(thread2, 49);
	//
	printf("pthread_create() for thread 1 returns: %d\n",iret1);
    printf("pthread_create() for thread 2 returns: %d\n",iret2); //TODO some error number returns

	/* Wait till threads are complete before main continues. Unless we  */
	/* wait we run the risk of executing an exit which will terminate   */
	/* the process and all threads before the threads have completed.   */

	pthread_join( thread1, NULL);
    pthread_join( thread2, NULL);


	exit(EXIT_SUCCESS);
}

void *testThread1(void *ptr) {

	char *message;
	message = (char *) ptr;

    while(true) {

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

void *testThread2(void *ptr){
    char *message;
    message = (char *) ptr;
    struct timespec t_Thread2;  //struct for keeping time (not actual monotonic clock)
    int index = 0;              //index for the probe getting and storing encoder info

    /*Stuff I want to do*/
    /*here should start the things used with the rt preempt patch*/

    clock_gettime(CLOCK_MONOTONIC ,&t_Thread2);     //get the current time and store in the timespec struct
                                                   /* start after one second */
    t_Thread2.tv_sec++;                             //increment the timespec struct time by one full second so that
                                                    //we can get a delay in the next step

    while(true) {

        /* wait until next shot */
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t_Thread2, NULL);  //delay until time stored in the timespec

        /* do the stuff */
        //probe the encoder values every millisecond while the interrupts are happening
    	//printf("%f\n",netAngleDegree);

        probeAngleDeg[index] = netAngleDegree; //store current netAngleDegree
        probeIncrement[index] = netAngleIncrement;
        index++;                            //increment index

        if(index > PROBE_STORAGE_SIZE){                   //if index is past storage limit, print
        	printProbe();
            return (void*) NULL;
        }


		/* calculate next shot */
    	t_Thread2.tv_nsec += INTERVAL;      //wait another millisecond so that delay will happen again before next probe

    	while (t_Thread2.tv_nsec >= NSEC_PER_SEC) {
    		t_Thread2.tv_nsec -= NSEC_PER_SEC;
    		t_Thread2.tv_sec++;
    	}
    }

    return (void*) NULL;
}
