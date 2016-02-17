/**
 * element.i
 * 
 */
/*
 * OpenCMISS-Zinc Library
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

%module(package="opencmiss.zinc") element

%include "integervaluesarraytypemap.i"
%include "pyzincstringhandling.i"

%import "field.i"
%import "node.i"

%extend OpenCMISS::Zinc::Element {
	bool operator==(const OpenCMISS::Zinc::Element& other) const
	{
		return *($self) == other;
	}
}

%extend OpenCMISS::Zinc::Mesh {
	bool operator==(const OpenCMISS::Zinc::Mesh& other) const
	{
		return *($self) == other;
	}
}

%{
#include "opencmiss/zinc/fieldimage.hpp"
#include "opencmiss/zinc/element.hpp"
%}

%include "opencmiss/zinc/element.hpp"

