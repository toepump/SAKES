//constants
extern MAX_PULSE = 30000
extern PROBE_STORAGE_SIZE = 20000;             // the arbitrary size of stored
                                        // probe's storage
//global variables
extern int state = 0;                              //state of channels
extern int netAngleIncrement = 0;                  //storage for temporary netAngleIncrement, to copy in to netAngleIncrement
extern int RealNetAngleIncrement = 0;              //storage for actual netAngleIncrement, used while being probed
extern const int INTERVAL =1000000;                //in nanosecond
extern std::mutex mtx;                             //probingThread mutex
extern std::mutex dataMtx;                         //actual mutex to prevent data corruption

extern int index = 0;

extern double probeAngleDeg[PROBE_STORAGE_SIZE];   // the strorage space for probed data
extern int probeIncrement[PROBE_STORAGE_SIZE];     // the storage space for the probed incremental data
extern double outputNetAngle[MAX_PULSE];           // the storage space for the netAngle debug
extern int outputState[MAX_PULSE];                 // the storage space for the state debug
extern int outputNetIncrement[MAX_PULSE];          //Store the value at each interrupt
