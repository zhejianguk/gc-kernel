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

int main (void)
{
	uint64_t hart_id = (uint64_t) CHK1_ID;
	// GC variables
	uint64_t Header,Payload, PC, Inst, S_Header, S_Payload;
	dequeue shadow;

    //================== Initialisation ==================//
    // Bound current thread to hart_id
    if (gc_pthread_setaffinity(hart_id) != 0) {
		printf ("[Rocket-C%x]: pthread_setaffinity failed.", hart_id);
	}
	
	initialize(&shadow);
	ghe_initailised();
	//===================== Execution =====================//
	while (ghe_checkght_status() != 0x02){
		while (ghe_status() != GHE_EMPTY){
			ROCC_INSTRUCTION_D (1, Payload, 0x0D);
			uint64_t type = Payload & 0x03;

			if (type == 1) {
				enqueueF(&shadow, Payload);
			} else if (type == 2) {
				if (empty(&shadow) != 1) {
					uint64_t comp = dequeueF(&shadow) + 1;
					if (comp != Payload){
						printf("\r\n [Rocket-C%x-SS]: **Error** Exp:%x v.s. Pul:%x! \r\n", hart_id, comp>>2, Payload>>2);
					}
				}
			}
		}

		// If all push and pull are handled
   		if ((ghe_sch_status() == 0x01) && (ghe_status() == GHE_EMPTY)) {
			// Send unpaired pushes
			while ((empty(&shadow) == 0)) {
				dequeueR(&shadow);
			}
		
			// Send termination flag
			while (ghe_agg_status() == GHE_FULL) {
			}
      		ghe_agg_push (hart_id, 0xFFFFFFFF);

			while ((ghe_sch_status() == 0x01) && (ghe_status() == GHE_EMPTY)) {
				// Wait the core to be waked up
			}
      	}
	}
	//=================== Post execution ===================//
	ghe_deinitailised();
	ghe_release();

	return 0;
}
