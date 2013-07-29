/***************************************************************************//**
 * FILE : cmiss_region_stream.hpp
 *
 * The private interface to Cmiss_region_stream.
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

#if !defined (CMISS_REGION_STREAM_HPP)
#define CMISS_REGION_STREAM_HPP

#include "zinc/types/fieldid.h"
#include "zinc/region.h"
#include "zinc/status.h"
#include "stream/cmiss_stream_private.hpp"

struct Cmiss_region_resource_properties : Cmiss_resource_properties
{
public:

	Cmiss_region_resource_properties(Cmiss_stream_resource_id resource_in) :
		Cmiss_resource_properties(resource_in), time_enabled(false),
		domain_type_enabled(false), time(0.0), domain_type(CMISS_FIELD_DOMAIN_TYPE_INVALID)
	{
	}

	~Cmiss_region_resource_properties()
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

	enum Cmiss_field_domain_type getDomainType()
	{
		return domain_type;
	}

	int isDomainTypeEnabled()
	{
		return domain_type_enabled;
	}

	int setDomainType(enum Cmiss_field_domain_type domain_type_in)
	{
		switch (domain_type_in)
		{
			case CMISS_FIELD_DOMAIN_NODES:
			case CMISS_FIELD_DOMAIN_DATA:
			case CMISS_FIELD_DOMAIN_ELEMENTS_HIGHEST_DIMENSION:
			{
				domain_type = domain_type_in;
				domain_type_enabled = true;
				return CMISS_OK;
			} break;
			default:
			{
				return CMISS_ERROR_GENERAL;
			} break;
		}
		return CMISS_ERROR_GENERAL;
	}

	int disableDomainType()
	{
		domain_type_enabled = false;
		return CMISS_OK;
	}

private:
	bool time_enabled, domain_type_enabled;
	double time;
	enum Cmiss_field_domain_type domain_type;
};

struct Cmiss_stream_information_region : Cmiss_stream_information
{
public:

	Cmiss_stream_information_region(struct Cmiss_region *region_in) :
		region(Cmiss_region_access(region_in)), root_region(Cmiss_region_access(region_in))
	{
		time_enabled = false;
		time = 0.0;
	}

	virtual ~Cmiss_stream_information_region()
	{
		if (root_region)
			Cmiss_region_destroy(&root_region);
		if (region)
			Cmiss_region_destroy(&region);
	}

	virtual int addResource(Cmiss_stream_resource_id resourec_in)
	{
		if (resourec_in && !findResourceInList(resourec_in))
		{
			Cmiss_region_resource_properties *resource_properties =
				new Cmiss_region_resource_properties(resourec_in);
			appendResourceProperties(resource_properties);
			return 1;
		}
		else
		{
			return 0;
		}
	}

	virtual Cmiss_resource_properties *createResourceProperties(Cmiss_stream_resource_id resource)
	{
		if (resource)
			return new Cmiss_region_resource_properties(resource);
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

	double getResourceTime(Cmiss_stream_resource_id resource)
	{
		if (resource)
		{
			Cmiss_region_resource_properties *resource_properties =
				(Cmiss_region_resource_properties *)findResourceInList(resource);
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

	bool isResourceTimeEnabled(Cmiss_stream_resource_id resource)
	{
		if (resource)
		{
			Cmiss_region_resource_properties *resource_properties =
				(Cmiss_region_resource_properties *)findResourceInList(resource);
			if (resource_properties)
			{
				return resource_properties->isTimeEnabled();
			}
		}
		return false;
	}

	int setResourceTime(Cmiss_stream_resource_id resource, double time_in)
	{
		if (resource)
		{
			Cmiss_region_resource_properties *resource_properties =
				(Cmiss_region_resource_properties *)findResourceInList(resource);
			if (resource_properties)
			{
				resource_properties->setTime(time_in);
				return 1;
			}
		}
		return 0;
	}

	enum Cmiss_field_domain_type getResourceDomainType(Cmiss_stream_resource_id resource)
	{
		if (resource)
		{
			Cmiss_region_resource_properties *resource_properties =
				(Cmiss_region_resource_properties *)findResourceInList(resource);
			if (resource_properties)
			{
				return resource_properties->getDomainType();
			}
		}
		return CMISS_FIELD_DOMAIN_TYPE_INVALID;
	}

	int setResourceDomainType(Cmiss_stream_resource_id resource, enum Cmiss_field_domain_type domain_type_in)
	{
		if (resource)
		{
			Cmiss_region_resource_properties *resource_properties =
				(Cmiss_region_resource_properties *)findResourceInList(resource);
			if (resource_properties)
			{
				return resource_properties->setDomainType(domain_type_in);
			}
		}
		return CMISS_ERROR_ARGUMENT;
	}

	bool isResourceDomainTypeEnabled(Cmiss_stream_resource_id resource)
	{
		if (resource)
		{
			Cmiss_region_resource_properties *resource_properties =
				(Cmiss_region_resource_properties *)findResourceInList(resource);
			if (resource_properties)
			{
				return resource_properties->isDomainTypeEnabled();
			}
		}
		return false;
	}

	int disableResourceDomainType(Cmiss_stream_resource_id resource)
	{
		if (resource)
		{
			Cmiss_region_resource_properties *resource_properties =
				(Cmiss_region_resource_properties *)findResourceInList(resource);
			if (resource_properties)
			{
				return resource_properties->disableDomainType();
			}
		}
		return CMISS_ERROR_ARGUMENT;
	}

	Cmiss_region_id getRegion()
	{
		return region;
	}

	Cmiss_region_id getRootRegion()
	{
		return root_region;
	}

	int setRootRegion(Cmiss_region_id root_region_in)
	{
		if (root_region != root_region_in)
		{
			if (root_region)
			{
				Cmiss_region_destroy(&root_region);
			}
			root_region = Cmiss_region_access(root_region_in);
		}
		return 1;
	}


private:
	double time;
	bool time_enabled;
	struct Cmiss_region *region, *root_region;
};

Cmiss_region_id Cmiss_stream_information_region_get_region_private(
	Cmiss_stream_information_region_id stream_information);

Cmiss_region_id Cmiss_stream_information_region_get_root_region(
	Cmiss_stream_information_region_id stream_information);

#endif /* CMISS_REGION_STREAM_HPP */
