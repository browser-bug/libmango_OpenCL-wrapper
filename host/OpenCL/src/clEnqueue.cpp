#include "clEnqueue.h"

#include "clKernel.h"
#include "clProgram.h"
#include "clDevice.h"
#include "clContext.h"
#include "clCommandQueue.h"
#include "clMem.h"
#include "clEvent.h"

cl_int addEventToQueue(cl_command_queue queue, cl_event event)
{
    if (queue->events != NULL)
        queue->events = (cl_event *)realloc(queue->events, sizeof(cl_event *) * (queue->command_count + 1)); // FIX : maybe reallocation can be smarter, *2 just when necessary instead of +1 everytime
    else
        queue->events = (cl_event *)calloc(1, sizeof(cl_event));

    queue->events[queue->command_count] = event;
    queue->command_count++;
    return CL_SUCCESS;
}

cl_int cl_enqueue_ND_range_kernel(cl_command_queue command_queue,
                                  cl_kernel kernel,
                                  cl_uint num_events_in_wait_list,
                                  const cl_event *event_wait_list,
                                  cl_event *event)
{
    std::cout << "[clEnqueueNDRangeKernel] setting args for kernel: " << kernel->kernel << std::endl;
    std::vector<mango::Arg *> args_vec((mango::Arg **)kernel->args, (mango::Arg **)(kernel->args + kernel->args_num));
    mango_args_t *args = mango_set_args_from_vector(kernel->kernel, &args_vec);
    std::cout << "[clEnqueueNDRangeKernel] succesfully created args" << std::endl;

    if (event_wait_list)
    {
        if (num_events_in_wait_list > 0)
        {
            for (int i = 0; i < num_events_in_wait_list; i++)
            {
                if (event_wait_list[i]->ctx == NULL || command_queue->ctx != event_wait_list[i]->ctx)
                    return CL_INVALID_CONTEXT;
                std::cout << "[clEnqueueNDRangeKernel] waiting for the event : " << event_wait_list[i]->ev << std::endl;
                mango_wait(event_wait_list[i]->ev);
                std::cout << "[clEnqueueNDRangeKernel] finished waiting for event : " << event_wait_list[i]->ev << std::endl;
            }
        }
        else
        {
            return CL_INVALID_EVENT_WAIT_LIST;
        }
    }
    else
    {
        if (num_events_in_wait_list > 0)
            return CL_INVALID_EVENT_WAIT_LIST;
    }

    if (event)
    {
        (*event) = (cl_event)malloc(sizeof(struct _cl_event));
        (*event)->ev = mango_start_kernel(kernel->kernel, args, NULL);
        std::cout << "[clEnqueueNDRangeKernel] creating event : " << (*event)->ev << std::endl;
        (*event)->ctx = command_queue->ctx;

        if (addEventToQueue(command_queue, (*event)) != CL_SUCCESS)
            return CL_OUT_OF_RESOURCES; // FIX : random error, maybe it is not even needed
    }
    else
        mango_start_kernel(kernel->kernel, args, NULL);

    return CL_SUCCESS;
}

cl_int cl_enqueue_write_buffer(cl_command_queue command_queue,
                               cl_mem buffer,
                               const void *ptr,
                               cl_uint num_events_in_wait_list,
                               const cl_event *event_wait_list,
                               cl_event *event)
{
    if (!command_queue)
        return CL_INVALID_COMMAND_QUEUE;
    if (!buffer)
        return CL_INVALID_MEM_OBJECT;
    if (!ptr)
        return CL_INVALID_VALUE;
    if (command_queue->ctx == NULL || buffer->ctx == NULL || command_queue->ctx != buffer->ctx)
        return CL_INVALID_CONTEXT;

    if (event_wait_list)
    {
        if (num_events_in_wait_list > 0)
        {
            for (int i = 0; i < num_events_in_wait_list; i++)
            {
                if (event_wait_list[i]->ctx == NULL || command_queue->ctx != event_wait_list[i]->ctx)
                    return CL_INVALID_CONTEXT;
                std::cout << "[clEnqueueWriteBuffer] waiting for the event : " << event_wait_list[i]->ev << std::endl;
                mango_wait(event_wait_list[i]->ev);
                std::cout << "[clEnqueueWriteBuffer] finished waiting for event : " << event_wait_list[i]->ev << std::endl;
            }
        }
        else
        {
            return CL_INVALID_EVENT_WAIT_LIST;
        }
    }
    else
    {
        if (num_events_in_wait_list > 0)
            return CL_INVALID_EVENT_WAIT_LIST;
    }

    printf("Enqueuing write buffer %d. Current specificication assumes asynchronous transfer.\n", buffer->id);
    if (event)
    {
        (*event) = (cl_event)malloc(sizeof(struct _cl_event));
        (*event)->ev = mango_write(ptr, buffer->buffer, DIRECT, 0);
        std::cout << "[clEnqueueWriteBuffer] creating event : " << (*event)->ev << std::endl;
        (*event)->ctx = command_queue->ctx;
    }
    else
        mango_write(ptr, buffer->buffer, DIRECT, 0);

    return CL_SUCCESS;
}

cl_int cl_enqueue_read_buffer(cl_command_queue command_queue,
                              cl_mem buffer,
                              void *ptr,
                              cl_uint num_events_in_wait_list,
                              const cl_event *event_wait_list,
                              cl_event *event)
{
    if (!command_queue)
        return CL_INVALID_COMMAND_QUEUE;
    if (!buffer)
        return CL_INVALID_MEM_OBJECT;
    if (!ptr)
        return CL_INVALID_VALUE;
    if (command_queue->ctx == NULL || buffer->ctx == NULL || command_queue->ctx != buffer->ctx)
        return CL_INVALID_CONTEXT;

    if (event_wait_list)
    {
        if (num_events_in_wait_list > 0)
        {
            for (int i = 0; i < num_events_in_wait_list; i++)
            {
                if (event_wait_list[i]->ctx == NULL || command_queue->ctx != event_wait_list[i]->ctx)
                    return CL_INVALID_CONTEXT;
                std::cout << "[clEnqueueReadBuffer] waiting for the event : " << event_wait_list[i]->ev << std::endl;
                mango_wait(event_wait_list[i]->ev);
                std::cout << "[clEnqueueReadBuffer] finished waiting for event : " << event_wait_list[i]->ev << std::endl;
            }
        }
        else
        {
            return CL_INVALID_EVENT_WAIT_LIST;
        }
    }
    else
    {
        if (num_events_in_wait_list > 0)
            return CL_INVALID_EVENT_WAIT_LIST;
    }

    printf("Enqueuing read buffer %d. Current specificication assumes asynchronous transfer.\n", buffer->id);
    if (event)
    {
        (*event) = (cl_event)malloc(sizeof(struct _cl_event));
        (*event)->ev = mango_read(ptr, buffer->buffer, DIRECT, 0);
        std::cout << "[clEnqueueReadBuffer] creating event : " << (*event)->ev << std::endl;
        (*event)->ctx = command_queue->ctx;
    }
    else
        mango_read(ptr, buffer->buffer, DIRECT, 0);

    return CL_SUCCESS;
}