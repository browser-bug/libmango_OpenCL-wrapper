// Multiply two matrices A * B = C

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "host/OpenCL/include/cl.h"

// Allocates a matrix with random float entries.
void randomInit(float *data, int size)
{
    for (int i = 0; i < size; ++i)
        data[i] = rand() / (float)RAND_MAX;
}

/////////////////////////////////////////////////////////
// Program main
/////////////////////////////////////////////////////////

int main(int argc, char **argv)
{

    // set seed for rand()
    srand(2006);

    int rows, columns;

    if (argc < 3)
    {
        printf("error, bad arguments\n");
        printf("Arguments: <rows> <columns>\n");
        exit(1);
    }

    rows = atoi(argv[1]);
    columns = atoi(argv[2]);

    if ((rows == 0) || (columns == 0))
    {
        printf("error, rows and columns should be not zero\n");
        printf("rows = %d; columns = %d\n", rows, columns);
        exit(1);
    }

    // 1. allocate host memory for matrices A and B
    unsigned int size_A = rows * columns;
    unsigned int mem_size_A = sizeof(float) * size_A;
    float *h_A = (float *)malloc(mem_size_A);

    unsigned int size_B = rows * columns;
    unsigned int mem_size_B = sizeof(float) * size_B;
    float *h_B = (float *)malloc(mem_size_B);

    // 2. initialize host memory
    randomInit(h_A, size_A);
    randomInit(h_B, size_B);

    // 4. allocate host memory for the result C
    unsigned int size_C = rows * columns;
    unsigned int mem_size_C = sizeof(float) * size_C;
    float *h_C = (float *)malloc(mem_size_C);

    // 5. Initialize OpenCL
    // OpenCL specific variables
    cl_context clGPUContext;
    cl_command_queue clCommandQue;
    cl_program clProgram;
    cl_kernel clKernel;

    size_t dataBytes;
    size_t kernelLength;
    cl_int errcode;

    // OpenCL device memory for matrices
    cl_mem d_A;
    cl_mem d_B;
    cl_mem d_C;

    /*****************************************/
    /* Initialize OpenCL */
    /*****************************************/
    clGPUContext = clCreateContextFromType(0,
                                           CL_DEVICE_TYPE_GPU,
                                           NULL, NULL, &errcode);

    // get the list of GPU devices associated
    // with context
    errcode = clGetContextInfo(clGPUContext,
                               CL_CONTEXT_DEVICES, 0, NULL,
                               &dataBytes);
    cl_device_id *clDevices = (cl_device_id *)malloc(dataBytes);
    errcode |= clGetContextInfo(clGPUContext,
                                CL_CONTEXT_DEVICES, dataBytes,
                                clDevices, NULL);

    //Create a command-queue
    clCommandQue = clCreateCommandQueue(clGPUContext,
                                        clDevices[0], 0, &errcode);

    // Setup device memory
    d_C = clCreateBuffer(clGPUContext,
                         CL_MEM_READ_WRITE,
                         mem_size_A, NULL, &errcode);
    d_A = clCreateBuffer(clGPUContext,
                         CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                         mem_size_A, h_A, &errcode);
    d_B = clCreateBuffer(clGPUContext,
                         CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                         mem_size_B, h_B, &errcode);

    // 6. Load and build OpenCL kernel

    FILE *fp;
    char *clMatrixMul;

    fp = fopen("/kernel/kernel.cl", "rb"); // check relative path
    if (!fp)
    {
        printf("Failed to load kernel\n");
        return 1;
    }

    fseek(fp, 0, SEEK_END);
    kernelLength = ftell(fp);
    rewind(fp);
    clMatrixMul = (char *)malloc(kernelLength + 1);
    clMatrixMul[kernelLength] = '\0';
    fread(clMatrixMul, sizeof(char), kernelLength, fp);
    fclose(fp);

/*     char *clMatrixMul = oclLoadProgSource("kernel.cl",
                                          "// My comment\n",
                                          &kernelLength); */

    clProgram = clCreateProgramWithSource(clGPUContext,
                                          1, (const char **)&clMatrixMul,
                                          &kernelLength, &errcode);

    errcode = clBuildProgram(clProgram, 0,
                             NULL, NULL, NULL, NULL);

    clKernel = clCreateKernel(clProgram,
                              "matrixMul", &errcode);

    // 7. Launch OpenCL kernel
    size_t localWorkSize[2], globalWorkSize[2];

    int wA = columns;
    int wC = columns;
    errcode = clSetKernelArg(clKernel, 0,
                             sizeof(cl_mem), (void *)&d_C);
    errcode |= clSetKernelArg(clKernel, 1,
                              sizeof(cl_mem), (void *)&d_A);
    errcode |= clSetKernelArg(clKernel, 2,
                              sizeof(cl_mem), (void *)&d_B);
    errcode |= clSetKernelArg(clKernel, 3,
                              sizeof(int), (void *)&wA);
    errcode |= clSetKernelArg(clKernel, 4,
                              sizeof(int), (void *)&wC);

    localWorkSize[0] = 3;
    localWorkSize[1] = 3;
    globalWorkSize[0] = 3;
    globalWorkSize[1] = 3;

    errcode = clEnqueueNDRangeKernel(clCommandQue,
                                     clKernel, 2, NULL, globalWorkSize,
                                     localWorkSize, 0, NULL, NULL);

    // 8. Retrieve result from device
    errcode = clEnqueueReadBuffer(clCommandQue,
                                  d_C, CL_TRUE, 0, mem_size_C,
                                  h_C, 0, NULL, NULL);

    // 9. print out the results
    printf("\n\nMatrix C (Results)\n");
    for (int i = 0; i < size_C; i++)
    {
        printf("%f ", h_C[i]);
        if (((i + 1) % columns) == 0)
            printf("\n");
    }
    printf("\n");

    // 10. clean up memory
    free(h_A);
    free(h_B);
    free(h_C);

    clReleaseMemObject(d_A);
    clReleaseMemObject(d_C);
    clReleaseMemObject(d_B);

    free(clDevices);
    free(clMatrixMul);
    clReleaseContext(clGPUContext);
    clReleaseKernel(clKernel);
    clReleaseProgram(clProgram);
    clReleaseCommandQueue(clCommandQue);
}