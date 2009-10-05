
# Defines CORE_FIELDS_LIB_SRCS

SET( CORE_FIELDS_LIB_SRCS
	source/api/cmiss_element.c
	source/api/cmiss_node.c
	source/api/cmiss_time_sequence.c
	source/general/io_stream.c
  source/general/statistics.c
	${FINITE_ELEMENT_CORE_SRCS}
	${COMPUTED_FIELD_CORE_SRCS}
	${REGION_SRCS} )


