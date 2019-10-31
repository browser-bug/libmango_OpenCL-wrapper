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

    std::vector<mango::Arg *> arguments;
    mango_task_graph_t *tg = NULL;
    mango_buffer_t writeBuf;

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
    };


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

    cl_context context = NULL;
    cl_context clCreateContext(const cl_context_properties *properties,
                               cl_uint num_devices,
                               const cl_device_id *devices,
                               void(CL_CALLBACK *pfn_notify)(const char *, const void *, size_t, void *),
                               void *user_data,
                               cl_int *errcode_ret)
    {
        if (mango_init("test", "test_manga") == SUCCESS)
        {
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
        // FIX a task_graph creation section could be added

        return commands;
    }

    cl_program program = NULL;
    cl_program clCreateProgramWithBinary(cl_context context,
                                         cl_uint num_devices,
                                         const cl_device_id *device_list,
                                         const size_t *lengths,
                                         const unsigned char **binaries,
                                         cl_int *binary_status,
                                         cl_int *errcode_ret)
    {
        // #ifdef GNEMU
        //         char kernel_file[] = "/opt/mango/usr/local/share/matrix_multiplication/matrix_multiplication_dev";
        // #else
        //         char kernel_file[] = "/opt/mango/usr/local/share/matrix_multiplication/memory.data.fpga.datafile";
        // #endif

        // Force to read the matrix_multiplication_dev file
        char kernel_binary[] = "/opt/mango/usr/local/share/matrix_multiplication/matrix_multiplication_dev";

        program = (cl_program)malloc(sizeof(struct _cl_program));
        program->kernfunc = mango_kernelfunction_init();

        // Load the kernel in PEAK mode
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

    cl_kernel kern = NULL;
    cl_kernel clCreateKernel(cl_program program,
                             const char *kernel_name,
                             cl_int *errcode_ret)
    {
        kern = (cl_kernel)malloc(sizeof(struct _cl_kernel));

        kern->id = KID;
        // FIX find a way to make a variadic call to mango_register_kernel
        kern->kernel = mango_register_kernel(kern->id, program->kernfunc, 2, 1, B1, B2, B3);

        tg = mango_task_graph_add_kernel(NULL, &(kern->kernel));
        std::cout << "[TASK_GRAPH] added new kernel to tg (address) : " << tg << std::endl;
        program->kernel = kern->kernel;

        *errcode_ret = CL_SUCCESS;
        return kern;
    }

    cl_mem clCreateBuffer(cl_context context,
                          cl_mem_flags flags,
                          size_t size,
                          void *host_ptr,
                          cl_int *errcode_ret)
    {
        // TODO: get kernel based on the context
        cl_mem memory = NULL;
        memory = (cl_mem)malloc(sizeof(struct _cl_mem));

        memory->id = buffer_id;
        buffer_id++;

        // FIX this need to be generic
        if (flags != (CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR))
        {
            memory->buffer = mango_register_memory(memory->id, size, BUFFER, 1, 0, context->p->kernel);
            writeBuf = memory->buffer;
        }
        else
        {
            memory->buffer = mango_register_memory(memory->id, size, BUFFER, 0, 1, context->p->kernel);
        }

        tg = mango_task_graph_add_buffer(tg, &(memory->buffer));

        std::cout << "[TASK_GRAPH] added new buffer to tg (address) : " << tg << std::endl;

        // Data transfer host->device
        if (host_ptr != NULL)
        {
            mango_write(host_ptr, memory->buffer, DIRECT, 0);
        }

        // std::cout << "Returning memory with address: " << memory << std::endl;
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
        std::cout << "Passing mango_buffer with address: " << arg_value << " and value: " << (*(uint32_t *)value) << std::endl;
        mango_arg_t *arg = mango_arg(kernel->kernel, value, arg_size, arg_type);

        // std::cout << "created arg at address: " << arg << std::endl;
        arguments.push_back((mango::Arg *)arg);
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
        mango_arg_t *arg_ev = NULL;
        if (event != NULL)
        {
            cl_event e = *event;
            e = (cl_event)malloc(sizeof(struct _cl_event));
            e->ev = mango_get_buffer_event(writeBuf);
            arg_ev = mango_arg(kernel->kernel, &(e->ev), sizeof(uint64_t), EVENT);
        }

        tg = mango_task_graph_add_event(tg, NULL);

        printf("[BUFFER] Allocating new resources\n");
        mango_resource_allocation(tg);
        printf("[BUFFER] Allocation completed\n");

        /* Putting togheter the arguments */
        // TODO convert the vector data into the variadic parameter of mango_set_args

        std::cout << "Setting args for kernel: " << kernel->kernel << std::endl;
        mango_args_t *args = mango_set_args(kernel->kernel, 6, arguments.at(0),arguments.at(1),arguments.at(2),arguments.at(3),arguments.at(4),arg_ev);
        printf("Succesfully created args\n");

        /* spawn kernel */
        mango_event_t ev = mango_start_kernel(kernel->kernel, args, NULL);

        /* wait for kernel completion */
        mango_wait(ev);

        return CL_SUCCESS;
    }

    cl_int clWaitForEvents(cl_uint num_events,
                           const cl_event *event_list)
    {
        // TODO : make it iterate over the event list
        cl_event e = *event_list;
        mango_wait(e->ev);

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
