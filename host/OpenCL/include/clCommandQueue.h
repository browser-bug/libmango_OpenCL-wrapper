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
        _cl_command_queue(
            cl_context ctx,
            cl_device_id dev)
            : ctx(ctx), device(dev) {}

        cl_context ctx;      /* parent context */
        cl_device_id device; /* its device */

        mango_task_graph_t *tgx = NULL; /* task_graph associated with this command_queue */

        std::vector<cl_event> events; /* enqueued events */
    };

    cl_command_queue cl_create_command_queue(cl_context context,
                                             cl_device_id device,
                                             cl_int *errcode_ret);

    cl_int cl_get_command_queue_info(cl_command_queue command_queue,
                                     cl_command_queue_info param_name,
                                     size_t param_value_size,
                                     void *param_value,
                                     size_t *param_value_size_ret);

    cl_int cl_finish(cl_command_queue command_queue);

#ifdef __cplusplus
}
#endif
