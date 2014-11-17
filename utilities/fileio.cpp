/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "fileio.hpp"

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

namespace {

void createSubfolder(const char *name)
{
#ifdef _WIN32
	_mkdir(name);
#else 
	mkdir(name, 0777);
#endif
}

}

ManageOutputFolder::ManageOutputFolder(const char *folderNameIn) :
	folderName(folderNameIn)
{
	createSubfolder(folderNameIn);
}

ManageOutputFolder::~ManageOutputFolder()
{
	// future: remove output folder
}
