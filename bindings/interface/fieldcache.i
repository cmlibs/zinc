/**
 * fieldcache.i
 * 
 * Swig interface file for wrapping api functions in api/fieldcache.hpp
 */
/*
 * OpenCMISS-Zinc Library
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

%module(package="opencmiss.zinc") fieldcache
%include "doublevaluesarraytypemap.i"
%include "pyzincstringhandling.i"
%typemap(in) (const char *stringValue) = (const char *name);

%import "field.i"
%import "element.i"
%import "node.i"

%{
#include "opencmiss/zinc/fieldcache.hpp"
%}

%include "opencmiss/zinc/fieldcache.hpp"

