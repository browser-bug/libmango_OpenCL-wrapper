#include "mango_hn_nuplus.h"

mango_context_t ctx;

mango_exit_t mango_nuplus_init(char **argv){

	ctx.event_exit.vaddr = (uint32_t *) argv[1]; 
        ctx.event_a.vaddr = (uint32_t *) argv[2]; 
        ctx.event_b.vaddr = (uint32_t *) argv[3]; 
        ctx.event_r.vaddr = (uint32_t *) argv[4]; 

	return SUCCESS;
}

void mango_nuplus_close(int status){
    
        mango_nuplus_write_synchronization(&ctx.event_exit, 1);
        //exit(status);
}


void mango_nuplus_write_synchronization(mango_event_t *e, uint32_t value){
        uint32_t reg = value;
        *(e->vaddr) = reg;
}

uint32_t mango_nuplus_read_synchronization(mango_event_t *e){
        uint32_t reg = *(e->vaddr);
        return reg;
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

uint32_t mango_nuplus_atox(const char *s) {
  int v=0;
  int sign=1;
  while ((unsigned int)(*s-9) < 5u || *s == ' ') s++;
  switch (*s) {
  case '-': sign=-1;
  case '+': ++s;
  }
  while ((unsigned int) ((*s | 0x20) - 'a') < 26u || (unsigned int) (*s - '0') < 10u) {
    v= (unsigned int) (*s - '0') < 10u ? v*16+*s-'0' : v*16+(*s & 0x5F)-'A'+10;
	++s;
  }
  return sign==-1?-v:v;
}

