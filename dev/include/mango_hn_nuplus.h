/*! \file
 *  \brief GN mapping of device side library
 * This file provides an implementation of the device runtime on a general
 * purpose CPU (or any other system supporting POSIX threads and semaphores)
 */
#ifndef MANGO_HN_GN_H
#define MANGO_HN_GN_H
#include "mango_hn_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief Initialize mango-dev lib 
 * \returns Success code, unless something bad happens.
 */
mango_exit_t mango_nuplus_init(char **argv);

/*! \brief Shutdown mango-dev lib and terminate kernel 
 * \param Exit status
 */
void mango_nuplus_close(int status);

/*! \brief Write an event
 * \param event An synchronization event on the HN side
 * \param value An integer value
 * write value to the TILEREG register associated with event
 */
void mango_nuplus_write_synchronization(mango_event_t *event, uint32_t value);

/*! \brief Read and reset an event
 * \param event An synchronization event on the HN side
 * read value from the TILEREG register associated with event and replace it
 * with 0
 */
uint32_t mango_nuplus_read_synchronization(mango_event_t *event);

/*! \brief Run task in range parallel copies. 
 * \param task The address of the function code to run. 
 * \param range The number of threads to spawn.
 * \returns A barrier event for signaling completion.
 * The tasks share the same memory space as their creator. Tasks can only be
 * created from the main processing element, which runs the initial binary after
 * being activated by the GN node.
 */
mango_event_t *mango_nuplus_spawn(void *(*task)(task_args *), uint32_t range);

/*! \brief Deallocate the task_args data structure
 * \param a The arguments for the taks
 */
void mango_nuplus_task_arg_free(task_args *a);

/*! \brief Suspend the task
 * This function is only useful in GN mode to put a task to sleep
 */
void mango_nuplus_suspend();

/*! \brief Convert input arguments into hexadecimal integers. 
 * Same behaviour as strtol with hexadecimal base, although returns a 32-bit integer.
 * \param s C-string beginning with the representation of an integral number.
 */
uint32_t mango_nuplus_atox(const char *s);

void mango_nuplus_endian_row (uint32_t * data, int size);
 
#ifdef __cplusplus
}
#endif

#endif /* MANGO_HN_H */
