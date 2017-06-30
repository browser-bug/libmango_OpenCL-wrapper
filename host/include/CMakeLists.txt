set (HEADER_FILES
	buffer.h
	context.h
	event.h
	kernel.h
	kernel_arguments.h
	logger.h
	mango.h
	mango_types.h
	mm.h
	task_graph.h
	tlb.h
	)


install (FILES ${HEADER_FILES} DESTINATION ${MANGO_ROOT}/include/libmango/host)

