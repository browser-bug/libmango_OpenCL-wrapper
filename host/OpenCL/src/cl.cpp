#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>     /* va_list, va_start, va_arg, va_end */
#include "cl.h"

#ifdef __cplusplus
 extern "C" {
#endif  
	

struct _cl_context
{
    cl_program p;
    mango_task_graph_t *tg;
};

struct _cl_program
{
    kernelfunction *kernfunc;
    mango_task_graph_t *tg;
    // TO BE FIXED: maybe it has to be moved
    mango_kernel_t mango_kernel;
};


// data is used  for the User's input data, in the case of a matrix multiplication it should contain a pointer to the matrix.
struct _cl_mem
{
    uint32_t id;
    mango_buffer_t buffer;
    void * data;

    // e is used to get_buffer_event of memC
    mango_event_t e ;
    // mango_ev is used as event from the kernel, it is here because in clEnqueuReadBuffer i didnt have access to kernel
    mango_event_t mango_ev;  
};


struct _cl_kernel
{
    uint32_t id;
    mango_kernel_t mango_kernel;
    std::vector<mango::Arg *> arguments;
    mango_event_t mango_ev;
};

struct _cl_platform_id
{

};


struct _cl_device_id
{	 
  // TO BE FIXED: maybe it should have at least an ID
};

struct _cl_command_queue
{
  // TO BE FIXED: maybe it should have at least an ID
};

struct _cl_event
{
  // TO BE FIXED: maybe it should have at least an ID
};

struct _cl_sampler
{
  // TO BE FIXED: maybe it should have at least an ID
};


     
cl_int clGetPlatformIDs(cl_uint num_entries, cl_platform_id *entries, cl_uint *num_platforms)
{
	// TO BE IMPLEMENTED
	return CL_SUCCESS;
}


cl_int clGetDeviceIDs(cl_platform_id  platform,
                cl_device_type    device_type , 
                cl_uint           num_entries , 
                cl_device_id *    devices , 
                cl_uint *         num_devices )
{
	 // TO BE IMPLEMENTED
	return CL_SUCCESS;
}


cl_context clCreateContext(const cl_context_properties *  properties ,
                 cl_uint                  num_devices ,
                 const cl_device_id *     devices ,
                 void (CL_CALLBACK *  pfn_notify )(const char *, const void *, size_t, void *),
                 void *                   user_data ,
                 cl_int *                 errcode_ret )
 {

    int result_init;    
    if ((result_init=mango_init("test", "test_manga")) == SUCCESS)
    {
        cl_context context = NULL;
        context = (cl_context)malloc(sizeof(struct _cl_context));
        return context;
    }else
    {
      printf("THERE WAS AN ERROR WHILE CALLING MANGO_INIT RESULT CODE: %d\n",result_init);
    }

 }


cl_command_queue clCreateCommandQueue(cl_context  context ,
                                      cl_device_id device ,
                                      cl_command_queue_properties  properties ,
                                      cl_int *  errcode_ret )
{

    // TO BE IMPLEMENTED: it returns a pointer to a _cl_command_queue just to make think the user
    // that the commandqueue was created correctly
    cl_command_queue commands;
    commands = (cl_command_queue)malloc(sizeof(_cl_command_queue));

    return commands;
}





cl_program clCreateProgramWithBinary(cl_context                      context ,
                           cl_uint                         num_devices ,
                           const cl_device_id *            device_list ,
                           const size_t *                  lengths ,
                           const unsigned char **          binaries ,
                           cl_int *                        binary_status ,
                           cl_int *                       errcode_ret )
{

    cl_program program = NULL;
    program = (cl_program)malloc(sizeof(struct _cl_program));
    program->kernfunc = mango_kernelfunction_init();
    char kernel_file[] = "/opt/mango/usr/local/share/matrix_multiplication/matrix_multiplication_dev";


    // Associate program with context
    context->p = program;
    // load the kernel binary 
	  mango_load_kernel(kernel_file, program->kernfunc, GN, BINARY);

     return program;
}



// since we are working with a binary file for the kernel probably this function wont be useful
// so it should do nothing
cl_int  clBuildProgram(cl_program           program,
                cl_uint               num_devices,
                const cl_device_id * device_list ,
             const char *         options , 
                void (CL_CALLBACK *   pfn_notify )(cl_program program , void *  user_data ),
                void *                user_data )
{

	return CL_SUCCESS;
}


   

#define KID 1
#define B1 1
#define B2 2
#define B3 3



cl_kernel clCreateKernel(cl_program       program ,
                const char *     kernel_name ,
                cl_int *         errcode_ret , ...) {
   

	*errcode_ret = CL_SUCCESS;
	 
  cl_kernel kernel;
	kernel = (cl_kernel) malloc(sizeof(_cl_kernel));

	kernel->id = KID;

  kernel->mango_kernel = mango_register_kernel(kernel->id, program->kernfunc, 2, 1, B1, B2, B3);  

	program->tg = NULL;
  program->mango_kernel = kernel->mango_kernel;

  printf("OKAY MANGO KERNEL IS : %d\n",(kernel->mango_kernel));
  program->tg = mango_task_graph_add_kernel(program->tg, &(program->mango_kernel));
  printf("OKAY MANGO KERNEL HAS BEEN ADDED TO TASK GRAPH\n");
	 return kernel;
}



  // to be fixed, we would like to have a dynamic number of buffers. I mean it should know when to call mango_resource_allocation
  // mango_task_graph_create will be fixed with mango_task_add_buffer and mango_task_add kernel
  int i=0;
  cl_mem d_C;
  cl_mem d_A;
  cl_mem d_B;
  
  cl_mem clCreateBuffer(cl_context context,
                        cl_mem_flags flags,
                        size_t size,
                        void *host_ptr,
                        cl_int *errcode_ret)
  {

  i++;
     if(i==3){
       	d_C = (cl_mem)malloc(sizeof(struct _cl_mem));
        d_C->id = i;
        // host_ptr contains a pointer to the matrix C
        d_C->data  = host_ptr;
        d_C->buffer = mango_register_memory(d_C->id, size, BUFFER, 1, 0, context->p->mango_kernel);
        context->p->tg = mango_task_graph_add_buffer(context->p->tg, &(d_C->buffer));

      	//context->p->tg = mango_task_graph_create(1, 3, 0, context->p->mango_kernel, d_A->buffer, d_B->buffer, d_C->buffer);
       

       	return d_C;

     }else if(i==2){
          d_B = (cl_mem)malloc(sizeof(struct _cl_mem));
          d_B->id= i;
          d_B->data = host_ptr;
          d_B->buffer = mango_register_memory(d_B->id, size, BUFFER, 0, 1, context->p->mango_kernel);
      //    context->p->tg = mango_task_graph_add_kernel(context->p->tg, &(context->p->mango_kernel));
        context->p->tg = mango_task_graph_add_buffer(context->p->tg, &(d_B->buffer));

          return d_B;

     }else if(i==1){
     		d_A = (cl_mem)malloc(sizeof(struct _cl_mem));
        d_A->id = i;
        d_A->data = host_ptr;
        d_A->buffer = mango_register_memory(d_A->id, size, BUFFER, 0, 1, context->p->mango_kernel);
      //  context->p->tg = mango_task_graph_add_kernel(context->p->tg, &(context->p->mango_kernel));
        context->p->tg = mango_task_graph_add_buffer(context->p->tg, &(d_A->buffer));

        return d_A;

     }else{
     	printf("ERROR IN CREATE BUFFER\n");
     }

/*
      // FIX this need to be generic, B1 can't be static
     /* if (flags != (CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR))
      {
          
          memory->buffer = mango_register_memory(memory->id, size, BUFFER, 1, 0, context->p->kernel);
      }
      else
      {
          memory->buffer = mango_register_memory(memory->id, size, BUFFER, 0, 1, context->p->kernel);
      }


      
      	context->p->tg = mango_task_graph_add_kernel(context->p->tg, &(kern->kernel));
      	context->p->tg = mango_task_graph_add_buffer(context->p->tg, &(memory->buffer));


*/
      // std::cout << "Returning memory with address: " << memory << std::endl;

  }





  // TO BE FIXED: we should have a dynamic container that allows to have multiple args 
  /* Execution preparation */
  mango_arg_t *arg1 ;
  mango_arg_t *arg2 ;
  mango_arg_t *arg3 ;
  mango_arg_t *arg4 ;
  mango_arg_t *arg5 ;
  mango_arg_t *arg6;

  mango_args_t *args;

cl_int cl_update_events_allocate_resource(cl_program program){

    program->tg = mango_task_graph_update_events(program->tg);
    mango_resource_allocation(program->tg);

    return CL_SUCCESS;
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
       
    }
    if(arg_index == 0){
      arg1 = mango_arg( kernel->mango_kernel, &(d_A->buffer), arg_size, arg_type);
    }else if(arg_index == 1){
      arg2 = mango_arg( kernel->mango_kernel, &(d_B->buffer), arg_size, arg_type);
    }else if (arg_index == 2){
      arg3 = mango_arg( kernel->mango_kernel, &(d_C->buffer), arg_size, arg_type);
    }else if (arg_index == 3 ){
      arg4 = mango_arg( kernel->mango_kernel, value, arg_size, arg_type);
    }else if (arg_index == 4){
      arg5 = mango_arg( kernel->mango_kernel, value, arg_size, arg_type);
      d_C->e= mango_get_buffer_event(d_C->buffer);
      arg6 = mango_arg( kernel->mango_kernel, &(d_C->e), sizeof(uint64_t), EVENT );
      args =mango_set_args(kernel->mango_kernel, 6, arg1, arg2, arg3, arg4, arg5, arg6);
    }else{
      printf("ERROR in clSEtKErnelARG!!!\n");
    }


    // FIX arg_value must be the address of a mango_buffer_t
   // std::cout << "Passing mango_buffer with address: " << arg_value << " and value: " << (*(uint32_t *)value) << std::endl;
  //  mango_arg_t *arg = mango_arg(mykernel->kernel, value, arg_size, arg_type);

   // std::cout << "created arg at address: " << arg << std::endl;
   // mykernel->arguments.push_back((mango::Arg *)arg);

    return CL_SUCCESS;
}



 // work dim can be used to iterate and get all the buffers that are used as input
 // event can be used as result of mango_start_kernel
 cl_int clEnqueueNDRangeKernel(cl_command_queue  command_queue ,
                        cl_kernel         kernel ,
                        cl_uint           work_dim ,
                        const size_t *    global_work_offset ,
                        const size_t *    global_work_size ,
                        const size_t *    local_work_size ,
                        cl_uint           num_events_in_wait_list ,
                        const cl_event *  event_wait_list ,
                        cl_event *        event) 
 {
  
  /* Data transfer host->device */
  mango_write(d_A->data, d_A->buffer, DIRECT, 0);
  mango_write(d_B->data, d_B->buffer, DIRECT, 0);

  /* spawn kernel */
  kernel->mango_ev = mango_start_kernel(kernel->mango_kernel, args, NULL);

  d_C->mango_ev = kernel->mango_ev;



  
 }


 // maybe blocking_read should be used for mango_wait(d_C->e)
 cl_int  clEnqueueReadBuffer(cl_command_queue     command_queue ,
                     cl_mem               buffer ,
                     cl_bool              blocking_read ,
                     size_t               offset ,
                     size_t               size , 
                     void *               ptr ,
                     cl_uint              num_events_in_wait_list ,
                     const cl_event *     event_wait_list ,
                     cl_event *           event )
 {

    /* reading results */
    mango_wait(d_C->e);
    mango_read(ptr, d_C->buffer, DIRECT, 0);


    /* wait for kernel completion */
    mango_wait(d_C->mango_ev);
 
    return CL_SUCCESS;

 }

                       
 cl_int  clReleaseMemObject(cl_mem  memobj ) 
 {
  if(memobj != NULL)
    free(memobj);
  return CL_SUCCESS;
 }


 cl_int  clReleaseProgram(cl_program  program )
 {

  /* shut down the mango infrastructure */
  mango_resource_deallocation(program->tg);
  mango_task_graph_destroy_all(program->tg);
  free(program);
 }


 cl_int  clReleaseKernel(cl_kernel    kernel )
 {
  if(kernel != NULL)
    free(kernel);
  return CL_SUCCESS;
 }


 cl_int  clReleaseCommandQueue(cl_command_queue  command_queue )
 {
  if(command_queue != NULL)
    free(command_queue);
  return CL_SUCCESS;
 }


 cl_int  clReleaseContext(cl_context context ) 
 {

  if(mango_release() != SUCCESS){
    printf("PROBLEM WHILE RELEASING MANGO!\n");
  }
  if(context != NULL)
    free(context);
  return CL_SUCCESS;
  }


#ifdef __cplusplus
 }
#endif     






