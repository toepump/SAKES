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

int count1 = 0;
int count2 = 0;
int times = 100000;

void *testThread1(void *ptr);
void *testThread2(void *ptr);


	int main(int argc, char* argv[])
        {
			//Creation of the thread
			pthread_t thread1, thread2;
			const char *message1 = "Thread 1";
			const char *message2 = "Thread 2";
			int  iret1, iret2;

			/*set attribute */

			pthread_attr_t attr1, attr2; //Creation of the variable for the attribute
			struct sched_param parm1, parm2; //Creation of new sched_param
			pthread_attr_init(&attr1); //Initialize the thread attributes with default attribute
			pthread_attr_init(&attr2); //Initialize the thread attributes with default attribute


			/* Create independent threads each of which will execute function */

				pthread_attr_getschedparam(&attr1, &parm1); // put the scheduling param of att to parm
				parm1.sched_priority = sched_get_priority_min(SCHED_FIFO); //return the minimum priority
				pthread_attr_setschedpolicy(&attr1, SCHED_FIFO); //set the scheduling policy of attr1 as FIFIO
				pthread_attr_setschedparam(&attr1, &parm1); //set the scheduling parameter of attr1 as parm1

				iret1 = pthread_create(&thread1, &attr1, (void*) testThread1,(void*) message1); //create a thread that launch the print_message_function with the arguments  message1
				pthread_setschedparam(thread1, SCHED_FIFO, &parm1); // sets the scheduling and parameters of thread1 with SCHED_FIFO and parm1
																	// if it fails, return not 0
				//===============================================
				pthread_attr_getschedparam(&attr2, &parm2);
				parm2.sched_priority = sched_get_priority_min(SCHED_FIFO);
				pthread_attr_setschedpolicy(&attr2, SCHED_FIFO);
				pthread_attr_setschedparam(&attr2, &parm2);

				iret2 = pthread_create(&thread2, &attr2, (void*) testThread2, (void*) message2);
				pthread_setschedparam(thread2, SCHED_FIFO, &parm2);

				//set priority each thread
			pthread_setschedprio(thread1, 49);
			pthread_setschedprio(thread2, 49);

			//
			printf("pthread_create() for thread 1 returns: %d\n",iret1);
			printf("pthread_create() for thread 2 returns: %d\n",iret2);

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

	/*Stuff I want to do*/
	/*here should start the things used with the rt preempt patch*/

	return (void*) NULL;
}



void *testThread2(void *ptr) {
	char *message;
	message = (char *) ptr;

	/*Stuff I want to do*/
	/*here should start the things used with the rt preempt patch*/



	return (void*) NULL;
}
