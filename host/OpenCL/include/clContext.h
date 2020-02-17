#include "CL/cl.h"

#ifdef __cplusplus
#include <stdio.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    struct _cl_context
    {
        cl_command_queue queue; /* the command queue corresponds to the TaskGraph in MANGO */
        cl_program program;     /* the program associated with this context */
        const cl_device_id *devices;
        cl_uint device_num;
        cl_mem *mem_objects; /* all memory objects currently associated with this context */
        cl_uint mem_object_num;
    };

    cl_context cl_create_context(const cl_context_properties *properties,
                                 cl_uint num_devices,
                                 const cl_device_id *devices,
                                 void(CL_CALLBACK *pfn_notify)(const char *, const void *, size_t, void *),
                                 void *user_data,
                                 cl_int *errcode_ret);

#ifdef __cplusplus
}
#endif
