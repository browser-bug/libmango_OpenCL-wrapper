#include "cl.h"

#ifdef __cplusplus
#include <stdio.h>
#endif

// Support definitions to be eliminated
uint32_t kernel_id = 1;
uint32_t buffer_id = 1;
#define KID 1
#define B1 1
#define B2 2
#define B3 3

#ifdef __cplusplus
extern "C"
{
#endif

    std::vector<mango_arg_t *> bufferArguments;
    std::vector<mango_arg_t *> eventArguments;
    std::vector<cl_mem> hostBuffers;
    std::vector<cl_mem> eventBuffers;

    /* Global TaskGraph */
    mango_task_graph_t *tg = NULL;

    struct _cl_context
    {
        cl_program p;
    };

    struct _cl_program
    {
        kernelfunction *kernfunc;
        // FIX maybe it has to be moved
        mango_kernel_t kernel;
    };

    struct _cl_mem
    {
        uint32_t id;
        mango_buffer_t buffer;
        void *host_buffer;
        int type;
    };

    struct _cl_kernel
    {
        uint32_t id;
        mango_kernel_t kernel;
        std::vector<mango::Arg *> args;
    };

    struct _cl_event
    {
        mango_event_t ev;
    };

    struct _cl_command_queue
    {
        //TODO: spostare taskgraph globale qui dentro
    };

    /* HELPER FUNCTIONS */
    void createEventArguments(mango_buffer_t buffer, mango_kernel_t kernel)
    {
        mango_arg_t *arg_ev = NULL;
        cl_event buf_event = NULL;

        buf_event = (cl_event)malloc(sizeof(struct _cl_event));
        buf_event->ev = mango_get_buffer_event(buffer);

        std::cout << "[KERNEL " << kernel << "] Creating a new event for buffer ID: " << buffer << std::endl;
        arg_ev = mango_arg(kernel, &(buf_event->ev), sizeof(uint64_t), EVENT);
        eventArguments.push_back(arg_ev);
    }

    /* API IMPLEMENTATION */

    cl_int clGetPlatformIDs(cl_uint num_entries,
                            cl_platform_id *platforms,
                            cl_uint *num_platforms)
    {
        printf("GetPlatformIDs is not implemented\n");
        return CL_SUCCESS;
    }

    cl_int clGetDeviceIDs(cl_platform_id platform,
                          cl_device_type device_type,
                          cl_uint num_entries,
                          cl_device_id *devices,
                          cl_uint *num_devices)
    {
        printf("GetDeviceIDs is not implemented\n");
        return CL_SUCCESS;
    }

    cl_context clCreateContext(const cl_context_properties *properties,
                               cl_uint num_devices,
                               const cl_device_id *devices,
                               void(CL_CALLBACK *pfn_notify)(const char *, const void *, size_t, void *),
                               void *user_data,
                               cl_int *errcode_ret)
    {
        // FIX: the name "test" must be associated to something specific?
        // FIX: usare chiamata di sistema per nome processo prname ??
        // FIX: user_data utilizzabile come receipe per mango_init ?
        if (mango_init("test", "test_manga") == SUCCESS)
        {
            cl_context context = NULL;
            context = (cl_context)malloc(sizeof(struct _cl_context));
            return context;
        }
    }

    cl_command_queue clCreateCommandQueue(cl_context context,
                                          cl_device_id device,
                                          cl_command_queue_properties properties,
                                          cl_int *errcode_ret)
    {
        printf("CreateCommandQueue is not implemented\n");
        cl_command_queue commands;
        commands = (cl_command_queue)malloc(sizeof(struct _cl_command_queue));
        return commands;
    }

    /* FIX: is there a way to pass binaries as the filepath and not as a fread output? */
    cl_program clCreateProgramWithBinary(cl_context context,
                                         cl_uint num_devices,
                                         const cl_device_id *device_list,
                                         const size_t *lengths,
                                         const unsigned char **binaries,
                                         cl_int *binary_status,
                                         cl_int *errcode_ret)
    {
        cl_program program = NULL;
        program = (cl_program)malloc(sizeof(struct _cl_program));
        program->kernfunc = mango_kernelfunction_init();

        char kernel_binary[] = "/opt/mango/usr/local/share/matrix_multiplication/matrix_multiplication_dev";

        mango_load_kernel(kernel_binary, program->kernfunc, GN, BINARY);

        // Associate program with context
        context->p = program;
        return program;
    }

    cl_int clBuildProgram(cl_program program,
                          cl_uint num_devices,
                          const cl_device_id *device_list,
                          const char *options,
                          void(CL_CALLBACK *pfn_notify)(cl_program program, void *user_data),
                          void *user_data)
    {
        printf("BuildProgram is not implemented\n");
        return CL_SUCCESS;
    }

    cl_kernel clCreateKernel(cl_program program,
                             const char *kernel_name,
                             cl_int *errcode_ret)
    {
        cl_kernel kernel = NULL;
        kernel = (cl_kernel)malloc(sizeof(struct _cl_kernel));

        // FIX: mettere il kernel_id dentro program?
        kernel->id = kernel_id;
        kernel_id++;
        // FIX find a way to make a variadic call to mango_register_kernel
        // FIX: passare un puntatore a un vettore e modificare mango 
        kernel->kernel = mango_register_kernel(kernel->id, program->kernfunc, 2, 1, B1, B2, B3);

        tg = mango_task_graph_add_kernel(NULL, &(kernel->kernel));
        std::cout << "[TASK_GRAPH] added new kernel to tg (address) : " << tg << std::endl;
        program->kernel = kernel->kernel;

        *errcode_ret = CL_SUCCESS;
        return kernel;
    }

    cl_mem clCreateBuffer(cl_context context,
                          cl_mem_flags flags,
                          size_t size,
                          void *host_ptr,
                          cl_int *errcode_ret)
    {
        if (!context)
        {
            if (!errcode_ret)
                *errcode_ret = CL_INVALID_CONTEXT;
            std::cout << "[clCreateBuffer] invalid context" << std::endl;
            return NULL;
        }
        if (size == 0 || size > CL_DEVICE_MAX_MEM_ALLOC_SIZE)
        {
            if (!errcode_ret)
                *errcode_ret = CL_INVALID_BUFFER_SIZE;
            std::cout << "[clCreateBuffer] invalid buffer size" << std::endl;
            return NULL;
        }
        // TODO: CL_INVALID_HOST_PTR if host_ptr is NULL and CL_MEM_USE_HOST_PTR or CL_MEM_COPY_HOST_PTR are set in flags or
        //       if host_ptr is not NULL but CL_MEM_COPY_HOST_PTR or CL_MEM_USE_HOST_PTR are not set in flags.
        /* if ((!host_ptr && ((flags & CL_MEM_USE_HOST_PTR) == CL_MEM_USE_HOST_PTR || (flags & CL_MEM_COPY_HOST_PTR) == CL_MEM_COPY_HOST_PTR)) ||
            (host_ptr && ((flags & CL_MEM_COPY_HOST_PTR) != CL_MEM_COPY_HOST_PTR || (flags & CL_MEM_USE_HOST_PTR) != CL_MEM_COPY_HOST_PTR)))
        {
            if (!errcode_ret)
                *errcode_ret = CL_INVALID_HOST_PTR;
            std::cout << "[clCreateBuffer] invalid host ptr" << std::endl;
            return NULL;
        } */

        cl_mem memory = NULL;
        memory = (cl_mem)malloc(sizeof(struct _cl_mem));

        memory->id = buffer_id;
        buffer_id++;

        memory->host_buffer = host_ptr;

        // FIX this need to be generic
        // TODO: aggiungere magari un messaggio che informa che mango ingora i flag openCL
        if ((flags & CL_MEM_READ_WRITE) == CL_MEM_READ_WRITE)
        {
            // FIX: need to change behaviour in case of CL_MEM_READ_WRITE flag
            memory->buffer = mango_register_memory(memory->id, size, BUFFER, 1, 1, context->p->kernel, context->p->kernel);
            memory->type = CL_MEM_READ_WRITE;
        }
        else if ((flags & CL_MEM_WRITE_ONLY) == CL_MEM_WRITE_ONLY)
        {
            memory->buffer = mango_register_memory(memory->id, size, BUFFER, 1, 0, context->p->kernel);
            memory->type = CL_MEM_WRITE_ONLY;
        }
        else if ((flags & CL_MEM_READ_ONLY) == CL_MEM_READ_ONLY)
        {
            memory->buffer = mango_register_memory(memory->id, size, BUFFER, 0, 1, context->p->kernel);
            memory->type = CL_MEM_READ_ONLY;
        }
        else
        {
            if (!errcode_ret)
                *errcode_ret = CL_INVALID_VALUE;
            std::cout << "[clCreateBuffer] invalid flag value" << std::endl;
            return NULL;
        }

        if (tg)
            tg = mango_task_graph_add_buffer(tg, &(memory->buffer));
        else
            tg = mango_task_graph_add_buffer(NULL, &(memory->buffer));

        if (((flags & CL_MEM_USE_HOST_PTR) == CL_MEM_USE_HOST_PTR) || ((flags & CL_MEM_COPY_HOST_PTR) == CL_MEM_COPY_HOST_PTR))
        {
            hostBuffers.push_back(memory);
        }
        // FIX: find a more elegant way to do it. How can we know that the buffer will be protected by an event?
        if (!memory->host_buffer)
        {
            std::cout << "[BUFFER] pushing back for future event creating buffer ID: " << memory->id << std::endl;
            eventBuffers.push_back(memory);
        }

        return memory;
    }

    cl_int clSetKernelArg(cl_kernel kernel,
                          cl_uint arg_index,
                          size_t arg_size,
                          const void *arg_value)
    {
        mango_buffer_type_t arg_type;
        const void *value;

        switch (arg_size)
        {
        case sizeof(cl_mem):
            arg_size = sizeof(uint64_t);
            arg_type = BUFFER;
            value = &((*(cl_mem *)arg_value)->buffer);
            break;

        // FIX: mettere come default
        case sizeof(int):
            arg_size = sizeof(uint32_t);
            arg_type = SCALAR;
            value = arg_value;
            break;

        default:
            return CL_BUILD_ERROR;
        }

        // std::vector<mango::Arg *> args;
        // FIX arg_value must be the address of a mango_buffer_t
        std::cout << "Creating new mango_arg for mango_buffer with address: " << arg_value << " and value: " << (*(uint32_t *)value) << std::endl;
        mango_arg_t *arg = mango_arg(kernel->kernel, value, arg_size, arg_type);

        // std::cout << "created arg at address: " << arg << std::endl;
        bufferArguments.push_back(arg);
        // std::cout << "Added new argument: " << (mango_arg_t *)arguments.back() << " [VEC_SIZE] = " << arguments.size() << std::endl;

        return CL_SUCCESS;
    }

    cl_int clEnqueueNDRangeKernel(cl_command_queue command_queue,
                                  cl_kernel kernel,
                                  cl_uint work_dim,
                                  const size_t *global_work_offset,
                                  const size_t *global_work_size,
                                  const size_t *local_work_size,
                                  cl_uint num_events_in_wait_list,
                                  const cl_event *event_wait_list,
                                  cl_event *event)
    {
        /* Check for events */
        // if (event != NULL)
        // {
        // FIX: not his function, must be found a more elegant way.
        tg = mango_task_graph_add_event(tg, NULL);

        mango_arg_t *arg_ev = NULL;
        (*event) = (cl_event)malloc(sizeof(struct _cl_event));
        (*event)->ev = mango_get_buffer_event(eventBuffers.at(0)->buffer);
        arg_ev = mango_arg(kernel->kernel, &((*event)->ev), sizeof(uint64_t), EVENT);

        // /* Create event args for event protected buffers */
        // for (auto &eb : eventBuffers)
        // {
        //     createEventArguments(eb->buffer, kernel->kernel);
        // }

        printf("[BUFFER] Allocating new resources\n");
        mango_resource_allocation(tg);
        printf("[BUFFER] Allocation completed\n");

        /* Putting togheter the arguments */
        // TODO convert the vector data into the variadic parameter of mango_set_args

        std::cout << "Setting args for kernel: " << kernel->kernel << std::endl;
        // TODO:
        mango_args_t *args = mango_set_args(kernel->kernel, 6, bufferArguments.at(0), bufferArguments.at(1), bufferArguments.at(2), bufferArguments.at(3), bufferArguments.at(4), eventArguments.at(0));
        printf("Succesfully created args\n");

        /* Write host->device buffers */
        for (auto &b : hostBuffers)
        {
            if (b->type == CL_MEM_READ_ONLY)
            {
                mango_write(b->host_buffer, b->buffer, DIRECT, 0);
            }
        }

        /* spawn kernel */
        mango_event_t kernEvent = mango_start_kernel(kernel->kernel, args, NULL);

        /* wait for kernel completion */
        mango_wait(kernEvent);

        // } // event handler

        return CL_SUCCESS;
    }

    cl_int clEnqueueReadBuffer(cl_command_queue command_queue,
                               cl_mem buffer, /* kernel buffer to be read */
                               cl_bool blocking_read,
                               size_t offset,
                               size_t size,
                               void *ptr, /* host buffer */
                               cl_uint num_events_in_wait_list,
                               const cl_event *event_wait_list,
                               cl_event *event)
    {
        mango_wait((*event)->ev);
        mango_read(ptr, buffer->buffer, DIRECT, 0);

        return CL_SUCCESS;
    }

    cl_int clReleaseProgram(cl_program program)
    {
        mango_resource_deallocation(tg);
        mango_task_graph_destroy_all(tg);
        mango_release();
    }

#ifdef __cplusplus
}
#endif
