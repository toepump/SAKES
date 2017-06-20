/*
 * TestRT_3.cpp
 *
 * Created on: May 15, 2017
 *   Author: Vincent
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
#include <math.h>
#include <sched.h>

using namespace std;

#define NSEC_PER_SEC  (1000000000) /* The number of nsecs per sec. */

void timespec_diff(struct timespec *start, struct timespec *stop,struct timespec *result);

int setParamThreadFIFO(pthread_attr_t attr, struct sched_param param, int priority);
int storeIntoOutput(struct output *output, int increment, struct timeStruct *time);
int fileTestMotor(struct output *output1,struct output *output2,struct output *output3);

int setTimeOrigin(struct timeStruct *time);
int getTimeSinceOrigin(struct timeStruct *time);

void *testThread1(void *ptr);
void *testThread2(void *ptr);
void *testThread3(void *ptr);


const int TIME_MAX = 2010; // time max for the loop in ms
const int TIME_MAX_ENC = 5110; // time max for the loop in ms

const int INTERVALMS =1000000; // in nanosecond

const int INTERVAL_T2 = 1000000; //in nanosecond, interval for the thread 2

const int ONESECINNANO = 1000000000; //one second in nanosecond unit
double INTERVAL_S=double(INTERVALMS)/1000000000.0;

int ticks_t1=0; //Incremental value for the thread 1
int ticks_t2=0; //Incremental value for the thread 2
int ticks_t3=0;
int sizeArrayOuput=5000;

struct timeStruct{

	double originSec; //the origin of time in second
	double originNano; //the origin of time, the part in nanosecond
	double originGlobalMilli; //The origin time, with oth part, nano and second, expressed in ms

	double tSec; //the time in sec
	double tNano; //the part in nano sec
	double tMilli; //the global time in ms

};

struct output{
	double timeInMilli[5000];
	int increment[5000];
};

timeStruct timeThread1={.originSec=0.0, .originNano=0.0, .originGlobalMilli=0.0, .tSec=0.0, .tNano=0.0, .tMilli=0.0};
timeStruct timeThread2={.originSec=0.0, .originNano=0.0, .originGlobalMilli=0.0, .tSec=0.0, .tNano=0.0, .tMilli=0.0};
timeStruct timeThread3={.originSec=0.0, .originNano=0.0, .originGlobalMilli=0.0, .tSec=0.0, .tNano=0.0, .tMilli=0.0};
timeStruct difference={.originSec=0.0, .originNano=0.0, .originGlobalMilli=0.0, .tSec=0.0, .tNano=0.0, .tMilli=0.0};

output output1;
output output2;
output output3;

void timespec_diff(struct timespec *start, struct timespec *stop, struct timespec *result){
    if ((stop->tv_nsec - start->tv_nsec) < 0)
    {
        result->tv_sec = stop->tv_sec - start->tv_sec - 1;
        result->tv_nsec = stop->tv_nsec - start->tv_nsec + 1000000000;
    } else {
        result->tv_sec = stop->tv_sec - start->tv_sec;
        result->tv_nsec = stop->tv_nsec - start->tv_nsec;
    }

    return;
}

int storeIntoOutput(struct output *output, int increment, struct timeStruct *time){

	output->increment[increment]=increment;
	output->timeInMilli[increment]=time->tMilli; //the time in milli since the beginning of the experiment

	return 0;
}

int fileTestMotor(struct output *output1,struct output *output2,struct output *output3){

double meanTime[3]={0, 0, 0};
double maxTimeInterval[3]={0,0,0};
double variance[3]={0,0,0};
int i, j;

	cout << "Printing of the output starts" << endl;

		FILE *fj1=fopen("fileTestRT4.dat","w");

		fprintf(fj1,"indexOutput; TimeInMilli1; TimeInMilli2; TimeInMilli3; increment 1; increment 2; increment 3;\r\n");

		while(i<TIME_MAX){
		    fprintf(fj1,"%d;%f;%f;%f;%d;%d;%d; \r\n",i+1, output1->timeInMilli[i], output2->timeInMilli[i],output3->timeInMilli[i], output1->increment[i],output2->increment[i],output3->increment[i]);

		    if(i==TIME_MAX-1){
		    	fclose(fj1);
		    }
		    i++ ;
		}

	cout << "Printing of the output is done" << endl;

	for(i=0;i<TIME_MAX-1;i++){
		if((output1->timeInMilli[i+1]-output1->timeInMilli[i])>maxTimeInterval[0])
		{
			maxTimeInterval[0]=output1->timeInMilli[i+1]-output1->timeInMilli[i];
		}
		meanTime[0]=meanTime[0]+output1->timeInMilli[i+1]-output1->timeInMilli[i];
	}

	meanTime[0]=meanTime[0]/double(TIME_MAX);



	for(i=0;i<TIME_MAX-1;i++){
		variance[0]=variance[0]+fabs(output1->timeInMilli[i+1]-output1->timeInMilli[i]-meanTime[0]);
	}
	variance[0]=variance[0]/double(TIME_MAX);



	printf("T1: Mean Time: %f \n", meanTime[0]);
	printf("T1: Max Time: %f \n", maxTimeInterval[0]);
	printf("T1: Variance: %f \n \n", variance[0]);


	for(i=0;i<TIME_MAX-1;i++){
		if((output2->timeInMilli[i+1]-output2->timeInMilli[i])>maxTimeInterval[1])
		{
			maxTimeInterval[1]=output2->timeInMilli[i+1]-output2->timeInMilli[i];
		}
		meanTime[1]=meanTime[1]+output2->timeInMilli[i+1]-output2->timeInMilli[i];
	}

	meanTime[1]=meanTime[1]/double(TIME_MAX);

	printf("Mean Time: %f \n", meanTime[1]);
	printf("Max Time: %f \n", maxTimeInterval[1]);

	return 0;
}

int setTimeOrigin(struct timeStruct *time){

	struct timespec timeFetcher;
	clock_gettime(CLOCK_MONOTONIC ,&timeFetcher);

	time->originSec=double(timeFetcher.tv_sec);
	time->originNano=double(timeFetcher.tv_nsec);
	time->originGlobalMilli=time->originSec*1000.0+time->originNano/1000000.0;

	time->tSec=0.0;
	time->tNano=0.0;
	time->tMilli=0.0;

	return 0;
}

int getTimeSinceOrigin(struct timeStruct *time){

	struct timespec timeFetcher;
	clock_gettime(CLOCK_MONOTONIC ,&timeFetcher);

	time->tSec=double(timeFetcher.tv_sec)-time->originSec;
	time->tNano=double(timeFetcher.tv_nsec)-time->originNano;
	time->tMilli=time->tSec*1000.0+time->tNano/1000000.0;

	return 0;
}

int setParamThreadFIFO(pthread_attr_t attr, struct sched_param param, int priority){
	//Function: Set the attr and parm as a FIFO function with priority


	int checkParam; //Variable to check if the setting of the thread is okay

	//Create independent threads each of which will execute function
	pthread_attr_getschedparam(&attr, &param); // put the scheduling param of att to parm
	checkParam=param.sched_priority = priority; //return the minimum priority
	checkParam=pthread_attr_setschedpolicy(&attr, SCHED_FIFO); //set the scheduling policy of attr1 as FIFIO
	checkParam=pthread_attr_setschedparam(&attr, &param); //set the scheduling parameter of attr1 as parm1

	if(checkParam!=0){
		cout << "Problem in the initialization of a thread "<< endl;
		checkParam=0;
	}

	return checkParam;
}

int main(int argc, char* argv[]){

	//Creation of the thread
	pthread_t thread1, thread2, thread3;
	const char *message1 = "Thread 1";
	const char *message2 = "Thread 2";
	const char *message3 = "Thread 3";
	int iret1, iret2, iret3;

	pthread_attr_t attr1, attr2, attr3; //Creation of the variable for the attribute
	struct sched_param param1, param2, param3, parm; //Creation of new sched_param

	int checkInitThread;

	int checkAttrInit;
	int checkschedParam;
	int setInherited;
	int setPolicy;

	int min, max;

	min = sched_get_priority_min( SCHED_FIFO );
	max = sched_get_priority_max( SCHED_FIFO );

	/* Diplay max and min FIFO priority
	printf("Min priority FIFO: %d \n", min);
	printf("Max priority FIFO: %d \n", max);
	 */
	checkAttrInit=pthread_attr_init(&attr1); //Initialize the thread attributes with default attribute
	if(checkAttrInit!=0){
		printf("Problem attribute 1: %d \n", checkAttrInit);
	}

	checkAttrInit=pthread_attr_init(&attr2); //Initialize the thread attributes with default attribute
	if(checkAttrInit!=0){
		printf("Problem attribute 2: %d \n", checkAttrInit);
	}

	checkAttrInit=pthread_attr_init(&attr3); //Initialize the thread attributes with default attribute
	if(checkAttrInit!=0){
		printf("Problem attribute 3: %d \n", checkAttrInit);
	}



	setInherited=pthread_attr_setinheritsched(&attr1, PTHREAD_EXPLICIT_SCHED);
	if(setInherited!=0){
		printf("Problem set inherited 1: %d \n", setInherited);
	}

	setInherited=pthread_attr_setinheritsched(&attr2, PTHREAD_EXPLICIT_SCHED);
	if(setInherited!=0){
		printf("Problem set inherited 2: %d \n", setInherited);
	}

	setInherited=pthread_attr_setinheritsched(&attr3, PTHREAD_EXPLICIT_SCHED);
	if(setInherited!=0){
		printf("Problem set inherited 3: %d \n", setInherited);
	}


	setPolicy=pthread_attr_setschedpolicy(&attr1, SCHED_FIFO);
	if(setPolicy!=0){
		printf("Problem set policy 1: %d \n", setPolicy);
	}

	setPolicy=pthread_attr_setschedpolicy(&attr2, SCHED_FIFO);
	if(setPolicy!=0){
		printf("Problem set policy 2: %d \n", setPolicy);
	}

	setPolicy=pthread_attr_setschedpolicy(&attr3, SCHED_FIFO);
	if(setPolicy!=0){
		printf("Problem set policy 3: %d \n", setPolicy);
	}

	param1.sched_priority = 40;
	param2.sched_priority = 70;
	param3.sched_priority = 50;

	checkschedParam=pthread_attr_setschedparam(&attr1, &param1);
	if(checkschedParam!=0){
		printf("Problem set param 1: %d \n", checkschedParam);
	}

	checkschedParam=pthread_attr_setschedparam(&attr2, &param2);
	if(checkschedParam!=0){
		printf("Problem set param 1: %d \n", checkschedParam);
	}

	checkschedParam=pthread_attr_setschedparam(&attr3, &param3);
	if(checkschedParam!=0){
		printf("Problem set param 1: %d \n", checkschedParam);
	}


	iret1 = pthread_create(&thread1, &attr1, testThread1, (void*) message1);
	//iret2 = pthread_create(&thread2, &attr2, testThread2, (void*) message2);
	//iret3 = pthread_create(&thread3, &attr3, testThread3, (void*) message3);


	// if it fails, return not 0
	printf("pthread_create() 1 for returns: %d\n", iret1);
	//printf("pthread_create() 2 for returns: %d\n", iret2);
	//printf("pthread_create() 3 for returns: %d\n", iret3);

	pthread_join( thread1, NULL);
	//pthread_join( thread2, NULL);
	//pthread_join( thread3, NULL);


	fileTestMotor(&output1, &output2, &output3);

	exit(EXIT_SUCCESS);
}


void *testThread1(void *ptr) {

	char *message;
	message = (char *) ptr;
	struct timespec waitTime;
	struct timespec start;
	struct timespec previous_start;
	struct timespec end;
	struct timespec diff;
	struct timespec remain;

	int testValue=0;
	int i;

	int sleepOK=0;

	waitTime.tv_sec=1;
	waitTime.tv_nsec=0;

	setTimeOrigin(&timeThread1);

	while(ticks_t1<TIME_MAX+1){

		/* wait until next shot */


		if(sleepOK == 0){

			clock_gettime(CLOCK_MONOTONIC, &start);

			timespec_diff(&previous_start, &start, &diff);


			difference=double(diff.tv_sec)*1000.0+double(diff.tv_nsec)/1000000.0;
			getTimeSinceOrigin(&timeThread1);

			if(ticks_t1>0)
			{
				storeIntoOutput(&output1, ticks_t1, &difference);
			}


	  		testValue=0;

	  		for(i=0;i<30000;i++){
	  			testValue++;
	  		}

	  		ticks_t1++; // Increment the ticks value


	  		previous_start.tv_sec=start.tv_sec;
	  		previous_start.tv_nsec=start.tv_nsec;


	  		/*

	  		clock_gettime(CLOCK_MONOTONIC, &end);
	  		if(diff.tv_sec==0 && diff.tv_nsec < 1000000){
	  			waitTime.tv_sec=0;
	  			waitTime.tv_nsec=800000-diff.tv_nsec;
	  			*/

	  		waitTime.tv_nsec+=1000000;
	  		if(waitTime.tv_nsec> 999999999){
	  			waitTime.tv_sec+=1;
	  			waitTime.tv_nsec-=1000000000;
	  		}else{
	  			//cout << "The thread is not done in 1 ms" << endl;
	  			//cout << ticks_t1 << endl;
	  		}

	  		sleepOK = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &waitTime, &remain);
		}else{

			cout << "The thread is not done in 1 ms" << endl;

		}

  }

	return (void*) NULL;
}

void *testThread2(void *ptr) {

	char *message;
	message = (char *) ptr;
	struct timespec waitTime;
	struct timespec start;
	struct timespec end;
	struct timespec diff;
	struct timespec remain;

	int testValue=0;
	int i;

	int sleepOK=0;

	waitTime.tv_sec=1;
	waitTime.tv_nsec=0;

	setTimeOrigin(&timeThread2);

	while(ticks_t2<TIME_MAX+1){

		/* wait until next shot */
		sleepOK = clock_nanosleep(CLOCK_MONOTONIC, 0, &waitTime, &remain);

		if(sleepOK == 0){

			clock_gettime(CLOCK_MONOTONIC, &start);

			getTimeSinceOrigin(&timeThread2);

	  		storeIntoOutput(&output2, ticks_t2, &timeThread2);

	  		testValue=0;

	  		for(i=0;i<30000;i++){
	  			testValue++;
	  		}

	  		ticks_t2++; // Increment the ticks value

	  		clock_gettime(CLOCK_MONOTONIC, &end);

	  		timespec_diff(&start, &end, &diff);

	  		if(diff.tv_sec==0 && diff.tv_nsec < 1000000){
	  			waitTime.tv_sec=0;
	  			waitTime.tv_nsec=900000-diff.tv_nsec;
	  		}else{
	  			//cout << "The thread 2 is not done in 1 ms" << endl;
	  			//cout << ticks_t2 << endl;
	  		}
		}else{

			//cout << "The thread 2 is not done in 1 ms" << endl;

		}

  }

	return (void*) NULL;
}

void *testThread3(void *ptr) {

	char *message;
	message = (char *) ptr;
	struct timespec waitTime;
	struct timespec start;
	struct timespec end;
	struct timespec diff;
	struct timespec remain;

	int testValue=0;
	int i;

	int sleepOK=0;

	waitTime.tv_sec=1;
	waitTime.tv_nsec=0;

	setTimeOrigin(&timeThread3);

	while(ticks_t3<TIME_MAX+1){

		/* wait until next shot */
		sleepOK = clock_nanosleep(CLOCK_MONOTONIC, 0, &waitTime, &remain);

		if(sleepOK == 0){

			clock_gettime(CLOCK_MONOTONIC, &start);

			getTimeSinceOrigin(&timeThread3);

	  		storeIntoOutput(&output3, ticks_t3, &timeThread3);

	  		testValue=0;

	  		for(i=0;i<30000;i++){
	  			testValue++;
	  		}

	  		ticks_t3++; // Increment the ticks value

	  		clock_gettime(CLOCK_MONOTONIC, &end);

	  		timespec_diff(&start, &end, &diff);

	  		if(diff.tv_sec==0 && diff.tv_nsec < 1000000){
	  			waitTime.tv_sec=0;
	  			waitTime.tv_nsec=900000-diff.tv_nsec;
	  		}else{
	  			//cout << "The thread 3 is not done in 1 ms" << endl;
	  			//cout << ticks_t3 << endl;
	  		}
		}else{

			//cout << "The thread 3 is not done in 1 ms" << endl;

		}

  }

	return (void*) NULL;
}
