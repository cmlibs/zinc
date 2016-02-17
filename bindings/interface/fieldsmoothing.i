/**
 * fieldsmoothing.i
 * 
 * Swig interface file for wrapping api functions in zinc/fieldsmoothing.hpp
 */
/*
 * OpenCMISS-Zinc Library
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

%module(package="opencmiss.zinc") fieldsmoothing

%{
#include "opencmiss/zinc/fieldsmoothing.hpp"
%}

%include "opencmiss/zinc/fieldsmoothing.hpp"

