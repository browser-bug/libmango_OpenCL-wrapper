#include "clKernel.h"

#include "clProgram.h"
#include "clDevice.h"
#include "clContext.h"
#include "clCommandQueue.h"
#include "clMem.h"
#include "clEvent.h"

cl_kernel cl_create_kernel(cl_program program,
                           const char *kernel_name, /* this is the binary path */
                           cl_int *errcode_ret,
                           cl_int kernel_id)
{
    // TODO: check that all devices are associated with the correct context
    assert(kernel_name && "the binary path must be specified");

    cl_kernel kernel = NULL;
    kernel = (cl_kernel)malloc(sizeof(struct _cl_kernel));

    int i = 0;
    mango_exit_t err;
    for (int i = 0; i < program->num_kernel_functions; i++)
    {
        if (strcmp(program->kernel_functions[i].binary, kernel_name) == 0)
            break;
    }
    if (i == program->num_kernel_functions)
    {
        printf("[clCreateKernel] kernel not found\n");
        free(kernel);
        kernel = NULL;
        return kernel;
    }

    kernel->id = kernel_id;
    mango_kernel_function kern_funct = program->kernel_functions[i];
    kernel->device = kern_funct.device;

    kernel->args = NULL;
    kernel->args_num = 0;

    // FIX : maybe move this copy part in the mango_register_kernel_with_buffers()
    std::vector<uint32_t> buf_in(kern_funct.buffers_in, kern_funct.buffers_in + kern_funct.num_buffers_in);
    std::vector<uint32_t> buf_out(kern_funct.buffers_out, kern_funct.buffers_out + kern_funct.num_buffers_out);

    kernel->kernel = mango_register_kernel_with_buffers(kernel->id,
                                                        kern_funct.function,
                                                        &buf_in,
                                                        &buf_out);

    // Get the task graph associated with the kernel (via its device)
    kernel->device->queue->tgx = mango_task_graph_add_kernel(kernel->device->queue->tgx, &(kernel->kernel));
    std::cout << "[TASK_GRAPH] added new kernel to tg (address) : " << kernel->device->queue->tgx << std::endl;

    return kernel;
}

cl_int cl_set_kernel_arg(cl_kernel kernel,
                      cl_uint arg_index,
                      size_t arg_size,
                      const void *arg_value,
                      cl_argument_type arg_type)
{
    mango_buffer_type_t argument_type;
    const void *value;

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

        default: // error needed ?
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
        *argEvent = (cl_event)malloc(sizeof(struct _cl_event));
        (*argEvent)->ev = mango_get_buffer_event(((mango::Arg *)(kernel->args[arg_index]))->get_id());
        (*argEvent)->ctx = kernel->device->queue->ctx;
        value = &((*argEvent)->ev);
        std::cout << "[clSetKernelArg] creating new mango_arg for mango_event (" << arg_value << ") with value: " << (*(uint32_t *)value) << std::endl;
        break;
    }

    default:
        return CL_BUILD_ERROR;
    }

    mango_arg_t *arg = mango_arg(kernel->kernel, value, arg_size, argument_type);

    if (kernel->args != NULL)
        kernel->args = (mango_arg_t **)realloc(kernel->args, sizeof(mango_arg_t *) * (kernel->args_num + 1)); // FIX : maybe reallocation can be smarter, *2 just when necessary instead of +1 everytime
    else                                                                                                      // initialize kernel args
        kernel->args = (mango_arg_t **)calloc(1, sizeof(mango_arg_t *));

    kernel->args[kernel->args_num] = arg;
    kernel->args_num++;

    return CL_SUCCESS;
}

// FIX: to be implemented for future use. Partial implementation already present.
/* 
cl_int cl_create_kernels_in_program(cl_program program,
                                    cl_uint num_kernels,
                                    cl_kernel *kernels,
                                    cl_uint *num_kernels_ret)
{
    if (!program)
        return CL_INVALID_PROGRAM;
    if (program->kernels != NULL && program->num_kernel_functions <= 0)
        return CL_INVALID_PROGRAM_EXECUTABLE;
    if (kernels && num_kernels < program->num_kernel_functions)
        return CL_INVALID_VALUE;

    if (num_kernels_ret)
        *num_kernels_ret = program->num_kernel_functions;

    if (kernels)
    {
        for (int i = 0; i < program->num_kernel_functions; i++)
        {
            assert(program->kernel_functions[i].buffers_in && program->kernel_functions[i].buffers_out && "input and output buffers must be set first");
            kernels[i] = (cl_kernel)malloc(sizeof(struct _cl_kernel));
            kernels[i]->id = kernel_id; // FIX : dare anche qui la possibilità all'utente di specificare l'id? Se sì, come?
            kernel_id++;

            // FIX : maybe move this copy part in the mango_register_kernel_with_buffers()
            std::vector<uint32_t> buf_in(program->kernel_functions[i].buffers_in, program->kernel_functions[i].buffers_in + program->kernel_functions[i].num_buffers_in);
            std::vector<uint32_t> buf_out(program->kernel_functions[i].buffers_out, program->kernel_functions[i].buffers_out + program->kernel_functions[i].num_buffers_out);

            kernels[i]->kernel_function = program->kernel_functions[i];
            kernels[i]->args = NULL;
            kernels[i]->args_num = 0;
            kernels[i]->kernel_function.in_buffer_register_id = 0;
            kernels[i]->kernel_function.out_buffer_register_id = 0;
            kernels[i]->device = program->kernel_functions[i].device;

            // TODO : convert the two buffer arrays into vectors and modify mango_register_kernel_with_buffers to COPY
            kernels[i]->kernel = mango_register_kernel_with_buffers(kernels[i]->id,
                                                                    program->kernel_functions[i].function,
                                                                    &buf_in,
                                                                    &buf_out);

            // Get the task graph associated with the kernel (via its device)
            kernels[i]->device->queue->tgx = kernels[i]->device->queue->tgx
                                                 ? mango_task_graph_add_kernel(kernels[i]->device->queue->tgx, &(kernels[i]->kernel))
                                                 : mango_task_graph_add_kernel(NULL, &(kernels[i]->kernel));

            std::cout << "[CreateKernelsInProgram] Initializing kernel with id: " << i << std::endl;
            std::cout << "[TASK_GRAPH] added new kernel to tg (address) : " << kernels[i]->device->queue->tgx << std::endl;
        }
        program->kernels = kernels;
    }
    return CL_SUCCESS;
} */