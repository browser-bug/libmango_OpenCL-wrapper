#include "CL/cl.h"

#ifdef __cplusplus
#include <stdio.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    // TODO: this list must be populated with a mango function yet to be implemented, this is just a possible scenario for testing purposes
    static std::vector<mango_unit_type_t> availableUnits = {GN, GN, GPGPU, PEAK};

    // TODO: could be populated with many more informations about the device in the future
    struct _cl_device_id
    {
        _cl_device_id(
            cl_command_queue q)
            : queue(q) {}
            
        cl_command_queue queue; /* command_queue associated with this device */

        mango_unit_type_t device_type;
    };

    cl_int cl_get_device_ids(cl_device_type device_type,
                             cl_uint num_entries,
                             cl_device_id *devices,
                             cl_uint *num_devices);

    cl_int cl_get_device_info(cl_device_id device,
                              cl_device_info param_name,
                              size_t param_value_size,
                              void *param_value,
                              size_t *param_value_size_ret);

#ifdef __cplusplus
}
#endif
