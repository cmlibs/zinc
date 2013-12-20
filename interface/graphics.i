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

%import "field.i"
%import "tessellation.i"
%import "glyph.i"
%import "material.i"
%import "scenecoordinatesystem.i"
%import "spectrum.i"
%import "font.i"

%{
#include "zinc/graphics.hpp"
#include "zinc/fieldconditional.hpp"
%}

%include "zinc/graphics.hpp"

