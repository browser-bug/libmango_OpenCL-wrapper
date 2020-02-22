#include "clEnqueue.h"

#include "clKernel.h"
#include "clProgram.h"
#include "clDevice.h"
#include "clContext.h"
#include "clCommandQueue.h"
#include "clMem.h"
#include "clEvent.h"

// TODO all event checking related code could be moved inside this function to avoid code repetition
/* checks that all events in event_wait_list are inside the context */
bool checkWaitListInContext(cl_context ctx, const cl_event *event_wait_list, cl_uint num_events_in_wait_list, cl_int *err)
{
    for (int i = 0; i < num_events_in_wait_list; i++)
    {
        if (ctx != event_wait_list[i]->ctx)
        {
            *err = CL_INVALID_CONTEXT;
            return false;
        }
    }
    return true;
}

cl_int cl_enqueue_task(cl_command_queue command_queue,
                       cl_kernel kernel,
                       cl_uint num_events_in_wait_list,
                       const cl_event *event_wait_list,
                       cl_event *event)
{
    cl_int err = CL_SUCCESS;

    mango_args_t *args;
    mango_event_t task_event;

    if (command_queue == NULL)
    {
        err = CL_INVALID_COMMAND_QUEUE;
        goto exit;
    }

    if (kernel == NULL)
    {
        err = CL_INVALID_KERNEL;
        goto exit;
    }

    assert(kernel->program && "[clEnqueueTask] kernel isn't associated with a program");
    /* check that the command queue and the kernel have the same context */
    if (command_queue->ctx != kernel->program->ctx)
    {
        err = CL_INVALID_CONTEXT;
        goto exit;
    }

    if ((event_wait_list == NULL) && (num_events_in_wait_list > 0))
    {
        err = CL_INVALID_EVENT_WAIT_LIST;
        goto exit;
    }

    if ((event_wait_list != NULL) && (num_events_in_wait_list == 0))
    {
        err = CL_INVALID_EVENT_WAIT_LIST;
        goto exit;
    }

    if (event_wait_list)
    {
        if (!checkWaitListInContext(command_queue->ctx, event_wait_list, num_events_in_wait_list, &err))
            goto exit;
    }

    /* check and wait for every event in the wait list */
    if (event_wait_list)
    {
        for (int i = 0; i < num_events_in_wait_list; i++)
        {
            std::cout << "[clEnqueueTask] waiting for the event : " << event_wait_list[i]->ev << std::endl;
            mango_wait(event_wait_list[i]->ev);
            std::cout << "[clEnqueueTask] finished waiting for event : " << event_wait_list[i]->ev << std::endl;
        }
    }

    std::cout << "[clEnqueueTask] setting args for kernel: " << kernel->kernel << std::endl;
    args = mango_set_args_from_vector(kernel->kernel, &kernel->args);

    std::cout << "[clEnqueueTask] starting the kernel: " << kernel->kernel << std::endl;
    task_event = mango_start_kernel(kernel->kernel, args, NULL);

    /* if present, store the kernel_start_event into 'event' variable */
    if (event)
    {
        std::cout << "[clEnqueueTask] creating event : " << task_event << std::endl;
        (*event) = new (std::nothrow) _cl_event;
        if (!(*event))
        {
            err = CL_OUT_OF_HOST_MEMORY;
            goto exit;
        }

        (*event)->ev = task_event;
        (*event)->queue = command_queue;
        (*event)->ctx = command_queue->ctx;
        (*event)->event_type = CL_COMMAND_NDRANGE_KERNEL; // since clEnqueueTask is actually deprecated we can assume is always of this type

        /* add event to queue */
        command_queue->events.push_back(*event);
    }

exit:
    return err;
}

cl_int cl_enqueue_write_buffer(cl_command_queue command_queue,
                               cl_mem buffer,
                               const void *ptr,
                               cl_uint num_events_in_wait_list,
                               const cl_event *event_wait_list,
                               cl_event *event)
{
    cl_int err = CL_SUCCESS;

    mango_event_t task_event;

    if (command_queue == NULL)
    {
        err = CL_INVALID_COMMAND_QUEUE;
        goto exit;
    }
    if (buffer == NULL)
    {
        err = CL_INVALID_MEM_OBJECT;
        goto exit;
    }
    if (ptr == NULL)
    {
        err = CL_INVALID_VALUE;
        goto exit;
    }
    if (command_queue->ctx != buffer->ctx)
    {
        err = CL_INVALID_CONTEXT;
        goto exit;
    }
    if (ptr == NULL)
    {
        err = CL_INVALID_VALUE;
        goto exit;
    }
    if ((event_wait_list == NULL) && (num_events_in_wait_list > 0))
    {
        err = CL_INVALID_EVENT_WAIT_LIST;
        goto exit;
    }

    if ((event_wait_list != NULL) && (num_events_in_wait_list == 0))
    {
        err = CL_INVALID_EVENT_WAIT_LIST;
        goto exit;
    }

    if (event_wait_list)
    {
        if (!checkWaitListInContext(command_queue->ctx, event_wait_list, num_events_in_wait_list, &err))
            goto exit;
    }

    /* check and wait for every event in the wait list */
    if (event_wait_list)
    {
        for (int i = 0; i < num_events_in_wait_list; i++)
        {
            std::cout << "[clEnqueueWriteBuffer] waiting for the event : " << event_wait_list[i]->ev << std::endl;
            mango_wait(event_wait_list[i]->ev);
            std::cout << "[clEnqueueWriteBuffer] finished waiting for event : " << event_wait_list[i]->ev << std::endl;
        }
    }

    printf("[clEnqueueWriteBuffer] Enqueuing write buffer %d. Current specificication assumes asynchronous transfer.\n", buffer->id);
    task_event = mango_write(ptr, buffer->buffer, DIRECT, 0);

    /* if present, store the mango_write_event into 'event' variable */
    if (event)
    {
        std::cout << "[clEnqueueWriteBuffer] creating event : " << task_event << std::endl;
        (*event) = new (std::nothrow) _cl_event;
        if (!(*event))
        {
            err = CL_OUT_OF_HOST_MEMORY;
            goto exit;
        }

        (*event)->ev = task_event;
        (*event)->queue = command_queue;
        (*event)->ctx = command_queue->ctx;
        (*event)->event_type = CL_COMMAND_WRITE_BUFFER;

        /* add event to queue */
        command_queue->events.push_back(*event);
    }

exit:
    return err;
}

cl_int cl_enqueue_read_buffer(cl_command_queue command_queue,
                              cl_mem buffer,
                              void *ptr,
                              cl_uint num_events_in_wait_list,
                              const cl_event *event_wait_list,
                              cl_event *event)
{
    cl_int err = CL_SUCCESS;

    mango_event_t task_event;

    if (command_queue == NULL)
    {
        err = CL_INVALID_COMMAND_QUEUE;
        goto exit;
    }
    if (buffer == NULL)
    {
        err = CL_INVALID_MEM_OBJECT;
        goto exit;
    }
    if (ptr == NULL)
    {
        err = CL_INVALID_VALUE;
        goto exit;
    }
    if (command_queue->ctx != buffer->ctx)
    {
        err = CL_INVALID_CONTEXT;
        goto exit;
    }
    if (ptr == NULL)
    {
        err = CL_INVALID_VALUE;
        goto exit;
    }
    if ((event_wait_list == NULL) && (num_events_in_wait_list > 0))
    {
        err = CL_INVALID_EVENT_WAIT_LIST;
        goto exit;
    }

    if ((event_wait_list != NULL) && (num_events_in_wait_list == 0))
    {
        err = CL_INVALID_EVENT_WAIT_LIST;
        goto exit;
    }

    if (event_wait_list)
    {
        if (!checkWaitListInContext(command_queue->ctx, event_wait_list, num_events_in_wait_list, &err))
            goto exit;
    }

    /* check and wait for every event in the wait list */
    if (event_wait_list)
    {
        for (int i = 0; i < num_events_in_wait_list; i++)
        {
            std::cout << "[clEnqueueReadBuffer] waiting for the event : " << event_wait_list[i]->ev << std::endl;
            mango_wait(event_wait_list[i]->ev);
            std::cout << "[clEnqueueReadBuffer] finished waiting for event : " << event_wait_list[i]->ev << std::endl;
        }
    }

    printf("[clEnqueueReadBuffer] Enqueuing read buffer %d. Current specificication assumes asynchronous transfer.\n", buffer->id);
    task_event = mango_read(ptr, buffer->buffer, DIRECT, 0);

    /* if present, store the mango_write_event into 'event' variable */
    if (event)
    {
        std::cout << "[clEnqueueReadBuffer] creating event : " << task_event << std::endl;
        (*event) = new (std::nothrow) _cl_event;
        if (!(*event))
        {
            err = CL_OUT_OF_HOST_MEMORY;
            goto exit;
        }

        (*event)->ev = task_event;
        (*event)->queue = command_queue;
        (*event)->ctx = command_queue->ctx;
        (*event)->event_type = CL_COMMAND_READ_BUFFER;

        /* add event to queue */
        command_queue->events.push_back(*event);
    }

exit:
    return err;
}