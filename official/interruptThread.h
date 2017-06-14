/*
Purpose: constantly be interrupted by encoder pulses found through GPIO
         in linux filesystem 'value' files. Upon receiving interrupt from either
         channel, call EventA or EventB functions.
*/
void *interruptThread(void *ptr);

/*
Purpose: constantly be listening for interrupt coming from channelA
         if interrupted, then call counter function with argument
         indicating channelA was the interrupted channel
*/
static gboolean EventA( GIOChannel *channel, GIOCondition condition, gpointer user_data );

/*
Purpose: constantly be listening for intterupt coming from channelB
         if interruped, then call counter functino with argument
         indicating channelB was the interrupted channel
*/
static gboolean EventB( GIOChannel *channel, GIOCondition condition, gpointer user_data );
