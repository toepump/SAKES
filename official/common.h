#ifndef COMMON_H
#define COMMON_H

#define NSEC_PER_SEC    (1000000000)           /* The number of nsecs per sec. */
#define NSEC_PER_MSEC   (1000000)              //number of nsecs in milliseconds

//constants
extern int MAX_PULSE;
extern int PROBE_STORAGE_SIZE;              // the arbitrary size of stored
                                                    // probe's storage
//global variables
extern int state;                               //state of channels
extern int netAngleIncrement;                   //storage for temporary netAngleIncrement, to copy in to netAngleIncrement
extern int RealNetAngleIncrement;               //storage for actual netAngleIncrement, used while being probed
extern int INTERVAL;                      //in nanosecond
extern std::mutex mtx;                              //probingThread mutex
extern std::mutex dataMtx;                          //mutex to protect data

extern int index;
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
