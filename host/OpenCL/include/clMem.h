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
        cl_context ctx; /* the context associated with the mem */

        mango_buffer_t buffer;
        uint32_t id; /* buffer id specified during creation */

        cl_mem_object_type type; /* we only have CL_MEM_BUFFER_TYPE */
        void *host_ptr;          /* pointer of the host mem */
        cl_mem_flags flags;      /* flags that are specified during creation */
        size_t size;             /* requested size specified during creation */
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

    cl_int cl_get_mem_object_info(cl_mem memobj,
                                  cl_mem_info param_name,
                                  size_t param_value_size,
                                  void *param_value,
                                  size_t *param_value_size_ret);

#ifdef __cplusplus
}
#endif
