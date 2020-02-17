#include "clMem.h"

#include "clKernel.h"
#include "clContext.h"
#include "clCommandQueue.h"

// support function for clCreateBuffer
void extractKernelIDs(std::vector<mango_kernel_t> *_kernels, cl_kernel *kernels, cl_int num_kernels)
{
    _kernels->clear();
    for (int i = 0; i < num_kernels; i++)
        _kernels->push_back(kernels[i]->kernel);
}

cl_mem cl_create_buffer(cl_context context,
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
    cl_mem memory = NULL;
    memory = (cl_mem)malloc(sizeof(struct _cl_mem));

    memory->host_ptr = host_ptr;
    memory->ctx = context;
    memory->id = buffer_id;

    std::vector<mango_kernel_t> _kernels_in;
    std::vector<mango_kernel_t> _kernels_out;
    extractKernelIDs(&_kernels_in, kernels_in, num_kernels_in);
    extractKernelIDs(&_kernels_out, kernels_out, num_kernels_out);

    std::cout << "[clCreateBuffer] ignoring user flags" << std::endl;
    if ((flags & CL_MEM_READ_WRITE) == CL_MEM_READ_WRITE)
    {
        // TODO : to be implemented
        // assert(kernels_in && kernels_out && "specify input and output kernels");
        // memory->buffer = mango_register_memory(memory->id, size, BUFFER, 1, 1, context->program->kernel, context->program->kernel);
        // memory->type = CL_MEM_READ_WRITE;
    }
    else if ((flags & CL_MEM_WRITE_ONLY) == CL_MEM_WRITE_ONLY)
    {
        assert(kernels_in && "specify input kernels");

        memory->type = CL_MEM_WRITE_ONLY;
        memory->buffer = mango_register_memory_with_kernels(memory->id, size, BUFFER, &_kernels_in, NULL);
    }
    else if ((flags & CL_MEM_READ_ONLY) == CL_MEM_READ_ONLY)
    {
        assert(kernels_out && "specify output kernels");

        memory->type = CL_MEM_READ_ONLY;
        memory->buffer = mango_register_memory_with_kernels(memory->id, size, BUFFER, NULL, &_kernels_out);
    }
    else
    {
        if (!errcode_ret)
            *errcode_ret = CL_INVALID_VALUE;
        std::cout << "[clCreateBuffer] please specify CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY | CL_MEM_READ_WRITE" << std::endl;
        free(memory);
        memory = NULL;
        return memory;
    }

    memory->ctx->queue->tgx = mango_task_graph_add_buffer(memory->ctx->queue->tgx, &(memory->buffer));
    std::cout << "[TASK_GRAPH] added new buffer to tg (address) : " << memory->ctx->queue->tgx << std::endl;

    if (context->mem_objects != NULL)
        context->mem_objects = (cl_mem *)realloc(context->mem_objects, sizeof(cl_mem *) * (context->mem_object_num + 1)); // FIX : maybe reallocation can be smarter, *2 just when necessary instead of +1 everytime
    else                                                                                                                  // initialize mem objects
        context->mem_objects = (cl_mem *)calloc(1, sizeof(cl_mem));

    context->mem_objects[context->mem_object_num] = memory;
    context->mem_object_num++;

    return memory;
}