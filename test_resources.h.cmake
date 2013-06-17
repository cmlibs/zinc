
#ifndef TEST_RESOURCES_H_
#define TEST_RESOURCES_H_

class TestResources
{

public:

	enum ResourcesName
	{
		FIELDIMAGE_PNG_RESOURCE = 1,
		FIELDIMAGE_BIGENDIAN_RESOURCE = 2,
		FIELDIMAGE_LITTLEENDIAN_RESOURCE = 3,
		FIELDMODULE_EXNODE_RESOURCE = 4,
		FIELDMODULE_CUBE_RESOURCE = 5,
		FIELDMODULE_REGION_INPUT_RESOURCE = 6
	};

	TestResources()
	{  }

	~TestResources()
	{  }

	static const char *getLocation(ResourcesName resourceName)
	{ 
		if (resourceName == TestResources::FIELDIMAGE_PNG_RESOURCE)
		{
			return "@FIELDIMAGE_PNG_RESOURCE@";
		}
		if (resourceName == TestResources::FIELDIMAGE_BIGENDIAN_RESOURCE)
		{
			return "@FIELDIMAGE_BIGENDIAN_RESOURCE@";
		}
		if (resourceName == TestResources::FIELDIMAGE_LITTLEENDIAN_RESOURCE)
		{
			return "@FIELDIMAGE_LITTLEENDIAN_RESOURCE@";
		}
		if (resourceName == TestResources::FIELDMODULE_EXNODE_RESOURCE)
		{
			return "@FIELDMODULE_EXNODE_RESOURCE@";
		}
		if (resourceName == TestResources::FIELDMODULE_CUBE_RESOURCE)
		{
			return "@FIELDMODULE_CUBE_RESOURCE@";
		}
		if (resourceName == TestResources::FIELDMODULE_REGION_INPUT_RESOURCE)
		{
			return "@FIELDMODULE_REGION_INPUT_RESOURCE@";
		}
		return 0;
	}
};

#endif /* TEST_RESOURCES_H_ */

