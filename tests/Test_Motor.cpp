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
void polyEval(double coeffs[], double *time, double *angle);
void polyAngToIncAng(double *polyAng, struct encoder *encoder);
void *testThread1(void *ptr);
void *testThread2(void *ptr);

struct encoder{
	int angInc; //value of the angle in increment
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
const int MAXDEGREEPOLY=57; //57 for coeffs1,  112 for coeffs2 or 3

double coeffs1[MAXDEGREEPOLY]={ 2161704178.57744, -7678966834.50137, 7321336263.45535, 0.0, 0.0, 0.0, -4367145721.75599, 0.0, 0.0, 3978221771.45533, 0.0, 0.0, -985186083.37859, 0.0, 0.0, 0.0, 0.0, -2539164036.74563, 0.0, 0.0, 0.0, 5490325943.05484, 0.0, 0.0, 0.0, -7322471014.88356, 0.0, 0.0, 0.0, 8222278890.26401, 0.0, 0.0, 0.0, -10920714548.72234, 0.0, 0.0,16644524938.81657, 0.0, -16364372664.70290, 0.0,8080579968.97197, 0.0, 0.0, -2834740327.51965, 0.0,  1331213566.88150,  1010961461.30280, -2387418495.97765,1659337298.89251, -622192768.04629, 138921217.362358, -18305060.8984875,1320820.91241054, -50395.6593240169, 1594.07054186194, 38.7663699452653,11.8011809149536};
//double coeffs2[MAXDEGREEPOLY]={-7804126.68756267, 0, 25506481.892212, 0, 0, -31995497.6264176, 0, 0, 0, 0, 0, 0, 0, 60180709.9392192, 0, 0, 0, 0, -91290146.7205068, 0, 0, 0, 0, 0, 62966686.2936572, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -41489120.4273827, 0, 0, 0, 0, 0, 0, 0, 0, 0, 51569710.2406328, 0, 0, 0, 0, 0, 0, 0, 0, 0 , -61482650.4011263, 0, 0, 0, 0, 0, 0, 0, 0, 0, 119151566.929812, 0, 0, 0, 0, -158836801.811755, 0, 0, 0, 0, 107845792.501427, 0, 0, 0, 0, -40438704.5594945, 0, 0, 0, 0, -2017204.55659056, 0, 0, 0, 22368787.3291233, 0, 0, 0, -38587201.2961324,0, 0, 68338463.6321268, 0, -91431570.8652916, 0, 136730933.950438, -146381938.709179, 76449935.5389264, -22951812.7425929, 3901914.86045682, -302425.558782687, -2995.35800355861, 1143.64231358147, 70.3494501822929, 11.3679676751559};
//double coeffs3[MAXDEGREEPOLY]={-5568914.69050683, 0, 18550269.9593775, 0, 0, -23986691.8154377, 0, 0, 0, 0, 0, 0, 0, 49531094.6617748, 0, 0, 0, 0, -80515111.3284084, 0, 0, 0, 0, 0, 61150107.658354, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -52944834.5893432, 0, 0, 0, 0, 0, 0, 0, 0, 0, 88145386.7882361, 0, 0, 0, 0, 0, 0, 0, 0, 0, -156446891.058224, 0, 0, 0, 0, 0, 0, 0, 0, 0, 533593350.532314, 0, 0, 0, 0, -1048894258.17445, 0, 0, 0, 0, 1199016711.57069, 0, 0, 0, 0, -1033184989.92329, 0, 0, 0, 0, 862028565.726653, 0, 0, 0, -705213545.729899, 0, 0, 0, 614832548.422423, 0, 0, -746208588.092968, 0, 791392111.350242, 0, -947590096.72696, 914359301.492975, -436255303.466556, 123792199.437433, -21816443.1312183, 2387498.2633569, -158624.061403544, 4988.62315434288, 158.259318872659, 11.4720337508105};

//Specification of the knee encoder E30S4-3000-6-L-5 from Autonics, with channel A and B activated
struct encoder kneeTwoAuto={.angInc=0, .angDeg=0.0, .pulsePerTurn=12000, .numOfChannel=2, .numOfEdge=2, .velDegSec=0.0, .accDegSec=0.0};

void polyEval(double coeffs[], double *time, double *angle){

	int i=0;
	double result=0.0;

	for(i=0;i<MAXDEGREEPOLY-1;i++){
		result+=coeffs[i]*pow(*time,MAXDEGREEPOLY-1-i);
	}
	result+=coeffs[MAXDEGREEPOLY-1];

	*angle=result;
}

void polyAngToIncAng(double *polyAng, struct encoder *polyEnc){
	polyEnc->angInc=int(*polyAng*polyEnc->pulsePerTurn/360.0);
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

	double timeTestPoly=0.0;
	double angTestPoly=0.0;
	int angIncTestPoly=0;

	//Variable for the polynomial function

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
  		polyEval(coeffs1, &timeTestPoly, &angTestPoly);
  		polyAngToIncAng(&angTestPoly, &kneeTwoAuto);
  		angIncTestPoly=kneeTwoAuto.angInc;
  		cout << "Time: "<< timeTestPoly << endl;
  		cout << "Angle in incr: "<< angIncTestPoly << endl;
  		cout << "Angle in degree: "<< angTestPoly << endl;

  		timeTestPoly+=0.02;

  		if(timeTestPoly>0.99){
  			timeTestPoly=0;
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
