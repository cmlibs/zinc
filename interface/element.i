/**
 * element.i
 * 
 */
/*
 * OpenCMISS-Zinc Library
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

%module(package="opencmiss.zinc") element

%include "integervaluesarraytypemap.i"

%import "field.i"
%import "node.i"

%{
#include "zinc/fieldimage.hpp"
#include "zinc/element.hpp"
%}

%include "zinc/element.hpp"

