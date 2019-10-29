#include <stdlib.h>
#include <stdio.h>
#include "cl.h"

#ifdef __cplusplus
 extern "C" {
#endif  
	

struct _cl_context
{
    cl_program p;
    // cl_kern k;
    // cl_mem m;
    mango_task_graph_t *tg;
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
    void * data;
};

struct _cl_kernel
{
    uint32_t id;
    mango_kernel_t kernel;
    std::vector<mango::Arg *> arguments;
};

struct _cl_platform_id
{

	struct _cl_device_id *devices;
};


struct _cl_device_id
{	

};
struct _cl_command_queue
{

};
struct _cl_event
{

};

struct _cl_sampler
{

};


     
cl_int clGetPlatformIDs(cl_uint num_entries, cl_platform_id *entries, cl_uint *num_platforms){
	// to be implemented
	return CL_SUCCESS;}

cl_int clGetDeviceIDs(cl_platform_id  platform,
                cl_device_type    device_type , 
                cl_uint           num_entries , 
                cl_device_id *    devices , 
                cl_uint *         num_devices ){
	// to be implemented
	return CL_SUCCESS;}



cl_context mycontext = NULL;
// this function allows to create a context
cl_context clCreateContext(const cl_context_properties *  properties ,
                 cl_uint                  num_devices ,
                 const cl_device_id *     devices ,
                 void (CL_CALLBACK *  pfn_notify )(const char *, const void *, size_t, void *),
                 void *                   user_data ,
                 cl_int *                 errcode_ret )
 {
        if (mango_init("test", "test_manga") == SUCCESS)
        {
            
            mycontext = (cl_context)malloc(sizeof(struct _cl_context));
            return mycontext;
        }
 }


cl_command_queue clCreateCommandQueue(cl_context /* context */,
                                      cl_device_id /* device */,
                                      cl_command_queue_properties /* properties */,
                                      cl_int * /* errcode_ret */)
{
    printf("CreateCommandQueue is not implemented\n");
    cl_command_queue commands;
    commands = (cl_command_queue)malloc(sizeof(_cl_command_queue));
    

    return commands;
}




cl_program myprogram = NULL;

cl_program clCreateProgramWithBinary(cl_context                      context ,
                           cl_uint                         num_devices ,
                           const cl_device_id *            device_list ,
                           const size_t *                  lengths ,
                           const unsigned char **          binaries ,
                           cl_int *                        binary_status ,
                           cl_int *                       errcode_ret ){



     myprogram = (cl_program)malloc(sizeof(struct _cl_program));
     myprogram->kernfunc = mango_kernelfunction_init();
  char kernel_file[] = "/opt/mango/usr/local/share/matrix_multiplication/matrix_multiplication_dev";

     // Associate program with context
     mycontext->p = myprogram;
	  mango_load_kernel(kernel_file, myprogram->kernfunc, GN, BINARY);


     return myprogram;
}




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



cl_kernel mykernel;
cl_kernel clCreateKernel(cl_program       program ,
                const char *     kernel_name ,
                cl_int *         errcode_ret ) {
   

	*errcode_ret = CL_SUCCESS;
	 
	mykernel = (cl_kernel) malloc(sizeof(_cl_kernel));

	mykernel->id = KID;

  mykernel->kernel = mango_register_kernel(mykernel->id, myprogram->kernfunc, 2, 1, B1, B2, B3);  

	myprogram->tg = NULL;
  myprogram->kernel = mykernel->kernel;
	 return mykernel;
 
}


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
       	printf("chiamando 3\n");
        d_C->data  = host_ptr;
        d_C->buffer = mango_register_memory(d_C->id, size, BUFFER, 1, 0, mycontext->p->kernel);
        printf("chiamando 4\n");

      	mycontext->p->tg = mango_task_graph_create(1, 3, 0, mycontext->p->kernel, d_A->buffer, d_B->buffer, d_C->buffer);
        printf("chiamando 5\n");

        mango_resource_allocation(mycontext->p->tg);

        printf("chiamando 6\n");
      	return d_C;

     }else if(i==2){
          printf("chiamando 2\n");
     	    d_B = (cl_mem)malloc(sizeof(struct _cl_mem));
          d_B->id= i;
          d_B->data = host_ptr;
          d_B->buffer = mango_register_memory(d_B->id, size, BUFFER, 0, 1, mycontext->p->kernel);

          return d_B;

     }else if(i==1){
        printf("chiamando 1\n");
     		d_A = (cl_mem)malloc(sizeof(struct _cl_mem));
        d_A->id = i;
        d_A->data = host_ptr;
        d_A->buffer = mango_register_memory(d_A->id, size, BUFFER, 0, 1, mycontext->p->kernel);
        return d_A;

     }else{
     	printf("ERROR IN CREATED BUFFER\n");
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


      /*
            i++;
            switch(i){
            	case 1: b1 = memory;break;
            	case 2: b2 = memory;break;
            	case 3:
      	        	//
      	        	//i=0;
      	        	break;
            }
            
      /*        
      	context->p->tg = mango_task_graph_add_kernel(NULL, &(kern->kernel));
      	context->p->tg = mango_task_graph_add_buffer(context->p->tg, &(memory->buffer));


*/
      // std::cout << "Returning memory with address: " << memory << std::endl;
  }




  /* Execution preparation */
  mango_arg_t *arg1 ;
  mango_arg_t *arg2 ;
  mango_arg_t *arg3 ;
  mango_arg_t *arg4 ;
  mango_arg_t *arg5 ;
  mango_event_t e ;
  mango_arg_t *arg6;

  mango_args_t *args;


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
      arg1 = mango_arg( mykernel->kernel, &(d_A->buffer), arg_size, arg_type);
    }else if(arg_index == 1){
      arg2 = mango_arg( mykernel->kernel, &(d_B->buffer), arg_size, arg_type);
    }else if (arg_index == 2){
      arg3 = mango_arg( mykernel->kernel, &(d_C->buffer), arg_size, arg_type);
    }else if (arg_index == 3 ){
      arg4 = mango_arg( mykernel->kernel, value, arg_size, arg_type);
    }else if (arg_index == 4){
      arg5 = mango_arg( mykernel->kernel, value, arg_size, arg_type);
      e= mango_get_buffer_event(d_C->buffer);
      arg6 = mango_arg( mykernel->kernel, &e, sizeof(uint64_t), EVENT );
      args =mango_set_args(mykernel->kernel, 6, arg1, arg2, arg3, arg4, arg5, arg6);
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
mango_event_t ev;
 cl_int clEnqueueNDRangeKernel(cl_command_queue  command_queue ,
                        cl_kernel         kernel ,
                        cl_uint           work_dim ,
                        const size_t *    global_work_offset ,
                        const size_t *    global_work_size ,
                        const size_t *    local_work_size ,
                        cl_uint           num_events_in_wait_list ,
                        const cl_event *  event_wait_list ,
                        cl_event *        event) {


  /* Data transfer host->device */
  mango_write(d_A->data, d_A->buffer, DIRECT, 0);
  mango_write(d_B->data, d_B->buffer, DIRECT, 0);

  /* spawn kernel */
  ev = mango_start_kernel(mykernel->kernel, args, NULL);

  /* reading results */
  mango_wait(e);
  mango_read(d_C->data, d_C->buffer, DIRECT, 0);

  /* wait for kernel completion */
  mango_wait(ev);


  /* shut down the mango infrastructure */
  mango_resource_deallocation(mycontext->p->tg);
  mango_task_graph_destroy_all(mycontext->p->tg);
  mango_release();
  
 }

/* 
	struct _cl_device_id device0;


	void init(cl_device_id * device){
			device[0] = &device0;
			device0.ciao = "AABB";
			printf("%s\n", ((device[0])->ciao) );

	}


 	void get_device_info(cl_device_id * device)
    {
			printf("%s\n", ((device[0])->ciao) );


    }


    */
#ifdef __cplusplus
 }
#endif     






