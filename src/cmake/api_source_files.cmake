
# Defines API_HDRS

# Zinc Library
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

SET( API_HDRS
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/context.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/core.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/differentialoperator.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/element.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/elementbasis.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/elementfieldtemplate.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/elementtemplate.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/field.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldapply.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldarithmeticoperators.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldassignment.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldcache.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldcomposite.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldconditional.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldconstant.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldcoordinatetransformation.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldderivatives.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldfibres.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldfiniteelement.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldgroup.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldimage.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldimageprocessing.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldlogicaloperators.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldmatrixoperators.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldmeshoperators.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldmodule.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldnodesetoperators.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldparameters.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldrange.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldsceneviewerprojection.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldsmoothing.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldtime.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldtrigonometry.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldvectoroperators.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/glyph.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/graphics.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/font.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/light.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/logger.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/material.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/mesh.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/node.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/nodeset.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/nodetemplate.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/optimisation.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/region.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/result.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/scene.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/scenefilter.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/scenepicker.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/selection.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/shader.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/spectrum.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/status.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/stream.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/streamimage.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/streamregion.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/streamscene.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/tessellation.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/timenotifier.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/timekeeper.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/timesequence.h)

SET( API_TYPES_HDRS
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/cinline.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/contextid.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/differentialoperatorid.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/elementid.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/elementbasisid.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/elementfieldtemplateid.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/elementtemplateid.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/fieldapplyid.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/fieldcacheid.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/fieldcompositeid.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/fieldconstantid.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/fieldfiniteelementid.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/fieldgroupid.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/fieldid.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/fieldassignmentid.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/fieldderivativesid.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/fieldimageid.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/fieldimageprocessingid.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/fieldmatrixoperatorsid.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/fieldmeshoperatorsid.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/fieldmoduleid.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/fieldnodesetoperatorsid.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/fieldparametersid.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/fieldrangeid.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/fieldsmoothingid.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/fontid.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/glyphid.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/graphicsid.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/lightid.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/loggerid.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/materialid.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/meshid.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/nodeid.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/nodesetid.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/nodetemplateid.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/optimisationid.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/regionid.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/scenecoordinatesystem.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/scenefilterid.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/sceneid.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/scenepickerid.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/selectionid.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/shaderid.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/spectrumid.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/streamid.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/tessellationid.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/timekeeperid.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/timenotifierid.h
  ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/timesequenceid.h )

IF( USE_OPENCASCADE )
	SET( API_HDRS ${API_HDRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldcad.h)
	SET( API_TYPES_HDRS ${API_TYPES_HDRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/fieldcadid.h )
ENDIF( USE_OPENCASCADE )

IF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )
	SET( API_HDRS ${API_HDRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/sceneviewer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/sceneviewerinput.h)
	SET( API_TYPES_HDRS ${API_TYPES_HDRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/sceneviewerid.h
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/sceneviewerinputid.h )
ENDIF( ${GRAPHICS_API} MATCHES OPENGL_GRAPHICS )

