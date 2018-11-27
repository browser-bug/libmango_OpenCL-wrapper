set (HEADER_FILES
	buffer.h
	context.h
	event.h
	kernel.h
	kernel_arguments.h
	logger.h
	mango.h
	mango_types.h
	mango_types_c.h
	mm.h
	process.h
	task_graph.h
	tlb.h
)


if(CONFIG_LIBMANGO_PROFILING_MODE)
	set (HEADER_FILES ${HEADER_FILES} profiling.h)
	add_definitions(-DCONFIG_LIBMANGO_PROFILING_MODE)
endif(CONFIG_LIBMANGO_PROFILING_MODE)


install (FILES ${HEADER_FILES} DESTINATION ${MANGO_ROOT}/include/libmango/host)

