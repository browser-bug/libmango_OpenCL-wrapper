#include "clContext.h"

#include "clDevice.h"
#include "clExceptions.h"

#include <string>

// Support function for clCreateContext
/* 
The core structure of this code comes from the Stack Overflow Network.
Link to the original answer/question: https://stackoverflow.com/questions/15545341/process-name-from-its-pid-in-linux/22214304#22214304
Author: Martin York https://stackoverflow.com/users/341032/qjgui
 */
const char *getProcessNameByPid(const int pid)
{
    char *name = (char *)calloc(1024, sizeof(char));
    if (name)
    {
        sprintf(name, "/proc/%d/cmdline", pid);
        FILE *f = fopen(name, "r");
        if (f)
        {
            size_t size;
            size = fread(name, sizeof(char), 1024, f);
            if (size > 0)
            {
                if ('\n' == name[size - 1])
                    name[size - 1] = '\0';
            }
            name += 2; // removing first two characters (es. './')
            fclose(f);
        }
    }
    assert(name && "error in retrieving process_name, try again");
    return name;
}

cl_context cl_create_context(cl_uint num_devices,
                             const cl_device_id *devices,
                             void *mango_receipe,
                             cl_int *errcode_ret)
{
    cl_context context = NULL;

    assert(mango_receipe && "user_data must specify a mango_receipe");
    const int pid = getpid();
    const char *process_name = getProcessNameByPid(pid);
    const char *receipe = (char *)mango_receipe;

    if (devices == NULL)
        throw cl_error(CL_INVALID_DEVICE);

    if (mango_init(process_name, receipe) != SUCCESS)
        throw cl_error(CL_INVALID_VALUE);

    context = new _cl_context(nullptr, nullptr);
    context->devices = std::vector<cl_device_id>(devices, devices + num_devices);

    if (errcode_ret)
        *errcode_ret = CL_SUCCESS;
    return context;
}

// TODO : to be tested
cl_context cl_create_context_from_type(cl_device_type device_type,
                                       void *mango_receipe,
                                       cl_int *errcode_ret)
{
    cl_context context = NULL;
    cl_int err = CL_SUCCESS;
    cl_device_id *devices = NULL;
    cl_uint num_devices = 0;
    const cl_device_type valid_type = CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_ACCELERATOR | CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_CUSTOM | CL_DEVICE_TYPE_DEFAULT;

    if ((device_type & valid_type) == 0)
        throw cl_error(CL_INVALID_DEVICE_TYPE);

    // get devices num first
    err = cl_get_device_ids(device_type, 0, NULL, &num_devices);
    if (err != CL_SUCCESS)
        throw cl_error(err);

    assert(num_devices > 0);
    devices = new cl_device_id[num_devices];

    err = cl_get_device_ids(device_type, num_devices, devices, &num_devices);
    throw cl_error(err);

    context = cl_create_context(num_devices, devices, mango_receipe, &err);

    if (devices)
        delete[] devices;
    if (errcode_ret)
        *errcode_ret = err;
    return context;
}

// TODO : to be tested
cl_int cl_get_context_info(cl_context context,
                           cl_context_info param_name,
                           size_t param_value_size,
                           void *param_value,
                           size_t *param_value_size_ret)
{
    const void *src_ptr = NULL;
    size_t src_size = 0;
    cl_uint n, ref;

    if (context == NULL)
        return CL_INVALID_CONTEXT;

    switch (param_name)
    {
    case CL_CONTEXT_DEVICES:
        src_ptr = &context->devices[0];
        src_size = sizeof(cl_device_id) * context->devices.size();
        break;
    case CL_CONTEXT_NUM_DEVICES:
        n = context->devices.size();
        src_ptr = &n;
        src_size = sizeof(cl_uint);
        break;
    default:
        std::cout << "[clGetContextInfo] " << param_name << "is invalid or not supported yet\n";
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