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

%import "region.i"
%import "sceneviewer.i"

%{
#include "zinc/context.hpp"
%}

%include "zinc/context.hpp"

