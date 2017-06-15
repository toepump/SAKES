#ifndef COMMON_H
#define COMMON_H

#define NSEC_PER_SEC    (1000000000)           /* The number of nsecs per sec. */
#define NSEC_PER_MSEC   (1000000)              //number of nsecs in milliseconds

//constants
extern MAX_PULSE = 30000
extern PROBE_STORAGE_SIZE = 20000;             // the arbitrary size of stored
                                               // probe's storage
//global variables
extern int state = 0;                              //state of channels
extern int netAngleIncrement = 0;                  //storage for temporary netAngleIncrement, to copy in to netAngleIncrement
extern int RealNetAngleIncrement = 0;              //storage for actual netAngleIncrement, used while being probed
extern int INTERVAL = 1000000;                     //in nanosecond
extern std::mutex mtx;                             //probingThread mutex
extern std::mutex dataMtx;                         //mutex to protect data

extern int index = 0;
extern int indexOutput = 0;

extern double probeAngleDeg[PROBE_STORAGE_SIZE];   // the strorage space for probed data
extern int probeIncrement[PROBE_STORAGE_SIZE];     // the storage space for the probed incremental data
extern double outputNetAngle[MAX_PULSE];           // the storage space for the netAngle debug
extern int outputState[MAX_PULSE];                 // the storage space for the state debug
extern int outputNetIncrement[MAX_PULSE];          //Store the value at each interrupt
extern double netAngleDegree=0;                    //the net angle in degree

extern int failInt = 0;
extern init = 0;


#endif
