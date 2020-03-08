#include "clEvent.h"

#include "clExceptions.h"

// /*!
//  * \enum mango_event_status_t
//  * \brief States for events
//  */
// typedef enum MangoEventStatus { LOCK=0, READ, WRITE, END_FIFO_OPERATION } mango_event_status_t;

cl_int cl_wait_for_events(cl_uint num_events,
                          const cl_event *event_list)
{
    if (event_list == NULL || num_events <= 0)
        throw cl_error(CL_INVALID_VALUE);

    for (int i = 0; i < num_events; i++)
    {
        if (!event_list[i])
            throw cl_error(CL_INVALID_EVENT);

        if (event_list[i]->status == mango_event_status_t::LOCK) // this event has yet to finish so we wait
        {
            std::cout << "[clWaitForEvents] waiting for event : " << event_list[i]->ev << std::endl;
            switch (event_list[i]->event_type)
            {
            case CL_COMMAND_READ_BUFFER:
                mango_wait_state(event_list[i]->ev, mango_event_status_t::READ);
                event_list[i]->status = mango_event_status_t::READ;
                break;

            case CL_COMMAND_WRITE_BUFFER:
                mango_wait_state(event_list[i]->ev, mango_event_status_t::WRITE);
                event_list[i]->status = mango_event_status_t::WRITE;
                break;

            default: // In every other case we let MANGO decide the correct event status to wait
                mango_wait(event_list[i]->ev);
                event_list[i]->status = mango_event_status_t::END_FIFO_OPERATION;
                break;
            }
            std::cout << "[clWaitForEvents] finished waiting for event : " << event_list[i]->ev << std::endl;
        }
    }

    return CL_SUCCESS;
}

cl_int cl_get_event_info(cl_event event,
                         cl_event_info param_name,
                         size_t param_value_size,
                         void *param_value,
                         size_t *param_value_size_ret)
{
    void *src_ptr = NULL;
    size_t src_size = 0;
    cl_int status;

    if (event == NULL)
        return CL_INVALID_EVENT;

    switch (param_name)
    {
    case CL_EVENT_COMMAND_QUEUE:
        src_ptr = &event->queue;
        src_size = sizeof(cl_command_queue);
        break;
    case CL_EVENT_COMMAND_EXECUTION_STATUS:
        // TODO is this mapping with mango_event_status correct?
        switch (event->status)
        {
        case mango_event_status_t::LOCK:
            status = CL_QUEUED;
            break;

        default: // mango_event_status_t = READ | WRITE | END_FIFO_OPERATION
            status = CL_COMPLETE;
            break;
        }
        src_ptr = &status;
        src_size = sizeof(cl_int);
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