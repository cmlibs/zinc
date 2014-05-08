
# Defines API_HDRS

# OpenCMISS-Zinc Library
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

SET( API_HDRS
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/context.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/core.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/differentialoperator.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/element.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/field.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/fieldalias.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/fieldarithmeticoperators.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/fieldcache.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/fieldcomposite.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/fieldconditional.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/fieldconstant.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/fieldcoordinatetransformation.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/fieldderivatives.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/fieldfibres.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/fieldfiniteelement.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/fieldgroup.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/fieldimage.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/fieldimageprocessing.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/fieldlogicaloperators.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/fieldmatrixoperators.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/fieldmeshoperators.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/fieldmodule.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/fieldnodesetoperators.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/fieldsceneviewerprojection.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/fieldsubobjectgroup.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/fieldtime.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/fieldtrigonometry.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/fieldvectoroperators.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/glyph.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/graphics.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/font.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/material.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/node.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/optimisation.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/region.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/scene.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/scenefilter.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/scenepicker.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/selection.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/spectrum.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/status.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/stream.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/streamimage.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/streamregion.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/tessellation.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/timenotifier.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/timekeeper.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/timesequence.h)

SET( API_TYPES_HDRS
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/types/cinline.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/types/contextid.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/types/differentialoperatorid.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/types/elementid.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/types/fieldaliasid.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/types/fieldcacheid.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/types/fieldcompositeid.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/types/fieldfiniteelementid.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/types/fieldgroupid.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/types/fieldid.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/types/fieldimageid.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/types/fieldimageprocessingid.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/types/fieldmeshoperatorsid.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/types/fieldmoduleid.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/types/fieldsubobjectgroupid.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/types/fontid.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/types/glyphid.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/types/graphicsid.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/types/materialid.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/types/nodeid.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/types/optimisationid.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/types/regionid.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/types/scenecoordinatesystem.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/types/scenefilterid.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/types/sceneid.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/types/scenepickerid.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/types/selectionid.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/types/spectrumid.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/types/streamid.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/types/tessellationid.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/types/timekeeperid.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/types/timenotifierid.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/types/timesequenceid.h )

IF( USE_OPENCASCADE )
	SET( API_HDRS ${API_HDRS}
		${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/fieldcad.h)
	SET( API_TYPES_HDRS ${API_TYPES_HDRS}
		${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/types/fieldcadid.h )
ENDIF( USE_OPENCASCADE )

IF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )
	SET( API_HDRS ${API_HDRS}
		${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/sceneviewer.h
		${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/sceneviewerinput.h)
	SET( API_TYPES_HDRS ${API_TYPES_HDRS}
		${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/types/sceneviewerid.h
		${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/types/sceneviewerinputid.h )
ENDIF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )

