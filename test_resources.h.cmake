
#ifndef TEST_RESOURCES_H_
#define TEST_RESOURCES_H_

class TestResources
{

public:

	enum ResourcesName
	{
		IMAGE_PNG_RESOURCE = 1,
		IMAGE_ANALYZE_BIGENDIAN_RESOURCE = 2,
		IMAGE_ANALYZE_LITTLEENDIAN_RESOURCE = 3,
		IMAGE_ANALYZE_LITTLEENDIAN_TAR_RESOURCE = 4,
		FIELDMODULE_EXNODE_RESOURCE = 5,
		FIELDMODULE_CUBE_RESOURCE = 6,
		FIELDMODULE_CUBE_GRID_RESOURCE = 7,
		FIELDMODULE_REGION_INPUT_RESOURCE = 8,
		FIELDMODULE_EMBEDDING_ISSUE3614_RESOURCE = 9,
		TESTIMAGE_GRAY_JPG_RESOURCE = 10,
		FIELDIMAGE_BLOCKCOLOURS_RESOURCE = 11,
		HEART_EXNODE_GZ = 12,
		HEART_EXELEM_GZ = 13
	};

	TestResources()
	{  }

	~TestResources()
	{  }

	static const char *getLocation(ResourcesName resourceName)
	{
		if (resourceName == TestResources::IMAGE_PNG_RESOURCE)
		{
			return "@IMAGE_PNG_RESOURCE@";
		}
		if (resourceName == TestResources::IMAGE_ANALYZE_BIGENDIAN_RESOURCE)
		{
			return "@IMAGE_ANALYZE_BIGENDIAN_RESOURCE@";
		}
		if (resourceName == TestResources::IMAGE_ANALYZE_LITTLEENDIAN_RESOURCE)
		{
			return "@IMAGE_ANALYZE_LITTLEENDIAN_RESOURCE@";
		}
		if (resourceName == TestResources::IMAGE_ANALYZE_LITTLEENDIAN_TAR_RESOURCE)
		{
			return "@IMAGE_ANALYZE_LITTLEENDIAN_TAR_RESOURCE@";
		}
		if (resourceName == TestResources::FIELDMODULE_EXNODE_RESOURCE)
		{
			return "@FIELDMODULE_EXNODE_RESOURCE@";
		}
		if (resourceName == TestResources::FIELDMODULE_CUBE_RESOURCE)
		{
			return "@FIELDMODULE_CUBE_RESOURCE@";
		}
		if (resourceName == TestResources::FIELDMODULE_CUBE_GRID_RESOURCE)
		{
			return "@FIELDMODULE_CUBE_GRID_RESOURCE@";
		}
		if (resourceName == TestResources::FIELDMODULE_REGION_INPUT_RESOURCE)
		{
			return "@FIELDMODULE_REGION_INPUT_RESOURCE@";
		}
		if (resourceName == TestResources::FIELDMODULE_EMBEDDING_ISSUE3614_RESOURCE)
		{
			return "@FIELDMODULE_EMBEDDING_ISSUE3614_RESOURCE@";
		}
		if (resourceName == TestResources::TESTIMAGE_GRAY_JPG_RESOURCE)
		{
			return "@TESTIMAGE_GRAY_JPG_RESOURCE@";
		}
		if (resourceName == TestResources::FIELDIMAGE_BLOCKCOLOURS_RESOURCE)
		{
			return "@FIELDIMAGE_BLOCKCOLOURS_RESOURCE@";
		}
		if (resourceName == TestResources::HEART_EXNODE_GZ)
		{
			return "@HEART_EXNODE_GZ@";
		}
		if (resourceName == TestResources::HEART_EXELEM_GZ)
		{
			return "@HEART_EXELEM_GZ@";
		}
		return 0;
	}
};

#endif /* TEST_RESOURCES_H_ */

