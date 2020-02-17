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
        mango_event_t ev;

        cl_context ctx;
    };

    cl_int cl_wait_for_events(cl_uint num_events,
                              const cl_event *event_list);

#ifdef __cplusplus
}
#endif
