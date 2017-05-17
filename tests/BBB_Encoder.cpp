//============================================================================
// Name        : BBB_Encoder.cpp
// Author      : Vincent Babin and Michael Kentaro Cho
// Version     :
// Copyright   : Free Software
// Description : Test file for BBB interrupt handling and acheiving RT-Preempt
//               low latency interrupts through BBB PRU pins
//============================================================================

#include<iostream>
#include<unistd.h>
#include<fstream>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<glib-2.0/glib.h>

using namespace std;
int encoder=0;
double encfwd=0;
double encbwd=0;
int angle=0;
int state=0;


static gboolean EventA( GIOChannel *channel, GIOCondition condition, gpointer user_data );
static gboolean EventB( GIOChannel *channel, GIOCondition condition, gpointer user_data );

void counter(int nb_signal);


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

void counter(int nb_signal) {

static int init=0;

init++;

if(init>2){

	if(state==1){
			if(nb_signal==2){
				encoder++;
			
encfwd++;	state=2;
			}else if(nb_signal==1){
encbwd++; encoder--;	
		state=4;
			}else{
				cout << "problem with the counter in case 1" << endl;
			}
				


		}else if(state==2){
				if(nb_signal==1){
					encoder++;
					encfwd++;
					state=3;
				}else if(nb_signal==2){
				state=1;
					encoder--;
					encbwd++;
				}else{
				cout << "problem with the counter in case 2" << endl;
				}
				

		}else if(state==3){
				if(nb_signal==2){
					encoder++;
					encfwd++;				
	state=4;
				}else if(nb_signal==1){
				encoder--; encbwd++;					state=2;
				}else{
					cout << "problem with the counter in case 3" << endl;
				}
				

		}else if(state==4){
				if(nb_signal==1){
				
encfwd++;	encoder++;
					state=1;
				}else if(nb_signal==2){
	encbwd++; encoder--;				state=3;
				}else{
					cout << "problem with the counter in case 4" << endl;
				}
				
	}


	if(encoder%3000==0){
			angle=encoder/1000;
			angle++;
			cout << "Counter: Value Angle " << angle << endl;
			cout << "Coubter encoder : " << encoder << endl;
			cout << "fwd  " << encfwd << " bwd" << encbwd<< endl;	

	}

	if(encoder%3000==0){
			cout << "Counter: State value" << state << endl;
			cout << "  " << endl;
		}
}

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

	initCounter();

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



