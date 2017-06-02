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

using namespace std;

#define NSEC_PER_SEC    (1000000000) /* The number of nsecs per sec. */

const int TIME_MAX = 60000; // time max for the loop in ms
const int INTERVALMS =1000000; // in nanosecond


int setParamThread(pthread_attr_t attr, struct sched_param parm, int priority);
void *testThread1(void *ptr);
void *testThread2(void *ptr);

int ticks_t1=0; //Incremental value for the thread 1
int ticks_t2=0; //Incremental value for the thread 2





int setParamThread(pthread_attr_t attr, struct sched_param parm, int priority){
	/*Function: Set the parameters
	 * Create the thread @thread, with the priority @priority
	 * associated with the iret @iret, the function @function and the message @*message
	 */

	int checkParam; //Variable to check if the setting of the thread is okay

	pthread_attr_init(&attr); //Initialize the thread attributes with default attribute

	/* Create independent threads each of which will execute function */
	pthread_attr_getschedparam(&attr, &parm); // put the scheduling param of att to parm
	checkParam=parm.sched_priority = priority; //return the minimum priority
	checkParam=pthread_attr_setschedpolicy(&attr, SCHED_FIFO); //set the scheduling policy of attr1 as FIFIO
	checkParam=pthread_attr_setschedparam(&attr, &parm); //set the scheduling parameter of attr1 as parm1

	if(checkParam!=0){
		cout << "Problem in the initialization of a thread "<< endl;
		checkParam=0;
	}

	return checkParam;
}

int main(int argc, char* argv[]){

	//Creation of the thread
	pthread_t thread1, thread2;
	const char *message1 = "Thread 1";
	const char *message2 = "Thread 2";
	int  iret1, iret2;

	pthread_attr_t attr1, attr2; //Creation of the variable for the attribute
	struct sched_param parm1, parm2; //Creation of new sched_param

	int checkInitThread;

	pthread_attr_init(&attr1); //Initialize the thread attributes with default attribute

	//checkInitThread=setParamThread(attr1, parm1, 49);
	checkInitThread=setParamThread(attr2, parm2, 49);

	pthread_attr_getschedparam(&attr1, &parm1); // put the scheduling param of att to parm
	checkInitThread=parm1.sched_priority = 40; //return the minimum priority
	checkInitThread=pthread_attr_setschedpolicy(&attr1, SCHED_FIFO); //set the scheduling policy of attr1 as FIFIO
	checkInitThread=pthread_attr_setschedparam(&attr1, &parm1); //set the scheduling parameter of attr1 as parm1
	if(checkInitThread!=0){
		cout << "Problem in the initialization of the thread 1 : "<< endl;
		checkInitThread=0;
	}


	cout << "Ici 0025 " << endl;


	iret1 = pthread_create(&thread1, &attr1, testThread1, (void*) message1);
	cout << "Ici 003 " << endl;
	iret2 = pthread_create(&thread2, &attr2, testThread2, (void*) message2);

	cout << "Ici 004 " << endl;

	//create a thread that launch the print_message_function with the arguments  message1
	pthread_setschedparam(thread1, SCHED_FIFO, &parm1);
	pthread_setschedparam(thread2, SCHED_FIFO, &parm2); // sets the scheduling and parameters of thread1 with SCHED_FIFO and parm1
														// if it fails, return not 0


	cout << "Ici 1 " << endl;

	printf("pthread_create() for returns: %d\n", iret1);
	printf("pthread_create() for returns: %d\n", iret2);

	/* Wait till threads are complete before main continues. Unless we  */
	/* wait we run the risk of executing an exit which will terminate   */
	/* the process and all threads before the threads have completed.   */

	cout << "Ici 2 " << endl;

	pthread_join( thread1, NULL);
	pthread_join( thread2, NULL);

	cout << "Ici 3 " << endl;

	exit(EXIT_SUCCESS);
}

void *testThread1(void *ptr) {

	char *message;
	message = (char *) ptr;
	struct timespec t_Thread1;


	/*Stuff I want to do*/
	/*here should start the things used with the rt preempt patch*/

    clock_gettime(CLOCK_MONOTONIC ,&t_Thread1);
    /* start after one second */
    t_Thread1.tv_sec++;

    while(ticks_t1<TIME_MAX+1) {

    	/* wait until next shot */
    	clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t_Thread1, NULL);

    	/* do the stuff */


    	if(ticks_t1%5000==0){
    		cout << "Counter of ticks for the thread 1: " << ticks_t1 << endl;
    	}

    	ticks_t1++; // Increment the ticks value

		/* calculate next shot */
    	t_Thread1.tv_nsec += INTERVALMS;

    	while (t_Thread1.tv_nsec >= NSEC_PER_SEC) {
    		t_Thread1.tv_nsec -= NSEC_PER_SEC;
    		t_Thread1.tv_sec++;
    	}
    }

	return (void*) NULL;
}



void *testThread2(void *ptr) {

	char *message;
	message = (char *) ptr;
	struct timespec t_Thread2;

	/*Stuff I want to do*/
	/*here should start the things used with the rt preempt patch*/

    clock_gettime(CLOCK_MONOTONIC ,&t_Thread2);
    /* start after one second */
    t_Thread2.tv_sec++;

    while(ticks_t2<TIME_MAX+1) {

    	/* wait until next shot */
    	clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t_Thread2, NULL);

    	/* do the stuff */
    	if(ticks_t2%1000==0){
    		cout << "Add 2, thread 2: " << ticks_t2 << endl;
    	}

    	ticks_t2++; // Increment the ticks value

		/* calculate next shot */
    	t_Thread2.tv_nsec += INTERVALMS;

    	while (t_Thread2.tv_nsec >= NSEC_PER_SEC) {
    		t_Thread2.tv_nsec -= NSEC_PER_SEC;
    		t_Thread2.tv_sec++;
    	}
    }

	return (void*) NULL;
}
