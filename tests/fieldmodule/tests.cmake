# OpenCMISS-Zinc Library Unit Tests
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

SET(CURRENT_TEST fieldmodule)
LIST(APPEND API_TESTS ${CURRENT_TEST})
SET(${CURRENT_TEST}_SRC
  ${CURRENT_TEST}/create_composite.cpp
  ${CURRENT_TEST}/create_derivatives.cpp
  ${CURRENT_TEST}/create_if.cpp
  ${CURRENT_TEST}/create_nodeset_operators.cpp
  ${CURRENT_TEST}/create_vectoroperators.cpp
  ${CURRENT_TEST}/region_io.cpp
  ${CURRENT_TEST}/create_image_processing.cpp
  ${CURRENT_TEST}/create_fibre_axes.cpp
  ${CURRENT_TEST}/elementbasis.cpp
  ${CURRENT_TEST}/fieldapply.cpp
  ${CURRENT_TEST}/fieldarithmeticoperators.cpp
  ${CURRENT_TEST}/fieldassignment.cpp
  ${CURRENT_TEST}/fieldconditional.cpp
  ${CURRENT_TEST}/fieldconstant.cpp
  ${CURRENT_TEST}/fieldimage.cpp
  ${CURRENT_TEST}/fielditerator.cpp
  ${CURRENT_TEST}/fieldlogicaloperators.cpp
  ${CURRENT_TEST}/fieldmatrixoperatorstests.cpp
  ${CURRENT_TEST}/fieldmeshoperators.cpp
  ${CURRENT_TEST}/fieldmoduledescription.cpp
  ${CURRENT_TEST}/fieldmodulenotifier.cpp
  ${CURRENT_TEST}/fieldparameterstests.cpp
  ${CURRENT_TEST}/field_operator_derivatives.cpp
  ${CURRENT_TEST}/fieldrange.cpp
  ${CURRENT_TEST}/fieldsmoothing.cpp
  ${CURRENT_TEST}/fieldtests.cpp
  ${CURRENT_TEST}/finiteelement.cpp
  ${CURRENT_TEST}/nodesandelements.cpp
  ${CURRENT_TEST}/numerical_operators.cpp
  ${CURRENT_TEST}/timesequence.cpp
	)
