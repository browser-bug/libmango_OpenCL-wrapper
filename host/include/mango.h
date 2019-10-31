#ifndef MANGO_H
#define MANGO_H
/*! \mainpage
 * This set of header files describes the software interfaces employed by the
 * applications and the runtime manager on the GN node.
 * In particular:
 *   - mango.h describes the C API used by the applications to interact with the
 *     resource manager and to prepare and launch kernels on different devices. 
 *   - mango_types_c.h provides the definitions of C constants.
 *   - context.h, buffer.h, kernel.h, event.h, task_graph.h compose the C++ API.
 *   - mango_types.h provides the definitions of the C++ constants.
 *   - mm.h provides a minimal virtual address management and memory allocation
 *     capabilities.
 *   - process.h and debug.h provide support functionalities for debugging and
 *     simple emulation of the MANGO hardware units.
 *   - In folder include_dev, mango_hn.h provides the device-side runtime   
 *     API, and mango_hn_types.h the associated types and constants.
 *   - In folder include_dev, mango_hn_gn.h provides an implementation of the
 *     device-side runtime API for the emulated unit.
 *
 * \note Roadmap items implemented
 *   - Design and implement device side API for GN emulation
 *   - Implement an example application in GN emulation
 *   - Implement support for FIFO in GN emulation
 *   - Implement MM
 *   - C++ API
 *
 * \todo Roadmap:
 *   - Write wrappers for C in mango.cpp
 *   - Cleanup and possibly move code from .h to .cpp
 *   - Implement BarbequeRTRM integration in GN emulation
 *   - Implement HW description data structures (MM + BBQ)
 *   - Port communications to MANGO low-level PCIe primitives
 *   - Implement mango_load_kernel and mango_start_kernel over MANGO low-level
 *     primitives
 *   - Implement mango_set_args for PEAK and NU+
 *   - Port device API for PEAK and NU+
 *   - Integration example on PEAK
 *   - Integration example on NU+
 *
 * \note The API requires C++14 (mostly for the current emulated implementation)
 */

/*! \file mango.h
 * \brief This header file describes the API for MANGO applications.
 *
 * The API allows to prepare and launch kernels on the HN nodes.
 * It masks all interactions with the GN-side runtime, memory allocator, and
 * BarbequeRTRM.
 *
 */
#ifndef __cplusplus
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#endif

#ifdef __cplusplus
#include "context.h"

extern "C" {
#endif 

//typedef mango::ExitCode mango_exit_t;
//typedef mango::UnitType mango_unit_type_t;
//typedef mango::BufferType mango_buffer_type_t;
//typedef mango::FileType filetype;

typedef uint32_t mango_kernel_t;
typedef uint32_t mango_buffer_t;
typedef uint32_t mango_event_t;
typedef struct KernelFunction kernelfunction;
typedef struct TaskGraph mango_task_graph_t;
typedef struct KernelArguments mango_args_t;
typedef struct Arg mango_arg_t;


#include "mango_types_c.h"

/*! @name Initialization and shutdown */
///@{

/*! \brief Initialize runtime library */
mango_exit_t mango_init(const char *application_name, const char *recipe);

/*! \brief Shutdown runtime library */
mango_exit_t mango_release();

///@}

/*! @name Kernel loading */
///@{

/*! \brief Initialize a kernelfunction data structure */
kernelfunction *mango_kernelfunction_init();

/*! \brief Initialize a kernelfunction data structure 
 * \param kname Kernel file name or string representation
 * \param kernel Target kernel data structure
 * \param unit The type of unit for which this implementation is compiled/optimized
 * \param t The type of file (source file, binary file, or source string)
 * \returns An exit code; kernel is modified by adding the appropriate kernel implementation
 */
mango_exit_t mango_load_kernel(const char *kname, kernelfunction *kernel, mango_unit_type_t unit, filetype t);

///@}


/*! @name Registration of task graph components */
///@{

/*! \brief Register a kernel with BarbequeRTRM
 * \param kernel_id A unique identifier of the kernel, used to match it to the
 * recipe specification
 * \param kernel The kernel to run, possibly in multiple version compiled for
 * different accelerator architectures
 * \param nbuffers_in Number of buffers read by the kernel
 * \param nbuffers_out Number of buffers written by the kernel
 * \param ... The variadic parameters define which buffers will be accessed (read
 * first, then write) by the kernel, and should all be of type mango_buffer_t 
 * \return The registered kernel
 *
 * \note thread count and unit type have been moved to the RTlib
 * \deprecated  thread_count The number of parallel instances to run. If set to
 * 0, it is interpreted as "as many as possible" by the resource manager
 * \deprecated  ut The list of unit types in order of preference
 * (STOP-terminated), if NULL rely on BarbequeRTRM recipes.
 */
mango_kernel_t mango_register_kernel(uint32_t kernel_id, kernelfunction *kernel, 
					unsigned int nbuffers_in, unsigned  int nbuffers_out, ...);

/*! \brief De-register the kernel with BarbequeRTRM
 * \param kernel The kernel to remove
 */
void mango_deregister_kernel(mango_kernel_t kernel);


/*! \brief Register a memory region to use as a buffer for communication
 * \param buffer_id Id of the buffer
 * \param size Size of the buffer
 * \param mode Type of buffer
 * \param nkernels Number of kernels which will write the buffer
 * \param nkernels Number of kernels which will read the buffer
 * \param ... The variadic parameters define which kernels will access (write
 * first, then read) the buffer, and should all be of type mango_kernel_t 
 * \return A data structure containing the physical address and the size of the
 * registered memory region
 *
 * \note The variadic parameters do not need to be set. Setting nkernels_[in|out] 
 * to 0 tells the memory allocator to perform the best allocation based on the
 * resources currently associated with the entire application.
 * \note In case of "FIFO" types, an appropriate background process should be
 * in charge of setting up the communication. In this case, a single mango_write
 * or mango_read will setup the streamed communication
 * \note The bandwidth parameter has been moved to the RTlib
 */
mango_buffer_t mango_register_memory(uint32_t buffer_id, size_t size, mango_buffer_type_t mode, unsigned int nkernels_in, unsigned int nkernels_out, ...);

/*! \brief Deallocate registered memory
 * \param mem The memory buffer to deallocate
 *
 * This function frees space from the HN node for use by future kernels.
 */
void mango_deregister_memory(mango_buffer_t mem);

/*! \brief Register a HN synchronization event (semaphore?)
 * \param nkernels Number of kernels which will write the event
 * \param nkernels Number of kernels which will read the event
 * \param ... The variadic parameters define which kernels will access the
 * buffer, and should all be of type mango_kernel_t 
 * \return A data structure representing an event accessible by all kernels in
 * the application
 * \note UPV comment: the synchronization event will be a register with special
 * read and write functionality. When read the register content will be
 * retrieved but the will be set to zero. When written the content will be
 * incremented by the value to be written.
 */
mango_event_t mango_register_event(unsigned int nkernels_in, unsigned int nkernels_out, ...);

/*! \brief Deallocate registered event
 * \param event The synchronization event to deallocate
 *
 * This function frees space from the TILEREG
 */
void mango_deregister_event(mango_event_t event);

/*! \brief Get an event from the corresponding buffer 
 * This function is needed to keep mango_buffer_t opaque in the C interface.
 */
mango_event_t mango_get_buffer_event(mango_buffer_t buffer);

///@}


/*! @name Task graph definition  */
///@{

/*! \brief Define a task graph
 * \param kernels NULL-terminated array of pointers to mango_kernel_t structures
 * representing the kernels in the task graph
 * \param buffers NULL-terminated array of pointers to mango_buffer_t structures
 * representing the buffers in the task graph
 * \param events NULL-terminated array of pointers to mango_event_t structures
 * representing the synchronization events in the task graph
 * \returns A pointer to the task graph data structure
 */
mango_task_graph_t *mango_task_graph_vcreate(mango_kernel_t **kernels, mango_buffer_t **buffers, mango_event_t **events);

/*! \brief Define a task graph 
 * \param k number of kernels 
 * \param b number of buffers
 * \param e number of events
 * \param ... The variadic parameters must be exactly k mango_kernel_t *, b
 * mango_buffer_t *, and e mango_event_t *, representing the kernels, buffers
 * and events employed in this task graph.
 * \returns A pointer to the task graph data structure
 */
mango_task_graph_t *mango_task_graph_create(int k, int b, int e, ...);

/*! \brief Destroy a task graph
 * \param task_graph The task graph to destroy
 */
void mango_task_graph_destroy(mango_task_graph_t *task_graph);

/*! \brief Destroy a task graph and deregister all of its components
 * \param task_graph The task graph to destroy
 */
void mango_task_graph_destroy_all(mango_task_graph_t *task_graph);


/*! \brief Add a kernel to the task graph
 * \param tg The task graph (if NULL, a new one is created)
 * \param kernel The kernel to add
 * \returns The task graph (identical to tg unless tg==NULL)
 */
mango_task_graph_t *mango_task_graph_add_kernel(mango_task_graph_t *tg, mango_kernel_t *kernel);

/*! \brief Remove a kernel from the task graph
 * \param tg The task graph
 * \param kernel The kernel to add
 * \returns The task graph (NULL if empty)
 */
mango_task_graph_t *mango_task_graph_remove_kernel(mango_task_graph_t *tg, mango_kernel_t *kernel);

/*! \brief Add a buffer to the task graph
 * \param tg The task graph (if NULL, a new one is created)
 * \param buffer The buffer to add
 * \returns The task graph (identical to tg unless tg==NULL)
 */
mango_task_graph_t *mango_task_graph_add_buffer(mango_task_graph_t *tg, mango_buffer_t *buffer);

/*! \brief Remove a buffer from the task graph
 * \param tg The task graph
 * \param buffer The buffer to add
 * \returns The task graph (NULL if empty)
 */
mango_task_graph_t *mango_task_graph_remove_buffer(mango_task_graph_t *tg, mango_buffer_t *buffer);

/*! \brief Add a event to the task graph
 * \param tg The task graph (if NULL, a new one is created)
 * \param event The event to add
 * \returns The task graph (identical to tg unless tg==NULL)
 */
mango_task_graph_t *mango_task_graph_add_event(mango_task_graph_t *tg, mango_event_t *event);

/*! \brief Remove a event from the task graph
 * \param tg The task graph
 * \param event The event to add
 * \returns The task graph (NULL if empty)
 */
mango_task_graph_t *mango_task_graph_remove_event(mango_task_graph_t *tg, mango_event_t *event);

mango_task_graph_t *mango_task_graph_update_events(mango_task_graph_t *tg);

///@}

/*! @name Resource Allocation  */
///@{

/*! \brief Resource Allocation for a task graph of the application
 * \param tg The task graph to allocate resources for
 * \returns An exit code signalling the correct allocation (or not)
 */
mango_exit_t mango_resource_allocation(mango_task_graph_t *tg);

/*! \brief Resource Allocation for a task graph of the application
 * \param tg The task graph to deallocate resources for
 */
void mango_resource_deallocation(mango_task_graph_t *tg);

///@}


/*! @name Synchronization primitives
 */
///@{

/*! \brief High level wait primitive
 * \param e A synchronization event
 */
 void mango_wait(mango_event_t e);

/*! \brief High level wait primitive
 * \param e A synchronization event
 * \param state Value waited for
 */
void mango_wait_state(mango_event_t e, uint32_t state);

/*! \brief Initialize an event
 * \param event A synchronization event on the HN side
 * \param value An integer value
 * write value to the TILEREG register associated with event
 */
void mango_write_synchronization(mango_event_t event, uint32_t value);

/*! \brief Read and reset an event
 * \param event A synchronization event on the HN side
 * read value from the TILEREG register associated with event and replace it
 * with 0
 */
uint32_t mango_read_synchronization(mango_event_t event);

/*! \brief lock and read event 
 * \param event A synchronization event on the HN side
 * read value from the TILEREG register associated with event and replace it
 * with 0, when it becomes != 0 (i.e., when unlocked)
 */
uint32_t mango_lock(mango_event_t e);

///@}



/*! @name GN-HN data transfer */
///@{

/*! \brief Memory transfer from GN to HN
 * \param GN_buffer A pointer to memory in the GN address space
 * \param HN_buffer A data structure representing a memory region allocated
 *        to the application on the HN
 * \param mode Type of transfer mode to perform.
 * \param global_size Size of the GN buffer; ignored in DIRECT mode
 * This function performs a copy between a memory region in the GN address space  * and one in the HN address space. The copy works on the entire buffer size
 * specified in the HN buffer descriptor.
 * \note Current specification assumes asynchronous transfer
 */
mango_event_t mango_write(const void *GN_buffer, mango_buffer_t HN_buffer,
				mango_communication_mode_t mode, size_t global_size);


/*! \brief Memory transfer from HN to GN
 * \param GN_buffer A pointer to memory in the GN address space
 * \param HN_buffer A data structure representing a memory region allocated
 *        to the application on the HN
 * \param mode Type of transfer mode to perform.
 * \param global_size Size of the GN buffer; ignored in DIRECT mode
 * This function performs a copy between a memory region in the HN address space
 * and one in the GN address space. The copy works on the entire buffer size
 * specified in the HN buffer descriptor.
 * \note Current specification assumes asynchronous transfer
 */
mango_event_t mango_read(void *GN_buffer, mango_buffer_t HN_buffer, mango_communication_mode_t mode, size_t global_size);
///@}


/*! @name Kernel launch */
///@{

/*! \brief Build an argument parameter
 * \param value The value or address of the argument
 * \param size The size of the argument
 * \param t The type of argument (scalar, event or buffer)
 * \returns a mango_arg_t structure
 */
mango_arg_t *mango_arg(mango_kernel_t kernel, const void *value, size_t size, mango_buffer_type_t t);

/*! \brief Set up the arguments for a kernel
 * \param kernel The kernel for which the parameters are set
 * \param argc The number of arguments to pass
 * \param ...  The variadic parameters represent the actual arguments,
 * and should all be of type mango_arg_t
 * \returns The packaged arguments for the loaded kernel
 * \note Can be called multiple times to change the arguments
 */
mango_args_t *mango_set_args(mango_kernel_t kernel, int argc, ...);

/*! \brief Run a kernel
 * \param kernel The registered kernel to run
 * \param args The structure representing the arguments
 * \param even The event which the kernel should notify upon completion; if NULL,
 * a new one will be created
 * \returns An event on which the application can synchronize to know whether
 * the kernel has completed (identical to the event parameter unless the former
 * is NULL).
 */
mango_event_t mango_start_kernel(mango_kernel_t kernel, mango_args_t *args, mango_event_t event);

/*!
 * \brief Returns the processor id assigned to the kernel. It must be called AFTER
 *        a successful resource allocation.
 * \param kernel The kernel to query
 */
uint32_t mango_get_unit_id(mango_kernel_t kernel);

/*!
 * \brief Returns the maximum number of buffers allowed (it depends on the architecture).
 */
uint16_t mango_get_max_nr_buffers(void);

/*!
 * \brief Returns the processor type assigned to the kernel specified as argument.
 *        It must be called AFTER a successful resource allocation.
 */
mango_unit_type_t mango_get_unit_arch(mango_kernel_t kernel);


///@}

#ifdef __cplusplus
}
#endif

#endif /* MANGO_H */
