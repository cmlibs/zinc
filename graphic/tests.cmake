
SET(CURRENT_TEST graphic)
LIST(APPEND API_TESTS ${CURRENT_TEST})
SET(${CURRENT_TEST}_SRC
    ${CURRENT_TEST}/contours.cpp
    ${CURRENT_TEST}/graphic.cpp
    ${CURRENT_TEST}/graphics_filter.cpp
    ${CURRENT_TEST}/graphics_material.cpp
    ${CURRENT_TEST}/font.cpp
    ${CURRENT_TEST}/scene_picker.cpp
    ${CURRENT_TEST}/spectrum.cpp
    ${CURRENT_TEST}/streamlines.cpp
    ${CURRENT_TEST}/tessellation.cpp
    )

