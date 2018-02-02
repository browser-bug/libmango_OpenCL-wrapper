/*! \file
 *  \brief Device-side application programming interface
 */
#ifndef MANGO_HN_H
#define MANGO_HN_H
#include "mango_hn_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! @name Initialization and shutdown */
///@{

/*! \brief Initialize mango-dev lib 
 * \returns Success code, unless something bad happens.
 */
mango_exit_t mango_init(char ** argv);

/*! \brief Shutdown mango-dev lib and terminate kernel 
 * \param Exit status
 */
void mango_close(int status);

///@}


/*! @name Buffer mapping and unmapping */
///@{

/*! \brief MANGO memory map primitive
 * \param phy_addr The physical memory address
 * \param size The size of the buffer
 * \returns A pointer to the device virtual memory
 */
void *mango_map(uint64_t phy_addr, size_t size);


/*! \brief MANGO memory un-map primitive
 * \param mem The device virtual memory address
 * Releases the mapped memory
 */
void mango_unmap(void *mem);

///@}


/*! @name Synchronization */
///@{

/*! \brief Initialize an event
 * \param event An synchronization event on the HN side
 * \returns A pointer to the event object
 */
mango_event_t *mango_get_event(uint32_t id);

/*! brief Lock and read event
 * \param event An synchronization event on the HN side
 * \returns Value of event as soon as it is not zero
 */
uint32_t mango_lock(mango_event_t *e);

/*! \brief Wait function (typically used through mango_barrier or mango_join)
 * \param e  The event waited
 * \param state The value of the event on which the wait is ended
 */
void mango_wait(mango_event_t *e, uint32_t state);

/*! \brief Write an event
 * \param event An synchronization event on the HN side
 * \param value An integer value
 * write value to the TILEREG register associated with event
 */
void mango_write_synchronization(mango_event_t *event, uint32_t value);

/*! \brief Read and reset an event
 * \param event An synchronization event on the HN side
 * read value from the TILEREG register associated with event and replace it
 * with 0
 */
uint32_t mango_read_synchronization(mango_event_t *event);

/*! \brief Wait for all threads to complete a section of code
 * \param args The arguments of the task
 * \param fifo An synchronization event for a FIFO read, or nothing.
 */
void mango_barrier(task_args *args, mango_event_t *fifo);

///@}


/*! @name Task management */
///@{

/*! \brief Run task in range parallel copies. 
 * \param task The address of the function code to run. 
 * \param range The number of threads to spawn.
 * \returns A barrier event for signaling completion.
 * The tasks share the same memory space as their creator. Tasks can only be
 * created from the main processing element, which runs the initial binary after
 * being activated by the GN node.
 */
mango_event_t *mango_spawn(void *(*task)(task_args *), uint32_t range);

/*! \brief wait for parallel tasks to complete
 *  \param e The completion event
 */
void mango_join(mango_event_t *e);

/*! \brief exit thread
 *  \returns NULL;
 */
void *mango_exit();

/*! \brief Get task id
 * \returns The task id in [0,range)
 */
#define tid(s) (s->tid)

///@}

// Internal function, do not use
uint32_t* mango_memory_map(uint64_t a);

#ifdef __cplusplus
}
#endif

#endif /* MANGO_HN_H */
