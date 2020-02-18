#include "cl.h"
#ifdef __cplusplus
#include <stdio.h>
#include <memory>
#endif


#define MAX_KERNEL_BUFFERS 15

#ifdef __cplusplus
extern "C"
{
#endif

    // TODO: this array must be populated with a mango function yet to be implemented
    std::array<mango_unit_type_t, 4> availableUnits = {GN, GN, GPGPU, PEAK};

    struct _cl_context
    {
        cl_command_queue queue; /* the command queue corresponds to the TaskGraph in MANGO */
        cl_program program;     /* the program associated with this context */
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

        uint32_t *buffers_out;
        int num_buffers_out;

        const char *binary;
        cl_device_id device; /* device associated with this kernel function */
    } mango_kernel_function;

    struct _cl_program
    {
        cl_context ctx; /* parent context */
        //std::map<uint32_t, mango_kernel_function> *map_kernel_functions; 
        std::map<const char*, mango_kernel_function> *map_kernel_functions;
        cl_int num_kernel_functions;

        // cl_kernel *kernels; // FIX: not needed?
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
        mango_task_graph_t *tgx; /* task_graph associated with this command_queue */

        cl_device_id device; /* its device */
        cl_context ctx;      /* parent context */
    };

    struct _cl_device_id
    {
        mango_unit_type_t device_type;
        cl_command_queue queue; /* command_queue associated with this device */
    };

    /* HELPER FUNCTIONS */
    // FIX : is there a better way to specify which kernel_function we want to set than using an index?
    //cl_int clSetInputBufferIDs(cl_program program, uint32_t kernel_id, unsigned int nbuffers_in, ...)
    cl_int clSetInputBufferIDs(cl_program program, const char* binary, unsigned int nbuffers_in, ...)
    {
        assert(program->map_kernel_functions != NULL && "map has not been initialized");
      //  assert(kernel_id > 0 && "invalid kernel id");
        assert(nbuffers_in < MAX_KERNEL_BUFFERS && "exceeded the maximum number of kernel buffers available");
        
        mango_kernel_function *k1 = &(*program->map_kernel_functions)[binary];
        //mango_kernel_function *k1 = &(*program->map_kernel_functions)[kernel_id];

        if (k1->buffers_in)
            free(k1->buffers_in);

        k1->buffers_in = (uint32_t *)malloc(nbuffers_in * sizeof(uint32_t));
        k1->num_buffers_in = nbuffers_in;

        va_list list;
        va_start(list, nbuffers_in);
        for (unsigned int i = 0; i < nbuffers_in; i++)
        {
            uint32_t in_id = (uint32_t)va_arg(list, cl_uint);
            k1->buffers_in[i] = in_id;
        }
        va_end(list);
    }

    // FIX : is there a better way to specify which kernel_function we want to set than using an index?
    //cl_int clSetOutputBufferIDs(cl_program program, uint32_t kernel_id, unsigned int nbuffers_out, ...)
    cl_int clSetOutputBufferIDs(cl_program program, const char* binary, unsigned int nbuffers_out, ...)
    {
        assert(program->map_kernel_functions != NULL && "map has not been initialized");
        //assert(kernel_id > 0 && "invalid kernel id");
        assert(nbuffers_out < MAX_KERNEL_BUFFERS && "exceeded the maximum number of kernel buffers available");
        
        mango_kernel_function *k1 = &(*program->map_kernel_functions)[binary];
        //mango_kernel_function *k1 = &(*program->map_kernel_functions)[kernel_id];
        if (k1->buffers_out)
            free(k1->buffers_out);
        
        k1->buffers_out = (uint32_t *)malloc(nbuffers_out * sizeof(uint32_t));
        k1->num_buffers_out = nbuffers_out;
        
        va_list list;
        va_start(list, nbuffers_out);
        for (unsigned int i = 0; i < nbuffers_out; i++)
        {
            uint32_t out_id = (uint32_t)va_arg(list, cl_uint);
            k1->buffers_out[i] = out_id;
        }
        va_end(list);
    }

    /* API IMPLEMENTATION */

    cl_int clGetPlatformIDs(cl_uint num_entries,
                            cl_platform_id *platforms,
                            cl_uint *num_platforms)
    {
        printf("GetPlatformIDs is not implemented\n");
        return CL_SUCCESS;
    }

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

    cl_int clGetDeviceIDs(cl_platform_id platform,
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

    // support function for clCreateContext
    const char *get_process_name_by_pid(const int pid)
    {
        char *name = NULL;
        name = (char *)calloc(1024, sizeof(char));
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

    cl_context clCreateContext(const cl_context_properties *properties,
                               cl_uint num_devices,
                               const cl_device_id *devices,
                               void(CL_CALLBACK *pfn_notify)(const char *, const void *, size_t, void *),
                               void *user_data,
                               cl_int *errcode_ret)
    {
        assert(user_data && "user_data must specify a mango_receipe");

        const int pid = getpid();
        const char *process_name = get_process_name_by_pid(pid);
        const char *receipe = (char *)user_data;

        cl_context context = NULL;
        if (mango_init(process_name, receipe) == SUCCESS)
        {
            context = (cl_context)malloc(sizeof(struct _cl_context));
            context->devices = devices;
            context->device_num = num_devices;
            context->mem_objects = NULL;
            context->mem_object_num = 0;
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

        if (!device)
        {
            if (!errcode_ret)
                *errcode_ret CL_INVALID_DEVICE;
            goto err;
        }

        context->queue = queue;
        queue->ctx = context;
        // If you need to associate device with its corresponding queue
        for (int i = 0; i < context->device_num; i++)
            context->devices[i]->queue = queue;

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
        // program->kernel_functions = (mango_kernel_function *)calloc(num_devices, sizeof(mango_kernel_function)); // FIX : not quite sure of the correctness of this
        // program->num_kernel_functions = num_devices;
        program->map_kernel_functions = new std::map<const char*, mango_kernel_function>();
        mango_exit_t err;

        for (int i = 0; i < num_devices; i++)
        {
            mango_unit_type_t device_type = device_list[i]->device_type;

            std::cout << "[clCreateProgramWithBinary] initializing new kernel for device_type: " << device_type << std::endl;
        
          //  int kernel_id = i+1;
            const char *path = binaries[i];
            
            // PROBLEMA: qui si dovrebbe sapere a priori gli id che l'utente andrà a specificare in clCreateKernel
            
            (*program->map_kernel_functions)[path].function = mango_kernelfunction_init();

            (*program->map_kernel_functions)[path].device = device_list[i];
            (*program->map_kernel_functions)[path].binary = binaries[i];
            (*program->map_kernel_functions)[path].buffers_in = NULL;
            (*program->map_kernel_functions)[path].buffers_out = NULL;

            err = mango_load_kernel(path, (*program->map_kernel_functions)[path].function, device_type, BINARY);
            if (err != SUCCESS)
            {
                if (binary_status)
                    binary_status[i] = CL_INVALID_BINARY;
                if (errcode_ret)
                    *errcode_ret = CL_INVALID_BINARY;
                free(program->map_kernel_functions);
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

        for ( const auto &p : (*program->map_kernel_functions) )
        {
            i++;
            if (strcmp(p.first, kernel_name) == 0)
                break;
        } 
        if (i == 0)
        {
            printf("[clCreateKernel] kernel not found\n");
            free(kernel);
            kernel = NULL;
            return kernel;
        }

        kernel->id = kernel_id;
        kernel->kernel_function = (*program->map_kernel_functions)[kernel_name];
        kernel->device = kernel->kernel_function.device;

        kernel->args = NULL;
        kernel->args_num = 0;

        // FIX : maybe move this copy part in the mango_register_kernel_with_buffers()
        std::vector<uint32_t> buf_in(kernel->kernel_function.buffers_in, kernel->kernel_function.buffers_in + kernel->kernel_function.num_buffers_in);
        std::vector<uint32_t> buf_out(kernel->kernel_function.buffers_out, kernel->kernel_function.buffers_out + kernel->kernel_function.num_buffers_out);

        kernel->kernel = mango_register_kernel_with_buffers(kernel->id,
                                                            kernel->kernel_function.function,
                                                            &buf_in,
                                                            &buf_out);

        // Get the task graph associated with the kernel (via its device)
        kernel->device->queue->tgx = mango_task_graph_add_kernel(kernel->device->queue->tgx, &(kernel->kernel));
        std::cout << "[TASK_GRAPH] added new kernel to tg (address) : " << kernel->device->queue->tgx << std::endl;

        return kernel;
    }

    // support function for clCreateBuffer
    void extractKernelIDs(std::vector<mango_kernel_t>* _kernels, cl_kernel *kernels, cl_int num_kernels)
    {
        _kernels->clear();
        for (int i = 0; i < num_kernels; i++)
            _kernels->push_back(kernels[i]->kernel);
    }

    cl_mem clCreateBuffer(cl_context context, // FIX : non cambiare i parametri ma aggiungi in coda un array di kernels da passare a mango_register_buffer
                          cl_mem_flags flags,
                          size_t size,
                          void *host_ptr,
                          cl_int *errcode_ret,
                          cl_int num_kernels_in,
                          cl_kernel *kernels_in,
                          cl_int num_kernels_out,
                          cl_kernel *kernels_out,
                          cl_int buffer_id)
    {
        cl_mem memory = NULL;
        memory = (cl_mem)malloc(sizeof(struct _cl_mem));

        memory->host_ptr = host_ptr;
        memory->ctx = context;
        memory->id = buffer_id;


        std::vector<mango_kernel_t> _kernels_in;
        std::vector<mango_kernel_t> _kernels_out;
        extractKernelIDs(&_kernels_in, kernels_in, num_kernels_in);
        extractKernelIDs(&_kernels_out, kernels_out, num_kernels_out);

        std::cout << "[clCreateBuffer] ignoring user flags" << std::endl;
        if ((flags & CL_MEM_READ_WRITE) == CL_MEM_READ_WRITE)
        {
            // TODO : to be implemented
            // assert(kernels_in && kernels_out && "specify input and output kernels");
            // memory->buffer = mango_register_memory(memory->id, size, BUFFER, 1, 1, context->program->kernel, context->program->kernel);
            // memory->type = CL_MEM_READ_WRITE;
        }
        else if ((flags & CL_MEM_WRITE_ONLY) == CL_MEM_WRITE_ONLY)
        {
            assert(kernels_in && "specify input kernels");
            
            memory->type = CL_MEM_WRITE_ONLY;
            memory->buffer = mango_register_memory_with_kernels(memory->id, size, BUFFER, &_kernels_in, NULL);
        }
        else if ((flags & CL_MEM_READ_ONLY) == CL_MEM_READ_ONLY)
        {
            assert(kernels_out && "specify output kernels");

            memory->type = CL_MEM_READ_ONLY;
            memory->buffer = mango_register_memory_with_kernels(memory->id, size, BUFFER, NULL, &_kernels_out);
        }
        else
        {
            if (!errcode_ret)
                *errcode_ret = CL_INVALID_VALUE;
            std::cout << "[clCreateBuffer] please specify CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY | CL_MEM_READ_WRITE" << std::endl;
            free(memory);
            memory = NULL;
            return memory;
        }

        memory->ctx->queue->tgx = mango_task_graph_add_buffer(memory->ctx->queue->tgx, &(memory->buffer));
        std::cout << "[TASK_GRAPH] added new buffer to tg (address) : " << memory->ctx->queue->tgx << std::endl;

        if (context->mem_objects != NULL)
            context->mem_objects = (cl_mem *)realloc(context->mem_objects, sizeof(cl_mem *) * (context->mem_object_num + 1)); // FIX : maybe reallocation can be smarter, *2 just when necessary instead of +1 everytime
        else                                                                                                                  // initialize mem objects
            context->mem_objects = (cl_mem *)calloc(1, sizeof(cl_mem));

        context->mem_objects[context->mem_object_num] = memory;
        context->mem_object_num++;

        return memory;
    }

    cl_int clSetKernelArg(cl_kernel kernel,
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

        /* DA RIMUOVERE
            switch (arg_size)
            {
            case sizeof(cl_mem):
                arg_size = sizeof(uint64_t); // this is not needed in mango_set_args
                argument_type = BUFFER;
                value = &((*(cl_mem *)arg_value)->buffer);
                break;

            // TODO : aggiungere casistica per eventi e fixare il fatto che tutte le struct cl_mem, cl_event ecc. sono viste con la stessa dimensione.
            // case sizeof(cl_event):
            //     arg_size = sizeof(uint64_t);
            //     argument_type = EVENT;
            //     value = &((*(cl_event *)arg_value)->ev);
            //     break;

            // FIX: mettere come default
            case sizeof(int):
                arg_size = sizeof(uint32_t);
                argument_type = SCALAR;
                value = arg_value;
                break;

            default:
                return CL_BUILD_ERROR;
            } 
        */

        mango_arg_t *arg = mango_arg(kernel->kernel, value, arg_size, argument_type);

        if (kernel->args != NULL)
            kernel->args = (mango_arg_t **)realloc(kernel->args, sizeof(mango_arg_t *) * (kernel->args_num + 1)); // FIX : maybe reallocation can be smarter, *2 just when necessary instead of +1 everytime
        else                                                                                                      // initialize kernel args
            kernel->args = (mango_arg_t **)calloc(1, sizeof(mango_arg_t *));

        kernel->args[kernel->args_num] = arg;
        kernel->args_num++;

        return CL_SUCCESS;
    }

    cl_int clKernelAndBufferAllocation(cl_command_queue command_queue)
    {
        printf("[clKernelAndBufferAllocation] allocating new resources\n");
        command_queue->tgx = mango_task_graph_add_event(command_queue->tgx, NULL); // FIX : this can't stay here
        mango_exit_t err = mango_resource_allocation(command_queue->tgx);
        if (err == SUCCESS)
        {
            printf("[clKernelAndBufferAllocation] allocation completed\n");
            return CL_SUCCESS;
        }
        else
            return CL_OUT_OF_RESOURCES;
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
        std::cout << "[clEnqueueNDRangeKernel] setting args for kernel: " << kernel->kernel << std::endl;
        std::vector<mango::Arg *> args_vec((mango::Arg **)kernel->args, (mango::Arg **)(kernel->args + kernel->args_num));
        mango_args_t *args = mango_set_args_from_vector(kernel->kernel, &args_vec);
        std::cout << "[clEnqueueNDRangeKernel] succesfully created args" << std::endl;

        if (event_wait_list)
        {
            if (num_events_in_wait_list > 0)
            {
                for (int i = 0; i < num_events_in_wait_list; i++)
                {
                    if (event_wait_list[i]->ctx == NULL || command_queue->ctx != event_wait_list[i]->ctx)
                        return CL_INVALID_CONTEXT;
                    std::cout << "[clEnqueueNDRangeKernel] waiting for the event : " << event_wait_list[i]->ev << std::endl;
                    mango_wait(event_wait_list[i]->ev);
                    std::cout << "[clEnqueueNDRangeKernel] finished waiting for event : " << event_wait_list[i]->ev << std::endl;
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

        if (event)
        {
            (*event) = (cl_event)malloc(sizeof(struct _cl_event));
            (*event)->ev = mango_start_kernel(kernel->kernel, args, NULL);
            std::cout << "[clEnqueueNDRangeKernel] creating event : " << (*event)->ev << std::endl;
            (*event)->ctx = command_queue->ctx;
        }
        else
            mango_start_kernel(kernel->kernel, args, NULL);

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
                    std::cout << "[clEnqueueWriteBuffer] waiting for the event : " << event_wait_list[i]->ev << std::endl;
                    mango_wait(event_wait_list[i]->ev);
                    std::cout << "[clEnqueueWriteBuffer] finished waiting for event : " << event_wait_list[i]->ev << std::endl;
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
        if (event)
        {
            (*event) = (cl_event)malloc(sizeof(struct _cl_event));
            (*event)->ev = mango_write(ptr, buffer->buffer, DIRECT, 0);
            std::cout << "[clEnqueueWriteBuffer] creating event : " << (*event)->ev << std::endl;
            (*event)->ctx = command_queue->ctx;
        }
        else
            mango_write(ptr, buffer->buffer, DIRECT, 0);

        return CL_SUCCESS;
    }

    cl_int clEnqueueReadBuffer(cl_command_queue command_queue,
                               cl_mem buffer,
                               cl_bool blocking_write, /* not needed */
                               size_t offset,          /* not used for now */
                               size_t size,            /* not used for now */
                               void *ptr,
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
                    std::cout << "[clEnqueueReadBuffer] waiting for the event : " << event_wait_list[i]->ev << std::endl;
                    mango_wait(event_wait_list[i]->ev);
                    std::cout << "[clEnqueueReadBuffer] finished waiting for event : " << event_wait_list[i]->ev << std::endl;
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
        if (event)
        {
            (*event) = (cl_event)malloc(sizeof(struct _cl_event));
            (*event)->ev = mango_read(ptr, buffer->buffer, DIRECT, 0);
            std::cout << "[clEnqueueReadBuffer] creating event : " << (*event)->ev << std::endl;
            (*event)->ctx = command_queue->ctx;
        }
        else
            mango_read(ptr, buffer->buffer, DIRECT, 0);

        return CL_SUCCESS;
    }

    cl_int clWaitForEvents(cl_uint num_events,
                           const cl_event *event_list)
    {
        if (num_events <= 0 || event_list == NULL)
            return CL_INVALID_VALUE;

        for (int i = 0; i < num_events; i++)
        {
            if (!event_list[i])
                return CL_INVALID_EVENT;
            std::cout << "[clWaitForEvents] waiting for event : " << event_list[i]->ev << std::endl;
            mango_wait(event_list[i]->ev);
            std::cout << "[clWaitForEvents] finished waiting for event : " << event_list[i]->ev << std::endl;
        }

        return CL_SUCCESS;
    }

    cl_int clReleaseProgram(cl_program program)
    {
        if (program == NULL)
            return CL_INVALID_PROGRAM;

        printf("ReleaseProgram is not implemented\n");
        return CL_SUCCESS;
    }

    cl_int clReleaseMemObject(cl_mem memobj)
    {
        if (memobj == NULL)
            return CL_INVALID_MEM_OBJECT;

        printf("[clReleaseMemObject] deallocating buffer %d\n", memobj->id);
        mango_deregister_memory(memobj->buffer);
        return CL_SUCCESS;
    }

    cl_int clReleaseKernel(cl_kernel kernel)
    {
        if (kernel == NULL)
            return CL_INVALID_KERNEL;

        // mango_deregister_kernel(kernel->kernel); // TODO : implement it in mango.cpp
        printf("ReleaseKernel is not implemented\n");
        return CL_SUCCESS;
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
