#include "CL/cl.h"

#ifdef __cplusplus
#include <stdio.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    /* This maps a kernel function in MANGO */
    typedef struct
    {
        cl_device_id device; /* device associated with this kernel function */

        kernelfunction *function;

        const char *binary;                /* path to the program binary */
        std::vector<uint32_t> buffers_in;  /* function input buffers */
        std::vector<uint32_t> buffers_out; /* function output buffers */
    } mango_kernel_function;

    struct _cl_program
    {
        cl_context ctx; /* parent context */

        std::vector<mango_kernel_function> kernel_functions; /* all kernels included in the program */
    };

    cl_program cl_create_program_with_binary(cl_context context,
                                             cl_uint num_devices,
                                             const cl_device_id *device_list,
                                             const char **binaries,
                                             cl_int *binary_status,
                                             cl_int *errcode_ret);

    cl_int cl_get_program_info(cl_program program,
                               cl_program_info param_name,
                               size_t param_value_size,
                               void *param_value,
                               size_t *param_value_size_ret);

#ifdef __cplusplus
}
#endif
