/***************************************************************************//**
 * FILE : cmiss_region_stream.hpp
 *
 * The private interface to cmzn_region_stream.
 *
 */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005-2011
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#if !defined (CMZN_REGION_STREAM_HPP)
#define CMZN_REGION_STREAM_HPP

#include "zinc/types/fieldid.h"
#include "zinc/region.h"
#include "zinc/status.h"
#include "stream/cmiss_stream_private.hpp"

struct cmzn_region_resource_properties : cmzn_resource_properties
{
public:

	cmzn_region_resource_properties(cmzn_stream_resource_id resource_in) :
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

struct cmzn_stream_information_region : cmzn_stream_information
{
public:

	cmzn_stream_information_region(struct cmzn_region *region_in) :
		region(cmzn_region_access(region_in)), root_region(cmzn_region_access(region_in))
	{
		time_enabled = false;
		time = 0.0;
	}

	virtual ~cmzn_stream_information_region()
	{
		if (root_region)
			cmzn_region_destroy(&root_region);
		if (region)
			cmzn_region_destroy(&region);
	}

	virtual int addResource(cmzn_stream_resource_id resourec_in)
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

	virtual cmzn_resource_properties *createResourceProperties(cmzn_stream_resource_id resource)
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

	double getResourceTime(cmzn_stream_resource_id resource)
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

	bool isResourceTimeEnabled(cmzn_stream_resource_id resource)
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

	int setResourceTime(cmzn_stream_resource_id resource, double time_in)
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

	int getResourceDomainType(cmzn_stream_resource_id resource)
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

	int setResourceDomainType(cmzn_stream_resource_id resource, int domain_type_in)
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

cmzn_region_id cmzn_stream_information_region_get_region_private(
	cmzn_stream_information_region_id stream_information);

cmzn_region_id cmzn_stream_information_region_get_root_region(
	cmzn_stream_information_region_id stream_information);

#endif /* CMZN_REGION_STREAM_HPP */
