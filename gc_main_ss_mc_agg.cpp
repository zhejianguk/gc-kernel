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
#include "libraries/deque.h"

uint64_t nxt_target (uint64_t c_current, uint64_t c_start, uint64_t c_end)
{
	uint64_t c_nxt;
	if (c_current == c_end) {
    	c_nxt = c_start;
  	} else {
    	c_nxt = c_current + 1;
  	}
  	return c_nxt;
}


int main (void)
{
	uint64_t hart_id = (uint64_t) AGG_CORE_ID;
	// GC variables
	uint64_t Header,Payload, PC, Inst;
	uint64_t CurrentTarget = 1;

	dequeue queues[NUM_CORES];
	dequeue shadow_agg;

	// GC variables
    //================== Initialisation ==================//
    // Bound current thread to hart_id
    if (gc_pthread_setaffinity(hart_id) != 0) {
		printf ("[Rocket-C%x]: pthread_setaffinity failed.", hart_id);
	}
	
	initialize(&shadow_agg);
	for (int i = 0; i < NUM_CORES; i ++)
  	{
    	initialize(&queues[i]);
  	}
	
	ghe_initailised();
	//===================== Execution =====================//
	if (GC_DEBUG){
  		printf("[Rocket-SS-AGG]: == Current Target: %x. == \r\n", CurrentTarget);
	}

	while (ghe_checkght_status() != 0x02){
		while (ghe_status() != GHE_EMPTY){
			ROCC_INSTRUCTION_D (1, Header, 0x0A);
			ROCC_INSTRUCTION_D (1, Payload, 0x0D);
			uint64_t from = Header&0xF;

			if (GC_DEBUG){
				printf("[Rocket-SS-AGG]: Header: %x, Payload: %x \r\n", Header, Payload);
			}

			if (from == CurrentTarget) { // Push
				uint64_t type = Payload & 0x03;
				if (type == 3) { // clear queue
					// clear_queue(CurrentTarget);
					ROCC_INSTRUCTION_S(1, 0x01 << (CurrentTarget-1), 0x21); // Restart CurrentTarget for scheduling
					CurrentTarget = nxt_target(CurrentTarget, 1, NUM_CORES-2);
					if (GC_DEBUG){
						printf("[Rocket-SS-AGG]: == Current Target: %x. == \r\n", CurrentTarget);
					}

					while ((empty(&queues[CurrentTarget]) == 0) && ((queueT(&queues[CurrentTarget]) & 0xFFFFFFFF) == 0xFFFFFFFF)) {
						dequeueF(&queues[CurrentTarget]); // remove the top
						// clear queue (CurrentTarget);
						ROCC_INSTRUCTION_S(1, 0x01 << (CurrentTarget-1), 0x21); // Restart CurrentTarget for scheduling
						CurrentTarget = nxt_target(CurrentTarget, 1, NUM_CORES-2);
						if (GC_DEBUG){
							printf("[Rocket-SS-AGG]: == Current Target: %x. == \r\n", CurrentTarget);
						}
					}
					// clear_queue(CurrentTarget);
				}
			} else {
				enqueueF(&queues[from], Payload);
			}
		}
	}
	//=================== Post execution ===================//
	ghe_deinitailised();
	ghe_release();

	return 0;
}
