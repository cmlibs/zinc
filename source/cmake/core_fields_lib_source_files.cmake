
# Defines CORE_FIELDS_LIB_SRCS

SET( CORE_FIELDS_LIB_SRCS
	source/general/io_stream.cpp
	source/general/statistics.c
	source/mesh/cmiss_element_private.cpp
	source/mesh/cmiss_node_private.cpp
	source/stream/cmiss_stream_private.cpp
	${FINITE_ELEMENT_CORE_SRCS}
	${COMPUTED_FIELD_CORE_SRCS}
	${REGION_SRCS} )

SET( CORE_FIELDS_LIB_HDRS
	source/api/zn_element.h
	source/api/zn_node.h
	source/api/zn_time_sequence.h
	source/general/io_stream.h
	source/general/statistics.h
	source/mesh/cmiss_element_private.hpp
	source/mesh/cmiss_node_private.hpp
	source/stream/cmiss_stream_private.hpp
	${FINITE_ELEMENT_CORE_HDRS}
	${COMPUTED_FIELD_CORE_HDRS}
	${REGION_HDRS} )
