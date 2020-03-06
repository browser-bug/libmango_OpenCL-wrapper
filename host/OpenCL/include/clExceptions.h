#include "CL/cl.h"

/* A new exception struct for throws */
typedef struct _cl_error : public std::exception
{
    _cl_error(cl_int msg) : errcode(msg) {}

    cl_int errcode;

    const char *what() const throw()
    {
        return "OpenCL error, check errcode for more info.";
    }
} cl_error;