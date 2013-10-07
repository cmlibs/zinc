/***************************************************************************//**
 * FILE : cmiss_region_stream.hpp
 *
 * The private interface to cmzn_region_stream.
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if !defined (CMZN_REGION_STREAM_HPP)
#define CMZN_REGION_STREAM_HPP

#include "zinc/types/fieldid.h"
#include "zinc/region.h"
#include "zinc/status.h"
#include "stream/cmiss_stream_private.hpp"

struct cmzn_region_resource_properties : cmzn_resource_properties
{
public:

	cmzn_region_resource_properties(cmzn_streamresource_id resource_in) :
		cmzn_resource_properties(resource_in), time_enabled(false),
		time(0.0), domain_type((int)CMZN_FIELD_DOMAIN_TYPE_INVALID)
	{
	}

	~cmzn_region_resource_properties()
	{
	}

	double getTime()
	{
		return time;
	}

	bool isTimeEnabled()
	{
		return time_enabled;
	}

	int setTime(double time_in)
	{
		time = time_in;
		time_enabled = true;
		return 1;
	}

	int getDomainType()
	{
		return domain_type;
	}

	int setDomainType(int domain_type_in)
	{
		domain_type = domain_type_in;
		return CMZN_OK;
	}

private:
	bool time_enabled;
	double time;
	int domain_type;
};

struct cmzn_streaminformation_region : cmzn_streaminformation
{
public:

	cmzn_streaminformation_region(struct cmzn_region *region_in) :
		region(cmzn_region_access(region_in)), root_region(cmzn_region_access(region_in))
	{
		time_enabled = false;
		time = 0.0;
	}

	virtual ~cmzn_streaminformation_region()
	{
		if (root_region)
			cmzn_region_destroy(&root_region);
		if (region)
			cmzn_region_destroy(&region);
	}

	virtual int addResource(cmzn_streamresource_id resourec_in)
	{
		if (resourec_in && !findResourceInList(resourec_in))
		{
			cmzn_region_resource_properties *resource_properties =
				new cmzn_region_resource_properties(resourec_in);
			appendResourceProperties(resource_properties);
			return 1;
		}
		else
		{
			return 0;
		}
	}

	virtual cmzn_resource_properties *createResourceProperties(cmzn_streamresource_id resource)
	{
		if (resource)
			return new cmzn_region_resource_properties(resource);
		return NULL;
	}

	double getTime()
	{
		return time;
	}

	bool isTimeEnabled()
	{
		return time_enabled;
	}

	int setTime(double time_in)
	{
		time = time_in;
		return 1;
	}

	double getResourceTime(cmzn_streamresource_id resource)
	{
		if (resource)
		{
			cmzn_region_resource_properties *resource_properties =
				(cmzn_region_resource_properties *)findResourceInList(resource);
			if (resource_properties)
			{
				if (resource_properties->isTimeEnabled())
					return resource_properties->getTime();
				else
					return time;
			}
		}
		return 0.0;
	}

	bool isResourceTimeEnabled(cmzn_streamresource_id resource)
	{
		if (resource)
		{
			cmzn_region_resource_properties *resource_properties =
				(cmzn_region_resource_properties *)findResourceInList(resource);
			if (resource_properties)
			{
				return resource_properties->isTimeEnabled();
			}
		}
		return false;
	}

	int setResourceTime(cmzn_streamresource_id resource, double time_in)
	{
		if (resource)
		{
			cmzn_region_resource_properties *resource_properties =
				(cmzn_region_resource_properties *)findResourceInList(resource);
			if (resource_properties)
			{
				resource_properties->setTime(time_in);
				return 1;
			}
		}
		return 0;
	}

	int getResourceDomainType(cmzn_streamresource_id resource)
	{
		if (resource)
		{
			cmzn_region_resource_properties *resource_properties =
				(cmzn_region_resource_properties *)findResourceInList(resource);
			if (resource_properties)
			{
				return resource_properties->getDomainType();
			}
		}
		return CMZN_FIELD_DOMAIN_TYPE_INVALID;
	}

	int setResourceDomainType(cmzn_streamresource_id resource, int domain_type_in)
	{
		if (resource)
		{
			cmzn_region_resource_properties *resource_properties =
				(cmzn_region_resource_properties *)findResourceInList(resource);
			if (resource_properties)
			{
				return resource_properties->setDomainType(domain_type_in);
			}
		}
		return CMZN_ERROR_ARGUMENT;
	}

	cmzn_region_id getRegion()
	{
		return region;
	}

	cmzn_region_id getRootRegion()
	{
		return root_region;
	}

	int setRootRegion(cmzn_region_id root_region_in)
	{
		if (root_region != root_region_in)
		{
			if (root_region)
			{
				cmzn_region_destroy(&root_region);
			}
			root_region = cmzn_region_access(root_region_in);
		}
		return 1;
	}


private:
	double time;
	bool time_enabled;
	struct cmzn_region *region, *root_region;
};

cmzn_region_id cmzn_streaminformation_region_get_region_private(
	cmzn_streaminformation_region_id streaminformation);

cmzn_region_id cmzn_streaminformation_region_get_root_region(
	cmzn_streaminformation_region_id streaminformation);

#endif /* CMZN_REGION_STREAM_HPP */
