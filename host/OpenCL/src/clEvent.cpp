#include "clEvent.h"

cl_int cl_wait_for_events(cl_uint num_events,
                          const cl_event *event_list)
{
    if (num_events <= 0 || event_list == NULL)
        return CL_INVALID_VALUE;

    for (int i = 0; i < num_events; i++)
    {
        if (!event_list[i])
            return CL_INVALID_EVENT;
        std::cout << "[clWaitForEvents] waiting for event : " << event_list[i]->ev << std::endl;
        mango_wait(event_list[i]->ev);
        std::cout << "[clWaitForEvents] finished waiting for event : " << event_list[i]->ev << std::endl;
    }

    return CL_SUCCESS;
}