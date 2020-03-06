#include "CL/cl.h"

#ifdef __cplusplus
#include <stdio.h>
#include <vector>
#include <unistd.h> /* To get process IDs etc. */
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    struct _cl_context
    {
        _cl_context(
            cl_command_queue q,
            cl_program prog)
            : queue(q), program(prog) {}

        cl_command_queue queue; /* this corresponds to the task_graph in MANGO */
        cl_program program;     /* the program associated with this context */

        std::vector<cl_device_id> devices; /* all devices associated with this context */
        std::vector<cl_mem> mem_objects;   /* all memory objects associated with this context */
    };

    cl_context cl_create_context(cl_uint num_devices,
                                 const cl_device_id *devices,
                                 void *mango_receipt,
                                 cl_int *errcode_ret);

    cl_context cl_create_context_from_type(cl_device_type device_type,
                                           void *mango_receipt,
                                           cl_int *errcode_ret);

    cl_int cl_get_context_info(cl_context context,
                               cl_context_info param_name,
                               size_t param_value_size,
                               void *param_value,
                               size_t *param_value_size_ret);

#ifdef __cplusplus
}
#endif
