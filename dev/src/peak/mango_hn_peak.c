#include "mango_hn_peak.h"
#include "debug.h"

#include <stdlib.h>

mango_context_t ctx;

uint32_t* mango_memory_map(uint64_t a) {
	return a;
}


mango_exit_t mango_peak_init(char **argv){

        printf("mango_init (peak)\n");

        ctx.event_exit.vaddr=(uint32_t *)strtol(argv[1], NULL, 16);
        ctx.event_a.vaddr=(uint32_t *)strtol(argv[2], NULL, 16);
        ctx.event_b.vaddr=(uint32_t *)strtol(argv[3], NULL, 16);
        ctx.event_r.vaddr=(uint32_t *)strtol(argv[4], NULL, 16);

        printf("Return event address: %p\n", ctx.event_exit.vaddr);
        printf("Task event address: %p\n", ctx.event_a.vaddr);
        printf("Barrier event address: %p\n", ctx.event_b.vaddr);
        printf("Release event address: %p\n", ctx.event_r.vaddr);

        return SUCCESS;
}

void mango_peak_close(int status){
        
        printf("mango close (peak)\n");

        mango_peak_write_synchronization(&ctx.event_exit, 1);

        exit(status);
}


void mango_peak_write_synchronization(mango_event_t *e, uint32_t value){

        //printf("PEAK - mango_write_synchronization %p <- %lu\n", e->vaddr, value);
        *(e->vaddr) = value;
}

uint32_t mango_peak_read_synchronization(mango_event_t *e){

        //printf("PEAK - mango_peak_read_synchronization %p\n", e->vaddr);
        return *(e->vaddr);
}


void mango_peak_suspend(){
        //printf("mango_peak_suspend not implemented\n");
}

mango_event_t *mango_peak_spawn(void *(*task)(task_args *), uint32_t range){
        //printf("mango_peak_spawn not implemented\n");
	return NULL;
}

void mango_peak_task_arg_free(task_args *a){
        printf("mango_peak_task_arg_free not implemented\n");

}
