#include "peakos_user.h"
#include "asm_call.h"

void peakos_halt_syscall(void) {
  __peakos_halt_syscall();
}

void peakos_get_timestamp(unsigned int *t_hi, unsigned int *t_lo, unsigned int *mhz) {
  __peakos_get_timestamp(t_hi, t_lo, mhz);
}

void peakos_accelerate(void *arg) {
  __peakos_accelerate(arg);
}
