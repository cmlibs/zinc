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

%include "pyzincstringhandling.i"

%{
#include "zinc/stream.hpp"
#include "zinc/streamimage.hpp"
#include "zinc/streamregion.hpp"
#include "zinc/streamscene.hpp"
%}

%extend OpenCMISS::Zinc::Streaminformation {
	OpenCMISS::Zinc::StreamresourceMemory createStreamresourceMemoryBuffer(const void *buffer, unsigned int buffer_length)
	{
		return  $self->createStreamresourceMemoryBufferCopy(buffer, buffer_length);
	}
}

%ignore OpenCMISS::Zinc::Streaminformation::createStreamresourceMemoryBuffer(const void *buffer, unsigned int buffer_length);

%include "zinc/stream.hpp"
