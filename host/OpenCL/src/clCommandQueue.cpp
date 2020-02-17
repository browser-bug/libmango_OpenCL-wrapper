#include "clCommandQueue.h"

#include "clContext.h"
#include "clDevice.h"
#include "clEvent.h"

cl_command_queue cl_create_command_queue(cl_context context,
                                         cl_device_id device,
                                         cl_command_queue_properties properties,
                                         cl_int *errcode_ret)
{
    cl_command_queue queue = NULL;
    queue = (cl_command_queue)malloc(sizeof(struct _cl_command_queue));

    if (!context)
    {
        if (!errcode_ret)
            *errcode_ret CL_INVALID_CONTEXT;
        goto err;
    }

    if (!device)
    {
        if (!errcode_ret)
            *errcode_ret CL_INVALID_DEVICE;
        goto err;
    }

    context->queue = queue;
    queue->ctx = context;
    // If you need to associate device with its corresponding queue
    for (int i = 0; i < context->device_num; i++)
        context->devices[i]->queue = queue;

    /* initialize events queue */
    queue->events = NULL;
    queue->command_count = 0;
    // queue->last_event.event = NULL;
    // queue->last_event.next = NULL;

    /* initialize task graph */
    queue->tgx = NULL;
    return queue;

err:
    free(queue);
    queue = NULL;
    return queue;
}

cl_int cl_finish(cl_command_queue command_queue)
{
    if (command_queue == NULL)
        return CL_INVALID_COMMAND_QUEUE;

    for (int i = 0; i < command_queue->command_count; i++)
    {
        mango_wait(command_queue->events[i]->ev);
    }

    return CL_SUCCESS;
}