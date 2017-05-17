/*
 * TestRT_1.cpp
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

#define MY_PRIORITY (49) /* we use 49 as the PRREMPT_RT use 50
                            as the priority of kernel tasklets
                            and interrupt handler by default */

#define MAX_SAFE_STACK (8*1024) /* The maximum stack size which is
                                   guaranteed safe to access without
                                   faulting */

#define NSEC_PER_SEC    (1000000000) /* The number of nsecs per sec. */

void stack_prefault(void) {

        unsigned char dummy[MAX_SAFE_STACK];

        memset(dummy, 0, MAX_SAFE_STACK);
        return;
        void *print_message_function( void *ptr );

}

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

				iret1 = pthread_create(&thread1, &attr1, (void*) print_message_function,(void*) message1);
				pthread_setschedparam(thread1, SCHED_FIFO, &parm1);

				//===============================================
				pthread_attr_getschedparam(&attr2, &parm2);
				parm2.sched_priority = sched_get_priority_min(SCHED_FIFO);
				pthread_attr_setschedpolicy(&attr2, SCHED_FIFO);
				pthread_attr_setschedparam(&attr2, &parm2);

				iret2 = pthread_create(&thread2, &attr2, (void*) print_message_function,
						(void*) message2);
				pthread_setschedparam(thread2, SCHED_FIFO, &parm2);


		/*
	Function: pthread_create
	Arguments:
    	thread - returns the thread id. (unsigned long int defined in bits/pthreadtypes.h)
    	attr - Set to NULL if default thread attributes are used. (else define members of the struct pthread_attr_t defined in bits/pthreadtypes.h) Attributes include:
        	detached state (joinable? Default: PTHREAD_CREATE_JOINABLE. Other option: PTHREAD_CREATE_DETACHED)
        	scheduling policy (real-time? PTHREAD_INHERIT_SCHED,PTHREAD_EXPLICIT_SCHED,SCHED_OTHER)
        	scheduling parameter
        	inheritsched attribute (Default: PTHREAD_EXPLICIT_SCHED Inherit from parent thread: PTHREAD_INHERIT_SCHED)
        	scope (Kernel threads: PTHREAD_SCOPE_SYSTEM User threads: PTHREAD_SCOPE_PROCESS Pick one or the other not both.)
        	guard size
        	stack address (See unistd.h and bits/posix_opt.h _POSIX_THREAD_ATTR_STACKADDR)
        	stack size (default minimum PTHREAD_STACK_SIZE set in pthread.h),
		void * (*start_routine) - pointer to the function to be threaded. Function has a single argument: pointer to void.
		*arg - pointer to argument of function. To pass multiple arguments, send a pointer to a structure.

			*/

			/* Create independent threads each of which will execute function */

				//set priority each thread
			pthread_setschedprio(thread1, 49);
			pthread_setschedprio(thread2, 49);

			printf("pthread_create() for thread 1 returns: %d\n",iret1);
			printf("pthread_create() for thread 2 returns: %d\n",iret2);

             /* Wait till threads are complete before main continues. Unless we  */
             /* wait we run the risk of executing an exit which will terminate   */
             /* the process and all threads before the threads have completed.   */

             pthread_join( thread1, NULL);
             pthread_join( thread2, NULL);

             exit(EXIT_SUCCESS);
        }

        void print_message_function( void *ptr )
        {
             char *message;
             message = (char *) ptr;
             printf("%s \n", message);
        }
