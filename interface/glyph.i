/**
 * glyph.i
 *
 */
/*
 * OpenCMISS-Zinc Library
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

%module(package="opencmiss.zinc") glyph

%import "material.i"
%import "spectrum.i"

%extend OpenCMISS::Zinc::Glyph {
	bool operator==(const OpenCMISS::Zinc::Glyph& other) const
	{
		return *($self) == other;
	}
}

%{
#include "zinc/glyph.hpp"
#include "zinc/graphics.hpp"
%}

%include "zinc/glyph.hpp"
