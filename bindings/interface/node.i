/**
 * node.i
 * 
 */
/*
 * OpenCMISS-Zinc Library
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

%module(package="opencmiss.zinc") node

%include "pyzincstringhandling.i"

%import "field.i"

%extend OpenCMISS::Zinc::Node {
	bool operator==(const OpenCMISS::Zinc::Node& other) const
	{
		return *($self) == other;
	}
}

%extend OpenCMISS::Zinc::Nodeset {
	bool operator==(const OpenCMISS::Zinc::Nodeset& other) const
	{
		return *($self) == other;
	}
}

%{
#include "zinc/node.hpp"
#include "zinc/field.hpp"
#include "zinc/fieldconditional.hpp"
%}

%include "zinc/node.hpp"

