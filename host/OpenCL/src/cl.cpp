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

#define MAX_KERNEL_BUFFERS 15

#ifdef __cplusplus
extern "C"
{
#endif

    // FIX: this array must be populated with a mango function yet to be implemented
    std::array<mango_unit_type_t, 4> availableUnits = {GN, GN, GPGPU, PEAK};

    std::vector<mango_arg_t *> bufferArguments;
    std::vector<mango_arg_t *> eventArguments;
    std::vector<cl_mem> hostBuffers;
    std::vector<cl_mem> eventBuffers;

    std::vector<uint32_t> buffer_in;
    std::vector<uint32_t> buffer_out;

    /* Global TaskGraph */
    mango_task_graph_t *tg = NULL;

    struct _cl_context
    {
        cl_command_queue queue; /* the command queue corresponds to the TaskGraph in MANGO */
        cl_program p;           /* the program associated with this context */
        const cl_device_id *devices;
        cl_uint device_num;
        cl_mem *mem_objects; /* all memory objects currently associated with this context */
        cl_uint mem_object_num;
    };

    typedef struct
    {
        kernelfunction *function;
        uint32_t *buffers_in;
        int num_buffers_in;
        int in_buffer_register_id;
        uint32_t *buffers_out;
        int num_buffers_out;
        int out_buffer_register_id;

        cl_device_id device; /* device associated with this kernel function */
    } mango_kernel_function;

    struct _cl_program
    {
        cl_context ctx; /* parent context */
        mango_kernel_function *kernel_functions;
        cl_int num_kernel_functions;
        cl_kernel *kernels; // FIX: cambiare con un puntatore a cl_kernel

        // cl_kernel *ker; /* all kernels included in the  */
        // alternative to *ker ?
        // list_head kernels;
        // cl_int num_kernels;
    };

    struct _cl_kernel
    {
        uint32_t id;
        mango_kernel_function kernel_function;
        mango_kernel_t kernel;
        cl_device_id device; /* device associated with this kernel */

        mango_arg_t **args;
        int args_num;
    };

    struct _cl_mem
    {
        uint32_t id;
        mango_buffer_t buffer;
        void *host_ptr;
        int type;

        cl_context ctx;
    };

    struct _cl_event
    {
        mango_event_t ev;

        cl_context ctx;
    };

    struct _cl_command_queue
    {
        //TODO: spostare taskgraph globale qui dentro
        mango_task_graph_t *tgx; /* task_graph associated with this command_queue */
        cl_context ctx;          /* parent context */
        cl_device_id device;     /* its device */
    };

    struct _cl_device_id
    {
        mango_unit_type_t device_type;
        cl_command_queue queue;
    };

    /* HELPER FUNCTIONS */
    // FIX : is there a better way to specify which kernel_function we want to set than using an index?
    cl_int clSetInputBufferIDs(cl_program program, unsigned int kernel_function_index, unsigned int nbuffers_in, ...)
    {
        assert(kernel_function_index >= 0 && kernel_function_index < program->num_kernel_functions && "the kernel function index is not valid");
        assert(nbuffers_in < MAX_KERNEL_BUFFERS && "exceeded the maximum number of kernel buffers available");
        program->kernel_functions[kernel_function_index].buffers_in = (uint32_t *)malloc(nbuffers_in * sizeof(uint32_t));

        program->kernel_functions[kernel_function_index].num_buffers_in = nbuffers_in;
        va_list list;
        va_start(list, nbuffers_in);
        for (unsigned int i = 0; i < nbuffers_in; i++)
        {
            uint32_t in_id = (uint32_t)va_arg(list, cl_uint);
            program->kernel_functions[kernel_function_index].buffers_in[i] = in_id;
        }
        va_end(list);
    }

    // FIX : is there a better way to specify which kernel_function we want to set than using an index?
    cl_int clSetOutputBufferIDs(cl_program program, unsigned int kernel_function_index, unsigned int nbuffers_out, ...)
    {
        assert(kernel_function_index >= 0 && kernel_function_index < program->num_kernel_functions && "the kernel function index is not valid");
        assert(nbuffers_out < MAX_KERNEL_BUFFERS && "exceeded the maximum number of kernel buffers available");
        program->kernel_functions[kernel_function_index].buffers_out = (uint32_t *)malloc(nbuffers_out * sizeof(uint32_t));

        program->kernel_functions[kernel_function_index].num_buffers_out = nbuffers_out;
        va_list list;
        va_start(list, nbuffers_out);
        for (unsigned int i = 0; i < nbuffers_out; i++)
        {
            uint32_t out_id = (uint32_t)va_arg(list, cl_uint);
            program->kernel_functions[kernel_function_index].buffers_out[i] = out_id;
        }
        va_end(list);
    }

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

        // get from user_data the receipe
        assert(user_data && "user_data must specify a mango_receipe");
        const char *receipe = (char *)user_data;

        cl_context context = NULL;
        if (mango_init(process_name, receipe) == SUCCESS)
        {
            context = (cl_context)malloc(sizeof(struct _cl_context));
            context->devices = devices;
            context->device_num = num_devices;
            return context;
        }
        else
        {
            if (!errcode_ret)
                *errcode_ret = CL_INVALID_VALUE;
            return context;
        }
    }

    cl_command_queue clCreateCommandQueue(cl_context context,
                                          cl_device_id device,
                                          cl_command_queue_properties properties,
                                          cl_int *errcode_ret)
    {
        cl_command_queue queue = NULL;
        queue = (cl_command_queue)malloc(sizeof(struct _cl_command_queue));

        if (!context)
        {
            if (!errcode_ret)
                *errcode_ret CL_INVALID_CONTEXT;
            goto err;
        }
        context->queue = queue;

        if (!device)
        {
            if (!errcode_ret)
                *errcode_ret CL_INVALID_DEVICE;
            goto err;
        }
        // associate device with its corresponding queue
        device->queue = queue;

        queue->ctx = context;
        queue->device = device;
        /* initialize task graph */
        queue->tgx = NULL;
        return queue;

    err:
        free(queue);
        queue = NULL;
        return queue;
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
        assert(context && "context must be a valid pointer");
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
        program->kernel_functions = (mango_kernel_function *)calloc(num_devices, sizeof(mango_kernel_function)); // FIX : not quite sure of the correctness of this
        program->num_kernel_functions = 0;
        // program->kernfunc = mango_kernelfunction_init();

        mango_exit_t err;
        // char kernel_binary[] = "/opt/mango/usr/local/share/matrix_multiplication/matrix_multiplication_dev";
        for (int i = 0; i < num_devices; i++)
        {
            mango_unit_type_t device_type = device_list[i]->device_type;
            std::cout << "[CREATE PROG. BINARY] initializing new kernel for device_type: " << device_type << std::endl;
            program->kernel_functions[i].function = mango_kernelfunction_init();
            program->num_kernel_functions++;

            program->kernel_functions[i].device = device_list[i];

            switch (device_type)
            {
            case GN:
                std::cout << "[CREATE PROG. BINARY] loading kernel at position: " << program->kernel_functions[i].function << std::endl;
                err = mango_load_kernel(binaries[i], program->kernel_functions[i].function, GN, BINARY);
                break;
            case PEAK:
                err = mango_load_kernel(binaries[i], program->kernel_functions[i].function, PEAK, BINARY);
                break;
            case GPGPU:
                err = mango_load_kernel(binaries[i], program->kernel_functions[i].function, GPGPU, BINARY);
                break;

            default:
                printf("The architecture is not currently supported\n");
                free(program->kernel_functions);
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
                free(program->kernel_functions);
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

    // TODO : clCreateKernel probably must take as input not the cl_program but a mango::kernelfunction (vedi MangoDocumentation for OpenCL example)

    cl_int clCreateKernelsInProgram(cl_program program,
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
                kernels[i]->id = kernel_id;
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

                std::cout << "[CreateKernelsInProgram] Tentativo NR 1 " << kernels[i]->device << std::endl;
                std::cout << "[CreateKernelsInProgram] Tentativo NR 2 " << kernels[i]->device->queue << std::endl;
                std::cout << "[CreateKernelsInProgram] Tentativo NR 3 " << kernels[i]->device->queue->tgx << std::endl;

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
    }

    // this must take a kernel as input instead of the context
    cl_mem clCreateBuffer(cl_kernel kernel,
                          cl_mem_flags flags,
                          size_t size,
                          void *host_ptr,
                          cl_int *errcode_ret)
    {
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

        memory->host_ptr = host_ptr;

        // FIX this need to be generic
        // TODO: aggiungere magari un messaggio che informa che mango ingora i flag openCL
        if ((flags & CL_MEM_READ_WRITE) == CL_MEM_READ_WRITE)
        {
            // TODO : to be implemented
            // memory->buffer = mango_register_memory(memory->id, size, BUFFER, 1, 1, context->p->kernel, context->p->kernel);
            // memory->type = CL_MEM_READ_WRITE;
        }
        else if ((flags & CL_MEM_WRITE_ONLY) == CL_MEM_WRITE_ONLY)
        {
            assert(kernel->kernel_function.num_buffers_out > 0 && kernel->kernel_function.out_buffer_register_id < kernel->kernel_function.num_buffers_out && "incompatible buffer ID with kernel function definition");

            memory->id = kernel->kernel_function.buffers_out[kernel->kernel_function.out_buffer_register_id];
            kernel->kernel_function.out_buffer_register_id++;

            memory->buffer = mango_register_memory(memory->id, size, BUFFER, 1, 0, kernel->kernel);
            memory->type = CL_MEM_WRITE_ONLY;

            memory->ctx = kernel->device->queue->ctx;
        }
        else if ((flags & CL_MEM_READ_ONLY) == CL_MEM_READ_ONLY)
        {
            assert(kernel->kernel_function.num_buffers_in > 0 && kernel->kernel_function.in_buffer_register_id < kernel->kernel_function.num_buffers_in && "incompatible buffer ID with kernel function definition");

            memory->id = kernel->kernel_function.buffers_in[kernel->kernel_function.in_buffer_register_id];
            kernel->kernel_function.in_buffer_register_id++;

            memory->buffer = mango_register_memory(memory->id, size, BUFFER, 0, 1, kernel->kernel);
            memory->type = CL_MEM_READ_ONLY;

            memory->ctx = kernel->device->queue->ctx;
        }
        else
        {
            if (!errcode_ret)
                *errcode_ret = CL_INVALID_VALUE;
            std::cout << "[clCreateBuffer] invalid flag value" << std::endl;
            return NULL;
        }

        kernel->device->queue->tgx = kernel->device->queue->tgx
                                         ? mango_task_graph_add_buffer(kernel->device->queue->tgx, &(memory->buffer))
                                         : mango_task_graph_add_buffer(NULL, &(memory->buffer));

        if (((flags & CL_MEM_USE_HOST_PTR) == CL_MEM_USE_HOST_PTR) || ((flags & CL_MEM_COPY_HOST_PTR) == CL_MEM_COPY_HOST_PTR))
        {
            hostBuffers.push_back(memory);
        }
        // FIX: find a more elegant way to do it. How can we know that the buffer will be protected by an event?
        if (!memory->host_ptr)
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

        if (kernel->args != NULL)
            kernel->args = (mango_arg_t **)realloc(kernel->args, sizeof(mango_arg_t *) * (kernel->args_num + 1)); // FIX : maybe reallocation can be smarter, *2 just when necessary instead of +1 everytime
        else                                                                                                      // initialize kernel args
            kernel->args = (mango_arg_t **)calloc(1, sizeof(mango_arg_t *));
        kernel->args[kernel->args_num++] = arg;

        // std::cout << "created arg at address: " << arg << std::endl;
        bufferArguments.push_back(arg);
        // std::cout << "Added new argument: " << (mango_arg_t *)arguments.back() << " [VEC_SIZE] = " << arguments.size() << std::endl;

        return CL_SUCCESS;
    }

    // TODO : change with clEnqueueTask
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
        command_queue->tgx = mango_task_graph_add_event(command_queue->tgx, NULL);

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
        mango_resource_allocation(command_queue->tgx);
        printf("[BUFFER] Allocation completed\n");

        for (auto &b : hostBuffers)
        {
            if (b->type == CL_MEM_READ_ONLY)
            {
                mango_write(b->host_ptr, b->buffer, DIRECT, 0); // TODO:  magari spostarlo in clCreateBuffer
            }
        }

        /* Putting togheter the arguments */

        std::cout << "Setting args for kernel: " << kernel->kernel << std::endl;

        // TODO : add mango_set_args alternative function without variadic parameters
        mango_args_t *args = mango_set_args(kernel->kernel, 6, kernel->args[0], kernel->args[1], kernel->args[2], kernel->args[3], kernel->args[4], arg_ev);
        printf("Succesfully created args\n");

        /* Write host->device buffers */

        /* spawn kernel */
        mango_event_t kernEvent = mango_start_kernel(kernel->kernel, args, NULL);

        /* wait for kernel completion */
        mango_wait(kernEvent);

        // } // event handler
        return CL_SUCCESS;
    }

    cl_int clEnqueueWriteBuffer(cl_command_queue command_queue,
                                cl_mem buffer,
                                cl_bool blocking_write, /* not needed */
                                size_t offset,          /* not used for now */
                                size_t size,            /* not used for now */
                                const void *ptr,
                                cl_uint num_events_in_wait_list,
                                const cl_event *event_wait_list,
                                cl_event *event)
    {
        if (!command_queue)
            return CL_INVALID_COMMAND_QUEUE;
        if (!buffer)
            return CL_INVALID_MEM_OBJECT;
        if (!ptr)
            return CL_INVALID_VALUE;
        if (command_queue->ctx == NULL || buffer->ctx == NULL || command_queue->ctx != buffer->ctx)
            return CL_INVALID_CONTEXT;

        if (event_wait_list)
        {
            if (num_events_in_wait_list > 0)
            {
                for (int i = 0; i < num_events_in_wait_list; i++)
                {
                    if (event_wait_list[i]->ctx == NULL || command_queue->ctx != event_wait_list[i]->ctx)
                        return CL_INVALID_CONTEXT;
                    mango_wait(event_wait_list[i]->ev);
                }
            }
            else
            {
                return CL_INVALID_EVENT_WAIT_LIST;
            }
        }
        else
        {
            if (num_events_in_wait_list > 0)
                return CL_INVALID_EVENT_WAIT_LIST;
        }

        printf("Enqueuing write buffer %d. Current specificication assumes asynchronous transfer.\n", buffer->id);
        if (!event)
            (*event)->ev = mango_write(ptr, buffer->buffer, DIRECT, 0);
        else
            mango_write(ptr, buffer->buffer, DIRECT, 0);

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
        if (!command_queue)
            return CL_INVALID_COMMAND_QUEUE;
        if (!buffer)
            return CL_INVALID_MEM_OBJECT;
        if (!ptr)
            return CL_INVALID_VALUE;
        if (command_queue->ctx == NULL || buffer->ctx == NULL || command_queue->ctx != buffer->ctx)
            return CL_INVALID_CONTEXT;

        if (event_wait_list)
        {
            if (num_events_in_wait_list > 0)
            {
                for (int i = 0; i < num_events_in_wait_list; i++)
                {
                    if (event_wait_list[i]->ctx == NULL || command_queue->ctx != event_wait_list[i]->ctx)
                        return CL_INVALID_CONTEXT;
                    mango_wait(event_wait_list[i]->ev);
                }
            }
            else
            {
                return CL_INVALID_EVENT_WAIT_LIST;
            }
        }
        else
        {
            if (num_events_in_wait_list > 0)
                return CL_INVALID_EVENT_WAIT_LIST;
        }

        printf("Enqueuing read buffer %d. Current specificication assumes asynchronous transfer.\n", buffer->id);
        if (!event)
            (*event)->ev = mango_read(ptr, buffer->buffer, DIRECT, 0);
        else
            mango_read(ptr, buffer->buffer, DIRECT, 0);

        return CL_SUCCESS;
    }

    cl_int clReleaseProgram(cl_program program)
    {
        printf("ReleaseProgram is not implemented\n");
    }

    cl_int clReleaseCommandQueue(cl_command_queue command_queue)
    {
        mango_resource_deallocation(command_queue->tgx);
        mango_task_graph_destroy_all(command_queue->tgx);
    }

    cl_int clReleaseContext(cl_context context)
    {
        mango_release();
    }

#ifdef __cplusplus
}
#endif
