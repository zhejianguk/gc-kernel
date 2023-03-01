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

void gcStartup (void) __attribute__ ((constructor));
void gcCleanup (void) __attribute__ ((destructor));

pthread_t threads[NUM_CORES-1];

void* thread_shadowstack_gc(void* args){
    uint64_t hart_id = (uint64_t) args;
	
	// GC variables
	uint64_t Header, Opcode, Func, Rd, RS1, Payload, PC, Inst;
    // dequeue shadow_header, shadow_payload;
	dequeue shadow_payload;

  	// initialize(&shadow_header);
  	initialize(&shadow_payload);

	//================== Initialisation ==================//
	if (gc_pthread_setaffinity(hart_id) != 0){
		printf ("[Rocket-C%x-SS]: pthread_setaffinity failed.", hart_id);
	}
	
	if (GC_DEBUG == 1) {
		printf ("[Rocket-C%x-SS]: ShadowStack is initialised! \r\n", hart_id);
	}

	ghe_initailised();

	//===================== Execution =====================//
	while (ghe_checkght_status() != 0x02){
		while (ghe_status() != GHE_EMPTY){
			ROCC_INSTRUCTION_D (1, Header, 0x0A);
			ROCC_INSTRUCTION_D (1, Payload, 0x0D);

			Opcode = Header & 0x7F;
			Func = (Header & 0x7000) >> 12;
			Rd = (Header & 0xF80) >> 7;
			RS1 = (Header & 0xF8000) >> 15;

			// Push -- a function is called
			if ((((Opcode == 0x6F) || (Opcode == 0x67)) && (Rd == 0x01))) {
				if (full(&shadow_payload) == 0) {
					// enqueueF(&shadow_header, Header);
					enqueueF(&shadow_payload, Payload);
					if (GC_DEBUG) {
						PC = Header >> 32;
						Inst = Header & 0xFFFFFFFF;
						printf("[C%x SS]: <<Pushed>> Expected: %x.                        PC: %x. Inst: %x. \r\n", hart_id, Payload, PC, Inst);
					}
				}
			}

			// Pull -- a function is returned
			if (((Opcode == 0x67) && (Rd == 0x00) && (RS1 == 0x01))) {
				if (empty(&shadow_payload) == 1) {
					if (GC_DEBUG == 1){
						PC = Header >> 32;
						Inst = Header & 0xFFFFFFFF;
						printf("[C%x SS]: ==Empty== Uninteded: %x.                        PC: %x. Inst: %x. \r\n", hart_id, Payload, PC, Inst);
					}
				} else {
					uint64_t comp = dequeueF(&shadow_payload);
					// dequeueF(&shadow_header);

					if (comp != Payload){
						PC = Header >> 32;
						Inst = Header & 0xFFFFFFFF;
						printf("[C%x SS]: **Error**  Expected: %x. v.s. Pulled: %x. PC: %x. Inst: %x. \r\n", hart_id, comp, Payload, PC, Inst);
						// return -1;
					} else {
						if (GC_DEBUG) {
							PC = Header >> 32;
							Inst = Header & 0xFFFFFFFF;
							printf("[C%x SS]: --Paried-- Expected: %x. v.s. Pulled: %x. PC: %x. Inst: %x. \r\n", hart_id, comp, Payload, PC, Inst);
						}
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
	
	// GC threads
    for (uint64_t i = 0; i < NUM_CORES - 1; i++) {
		pthread_create(&threads[i], NULL, thread_shadowstack_gc, (void *) (i+1));
	}

	while (ght_get_initialisation() == 0){
 	}
	
	ght_set_satp_priv();
	printf("[Boom-C-%x]: Test is now started: \r\n", BOOM_ID);
	asm volatile("fence rw, rw;");
	ght_set_status_01 (); // ght: start
    //===================== Execution =====================//
}
  
void gcCleanup (void)
{	
	//=================== Post execution ===================//
    ght_set_status_02 ();

    // GC threads.
	for (uint64_t i = 0; i < NUM_CORES-1; i++) {
		pthread_join(threads[i], NULL);
	}

	if (GC_DEBUG == 1){
		printf("[BOOM-C-%x]: Test is now completed! \r\n", BOOM_ID);
	}

	ght_unset_satp_priv();
	ght_set_status_00 ();
}