/**
 * scene.i
 *
 */
/*
 * OpenCMISS-Zinc Library
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

%module(package="opencmiss.zinc") scene

%import "graphics.i"
%import "scenefilter.i"
%import "selection.i"
%import "timekeeper.i"
%import "scenepicker.i"
%import "streamscene.i"

%extend OpenCMISS::Zinc::Scene {
	bool operator==(const OpenCMISS::Zinc::Scene& other) const
	{
		return *($self) == other;
	}
}

%{
#include "zinc/scene.hpp"
#include "zinc/scenepicker.hpp"
#include "zinc/streamscene.hpp"
#include "zinc/sceneviewer.hpp"
%}

%include "zinc/scene.hpp"

