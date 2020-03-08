#include "clMem.h"

#include "clKernel.h"
#include "clContext.h"
#include "clCommandQueue.h"
#include "clExceptions.h"

// support function for clCreateBuffer
std::vector<mango_kernel_t> extractKernelIDs(cl_kernel *kernels, cl_int num_kernels)
{
    std::vector<mango_kernel_t> tempVect;
    for (int i = 0; i < num_kernels; i++)
        tempVect.push_back(kernels[i]->kernel);
    return tempVect;
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
    /* we can accept only these flags for now */
    cl_mem_flags valid_flags = CL_MEM_READ_WRITE | CL_MEM_WRITE_ONLY | CL_MEM_READ_ONLY;
    std::vector<mango_kernel_t> _kernels_in;
    std::vector<mango_kernel_t> _kernels_out;
    mango_buffer_t buf_t;

    if (context == NULL)
        throw cl_error(CL_INVALID_CONTEXT);

    if ((flags & valid_flags) == 0)
    {
        std::cout << "[clCreateBuffer] please specify one of the available flags: CL_MEM_READ_WRITE | CL_MEM_WRITE_ONLY | CL_MEM_READ_ONLY\n";
        throw cl_error(CL_INVALID_VALUE);
    }

    if (size == 0)
        throw cl_error(CL_INVALID_BUFFER_SIZE);

    // TODO when we'll have the possibility to get device's infos we'll do this
    // if(size > getDeviceMaxAllocSize(context->devices[0])){
    //   throw cl_error(CL_INVALID_BUFFER_SIZE);

    /* get the IDs for the mango_register_memory process */
    _kernels_in = extractKernelIDs(kernels_in, num_kernels_in);
    _kernels_out = extractKernelIDs(kernels_out, num_kernels_out);

    if ((flags & CL_MEM_READ_WRITE) == CL_MEM_READ_WRITE)
    {
        if (_kernels_in.empty() || _kernels_out.empty())
        {
            std::cout << "[clCreateBuffer] specify input and output kernels";
            throw cl_error(CL_INVALID_VALUE);
        }
        buf_t = mango_register_memory_with_kernels(buffer_id, size, BUFFER, &_kernels_in, &_kernels_out);
    }
    else if ((flags & CL_MEM_WRITE_ONLY) == CL_MEM_WRITE_ONLY)
    {
        if (_kernels_in.empty())
        {
            std::cout << "[clCreateBuffer] specify input kernels";
            throw cl_error(CL_INVALID_VALUE);
        }
        buf_t = mango_register_memory_with_kernels(buffer_id, size, BUFFER, &_kernels_in, NULL);
    }
    else if ((flags & CL_MEM_READ_ONLY) == CL_MEM_READ_ONLY)
    {
        if (_kernels_out.empty())
        {
            std::cout << "[clCreateBuffer] specify output kernels";
            throw cl_error(CL_INVALID_VALUE);
        }
        buf_t = mango_register_memory_with_kernels(buffer_id, size, BUFFER, NULL, &_kernels_out);
    }

    /* allocate a new mem object */
    memory = new _cl_mem(context);

    memory->id = buffer_id;
    memory->host_ptr = host_ptr;
    memory->flags = flags;
    memory->size = size;
    memory->type = CL_MEM_OBJECT_BUFFER;
    memory->buffer = buf_t;

    memory->ctx->queue->tgx = mango_task_graph_add_buffer(memory->ctx->queue->tgx, &(memory->buffer));
    // std::cout << "[TASK_GRAPH] added new buffer to tg (address) : " << memory->ctx->queue->tgx << std::endl;

    /* add the buffer to the context mem objects vector */
    context->mem_objects.push_back(memory);

    if (errcode_ret)
        *errcode_ret = CL_SUCCESS;
    return memory;
}

// TODO : to be tested
cl_int cl_get_mem_object_info(cl_mem memobj,
                              cl_mem_info param_name,
                              size_t param_value_size,
                              void *param_value,
                              size_t *param_value_size_ret)
{
    const void *src_ptr = NULL;
    size_t src_size = 0;
    size_t ptr;

    if (memobj == NULL)
        return CL_INVALID_MEM_OBJECT;

    switch (param_name)
    {
    case CL_MEM_TYPE:
    {
        src_ptr = &(memobj->type);
        src_size = sizeof(cl_mem_object_type);
        break;
    }
    case CL_MEM_FLAGS:
        src_ptr = &memobj->flags;
        src_size = sizeof(cl_mem_flags);
        break;
    case CL_MEM_SIZE:
        src_ptr = &memobj->size;
        src_size = sizeof(size_t);
        break;
    case CL_MEM_HOST_PTR:
    {
        ptr = 0;
        ptr = (size_t)memobj->host_ptr;
        src_ptr = &ptr;
        src_size = sizeof(size_t);
        break;
    }
    case CL_MEM_CONTEXT:
        src_ptr = &memobj->ctx;
        src_size = sizeof(cl_context);
        break;
    default:
        return CL_INVALID_VALUE;
    }

    if (param_value && param_value_size < src_size)
        return CL_INVALID_VALUE;
    if (param_value && param_value_size)
        memcpy(param_value, src_ptr, src_size);
    if (param_value_size_ret)
        *param_value_size_ret = src_size;
    return CL_SUCCESS;
}
