#include <stdio.h>
#include <cl.h>

int main(){

    size_t dataBytes;
    cl_device_id *device = (cl_device_id *)malloc(dataBytes);

    init(device);
    get_device_info(device);
    return 0;
}