
# Defines OPENCASCADE_SRCS

IF( USE_OPENCASCADE )
	SET( OPENCASCADE_SRCS 
		#source/cad/occpartfactory.cpp
		#source/cad/graphicimporter.cpp
		source/cad/cad_element.cpp
		source/cad/cad_tool.cpp
		source/cad/entity.cpp
		source/cad/element_identifier.cpp
		source/cad/point.cpp
		source/cad/curve.cpp
		source/cad/surface.cpp
		#source/cad/transformoccmodel.cpp
		source/cad/geometricshape.cpp
		source/cad/topologicalshape.cpp
		#source/cad/topologicalshaperoot.cpp
		source/cad/cadimporter.cpp
		source/cad/opencascadeimporter.cpp
		source/cad/opencascadeformatreader.cpp
		#source/cad/fileimportexport.cpp
		source/cad/computed_field_cad_topology.cpp 
		source/cad/computed_field_cad_geometry.cpp
		source/cad/computed_field_cad_colour.cpp
		source/cad/field_location.cpp
		source/cad/cad_geometry_to_graphics_object.cpp )
	
	SET( OPENCASCADE_HDRS
		source/cad/cad_element.h
		source/cad/cad_geometry_to_graphics_object.h
		source/cad/cad_tool.h
		source/cad/curve.h
		source/cad/point.h
		source/cad/cadimporter.h
		source/cad/entity.h
		source/cad/element_identifier.h
		#source/cad/occpartfactory.h
		source/cad/surface.h
		source/cad/field_location.hpp
		source/cad/topologicalshape.h
		source/cad/computed_field_cad_colour.h
		#source/cad/fileimportexport.h
		source/cad/opencascadeformatreader.h
		source/cad/topologicalshaperoot.h
		source/cad/computed_field_cad_geometry.h
		source/cad/transformoccmodel.h
		source/cad/computed_field_cad_topology.h
		source/cad/geometricshape.h
		source/cad/opencascadeimporter.h
		source/cad/computed_field_cad_topology_private.h )
		#source/cad/graphicimporter.h )

ENDIF( USE_OPENCASCADE )

