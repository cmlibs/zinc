/**
 * light.i
 *
 * Swig interface file for Zinc light.
 */
/*
 * OpenCMISS-Zinc Library
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

%module(package="opencmiss.zinc") light

%include "doublevaluesarraytypemap.i"
%include "pyzincstringhandling.i"

%extend OpenCMISS::Zinc::Light {
	bool operator==(const OpenCMISS::Zinc::Light& other) const
	{
		return *($self) == other;
	}
}

%{
#include "zinc/light.hpp"
%}

%include "zinc/light.hpp"
