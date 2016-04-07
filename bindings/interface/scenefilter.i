/**
 * scenefilter.i
 *
 */
/*
 * OpenCMISS-Zinc Library
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

%module(package="opencmiss.zinc") scenefilter

%include "pyzincstringhandling.i"
%typemap(in) (const char *matchName) = (const char *name);

%import "graphics.i"
%import "region.i"

%extend OpenCMISS::Zinc::Scenefilter {
	bool operator==(const OpenCMISS::Zinc::Scenefilter& other) const
	{
		return *($self) == other;
	}
}

%{
#include "opencmiss/zinc/scenefilter.hpp"
#include "opencmiss/zinc/fieldconditional.hpp"
%}

%include "opencmiss/zinc/scenefilter.hpp"

