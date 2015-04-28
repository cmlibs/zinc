/**
 * spectrum.i
 *
 */
/*
 * OpenCMISS-Zinc Library
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

%module(package="opencmiss.zinc") spectrum

%include "pyzincstringhandling.i"

%extend OpenCMISS::Zinc::Spectrum {
	bool operator==(const OpenCMISS::Zinc::Spectrum& other) const
	{
		return *($self) == other;
	}
}

%{
#include "zinc/spectrum.hpp"
%}

%include "zinc/spectrum.hpp"
