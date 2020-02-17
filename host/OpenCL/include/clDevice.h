#include "CL/cl.h"

#ifdef __cplusplus
#include <stdio.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    struct _cl_device_id
    {
        mango_unit_type_t device_type;
        cl_command_queue queue; /* command_queue associated with this device */
    };

    cl_int cl_get_device_ids(cl_platform_id platform,
                             cl_device_type device_type,
                             cl_uint num_entries,
                             cl_device_id *devices,
                             cl_uint *num_devices);

#ifdef __cplusplus
}
#endif
