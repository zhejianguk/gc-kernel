#ifndef _GNU_SOURCE
	#define _GNU_SOURCE             /* See feature_test_macros(7) */
#endif

#include <stack>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)
#include "libraries/ght.h"
#include "libraries/ghe.h"
#include "libraries/gc_top.h"

void gcStartup (void) __attribute__ ((constructor));
void gcCleanup (void) __attribute__ ((destructor));

pthread_t threads[NUM_CORES-1];

void* thread_shadowstack_gc(void* args){
	uint64_t hart_id = (uint64_t) args;
	std::deque<uint64_t> shadow;

	// GC variables
	uint64_t Header,Payload, PC, Inst;
	//================== Initialisation ==================//
	if (gc_pthread_setaffinity(hart_id) != 0){
		printf ("[Rocket-C%x-SS]: pthread_setaffinity failed.", hart_id);
	}

	ghe_initailised();
	//===================== Execution =====================//
	while (ghe_checkght_status() != 0x02){
		while (ghe_status() != GHE_EMPTY){
			ROCC_INSTRUCTION_D (1, Payload, 0x0D);
			uint64_t type = Payload & 0x03;

			if (type == 1) {
				shadow.push_front(Payload);
			} else if (type == 2) {
				if (!shadow.empty()) {
					uint64_t comp = shadow.front() + 1;
					shadow.pop_front();
					if (comp != Payload){
						printf("[Rocket-C%x-SS]: **Error** Exp:%x v.s. Pul:%x! \r\n", hart_id, comp>>2, Payload>>2);
					}
				}
			}
		}
	}
	//=================== Post execution ===================//
	ghe_deinitailised();
	ghe_release();

	return NULL; 
}



void gcStartup (void)
{
    //================== Initialisation ==================//
    // Bound current thread to BOOM
    if (gc_pthread_setaffinity(BOOM_ID) != 0) {
		printf ("[BOOM-C%x]: pthread_setaffinity failed.", BOOM_ID);
	}
	
	for (uint64_t i = 0; i < NUM_CORES - 1; i++) {
		pthread_create(&threads[i], NULL, thread_shadowstack_gc, (void *) (i+1));
	}

	while (ght_get_initialisation() == 0){
 	}
	
	ght_set_satp_priv();
	printf("[Boom-C-%x]: Test is now started: \r\n", BOOM_ID);
	ght_set_status_01 (); // ght: start
    //===================== Execution =====================//
}
  
void gcCleanup (void)
{	
	//=================== Post execution ===================//
    ght_set_status_02 ();

	for (uint64_t i = 0; i < NUM_CORES-1; i++) {
		pthread_join(threads[i], NULL);
	}

	if (GC_DEBUG == 1){
		printf("[BOOM-C-%x]: Test is now completed! \r\n", BOOM_ID);
	}

	ght_unset_satp_priv();
	ght_set_status_00 ();
}