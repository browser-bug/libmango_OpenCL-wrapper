#include "mango_hn.h"
#include "mango_hn_gn.h"
#include "mango_hn_peak.h"
#include "debug.h"

#ifndef ARCH
#define ARCH(t) mango_gn_ ## t
#endif /* ARCH */

mango_exit_t mango_init(char **argv){
	return ARCH(init)(argv);
}

void mango_close(int status){
	ARCH(close)(status);
}


void mango_write_synchronization(mango_event_t *e, uint32_t value){
	ARCH(write_synchronization)(e,value);
}

uint32_t mango_read_synchronization(mango_event_t *e){
	return ARCH(read_synchronization)(e);
}

void *mango_exit(task_args *a){
	//dprint("Thread %d initiating exit on event %d \n", tid(a), a->event->vaddr);
	uint32_t v=mango_lock(a->event);
	//dprint("Thread %d unlocking event %d with %d to %d\n", tid(a), a->event->vaddr, v, v-1);
	mango_write_synchronization(a->event,v-1);
	ARCH(task_arg_free)(a);
	return NULL;
}


mango_event_t *mango_spawn(void *(*task)(task_args *), uint32_t range){
	return ARCH(spawn)(task,range);
}

uint32_t mango_lock(mango_event_t *e){
	uint32_t value=0;
	//dprint("Acquiring lock on event %p\n", e->vaddr);
	do {
		value = mango_read_synchronization(e);
	} while (value==0);
	return value;
}



void mango_wait(mango_event_t *e, uint32_t state){
	uint32_t value=0;
	do {
		value=mango_lock(e);
		//dprint("Releasing event %p (value %d)\n", e->vaddr, value); 
		mango_write_synchronization(e,value);
		//dprint("Checking event %p\n", e->vaddr);
		if (value!=state){ 
			ARCH(suspend)();
		}
	}	while(value!=state);
}

void mango_join(mango_event_t *e){
	mango_wait(e, READ);
	dprint("Closing down the workgroup\n");
}

uint32_t mango_signal(mango_event_t *e){
	uint32_t value=mango_lock(e);
	//dprint("Signal event %d %d->%d\n", e->vaddr, value, value-1);
	mango_write_synchronization(e,value-1);
	return value-1;
}

void mango_barrier(task_args *args, mango_event_t *fifo){
	uint32_t value, go, count;
	mango_event_t *e=args->barrier;
	dprint("Barrier %p reached by Thread %d!\n", e->vaddr, tid(args));
	go=mango_lock(args->release);
	mango_write_synchronization(args->release,go);
	go=(go==1?2:1);
	count=mango_signal(e);
	if (count>1) {	
		dprint("Thread %d waiting restart\n", tid(args));
		mango_wait(args->release,go);	
	} else {
		dprint("Barrier reached by all Threads!\n");
		if (fifo) { 
			dprint("Acquiring fifo lock on event %p\n", fifo->vaddr);
			value=mango_lock(fifo);
			dprint("Acquired lock on fifo event %p, value %d\n", fifo->vaddr, value);
			if (value!=READ){
				mango_write_synchronization(fifo, WRITE);
				dprint("Waiting for fifo on event %p, value set at %d, waiting for %d\n", fifo->vaddr, WRITE, READ);
				mango_wait(fifo, READ);
				mango_lock(fifo);
			}
			mango_write_synchronization(fifo,3);
		}
		dprint("FIFO wait complete\n");
		mango_lock(e);
		mango_write_synchronization(e,args->ntasks+1);
		dprint("Barrier reset on event %d\n", e->vaddr);
		mango_lock(args->release);
		mango_write_synchronization(args->release,go);
		dprint("FIFO ready, threads released\n");
	}
}

