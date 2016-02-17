/**
 * context.i
 *
 * Swig interface file for zinc context.
 */
/*
 * OpenCMISS-Zinc Library
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

%module(package="opencmiss.zinc") context

%import "font.i"
%import "glyph.i"
%import "light.i"
%import "logger.i"
%import "material.i"
%import "region.i"
%import "scenefilter.i"
%import "sceneviewer.i"
%import "spectrum.i"
%import "tessellation.i"
%import "timekeeper.i"
%include "integervaluesarraytypemap.i"

%extend OpenCMISS::Zinc::Context {
	bool operator==(const OpenCMISS::Zinc::Context& other) const
	{
		return *($self) == other;
	}
}

%{
#include "opencmiss/zinc/context.hpp"
#include "opencmiss/zinc/font.hpp"
#include "opencmiss/zinc/light.hpp"
#include "opencmiss/zinc/logger.hpp"
#include "opencmiss/zinc/glyph.hpp"
#include "opencmiss/zinc/material.hpp"
#include "opencmiss/zinc/region.hpp"
#include "opencmiss/zinc/scenefilter.hpp"
#include "opencmiss/zinc/sceneviewer.hpp"
#include "opencmiss/zinc/spectrum.hpp"
#include "opencmiss/zinc/tessellation.hpp"
#include "opencmiss/zinc/timekeeper.hpp"
%}

%include "opencmiss/zinc/context.hpp"

