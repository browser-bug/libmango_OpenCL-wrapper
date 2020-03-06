#include "CL/cl.h"

#ifdef __cplusplus
#include <stdio.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    struct _cl_kernel
    {
        _cl_kernel(
            cl_device_id d,
            cl_program p)
            : device(d), program(p) {}

        cl_device_id device; /* device associated with this kernel */
        cl_program program;  /* program that owns this structure */

        mango_kernel_t kernel;

        uint32_t id;                     /* kernel identifier */
        std::vector<mango_arg_t *> args; /* kernel arguments */
    };

    cl_kernel cl_create_kernel(cl_program program,
                               const char *binary_path,
                               cl_int *errcode_ret,
                               cl_int kernel_id);

    cl_int cl_set_kernel_arg(cl_kernel kernel,
                             cl_uint arg_index,
                             size_t arg_size,
                             const void *arg_value,
                             cl_argument_type arg_type);

    cl_int cl_get_kernel_info(cl_kernel kernel,
                              cl_kernel_info param_name,
                              size_t param_value_size,
                              void *param_value,
                              size_t *param_value_size_ret);

#ifdef __cplusplus
}
#endif
