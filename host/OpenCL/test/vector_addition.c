#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <CL/cl.h>
 
// OpenCL kernel. Each work item takes care of one element of c
const char *kernelSource =                                       "\n" \
"#pragma OPENCL EXTENSION cl_khr_fp64 : enable                    \n" \
"__kernel void vecAdd(  __global double *a,                       \n" \
"                       __global double *b,                       \n" \
"                       __global double *c,                       \n" \
"                       const unsigned int n)                    \n" \
"{                                                               \n" \
"    //Get our global thread ID                                  \n" \
"    int id = get_global_id(0);                                  \n" \
"                                                                \n" \
"    //Make sure we do not go out of bounds                      \n" \
"    if (id < n)                                                 \n" \
"        c[id] = a[id] + b[id];                                  \n" \
"}                                                               \n" \
                                                                "\n" ;
 
int main( int argc, char* argv[] )
{
    // Length of vectors
    unsigned int n = 20;
 
    // Host input vectors
    int *h_a;
    int *h_b;
    // Host output vector
    int *h_c;
 
    // Device input buffers
    cl_mem d_a;
    cl_mem d_b;
    // Device output buffer
    cl_mem d_c;

    cl_platform_id cpPlatform;        // OpenCL platform
    cl_device_id device_id;           // device ID
    cl_context context;               // context
    cl_command_queue queue;           // command queue
    cl_program program;               // program
    cl_kernel kernel;                 // kernel


    // Size, in bytes, of each vector
    size_t bytes = n*sizeof(int);
 
    // Allocate memory for each vector on host
    h_a = (int*)malloc(bytes);
    h_b = (int*)malloc(bytes);
    h_c = (int*)malloc(bytes);
 
    // Initialize vectors on host
    int i;
    for( i = 0; i < n; i++ )
    {
        h_a[i] = 5;
        h_b[i] = 5;
    }
 
    size_t globalSize, localSize;
    cl_int err;
 
    // Number of work items in each local work group
    localSize = 64;
 
    // Number of total work items - localSize must be devisor
    globalSize = ceil(n/(float)localSize)*localSize;
    printf("Initializing OpenCL device...\n");


    clGetPlatformIDs(1, &cpPlatform, NULL);
    
    // Get ID for the device
    err = clGetDeviceIDs(cpPlatform, CL_DEVICE_TYPE_CPU, 1, &device_id, NULL);

    // Create a context  
    const char *mango_recipe = "test_manga";
    context = clCreateContext(0, 1, &device_id, NULL, mango_recipe, &err);

    // Create a command queue 
    queue = clCreateCommandQueue(context, device_id, 0, &err);

    cl_int errNum = 0;
    cl_int binaryStatus[1];
    const char *programBinaryPaths[1];
    programBinaryPaths[0] = "/opt/mango/usr/local/share/matrix_multiplication/vector_addition_dev";
  
    // Create program from the binary Kernel

    program = clCreateProgramWithBinary(context, 1, &device_id,
                                        NULL, /* not needed */
                                        (const char **)programBinaryPaths,
                                        binaryStatus,
                                        &errNum);
    
    // Set the buffer IDS for the kernel we wish to run first
    clSetInputBufferIDs(program, programBinaryPaths[0], 2, 1, 2);
    clSetOutputBufferIDs(program, programBinaryPaths[0], 1, 3);
    

    // Create the compute kernel in the program we wish to run
    kernel = clCreateKernel(program, programBinaryPaths[0], NULL, 1);
    
    // Create the input and output arrays in device memory for our calculation
    d_a = clCreateBuffer(context, CL_MEM_READ_ONLY, bytes, h_a, &err,
                         0, NULL,
                         1, &kernel,
                         1);
    d_b = clCreateBuffer(context, CL_MEM_READ_ONLY, bytes, h_b, &err,
                         0, NULL,
                         1, &kernel,
                         2);
    d_c = clCreateBuffer(context, CL_MEM_WRITE_ONLY,bytes, NULL, &err,
                         1, &kernel,
                         0, NULL,
                         3);
   if (!d_a || !d_b || !d_c)
    {
        printf("Error: Failed to allocate device memory!\n");
        exit(1);
    }

    int cols=1;
    // Set the arguments to our compute kernel
    err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&d_a, CL_BUFFER_ARG);
    err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&d_b, CL_BUFFER_ARG);
    err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&d_c, CL_BUFFER_ARG);
    err |= clSetKernelArg(kernel, 3, sizeof(int), (void *)&n, CL_SCALAR_ARG);
    err |= clSetKernelArg(kernel, 4, sizeof(int), (void *)&cols, CL_SCALAR_ARG);
    cl_event bufferEvent;
    err |= clSetKernelArg(kernel, 2, sizeof(cl_event), (void *)&bufferEvent, CL_EVENT_ARG);

    // User needs to explicitly allocate resources for the task graph
    clKernelAndBufferAllocation(queue);


     // Write our data set into the input array in device memory
    err = clEnqueueWriteBuffer(queue, d_a, NULL, NULL,
                                   NULL, h_a, NULL, NULL, NULL);
    err |= clEnqueueWriteBuffer(queue, d_b, NULL, NULL,
                                   NULL, h_b, NULL, NULL, NULL);
     // Adding an event to synchronization purpuses
    cl_event kernelEvent;

    // Execute the kernel over the entire range of the data set  
    err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &globalSize, &localSize, 0, NULL, &kernelEvent);
  
    // Wait for the command queue to get serviced before reading back results
    clFinish(queue);
 
    //Retrieve result from device
    cl_event readEvent;
    err = clEnqueueReadBuffer(queue, d_c, CL_TRUE, 0, bytes, h_c, 1, &bufferEvent, NULL);
    
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to read output array! %d\n", err);
        exit(1);
    }
     printf("Vector Addition completed...\n");


    //Sum up vector c and print result divided by n
    double sum = 0;
    for(i=0; i<n; i++)
        sum += h_c[i];
    printf("final result: %f\n", sum/(double)n);

    // release OpenCL resources
    clReleaseMemObject(d_a);
    clReleaseMemObject(d_b);
    clReleaseMemObject(d_c);
    clReleaseProgram(program);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
 
    //release host memory
    free(h_a);
    free(h_b);
    free(h_c);
 
    return 0;
}