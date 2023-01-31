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
#include "malloc.h"
#include "sys/mman.h"


void gcStartup (void) __attribute__ ((constructor));
void gcCleanup (void) __attribute__ ((destructor));

pthread_t threads[NUM_CORES-1];
char* shadow;

void* thread_sanitiser_gc(void* args){
    uint64_t hart_id = (uint64_t) args;
    //================== Initialisation ==================//
	// GC variables
	uint64_t Address = 0x0;
  	uint64_t Err_Cnt = 0x0;
	uint64_t Header = 0x0;
	uint64_t PC = 0x0;
	uint64_t Inst = 0x0;

	if (gc_pthread_setaffinity(hart_id) != 0){
		printf ("[Rocket-C%x-PMC]: pthread_setaffinity failed.", hart_id);
	} else {
		ghe_go();
		ghe_initailised(1);
	}

	//===================== Execution =====================// 
	while (ghe_checkght_status() != 0x02){
		while (ghe_status() != GHE_EMPTY){
			// ROCC_INSTRUCTION_D (1, Header, 0x0A);
			// ROCC_INSTRUCTION_D (1, Address, 0x0D);
			ROCC_INSTRUCTION_D (1, Address, 0x0C);
			ROCC_INSTRUCTION_D (1, Header, 0x0B);

			char bits = shadow[(Address)>>7];

			if (bits != 0){
				if(bits & (1<<((Address >> 7)&8))) {
					printf("[Rocket-C%x-Sani]: **Error** illegal accesses at %lx, PC: %x. \r\n", hart_id, Address, Header);
				}
			} else {
				// no nothing
			}
		}

		// Dedicated for shadowstack 
		if (ghe_checkght_status() == 0x04) {
			ghe_complete();
			while((ghe_checkght_status() == 0x04)) {
				// Wait big core to re-start
			}
			ghe_go();
		}
	}

	//=================== Post execution ===================//
	ghe_initailised(0);
	ghe_release();

	if (hart_id != 0){ // Invalid condition, making sure the Err_Cnt is not optimised.
		printf("Rocket-C%x-Sani]: Completed, %d illegal accesses are detected. \r\n", hart_id);
	}

	return NULL; 
}

void gcStartup (void)
{
    //================== Initialisation ==================//
    // Bound current thread to BOOM
    if (gc_pthread_setaffinity(BOOM_ID) != 0) {
		printf ("[Boom-C%x]: pthread_setaffinity failed.", BOOM_ID);
	}

	uint64_t map_size = (long long) 4*1024*1024*1024*sizeof(char);
	// shadow memory
	shadow = mmap(NULL,
			map_size,
			PROT_WRITE | PROT_READ,
			MAP_PRIVATE | MAP_ANON | MAP_NORESERVE,
			-1, 0);

	if(shadow == NULL) {
		printf("[Boom-C%x]: Error! memory is not allocated.", BOOM_ID);
		exit(0);
	}
	
	// asm volatile("fence rw, rw;");

	// GC threads
    for (uint64_t i = 0; i < NUM_CORES - 1; i++) {
		pthread_create(&threads[i], NULL, thread_sanitiser_gc, (void *) (i+1));
	}

	while (ght_get_initialisation() == 0){
 	}
	
	ght_set_satp_priv();

	printf("[Boom-%x]: Test is now started: \r\n", BOOM_ID);
	ght_set_status (0x01); // ght: start
    //===================== Execution =====================//
}
  
void gcCleanup (void)
{	
	//=================== Post execution ===================//
	uint64_t status;
    ght_set_status (0x02);

	while (((status = ght_get_status()) < 0x1FFFF) || (ght_get_buffer_status() != GHT_EMPTY)){
	}

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

