#include "clProgram.h"

#include "clContext.h"
#include "clDevice.h"

#include <algorithm>

/* checks that all devices in device_list are inside the context */
bool checkDevicesInContext(cl_context ctx, const cl_device_id *device_list, cl_uint num_devices)
{
    if (num_devices <= 0)
        return false;

    for (int i = 0; i < num_devices; i++)
    {
        if (std::find(ctx->devices.begin(), ctx->devices.end(), device_list[i]) == ctx->devices.end())
            return false;
    }
    return true;
}

cl_program cl_create_program_with_binary(cl_context context,
                                         cl_uint num_devices,
                                         const cl_device_id *device_list,
                                         const char **binaries,
                                         cl_int *binary_status,
                                         cl_int *errcode_ret)
{
    cl_program program = NULL;
    cl_int err = CL_SUCCESS;

    mango_exit_t mango_err;        /* stores errors from mango function calls */
    mango_unit_type_t device_type; /* stores mango unit type of each device */
   //mango_kernel_function tempKN;  /* temp. variable storing the kernel function data */
    if (context == NULL)
    {
        err = CL_INVALID_CONTEXT;
        goto exit;
    }
    if (device_list == NULL || num_devices <= 0)
    {
        err = CL_INVALID_DEVICE;
        goto exit;
    }
    if (!checkDevicesInContext(context, device_list, num_devices))
    {
        err = CL_INVALID_DEVICE;
        goto exit;
    }
    if (binaries == NULL)
    {
        err = CL_INVALID_VALUE;
        if (binary_status)
            binary_status[0] = CL_INVALID_VALUE;
        goto exit;
    }

    program = new (std::nothrow) _cl_program();
    if (!program)
    {
        err = CL_OUT_OF_HOST_MEMORY;
        goto exit;
    }

    for (int i = 0; i < num_devices; i++)
    {
        std::cout << "[clCreateProgramWithBinary] initializing new kernel for device_type: " << device_list[i]->device_type << std::endl;

        device_type = device_list[i]->device_type;
        const char *path = binaries[i];


        (program->map_kernel_functions)[path].function = mango_kernelfunction_init();
        (program->map_kernel_functions)[path].device = device_list[i];
        (program->map_kernel_functions)[path].binary = binaries[i];
        (program->map_kernel_functions)[path].buffers_in.clear();
        (program->map_kernel_functions)[path].buffers_out.clear();
        /* loading and initializing kernel data structure in MANGO */
        mango_err = mango_load_kernel(binaries[i], (program->map_kernel_functions)[path].function, device_type, BINARY);

        if (mango_err != SUCCESS)
        {
            program->map_kernel_functions.clear();

            if (binary_status)
                binary_status[i] = CL_INVALID_BINARY;
            err = CL_INVALID_BINARY;
            delete program;

            goto exit;
        }

        if (binary_status)
            binary_status[i] = CL_SUCCESS;
    }

    /* if everything went fine, we add the program into its coresponding context and viceversa*/
    context->program = program;
    program->ctx = context;

exit:
    if (errcode_ret)
        *errcode_ret = err;
    return program;
}

// TODO : to be tested
cl_int cl_get_program_info(cl_program program,
                           cl_program_info param_name,
                           size_t param_value_size,
                           void *param_value,
                           size_t *param_value_size_ret)
{
    const void *src_ptr = NULL;
    size_t src_size = 0;
    cl_uint num_dev, kernels_num;

    if (program == NULL)
        return CL_INVALID_PROGRAM;

    switch (param_name)
    {
    case CL_PROGRAM_CONTEXT:
        src_ptr = &program->ctx;
        src_size = sizeof(cl_context);
        break;
    case CL_PROGRAM_NUM_DEVICES:
        num_dev = program->ctx->devices.size();
        src_ptr = &num_dev;
        src_size = sizeof(cl_uint);
        break;
    case CL_PROGRAM_DEVICES:
        src_ptr = &program->ctx->devices[0];
        src_size = program->ctx->devices.size() * sizeof(cl_device_id);
        break;
    case CL_PROGRAM_NUM_KERNELS:
        kernels_num = program->map_kernel_functions.size();
        src_ptr = &kernels_num;
        src_size = sizeof(cl_uint);
        break;
    case CL_PROGRAM_BINARY_SIZES:
        // TODO need to be implemented first defining the relation between kernels and binaries
        std::cout << "[clGetProgramInfo] CL_PROGRAM_BINARY_SIZES not implemented yet\n";
        return CL_INVALID_VALUE;
    case CL_PROGRAM_BINARIES:
        // TODO need to be implemented first defining the relation between kernels and binaries
        std::cout << "[clGetProgramInfo] CL_PROGRAM_BINARIES not implemented yet\n";
        return CL_INVALID_VALUE;
    default:
        std::cout << "[clGetProgramInfo] " << param_name << "is invalid or not supported yet\n";
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