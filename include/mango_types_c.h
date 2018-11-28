/*! \file
 *  \brief C API types
 */
#ifndef MANGO_TYPES_C
#define MANGO_TYPES_C

typedef enum {
	SUCCESS = 0,         /*!< Successfull return */
	ERR_INVALID_VALUE,   /*!< Invalid value specified */
	ERR_INVALID_TASK_ID, /*!< Invalid task id provided */
	ERR_INVALID_KERNEL,  /*!< Invalid kernel structure */
	ERR_FEATURE_NOT_IMPLEMENTED, /*!< The feature is not implemented yet */
	ERR_INVALID_KERNEL_FILE, /*!< Kernel file is of an unknown or invalid type */
	ERR_UNSUPPORTED_UNIT, /*!< Unit type not supported in current configuration */
	ERR_OUT_OF_MEMORY,   /*! < Out of memory error */
	ERR_SEM_FAILED,      /*!< Semaphore open failure */
	ERR_MMAP_FAILED,     /*!< mmap failure */
	ERR_FOPEN,           /*!< file open failure */
	ERR_OTHER            /*!< Generic error */
} mango_exit_t;

typedef enum { LOCK, READ, WRITE, END_FIFO_OPERATION } mango_event_status_t;

/*! \brief Types of kernel sources/file types
 */
typedef enum { 
	UNKNOWN_KERNEL_SOURCE_TYPE, /*!< Unknown type, fail. */
	BINARY, /*!< Binary file, ready to load and execute */
	STRING, /*!< String containing the kernel source code */
	SOURCE  /*!< Text file containing the kernel source code */
} filetype;

/*! \brief Memory types
 *
 * Employed to configure memory buffer and scalar types. 
 */
typedef enum { 
	NONE, /*!< No type (typically, will cause errors) */
	FIFO, /*!< FIFO behaviour (a single asynchronous read or write from the GN
	* side will result in reading or writing the entire buffer with appropriate
	* synchronization */
	BUFFER, /*!< Buffer behaviour (each read or write is independent) */
	SCALAR, /*!< Scalar data type */
	EVENT /*!< Event parameter type */
} mango_buffer_type_t;

/*!
 * \enum mango_unit_type_t
 * \brief Mango computational unit (device) types
 */
typedef enum {
        PEAK, /*!< PEAK units */
        NUP,  /*!< NU+ units */
        DCT,
        GN,   /*!< Fall back to GN node if no other option is available */
        GPGPU,  /*!< GPU units */
        ARM,   /*!< ARM units */
        STOP, /*!< Terminator used to close arrays of mango_unit_type_t */
} mango_unit_type_t;


/*! \brief Data transfer modes
 *
 * Employed by the GN-HN data transfer functions to set up the desired type of
 * transfer. Can be expanded for supporting other modes.
 */
typedef enum { 
	DIRECT, /*!< Direct transfer mode */
	BURST /*!< Burst mode */
} mango_communication_mode_t;

#endif /* MANGO_TYPES_C */
