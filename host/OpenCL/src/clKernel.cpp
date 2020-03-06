#include "clKernel.h"

#include "clProgram.h"
#include "clDevice.h"
#include "clContext.h"
#include "clCommandQueue.h"
#include "clMem.h"
#include "clEvent.h"
#include "clExceptions.h"

cl_kernel cl_create_kernel(cl_program program,
                           const char *binary_path,
                           cl_int *errcode_ret,
                           cl_int kernel_id)
{
    cl_kernel kernel = NULL;

    mango_kernel_function kern_funct;
    int i;

    if (program == NULL)
        throw cl_error(CL_INVALID_PROGRAM);

    if (binary_path == NULL)
    {
        std::cout << "[clCreateKernel] the binary path must be specified.\n";
        throw cl_error(CL_INVALID_VALUE);
    }

    if (program->map_kernel_functions.empty())
        throw cl_error(CL_INVALID_PROGRAM_EXECUTABLE);

    if (program->map_kernel_functions.find(binary_path) == program->map_kernel_functions.end())
        throw cl_error(CL_INVALID_KERNEL_NAME);

    kern_funct = program->map_kernel_functions[binary_path];

    kernel = new _cl_kernel(kern_funct.device, program);
    kernel->id = kernel_id;

    kernel->kernel = mango_register_kernel_with_buffers(kernel->id,
                                                        kern_funct.function,
                                                        &kern_funct.buffers_in,
                                                        &kern_funct.buffers_out);

    // Get the task graph associated with the kernel (via its device)
    std::cout << "From new kernel: " << kernel->device->queue->tgx << "\t from old kern_funct" << kern_funct.device->queue->tgx << std::endl;
    kernel->device->queue->tgx = mango_task_graph_add_kernel(kernel->device->queue->tgx, &(kernel->kernel));

    if (errcode_ret)
        *errcode_ret = CL_SUCCESS;
    return kernel;
}

cl_int cl_set_kernel_arg(cl_kernel kernel,
                         cl_uint arg_index,
                         size_t arg_size,
                         const void *arg_value,
                         cl_argument_type arg_type)
{
    const void *value = NULL;
    mango_buffer_type_t argument_type;
    mango_arg_t *arg = NULL;

    if (kernel == NULL)
        throw cl_error(CL_INVALID_KERNEL);

    // This is not needed since in our case we have kernels generated from binaries,
    // so no informations (a priori) on the arguments etc.
    // if (arg_index >= kernel->args.size())
    //  throw cl_error(CL_INVALID_ARG_INDEX);

    if (arg_size == 0)
        throw cl_error(CL_INVALID_ARG_SIZE);

    if (arg_value == NULL)
        throw cl_error(CL_INVALID_ARG_VALUE);

    switch (arg_type)
    {
    case CL_SCALAR_ARG:
    {
        switch (arg_size)
        {
        case sizeof(int):
            arg_size = sizeof(uint32_t);
            break;
        case sizeof(long):
            arg_size = sizeof(uint64_t);
            break;
        case sizeof(short):
            arg_size = sizeof(u_int16_t);
            break;

        default: // TODO when can we return CL_INVALID_ARG_SIZE?
            arg_size = sizeof(u_int32_t);
            break;
        }

        argument_type = SCALAR;
        value = arg_value;

        std::cout << "[clSetKernelArg] creating new mango_arg for mango_scalar with value: " << (*(uint32_t *)value) << std::endl;
        break;
    }

    case CL_BUFFER_ARG:
    {
        arg_size = sizeof(uint64_t); // this is not needed in mango_arg
        argument_type = BUFFER;

        value = &((*(cl_mem *)arg_value)->buffer);

        std::cout << "[clSetKernelArg] creating new mango_arg for mango_buffer (" << arg_value << ") with value: " << (*(uint32_t *)value) << std::endl;
        break;
    }

    case CL_EVENT_ARG:
    {
        arg_size = sizeof(uint64_t); // this is not needed in mango_arg
        argument_type = EVENT;

        cl_event *argEvent = (cl_event *)arg_value;
        *argEvent = new _cl_event(kernel->device->queue->ctx, kernel->device->queue);

        /* in this case the arg_index refers to the buffer which the event is associated with */
        (*argEvent)->ev = mango_get_buffer_event(((mango::Arg *)(kernel->args[arg_index]))->get_id());
        value = &((*argEvent)->ev);

        std::cout << "[clSetKernelArg] creating new mango_arg for mango_event (" << arg_value << ") with value: " << (*(uint32_t *)value) << std::endl;
        break;
    }

    default:
        std::cout << "[clSetKernelArg] invalid kernel argument type\n";
        throw cl_error(CL_INVALID_VALUE);
    }

    /* allocate new mango_arg and add it to the kernel structure */
    arg = mango_arg(kernel->kernel, value, arg_size, argument_type);
    kernel->args.push_back(arg);

    return CL_SUCCESS;
}

// TODO : to be tested
cl_int cl_get_kernel_info(cl_kernel kernel,
                          cl_kernel_info param_name,
                          size_t param_value_size,
                          void *param_value,
                          size_t *param_value_size_ret)
{
    const void *src_ptr = NULL;
    size_t src_size = 0;
    const char *str = NULL;
    cl_int ref;
    cl_uint n;

    if (kernel == NULL)
        return CL_INVALID_KERNEL;

    switch (param_name)
    {
    case CL_KERNEL_CONTEXT:
        src_ptr = &kernel->program->ctx;
        src_size = sizeof(cl_context);
        break;
    case CL_KERNEL_PROGRAM:
        src_ptr = &kernel->program;
        src_size = sizeof(cl_program);
        break;
    case CL_KERNEL_NUM_ARGS:
        n = kernel->args.size();
        src_ptr = &n;
        src_size = sizeof(cl_uint);
        break;
    default:
        std::cout << "[clGetKernelInfo] " << param_name << "is invalid or not supported yet\n";
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