/**
 * graphics.i
 *
 */
/*
 * OpenCMISS-Zinc Library
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

%module(package="opencmiss.zinc") graphics
%include "doublevaluesarraytypemap.i"
%include "integervaluesarraytypemap.i"
%include "pyzincstringhandling.i"
%typemap(in) (const char *labelText) = (const char *name);

%import "field.i"
%import "font.i"
%import "glyph.i"
%import "material.i"
%import "scenecoordinatesystem.i"
%import "spectrum.i"
%import "tessellation.i"

%extend OpenCMISS::Zinc::Graphics {
	bool operator==(const OpenCMISS::Zinc::Graphics& other) const
	{
		return *($self) == other;
	}
}

%{
#include "opencmiss/zinc/fieldmodule.hpp"
#include "opencmiss/zinc/graphics.hpp"
#include "opencmiss/zinc/scene.hpp"
%}

%include "opencmiss/zinc/graphics.hpp"

