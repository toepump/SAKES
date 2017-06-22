#ifndef COMMON_H
#define COMMON_H

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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <glib-2.0/glib.h>
#include <mutex>

#define NSEC_PER_SEC    (1000000000)           /* The number of nsecs per sec. */
#define NSEC_PER_MSEC   (1000000)              //number of nsecs in milliseconds

//constants
const int MAX_PULSE = 30000;
const int PROBE_STORAGE_SIZE = 20000;                   // the arbitrary size of stored
                                                        // probe's storage
const int INTERVAL = 1000000;                           //in nanosecond
const double PULSE_PER_DEGREE = (3000*2*2)/3;

//global variables
extern int state;                               //state of channels
extern int netAngleIncrement;                   //storage for temporary netAngleIncrement, to copy in to netAngleIncrement
extern int RealNetAngleIncrement;               //storage for actual netAngleIncrement, used while being probed

extern std::mutex mtx;                              //probingThread mutex
extern std::mutex dataMtx;                          //mutex to protect data

extern int indexProbe;
extern int indexOutput;

extern double probeAngleDeg[PROBE_STORAGE_SIZE];    // the strorage space for probed data
extern int probeIncrement[PROBE_STORAGE_SIZE];      // the storage space for the probed incremental data
extern double outputNetAngle[MAX_PULSE];            // the storage space for the netAngle debug
extern int outputState[MAX_PULSE];                  // the storage space for the state debug
extern int outputNetIncrement[MAX_PULSE];           //Store the value at each interrupt
extern double netAngleDegree;                     //the net angle in degree

extern int failInt;
extern int init;


#endif
