
#pragma once

#include <string>

#include "test_exportdefinitions.h"

const char TESTS_RESOURCE_LOCATION[] = "@TESTS_RESOURCE_LOCATION@";
const char TESTS_OUTPUT_LOCATION[] = "@TESTS_OUTPUT_LOCATION@";

std::string TEST_EXPORT resourcePath(const std::string &resourceRelativePath = "");
std::string TEST_EXPORT fileContents(const std::string &fileName);

// Ensures folder of supplied name exists for lifetime of object
// Note doesn't yet clear folder.
class TEST_EXPORT ManageOutputFolder
{
    std::string folderName;
public:
    ManageOutputFolder(const std::string &folderNameIn);
    ~ManageOutputFolder();

    std::string getPath(const std::string &fileName) const;
};
