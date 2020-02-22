#include "clEvent.h"

cl_int cl_wait_for_events(cl_uint num_events,
                          const cl_event *event_list)
{
    cl_int err = CL_SUCCESS;

    if (event_list == NULL || num_events <= 0)
    {
        err = CL_INVALID_VALUE;
        goto exit;
    }

    for (int i = 0; i < num_events; i++)
    {
        if (!event_list[i])
        {
            err = CL_INVALID_EVENT;
            goto exit;
        }
        std::cout << "[clWaitForEvents] waiting for event : " << event_list[i]->ev << std::endl;
        mango_wait(event_list[i]->ev);
        std::cout << "[clWaitForEvents] finished waiting for event : " << event_list[i]->ev << std::endl;
    }

exit:
    return err;
}

cl_int cl_get_event_info(cl_event event,
                         cl_event_info param_name,
                         size_t param_value_size,
                         void *param_value,
                         size_t *param_value_size_ret)
{
    void *src_ptr = NULL;
    size_t src_size = 0;

    if (event == NULL)
        return CL_INVALID_EVENT;

    switch (param_name)
    {
    case CL_EVENT_COMMAND_QUEUE:
        src_ptr = &event->queue;
        src_size = sizeof(cl_command_queue);
        break;
    case CL_EVENT_CONTEXT:
        src_ptr = &event->ctx;
        src_size = sizeof(cl_context);
        break;
    case CL_EVENT_COMMAND_TYPE:
        src_ptr = &event->event_type;
        src_size = sizeof(cl_command_type);
        break;
    default:
        std::cout << "[clGetEventInfo] " << param_name << "is invalid or not supported yet\n";
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