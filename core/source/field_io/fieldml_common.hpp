/**
 * FILE : fieldml_common.hpp
 * 
 * Common data and classes for FieldML 0.5 I/O.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if !defined (CMZN_FIELDML_COMMON_HPP)
#define CMZN_FIELDML_COMMON_HPP

#include "zinc/element.h"

struct ShapeType
{
	const char *fieldmlName;
	const int dimension;
	enum cmzn_element_shape_type shape_type;
};

const ShapeType libraryShapes[] =
{
	{ "shape.unit.line",        1, CMZN_ELEMENT_SHAPE_TYPE_LINE },
	{ "shape.unit.square",      2, CMZN_ELEMENT_SHAPE_TYPE_SQUARE },
	{ "shape.unit.triangle",    2, CMZN_ELEMENT_SHAPE_TYPE_TRIANGLE },
	{ "shape.unit.cube",        3, CMZN_ELEMENT_SHAPE_TYPE_CUBE },
	{ "shape.unit.tetrahedron", 3, CMZN_ELEMENT_SHAPE_TYPE_TETRAHEDRON },
	{ "shape.unit.wedge12",     3, CMZN_ELEMENT_SHAPE_TYPE_WEDGE12 },
	{ "shape.unit.wedge13",     3, CMZN_ELEMENT_SHAPE_TYPE_WEDGE13 },
	{ "shape.unit.wedge23",     3, CMZN_ELEMENT_SHAPE_TYPE_WEDGE23 }
};
const int numLibraryShapes = sizeof(libraryShapes) / sizeof(ShapeType);

enum cmzn_element_shape_type getElementShapeFromFieldmlName(const char *shapeName);

const char *getFieldmlNameFromElementShape(enum cmzn_element_shape_type shapeType);

/**
 * @return  True if the file_name has an extension expected for FieldML,
 * e.g. .fieldml.
 */
bool filename_has_FieldML_extension(const char *filename);

#endif /* !defined (CMZN_FIELDML_COMMON_HPP) */
