#include "clDevice.h"

// TODO: this array must be populated with a mango function yet to be implemented
std::array<mango_unit_type_t, 4> availableUnits = {GN, GN, GPGPU, PEAK};

// support function for clGetDeviceIDs
cl_uint getNumDevicesByType(mango_unit_type_t type)
{
    cl_uint counter = 0;
    for (auto &unit : availableUnits)
    {
        if (unit == type)
            counter++;
    }
    return counter;
}

cl_int cl_get_device_ids(cl_platform_id platform,
                         cl_device_type device_type,
                         cl_uint num_entries,
                         cl_device_id *devices,
                         cl_uint *num_devices)
{

    if (devices)
        assert(num_entries > 0 && "num_entries must be greater than zero");

    cl_uint total_num = 0;

    switch (device_type)
    {
    case CL_DEVICE_TYPE_ALL:
        total_num = availableUnits.size();
        if (num_devices)
            *num_devices = total_num;
        if (devices)
        {
            assert(num_entries <= total_num && "num_entries must be <= total devices");
            for (int i = 0; i < num_entries; i++)
            {
                devices[i] = (cl_device_id)malloc(sizeof(struct _cl_device_id));
                devices[i]->device_type = availableUnits.at(i);
            }
        }
        break;
    case CL_DEVICE_TYPE_CPU:
    type_default:
        total_num = getNumDevicesByType(GN);
        if (num_devices)
            *num_devices = total_num;
        if (devices)
        {
            assert(num_entries <= total_num && "num_entries must be <= CPU devices");
            for (int i = 0; i < num_entries; i++)
                if (availableUnits.at(i) == GN)
                {
                    devices[i] = (cl_device_id)malloc(sizeof(struct _cl_device_id));
                    devices[i]->device_type = availableUnits.at(i);
                }
        }
        break;
    case CL_DEVICE_TYPE_GPU:
        total_num = getNumDevicesByType(GPGPU); // FIX : tutti tipi che non sono GPU e CPU
        if (num_devices)
            *num_devices = total_num;
        if (devices)
        {
            assert(num_entries <= total_num && "num_entries must be <= GPU devices");
            for (int i = 0; i < num_entries; i++)
                if (availableUnits.at(i) == GPGPU)
                {
                    devices[i] = (cl_device_id)malloc(sizeof(struct _cl_device_id));
                    devices[i]->device_type = availableUnits.at(i);
                }
        }
        break;
    case CL_DEVICE_TYPE_ACCELERATOR:
        total_num = getNumDevicesByType(PEAK); // FIX : tutti tipi che non sono GPU e CPU
        if (num_devices)
            *num_devices = total_num;
        if (devices)
        {
            assert(num_entries <= total_num && "num_entries must be <= PEAK devices");
            for (int i = 0; i < num_entries; i++)
                if (availableUnits.at(i) == PEAK)
                {
                    devices[i] = (cl_device_id)malloc(sizeof(struct _cl_device_id));
                    devices[i]->device_type = availableUnits.at(i);
                }
        }
        break;

    case CL_DEVICE_TYPE_DEFAULT:
        goto type_default;
        break;

    default:
        return CL_INVALID_DEVICE_TYPE;
    }

    if (num_devices && *num_devices == 0)
        return CL_DEVICE_NOT_FOUND;

    return CL_SUCCESS;
}
