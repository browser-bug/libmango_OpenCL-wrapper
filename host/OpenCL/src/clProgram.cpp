#include "clProgram.h"

#include "clContext.h"
#include "clDevice.h"

cl_program cl_create_program_with_binary(cl_context context,
                                         cl_uint num_devices,
                                         const cl_device_id *device_list,
                                         const size_t *lengths, /* not needed */
                                         const char **binaries,
                                         cl_int *binary_status,
                                         cl_int *errcode_ret)
{
    // TODO: check that all devices are associated with the correct context
    assert(context && "context must be a valid pointer");
    assert(device_list && "device_list must be a non-NULL value");
    assert(binaries && "binaries cannot be a NULL pointer");

    if (num_devices <= 0)
    {
        if (errcode_ret)
            *errcode_ret = CL_INVALID_VALUE;
        return NULL;
    }

    // Allocating new program
    cl_program program = NULL;
    program = (cl_program)malloc(sizeof(struct _cl_program));
    program->kernel_functions = (mango_kernel_function *)calloc(num_devices, sizeof(mango_kernel_function)); // FIX : not quite sure of the correctness of this
    program->num_kernel_functions = num_devices;

    mango_exit_t err;
    for (int i = 0; i < num_devices; i++)
    {
        mango_unit_type_t device_type = device_list[i]->device_type;

        std::cout << "[clCreateProgramWithBinary] initializing new kernel for device_type: " << device_type << std::endl;
        program->kernel_functions[i].function = mango_kernelfunction_init();

        program->kernel_functions[i].device = device_list[i];
        program->kernel_functions[i].binary = binaries[i];
        program->kernel_functions[i].buffers_in = NULL;
        program->kernel_functions[i].buffers_out = NULL;

        err = mango_load_kernel(binaries[i], program->kernel_functions[i].function, device_type, BINARY);
        if (err != SUCCESS)
        {
            if (binary_status)
                binary_status[i] = CL_INVALID_BINARY;
            if (errcode_ret)
                *errcode_ret = CL_INVALID_BINARY;
            free(program->kernel_functions);
            free(program);
            return NULL;
        }
        else // SUCCESS
        {
            if (binary_status)
                binary_status[i] = CL_SUCCESS;
            if (errcode_ret)
                *errcode_ret = CL_SUCCESS;
        }
    }

    // Associate program with context
    context->program = program;
    return program;
}