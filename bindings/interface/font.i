/**
 * font.i
 *
 */
/*
 * OpenCMISS-Zinc Library
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

%module(package="opencmiss.zinc") font

%include "pyzincstringhandling.i"

%import "graphics.i"

%extend OpenCMISS::Zinc::Font {
	bool operator==(const OpenCMISS::Zinc::Font& other) const
	{
		return *($self) == other;
	}
}

%{
#include "zinc/font.hpp"
#include "zinc/fieldconditional.hpp"
%}

%include "zinc/font.hpp"

