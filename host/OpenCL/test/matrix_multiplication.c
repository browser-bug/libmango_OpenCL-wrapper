////////////////////////////////////////////////////////////////////////////////
// Sample code taken from https://gitlab.com/ecatue/gpu_matrixmul_opencl.git
////////////////////////////////////////////////////////////////////////////////

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <stdbool.h>

#include "CL/cl.h"

////////////////////////////////////////////////////////////////////////////////
#define WA 1024
#define HA 1024
#define WB 1024

#define HB WA
#define WC WB
#define HC HA
////////////////////////////////////////////////////////////////////////////////

// Allocates a matrix with random float entries.
void randomMemInit(int *data, int size)
{
    int i;

    for (i = 0; i < size; ++i)
        data[i] = rand() % 100; //(int)RAND_MAX;
}

long LoadOpenCLKernel(char const *path, char **buf)
{
    FILE *fp;
    size_t fsz;
    long off_end;
    int rc;

    /* Open the file */
    fp = fopen(path, "r");
    if (NULL == fp)
    {
        return -1L;
    }

    /* Seek to the end of the file */
    rc = fseek(fp, 0L, SEEK_END);
    if (0 != rc)
    {
        return -1L;
    }

    /* Byte offset to the end of the file (size) */
    if (0 > (off_end = ftell(fp)))
    {
        return -1L;
    }
    fsz = (size_t)off_end;

    /* Allocate a buffer to hold the whole file */
    *buf = (char *)malloc(fsz + 1);
    if (NULL == *buf)
    {
        return -1L;
    }

    /* Rewind file pointer to start of file */
    rewind(fp);

    /* Slurp file into buffer */
    if (fsz != fread(*buf, 1, fsz, fp))
    {
        free(*buf);
        return -1L;
    }

    /* Close the file */
    if (EOF == fclose(fp))
    {
        free(*buf);
        return -1L;
    }

    /* Make sure the buffer is NUL-terminated, just in case */
    (*buf)[fsz] = '\0';

    /* Return the file size */
    return (long)fsz;
}


int main(int argc, char **argv)
{
    int err; // error code returned from api calls

    cl_device_id device_id;    // compute device id
    cl_context context;        // compute context
    cl_command_queue commands; // compute command queue
    cl_program program;        // compute program
    cl_kernel kernel;          // compute kernel

    // OpenCL device memory for matrices
    cl_mem d_A;
    cl_mem d_B;
    cl_mem d_C;

    // set seed for rand()
    srand(2014);

    //Allocate host memory for matrices A and B
    unsigned int size_A = WA * HA;
    unsigned int mem_size_A = sizeof(int) * size_A;
    int *h_A = (int *)malloc(mem_size_A);

    unsigned int size_B = WB * HB;
    unsigned int mem_size_B = sizeof(int) * size_B;
    int *h_B = (int *)malloc(mem_size_B);

    //Initialize host memory
    randomMemInit(h_A, size_A);
    randomMemInit(h_B, size_B);

    //Allocate host memory for the result C
    unsigned int size_C = WC * HC;
    unsigned int mem_size_C = sizeof(int) * size_C;
    int *h_C = (int *)malloc(mem_size_C);

    printf("Initializing OpenCL device...\n");


    cl_uint plt_cnt = 0;
    clGetPlatformIDs(0, 0, &plt_cnt);

    cl_platform_id platform_ids[plt_cnt];
    clGetPlatformIDs(plt_cnt, platform_ids, NULL);
 
    // Connect to a compute device

    err = clGetDeviceIDs(platform_ids[0], CL_DEVICE_TYPE_CPU, 1, &device_id, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to create a device group!\n");
        return EXIT_FAILURE;
    }

    // Create a compute context
    const char *mango_recipe = "test_manga";
    context = clCreateContext(NULL, 1, &device_id, NULL, mango_recipe, &err);
    if (!context)
    {
        printf("Error: Failed to create a compute context!\n");
        return EXIT_FAILURE;
    }

    // Create a command commands
    commands = clCreateCommandQueue(context, device_id, NULL, &err);
    if (!commands)
    {
        printf("Error: Failed to create a command queue!\n", err);
        return EXIT_FAILURE;
    }


    cl_int errNum = 0;
    cl_int binaryStatus[1];
    const char *programBinaryPaths[1];
    programBinaryPaths[0] = "/opt/mango/usr/local/share/matrix_multiplication/matrix_multiplication_dev";

    program = clCreateProgramWithBinary(context,
                                        1,
                                        &device_id,
                                        NULL, /* not needed */
                                        (const char **)programBinaryPaths,
                                        binaryStatus,
                                        &errNum);
    if (errNum != CL_SUCCESS)
    {
        printf("Error loading program binary.");
        return EXIT_FAILURE;
    }

    if (!program)
    {
        printf("Error: Failed to create compute program!\n");
        return EXIT_FAILURE;
    }

    // Set the buffer IDS for the kernel we wish to run first
    clSetInputBufferIDs(program, programBinaryPaths[0], 2, 1, 2);
    clSetOutputBufferIDs(program, programBinaryPaths[0], 1, 3);

    // Create the compute kernel in the program we wish to run
    kernel = clCreateKernel(program, programBinaryPaths[0], NULL, 1);

     if (!kernel || err != CL_SUCCESS)
     {
         printf("Error: Failed to create compute kernel!\n");
         return EXIT_FAILURE;
     }


    // Create the input and output arrays in device memory for our calculation
    d_A = clCreateBuffer(context,
                         CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                         mem_size_A, h_A, &err,
                         0, NULL,
                         1, &kernel,
                         1);
    d_B = clCreateBuffer(context,
                         CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                         mem_size_B, h_B, &err,
                         0, NULL,
                         1, &kernel,
                         2);

    d_C = clCreateBuffer(context,
                         CL_MEM_WRITE_ONLY,
                         mem_size_A, NULL, &err,
                         1, &kernel,
                         0, NULL,
                         3);

    if (!d_A || !d_B || !d_C)
    {
        printf("Error: Failed to allocate device memory!\n");
        return EXIT_FAILURE;
    }

    printf("Running matrix multiplication for matrices A (%dx%d) and B (%dx%d) ...\n", WA, HA, WB, HB);
    //Launch OpenCL kernel
    size_t localWorkSize[2], globalWorkSize[2];

    int wA = WA;
    int wC = WC;

    err = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&d_A, CL_BUFFER_ARG);
    err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&d_B, CL_BUFFER_ARG);
    err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&d_C, CL_BUFFER_ARG);
    err |= clSetKernelArg(kernel, 3, sizeof(int), (void *)&wA, CL_SCALAR_ARG);
    err |= clSetKernelArg(kernel, 4, sizeof(int), (void *)&wC, CL_SCALAR_ARG);
    cl_event bufferEvent; // necessary event for reading buffer with ID 2
    err |= clSetKernelArg(kernel, 2, sizeof(cl_event), (void *)&bufferEvent, CL_EVENT_ARG);

    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to set kernel arguments! %d\n", err);
        return EXIT_FAILURE;
    }

    // User needs to explicitly allocate resources for the task graph
    clKernelAndBufferAllocation(commands);


    // Writing buffers to device memory 
    cl_event writeEvent;
    
    err = clEnqueueWriteBuffer(commands, d_A, NULL, NULL, NULL, h_A, NULL, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to write buffer! %d\n", err);
        exit(1);
    }
    err = clEnqueueWriteBuffer(commands, d_B, NULL, NULL, NULL, h_B, NULL, NULL, NULL); // FIX : waiting on writeEvent gets stuck
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to write buffer! %d\n", err);
        exit(1);
    }

    localWorkSize[0] = 16;
    localWorkSize[1] = 16;
    globalWorkSize[0] = 1024;
    globalWorkSize[1] = 1024;

    // Adding an event to synchronization purpuses (necessary in MANGO)
    cl_event kernelEvent;
    err = clEnqueueNDRangeKernel(commands, kernel, 2, NULL, globalWorkSize, localWorkSize, 0, NULL, &kernelEvent);

    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to execute kernel! %d\n", err);
        exit(1);
    }

    // In MANGO, when we want to read, it is necessary to wait for the bufferEvent
    err = clEnqueueReadBuffer(commands, d_C, CL_TRUE, 0, mem_size_C, h_C, 1, &bufferEvent, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to read output array! %d\n", err);
        exit(1);
    }

    err = clWaitForEvents(1, &kernelEvent);

    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to wait for events! %d\n", err);
        exit(1);
    }


    //Shutdown and cleanup
    free(h_A);
    free(h_B);
    free(h_C);
    
    // deregister memory
    clReleaseMemObject(d_A);
    clReleaseMemObject(d_B);
    clReleaseMemObject(d_C);


    clReleaseProgram(program);
    // deregister kernel
    clReleaseKernel(kernel);
    // mango_resource_deallocation(tg); mango_task_graph_destroy_all(tg);
    clReleaseCommandQueue(commands);
    // mango_release()
    clReleaseContext(context);

    return 0;
}
