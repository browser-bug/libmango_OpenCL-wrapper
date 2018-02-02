#include "mango_hn_nuplus.h"

mango_context_t ctx;

mango_exit_t mango_nuplus_init(char **argv){
	
        //ctx.event_exit.vaddr=(uint32_t *)strtol(argv[1], NULL, 16);
	return SUCCESS;
}

void mango_nuplus_close(int status){

        mango_nuplus_write_synchronization(&ctx.event_exit, 1);

        //exit(status);
}


void mango_nuplus_write_synchronization(mango_event_t *e, uint32_t value){
	
        *(e->vaddr) = value;
	__builtin_nuplus_flush((int)(&(e->vaddr)));
}

uint32_t mango_nuplus_read_synchronization(mango_event_t *e){

        return *(e->vaddr);
}

mango_event_t *mango_nuplus_spawn(void *(*task)(task_args *), uint32_t range){
	//TODO: not implemented
	return NULL;
}

void mango_nuplus_task_arg_free(task_args *a){
	//TODO: not implemented
}


void mango_nuplus_suspend(){
	//TODO: not implemented
}


uint32_t* mango_memory_map(uint64_t a) {
	return a;
}
