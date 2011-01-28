
# Defines CORE_FIELDS_LIB_SRCS

SET( CORE_FIELDS_LIB_SRCS
	source/api/cmiss_element.cpp
	source/api/cmiss_node.cpp
	source/api/cmiss_time_sequence.c
	source/general/io_stream.c
	source/general/statistics.c
	${FINITE_ELEMENT_CORE_SRCS}
	${COMPUTED_FIELD_CORE_SRCS}
	${REGION_SRCS} )

SET( CORE_FIELDS_LIB_HDRS
	source/api/cmiss_element.h
	source/api/cmiss_node.h
	source/api/cmiss_time_sequence.h
	source/general/io_stream.h
	source/general/statistics.h
	${FINITE_ELEMENT_CORE_HDRS}
	${COMPUTED_FIELD_CORE_HDRS}
	${REGION_HDRS} )
