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

	// printf ("[Rocket-C%x-SS]: Checker core is now initalised!", hart_id);	
	
	

	while (1){
		ghe_go();
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
							printf("[Rocket-C%x-SS]: **Error** Exp:%x v.s. Pul:%x! \r\n", comp, Payload);
						}
					}
				}
			}

			// Dedicated for shadowstack 
			if (ghe_checkght_status() == 0x04) {
				ghe_complete();
				while(ghe_checkght_status() == 0x04) {
					ghe_complete();
				}
				ghe_go();
			}
		}
		//=================== Post execution ===================//
		ghe_deinitailised();
		ghe_release();

		while (ghe_checkght_status() == 0x02){
			// Wait for the completion
		}
	}

	return NULL; 
}

  
int main (void)
{	
   	//================== Initialisation ==================//
    // Bound current thread to BOOM
	/*
    if (gc_pthread_setaffinity(BOOM_ID) != 0) {
		printf ("[Boom-C%x]: pthread_setaffinity failed.", BOOM_ID);
	}
	*/

	// GC threads
    for (uint64_t i = 0; i < NUM_CORES - 1; i++) {
		pthread_create(&threads[i], NULL, thread_shadowstack_gc, (void *) (i+1));
	}

	for (uint64_t i = 0; i < NUM_CORES-1; i++) {
		pthread_join(threads[i], NULL);
	}

	return 0;
}

