/**
 * tessellation.i
 *
 */
/*
 * OpenCMISS-Zinc Library
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

%module(package="opencmiss.zinc") shader

%include "doublevaluesarraytypemap.i"
%include "integervaluesarraytypemap.i"
%include "pyzincstringhandling.i"

%extend OpenCMISS::Zinc::Shaderprogram {
	bool operator==(const OpenCMISS::Zinc::Shaderprogram& other) const
	{
		return *($self) == other;
	}
}

%extend OpenCMISS::Zinc::Shaderuniforms {
	bool operator==(const OpenCMISS::Zinc::Shaderuniforms& other) const
	{
		return *($self) == other;
	}
}

%{
#include "opencmiss/zinc/shader.hpp"
%}

%include "opencmiss/zinc/shader.hpp"
