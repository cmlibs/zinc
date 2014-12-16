
# Defines GENERAL_SRCS

# OpenCMISS-Zinc Library
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

SET( GENERAL_SRCS
	source/general/any_object.cpp
	source/general/callback.cpp
	source/general/child_process.cpp
	source/general/compare.cpp
	source/general/debug.cpp
	source/general/error_handler.cpp
	source/general/geometry.cpp
	source/general/image_utilities.cpp
	source/general/indexed_multi_range.cpp
	source/general/integration.cpp
	source/general/io_stream.cpp
	source/general/machine.cpp
	source/general/matrix_vector.cpp
	source/general/message.cpp
	source/general/multi_range.cpp
	source/general/myio.cpp
	source/general/mystring.cpp
	source/general/octree.cpp
	source/general/statistics.cpp
	source/general/time.cpp
	source/general/value.cpp
	source/jsoncpp/jsoncpp.cpp
	source/stream/stream_private.cpp )
SET( GENERAL_HDRS
	source/general/any_object.h
	source/general/any_object_definition.h
	source/general/any_object_private.h
	source/general/any_object_prototype.h
	source/general/block_array.hpp
	source/general/callback.h
	source/general/callback_class.hpp
	source/general/callback_private.h
	source/general/change_log.h
	source/general/change_log_private.h
	source/general/child_process.h
	source/general/cmiss_set.hpp
	source/general/compare.h
	source/general/debug.h
	source/general/enumerator.h
	source/general/enumerator_conversion.hpp
	source/general/enumerator_private.h
	source/general/enumerator_private.hpp
	source/general/error_handler.h
	source/general/geometry.h
	source/general/image_utilities.h
	source/general/indexed_list_private.h
	source/general/indexed_list_stl_private.hpp
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
	source/general/message.h
	source/general/multi_range.h
	source/general/myio.h
	source/general/mystring.h
	source/general/object.h
	source/general/octree.h
	source/general/random.h
	source/general/refcounted.hpp
	source/general/refhandle.hpp
	source/general/simple_list.h
	source/general/statistics.h
	source/general/time.h
	source/general/value.h
	source/jsoncpp/json.h
	source/jsoncpp/json-forwards.h
	source/stream/stream_private.hpp )
IF( NOT HAVE_VFSCANF )
	SET( GENERAL_SRCS ${GENERAL_SRCS}
		source/general/alt_vfscanf.c )
	SET( GENERAL_HDRS ${GENERAL_HDRS}
		source/general/alt_vfscanf.h )
ENDIF( NOT HAVE_VFSCANF )
IF( NOT HAVE_HEAPSORT )
	SET( GENERAL_SRCS ${GENERAL_SRCS}
		source/general/heapsort.cpp )
	SET( GENERAL_HDRS ${GENERAL_HDRS}
		source/general/heapsort.h )
ENDIF( NOT HAVE_HEAPSORT )
IF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )
	SET( GENERAL_SRCS ${GENERAL_SRCS} source/general/photogrammetry.cpp )
	SET( GENERAL_HDRS ${GENERAL_HDRS} source/general/photogrammetry.h )
ENDIF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )

