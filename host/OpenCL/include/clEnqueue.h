#include "CL/cl.h"

#ifdef __cplusplus
#include <stdio.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    cl_int cl_enqueue_task(cl_command_queue command_queue,
                           cl_kernel kernel,
                           cl_uint num_events_in_wait_list,
                           const cl_event *event_wait_list,
                           cl_event *event);

    cl_int cl_enqueue_write_buffer(cl_command_queue command_queue,
                                   cl_mem buffer,
                                   const void *ptr,
                                   cl_uint num_events_in_wait_list,
                                   const cl_event *event_wait_list,
                                   cl_event *event);

    cl_int cl_enqueue_read_buffer(cl_command_queue command_queue,
                                  cl_mem buffer,
                                  void *ptr,
                                  cl_uint num_events_in_wait_list,
                                  const cl_event *event_wait_list,
                                  cl_event *event);

#ifdef __cplusplus
}
#endif
