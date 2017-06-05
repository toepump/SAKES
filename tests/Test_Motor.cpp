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
int copyCurrToPrevEnc(struct encoder *previous, struct encoder *current);
int fetchAngInc(int *sourceAngInc, struct encoder *current);
int angleIncToDeg (struct encoder *current);
int calcVelAndAcc(struct encoder *current, struct encoder *previous);
int controller(struct encoder *encKnee, struct encoder *encMotor, struct motor *cmdMotor);
int cmdMotor(struct motor *cmdMotor);
int copyIntoOutput(struct encoder *encKnee, struct encoder *encMotor, struct motor *cmdMotor, struct output *output, int increment);
int fileTestMotor(struct output *output);
void *testThread1(void *ptr);
void *testThread2(void *ptr);

struct encoder{
	int angInc; //value of the angle in increment
	double angDeg; //value of the angle in degree
	double velDegSec; //the velocity in deg/sec
	double accDegSec; //the acceleration in deg/secˆ2
	int pulsePerTurn; //number of increment to make one turn for one channel
	int numOfChannel; //number of channel used for this encoder
	int numOfEdge; //number of edge detected for this encoder for one channel (either 1 or 2)
};

struct motor{
	const double dutyMin; //Value of duty min set in the maxon board
	const double dutyMax; //Value of duty max set in the maxon board
	const double velMotorMin; //Value of the velocity min set on the maxon board in rpm
	const double velMotorMax; //Value of the velocity max set on the maxon board in rpm
	const double degSecToRPM=1.0/6.0;; //to convert a speed from deg/sec to rpm
	double gearRatio=1.0/60.0; //from knee to motor, when the motor does one turn, how many turn does the knee
	double currentVelocity; //Value of the current velocity in rpm
	double currentDuty; //Value of the current duty
	double desiredVelocity; //Value of the desired velocity in rpm
	double desiredDuty; //Value of the desired duty
};

struct output{
	double motorCurreVelocity[100000]; //Value of the current velocity in rpm
	double motorCurrDuty[100000]; //Value of the current duty
	double motorDesVelocity[100000]; //Value of the desired velocity in rpm
	double motorDesDuty[100000]; //Value of the desired duty
	int kneeAngInc[100000]; //value of the angle in increment
	double kneeAngDeg[100000]; //value of the angle in degree
	double kneeVelDegSec[100000]; //the velocity in deg/sec
	double kneeAccDegSec[100000]; //the acceleration in deg/secˆ2
	int motorAngInc[100000]; //value of the angle in increment
	double motorAngDeg[100000]; //value of the angle in degree
	double motorVelDegSec[100000]; //the velocity in deg/sec
	double motorAccDegSec[100000]; //the acceleration in deg/secˆ2
};

const int TIME_MAX = 100000; // time max for the loop in ms
const int INTERVALMS =1000000; // in nanosecond
double INTERVAL_S=double(INTERVALMS/1000000000.0);

int ticks_t1=0; //Incremental value for the thread 1
int ticks_t2=0; //Incremental value for the thread 2
const int MAXDEGREEPOLY=57; //57 for coeffs1,  112 for coeffs2 or 3

double coeffs1[MAXDEGREEPOLY]={ 2161704178.57744, -7678966834.50137, 7321336263.45535, 0.0, 0.0, 0.0, -4367145721.75599, 0.0, 0.0, 3978221771.45533, 0.0, 0.0, -985186083.37859, 0.0, 0.0, 0.0, 0.0, -2539164036.74563, 0.0, 0.0, 0.0, 5490325943.05484, 0.0, 0.0, 0.0, -7322471014.88356, 0.0, 0.0, 0.0, 8222278890.26401, 0.0, 0.0, 0.0, -10920714548.72234, 0.0, 0.0,16644524938.81657, 0.0, -16364372664.70290, 0.0,8080579968.97197, 0.0, 0.0, -2834740327.51965, 0.0,  1331213566.88150,  1010961461.30280, -2387418495.97765,1659337298.89251, -622192768.04629, 138921217.362358, -18305060.8984875,1320820.91241054, -50395.6593240169, 1594.07054186194, 38.7663699452653,11.8011809149536};
//double coeffs2[MAXDEGREEPOLY]={-7804126.68756267, 0, 25506481.892212, 0, 0, -31995497.6264176, 0, 0, 0, 0, 0, 0, 0, 60180709.9392192, 0, 0, 0, 0, -91290146.7205068, 0, 0, 0, 0, 0, 62966686.2936572, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -41489120.4273827, 0, 0, 0, 0, 0, 0, 0, 0, 0, 51569710.2406328, 0, 0, 0, 0, 0, 0, 0, 0, 0 , -61482650.4011263, 0, 0, 0, 0, 0, 0, 0, 0, 0, 119151566.929812, 0, 0, 0, 0, -158836801.811755, 0, 0, 0, 0, 107845792.501427, 0, 0, 0, 0, -40438704.5594945, 0, 0, 0, 0, -2017204.55659056, 0, 0, 0, 22368787.3291233, 0, 0, 0, -38587201.2961324,0, 0, 68338463.6321268, 0, -91431570.8652916, 0, 136730933.950438, -146381938.709179, 76449935.5389264, -22951812.7425929, 3901914.86045682, -302425.558782687, -2995.35800355861, 1143.64231358147, 70.3494501822929, 11.3679676751559};
//double coeffs3[MAXDEGREEPOLY]={-5568914.69050683, 0, 18550269.9593775, 0, 0, -23986691.8154377, 0, 0, 0, 0, 0, 0, 0, 49531094.6617748, 0, 0, 0, 0, -80515111.3284084, 0, 0, 0, 0, 0, 61150107.658354, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -52944834.5893432, 0, 0, 0, 0, 0, 0, 0, 0, 0, 88145386.7882361, 0, 0, 0, 0, 0, 0, 0, 0, 0, -156446891.058224, 0, 0, 0, 0, 0, 0, 0, 0, 0, 533593350.532314, 0, 0, 0, 0, -1048894258.17445, 0, 0, 0, 0, 1199016711.57069, 0, 0, 0, 0, -1033184989.92329, 0, 0, 0, 0, 862028565.726653, 0, 0, 0, -705213545.729899, 0, 0, 0, 614832548.422423, 0, 0, -746208588.092968, 0, 791392111.350242, 0, -947590096.72696, 914359301.492975, -436255303.466556, 123792199.437433, -21816443.1312183, 2387498.2633569, -158624.061403544, 4988.62315434288, 158.259318872659, 11.4720337508105};

//Specification of the knee encoder E30S4-3000-6-L-5 from Autonics, with channel A and B activated
struct encoder kneePoly={.angInc=0, .angDeg=0.0, .velDegSec=0.0, .accDegSec=0.0, .pulsePerTurn=3000, .numOfChannel=2, .numOfEdge=2}; //for the polynomial function
struct encoder kneeCurrent={.angInc=0, .angDeg=0.0, .velDegSec=0.0, .accDegSec=0.0, .pulsePerTurn=3000, .numOfChannel=2, .numOfEdge=2}; //to be use for real
struct encoder kneePrevious={.angInc=0, .angDeg=0.0, .velDegSec=0.0, .accDegSec=0.0, .pulsePerTurn=3000, .numOfChannel=2, .numOfEdge=2}; //to be use for real

struct encoder motorCurrent={.angInc=0, .angDeg=0.0, .velDegSec=0.0, .accDegSec=0.0, .pulsePerTurn=3000, .numOfChannel=2, .numOfEdge=2}; //to be use for real
struct encoder motorPrevious={.angInc=0, .angDeg=0.0, .velDegSec=0.0, .accDegSec=0.0, .pulsePerTurn=3000, .numOfChannel=2, .numOfEdge=2}; //to be use for real

//Specification of the motor
struct motor maxon1={.dutyMin=0.10, .dutyMax=0.90, .velMotorMin=-8000.0, .velMotorMax=8000.0, .currentVelocity=0.0, .currentDuty=0.0, .desiredVelocity=0.0, .desiredDuty=0.0};

void polyEval(double coeffs[], double *time, double *angle){
	//from the coefficient in coeffs[] and the time in @time, give the angle in @angle
	int i=0;
	double result=0.0;

	for(i=0;i<MAXDEGREEPOLY-1;i++){
		result+=coeffs[i]*pow(*time,MAXDEGREEPOLY-1-i);
	}
	result+=coeffs[MAXDEGREEPOLY-1];
	*angle=result;
}

void polyAngToIncAng(double *polyAng, struct encoder *polyEnc){
	//put the value of the angle in degree to the value of angInc of the encoder
	polyEnc->angInc=int(*polyAng*polyEnc->pulsePerTurn*polyEnc->numOfChannel*polyEnc->numOfEdge/360.0);
}

int copyCurrToPrevEnc(struct encoder *previous, struct encoder *current){
	//Copy the value of the variable from @previous to @current
	previous->angInc;current->angInc;
	previous->angDeg;current->angDeg;
	previous->velDegSec;current->velDegSec;
	previous->accDegSec;current->accDegSec;

	return 0;
}

int fetchAngInc(int *sourceAngInc, struct encoder *current){
	//Copy the value of @sourceAngInc into @current.angInc,
	current->angInc=sourceAngInc;
	return 0;
}

int angleIncToDeg (struct encoder *current){
	/*Take the value of an angle in increment and turn it into an angle in deg,
	 * using the number of pulse per turn, the number of channel used and the number of edge that create an interrupt.
	 */
	current->angDeg=double(*current->angInc/current->pulsePerTurn/current->numOfChannel/current->numOfEdge*360.0);
	return 0;
}

int calcVelAndAcc(struct encoder *current, struct encoder *previous){
	//Calculate the current value of the velocity and the acceleration
	current->velDegSec=(previous->angDeg-current->angDeg)*INTERVAL_S;
	current->accDegSec=(previous->velDegSec-current->velDegSec)*INTERVAL_S;
	return 0;
}

int controller(struct encoder *encKnee, struct encoder *encMotor, struct motor *cmdMotor){

	//Simple controller
	//Evaluate the velocity of the motor in RPM
	cmdMotor->desiredVelocity=encKnee->velDegSec*cmdMotor->degSecToRPM*cmdMotor->gearRatio;

	//Calculate the value of the desired duty
	cmdMotor->desiredDuty=(cmdMotor->desiredVelocity-cmdMotor->velMotorMin)*(cmdMotor->dutyMax-cmdMotor->dutyMin)/(cmdMotor->velMotorMin-cmdMotor->velMotorMin)+cmdMotor->dutyMin;

	//Put the former value int current
	cmdMotor->currentVelocity=cmdMotor->desiredVelocity;
	cmdMotor->currentDuty=cmdMotor->desiredDuty;

	return 0;
}

int cmdMotor(struct motor *cmdMotor){

	//TODO

	return 0;
}

int copyIntoOutput(struct encoder *encKnee, struct encoder *encMotor, struct motor *cmdMotor, struct output *output, int increment){

	//Motor command
	output->motorCurreVelocity[increment]=cmdMotor->currentVelocity; //Value of the current velocity in rpm
	output->motorCurrDuty[increment]=cmdMotor->currentDuty; //Value of the current duty
	output->motorDesVelocity[increment]=cmdMotor->desiredVelocity; //Value of the desired velocity in rpm
	output->motorDesDuty[increment]=cmdMotor->desiredDuty; //Value of the desired duty

	//knee Encoder
	output->kneeAngInc[increment]=encKnee->angInc; //value of the angle in increment
	output->kneeAngDeg[increment]=encKnee->angDeg; //value of the angle in degree
	output->kneeVelDegSec[increment]=encKnee->velDegSec; //the velocity in deg/sec
	output->kneeAccDegSec[increment]=encKnee->accDegSec; //the acceleration in deg/secˆ2
	//Motor Encoder
	output->motorAngInc[increment]=encMotor->angInc; //value of the angle in increment
	output->motorAngDeg[increment]=encMotor->angDeg; //value of the angle in degree
	output->motorVelDegSec[increment]=encMotor->velDegSec; //the velocity in deg/sec
	output->motorAccDegSec[increment]=encMotor->accDegSec; //the acceleration in deg/secˆ2

	return 0;
}

int fileTestMotor(struct output *output){

	cout << "Printing of the output starts" << endl;

		int i=0;
		FILE *fj1=fopen("fileTestMotor.dat","w");

		fprintf(fj1,"indexOutput;Motor Current Velocity; Motor Current Duty; Motor Desired Velocity; Motor Desired Duty;"
				"Knee Enc Ang Inc; Knee Enc Ang Deg; Knee Enc Vel Deg/sec; Knee Acc Deg/secsec;"
				"Motor Enc Ang Inc; Motor Enc Ang Deg; Motor Enc Vel Deg/sec; Motor Acc Deg/secsec;"
				" \r\n");

		while(i<TIME_MAX){
		    fprintf(fj1,"%d;%d;%f;%d;%d;%d;\r\n",
		    	i+1, output->motorCurreVelocity[i], output->motorCurrDuty[i],output->motorDesVelocity[i], output->motorDesDuty[i],
				output->kneeAngInc[i], output->kneeAngDeg[i], output->kneeVelDegSec[i], output->kneeAccDegSec[i],
				output->motorAngInc[i], output->motorAngDeg[i], output->motorVelDegSec[i],output->motorAccDegSec[i]
		    );

		    if(i==TIME_MAX-1){
		    	fclose(fj1);
		    }
		    i++ ;
		}

	cout << "Printing of the output is done" << endl;

	return 0;
}

int


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
	double testValue2;

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

  	//if(ticks_t1%500==0){
  		//simulation of the polynomial
  		polyEval(coeffs1, &timeTestPoly, &angTestPoly); //put the value in angTestPoly
  		polyAngToIncAng(&angTestPoly, &kneePoly); //Copy it into kneePoly

  		//actual calculation
  		copyCurrToPrevEnc(&kneePrevious, &kneeCurrent); //copy the value from curr to previous
  		fetchAngInc(&kneePoly.angInc, &kneeCurrent); //take the value in kneeCurrent
  		angleIncToDeg(&kneeCurrent); //convert the value from inc to deg
  		calcVelAndAcc(&kneeCurrent, &motorCurrent);
  		controller(&kneeCurrent, &motorCurrent, &maxon1);
  		cmdMotor(&maxon1);

  		testValue2=kneePoly.angDeg;
  		cout << "Time: "<< timeTestPoly << endl;
  		cout << "Angle in degree (polynomial): "<< angTestPoly << endl;
  		cout << "Angle in degree (after fetch, in the encoder): "<< testValue2 << endl;
  		cout << " "<<  endl;

  		timeTestPoly+=0.001;

  		if(timeTestPoly>=0.999){
  			timeTestPoly=0.0;
  		//}
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
