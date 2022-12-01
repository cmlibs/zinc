#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

#include "test_resources.h"

std::string resourcePath(const std::string &resourceRelativePath)
{
    return std::string(TESTS_RESOURCE_LOCATION) + "/" + resourceRelativePath;
}

std::string fileContents(const std::string &fileName)
{
    std::ifstream file(resourcePath(fileName));
    std::stringstream buffer;

    buffer << file.rdbuf();

    return buffer.str();
}

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
    folderName(TESTS_OUTPUT_LOCATION + folderNameIn)
{
    createSubfolder(folderName);
}

ManageOutputFolder::~ManageOutputFolder()
{
    // future: remove output folder
}

std::string ManageOutputFolder::getPath(const std::string &fileName) const
{
    return folderName + fileName;
}
