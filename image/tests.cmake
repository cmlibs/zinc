# OpenCMISS-Zinc Library Unit Tests
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

SET(CURRENT_TEST image)
LIST(APPEND API_TESTS ${CURRENT_TEST})
SET(${CURRENT_TEST}_SRC
	${CURRENT_TEST}/image.cpp
	)

SET(IMAGE_PNG_RESOURCE "${CMAKE_CURRENT_SOURCE_DIR}/resources/image-1.png")
SET(IMAGE_ANALYZE_BIGENDIAN_RESOURCE "${CMAKE_CURRENT_SOURCE_DIR}/resources/bigendian.hdr")
SET(IMAGE_ANALYZE_LITTLEENDIAN_RESOURCE "${CMAKE_CURRENT_SOURCE_DIR}/resources/littleendian.hdr")
SET(IMAGE_ANALYZE_LITTLEENDIAN_TAR_RESOURCE "${CMAKE_CURRENT_SOURCE_DIR}/resources/littleendian.tar.gz")
