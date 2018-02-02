set (HEADER_FILES
	debug.h
	mango_hn.h
	mango_hn_peak.h
	mango_hn_nuplus.h
	mango_hn_types.h
	)


install (FILES ${HEADER_FILES} DESTINATION ${MANGO_ROOT}/include/libmango/dev)

