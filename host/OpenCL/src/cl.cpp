#include <stdlib.h>
#include <stdio.h>
#include "cl.h"

#ifdef __cplusplus
 extern "C" {
#endif  
	
    // struct for platform_id, just used to verify that it will work
	struct _cl_platform_id
	{
		//struct _cl_icd_dispatch const *const icd_dispatch;
		//cl_bool initialized;
		struct _cl_device_id *devices;
	};


    // same for device, also the device will contain an info that will be shown
	struct _cl_device_id
	{	
	char  *ciao;
	};

    // so we have a device
	struct _cl_device_id device0;


    // init connects the devices with the user's variable
	void init(cl_device_id * device){
			device[0] = &device0;
			device0.ciao = "AABB";
            
            // a print to show that users data can be shown
			printf("%s\n", ((device[0])->ciao) );

	}


 	void get_device_info(cl_device_id * device)
    {
            // after init we call this function and we should be able to see the info about the device anyway since we already got a reference to it
			printf("%s\n", ((device[0])->ciao) );


    }
#ifdef __cplusplus
 }
#endif     






