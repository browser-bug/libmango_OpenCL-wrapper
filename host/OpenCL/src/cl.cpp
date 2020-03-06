#include "clDevice.h"
#include "clContext.h"
#include "clCommandQueue.h"
#include "clEnqueue.h"
#include "clEvent.h"
#include "clProgram.h"
#include "clKernel.h"
#include "clMem.h"
#include "clExceptions.h"

#include "CL/cl.h"

#ifdef __cplusplus
#include <stdio.h>
#include <string>
#endif

// TODO is this related to the mango_get_max_nr_buffers() function of MANGO?
#define MAX_NUM_KERNEL_BUFFERS 10 /* this defines the max. number of I/O buffers a kernel function can have */

#ifdef __cplusplus
extern "C"
{
#endif

    /* API IMPLEMENTATION */

    cl_int clGetPlatformIDs(cl_uint num_entries,
                            cl_platform_id *platforms,
                            cl_uint *num_platforms)
    {
        printf("clGetPlatformIDs is not available or yet to be implemented.\n");
        return CL_SUCCESS;
    }

    cl_int clGetPlatformInfo(cl_platform_id platform,
                             cl_platform_info param_name,
                             size_t param_value_size,
                             void *param_value,
                             size_t *param_value_size_ret)
    {
        printf("clGetPlatformIDs is not available or yet to be implemented.\n");
        return CL_SUCCESS;
    }

    cl_int clGetDeviceIDs(cl_platform_id platform,
                          cl_device_type device_type,
                          cl_uint num_entries,
                          cl_device_id *devices,
                          cl_uint *num_devices)
    {
        return cl_get_device_ids(device_type,
                                 num_entries,
                                 devices,
                                 num_devices);
    }

    cl_int clGetDeviceInfo(cl_device_id device,
                           cl_device_info param_name,
                           size_t param_value_size,
                           void *param_value,
                           size_t *param_value_size_ret)
    {
        return cl_get_device_info(device,
                                  param_name,
                                  param_value_size,
                                  param_value,
                                  param_value_size_ret);
    }

    cl_int clCreateSubDevices(cl_device_id in_device,
                              const cl_device_partition_property *properties,
                              cl_uint num_devices,
                              cl_device_id *out_devices,
                              cl_uint *num_devices_ret)
    {
        printf("clCreateSubDevices is not available or yet to be implemented.\n");
        return CL_SUCCESS;
    }

    cl_int clRetainDevice(cl_device_id device)
    {
        printf("clRetainDevice is not available or yet to be implemented.\n");
        return CL_SUCCESS;
    }

    cl_int clReleaseDevice(cl_device_id device)
    {
        printf("clReleaseDevice is not available or yet to be implemented.\n");
        delete device;
        return CL_SUCCESS;
    }

    cl_context clCreateContext(const cl_context_properties *properties,
                               cl_uint num_devices,
                               const cl_device_id *devices,
                               void(CL_CALLBACK *pfn_notify)(const char *, const void *, size_t, void *),
                               void *user_data,
                               cl_int *errcode_ret)
    {
        try
        {
            return cl_create_context(num_devices,
                                     devices,
                                     user_data,
                                     errcode_ret);
        }
        catch (const std::bad_alloc &)
        {
            if (errcode_ret)
                *errcode_ret = CL_OUT_OF_HOST_MEMORY;
            return NULL;
        }
        catch (const cl_error &e)
        {
            if (errcode_ret)
                *errcode_ret = e.errcode;
            return NULL;
        }
    }

    cl_context clCreateContextFromType(const cl_context_properties *properties,
                                       cl_device_type device_type,
                                       void(CL_CALLBACK *pfn_notify)(const char *, const void *, size_t, void *),
                                       void *user_data,
                                       cl_int *errcode_ret)
    {
        try
        {
            return cl_create_context_from_type(device_type,
                                               user_data,
                                               errcode_ret);
        }
        catch (const std::bad_alloc &)
        {
            if (errcode_ret)
                *errcode_ret = CL_OUT_OF_HOST_MEMORY;
            return NULL;
        }
        catch (const cl_error &e)
        {
            if (errcode_ret)
                *errcode_ret = e.errcode;
            return NULL;
        }
    }

    cl_int clRetainContext(cl_context context)
    {
        printf("clRetainContext is not available or yet to be implemented.\n");
        return CL_SUCCESS;
    }

    cl_int clReleaseContext(cl_context context)
    {
        if (context == NULL)
            return CL_INVALID_CONTEXT;
        if (mango_release() != SUCCESS)
            return CL_OUT_OF_RESOURCES;
        delete context;
        return CL_SUCCESS;
    }

    cl_int clGetContextInfo(cl_context context,
                            cl_context_info param_name,
                            size_t param_value_size,
                            void *param_value,
                            size_t *param_value_size_ret)
    {
        return cl_get_context_info(context,
                                   param_name,
                                   param_value_size,
                                   param_value,
                                   param_value_size_ret);
    }

    cl_command_queue clCreateCommandQueue(cl_context context,
                                          cl_device_id device,
                                          cl_command_queue_properties properties,
                                          cl_int *errcode_ret)
    {
        try
        {
            return cl_create_command_queue(context,
                                           device,
                                           errcode_ret);
        }
        catch (const std::bad_alloc &)
        {
            if (errcode_ret)
                *errcode_ret = CL_OUT_OF_HOST_MEMORY;
            return NULL;
        }
        catch (const cl_error &e)
        {
            if (errcode_ret)
                *errcode_ret = e.errcode;
            return NULL;
        }
    }

    cl_int clRetainCommandQueue(cl_command_queue command_queue)
    {
        printf("clRetainCommandQueue is not available or yet to be implemented.\n");
        return CL_SUCCESS;
    }

    cl_int clReleaseCommandQueue(cl_command_queue command_queue)
    {
        if (command_queue == NULL)
            return CL_INVALID_COMMAND_QUEUE;

        mango_resource_deallocation(command_queue->tgx);
        mango_task_graph_destroy_all(command_queue->tgx);
        delete command_queue->tgx;
        delete command_queue;
        return CL_SUCCESS;
    }

    cl_int clGetCommandQueueInfo(cl_command_queue command_queue,
                                 cl_command_queue_info param_name,
                                 size_t param_value_size,
                                 void *param_value,
                                 size_t *param_value_size_ret)
    {
        return cl_get_command_queue_info(command_queue,
                                         param_name,
                                         param_value_size,
                                         param_value,
                                         param_value_size_ret);
    }

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
        try
        {
            return cl_create_buffer(context,
                                    flags,
                                    size,
                                    host_ptr,
                                    errcode_ret,
                                    num_kernels_in, kernels_in,
                                    num_kernels_out, kernels_out,
                                    buffer_id);
        }
        catch (const std::bad_alloc &)
        {
            if (errcode_ret)
                *errcode_ret = CL_OUT_OF_HOST_MEMORY;
            return NULL;
        }
        catch (const cl_error &e)
        {
            if (errcode_ret)
                *errcode_ret = e.errcode;
            return NULL;
        }
    }

    cl_mem clCreateSubBuffer(cl_mem buffer,
                             cl_mem_flags flags,
                             cl_buffer_create_type buffer_create_type,
                             const void *buffer_create_info,
                             cl_int *errcode_ret)
    {
        printf("clBuildProgram is not available or yet to be implemented.\n");
        return NULL;
    }

    cl_mem clCreateImage(cl_context context,
                         cl_mem_flags flags,
                         const cl_image_format *image_format,
                         const cl_image_desc *image_desc,
                         void *host_ptr,
                         cl_int *errcode_ret)
    {
        printf("clCreateImage is not available or yet to be implemented.\n");
        return NULL;
    }

    cl_int clRetainMemObject(cl_mem memobj)
    {
        printf("clRetainMemObject is not available or yet to be implemented.\n");
        return CL_SUCCESS;
    }

    cl_int clReleaseMemObject(cl_mem memobj)
    {
        if (memobj == NULL)
            return CL_INVALID_MEM_OBJECT;

        printf("[clReleaseMemObject] deallocating buffer %d\n", memobj->id);
        mango_deregister_memory(memobj->buffer);
        delete memobj;
        return CL_SUCCESS;
    }

    cl_int clGetSupportedImageFormats(cl_context context,
                                      cl_mem_flags flags,
                                      cl_mem_object_type image_type,
                                      cl_uint num_entries,
                                      cl_image_format *image_formats,
                                      cl_uint *num_image_formats)
    {
        printf("clGetSupportedImageFormats is not available or yet to be implemented.\n");
        return CL_SUCCESS;
    }

    cl_int clGetMemObjectInfo(cl_mem memobj,
                              cl_mem_info param_name,
                              size_t param_value_size,
                              void *param_value,
                              size_t *param_value_size_ret)
    {
        return cl_get_mem_object_info(memobj,
                                      param_name,
                                      param_value_size,
                                      param_value,
                                      param_value_size_ret);
    }

    cl_int clGetImageInfo(cl_mem image,
                          cl_image_info param_name,
                          size_t param_value_size,
                          void *param_value,
                          size_t *param_value_size_ret)
    {
        printf("clGetImageInfo is not available or yet to be implemented.\n");
        return CL_SUCCESS;
    }

    cl_int clSetMemObjectDestructorCallback(cl_mem memobj,
                                            void(CL_CALLBACK *pfn_notify)(cl_mem memobj, void *user_data),
                                            void *user_data)
    {
        printf("clGetImageInfo is not available or yet to be implemented.\n");
        return CL_SUCCESS;
    }

    cl_sampler clCreateSampler(cl_context context,
                               cl_bool normalized_coords,
                               cl_addressing_mode addressing_mode,
                               cl_filter_mode filter_mode,
                               cl_int *errcode_ret)
    {
        printf("clCreateSampler is not available or yet to be implemented.\n");
        return NULL;
    }

    cl_int clRetainSampler(cl_sampler sampler)
    {
        printf("clRetainSampler is not available or yet to be implemented.\n");
        return CL_SUCCESS;
    }

    cl_int clReleaseSampler(cl_sampler sampler)
    {
        printf("clReleaseSampler is not available or yet to be implemented.\n");
        delete sampler;
        return CL_SUCCESS;
    }

    cl_int clGetSamplerInfo(cl_sampler sampler,
                            cl_sampler_info param_name,
                            size_t param_value_size,
                            void *param_value,
                            size_t *param_value_size_ret)
    {
        printf("clGetSamplerInfo is not available or yet to be implemented.\n");
        return CL_SUCCESS;
    }

    cl_program clCreateProgramWithSource(cl_context context,
                                         cl_uint count,
                                         const char **strings,
                                         const size_t *lengths,
                                         cl_int *errcode_ret)
    {
        printf("clCreateProgramWithSource is not available in this implementation. Please only use and refer to clCreateProgramWithBinary.\n");
        return NULL;
    }

    cl_program clCreateProgramWithBinary(cl_context context,
                                         cl_uint num_devices,
                                         const cl_device_id *device_list,
                                         const size_t *lengths, /* not needed */
                                         const char **binaries,
                                         cl_int *binary_status,
                                         cl_int *errcode_ret)
    {
        try
        {
            return cl_create_program_with_binary(context,
                                                 num_devices,
                                                 device_list,
                                                 binaries,
                                                 binary_status,
                                                 errcode_ret);
        }
        catch (const std::bad_alloc &)
        {
            if (errcode_ret)
                *errcode_ret = CL_OUT_OF_HOST_MEMORY;
            return NULL;
        }
        catch (const cl_error &e)
        {
            if (errcode_ret)
                *errcode_ret = e.errcode;
            return NULL;
        }
    }

    cl_program clCreateProgramWithBuiltInKernels(cl_context context,
                                                 cl_uint num_devices,
                                                 const cl_device_id *device_list,
                                                 const char *kernel_names,
                                                 cl_int *errcode_ret)
    {
        printf("clCreateProgramWithBuiltInKernels is not available in this implementation. Please only use and refer to clCreateProgramWithBinary.\n");
        return NULL;
    }

    cl_int clRetainProgram(cl_program program)
    {
        printf("clBuildProgram is not available or yet to be implemented.\n");
        return CL_SUCCESS;
    }

    cl_int clReleaseProgram(cl_program program)
    {
        if (program == NULL)
            return CL_INVALID_PROGRAM;

        printf("clReleaseProgram is not available or yet to be implemented.\n");
        delete program;
        return CL_SUCCESS;
    }

    cl_int clBuildProgram(cl_program program,
                          cl_uint num_devices,
                          const cl_device_id *device_list,
                          const char *options,
                          void(CL_CALLBACK *pfn_notify)(cl_program program, void *user_data),
                          void *user_data)
    {
        printf("clBuildProgram is not available or yet to be implemented.\n");
        return CL_SUCCESS;
    }

    cl_int clCompileProgram(cl_program /* program */,
                            cl_uint /* num_devices */,
                            const cl_device_id * /* device_list */,
                            const char * /* options */,
                            cl_uint /* num_input_headers */,
                            const cl_program * /* input_headers */,
                            const char ** /* header_include_names */,
                            void(CL_CALLBACK * /* pfn_notify */)(cl_program /* program */, void * /* user_data */),
                            void * /* user_data */)
    {
        printf("clCompileProgram is not available or yet to be implemented.\n");
        return CL_SUCCESS;
    }

    cl_program clLinkProgram(cl_context /* context */,
                             cl_uint /* num_devices */,
                             const cl_device_id * /* device_list */,
                             const char * /* options */,
                             cl_uint /* num_input_programs */,
                             const cl_program * /* input_programs */,
                             void(CL_CALLBACK * /* pfn_notify */)(cl_program /* program */, void * /* user_data */),
                             void * /* user_data */,
                             cl_int * /* errcode_ret */)
    {
        printf("clLinkProgram is not available or yet to be implemented.\n");
        return NULL;
    }

    cl_int clUnloadPlatformCompiler(cl_platform_id platform)
    {
        printf("clUnloadPlatformCompiler is not available or yet to be implemented.\n");
        return CL_SUCCESS;
    }

    cl_int clGetProgramInfo(cl_program program,
                            cl_program_info param_name,
                            size_t param_value_size,
                            void *param_value,
                            size_t *param_value_size_ret)
    {
        return cl_get_program_info(program,
                                   param_name,
                                   param_value_size,
                                   param_value,
                                   param_value_size_ret);
    }

    cl_int clGetProgramBuildInfo(cl_program program,
                                 cl_device_id device,
                                 cl_program_build_info param_name,
                                 size_t param_value_size,
                                 void *param_value,
                                 size_t *param_value_size_ret)
    {
        printf("clGetProgramBuildInfo is not available or yet to be implemented.\n");
        return CL_SUCCESS;
    }

    cl_kernel clCreateKernel(cl_program program,
                             const char *kernel_name, /* this is the binary path */
                             cl_int *errcode_ret,
                             cl_int kernel_id)
    {
        try
        {
            return cl_create_kernel(program,
                                    kernel_name,
                                    errcode_ret,
                                    kernel_id);
        }
        catch (const std::bad_alloc &)
        {
            if (errcode_ret)
                *errcode_ret = CL_OUT_OF_HOST_MEMORY;
            return NULL;
        }
        catch (const cl_error &e)
        {
            if (errcode_ret)
                *errcode_ret = e.errcode;
            return NULL;
        }
    }

    // TODO: this can be implemented for future use after defining the realtionship between kernels and binaries in a program
    cl_int clCreateKernelsInProgram(cl_program program,
                                    cl_uint num_kernels,
                                    cl_kernel *kernels,
                                    cl_uint *num_kernels_ret)
    {
        printf("clCreateKernelsInProgram is not available or yet to be implemented.\n");
        return CL_SUCCESS;
    }

    cl_int clRetainKernel(cl_kernel kernel)
    {
        printf("clRetainKernel is not available or yet to be implemented.\n");
        return CL_SUCCESS;
    }

    cl_int clReleaseKernel(cl_kernel kernel)
    {
        if (kernel == NULL)
            return CL_INVALID_KERNEL;

        // mango_deregister_kernel(kernel->kernel); // TODO : implement it in mango.cpp
        printf("clReleaseKernel is not available or yet to be implemented.\n");
        delete kernel;
        return CL_SUCCESS;
    }

    cl_int clSetKernelArg(cl_kernel kernel,
                          cl_uint arg_index,
                          size_t arg_size,
                          const void *arg_value,
                          cl_argument_type arg_type)
    {
        try
        {
            return cl_set_kernel_arg(kernel,
                                     arg_index,
                                     arg_size,
                                     arg_value,
                                     arg_type);
        }
        catch (const std::bad_alloc &)
        {
            return CL_OUT_OF_HOST_MEMORY;
        }
        catch (const cl_error &e)
        {
            return e.errcode;
        }
    }

    cl_int clGetKernelInfo(cl_kernel kernel,
                           cl_kernel_info param_name,
                           size_t param_value_size,
                           void *param_value,
                           size_t *param_value_size_ret)
    {
        return cl_get_kernel_info(kernel,
                                  param_name,
                                  param_value_size,
                                  param_value,
                                  param_value_size_ret);
    }

    cl_int clGetKernelArgInfo(cl_kernel kernel,
                              cl_uint arg_indx,
                              cl_kernel_arg_info param_name,
                              size_t param_value_size,
                              void *param_value,
                              size_t *param_value_size_ret)
    {
        printf("clGetKernelArgInfo is not available or yet to be implemented.\n");
        return CL_SUCCESS;
    }

    cl_int clGetKernelWorkGroupInfo(cl_kernel kernel,
                                    cl_device_id device,
                                    cl_kernel_work_group_info param_name,
                                    size_t param_value_size,
                                    void *param_value,
                                    size_t *param_value_size_ret)
    {
        printf("clGetKernelWorkGroupInfo is not available or yet to be implemented.\n");
        return CL_SUCCESS;
    }

    cl_int clWaitForEvents(cl_uint num_events,
                           const cl_event *event_list)
    {
        try
        {
            return cl_wait_for_events(num_events,
                                      event_list);
        }
        catch (const std::bad_alloc &)
        {
            return CL_OUT_OF_HOST_MEMORY;
        }
        catch (const cl_error &e)
        {
            return e.errcode;
        }
    }

    cl_int clGetEventInfo(cl_event event,
                          cl_event_info param_name,
                          size_t param_value_size,
                          void *param_value,
                          size_t *param_value_size_ret)
    {
        return cl_get_event_info(event,
                                 param_name,
                                 param_value_size,
                                 param_value,
                                 param_value_size_ret);
    }

    // TODO : there's maybe a potential mapping with mango_register_event but this function needs some additional parameters
    cl_event clCreateUserEvent(cl_context context,
                               cl_int *errcode_ret)
    {
        printf("clCreateUserEvent is not available or yet to be implemented.\n");
        return NULL;
    }

    cl_int clRetainEvent(cl_event event)
    {
        printf("clRetainEvent is not available or yet to be implemented.\n");
        return CL_SUCCESS;
    }

    // TODO : there's mapping with mango_deregister_event but its implementation is missing for now
    cl_int clReleaseEvent(cl_event event)
    {
        printf("clReleaseEvent is not available or yet to be implemented.\n");
        delete event;
        return CL_SUCCESS;
    }

    cl_int clSetUserEventStatus(cl_event event,
                                cl_int execution_status)
    {
        printf("clSetUserEventStatus is not available or yet to be implemented.\n");
        return CL_SUCCESS;
    }

    cl_int clSetEventCallback(cl_event event,
                              cl_int command_exec_callback_type,
                              void(CL_CALLBACK *pfn_notify)(cl_event, cl_int, void *),
                              void *user_data)
    {
        printf("clSetEventCallback is not available or yet to be implemented.\n");
        return CL_SUCCESS;
    }

    cl_int clGetEventProfilingInfo(cl_event event,
                                   cl_profiling_info param_name,
                                   size_t param_value_size,
                                   void *param_value,
                                   size_t *param_value_size_ret)
    {
        printf("clGetEventProfilingInfo is not available or yet to be implemented.\n");
        return CL_SUCCESS;
    }

    // TODO : is this really needed? Does mango do an implicit flush as soon as commands are issued?
    cl_int clFlush(cl_command_queue command_queue)
    {
        printf("clFlush is not available or yet to be implemented.\n");
        return CL_SUCCESS;
    }

    cl_int clFinish(cl_command_queue command_queue)
    {
        return cl_finish(command_queue);
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
        try
        {
            return cl_enqueue_read_buffer(command_queue,
                                          buffer,
                                          blocking_write,
                                          ptr,
                                          num_events_in_wait_list, event_wait_list,
                                          event);
        }
        catch (const std::bad_alloc &)
        {
            return CL_OUT_OF_HOST_MEMORY;
        }
        catch (const cl_error &e)
        {
            return e.errcode;
        }
    }

    cl_int clEnqueueReadBufferRect(cl_command_queue command_queue,
                                   cl_mem buffer,
                                   cl_bool blocking_read,
                                   const size_t *buffer_offset,
                                   const size_t *host_offset,
                                   const size_t *region,
                                   size_t buffer_row_pitch,
                                   size_t buffer_slice_pitch,
                                   size_t host_row_pitch,
                                   size_t host_slice_pitch,
                                   void *ptr,
                                   cl_uint num_events_in_wait_list,
                                   const cl_event *event_wait_list,
                                   cl_event *event)
    {
        printf("clEnqueueReadBufferRect is not available or yet to be implemented.\n");
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
        try
        {
            return cl_enqueue_write_buffer(command_queue,
                                           buffer,
                                           blocking_write,
                                           ptr,
                                           num_events_in_wait_list, event_wait_list,
                                           event);
        }
        catch (const std::bad_alloc &)
        {
            return CL_OUT_OF_HOST_MEMORY;
        }
        catch (const cl_error &e)
        {
            return e.errcode;
        }
    }

    cl_int clEnqueueWriteBufferRect(cl_command_queue command_queue,
                                    cl_mem buffer,
                                    cl_bool blocking_write,
                                    const size_t *buffer_offset,
                                    const size_t *host_offset,
                                    const size_t *region,
                                    size_t buffer_row_pitch,
                                    size_t buffer_slice_pitch,
                                    size_t host_row_pitch,
                                    size_t host_slice_pitch,
                                    const void *ptr,
                                    cl_uint num_events_in_wait_list,
                                    const cl_event *event_wait_list,
                                    cl_event *event)
    {
        printf("clEnqueueWriteBufferRect is not available or yet to be implemented.\n");
        return CL_SUCCESS;
    }

    cl_int clEnqueueFillBuffer(cl_command_queue command_queue,
                               cl_mem buffer,
                               const void *pattern,
                               size_t pattern_size,
                               size_t offset,
                               size_t size,
                               cl_uint num_events_in_wait_list,
                               const cl_event *event_wait_list,
                               cl_event *event)
    {
        printf("clEnqueueFillBuffer is not available or yet to be implemented.\n");
        return CL_SUCCESS;
    }

    cl_int clEnqueueCopyBuffer(cl_command_queue command_queue,
                               cl_mem src_buffer,
                               cl_mem dst_buffer,
                               size_t src_offset,
                               size_t dst_offset,
                               size_t size,
                               cl_uint num_events_in_wait_list,
                               const cl_event *event_wait_list,
                               cl_event *event)
    {
        printf("clEnqueueCopyBuffer is not available or yet to be implemented.\n");
        return CL_SUCCESS;
    }

    cl_int clEnqueueCopyBufferRect(cl_command_queue command_queue,
                                   cl_mem src_buffer,
                                   cl_mem dst_buffer,
                                   const size_t *src_origin,
                                   const size_t *dst_origin,
                                   const size_t *region,
                                   size_t src_row_pitch,
                                   size_t src_slice_pitch,
                                   size_t dst_row_pitch,
                                   size_t dst_slice_pitch,
                                   cl_uint num_events_in_wait_list,
                                   const cl_event *event_wait_list,
                                   cl_event *event)
    {
        printf("clEnqueueCopyBufferRect is not available or yet to be implemented.\n");
        return CL_SUCCESS;
    }

    // FIX why this introduces a compilation error?
    // cl_int clEnqueueReadImage(cl_command_queue command_queue,
    //                           cl_mem image,
    //                           cl_bool blocking_read,
    //                           const size_t *origin[3],
    //                           const size_t *region[3],
    //                           size_t row_pitch,
    //                           size_t slice_pitch,
    //                           void *ptr,
    //                           cl_uint num_events_in_wait_list,
    //                           const cl_event *event_wait_list,
    //                           cl_event *event)
    // {
    //     printf("clEnqueueReadImage is not available or yet to be implemented.\n");
    //     return CL_SUCCESS;
    // }

    // FIX why this introduces a compilation error?
    // cl_int clEnqueueWriteImage(cl_command_queue command_queue,
    //                            cl_mem image,
    //                            cl_bool blocking_write,
    //                            const size_t *origin[3],
    //                            const size_t *region[3],
    //                            size_t input_row_pitch,
    //                            size_t input_slice_pitch,
    //                            const void *ptr,
    //                            cl_uint num_events_in_wait_list,
    //                            const cl_event *event_wait_list,
    //                            cl_event *event)
    // {
    //     printf("clEnqueueWriteImage is not available or yet to be implemented.\n");
    //     return CL_SUCCESS;
    // }

    // FIX why this introduces a compilation error?
    // cl_int clEnqueueFillImage(cl_command_queue command_queue,
    //                           cl_mem image,
    //                           const void *fill_color,
    //                           const size_t *origin[3],
    //                           const size_t *region[3],
    //                           cl_uint num_events_in_wait_list,
    //                           const cl_event *event_wait_list,
    //                           cl_event *event)
    // {
    //     printf("clEnqueueFillImage is not available or yet to be implemented.\n");
    //     return CL_SUCCESS;
    // }

    // FIX why this introduces a compilation error?
    // cl_int clEnqueueCopyImage(cl_command_queue command_queue,
    //                           cl_mem src_image,
    //                           cl_mem dst_image,
    //                           const size_t *src_origin[3],
    //                           const size_t *dst_origin[3],
    //                           const size_t *region[3],
    //                           cl_uint num_events_in_wait_list,
    //                           const cl_event *event_wait_list,
    //                           cl_event *event)
    // {
    //     printf("clEnqueueCopyImage is not available or yet to be implemented.\n");
    //     return CL_SUCCESS;
    // }

    // FIX why this introduces a compilation error?
    // cl_int clEnqueueCopyImageToBuffer(cl_command_queue command_queue,
    //                                   cl_mem src_image,
    //                                   cl_mem dst_buffer,
    //                                   const size_t *src_origin[3],
    //                                   const size_t *region[3],
    //                                   size_t dst_offset,
    //                                   cl_uint num_events_in_wait_list,
    //                                   const cl_event *event_wait_list,
    //                                   cl_event *event)
    // {
    //     printf("clEnqueueCopyImageToBuffer is not available or yet to be implemented.\n");
    //     return CL_SUCCESS;
    // }

    // FIX why this introduces a compilation error?
    // cl_int clEnqueueCopyBufferToImage(cl_command_queue command_queue,
    //                                   cl_mem src_buffer,
    //                                   cl_mem dst_image,
    //                                   size_t src_offset,
    //                                   const size_t *dst_origin[3],
    //                                   const size_t *region[3],
    //                                   cl_uint num_events_in_wait_list,
    //                                   const cl_event *event_wait_list,
    //                                   cl_event *event)
    // {
    //     printf("clEnqueueCopyBufferToImage is not available or yet to be implemented.\n");
    //     return CL_SUCCESS;
    // }

    void *clEnqueueMapBuffer(cl_command_queue command_queue,
                             cl_mem buffer,
                             cl_bool blocking_map,
                             cl_map_flags map_flags,
                             size_t offset,
                             size_t size,
                             cl_uint num_events_in_wait_list,
                             const cl_event *event_wait_list,
                             cl_event *event,
                             cl_int *errcode_ret)
    {
        printf("clEnqueueMapBuffer is not available or yet to be implemented.\n");
        return NULL;
    }

    // FIX why this introduces a compilation error?
    // void *clEnqueueMapImage(cl_command_queue command_queue,
    //                         cl_mem image,
    //                         cl_bool blocking_map,
    //                         cl_map_flags map_flags,
    //                         const size_t *origin[3],
    //                         const size_t *region[3],
    //                         size_t *image_row_pitch,
    //                         size_t *image_slice_pitch,
    //                         cl_uint num_events_in_wait_list,
    //                         const cl_event *event_wait_list,
    //                         cl_event *event,
    //                         cl_int *errcode_ret)
    // {
    //     printf("clEnqueueMapImage is not available or yet to be implemented.\n");
    //     return NULL;
    // }

    cl_int clEnqueueUnmapMemObject(cl_command_queue command_queue,
                                   cl_mem memobj,
                                   void *mapped_ptr,
                                   cl_uint num_events_in_wait_list,
                                   const cl_event *event_wait_list,
                                   cl_event *event)
    {
        printf("clEnqueueUnmapMemObject is not available or yet to be implemented.\n");
        return CL_SUCCESS;
    }

    cl_int clEnqueueMigrateMemObjects(cl_command_queue command_queue,
                                      cl_uint num_mem_objects,
                                      const cl_mem *mem_objects,
                                      cl_mem_migration_flags flags,
                                      cl_uint num_events_in_wait_list,
                                      const cl_event *event_wait_list,
                                      cl_event *event)
    {
        printf("clEnqueueMigrateMemObjects is not available or yet to be implemented.\n");
        return CL_SUCCESS;
    }

    cl_int clEnqueueTask(cl_command_queue command_queue,
                         cl_kernel kernel,
                         cl_uint num_events_in_wait_list,
                         const cl_event *event_wait_list,
                         cl_event *event)
    {
        try
        {
            return cl_enqueue_task(command_queue,
                                   kernel,
                                   num_events_in_wait_list, event_wait_list,
                                   event);
        }
        catch (const std::bad_alloc &)
        {
            return CL_OUT_OF_HOST_MEMORY;
        }
        catch (const cl_error &e)
        {
            return e.errcode;
        }
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
        try
        {
            return clEnqueueTask(command_queue,
                                 kernel,
                                 num_events_in_wait_list, event_wait_list,
                                 event);
        }
        catch (const std::bad_alloc &)
        {
            return CL_OUT_OF_HOST_MEMORY;
        }
        catch (const cl_error &e)
        {
            return e.errcode;
        }
    }

    // TODO check if this can be implemented in some way
    cl_int clEnqueueNativeKernel(cl_command_queue command_queue,
                                 void(CL_CALLBACK *user_func)(void *),
                                 void *args,
                                 size_t cb_args,
                                 cl_uint num_mem_objects,
                                 const cl_mem *mem_list,
                                 const void **args_mem_loc,
                                 cl_uint num_events_in_wait_list,
                                 const cl_event *event_wait_list,
                                 cl_event *event)
    {
        printf("clEnqueueNativeKernel is not available or yet to be implemented.\n");
        return CL_SUCCESS;
    }

    // TODO is the concept of "marker" present in mango?
    cl_int clEnqueueMarkerWithWaitList(cl_command_queue command_queue,
                                       cl_uint num_events_in_wait_list,
                                       const cl_event *event_wait_list,
                                       cl_event *event)
    {
        printf("clEnqueueMarkerWithWaitList is not available or yet to be implemented.\n");
        return CL_SUCCESS;
    }

    cl_int clEnqueueBarrierWithWaitList(cl_command_queue command_queue,
                                        cl_uint num_events_in_wait_list,
                                        const cl_event *event_wait_list,
                                        cl_event *event)
    {
        printf("clEnqueueBarrierWithWaitList is not available or yet to be implemented.\n");
        return CL_SUCCESS;
    }

    void *clGetExtensionFunctionAddressForPlatform(cl_platform_id platform,
                                                   const char *func_name)
    {
        printf("clGetExtensionFunctionAddressForPlatform is not available or yet to be implemented.\n");
        return NULL;
    }

    /* Additional functions to support the MANGO Platform */

    cl_int clSetInputBufferIDs(cl_program program, const char* binary, unsigned int nbuffers_in, ...)
    {
        assert(binary != NULL  && "the kernel function path is not valid ");
        assert(nbuffers_in < MAX_NUM_KERNEL_BUFFERS && "exceeded the maximum number of kernel buffers available");
       
        program->map_kernel_functions[binary].buffers_in.clear();

        va_list list;
        va_start(list, nbuffers_in);
        for (unsigned int i = 0; i < nbuffers_in; i++)
        {
            uint32_t in_id = (uint32_t)va_arg(list, cl_uint);
            program->map_kernel_functions[binary].buffers_in.push_back(in_id);
        }
        va_end(list);
    }

    
    cl_int clSetOutputBufferIDs(cl_program program, const char* binary, unsigned int nbuffers_out, ...)
    {
        assert(binary != NULL && "the kernel function path is not valid");
        assert(nbuffers_out < MAX_NUM_KERNEL_BUFFERS && "exceeded the maximum number of kernel buffers available");

        program->map_kernel_functions[binary].buffers_out.clear();

        va_list list;
        va_start(list, nbuffers_out);
        for (unsigned int i = 0; i < nbuffers_out; i++)
        {
            uint32_t out_id = (uint32_t)va_arg(list, cl_uint);
            program->map_kernel_functions[binary].buffers_out.push_back(out_id);
        }
        va_end(list);
    }

    
    cl_int clKernelAndBufferAllocation(cl_command_queue command_queue)
    {
        std::cout << "[clKernelAndBufferAllocation] allocating new resources in task graph: " << command_queue->tgx << std::endl;
        command_queue->tgx = mango_task_graph_add_event(command_queue->tgx, NULL); // FIX : this is the last part of mango allocation that should stay somewhere else
        mango_exit_t err = mango_resource_allocation(command_queue->tgx);
        if (err == SUCCESS)
        {
            printf("[clKernelAndBufferAllocation] allocation completed\n");
            return CL_SUCCESS;
        }
        else
            return CL_OUT_OF_RESOURCES;
    }

#ifdef __cplusplus
}
#endif
