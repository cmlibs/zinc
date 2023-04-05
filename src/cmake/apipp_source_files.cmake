
# Defines APIPP_SRCS, APIPP_TYPES_HDRS

# Zinc Library
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

SET( APIPP_HDRS
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/changemanager.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/context.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/differentialoperator.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/element.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/elementbasis.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/elementfieldtemplate.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/elementtemplate.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldcache.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/field.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldmodule.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldalias.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldapply.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldarithmeticoperators.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldassignment.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldcomposite.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldconditional.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldconstant.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldcoordinatetransformation.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldderivatives.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldfibres.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldfiniteelement.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldgroup.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldimage.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldimageprocessing.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldlogicaloperators.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldmatrixoperators.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldmeshoperators.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldnodesetoperators.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldparameters.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldrange.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldsceneviewerprojection.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldsmoothing.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldsubobjectgroup.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldtime.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldtrigonometry.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/fieldvectoroperators.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/glyph.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/graphics.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/font.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/light.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/logger.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/material.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/mesh.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/node.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/nodeset.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/nodetemplate.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/optimisation.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/region.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/result.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/scene.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/scenefilter.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/scenepicker.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/sceneviewer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/sceneviewerinput.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/shader.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/selection.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/spectrum.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/status.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/stream.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/streamimage.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/streamregion.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/streamscene.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/tessellation.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/timenotifier.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/timekeeper.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/timesequence.hpp
    )

SET( APIPP_TYPES_HDRS
    ${CMAKE_CURRENT_SOURCE_DIR}/api/cmlibs/zinc/types/scenecoordinatesystem.hpp
    )

