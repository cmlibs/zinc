
# OpenCMISS-Zinc Library
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Defines COMFILE_SRCS, CURVE_SRCS, ELEMENT_SRCS, EMOTER_SRCS, FINITE_ELEMENT_CORE_SRCS, FINITE_ELEMENT_GRAPHICS_SRCS,
# FINITE_ELEMENT_SRCS (definition includes the previous two), INTERACTION_SRCS, IO_DEVICES_SRCS, LICENSE_HDRS, NODE_SRCS,
# REGION_SRCS, SELECTION_SRCS, THREE_D_DRAWING_SRCS, TIME_SRCS

SET( CURVE_SRCS source/curve/curve.cpp )
SET( CURVE_HDRS source/curve/curve.h )

SET( DATASTORE_SRCS
	source/datastore/labels.cpp
	source/datastore/labelschangelog.cpp
	source/datastore/labelsgroup.cpp
	source/datastore/map.cpp
	source/datastore/mapindexing.cpp )
SET( DATASTORE_HDRS
	source/datastore/labels.hpp
	source/datastore/labelschangelog.hpp
	source/datastore/labelsgroup.hpp
	source/datastore/map.hpp
	source/datastore/maparray.hpp
	source/datastore/mapindexing.hpp )

SET( ELEMENT_SRCS
	source/element/element_operations.cpp )
SET( ELEMENT_HDRS
	source/element/element_operations.h )

SET( EMOTER_SRCS
	source/emoter/em_cmgui.cpp )
SET( EMOTER_HDRS
	source/emoter/em_cmgui.h )

SET( INTERACTION_SRCS
	source/interaction/interaction_graphics.cpp
	source/interaction/interaction_volume.cpp
	source/interaction/interactive_event.cpp )
SET( INTERACTION_HDRS
	source/interaction/interaction_graphics.h
	source/interaction/interaction_volume.h
	source/interaction/interactive_event.h )


SET( IO_DEVICES_SRCS
	source/io_devices/conversion.cpp
	source/io_devices/io_device.cpp )
SET( IO_DEVICES_HDRS
	source/io_devices/conversion.h
	source/io_devices/gst_scene.h
	source/io_devices/gst_transform.h
	source/io_devices/io_device.h )

SET( FIELD_IO_SRCS
	source/field_io/fieldml_common.cpp
	source/field_io/read_fieldml.cpp
	source/field_io/write_fieldml.cpp )
SET( FIELD_IO_HDRS
	source/field_io/fieldml_common.hpp
	source/field_io/read_fieldml.hpp
	source/field_io/write_fieldml.hpp )

SET( IMAGE_IO_SRCS
	source/image_io/analyze.cpp )
SET( IMAGE_IO_HDRS
	source/image_io/analyze.h
	source/image_io/analyze_header.h )

SET( LICENSE_HDRS source/license.h )

SET( MESH_SRCS
	source/mesh/cmiss_element_private.cpp
	source/mesh/cmiss_node_private.cpp )
SET( MESH_HDRS
	source/mesh/cmiss_node_private.hpp
	source/mesh/cmiss_element_private.hpp )

SET( NODE_SRCS source/node/node_operations.cpp )
SET( NODE_HDRS source/node/node_operations.h )

SET( REGION_SRCS source/region/cmiss_region.cpp
	source/stream/region_stream.cpp
	source/region/cmiss_region_write_info.cpp )
SET( REGION_HDRS source/region/cmiss_region.h
	source/region/cmiss_region_private.h
	source/stream/region_stream.hpp
	source/region/cmiss_region_write_info.h )

SET( SELECTION_SRCS source/selection/any_object_selection.cpp
	source/selection/element_point_ranges_selection.cpp )
SET( SELECTION_HDRS source/selection/any_object_selection.h
	source/selection/element_point_ranges_selection.h )

SET( TIME_SRCS
	source/time/time.cpp
	source/time/time_keeper.cpp )
SET( TIME_HDRS
	source/time/time.h
	source/time/time_keeper.hpp
	source/time/time_private.h )

SET( THREE_D_DRAWING_SRCS
	source/three_d_drawing/graphics_buffer.cpp )
SET( THREE_D_DRAWING_HDRS
	source/three_d_drawing/graphics_buffer.h )
IF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )
	#SET( THREE_D_DRAWING_SRCS ${THREE_D_DRAWING_SRCS} source/general/photogrammetry.cpp )
	SET( THREE_D_DRAWING_HDRS ${THREE_D_DRAWING_HDRS} source/three_d_drawing/abstract_graphics_buffer.h )
ENDIF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )
