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

#include "cl.h"

////////////////////////////////////////////////////////////////////////////////
#define WA 8
#define HA 8
#define WB 8

#define HB WA
#define WC WB
#define HC HA
////////////////////////////////////////////////////////////////////////////////

// Allocates a matrix with random float entries.
void randomMemInit(int *data, int size)
{
    int i;

    for (i = 0; i < size; ++i)
        data[i] = rand() % 100;//(int)RAND_MAX;
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

void kernel_function(int *A, int *B, int *D, int rows, int cols)
{
    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            int v = 0;
            for (int p = 0; p < rows; p++)
            {
                v = v + A[r * cols + p] * B[p * cols + c];
            }
            D[r * cols + c] = v;
        }
    }
    return;
}

// cl_program CreateProgramFromBinary(cl_context context, cl_device_id device, const char *fileName)
// {
//     FILE *fp = fopen(fileName, "rb");
//     if (fp == NULL)
//     {
//         return NULL;
//     }

//     // Determine the size of the binary
//     size_t binarySize;
//     fseek(fp, 0, SEEK_END);
//     binarySize = ftell(fp);
//     rewind(fp);

//     unsigned char *programBinary = malloc(binarySize);
//     fread(programBinary, 1, binarySize, fp);
//     fclose(fp);

//     cl_int errNum = 0;
//     cl_program program;
//     cl_int binaryStatus;

//     program = clCreateProgramWithBinary(context,
//                                         1,
//                                         &device,
//                                         &binarySize,
//                                         (const unsigned char **)&programBinary,
//                                         &binaryStatus,
//                                         &errNum);
//     free(programBinary);
//     if (errNum != CL_SUCCESS)
//     {
//         printf("Error loading program binary.");
//         return NULL;
//     }

//     if (binaryStatus != CL_SUCCESS)
//     {
//         printf("Invalid binary for device");
//         return NULL;
//     }

//     errNum = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
//     if (errNum != CL_SUCCESS)
//     {
//         // Determine the reason for the error
//         char buildLog[16384];
//         clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
//                               sizeof(buildLog), buildLog, NULL);

//         printf("Error in program: \n");
//         clReleaseProgram(program);
//         return NULL;
//     }

//     return program;
// }

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

    /* matrix used to verify the result */
    int *h_D = (int *)malloc(mem_size_C);

    /* Printing initialized matrices */
    // printf("\n\nMatrix A\n");
    // for (int i = 0; i < size_A; i++)
    // {
    //     printf("%f ", h_A[i]);
    //     if (((i + 1) % WC) == 0)
    //         printf("\n");
    // }
    // printf("\n");

    // printf("\n\nMatrix B\n");
    // for (int i = 0; i < size_B; i++)
    // {
    //     printf("%f ", h_B[i]);
    //     if (((i + 1) % WC) == 0)
    //         printf("\n");
    // }
    // printf("\n");

    printf("Initializing OpenCL device...\n");

    cl_uint plt_cnt = 0;
    clGetPlatformIDs(0, 0, &plt_cnt);

    cl_platform_id platform_ids[plt_cnt];
    clGetPlatformIDs(plt_cnt, platform_ids, NULL);

    // Connect to a compute device
    cl_uint dev_cnt = 0;
    clGetDeviceIDs(0, CL_DEVICE_TYPE_CPU, 0, 0, &dev_cnt);
    printf("Found %d devices for current platform\n", dev_cnt);

    cl_device_id device_ids[dev_cnt];
    err = clGetDeviceIDs(platform_ids[0], CL_DEVICE_TYPE_CPU, 2, device_ids, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to create a device group!\n");
        return EXIT_FAILURE;
    }

    // Create a compute context
    const char *mango_receipe = "test_manga";
    context = clCreateContext(NULL, 2, device_ids, NULL, mango_receipe, &err);
    if (!context)
    {
        printf("Error: Failed to create a compute context!\n");
        return EXIT_FAILURE;
    }

    // Create a command commands
    commands = clCreateCommandQueue(context, device_ids[0], NULL, &err);
    if (!commands)
    {
        printf("Error: Failed to create a command queue!\n");
        return EXIT_FAILURE;
    }

    cl_int errNum = 0;
    cl_int binaryStatus[dev_cnt];
    const char *programBinaryPaths[dev_cnt];
    programBinaryPaths[0] = "/opt/mango/usr/local/share/matrix_multiplication/matrix_multiplication_dev";
    programBinaryPaths[1] = "/opt/mango/usr/local/share/matrix_multiplication/matrix_multiplication_dev"; // potentially a second binary

    program = clCreateProgramWithBinary(context,
                                        1,
                                        device_ids,
                                        NULL, /* not needed */
                                        (const char **)programBinaryPaths,
                                        binaryStatus,
                                        &errNum);
    if (errNum != CL_SUCCESS)
    {
        printf("Error loading program binary.");
        return EXIT_FAILURE;
    }
    // for (int i = 0; i < dev_cnt; i++)
    // {
    //     if (binaryStatus[i] != CL_SUCCESS)
    //     {
    //         printf("Error with binary status %d\n", binaryStatus[i]);
    //         printf("Invalid binary for device at position %d\n", i);
    //         return EXIT_FAILURE;
    //     }
    // }
    if (!program)
    {
        printf("Error: Failed to create compute program!\n");
        return EXIT_FAILURE;
    }

    // Set the buffer IDS for the kernel we wish to run first
    clSetInputBufferIDs(program, programBinaryPaths[0], 2, 1, 2);
    clSetOutputBufferIDs(program, programBinaryPaths[0], 1, 3);
    // clSetInputBufferIDs(program, 1, 2, 4, 5);
    // clSetOutputBufferIDs(program, 1, 1, 6);

    // TODO : redefine first kernel structure inside cl_program and change the following functions to take kernels[0]
    cl_kernel kernels[dev_cnt];

    // err = clCreateKernelsInProgram(program, 1, kernels, NULL);
    // if (!kernels || err != CL_SUCCESS)
    // {
    //     printf("Error: Failed to create compute kernel!\n");
    //     exit(1);
    // }

    kernels[0] = clCreateKernel(program, programBinaryPaths[0], NULL, 1);

    // Create the input and output arrays in device memory for our calculation
    d_A = clCreateBuffer(context,
                         CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                         mem_size_A, h_A, &err,
                         0, NULL,
                         1, kernels,
                         1);
    d_B = clCreateBuffer(context,
                         CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                         mem_size_B, h_B, &err,
                         0, NULL,
                         1, kernels,
                         2);

    d_C = clCreateBuffer(context,
                         CL_MEM_WRITE_ONLY,
                         mem_size_A, NULL, &err, 
                         1, kernels,
                         0, NULL,
                         3);

    if (!d_A || !d_B || !d_C)
    {
        printf("Error: Failed to allocate device memory!\n");
        exit(1);
    }

    printf("Running matrix multiplication for matrices A (%dx%d) and B (%dx%d) ...\n", WA, HA, WB, HB);
    //Launch OpenCL kernel
    size_t localWorkSize[2], globalWorkSize[2];

    int wA = WA;
    int wC = WC;

    err = clSetKernelArg(kernels[0], 0, sizeof(cl_mem), (void *)&d_A, CL_BUFFER_ARG);
    err |= clSetKernelArg(kernels[0], 1, sizeof(cl_mem), (void *)&d_B, CL_BUFFER_ARG);
    err |= clSetKernelArg(kernels[0], 2, sizeof(cl_mem), (void *)&d_C, CL_BUFFER_ARG);
    err |= clSetKernelArg(kernels[0], 3, sizeof(int), (void *)&wA, CL_SCALAR_ARG);
    err |= clSetKernelArg(kernels[0], 4, sizeof(int), (void *)&wC, CL_SCALAR_ARG);
    cl_event bufferEvent;
    err |= clSetKernelArg(kernels[0], 2, sizeof(cl_event), (void *)&bufferEvent, CL_EVENT_ARG);

    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to set kernel arguments! %d\n", err);
        exit(1);
    }

    // User needs to explicitly allocate resources for the task graph
    clKernelAndBufferAllocation(commands);

    // Writing buffers to device memory
    cl_event writeEvent;
    // printf("Passo evento di write buffer con indirizzo %p\n", &writeEvent);
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

    // Adding an event to synchronization purpuses
    cl_event kernelEvent;
    err = clEnqueueNDRangeKernel(commands, kernels[0], 2, NULL, globalWorkSize, localWorkSize, 0, NULL, &kernelEvent);

    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to execute kernel! %d\n", err);
        exit(1);
    }

    //Retrieve result from device
    cl_event readEvent;
    err = clEnqueueReadBuffer(commands, d_C, CL_TRUE, 0, mem_size_C, h_C, 1, &bufferEvent, &readEvent);
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

    printf("Matrix multiplication completed...\n");

    //Check the result correctness
    kernel_function(h_A, h_B, h_D, WC, HC);

    int out = 0;
    for (int i = 0; i < WC; i++)
        for (int j = 0; j < HC; j++)
            if (h_D[i * HC + j] != h_C[i * HC + j])
            {
                printf("Incorrect value at %d, %d: %d vs %d\n", i, j, h_D[i * HC + j], h_C[i * HC + j]);
                out++;
            }

    if (out)
    {
        printf("Detected %d errors in the computation\n", out);
        exit(out);
    }
    else
    {
        printf("Matrix multiplication correctly performed\n");
        /* Printing result matrix */
        // printf("\n\nMatrix C\n");
        // for (int i = 0; i < size_A; i++)
        // {
        //     printf("%d ", h_D[i]);
        //     if (((i + 1) % WC) == 0)
        //         printf("\n");
        // }
        // printf("\n");
    }

    //Shutdown and cleanup
    free(h_A);
    free(h_B);
    free(h_C);
    free(h_D);

    // doesn't do anything
    clReleaseProgram(program);
    // deregister memory
    clReleaseMemObject(d_A);
    clReleaseMemObject(d_B);
    clReleaseMemObject(d_C);

    // deregister kernel
    clReleaseKernel(kernels[0]);
    // mango_resource_deallocation(tg); mango_task_graph_destroy_all(tg);
    clReleaseCommandQueue(commands);
    // mango_release()
    clReleaseContext(context);

    return 0;
}
