
# Defines APIPP_SRCS, APIPP_TYPES_HDRS

# OpenCMISS-Zinc Library
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

SET( APIPP_HDRS
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/context.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/differentialoperator.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/element.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/fieldcache.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/field.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/fieldmodule.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/fieldalias.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/fieldarithmeticoperators.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/fieldcomposite.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/fieldconditional.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/fieldconstant.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/fieldcoordinatetransformation.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/fieldderivatives.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/fieldfibres.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/fieldfiniteelement.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/fieldgroup.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/fieldimage.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/fieldimageprocessing.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/fieldlogicaloperators.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/fieldmatrixoperators.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/fieldmeshoperators.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/fieldnodesetoperators.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/fieldsceneviewerprojection.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/fieldsmoothing.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/fieldsubobjectgroup.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/fieldtime.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/fieldtrigonometry.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/fieldvectoroperators.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/glyph.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/graphics.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/font.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/light.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/logger.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/material.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/node.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/optimisation.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/region.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/scene.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/scenefilter.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/scenepicker.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/sceneviewer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/sceneviewerinput.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/selection.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/spectrum.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/status.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/stream.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/streamimage.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/streamregion.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/streamscene.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/tessellation.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/timenotifier.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/timekeeper.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/timesequence.hpp
    )

SET( APIPP_TYPES_HDRS
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/opencmiss/zinc/types/scenecoordinatesystem.hpp
    )

