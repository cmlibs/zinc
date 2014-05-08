
# Defines APIPP_SRCS, APIPP_TYPES_HDRS

# OpenCMISS-Zinc Library
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

SET( APIPP_HDRS
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/context.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/differentialoperator.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/element.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/fieldcache.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/field.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/fieldmodule.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/fieldalias.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/fieldarithmeticoperators.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/fieldcomposite.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/fieldconditional.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/fieldconstant.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/fieldcoordinatetransformation.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/fieldderivatives.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/fieldfibres.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/fieldfiniteelement.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/fieldgroup.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/fieldimage.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/fieldimageprocessing.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/fieldlogicaloperators.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/fieldmatrixoperators.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/fieldmeshoperators.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/fieldnodesetoperators.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/fieldsceneviewerprojection.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/fieldsubobjectgroup.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/fieldtime.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/fieldtrigonometry.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/fieldvectoroperators.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/glyph.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/graphics.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/font.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/material.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/node.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/optimisation.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/region.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/scene.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/scenefilter.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/scenepicker.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/sceneviewer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/sceneviewerinput.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/selection.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/spectrum.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/status.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/stream.hpp
		${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/streamimage.hpp
		${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/streamregion.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/tessellation.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/timenotifier.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/timekeeper.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/timesequence.hpp
    )

SET( APIPP_TYPES_HDRS
    ${CMAKE_CURRENT_SOURCE_DIR}/source/api/zinc/types/scenecoordinatesystem.hpp
    )

