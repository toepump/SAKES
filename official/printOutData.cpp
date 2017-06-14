#include "printOutData.h"

/*
Purpose: print out data collected from the encoders including:
         index, interrupt number, calculated angle of encoder, and state
*/
void printOutData(void){
    cout << "Printing of the output starts" << endl;

    int i=0;
    FILE *fj1=fopen("outputEncoder.dat","w");

    fprintf(fj1,"indexOutput;Net Increment;Net Angle (degrees);State;\r\n");

    while(i<MAX_PULSE){
        fprintf(fj1,"%d;%d;%f;%d;%d;%d;\r\n", i+1, outputNetIncrement[i], outputNetAngle[i],outputState[i]);

        if(i==MAX_PULSE-1){
            fclose(fj1);
        }
        i++ ;
    }

    cout << "Printing of the output is done" << endl;

}
