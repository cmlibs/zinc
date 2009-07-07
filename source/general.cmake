	
# Defines GENERAL_SRCS

SET( GENERAL_SRCS
	general/any_object.c
	general/callback.c
	general/child_process.c
	general/compare.c
	general/debug.c
	general/error_handler.c
	general/geometry.c
	general/heapsort.c
	general/image_utilities.c
	general/indexed_multi_range.c
	general/integration.c
	general/io_stream.c
	general/machine.c
	general/matrix_vector.c
	general/multi_range.c
	general/myio.c
	general/mystring.c
	general/octree.c
	general/statistics.c
	general/time.cpp
	general/value.c )

IF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )
	SET( GENERAL_SRCS ${GENERAL_SRCS}	general/photogrammetry.c )
ENDIF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )
	
