/*! \file
 *  \brief GN mapping of device side library
 * This file provides an implementation of the device runtime on a general
 * purpose CPU (or any other system supporting POSIX threads and semaphores)
 */
#ifndef MANGO_HN_GN_H
#define MANGO_HN_GN_H
#include "mango_hn_types.h"

/*! \brief Initialize mango-dev lib 
 * \returns Success code, unless something bad happens.
 */
mango_exit_t mango_gn_init();

/*! \brief Shutdown mango-dev lib and terminate kernel 
 * \param Exit status
 */
void mango_gn_close(int status);

/*! \brief Write an event
 * \param event An synchronization event on the HN side
 * \param value An integer value
 * write value to the TILEREG register associated with event
 */
void mango_gn_write_synchronization(mango_event_t *event, uint32_t value);

/*! \brief Read and reset an event
 * \param event An synchronization event on the HN side
 * read value from the TILEREG register associated with event and replace it
 * with 0
 */
uint32_t mango_gn_read_synchronization(mango_event_t *event);

/*! \brief Run task in range parallel copies. 
 * \param task The address of the function code to run. 
 * \param range The number of threads to spawn.
 * \returns A barrier event for signaling completion.
 * The tasks share the same memory space as their creator. Tasks can only be
 * created from the main processing element, which runs the initial binary after
 * being activated by the GN node.
 */
mango_event_t *mango_gn_spawn(void *(*task)(task_args *), uint64_t range);

/*! \brief Deallocate the task_args data structure
 * \param a The arguments for the taks
 */
void mango_gn_task_arg_free(task_args *a);

/*! \brief Suspend the task
 * This function is only useful in GN mode to put a task to sleep
 */
void mango_gn_suspend();

#endif /* MANGO_HN_H */
