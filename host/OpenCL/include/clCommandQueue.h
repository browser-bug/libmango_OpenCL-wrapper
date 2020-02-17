#include "CL/cl.h"

#ifdef __cplusplus
#include <stdio.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    struct _cl_command_queue
    {
        mango_task_graph_t *tgx; /* task_graph associated with this command_queue */

        cl_device_id device; /* its device */
        cl_context ctx;      /* parent context */

        cl_event *events;
        int command_count;
        // cl_event events;            /* events of the enqueued commands in enqueue order */
        // volatile int command_count; /* counter for unfinished command enqueued */
        // volatile sync_item last_event;
    };

    cl_command_queue cl_create_command_queue(cl_context context,
                                             cl_device_id device,
                                             cl_command_queue_properties properties,
                                             cl_int *errcode_ret);

    cl_int cl_finish(cl_command_queue command_queue);

#ifdef __cplusplus
}
#endif
