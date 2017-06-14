#include "interruptThread.h"
#include "common.h"
#include "counter.h"

/*
Purpose: constantly be interrupted by encoder pulses found through GPIO
         in linux filesystem 'value' files. Upon receiving interrupt from either
         channel, call EventA or EventB functions.
*/
void *interruptThread(void *ptr){
    char *message;
    message = (char *) ptr;

    while(true) {

        //initialize the looping for interrupt handling of both channels
        GMainLoop* loopA = g_main_loop_new(0, 0);
        GMainLoop* loopB = g_main_loop_new(0, 0);

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

    return (void*) NULL;
}

/*
Purpose: constantly be listening for interrupt coming from channelA
         if interrupted, then call counter function with argument
         indicating channelA was the interrupted channel
*/
static gboolean EventA( GIOChannel *channel, GIOCondition condition, gpointer user_data )
{
    const int channelSig=1;
    GError *error = 0;
    gsize bytes_read = 0;
    const int buf_sz = 1024;
    gchar buf[buf_sz] = {};
    g_io_channel_seek_position( channel, 0, G_SEEK_SET, 0 );
    GIOStatus rc = g_io_channel_read_chars( channel, buf,buf_sz - 1,&bytes_read,&error );
    counter(channelSig);
    return 1;
}

/*
Purpose: constantly be listening for intterupt coming from channelB
         if interruped, then call counter functino with argument
         indicating channelB was the interrupted channel
*/
static gboolean EventB( GIOChannel *channel, GIOCondition condition, gpointer user_data )
{
    const int channelSig=2;
    GError *error = 0;
    gsize bytes_read = 0;
    const int buf_sz = 1024;
    gchar buf[buf_sz] = {};
    g_io_channel_seek_position( channel, 0, G_SEEK_SET, 0 );
    GIOStatus rc = g_io_channel_read_chars( channel, buf,buf_sz - 1,&bytes_read,&error );
    counter(channelSig);
    return 1;
}
