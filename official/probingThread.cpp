#include "common.h"
#include "probingThread.h"
/*
Purpose: every millisecond, forcibly take mutex protecting encoder data
         and do complete processing of that data within 1 millisecond.
*/
void *probingThread(void *ptr){
    //do stuff on mutex protected data
    probeAngleDeg[index] = netAngleDegree; //store current netAngleDegree
    probeIncrement[index] = indexOutput;
    index++;                               //increment index

    if(index > PROBE_STORAGE_SIZE || indexOutput-1 == MAX_PULSE){               //if index is past storage limit, print
        cout << "number of failure: " << failInt << endl;
        printProbe();
        return (void*) NULL;
    }
    //release mutex if finished
    mtx.unlock();
    return (void*) NULL;
}

void printProbe(void){
    cout << "Printing of the probe starts" << endl;

    int i=0;
    FILE *fj2=fopen("probeCheck.data","w");

    fprintf(fj2, "Time (ms); Net Angle (degree); net Increment;\n");

    while(i<PROBE_STORAGE_SIZE){
        fprintf(fj2,  "%d;%f;%d;\r\n", i, probeAngleDeg[i], probeIncrement[i]);
        i++ ;
    }

    cout << "Printing of the output is done" << endl;

}
