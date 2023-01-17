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
	}

	ghe_initailised(1);

	//===================== Execution =====================// 
	while (ghe_checkght_status() != 0x02){
		uint32_t buffer_depth = ghe_get_bufferdepth();
		while (buffer_depth > 7) {
			ROCC_INSTRUCTION (1, 0x0D);
			ROCC_INSTRUCTION (1, 0x0D);
			ROCC_INSTRUCTION (1, 0x0D);
			ROCC_INSTRUCTION (1, 0x0D);
			ROCC_INSTRUCTION (1, 0x0D);
			ROCC_INSTRUCTION (1, 0x0D);
			ROCC_INSTRUCTION (1, 0x0D);
			ROCC_INSTRUCTION (1, 0x0D);
			perfc = perfc + 8;
			buffer_depth = ghe_get_bufferdepth();
		}

		if (buffer_depth > 0) {
		switch (buffer_depth){
			case 7: 
				ROCC_INSTRUCTION (1, 0x0D);
			case 6: 
				ROCC_INSTRUCTION (1, 0x0D);
			case 5: 
				ROCC_INSTRUCTION (1, 0x0D);
			case 4: 
				ROCC_INSTRUCTION (1, 0x0D);
			case 3: 
				ROCC_INSTRUCTION (1, 0x0D);
			case 2: 
				ROCC_INSTRUCTION (1, 0x0D);
			case 1: 
				ROCC_INSTRUCTION (1, 0x0D);
				perfc = perfc + buffer_depth;
			}
		}
	}

	//=================== Post execution ===================//
	if (GC_DEBUG == 1){
		printf("[Rocket-C%x-PMC]: Completed, PMC = %lx! \r\n", hart_id, perfc);
  	  	uint64_t e_counter = debug_ecounter();
		uint64_t remaining = ghe_get_bufferdepth();
		uint64_t f_counter = ghe_get_fifocounter();
		uint64_t df_counter = ghe_get_fifodcounter();
  	  	printf("[Rocket-C%x-PMC]: Debug, i-counter: %lx; remaining %x messages. \r\n", hart_id, e_counter, remaining);
		printf("[Rocket-C%x-PMC]: Debug, f-counter: %lx; df-counter: %lx. \r\n", hart_id, f_counter, df_counter);
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

	ght_set_satp_priv();

    // GC threads
    for (uint64_t i = 0; i < NUM_CORES - 1; i++) {
		pthread_create(&threads[i], NULL, thread_pmc, (void *) (i+1));
	}
	
	if (GC_DEBUG == 1){
		printf("[Boom-%x]: All Initalised. \r\n", BOOM_ID);
	}

	while (ght_get_initialisation() == 0){
 	}

	printf("[Boom-%x]: Test is now started: \r\n", BOOM_ID);
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
		
		// Below code is used to check the message counters -- ensuring no missing message
		uint64_t m_counter = debug_mcounter();
  	  	uint64_t i_counter = debug_icounter();
  	  	uint64_t g_counter = debug_gcounter();

  	  	printf("Debug, m-counter: %lx \r\n", m_counter);
  		printf("Debug, i-counter: %lx \r\n", i_counter);
  		printf("Debug, g-counter: %lx \r\n", g_counter);
	}

	ght_unset_satp_priv();
	ght_set_status (0x00);
}

