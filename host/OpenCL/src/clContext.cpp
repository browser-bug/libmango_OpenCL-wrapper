#include "clContext.h"

// support function for clCreateContext
const char *get_process_name_by_pid(const int pid)
{
    char *name = NULL;
    name = (char *)calloc(1024, sizeof(char));
    if (name)
    {
        sprintf(name, "/proc/%d/cmdline", pid);
        FILE *f = fopen(name, "r");
        if (f)
        {
            size_t size;
            size = fread(name, sizeof(char), 1024, f);
            if (size > 0)
            {
                if ('\n' == name[size - 1])
                    name[size - 1] = '\0';
            }
            name += 2; // removing first two characters (es. './')
            fclose(f);
        }
    }
    assert(name && "error in retrieving process_name, try again");
    return name;
}

cl_context cl_create_context(const cl_context_properties *properties,
                             cl_uint num_devices,
                             const cl_device_id *devices,
                             void(CL_CALLBACK *pfn_notify)(const char *, const void *, size_t, void *),
                             void *user_data,
                             cl_int *errcode_ret)
{
    assert(user_data && "user_data must specify a mango_receipe");

    const int pid = getpid();
    const char *process_name = get_process_name_by_pid(pid);
    const char *receipe = (char *)user_data;

    cl_context context = NULL;
    if (mango_init(process_name, receipe) == SUCCESS)
    {
        context = (cl_context)malloc(sizeof(struct _cl_context));
        context->devices = devices;
        context->device_num = num_devices;
        context->mem_objects = NULL;
        context->mem_object_num = 0;
        return context;
    }
    else
    {
        if (!errcode_ret)
            *errcode_ret = CL_INVALID_VALUE;
        return context;
    }
}