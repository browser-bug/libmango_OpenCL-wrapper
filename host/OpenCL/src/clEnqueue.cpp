#include "clEnqueue.h"

#include "clKernel.h"
#include "clProgram.h"
#include "clDevice.h"
#include "clContext.h"
#include "clCommandQueue.h"
#include "clMem.h"
#include "clEvent.h"
#include "clExceptions.h"

// TODO all event checking related code could be moved inside this function to avoid code repetition
/* checks that all events in event_wait_list are inside the context */
bool checkWaitListInContext(cl_context ctx, const cl_event *event_wait_list, cl_uint num_events_in_wait_list)
{
    for (int i = 0; i < num_events_in_wait_list; i++)
    {
        if (ctx != event_wait_list[i]->ctx)
            return false;
    }
    return true;
}

cl_int cl_enqueue_task(cl_command_queue command_queue,
                       cl_kernel kernel,
                       cl_uint num_events_in_wait_list,
                       const cl_event *event_wait_list,
                       cl_event *event)
{
    cl_event temp_event = NULL;

    mango_args_t *args;
    mango_event_t task_event;

    if (command_queue == NULL)
        throw cl_error(CL_INVALID_COMMAND_QUEUE);

    if (kernel == NULL)
        throw cl_error(CL_INVALID_KERNEL);

    assert(kernel->program && "[clEnqueueTask] kernel isn't associated with a program");
    /* check that the command queue and the kernel have the same context */
    if (command_queue->ctx != kernel->program->ctx)
        throw cl_error(CL_INVALID_CONTEXT);

    if ((event_wait_list == NULL) && (num_events_in_wait_list > 0))
        throw cl_error(CL_INVALID_EVENT_WAIT_LIST);

    if ((event_wait_list != NULL) && (num_events_in_wait_list == 0))
        throw cl_error(CL_INVALID_EVENT_WAIT_LIST);

    if (event_wait_list)
    {
        if (!checkWaitListInContext(command_queue->ctx, event_wait_list, num_events_in_wait_list))
            throw cl_error(CL_INVALID_CONTEXT);
    }

    /* check and wait for every event in the wait list */
    if (event_wait_list)
    {
        for (int i = 0; i < num_events_in_wait_list; i++)
        {
            // std::cout << "[clEnqueueTask] waiting for the event : " << event_wait_list[i]->ev << std::endl;
            cl_wait_for_events(1, &event_wait_list[i]);
            // std::cout << "[clEnqueueTask] finished waiting for event : " << event_wait_list[i]->ev << std::endl;
        }
    }

    // std::cout << "[clEnqueueTask] setting args for kernel: " << kernel->kernel << std::endl;
    args = mango_set_args_from_vector(kernel->kernel, &kernel->args);

    // std::cout << "[clEnqueueTask] starting the kernel: " << kernel->kernel << std::endl;
    task_event = mango_start_kernel(kernel->kernel, args, NULL);

    // std::cout << "[clEnqueueTask] creating event : " << task_event << std::endl;
    temp_event = new _cl_event(command_queue->ctx, command_queue);

    temp_event->ev = task_event;
    temp_event->event_type = CL_COMMAND_NDRANGE_KERNEL; // since clEnqueueTask is actually deprecated we can assume is always of this type

    /* add event to queue */
    command_queue->events.push_back(temp_event);

    /* if present, store the kernel_start_event into 'event' variable */
    if (event)
        *event = temp_event;

    return CL_SUCCESS;
}

cl_int cl_enqueue_write_buffer(cl_command_queue command_queue,
                               cl_mem buffer,
                               cl_bool blocking_write,
                               const void *ptr,
                               cl_uint num_events_in_wait_list,
                               const cl_event *event_wait_list,
                               cl_event *event)
{
    cl_event temp_event = NULL;

    mango_event_t task_event;

    if (command_queue == NULL)
        throw cl_error(CL_INVALID_COMMAND_QUEUE);

    if (buffer == NULL)
        throw cl_error(CL_INVALID_MEM_OBJECT);

    if (ptr == NULL)
        throw cl_error(CL_INVALID_VALUE);

    if (command_queue->ctx != buffer->ctx)
        throw cl_error(CL_INVALID_CONTEXT);

    if (ptr == NULL)
        throw cl_error(CL_INVALID_VALUE);

    if ((event_wait_list == NULL) && (num_events_in_wait_list > 0))
        throw cl_error(CL_INVALID_EVENT_WAIT_LIST);

    if ((event_wait_list != NULL) && (num_events_in_wait_list == 0))
        throw cl_error(CL_INVALID_EVENT_WAIT_LIST);

    if (event_wait_list)
    {
        if (!checkWaitListInContext(command_queue->ctx, event_wait_list, num_events_in_wait_list))
            throw cl_error(CL_INVALID_CONTEXT);
    }

    /* check and wait for every event in the wait list */
    if (event_wait_list)
    {
        for (int i = 0; i < num_events_in_wait_list; i++)
        {
            // std::cout << "[clEnqueueWriteBuffer] waiting for the event : " << event_wait_list[i]->ev << std::endl;
            cl_wait_for_events(1, &event_wait_list[i]);
            // std::cout << "[clEnqueueWriteBuffer] finished waiting for event : " << event_wait_list[i]->ev << std::endl;
        }
    }

    // printf("[clEnqueueWriteBuffer] Enqueuing write buffer %d. Current specificication assumes asynchronous transfer.\n", buffer->id);
    task_event = mango_write(ptr, buffer->buffer, DIRECT, 0);

    // TODO this needs a more in depth look
    // if (blocking_write)
    //     mango_wait_state(task_event, mango_event_status_t::WRITE);

    // std::cout << "[clEnqueueWriteBuffer] creating event : " << task_event << std::endl;
    temp_event = new _cl_event(command_queue->ctx, command_queue);

    temp_event->ev = task_event;
    temp_event->event_type = CL_COMMAND_WRITE_BUFFER;

    /* add event to queue */
    command_queue->events.push_back(temp_event);

    /* if present, store the mango_write_event into 'event' variable */
    if (event)
        *event = temp_event;

    return CL_SUCCESS;
}

cl_int cl_enqueue_read_buffer(cl_command_queue command_queue,
                              cl_mem buffer,
                              cl_bool blocking_read,
                              void *ptr,
                              cl_uint num_events_in_wait_list,
                              const cl_event *event_wait_list,
                              cl_event *event)
{
    cl_event temp_event = NULL;

    mango_event_t task_event;

    if (command_queue == NULL)
        throw cl_error(CL_INVALID_COMMAND_QUEUE);

    if (buffer == NULL)
        throw cl_error(CL_INVALID_MEM_OBJECT);

    if (ptr == NULL)
        throw cl_error(CL_INVALID_VALUE);

    if (command_queue->ctx != buffer->ctx)
        throw cl_error(CL_INVALID_CONTEXT);

    if (ptr == NULL)
        throw cl_error(CL_INVALID_VALUE);

    if ((event_wait_list == NULL) && (num_events_in_wait_list > 0))
        throw cl_error(CL_INVALID_EVENT_WAIT_LIST);

    if ((event_wait_list != NULL) && (num_events_in_wait_list == 0))
        throw cl_error(CL_INVALID_EVENT_WAIT_LIST);

    if (event_wait_list)
    {
        if (!checkWaitListInContext(command_queue->ctx, event_wait_list, num_events_in_wait_list))
            throw cl_error(CL_INVALID_CONTEXT);
    }

    /* check and wait for every event in the wait list */
    if (event_wait_list)
    {
        for (int i = 0; i < num_events_in_wait_list; i++)
        {
            // std::cout << "[clEnqueueReadBuffer] waiting for the event : " << event_wait_list[i]->ev << std::endl;
            cl_wait_for_events(1, &event_wait_list[i]);
            // std::cout << "[clEnqueueReadBuffer] finished waiting for event : " << event_wait_list[i]->ev << std::endl;
        }
    }

    // printf("[clEnqueueReadBuffer] Enqueuing read buffer %d. Current specificication assumes asynchronous transfer.\n", buffer->id);
    task_event = mango_read(ptr, buffer->buffer, DIRECT, 0);

    // TODO this needs a more in depth look
    // if (blocking_read)
    //     mango_wait_state(task_event, mango_event_status_t::READ);

    // std::cout << "[clEnqueueReadBuffer] creating event : " << task_event << std::endl;
    temp_event = new _cl_event(command_queue->ctx, command_queue);

    temp_event->ev = task_event;
    temp_event->event_type = CL_COMMAND_READ_BUFFER;

    /* add event to queue */
    command_queue->events.push_back(temp_event);

    /* if present, store the mango_write_event into 'event' variable */
    if (event)
        *event = temp_event;

    return CL_SUCCESS;
}