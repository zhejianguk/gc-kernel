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
#include "malloc.h"
#include "sys/mman.h"


void gcStartup (void) __attribute__ ((constructor));
void gcCleanup (void) __attribute__ ((destructor));

pthread_t threads[NUM_CORES-1];
char* shadow;
uint64_t map_size = (long long) 4*1024*1024*1024*sizeof(char);

void report_error(uint64_t offset){
	int PC;
	if (offset==0) {
		PC = ghe_get_fifocache0();
	} else {
		PC = ghe_get_fifocache1();
	}
	printf("**Error** accesses at PC: %x.\r\n", PC);
}


void* thread_sanitiser_gc(void* args){
    uint64_t hart_id = (uint64_t) args;
	// GC variables
	char bits = 0;
	char bits1 = 0;
	char bits2 = 0;

	uint64_t Address = 0x0;
	uint64_t Header = 0x0;
	uint64_t Address1 = 0x0;
	uint64_t Header1 = 0x0;
	uint64_t Address2 = 0x0;
	uint64_t Header2 = 0x0;

	//================== Initialisation ==================//
	if (gc_pthread_setaffinity(hart_id) != 0){
		printf ("[Rocket-C%x-Sani]: pthread_setaffinity failed.", hart_id);
	}
	
	ghe_go();
	ghe_initailised();

	//===================== Execution =====================//
	while (ghe_checkght_status() != 0x02){
		// Run analysis
		uint32_t buffer_depth = ghe_get_bufferdepth();
		while (buffer_depth > 9){
			ROCC_INSTRUCTION_D (1, Address1, 0x0D);
			ROCC_INSTRUCTION_D (1, Address2, 0x0D);
			bits1 = shadow[(Address1)>>7];
			if (bits1 != 0){
				if(bits1 & (1<<((Address1 >> 7)&8))) {
					report_error(1);
				}
			}
			bits2 = shadow[(Address2)>>7];
			if (bits2 != 0){
				if(bits2 & (1<<((Address2 >> 7)&8))) {
					report_error(0);
				}
			}

			ROCC_INSTRUCTION_D (1, Address1, 0x0D);
			ROCC_INSTRUCTION_D (1, Address2, 0x0D);
			bits1 = shadow[(Address1)>>7];
			if (bits1 != 0){
				if(bits1 & (1<<((Address1 >> 7)&8))) {
					report_error(1);
				}
			}
			bits2 = shadow[(Address2)>>7];
			if (bits2 != 0){
				if(bits2 & (1<<((Address2 >> 7)&8))) {
					report_error(0);
				}
			}

			ROCC_INSTRUCTION_D (1, Address1, 0x0D);
			ROCC_INSTRUCTION_D (1, Address2, 0x0D);
			bits1 = shadow[(Address1)>>7];
			if (bits1 != 0){
				if(bits1 & (1<<((Address1 >> 7)&8))) {
					report_error(1);
				}
			}
			bits2 = shadow[(Address2)>>7];
			if (bits2 != 0){
				if(bits2 & (1<<((Address2 >> 7)&8))) {
					report_error(0);
				}
			}
			
			ROCC_INSTRUCTION_D (1, Address1, 0x0D);
			ROCC_INSTRUCTION_D (1, Address2, 0x0D);
			bits1 = shadow[(Address1)>>7];
			if (bits1 != 0){
				if(bits1 & (1<<((Address1 >> 7)&8))) {
					report_error(1);
				}
			}
			bits2 = shadow[(Address2)>>7];
			if (bits2 != 0){
				if(bits2 & (1<<((Address2 >> 7)&8))) {
					report_error(0);
				}
			}
			
			ROCC_INSTRUCTION_D (1, Address1, 0x0D);
			ROCC_INSTRUCTION_D (1, Address2, 0x0D);
			bits1 = shadow[(Address1)>>7];
			if (bits1 != 0){
				if(bits1 & (1<<((Address1 >> 7)&8))) {
					report_error(1);
				}
			}
			bits2 = shadow[(Address2)>>7];
			if (bits2 != 0){
				if(bits2 & (1<<((Address2 >> 7)&8))) {
					report_error(0);
				}
			}
			buffer_depth = ghe_get_bufferdepth();
		}

		if (buffer_depth > 0){
			switch (buffer_depth){
				case 9:
					ROCC_INSTRUCTION_D (1, Address, 0x0D); 
					bits = shadow[(Address)>>7];
					if (bits != 0){
						if(bits & (1<<((Address >> 7)&8))) {
							report_error(0);
						}
					}

				case 8:
					ROCC_INSTRUCTION_D (1, Address, 0x0D); 
					bits = shadow[(Address)>>7];
					if (bits != 0){
						if(bits & (1<<((Address >> 7)&8))) {
							report_error(0);
						}
					}

				case 7:
					ROCC_INSTRUCTION_D (1, Address, 0x0D); 
					bits = shadow[(Address)>>7];
					if (bits != 0){
						if(bits & (1<<((Address >> 7)&8))) {
							report_error(0);
						}
					}

				case 6: 
					ROCC_INSTRUCTION_D (1, Address, 0x0D); 
					bits = shadow[(Address)>>7];
					if (bits != 0){
						if(bits & (1<<((Address >> 7)&8))) {
							report_error(0);
						}
					}

				case 5: 
					ROCC_INSTRUCTION_D (1, Address, 0x0D); 
					bits = shadow[(Address)>>7];
					if (bits != 0){
						if(bits & (1<<((Address >> 7)&8))) {
							report_error(0);
						}
					}

				case 4: 
					ROCC_INSTRUCTION_D (1, Address, 0x0D);
					bits = shadow[(Address)>>7];
					if (bits != 0){
						if(bits & (1<<((Address >> 7)&8))) {
							report_error(0);
						}
					}

				case 3: 
					ROCC_INSTRUCTION_D (1, Address, 0x0D); 
					bits = shadow[(Address)>>7];
					if (bits != 0){
						if(bits & (1<<((Address >> 7)&8))) {
							report_error(0);
						}
					}

				case 2: 
					ROCC_INSTRUCTION_D (1, Address, 0x0D); 
					bits = shadow[(Address)>>7];
					if (bits != 0){
						if(bits & (1<<((Address >> 7)&8))) {
							report_error(0);
						}
					}

				case 1: 
					ROCC_INSTRUCTION_D (1, Address, 0x0D); 
					bits = shadow[(Address)>>7];
					if (bits != 0){
						if(bits & (1<<((Address >> 7)&8))) {
							report_error(0);
						}
					}
					break;

				default:
					printf("Error: duff's device");
					exit(0);
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

	return NULL; 
}

void gcStartup (void)
{
    //================== Initialisation ==================//
    // Bound current thread to BOOM
    if (gc_pthread_setaffinity(BOOM_ID) != 0) {
		printf ("[BOOM-C%x]: pthread_setaffinity failed.", BOOM_ID);
	}
	ght_set_satp_priv();

	// shadow memory
	shadow = mmap(NULL,
				  map_size,
				  PROT_WRITE | PROT_READ,
				  MAP_PRIVATE | MAP_ANON | MAP_NORESERVE,
				  -1, 0);
	asm volatile("fence rw, rw;");

	if(shadow == NULL) {
		printf("[BOOM-C%x]: Error! memory is not allocated.", BOOM_ID);
	}
	// GC threads
    for (uint64_t i = 0; i < NUM_CORES - 1; i++) {
		pthread_create(&threads[i], NULL, thread_sanitiser_gc, (void *) (i+1));
	}

	while (ght_get_initialisation() == 0){
 	}
	asm volatile("fence rw, rw;");

	printf("[Boom-C-%x]: Test is now started: \r\n", BOOM_ID);
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
	
	int s = munmap(shadow, map_size);

	if(s != 0) {
		printf("[BOOM-C%x]: Error! memory is not allocated.", BOOM_ID);
	}

	if (GC_DEBUG == 1){
		printf("[BOOM-C-%x]: Test is now completed! \r\n", BOOM_ID);
	}

	ght_unset_satp_priv();
	ght_set_status_00 ();
}