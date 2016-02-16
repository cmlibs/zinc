/**
 * region.i
 *
 * Swig interface file for Zinc region API.
 */
/*
 * OpenCMISS-Zinc Library
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

%module(package="opencmiss.zinc") region

%include "pyzincstringhandling.i"

%import "fieldmodule.i"
%import "scene.i"
%import "streamregion.i"

%extend OpenCMISS::Zinc::Region {
	bool operator==(const OpenCMISS::Zinc::Region& other) const
	{
		return *($self) == other;
	}
}

%{
#include "opencmiss/zinc/fieldmodule.hpp"
#include "opencmiss/zinc/region.hpp"
#include "opencmiss/zinc/scene.hpp"
#include "opencmiss/zinc/streamregion.hpp"
%}

%include "opencmiss/zinc/region.hpp"

