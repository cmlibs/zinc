/**
 * stream.i
 *
 */
/*
 * OpenCMISS-Zinc Library
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

%module(package="opencmiss.zinc") stream
%typemap(in) (const void *buffer, unsigned int buffer_length)
{
	$1 = PyString_AsString($input);   /* char *str */
	$2 = PyString_Size($input);       /* int len   */
}

%{
#include "zinc/stream.hpp"
%}

%include "zinc/stream.hpp"
