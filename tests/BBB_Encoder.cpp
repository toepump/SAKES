//============================================================================
// Name        : BBB_Encoder.cpp
// Author      : Vincent Babin and Michael Kentaro Cho
// Version     :
// Copyright   : Free Software
// Description : Test file for BBB interrupt handling and acheiving RT-Preempt
//               low latency interrupts through BBB PRU pins
//============================================================================

//TODO: check that the channels arent being mixed or something

#include<iostream>
#include<unistd.h>
#include<fstream>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<glib-2.0/glib.h>

using namespace std;

const int MAX_PULSE = 30000; //maximum number of pulse recorded
const double PULSE_PER_DEGREE = 12000.0/360.0; // The number of pulse (interrupt) to complete one degree

int encoder=0;
int angle=0;
int state=0;
static int init=0;
int indexOutput = 0;
double netAngleDegree = 0;

//prototypes
static gboolean EventA( GIOChannel *channel, GIOCondition condition, gpointer user_data );
static gboolean EventB( GIOChannel *channel, GIOCondition condition, gpointer user_data );
void counter(int nb_signal);
void printProbe(void);

//storage
double outputNetAngle[MAX_PULSE]; //Store the value at each interrupt
int outputState[MAX_PULSE];
int outputNetIncrement[MAX_PULSE];


static gboolean EventA( GIOChannel *channel, GIOCondition condition, gpointer user_data )
{
    const int nb_signal=1;
    GError *error = 0;
    gsize bytes_read = 0;
    const int buf_sz = 1024;
    gchar buf[buf_sz] = {};
    g_io_channel_seek_position( channel, 0, G_SEEK_SET, 0 );
    GIOStatus rc = g_io_channel_read_chars( channel, buf,buf_sz - 1,&bytes_read,&error );
    counter(nb_signal);
    //cout << "Event A" << endl;
    return 1;
}

static gboolean EventB( GIOChannel *channel, GIOCondition condition, gpointer user_data )
{
    const int nb_signal=2;
    GError *error = 0;
    gsize bytes_read = 0;
    const int buf_sz = 1024;
    gchar buf[buf_sz] = {};
    g_io_channel_seek_position( channel, 0, G_SEEK_SET, 0 );
    GIOStatus rc = g_io_channel_read_chars( channel, buf,buf_sz - 1,&bytes_read,&error );
    counter(nb_signal);
    //cout << "Event B" << endl;
    return 1;
}

//test counter function to determine which direction encoder is and how much
void counter(int nb_signal) {
    init++;

    if(init>2 && indexOutput < MAX_PULSE){

        if(state==1){
            if(nb_signal==2){
                encoder++;
                state=2;
            }else if(nb_signal==1){
                encoder--;
                state=4;
            }else{
                cout << "problem with the counter in case 1" << endl;
            }



        }
        else if(state==2){
            if(nb_signal==1){
                encoder++;
                state=3;
            }else if(nb_signal==2){
                state=1;
                encoder--;
            }else{
                cout << "problem with the counter in case 2" << endl;
            }


        }
        else if(state==3){
            if(nb_signal==2){
                encoder++;
                state=4;
            }else if(nb_signal==1){
                encoder--;
                state=2;
            }else{
                cout << "problem with the counter in case 3" << endl;
            }


        }
        else if(state==4){
            if(nb_signal==1){
                encoder++;
                state=1;
            }else if(nb_signal==2){
                encoder--;
                state=3;
            }else{
                cout << "problem with the counter in case 4" << endl;
            }
        }

        // if(encoder%3000==0){
        //     angle=encoder/1000;
        //     angle++;
        // }

        netAngleDegree=double(encoder)/PULSE_PER_DEGREE;

        outputNetIncrement[indexOutput]=encoder;
        outputNetAngle[indexOutput]=netAngleDegree;
        outputState[indexOutput] = state;
        indexOutput++;

        if(indexOutput+1==MAX_PULSE){
            cout<< "checking if indexOutput is changing not. " << endl;
            cout<< "indexOutput-1 = " << indexOutput << "  MAX_PULSE = " << MAX_PULSE<< endl;
            cout << "printOutStarted" << endl;
            printOutData();
        }
    }
}

void printOutData(void){
    cout << "Printing of the output starts" << endl;

    int i=0;
    FILE *fj3=fopen("BBBencoderOutput.dat","w");

    fprintf(fj3,"indexOutput;Net Increment;Net Angle (degrees);State;EncoderForward;EncoderBackward;\r\n");

    while(i<MAX_PULSE){
        fprintf(fj3,"%d;%f;%d;\r\n", i+1, outputNetIncrement[i], outputNetAngle[i],outputState[i]);

        if(i==MAX_PULSE-1){
            fclose(fj3);
        }
        i++ ;
    }

    cout << "Printing of the output is done" << endl;

}

void initCounter(void){

    cout << "Initialization Counter" << endl;
    //Declaration of the initial state of the encoder signal//
    int initA;
    int initB;
    int initC;
    int initD;

    //Operation for A - signal A
    int fd = open( "/sys/class/gpio/gpio66/value", O_RDONLY | O_NONBLOCK );
    GIOChannel* channel = g_io_channel_unix_new(fd);
    GError *error = 0;
    gsize bytes_read = 0;
    const int buf_sz = 1024;
    gchar buf[buf_sz] = {};
    g_io_channel_seek_position( channel, 0, G_SEEK_SET, 0 );
    GIOStatus rc = g_io_channel_read_chars( channel, buf,buf_sz - 1,&bytes_read,&error );

    if(buf[0]=='0'){
        initA=0;
    }else if(buf[0]=='1'){
        initA=1;
    }else{
        cout << "probleme with the signal A init" << endl;
    }

    //Operation for C - signal B
    fd = open( "/sys/class/gpio/gpio69/value", O_RDONLY | O_NONBLOCK );
    channel = g_io_channel_unix_new(fd);
    bytes_read = 0;
    g_io_channel_seek_position( channel, 0, G_SEEK_SET, 0 );
    rc = g_io_channel_read_chars( channel, buf,buf_sz - 1,&bytes_read,&error );

    if(buf[0]=='0'){
        initB=0;
    }else if(buf[0]=='1'){
        initB=1;
    }else{
        cout << "probleme with the signal B init" << endl;
    }


    if((initA==0)&&(initB==1)){
        state=1;
        cout << "Initialize counter done, init state: " << state << endl;
    }else if((initA==0)&&(initB==0)){
        state=2;
        cout << "Initialize counter done, init state: " << state << endl;
    }else if((initA==1)&&(initB==0)){
        state=3;
        cout << "Initialize counter done, init state: " << state << endl;
    }else if((initA==1)&&(initB==1)){
        state=4;
        cout << "Initialize counter done, init state: " << state << endl;
    }else{
        cout << "Problem with the counter init" << endl;
    }

}

int main( int argc, char** argv )
{
    //get current value of both pins and store value to find initial state
    initCounter();

    //initialize loops for both events
    GMainLoop* loopA = g_main_loop_new( 0, 0 );
    GMainLoop* loopB = g_main_loop_new( 0, 0 );


    int fdA = open( "/sys/class/gpio/gpio66/value", O_RDONLY | O_NONBLOCK );
    GIOChannel* channelA = g_io_channel_unix_new(fdA);
    GIOCondition condA = GIOCondition(G_IO_PRI);
    guint idA = g_io_add_watch(channelA, condA, EventA, 0);

    int fdB = open( "/sys/class/gpio/gpio69/value", O_RDONLY | O_NONBLOCK );
    GIOChannel* channelB = g_io_channel_unix_new(fdB);
    GIOCondition condB = GIOCondition(G_IO_PRI);
    guint idB = g_io_add_watch(channelB, condB, EventB, 0);

    g_main_loop_run( loopA );
    g_main_loop_run( loopB );
}
