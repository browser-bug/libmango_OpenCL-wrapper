#include "CL/cl.h"

#ifdef __cplusplus
#include <stdio.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        kernelfunction *function;
        uint32_t *buffers_in;
        int num_buffers_in;

        uint32_t *buffers_out;
        int num_buffers_out;

        const char *binary;
        cl_device_id device; /* device associated with this kernel function */
    } mango_kernel_function;

    struct _cl_program
    {
        cl_context ctx; /* parent context */
        mango_kernel_function *kernel_functions;
        std::vector<int> vector;
        cl_int num_kernel_functions;

        // cl_kernel *kernels; // FIX: not needed?
    };

    cl_program cl_create_program_with_binary(cl_context context,
                                             cl_uint num_devices,
                                             const cl_device_id *device_list,
                                             const size_t *lengths, /* not needed */
                                             const char **binaries,
                                             cl_int *binary_status,
                                             cl_int *errcode_ret);

#ifdef __cplusplus
}
#endif
