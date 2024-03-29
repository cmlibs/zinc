# Zinc Library Unit Tests
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

SET(CURRENT_TEST fieldio)
LIST(APPEND API_TESTS ${CURRENT_TEST})
SET(${CURRENT_TEST}_SRC
  ${CURRENT_TEST}/ex_io.cpp
  ${CURRENT_TEST}/fieldml_basic.cpp
  ${CURRENT_TEST}/fieldml_hermite.cpp
	)
