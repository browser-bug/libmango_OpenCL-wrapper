/*! \file
 *  \brief Data types for the MANGO device side library
 */
#ifndef MANGO_HN_TYPES_H
#define MANGO_HN_TYPES_H
#include "mango_types_c.h"
#include <unistd.h>
#include <stdint.h>
#include <semaphore.h>

/*! \struct mango_event_t 
 * \brief Internal representation of event
 */
typedef struct _mango_event_t {
	uint32_t *vaddr;
} mango_event_t;

/*! \enum mango_event_status_t 
 * \brief Symbolic constants for synchronization operations
 */
typedef enum { LOCK, READ, WRITE, END_FIFO_OPERATION } mango_event_status_t;

/*! \struct mango_context_t
 * \brief A data structure for the device-side context 
 */
typedef struct _context {
	sem_t *semaphore; /*!< Only needed for current GN implementation */
	mango_event_t event_a; /*!< End of task group */
	mango_event_t event_b; /*!< Barrier */
	mango_event_t event_r; /*!< Restart */
	mango_event_t event_exit; /*!< End of kernel */
	uint8_t *memory; /*!< Only needed for current GN implementation */
	int tf;	 /*!< Only needed for current GN implementation */
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
