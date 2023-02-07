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

void* thread_pmc_gc(void* args){
    uint64_t hart_id = (uint64_t) args;
    //================== Initialisation ==================//
	// GC variables
	uint64_t base = 0;
	uint64_t end = 0x10000000000L;
	uint64_t perfc = 0;
	uint64_t addr;
	uint64_t addr1;
	uint64_t addr2;
	uint64_t addr3;
	uint64_t addr4;
	uint64_t addr5;
	uint64_t addr6;
	uint64_t addr7;
	uint64_t addr8;

	if (gc_pthread_setaffinity(hart_id) != 0){
		printf ("[Rocket-C%x-PMC]: pthread_setaffinity failed.", hart_id);
	}

	ghe_initailised(1);

	//===================== Execution =====================// 
	while (ghe_checkght_status() != 0x02){
		uint32_t buffer_depth = ghe_get_bufferdepth();
		while (buffer_depth > 7) {
			ROCC_INSTRUCTION_D (1, addr1, 0x0D);
			ROCC_INSTRUCTION_D (1, addr2, 0x0D);
			perfc += (addr1 > base && addr1 < end) ? 1 : 0;
			perfc += (addr2 > base && addr2 < end) ? 1 : 0;

			ROCC_INSTRUCTION_D (1, addr3, 0x0D);
			ROCC_INSTRUCTION_D (1, addr4, 0x0D);
			perfc += (addr3 > base && addr3 < end) ? 1 : 0;
			perfc += (addr4 > base && addr4 < end) ? 1 : 0;

			ROCC_INSTRUCTION_D (1, addr5, 0x0D);
			ROCC_INSTRUCTION_D (1, addr6, 0x0D);
			perfc += (addr5 > base && addr5 < end) ? 1 : 0;
			perfc += (addr6 > base && addr6 < end) ? 1 : 0;

			ROCC_INSTRUCTION_D (1, addr7, 0x0D);
			ROCC_INSTRUCTION_D (1, addr8, 0x0D);
			perfc += (addr7 > base && addr7 < end) ? 1 : 0;
			perfc += (addr8 > base && addr8 < end) ? 1 : 0;

			buffer_depth = ghe_get_bufferdepth();
		}

		if (buffer_depth > 0) {
			switch (buffer_depth){
				case 7: 
					ROCC_INSTRUCTION_D (1, addr, 0x0D);
					perfc += (addr > base && addr < end) ? 1 : 0;
				case 6: 
					ROCC_INSTRUCTION_D (1, addr, 0x0D);
					perfc += (addr > base && addr < end) ? 1 : 0;
				case 5: 
					ROCC_INSTRUCTION_D (1, addr, 0x0D);
					perfc += (addr > base && addr < end) ? 1 : 0;
				case 4: 
					ROCC_INSTRUCTION_D (1, addr, 0x0D);
					perfc += (addr > base && addr < end) ? 1 : 0;
				case 3: 
					ROCC_INSTRUCTION_D (1, addr, 0x0D);
					perfc += (addr > base && addr < end) ? 1 : 0;
				case 2: 
					ROCC_INSTRUCTION_D (1, addr, 0x0D);
					perfc += (addr > base && addr < end) ? 1 : 0;
				case 1: 
					ROCC_INSTRUCTION_D (1, addr, 0x0D);
					perfc += (addr > base && addr < end) ? 1 : 0;
					break;
				default:
					printf("[Rocket-C%x-PMC]: error: debuffs'device! \r\n" , hart_id);
			}
		}
	}
	//=================== Post execution ===================//
	if (hart_id == 0){ // Invalid condition, making sure the perfc is not optimised.
		printf("[Rocket-C%x-PMC]: Completed, PMC = %lx! \r\n", hart_id, perfc);
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
	}

    // GC threads
    for (uint64_t i = 0; i < NUM_CORES - 1; i++) {
		pthread_create(&threads[i], NULL, thread_pmc_gc, (void *) (i+1));
	}

	while (ght_get_initialisation() == 0){
 	}
	
	ght_set_satp_priv();
	printf("[Boom-%x]: Test is now started: \r\n", BOOM_ID);

 	// struct sched_param fifo_param;
  	// fifo_param.sched_priority=99;
 	// int s = pthread_setschedparam(pthread_self(), SCHED_FIFO, &fifo_param);
   	// if (s != 0) {
	// 	printf("Error: Scheduling policy! %x. \r\n", s);
   	// }

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
	
	if (GC_DEBUG == 1){
		printf("[Boom-%x]: Test is now completed: \r\n", BOOM_ID);
	}

	ght_unset_satp_priv();
	ght_set_status (0x00);
}

