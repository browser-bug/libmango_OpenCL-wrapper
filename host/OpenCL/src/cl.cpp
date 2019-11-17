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

    // FIX: this array must be populated with a mango function yet to be implemented
    std::array<mango_unit_type_t, 3> availableUnits = {GN, GPGPU, PEAK};

    std::vector<mango_arg_t *> bufferArguments;
    std::vector<mango_arg_t *> eventArguments;
    std::vector<cl_mem> hostBuffers;
    std::vector<cl_mem> eventBuffers;

    std::vector<uint32_t> buffer_in;
    std::vector<uint32_t> buffer_out;

    cl_int clSetInputBufferIDs(cl_program program, unsigned int nbuffers_in, ...)
    {
        buffer_in.clear();
        va_list list;
        va_start(list, nbuffers_in);
        for (unsigned int i = 0; i < nbuffers_in; i++)
        {
            uint32_t in_id = (uint32_t)va_arg(list, cl_uint);
            buffer_in.push_back(in_id);
        }
        va_end(list);
    }

    cl_int clSetOutputBufferIDs(cl_program program, unsigned int nbuffers_out, ...)
    {
        buffer_out.clear();
        va_list list;
        va_start(list, nbuffers_out);
        for (unsigned int i = 0; i < nbuffers_out; i++)
        {
            uint32_t out_id = (uint32_t)va_arg(list, cl_uint);
            buffer_out.push_back(out_id);
        }
        va_end(list);
    }

    /* Global TaskGraph */
    mango_task_graph_t *tg = NULL;

    struct _cl_context
    {
        cl_device_id *devices;
        cl_uint device_num;
        cl_command_queue *queues; /* all command queues currently associated with this context */
        cl_uint queue_num;
        cl_mem *mem_objects; /* all memory objects currently associated with this context */
        cl_uint mem_object_num;
        cl_program p; /* all programs associated with this context, for now: just one */
    };

    struct _cl_program
    {
        kernelfunction *kernfunc;
        mango_kernel_t kernel; // FIX: cambiare con un puntatore a cl_kernel
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

    struct _cl_device_id
    {
        mango_unit_type_t device_type;
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

    // support functions for devices
    cl_uint getNumDevicesByType(mango_unit_type_t type)
    {
        cl_uint counter = 0;
        for (auto &unit : availableUnits)
        {
            if (unit == type)
                counter++;
        }
        // std::cout << "retrieve " << counter << " devices of type " << type << std::endl;
        return counter;
    }

    cl_int clGetDeviceIDs(cl_platform_id platform,
                          cl_device_type device_type,
                          cl_uint num_entries,
                          cl_device_id *devices,
                          cl_uint *num_devices)
    {
        // printf("GetDeviceIDs is not implemented\n");
        // platform is ignored

        if (devices)
            assert(num_entries > 0 && "num_entries must be greater than zero");

        // cl_device_id availableDev[num_entries]; // TO BE REMOVED ???
        cl_uint available_dev = 0;

        switch (device_type)
        {
        case CL_DEVICE_TYPE_ALL:
            available_dev = availableUnits.size();
            if (num_devices)
                *num_devices = available_dev;
            if (devices)
            {
                assert(num_entries <= available_dev && "num_entries must be <= total devices");
                for (int i = 0; i < num_entries; i++)
                {
                    devices[i] = (cl_device_id)malloc(sizeof(struct _cl_device_id));
                    devices[i]->device_type = availableUnits.at(i);
                }
            }
            break;
        case CL_DEVICE_TYPE_CPU:
        type_default:
            available_dev = getNumDevicesByType(GN);
            if (num_devices)
                *num_devices = available_dev;
            if (devices)
            {
                assert(num_entries <= available_dev && "num_entries must be <= CPU devices");
                for (int i = 0; i < num_entries; i++)
                    if (availableUnits.at(i) == GN)
                    {
                        devices[i] = (cl_device_id)malloc(sizeof(struct _cl_device_id));
                        devices[i]->device_type = availableUnits.at(i);
                    }
            }
            break;

        case CL_DEVICE_TYPE_ACCELERATOR:
            available_dev = getNumDevicesByType(PEAK);
            if (num_devices)
                *num_devices = available_dev;
            if (devices)
            {
                assert(num_entries <= available_dev && "num_entries must be <= ACCELERATOR devices");
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

    // support functions for creating a context
    const char *get_process_name_by_pid(const int pid)
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
        return name;
    }

    cl_context clCreateContext(const cl_context_properties *properties,
                               cl_uint num_devices,
                               const cl_device_id *devices,
                               void(CL_CALLBACK *pfn_notify)(const char *, const void *, size_t, void *),
                               void *user_data,
                               cl_int *errcode_ret)
    {
        // FIX: user_data utilizzabile come receipe per mango_init (quali sono le possibili receipe?)
        const int pid = getpid();
        const char *process_name = get_process_name_by_pid(pid);
        assert(process_name && "error in retrieving process_name, try again");

        // get from user_date the receipe
        const char *receipe = (char *)user_data;

        if (mango_init(process_name, receipe) == SUCCESS)
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

    cl_program clCreateProgramWithBinary(cl_context context,
                                         cl_uint num_devices,
                                         const cl_device_id *device_list,
                                         const size_t *lengths, /* not needed */
                                         const char **binaries,
                                         cl_int *binary_status,
                                         cl_int *errcode_ret) /* optional */
    {
        // TODO: check that all devices are associated with the correct context
        assert(device_list && "device_list must be a non-NULL value");
        assert(binaries && "binaries cannot be a NULL pointer");
        if (num_devices <= 0 || !device_list)
        {
            if (errcode_ret)
                *errcode_ret = CL_INVALID_VALUE;
            return NULL;
        }

        // Allocating new program
        cl_program program = NULL;
        program = (cl_program)malloc(sizeof(struct _cl_program));
        program->kernfunc = mango_kernelfunction_init();

        mango_exit_t err;
        // char kernel_binary[] = "/opt/mango/usr/local/share/matrix_multiplication/matrix_multiplication_dev";
        for (int i = 0; i < num_devices; i++)
        {
            mango_unit_type_t device_type = device_list[i]->device_type; // TODO: find a way to extract the device from device_list
            std::cout << "[CREATE PROG. BINARY] loading new kernel for device_type: " << device_type << std::endl;
            switch (device_type)
            {
            case GN:
                err = mango_load_kernel(binaries[i], program->kernfunc, GN, BINARY);
                break;
            case PEAK:
                err = mango_load_kernel(binaries[i], program->kernfunc, PEAK, BINARY);
                break;
            case GPGPU:
                err = mango_load_kernel(binaries[i], program->kernfunc, GPGPU, BINARY);
                break;

            default:
                printf("The architecture is not currently supported\n");
                free(program);
                return NULL;
                break;
            }
            if (err != SUCCESS)
            {
                if (binary_status)
                    binary_status[i] = CL_INVALID_BINARY;
                if (errcode_ret)
                    *errcode_ret = CL_INVALID_BINARY;
                free(program);
                return NULL;
            }
            else // SUCCESS
            {
                if (binary_status)
                    binary_status[i] = CL_SUCCESS;
            }
        }

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

        // kernel->kernel = mango_register_kernel(kernel->id, program->kernfunc, 2, 1, B1, B2, B3);
        assert(!(buffer_in.empty() && buffer_out.empty()) && "input and output buffers must be set first");
        kernel->kernel = mango_register_kernel_with_buffers(kernel->id, program->kernfunc, &buffer_in, &buffer_out);

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
        mango_args_t *args = mango_set_args(kernel->kernel, 6, bufferArguments.at(0), bufferArguments.at(1), bufferArguments.at(2), bufferArguments.at(3), bufferArguments.at(4), arg_ev);
        printf("Succesfully created args\n");

        /* Write host->device buffers */
        for (auto &b : hostBuffers)
        {
            if (b->type == CL_MEM_READ_ONLY)
            {
                mango_write(b->host_buffer, b->buffer, DIRECT, 0); // TODO:  magari spostarlo in clCreateBuffer
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
