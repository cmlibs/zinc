	
# Defines GENERAL_SRCS

SET( GENERAL_SRCS
	source/general/any_object.c
	source/general/callback.c
	source/general/child_process.c
	source/general/compare.c
	source/general/debug.c
	source/general/error_handler.c
	source/general/geometry.cpp
	source/general/heapsort.c
	source/general/image_utilities.c
	source/general/indexed_multi_range.c
	source/general/integration.c
	source/general/io_stream.c
	source/general/machine.c
	source/general/matrix_vector.c
	source/general/multi_range.c
	source/general/myio.c
	source/general/mystring.c
	source/general/octree.c
	source/general/statistics.c
	source/general/time.cpp
	source/general/value.c )
SET( GENERAL_HDRS
	source/general/any_object.h
	source/general/any_object_definition.h
	source/general/any_object_private.h
	source/general/any_object_prototype.h
	source/general/block_array.hpp
	source/general/callback.h
	source/general/callback_class.hpp
	source/general/callback_motif.h
	source/general/callback_private.h
	source/general/change_log.h
	source/general/change_log_private.h
	source/general/child_process.h
	source/general/compare.h
	source/general/debug.h
	source/general/enumerator.h
	source/general/enumerator_private.h
	source/general/enumerator_private_cpp.hpp
	source/general/error_handler.h
	source/general/geometry.h
	source/general/heapsort.h
	source/general/image_utilities.h
	source/general/indexed_list_private.h
	source/general/indexed_multi_range.h
	source/general/integration.h
	source/general/io_stream.h
	source/general/list.h
	source/general/list_object_with_list_member_private.h
	source/general/list_private.h
	source/general/machine.h
	source/general/manager.h
	source/general/manager_private.h
	source/general/math.h
	source/general/matrix_vector.h
	source/general/multi_range.h
	source/general/myio.h
	source/general/mystring.h
	source/general/object.h
	source/general/octree.h
	source/general/postscript.h
	source/general/random.h
	source/general/simple_list.h
	source/general/statistics.h
	source/general/time.h
	source/general/value.h )
IF( NOT HAVE_VFSCANF )
	SET( GENERAL_SRCS ${GENERAL_SRCS}
		source/general/alt_vfscanf.c )
	SET( GENERAL_HDRS ${GENERAL_HDRS}
		source/general/alt_vfscanf.h )
ENDIF( NOT HAVE_VFSCANF )
IF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )
	SET( GENERAL_SRCS ${GENERAL_SRCS} source/general/photogrammetry.c )
	SET( GENERAL_HDRS ${GENERAL_HDRS} source/general/photogrammetry.h )
ENDIF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )
	
