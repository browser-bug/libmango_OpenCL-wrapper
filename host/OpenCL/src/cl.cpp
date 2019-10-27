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

extern "C"
{

    struct _cl_context
    {
        cl_program p;
        // cl_kern k;
        // cl_mem m;
        //mango_task_graph_t *tg;
    };

    struct _cl_program
    {
        kernelfunction *kernfunc;
        mango_task_graph_t *tg;
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
        std::vector<mango::Arg> arguments;
    };

    /* API IMPLEMENTATION */

    cl_int clGetPlatformIDs(cl_uint /* num_entries */,
                            cl_platform_id * /* platforms */,
                            cl_uint * /* num_platforms */)
    {
        printf("GetPlatformIDs is not implemented\n");
        return CL_SUCCESS;
    }

    cl_int clGetDeviceIDs(cl_platform_id /* platform */,
                          cl_device_type /* device_type */,
                          cl_uint /* num_entries */,
                          cl_device_id * /* devices */,
                          cl_uint * /* num_devices */)
    {
        printf("GetDeviceIDs is not implemented\n");
        return CL_SUCCESS;
    }

    cl_context clCreateContext(const cl_context_properties * /* properties */,
                               cl_uint /* num_devices */,
                               const cl_device_id * /* devices */,
                               void(CL_CALLBACK * /* pfn_notify */)(const char *, const void *, size_t, void *),
                               void * /* user_data */,
                               cl_int * /* errcode_ret */)
    {
        if (mango_init("test", "test_manga") == SUCCESS)
        {
            cl_context context = NULL;
            context = (cl_context)malloc(sizeof(struct _cl_context));
            return context;
        }
    }

    cl_command_queue clCreateCommandQueue(cl_context /* context */,
                                          cl_device_id /* device */,
                                          cl_command_queue_properties /* properties */,
                                          cl_int * /* errcode_ret */)
    {
        printf("CreateCommandQueue is not implemented\n");
        cl_command_queue commands;
        // FIX a task_graph creation section could be added

        return commands;
    }

    cl_program clCreateProgramWithSource(cl_context context,
                                         cl_uint count,
                                         const char **strings,
                                         const size_t *lengths,
                                         cl_int *errcode_ret)
    {
        // #ifdef GNEMU
        //         char kernel_file[] = "/opt/mango/usr/local/share/matrix_multiplication/matrix_multiplication_dev";
        // #else
        //         char kernel_file[] = "/opt/mango/usr/local/share/matrix_multiplication/memory.data.fpga.datafile";
        // #endif

        // Force to read the matrix_multiplication_dev file
        char kernel_file[] = "/opt/mango/usr/local/share/matrix_multiplication/matrix_multiplication_dev";

        cl_program program = NULL;
        program = (cl_program)malloc(sizeof(struct _cl_program));
        program->kernfunc = mango_kernelfunction_init();

        // Load the kernel in PEAK mode
        mango_load_kernel(kernel_file, program->kernfunc, PEAK, BINARY);

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
        cl_kernel kern = NULL;
        kern = (cl_kernel)malloc(sizeof(struct _cl_kernel));

        kern->id = KID;
        // FIX find a way to make a variadic call to mango_register_kernel
        kern->kernel = mango_register_kernel(kern->id, program->kernfunc, 2, 1, B1, B2, B3);

        program->tg = NULL;
        program->tg = mango_task_graph_add_kernel(NULL, &(kern->kernel));
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

        // FIX this need to be generic, B1 can't be static
        if (flags != (CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR))
        {
            memory->buffer = mango_register_memory(memory->id, size, BUFFER, 1, 0, context->p->kernel);
        }
        else
        {
            memory->buffer = mango_register_memory(memory->id, size, BUFFER, 0, 1, context->p->kernel);
        }

        context->p->tg = mango_task_graph_add_buffer(context->p->tg, &(memory->buffer));

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
            value = &(((cl_mem)arg_value)->buffer);
            break;

        case sizeof(int):
            arg_size = sizeof(uint32_t);
            arg_type = SCALAR;
            value = arg_value;
            break;

        default:
            return CL_BUILD_ERROR;
            break;
        }


        // FIX arg_value must be the address of a mango_buffer_t
        std::cout << "Passing mango_buffer with address: " << arg_value << " and value: " << (*(uint32_t *)value) << std::endl;
        mango_arg_t *arg = mango_arg(kernel->kernel, value, arg_size, arg_type);

        std::cout << "created arg at address: " << arg << std::endl;
        kernel->arguments.push_back((mango::Arg *)arg);

        return CL_SUCCESS;
    }
}
