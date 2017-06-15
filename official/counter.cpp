#include "common.h"
#include "counter.h"
#include "printOutData.h"

void counter(int channelSig){
    // TODO store actual netAngleIncrement and temp netAngleIncrement seperately
    // and protect the actual one with a mutex so that the probe reads a safe
    // data. every if statement update temp and copy to actual but surround actual
    // with mutex CONDITION VARIABLE MAYBE
    init++;

    if(init>2 && indexOutput<MAX_PULSE){
        if(state==1){
            if(channelSig==2){
                netAngleIncrement++;
                RealNetAngleIncrement = netAngleIncrement;
                state=2;
            }else if(channelSig==1){
                netAngleIncrement--;
                RealNetAngleIncrement = netAngleIncrement;
                state=4;
            }else{
                failInt++;
                //cout << "problem with the counter in case 1" << endl;
            }
        }

        else if(state==2){
            if(channelSig==1){
                netAngleIncrement++;
                RealNetAngleIncrement = netAngleIncrement;
                state=3;
            }else if(channelSig==2){
                state=1;
                netAngleIncrement--;
                RealNetAngleIncrement = netAngleIncrement;
            }else{
                failInt++;
                //cout << "problem with the counter in case 1" << endl;
            }
        }

        else if(state==3){
            if(channelSig==2){
                netAngleIncrement++;
                RealNetAngleIncrement = netAngleIncrement;
                state=4;
            }else if(channelSig==1){
                netAngleIncrement--;
                RealNetAngleIncrement = netAngleIncrement;
                state=2;
            }else{
                failInt++;
                //cout << "problem with the counter in case 1" << endl;
            }
        }

        else if(state==4){
            if(channelSig==1){
                netAngleIncrement++;
                RealNetAngleIncrement = netAngleIncrement;
                state=1;
            }else if(channelSig==2){
                netAngleIncrement--;
                RealNetAngleIncrement = netAngleIncrement;
                state=3;
            }else{
                failInt++;
                //cout << "problem with the counter in case 1" << endl;
            }

        }
        else{
            cout<<"state is fucked" << endl;
        }

        netAngleDegree=double(netAngleIncrement)/PULSE_PER_DEGREE;

        outputNetIncrement[indexOutput]=netAngleIncrement;
        outputNetAngle[indexOutput]=netAngleDegree;
        outputState[indexOutput] = state;
        indexOutput++;

        if(indexOutput+1==MAX_PULSE){
            printOutData();
        }
    }
}
