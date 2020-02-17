#include "clDevice.h"
#include "clContext.h"
#include "clCommandQueue.h"
#include "clEnqueue.h"
#include "clEvent.h"
#include "clProgram.h"
#include "clKernel.h"
#include "clMem.h"

#include "CL/cl.h"

#ifdef __cplusplus
#include <stdio.h>
#include <unistd.h> /* To get process IDs etc. */
#include <string.h>
#endif

#define MAX_NUM_KERNEL_BUFFERS 15 

#ifdef __cplusplus
extern "C"
{
#endif

    // typedef struct _sync_item
    // {
    //     cl_event event;
    //     sync_item *next;
    // } sync_item;

    /* ADDITIONAL FUNCTIONS */
    // FIX : is there a better way to specify which kernel_function we want to set than using an index?
    cl_int clSetInputBufferIDs(cl_program program, unsigned int kernel_function_index, unsigned int nbuffers_in, ...)
    {
        assert(kernel_function_index >= 0 && kernel_function_index < program->num_kernel_functions && "the kernel function index is not valid");
        assert(nbuffers_in < MAX_NUM_KERNEL_BUFFERS && "exceeded the maximum number of kernel buffers available");
        if (program->kernel_functions[kernel_function_index].buffers_in)
            free(program->kernel_functions[kernel_function_index].buffers_in);
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
        assert(nbuffers_out < MAX_NUM_KERNEL_BUFFERS && "exceeded the maximum number of kernel buffers available");
        if (program->kernel_functions[kernel_function_index].buffers_out)
            free(program->kernel_functions[kernel_function_index].buffers_out);
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
        return cl_get_device_ids(platform, device_type, num_entries, devices, num_devices);
    }

    cl_context clCreateContext(const cl_context_properties *properties,
                               cl_uint num_devices,
                               const cl_device_id *devices,
                               void(CL_CALLBACK *pfn_notify)(const char *, const void *, size_t, void *),
                               void *user_data,
                               cl_int *errcode_ret)
    {
        return cl_create_context(properties, num_devices, devices, pfn_notify, user_data, errcode_ret);
    }

    cl_command_queue clCreateCommandQueue(cl_context context,
                                          cl_device_id device,
                                          cl_command_queue_properties properties,
                                          cl_int *errcode_ret)
    {
        return cl_create_command_queue(context, device, properties, errcode_ret);
    }

    cl_program clCreateProgramWithBinary(cl_context context,
                                         cl_uint num_devices,
                                         const cl_device_id *device_list,
                                         const size_t *lengths, /* not needed */
                                         const char **binaries,
                                         cl_int *binary_status,
                                         cl_int *errcode_ret)
    {
        return cl_create_program_with_binary(context, num_devices, device_list, lengths, binaries, binary_status, errcode_ret);
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
        return cl_create_kernel(program, kernel_name, errcode_ret, kernel_id);
    }

    // FIX: not to be used ?
    /* cl_int clCreateKernelsInProgram(cl_program program,
                                    cl_uint num_kernels,
                                    cl_kernel *kernels,
                                    cl_uint *num_kernels_ret)
    {
        return cl_create_kernels_in_program(program, num_kernels, kernels, num_kernels_ret);
    } */

    cl_mem clCreateBuffer(cl_context context,
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
        return cl_create_buffer(context, flags, size, host_ptr, errcode_ret, num_kernels_in, kernels_in, num_kernels_out, kernels_out, buffer_id);
    }

    cl_int clSetKernelArg(cl_kernel kernel,
                          cl_uint arg_index,
                          size_t arg_size,
                          const void *arg_value,
                          cl_argument_type arg_type)
    {
        return cl_set_kernel_arg(kernel, arg_index, arg_size, arg_value, arg_type);
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
        return cl_enqueue_ND_range_kernel(command_queue, kernel, num_events_in_wait_list, event_wait_list, event);
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
        return cl_enqueue_write_buffer(command_queue, buffer, ptr, num_events_in_wait_list, event_wait_list, event);
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
        return cl_enqueue_read_buffer(command_queue, buffer, ptr, num_events_in_wait_list, event_wait_list, event);
    }

    cl_int clWaitForEvents(cl_uint num_events,
                           const cl_event *event_list)
    {
        return cl_wait_for_events(num_events, event_list);
    }

    cl_int clFinish(cl_command_queue command_queue)
    {
        return cl_finish(command_queue);
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
