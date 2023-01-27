/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "fileio.hpp"
#include <cstdio>
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

namespace {

void createSubfolder(const std::string &name)
{
#ifdef _WIN32
    _mkdir(name.c_str());
#else 
    mkdir(name.c_str(), 0755);
#endif
}

}

ManageOutputFolder::ManageOutputFolder(const std::string &folderNameIn) :
	folderName(folderNameIn)
{
    createSubfolder(folderNameIn);
}

ManageOutputFolder::~ManageOutputFolder()
{
	// future: remove output folder
}

char *readFileToString(const char *filename)
{
	FILE * f = fopen(filename, "rb");
	if (!f)
	{
		return nullptr;
	}
	fseek(f, 0, SEEK_END);
	const size_t length = static_cast<size_t>(ftell(f));
	char *stringBuffer = nullptr;
	stringBuffer = static_cast<char *>(malloc(length + 1));  // allow for zero terminating character
	if (stringBuffer)
	{
		fseek(f, 0, SEEK_SET);
		const size_t readLength = fread(stringBuffer, 1, length, f);
		stringBuffer[length] = 0;
		if (readLength != length)
		{
			// incomplete read
			free(stringBuffer);
			stringBuffer = nullptr;
		}
	}
	fclose(f);
	return stringBuffer;
}
