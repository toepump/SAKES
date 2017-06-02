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

using namespace std;

#define NSEC_PER_SEC  (1000000000) /* The number of nsecs per sec. */

int setParamThreadFIFO(pthread_attr_t attr, struct sched_param param, int priority);
void polyEval(double coeffs[], double x, double output);
void *testThread1(void *ptr);
void *testThread2(void *ptr);

struct encoder{
	int angIinc; //value of the angle in increment
	double angDeg; //value of the angle in degree
	int pulsePerTurn; //number of increment to make one turn
	int numOfChannel; //number of channel used for this encoder
	int numOfEdge; //number of edge detected for this encoder for one channel (either 1 or 2)
	double velDegSec; //the velocity in deg/sec
	double accDegSec; //the acceleration in deg/secˆ2
};

struct motor{
	const double dutyMin; //Value of duty min set in the maxon board
	const double dutyMax; //Value of duty max set in the maxon board
	const double velMotorMin; //Value of the velocity min set on the maxon board in rpm
	const double velMotorax; //Value of the velocity max set on the maxon board in rpm
	double currentVelocity; //Value of the current velocity in rpm
	double currentDuty; //Value of the current duty
	double desiredVelocity; //Value of the desired velocity in rpm
	double desiredDuty; //Value of the desired duty
};

const int TIME_MAX = 100000; // time max for the loop in ms
const int INTERVALMS =1000000; // in nanosecond

int ticks_t1=0; //Incremental value for the thread 1
int ticks_t2=0; //Incremental value for the thread 2
double coeffs1[57]={ 2161704178.57744, -7678966834.50137, 7321336263.45535, 0.0, 0.0, 0.0, -4367145721.75599, 0.0, 0.0, 3978221771.45533, 0.0, 0.0, -985186083.378593, 0.0, 0.0, 0.0, 0.0, -2539164036.74563, 0.0, 0.0, 0.0, 5490325943.05484, 0.0, 0.0, 0.0, -7322471014.88356, 0.0, 0.0, 0.0, 8222278890.26401, 0.0, 0.0, 0.0, -10920714548.7223, 0.0, 0.0,16644524938.8166, 0.0, -16364372664.7029, 0.0,8080579968.97197, 0.0, 0.0, -2834740327.51965, 0.0,  1331213566.8815,  1010961461.3028, -2387418495.97765,1659337298.89251, -622192768.04629, 138921217.362358, -18305060.8984875,1320820.91241054, -50395.6593240169, 1594.07054186194, 38.7663699452653,11.8011809149536};




void polyEval(double coeffs[], double x, double output){

	int degreeOfPoly=sizeof(coeffs); //The degree of the polynum
	int i;
	double result=0;

	for(i=0;i++;i<=degreeOfPoly){
		result+=coeffs[i]*pow(x,degreeOfPoly-i);
	}
	output=result;
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
	pthread_t thread1, thread2;
	const char *message1 = "Thread 1";
	const char *message2 = "Thread 2";
	int iret1, iret2;

	pthread_attr_t attr1, attr2; //Creation of the variable for the attribute
	struct sched_param param1, param2; //Creation of new sched_param

	int checkInitThread;

	pthread_attr_init(&attr1); //Initialize the thread attributes with default attribute
	pthread_attr_init(&attr2); //Initialize the thread attributes with default attribute

	checkInitThread=setParamThreadFIFO(attr1, param1, 49);
	checkInitThread=setParamThreadFIFO(attr2, param2, 49);

	iret1 = pthread_create(&thread1, &attr1, testThread1, (void*) message1);
	iret2 = pthread_create(&thread2, &attr2, testThread2, (void*) message2);

	//create a thread that launch the print_message_function with the arguments message1
	pthread_setschedparam(thread1, SCHED_FIFO, &param1);
	pthread_setschedparam(thread2, SCHED_FIFO, &param2); // sets the scheduling and parameters of thread1 with SCHED_FIFO and parm1
														// if it fails, return not 0
	printf("pthread_create() for returns: %d\n", iret1);
	printf("pthread_create() for returns: %d\n", iret2);

	/* Wait till threads are complete before main continues. Unless we */
	/* wait we run the risk of executing an exit which will terminate  */
	/* the process and all threads before the threads have completed.  */

	pthread_join( thread1, NULL);
	pthread_join( thread2, NULL);

	exit(EXIT_SUCCESS);
}

void *testThread1(void *ptr) {

	char *message;
	message = (char *) ptr;
	struct timespec t_Thread1;

	int testPolyEval=0;
	double timePolyEval=0;
	double angleTest;

	/*Stuff I want to do*/
	/*here should start the things used with the rt preempt patch*/

  clock_gettime(CLOCK_MONOTONIC ,&t_Thread1);
  /* start after one second */
  t_Thread1.tv_sec++;

  while(ticks_t1<TIME_MAX+1) {

  	/* wait until next shot */
  	clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t_Thread1, NULL);

  	/* do the stuff */
  	if(ticks_t1%500==0){
  		polyEval(coeffs1, timePolyEval, angleTest);
  		cout << "Time:  " << timePolyEval << " Angle:  " << angleTest << endl;
  		timePolyEval+=0.05;
  		if(timePolyEval>0.99){
  			timePolyEval=0.0;
  		}
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
  	if(ticks_t2%5000==0){
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
