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
	uint64_t Header,Payload, PC, Inst, S_Header, S_Payload;
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
				} else {
					// Send it to AGG
					while (ghe_agg_status() == GHE_FULL) {
					}
					ghe_agg_push (hart_id, Payload);
				}
			}
		}

		// If all push and pull are handled
   		if (ghe_sch_status() == 0x01) {
			// Send unpaired pushes
			while (!shadow.empty()) {
				while (ghe_agg_status() == GHE_FULL) {
				}
				S_Payload = shadow.back();
				ghe_agg_push (hart_id, S_Payload);
				shadow.pop_back();
			}
		
			// Send termination flag
			while (ghe_agg_status() == GHE_FULL) {
			}
      		ghe_agg_push (hart_id, 0xFFFFFFFF);

			while (ghe_sch_status() == 0x01) {
				// Wait the core to be waked up
			}
      	}
	}
	//=================== Post execution ===================//
	ghe_deinitailised();
	ghe_release();

	return NULL; 
}

/*
void clear_queue(int index)
{
  	while (empty(&queues[index]) == 0) {
		uint64_t Payload_q = dequeueR(&queues[index]);
		uint64_t type_q = Payload_q & 0x03;

		if (type_q == 1) {
			enqueueF(&shadow_agg, Payload_q);
		} else if (type_q == 2) {
			if (empty(&shadow_agg) != 1) {
				uint64_t comp_q = dequeueF(&shadow_agg) + 1;
				if (comp_q != Payload_q){
					printf("[Rocket-SS-AGG]: **Error** Exp:%x v.s. Pul:%x! \r\n", comp_q>>2, Payload_q>>2);
				}
			}
		}
	}
}
*/

static inline uint64_t nxt_target (uint64_t c_current, uint64_t c_start, uint64_t c_end)
{
	uint64_t c_nxt = (c_current == c_end) ? c_start : c_current + 1;
  	return c_nxt;
}


void* thread_shadowstack_agg_gc(void* args){
	uint64_t hart_id = (uint64_t) args;
	uint64_t CurrentTarget = 1;

	// GC variables
	uint64_t Header,Payload, PC, Inst;
	//================== Initialisation ==================//
	if (gc_pthread_setaffinity(hart_id) != 0){
		printf ("[Rocket-SS-AGG]: pthread_setaffinity failed.");
	}

	std::deque<uint64_t> queues[NUM_CORES];
	std::deque<uint64_t> shadow_agg;


	ghe_initailised();
	//===================== Execution =====================//
	if (GC_DEBUG){
  		printf("[Rocket-SS-AGG]: == Current Target: %x. == \r\n", CurrentTarget);
	}

	while (ghe_checkght_status() != 0x02){
		while (ghe_status() != GHE_EMPTY){
			ROCC_INSTRUCTION_D (1, Header, 0x0A);
			ROCC_INSTRUCTION_D (1, Payload, 0x0D);
			uint64_t from = Header;

			if (from == CurrentTarget) { // Push
				uint64_t type = Payload & 0x03;
				if (type == 1) {
					shadow_agg.push_front(Payload);
				} else if (type == 2) {
					if (!shadow_agg.empty()) {
						uint64_t comp = shadow_agg.front() + 1;
						shadow_agg.pop_front();
						if (comp != Payload){
							printf("[Rocket-AGG]: **Error** Exp:%x v.s. Pul:%x! \r\n", hart_id, comp>>2, Payload>>2);
						}
					}
				} else {
					ROCC_INSTRUCTION_S(1, 0x01 << (CurrentTarget-1), 0x21);	// Restart CurrentTarget for scheduling
					CurrentTarget = nxt_target(CurrentTarget, 1, NUM_CORES-2);
					if (GC_DEBUG){
						printf("[Rocket-SS-AGG]: == Current Target: %x. == \r\n", CurrentTarget);
					}

					while ((!shadow_agg.empty()) && (shadow_agg.front() == 0xFFFFFFFF))  {
						queues[CurrentTarget].pop_front();

						/* Clear Queue */
						while (!queues[CurrentTarget].empty()) {
							uint64_t Payload_q = queues[CurrentTarget].back();
							queues[CurrentTarget].pop_back();
							uint64_t type_q = Payload_q & 0x03;

							if (type_q == 1) {
								shadow_agg.push_front(Payload_q);
							} else if (type_q == 2) {
								if (!shadow_agg.empty()) {
									uint64_t comp_q = shadow_agg.front() + 1;
									shadow_agg.pop_front();
									if (comp_q != Payload_q){
										printf("[Rocket-SS-AGG]: **Error** Exp:%x v.s. Pul:%x! \r\n", comp_q>>2, Payload_q>>2);
									}
								}
							}
						}

						ROCC_INSTRUCTION_S(1, 0x01 << (CurrentTarget-1), 0x21); // Restart CurrentTarget for scheduling
						CurrentTarget = nxt_target(CurrentTarget, 1, NUM_CORES-2);
						if (GC_DEBUG){
							printf("[Rocket-SS-AGG]: == Current Target: %x. == \r\n", CurrentTarget);
						}
					}
					/* Clear Queue */
					while (!queues[CurrentTarget].empty()) {
						uint64_t Payload_q = queues[CurrentTarget].back();
						queues[CurrentTarget].pop_back();
						uint64_t type_q = Payload_q & 0x03;

						if (type_q == 1) {
							shadow_agg.push_front(Payload_q);
						} else if (type_q == 2) {
							if (!shadow_agg.empty()) {
								uint64_t comp_q = shadow_agg.front() + 1;
								shadow_agg.pop_front();
								if (comp_q != Payload_q){
									printf("[Rocket-SS-AGG]: **Error** Exp:%x v.s. Pul:%x! \r\n", comp_q>>2, Payload_q>>2);
								}
							}
						}
					}
				}
			} else {
				queues[from].push_front(Payload);
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
	
	for (uint64_t i = 0; i < NUM_CORES - 2; i++) {
		pthread_create(&threads[i], NULL, thread_shadowstack_gc, (void *) (i+1));
	}

	pthread_create(&threads[NUM_CORES-2], NULL, thread_shadowstack_agg_gc, (void *) (NUM_CORES-1));

	for (int c = 1; c < NUM_CORES -1; c++){
		ROCC_INSTRUCTION_S(1, 0x01 << (c-1), 0x21); // Restart Checkers
	}
	ght_cfg_se (0x00, NUM_CORES-2, 0x0f, 0x01); // Reset SE 0x00

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

	for (uint64_t i = 0; i < NUM_CORES-2; i++) {
		pthread_join(threads[i], NULL);
	}

	if (GC_DEBUG == 1){
		printf("[BOOM-C-%x]: Test is now completed! \r\n", BOOM_ID);
	}

	ght_unset_satp_priv();
	ght_set_status_00 ();
}