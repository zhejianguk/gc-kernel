#ifndef _GNU_SOURCE
	#define _GNU_SOURCE             /* See feature_test_macros(7) */
#endif
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "libraries/ght.h"
#include "libraries/gc_top.h"
#include "libraries/ghe.h"

/* Apply the constructor attribute to myStartupFun() so that it
     is executed before main() */
 void gcStartup (void) __attribute__ ((constructor));


 /* Apply the destructor attribute to myCleanupFun() so that it
    is executed after main() */
 void gcCleanup (void) __attribute__ ((destructor));
 
void gcStartup (void)
{
    // Bound current thread to BOOM
    if (gc_pthread_setaffinity(BOOM_ID) != 0){
		printf ("[Boom-C%x]: Pthread_setaffinity failed.", BOOM_ID);
		return NULL;
	} else{
		printf ("[Boom-C%x]: Initialised!\r\n", BOOM_ID);
	}
}
  
void gcCleanup (void)
{	
	printf ("[Boom-C%x]: Completed!\r\n", BOOM_ID);
}

