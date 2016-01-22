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
#include "zinc/context.hpp"
#include "zinc/font.hpp"
#include "zinc/light.hpp"
#include "zinc/logger.hpp"
#include "zinc/glyph.hpp"
#include "zinc/material.hpp"
#include "zinc/region.hpp"
#include "zinc/scenefilter.hpp"
#include "zinc/sceneviewer.hpp"
#include "zinc/spectrum.hpp"
#include "zinc/tessellation.hpp"
#include "zinc/timekeeper.hpp"
%}

%include "zinc/context.hpp"

