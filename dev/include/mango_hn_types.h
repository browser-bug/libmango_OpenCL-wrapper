/*! \file
 *  \brief Data types for the MANGO device side library
 */
#ifndef MANGO_HN_TYPES_H
#define MANGO_HN_TYPES_H
#include "mango_types_c.h"
#include <stdint.h>
#include <stddef.h>
#ifdef GNEMU
#include <semaphore.h>
#endif

/*! \struct mango_event_t 
 * \brief Internal representation of event
 */
typedef struct _mango_event_t {
	volatile uint32_t *vaddr;
} mango_event_t;

/*! \enum mango_event_status_t 
 * \brief Symbolic constants for synchronization operations
 */

/*! \struct mango_context_t
 * \brief A data structure for the device-side context 
 */
typedef struct _context {
	mango_event_t event_a; /*!< End of task group */
	mango_event_t event_b; /*!< Barrier */
	mango_event_t event_r; /*!< Restart */
	mango_event_t event_exit; /*!< End of kernel */
	uint32_t memory_size;
#ifdef GNEMU
	unsigned  long long memory_size;
	uint32_t *memory;
	sem_t *semaphore;
	int tf;
#else //GNEMU
    uint32_t memory_size;
#endif
} mango_context_t;


/*! \struct task_args
 *\brief Spawn arguments 
 */
typedef struct _task_args {
	mango_event_t *event; /*!< Synchronization event for end of task */
	uint32_t tid; /*!< Task id */
	mango_event_t *barrier; /*!< Barrier event */
	mango_event_t *release; /*!< Release event */
	uint32_t ntasks; /*!< Number of tasks in group */
} task_args;

#endif /* MANGO_HN_TYPES_H */
