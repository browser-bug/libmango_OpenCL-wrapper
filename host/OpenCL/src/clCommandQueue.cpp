#include "clCommandQueue.h"

#include "clContext.h"
#include "clDevice.h"
#include "clEvent.h"
#include "clExceptions.h"

cl_command_queue cl_create_command_queue(cl_context context,
                                         cl_device_id device,
                                         cl_int *errcode_ret)
{
    cl_command_queue queue = NULL;

    if (context == NULL)
        throw cl_error(CL_INVALID_CONTEXT);

    if (device == NULL)
        throw cl_error(CL_INVALID_DEVICE);

    queue = new _cl_command_queue(context, device);

    context->queue = queue;
    // asssociate device with the queue
    for (auto &d : context->devices)
        d->queue = queue;

    if (errcode_ret)
        *errcode_ret = CL_SUCCESS;
    return queue;
}

cl_int cl_finish(cl_command_queue command_queue)
{
    if (command_queue == NULL)
        return CL_INVALID_COMMAND_QUEUE;

    size_t queueLength = command_queue->events.size();
    for (int i = 0; i < queueLength; i++)
    {
        cl_event topEv = command_queue->events.back();
        mango_wait(topEv->ev);
        command_queue->events.pop_back();
    }

    return CL_SUCCESS;
}

// TODO : to be tested
cl_int cl_get_command_queue_info(cl_command_queue command_queue,
                                 cl_command_queue_info param_name,
                                 size_t param_value_size,
                                 void *param_value,
                                 size_t *param_value_size_ret)
{
    const void *src_ptr = NULL;
    size_t src_size = 0;
    cl_int ref;

    if (command_queue == NULL)
        return CL_INVALID_COMMAND_QUEUE;

    switch (param_name)
    {
    case CL_QUEUE_CONTEXT:
        src_ptr = &command_queue->ctx;
        src_size = sizeof(cl_context);
        break;
    case CL_QUEUE_DEVICE:
        src_ptr = &command_queue->device;
        src_size = sizeof(cl_device_id);
        break;
    default:
        std::cout << "[clGetContextInfo] " << param_name << "is invalid or not supported yet\n"
                  << std::endl;
        return CL_INVALID_VALUE;
    }

    if (param_value && param_value_size < src_size)
        return CL_INVALID_VALUE;
    if (param_value && param_value_size)
        memcpy(param_value, src_ptr, src_size);
    if (param_value_size_ret)
        *param_value_size_ret = src_size;
    return CL_SUCCESS;
}