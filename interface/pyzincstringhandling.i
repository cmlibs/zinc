/**
 * pyzincstringhandling.i
 *
 */
/*
 * OpenCMISS-Zinc Library
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "zinc/core.h"

%typemap(newfree) char * "free($1);";

%newobject *::getName(); 

%newobject *::getComponentName(int componentNumber);

%newobject *::evaluateString;

%newobject *::getProperty(const char* property);

%newobject *::getLabelText(int labelNumber);

%newobject *::getSolutionReport();

/* the following line handle binary data */
%apply (char *STRING, size_t LENGTH) { (const void *buffer, unsigned int buffer_length) }
