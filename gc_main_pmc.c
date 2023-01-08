#ifndef _GNU_SOURCE
	#define _GNU_SOURCE             /* See feature_test_macros(7) */
#endif
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)
#include "libraries/ght.h"
#include "libraries/ghe.h"
#include "libraries/gc_top.h"
#include "libraries/spin_lock.h"

void gcStartup (void) __attribute__ ((constructor));
void gcCleanup (void) __attribute__ ((destructor));

pthread_t threads[NUM_CORES-1];

void* thread_pmc(void* args){
    uint64_t hart_id = (uint64_t) args;
    //================== Initialisation ==================//
	// GC variables
	uint64_t perfc = 0;

	if (gc_pthread_setaffinity(hart_id) != 0){
		printf ("[Rocket-C%x-PMC]: pthread_setaffinity failed.", hart_id);
	} else{
		ghe_initailised(1);
	}

	//===================== Execution =====================// 
	while (ghe_checkght_status() != 0x02) {
		while (ghe_status() != GHE_EMPTY){
			ROCC_INSTRUCTION (1, 0x0D);
			perfc++;
		}
	}
	//=================== Post execution ===================//
	if (GC_DEBUG == 1){
		printf("[Rocket-C%x-PMC]: Completed, PMC = %x! \r\n", hart_id, perfc);
	}

	ghe_initailised(0);
	return NULL;
}

void gcStartup (void)
{
    //================== Initialisation ==================//
    // Bound current thread to BOOM
    if (gc_pthread_setaffinity(BOOM_ID) != 0) {
		printf ("[Boom-C%x]: pthread_setaffinity failed.", BOOM_ID);
	} else {
		ght_set_satp_priv();
		// if_tasks_initalised[BOOM_ID] = 1;
	}

    // GC threads
    for (uint64_t i = 0; i < NUM_CORES - 1; i++) {
		pthread_create(&threads[i], NULL, thread_pmc, (void *) (i+1));
	}
	
	if (GC_DEBUG == 1){
		printf("[Boom-%x]: Test is now started: \r\n", BOOM_ID);
	}

	while(ght_get_initialisation () != 1){	
	}

	ght_set_status (0x01); // ght: start
    //===================== Execution =====================//
}
  
void gcCleanup (void)
{	
	//=================== Post execution ===================//
    ght_set_status (0x02);

    // GC threads.
	for (uint64_t i = 0; i < NUM_CORES-1; i++) {
		pthread_join(threads[i], NULL);
	}

	ght_unset_satp_priv();
	ght_set_status (0x00);
}

