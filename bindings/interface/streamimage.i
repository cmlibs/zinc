/**
 * streamimage.i
 *
 * Swig interface file for Zinc image stream API.
 */
/*
 * OpenCMISS-Zinc Library
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

%module(package="opencmiss.zinc") streamimage

%include "pyzincstringhandling.i"

%import "stream.i"

%{
#include "opencmiss/zinc/streamimage.hpp"
%}

%include "opencmiss/zinc/streamimage.hpp"
