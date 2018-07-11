#include "mango_hn.h"
#include "mango_hn_nuplus.h"
//#include "debug.h"

#ifndef ARCH
#define ARCH(t) mango_nuplus_##t
#endif /* ARCH */

mango_exit_t mango_init(char **argv)
{
	return ARCH(init)(argv);
}

void mango_close(int status)
{
	ARCH(close)
	(status);
}

void mango_write_synchronization(mango_event_t *e, uint32_t value)
{
	ARCH(write_synchronization)
	(e, value);
}

uint32_t mango_read_synchronization(mango_event_t *e)
{
	return ARCH(read_synchronization)(e);
}

void *mango_exit(task_args *a)
{
	uint32_t v = mango_lock(a->event);
	mango_write_synchronization(a->event, v - 1);
	ARCH(task_arg_free)
	(a);
	return NULL;
}

mango_event_t *mango_spawn(void *(*task)(task_args *), uint32_t range)
{
	return ARCH(spawn)(task, range);
}

uint32_t mango_lock(mango_event_t *e)
{
	uint32_t value = 0;

	do{
		value = mango_read_synchronization(e);
	} while (value == 0);
	return value;
}

void mango_wait(mango_event_t *e, uint32_t state)
{
	uint32_t value = 0;
	uint32_t reg = state;

	do{
		value = mango_lock(e);

		if (value != reg){
			mango_write_synchronization(e, value);
		}
	} while (value != reg);
}

void mango_join(mango_event_t *e)
{
	mango_wait(e, READ);
}

uint32_t mango_signal(mango_event_t *e)
{
	uint32_t value = mango_lock(e);
	mango_write_synchronization(e, value - 1);

	return value - 1;
}

void mango_barrier(task_args *args, mango_event_t *fifo)
{
	uint32_t value, go, count;
	mango_event_t *e = args->barrier;
	go = mango_lock(args->release);
	mango_write_synchronization(args->release, go);
	go = (go == 1 ? 2 : 1);
	count = mango_signal(e);
	if (count > 1){
		mango_wait(args->release, go);
	}
	else{
		if (fifo){
			value = mango_lock(fifo);
			if (value != READ){
				mango_write_synchronization(fifo, WRITE);
				mango_wait(fifo, READ);
				mango_lock(fifo);
			}
			mango_write_synchronization(fifo, 3);
		}
		mango_lock(e);
		mango_write_synchronization(e, args->ntasks + 1);
		mango_lock(args->release);
		mango_write_synchronization(args->release, go);
	}
}
