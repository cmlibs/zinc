/**
 * scenepicker.i
 *
 */
/*
 * OpenCMISS-Zinc Library
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

%module(package="opencmiss.zinc") scenepicker

%include "doublevaluesarraytypemap.i"

%import "element.i"
%import "graphics.i"
%import "node.i"
%import "scenecoordinatesystem.i"
%import "sceneviewer.i"

%{
#include "zinc/fieldgroup.hpp"
#include "zinc/scene.hpp"
#include "zinc/scenefilter.hpp"
#include "zinc/scenepicker.hpp"
%}

%include "zinc/scenepicker.hpp"
