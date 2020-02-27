#include "clKernel.h"

#include "clProgram.h"
#include "clDevice.h"
#include "clContext.h"
#include "clCommandQueue.h"
#include "clMem.h"
#include "clEvent.h"

cl_kernel cl_create_kernel(cl_program program,
                           const char *binary_path,
                           cl_int *errcode_ret,
                           cl_int kernel_id)
{
    cl_kernel kernel = NULL;
    cl_int err = CL_SUCCESS;
    mango_kernel_function kern_funct;
    int i;

    if (program == NULL)
    {
        err = CL_INVALID_PROGRAM;
        goto exit;
    }
    if (binary_path == NULL)
    {
        std::cout << "[clCreateKernel] the binary path must be specified.\n";
        err = CL_INVALID_VALUE;
        goto exit;
    }
    if (program->map_kernel_functions.empty())
    {
        err = CL_INVALID_PROGRAM_EXECUTABLE;
        goto exit;
    }
    for (const auto& kv : program->map_kernel_functions) {
    if (strcmp(kv.second.binary, binary_path) == 0)
            break;
    }/*
    for (i = 0; i < program->map_kernel_functions.size(); i++)
    {
        if (strcmp(program->kernel_functions[].binary, binary_path) == 0)
            break;
    }*/
    if (i == program->map_kernel_functions.size())
    {
        err = CL_INVALID_KERNEL_NAME;
        goto exit;
    }

    kernel = new (std::nothrow) _cl_kernel;
    if (!kernel)
    {
        err = CL_OUT_OF_HOST_MEMORY;
        goto exit;
    }

    kernel->id = kernel_id;
    kern_funct = program->map_kernel_functions[binary_path];
    kernel->device = kern_funct.device;
    kernel->program = program;

    kernel->args.clear();

    kernel->kernel = mango_register_kernel_with_buffers(kernel->id,
                                                        kern_funct.function,
                                                        &kern_funct.buffers_in,
                                                        &kern_funct.buffers_out);

    // Get the task graph associated with the kernel (via its device)
    kernel->device->queue->tgx = mango_task_graph_add_kernel(kernel->device->queue->tgx, &(kernel->kernel));
    std::cout << "[TASK_GRAPH] added new kernel to tg (address) : " << kernel->device->queue->tgx << std::endl;

exit:
    if (errcode_ret)
        *errcode_ret = err;
    return kernel;
}

cl_int cl_set_kernel_arg(cl_kernel kernel,
                         cl_uint arg_index,
                         size_t arg_size,
                         const void *arg_value,
                         cl_argument_type arg_type)
{
    cl_int err = CL_SUCCESS;
    const void *value = NULL;
    mango_buffer_type_t argument_type;
    mango_arg_t *arg = NULL;

    if (kernel == NULL)
    {
        err = CL_INVALID_KERNEL;
        goto exit;
    }
    // This is not needed since in our case we have kernels generated from binaries,
    // so no informations (a priori) on the arguments etc.
    // if (arg_index >= kernel->args.size())
    // {
    //     err = CL_INVALID_ARG_INDEX;
    //     goto exit;
    // }
    if (arg_size == 0)
    {
        err = CL_INVALID_ARG_SIZE;
        goto exit;
    }
    if (arg_value == NULL)
    {
        err = CL_INVALID_ARG_VALUE;
        goto exit;
    }

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
        *argEvent = new (std::nothrow) _cl_event;
        if (!(*argEvent))
        {
            err = CL_OUT_OF_HOST_MEMORY;
            goto exit;
        }
        /* in this case the arg_index refers to the buffer which the event is associated */
        (*argEvent)->ev = mango_get_buffer_event(((mango::Arg *)(kernel->args[arg_index]))->get_id());
        (*argEvent)->ctx = kernel->device->queue->ctx;
        value = &((*argEvent)->ev);

        std::cout << "[clSetKernelArg] creating new mango_arg for mango_event (" << arg_value << ") with value: " << (*(uint32_t *)value) << std::endl;
        break;
    }

    default:
        err = CL_INVALID_VALUE;
        std::cout << "[clSetKernelArg] invalid kernel argument type\n";
        goto exit;
    }

    /* allocate new mango_arg and add it to the kernel structure */
    arg = mango_arg(kernel->kernel, value, arg_size, argument_type);
    kernel->args.push_back(arg);

exit:
    return err;
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