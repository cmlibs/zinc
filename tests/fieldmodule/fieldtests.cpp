/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include <opencmiss/zinc/core.h>
#include <opencmiss/zinc/field.hpp>

#include "utilities/testenum.hpp"
#include "zinctestsetupcpp.hpp"

TEST(ZincField, CoordinateSystemTypeEnum)
{
	const char *enumNames[7] = { nullptr, "RECTANGULAR_CARTESIAN", "CYLINDRICAL_POLAR",
		"SPHERICAL_POLAR", "PROLATE_SPHEROIDAL", "OBLATE_SPHEROIDAL", "FIBRE" };
	testEnum(7, enumNames, Field::CoordinateSystemTypeEnumToString, Field::CoordinateSystemTypeEnumFromString);
}

TEST(ZincField, DomainTypeEnum)
{
	const char *enumNames[8] = { nullptr, "POINT", "NODES", "DATAPOINTS",
		"MESH1D", "MESH2D", "MESH3D", "MESH_HIGHEST_DIMENSION" };
	testEnumBitShift(8, enumNames, Field::DomainTypeEnumToString, Field::DomainTypeEnumFromString);
}
