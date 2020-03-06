#include "clDevice.h"

#include "clExceptions.h"

// typedef enum
// {
//     PEAK, /*!< PEAK units */
//     NUP,  /*!< NU+ units */
//     DCT,
//     GN,    /*!< Fall back to GN node if no other option is available */
//     GPGPU, /*!< GPU units */
//     ARM,   /*!< ARM units */
//     STOP,  /*!< Terminator used to close arrays of mango_unit_type_t */
// } mango_unit_type_t;

std::vector<mango_unit_type_t> getAvailableDevicesByType(cl_device_type device_type)
{
    std::vector<mango_unit_type_t> tempVect;

    if (device_type == CL_DEVICE_TYPE_ALL)
        tempVect = availableUnits;
    else if (device_type == CL_DEVICE_TYPE_CPU ||
             device_type == CL_DEVICE_TYPE_DEFAULT)
    {
        for (auto &unit : availableUnits)
        {
            if (unit == GN)
                tempVect.push_back(unit);
        }
    }
    else if (device_type == CL_DEVICE_TYPE_GPU)
    {
        for (auto &unit : availableUnits)
        {
            if (unit == GPGPU)
                tempVect.push_back(unit);
        }
    }
    else
    { // everything else
        for (auto &unit : availableUnits)
        {
            if (unit != GPGPU || unit != GN)
                tempVect.push_back(unit);
        }
    }

    return tempVect;
}

cl_int cl_get_device_ids(cl_device_type device_type,
                         cl_uint num_entries,
                         cl_device_id *devices,
                         cl_uint *num_devices)
{
    /* CL_DEVICE_TYPE_CUSTOM is not available for now */
    const cl_device_type validTypes = CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_CPU |
                                      CL_DEVICE_TYPE_ACCELERATOR | CL_DEVICE_TYPE_DEFAULT;

    if ((devices == NULL && num_devices == NULL) ||
        (devices && num_entries == 0))
        return CL_INVALID_VALUE;
    if ((device_type & validTypes) == 0)
        return CL_INVALID_DEVICE_TYPE;

    std::vector<mango_unit_type_t> available_devices = getAvailableDevicesByType(device_type);
    cl_uint total_num = available_devices.size();

    if (total_num == 0)
        return CL_DEVICE_NOT_FOUND;
    if (num_entries > total_num)
    {
        std::cout << "[clGetDeviceIDs] num_entries must be <= total devices\n";
        return CL_INVALID_VALUE;
    }
    if (num_devices)
        *num_devices = total_num;
    if (devices)
    {
        for (int i = 0; i < num_entries; i++)
        {
            devices[i] = new _cl_device_id(nullptr);
            devices[i]->device_type = available_devices.at(i);
        }
    }

    return CL_SUCCESS;
}

// TODO : to be tested
cl_int cl_get_device_info(cl_device_id device,
                          cl_device_info param_name,
                          size_t param_value_size,
                          void *param_value,
                          size_t *param_value_size_ret)
{
    const void *src_ptr = NULL;
    size_t src_size = 0;

    if (device == NULL)
        return CL_INVALID_DEVICE;

    switch (param_name)
    {
    case CL_DEVICE_TYPE:
        src_ptr = &device->device_type;
        src_size = sizeof(device->device_type);
        break;
    default:
        std::cout << "[clGetDeviceInfo] " << param_name << "is invalid or not supported yet\n";
        return CL_INVALID_VALUE;
    }

    if (param_value && param_value_size < src_size)
        return CL_INVALID_VALUE;
    if (param_value && param_value_size)
        memcpy(param_value, src_ptr, src_size);
    if (param_value_size_ret)
        *param_value_size_ret = src_size;
    return CL_SUCCESS;
}
