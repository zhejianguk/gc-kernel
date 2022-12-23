#ifndef _GNU_SOURCE
	#define _GNU_SOURCE             /* See feature_test_macros(7) */
#endif
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "libraries/ght.h"
#include "libraries/gc_top.h"
#include "libraries/ghe.h"

void gcStartup (void)
{
    // Bound current thread to BOOM
    if (gc_pthread_setaffinity(BOOM_ID) != 0){
		printf ("[Boom-C%x]: pthread_setaffinity failed.", BOOM_ID);
	} else{
		// do nothing.
	}
}
  
void gcCleanup (void)
{	

}

