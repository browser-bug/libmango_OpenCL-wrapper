#include "CL/cl.h"

#ifdef __cplusplus
#include <stdio.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    struct _cl_mem
    {
        uint32_t id;
        mango_buffer_t buffer;
        void *host_ptr;
        int type;

        cl_context ctx;
    };

    cl_mem cl_create_buffer(cl_context context,
                            cl_mem_flags flags,
                            size_t size,
                            void *host_ptr,
                            cl_int *errcode_ret,
                            cl_int num_kernels_in,
                            cl_kernel *kernels_in,
                            cl_int num_kernels_out,
                            cl_kernel *kernels_out,
                            cl_int buffer_id);

#ifdef __cplusplus
}
#endif
