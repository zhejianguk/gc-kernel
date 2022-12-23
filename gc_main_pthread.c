#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)
#include "libraries/spin_lock.h"
#include "libraries/ght.h"
#include "libraries/ghe.h"
#include "libraries/gc_top.h"

int uart_lock;
int if_tasks_initalised[NUM_CORES] = {0};
int agg_core_id = 0x07;
pthread_t threads[NUM_CORES];

  void* thread_pmc(void* args){
    uint64_t hart_id = (uint64_t) args;
    //================== Initialisation ==================//
	// GC variables
	uint64_t perfc = 0;
	uint64_t Payload = 0x0;
	uint64_t ecounter;

	if (gc_pthread_setaffinity(hart_id) != 0){
		lock_acquire(&uart_lock);
		printf ("[Rocket-C%x]: pthread_setaffinity failed.", hart_id);
		lock_release(&uart_lock);
	} else{
		if_tasks_initalised[hart_id] = 1;
	}

    while (and_gate(if_tasks_initalised, NUM_CORES) == 1){
	}

	//===================== Execution =====================// 
	// Nothing to do here.
	//=================== Post execution ===================//
	lock_acquire(&uart_lock);
	printf("[Rocket-C%x]: Completed! \r\n", hart_id);
	lock_release(&uart_lock);

	return NULL;
}

/* implementation of myStartupFun */
void gcStartup (void)
{
    //================== Initialisation ==================//
    // Bound current thread to BOOM
    if (gc_pthread_setaffinity(BOOM_ID) != 0){
		lock_acquire(&uart_lock);
		printf ("[Boom-C%x]: pthread_setaffinity failed.", BOOM_ID);
		lock_release(&uart_lock);
	} else{
        if_tasks_initalised[BOOM_ID] = 1;
	}

    // GC threads
    for (uint64_t i = 1; i < NUM_CORES; i++) {
		pthread_create(&threads[i], NULL, thread_pmc, (void *) i);
	}

    while (and_gate(if_tasks_initalised, NUM_CORES) == 0){
	}
    
    /*=====================*/
    /*  GC configurations  */
    /*=====================*/
    lock_acquire(&uart_lock);
	printf("[Boom-%x]: Test is now started: \r\n", BOOM_ID);
	lock_release(&uart_lock);
    //===================== Execution =====================//
}
  
void gcCleanup (void)
{
    //=================== Post execution ===================//
    uint64_t status;

    // GC threads.
	for (uint64_t i = 1; i < NUM_CORES; i++) {
		pthread_join(threads[i], NULL);
	}

   	lock_acquire(&uart_lock);
	printf("[Boom-%x]: All tests are done! Status: %x. \r\n", BOOM_ID, status);
	lock_release(&uart_lock);
}
  