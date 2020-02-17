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
        uint32_t id;
        mango_kernel_t kernel;
        cl_device_id device; /* device associated with this kernel */

        mango_arg_t **args;
        int args_num;
    };

    cl_kernel cl_create_kernel(cl_program program,
                               const char *kernel_name, /* this is the binary path */
                               cl_int *errcode_ret,
                               cl_int kernel_id);

    cl_int cl_set_kernel_arg(cl_kernel kernel,
                             cl_uint arg_index,
                             size_t arg_size,
                             const void *arg_value,
                             cl_argument_type arg_type);

    cl_int cl_create_kernels_in_program(cl_program program,
                                        cl_uint num_kernels,
                                        cl_kernel *kernels,
                                        cl_uint *num_kernels_ret);

#ifdef __cplusplus
}
#endif
