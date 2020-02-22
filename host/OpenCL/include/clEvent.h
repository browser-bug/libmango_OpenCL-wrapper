#include "CL/cl.h"

#ifdef __cplusplus
#include <stdio.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    struct _cl_event
    {
        cl_context ctx;         /* the context associated with the event */
        cl_command_queue queue; /* the command queue associated with the event */

        mango_event_t ev; /* the event structure in MANGO */

        cl_command_type event_type; /* event type */
    };

    cl_int cl_wait_for_events(cl_uint num_events,
                              const cl_event *event_list);

    cl_int cl_get_event_info(cl_event event,
                             cl_event_info param_name,
                             size_t param_value_size,
                             void *param_value,
                             size_t *param_value_size_ret);

#ifdef __cplusplus
}
#endif
