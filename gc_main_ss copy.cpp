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

static inline void report_error(uint64_t hart_id, uint64_t comp, uint64_t address){
	uint64_t Header = ghe_get_fifocache0();
	uint64_t PC = Header >> 32;
	uint64_t Inst = Header & 0xFFFFFFFF;
	printf("[C%x SS]: **Error**  Expected: %x. v.s. Pulled: %x. PC: %x. Inst: %x. \r\n", hart_id, comp, address, PC, Inst);
}

void report_push(uint64_t hart_id, uint64_t address){
	uint64_t Header = ghe_get_fifocache0();
	uint64_t PC = Header >> 32;
	uint64_t Inst = Header & 0xFFFFFFFF;
	printf("[C%x SS]: <<Pushed>> Expected: %x.                        PC: %x. Inst: %x. \r\n", hart_id, address, PC, Inst);
}

void report_pair(uint64_t hart_id, uint64_t comp, uint64_t address){
	uint64_t Header = ghe_get_fifocache0();
	uint64_t PC = Header >> 32;
	uint64_t Inst = Header & 0xFFFFFFFF;
	printf("[C%x SS]: --Paried-- Expected: %x. v.s. Pulled: %x. PC: %x. Inst: %x. \r\n", hart_id, comp, address, PC, Inst);
}

void* thread_shadowstack_gc(void* args){
	uint64_t hart_id = (uint64_t) args;

	// GC variables
	uint64_t Header, Payload, Payload1, Payload2, type, address;
	//================== Initialisation ==================//
	if (gc_pthread_setaffinity(hart_id) != 0){
		printf ("[Rocket-C%x-SS]: pthread_setaffinity failed.", hart_id);
	}

	std::deque<uint64_t> shadow;
	asm volatile("fence rw, rw;");

	if (GC_DEBUG){
		printf ("[Rocket-C%x-SS]: ShadowStack is initialised! \r\n", hart_id);
	}
	
	ghe_initailised();
	//===================== Execution =====================//
	while (ghe_checkght_status() != 0x02){
		// Run analysis
		uint64_t buffer_depth = ghe_get_bufferdepth();
		while (buffer_depth > 7){			
			ROCC_INSTRUCTION_D (1, Payload1, 0x0D);
			ROCC_INSTRUCTION_D (1, Payload2, 0x0D);
	
			type = Payload1 & 0x03;
			address = Payload1 >> 2;
			if (type == 1) { // Push -- a function is called
				shadow.push_front(address);
				if (GC_DEBUG) {
					report_push (hart_id, address);
				}
			} else { // Pop -- a function is called
				if (type == 2) {
					if (!shadow.empty()) {
						uint64_t comp = shadow.front();
						shadow.pop_front();
						if (comp != address){
							report_error(hart_id, comp, address);
							// return -1;
						} else {
							if (GC_DEBUG) {
								report_pair(hart_id, comp, address);
							}
						}
					}
				}
			}

			type = Payload2 & 0x03;
			address = Payload2 >> 2;
			if (type == 1) { // Push -- a function is called
				shadow.push_front(address);
				if (GC_DEBUG) {
					report_push (hart_id, address);
				}
			} else { // Pop -- a function is called
				if (type == 2) {
					if (!shadow.empty()) {
						uint64_t comp = shadow.front();
						shadow.pop_front();
						if (comp != address){
							report_error(hart_id, comp, address);
							// return -1;
						} else {
							if (GC_DEBUG) {
								report_pair(hart_id, comp, address);
							}
						}
					}
				}
			}

			ROCC_INSTRUCTION_D (1, Payload1, 0x0D);
			ROCC_INSTRUCTION_D (1, Payload2, 0x0D);
			type = Payload1 & 0x03;
			address = Payload1 >> 2;
			if (type == 1) { // Push -- a function is called
				shadow.push_front(address);
				if (GC_DEBUG) {
					report_push (hart_id, address);
				}
			} else { // Pop -- a function is called
				if (type == 2) {
					if (!shadow.empty()) {
						uint64_t comp = shadow.front();
						shadow.pop_front();
						if (comp != address){
							report_error(hart_id, comp, address);
							// return -1;
						} else {
							if (GC_DEBUG) {
								report_pair(hart_id, comp, address);
							}
						}
					}
				}
			}

			type = Payload2 & 0x03;
			address = Payload2 >> 2;
			if (type == 1) { // Push -- a function is called
				shadow.push_front(address);
				if (GC_DEBUG) {
					report_push (hart_id, address);
				}
			} else { // Pop -- a function is called
				if (type == 2) {
					if (!shadow.empty()) {
						uint64_t comp = shadow.front();
						shadow.pop_front();
						if (comp != address){
							report_error(hart_id, comp, address);
							// return -1;
						} else {
							if (GC_DEBUG) {
								report_pair(hart_id, comp, address);
							}
						}
					}
				}
			}

			ROCC_INSTRUCTION_D (1, Payload1, 0x0D);
			ROCC_INSTRUCTION_D (1, Payload2, 0x0D);
			type = Payload1 & 0x03;
			address = Payload1 >> 2;
			if (type == 1) { // Push -- a function is called
				shadow.push_front(address);
				if (GC_DEBUG) {
					report_push (hart_id, address);
				}
			} else { // Pop -- a function is called
				if (type == 2) {
					if (!shadow.empty()) {
						uint64_t comp = shadow.front();
						shadow.pop_front();
						if (comp != address){
							report_error(hart_id, comp, address);
							// return -1;
						} else {
							if (GC_DEBUG) {
								report_pair(hart_id, comp, address);
							}
						}
					}
				}
			}

			type = Payload2 & 0x03;
			address = Payload2 >> 2;
			if (type == 1) { // Push -- a function is called
				shadow.push_front(address);
				if (GC_DEBUG) {
					report_push (hart_id, address);
				}
			} else { // Pop -- a function is called
				if (type == 2) {
					if (!shadow.empty()) {
						uint64_t comp = shadow.front();
						shadow.pop_front();
						if (comp != address){
							report_error(hart_id, comp, address);
							// return -1;
						} else {
							if (GC_DEBUG) {
								report_pair(hart_id, comp, address);
							}
						}
					}
				}
			}

			ROCC_INSTRUCTION_D (1, Payload1, 0x0D);
			ROCC_INSTRUCTION_D (1, Payload2, 0x0D);
			type = Payload1 & 0x03;
			address = Payload1 >> 2;
			if (type == 1) { // Push -- a function is called
				shadow.push_front(address);
				if (GC_DEBUG) {
					report_push (hart_id, address);
				}
			} else { // Pop -- a function is called
				if (type == 2) {
					if (!shadow.empty()) {
						uint64_t comp = shadow.front();
						shadow.pop_front();
						if (comp != address){
							report_error(hart_id, comp, address);
							// return -1;
						} else {
							if (GC_DEBUG) {
								report_pair(hart_id, comp, address);
							}
						}
					}
				}
			}

			type = Payload2 & 0x03;
			address = Payload2 >> 2;
			if (type == 1) { // Push -- a function is called
				shadow.push_front(address);
				if (GC_DEBUG) {
					report_push (hart_id, address);
				}
			} else { // Pop -- a function is called
				if (type == 2) {
					if (!shadow.empty()) {
						uint64_t comp = shadow.front();
						shadow.pop_front();
						if (comp != address){
							report_error(hart_id, comp, address);
							// return -1;
						} else {
							if (GC_DEBUG) {
								report_pair(hart_id, comp, address);
							}
						}
					}
				}
			}
			buffer_depth = ghe_get_bufferdepth();
		}

		if (buffer_depth > 0){
			switch (buffer_depth){				
				case 7:
				ROCC_INSTRUCTION_D (1, Payload, 0x0D);
				type = Payload & 0x03;
				address = Payload >> 2;
				if (type == 1) { // Push -- a function is called
					shadow.push_front(address);
					if (GC_DEBUG) {
						report_push (hart_id, address);
					}
				} else { // Pop -- a function is called
					if (type == 2) {
						if (!shadow.empty()) {
							uint64_t comp = shadow.front();
							shadow.pop_front();
							if (comp != address){
								report_error(hart_id, comp, address);
								// return -1;
							} else {
								if (GC_DEBUG) {
									report_pair(hart_id, comp, address);
								}
							}
						}
					}
				}

				case 6:
				ROCC_INSTRUCTION_D (1, Payload, 0x0D);
				type = Payload & 0x03;
				address = Payload >> 2;
				if (type == 1) { // Push -- a function is called
					shadow.push_front(address);
					if (GC_DEBUG) {
						report_push (hart_id, address);
					}
				} else { // Pop -- a function is called
					if (type == 2) {
						if (!shadow.empty()) {
							uint64_t comp = shadow.front();
							shadow.pop_front();
							if (comp != address){
								report_error(hart_id, comp, address);
								// return -1;
							} else {
								if (GC_DEBUG) {
									report_pair(hart_id, comp, address);
								}
							}
						}
					}
				}

				case 5:
				ROCC_INSTRUCTION_D (1, Payload, 0x0D);
				type = Payload & 0x03;
				address = Payload >> 2;
				if (type == 1) { // Push -- a function is called
					shadow.push_front(address);
					if (GC_DEBUG) {
						report_push (hart_id, address);
					}
				} else { // Pop -- a function is called
					if (type == 2) {
						if (!shadow.empty()) {
							uint64_t comp = shadow.front();
							shadow.pop_front();
							if (comp != address){
								report_error(hart_id, comp, address);
								// return -1;
							} else {
								if (GC_DEBUG) {
									report_pair(hart_id, comp, address);
								}
							}
						}
					}
				}

				case 4:
				ROCC_INSTRUCTION_D (1, Payload, 0x0D);
				type = Payload & 0x03;
				address = Payload >> 2;
				if (type == 1) { // Push -- a function is called
					shadow.push_front(address);
					if (GC_DEBUG) {
						report_push (hart_id, address);
					}
				} else { // Pop -- a function is called
					if (type == 2) {
						if (!shadow.empty()) {
							uint64_t comp = shadow.front();
							shadow.pop_front();
							if (comp != address){
								report_error(hart_id, comp, address);
								// return -1;
							} else {
								if (GC_DEBUG) {
									report_pair(hart_id, comp, address);
								}
							}
						}
					}
				}

				case 3:
				ROCC_INSTRUCTION_D (1, Payload, 0x0D);
				type = Payload & 0x03;
				address = Payload >> 2;
				if (type == 1) { // Push -- a function is called
					shadow.push_front(address);
					if (GC_DEBUG) {
						report_push (hart_id, address);
					}
				} else { // Pop -- a function is called
					if (type == 2) {
						if (!shadow.empty()) {
							uint64_t comp = shadow.front();
							shadow.pop_front();
							if (comp != address){
								report_error(hart_id, comp, address);
								// return -1;
							} else {
								if (GC_DEBUG) {
									report_pair(hart_id, comp, address);
								}
							}
						}
					}
				}

				case 2:
				ROCC_INSTRUCTION_D (1, Payload, 0x0D);
				type = Payload & 0x03;
				address = Payload >> 2;
				if (type == 1) { // Push -- a function is called
					shadow.push_front(address);
					if (GC_DEBUG) {
						report_push (hart_id, address);
					}
				} else { // Pop -- a function is called
					if (type == 2) {
						if (!shadow.empty()) {
							uint64_t comp = shadow.front();
							shadow.pop_front();
							if (comp != address){
								report_error(hart_id, comp, address);
								// return -1;
							} else {
								if (GC_DEBUG) {
									report_pair(hart_id, comp, address);
								}
							}
						}
					}
				}

				case 1:
				ROCC_INSTRUCTION_D (1, Payload, 0x0D);
				type = Payload & 0x03;
				address = Payload >> 2;
				if (type == 1) { // Push -- a function is called
					shadow.push_front(address);
					if (GC_DEBUG) {
						report_push (hart_id, address);
					}
				} else { // Pop -- a function is called
					if (type == 2) {
						if (!shadow.empty()) {
							uint64_t comp = shadow.front();
							shadow.pop_front();
							if (comp != address){
								report_error(hart_id, comp, address);
								// return -1;
							} else {
								if (GC_DEBUG) {
									report_pair(hart_id, comp, address);
								}
							}
						}
					}
				}
				break;

				default:
				printf("Error: duff's device");
				exit(0);
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