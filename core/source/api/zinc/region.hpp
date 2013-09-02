/***************************************************************************//**
 * FILE : region.hpp
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
 * The Original Code is libZinc.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2010
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
#ifndef CMZN_REGION_HPP__
#define CMZN_REGION_HPP__

#include "zinc/region.h"
#include "zinc/fieldmodule.hpp"
#include "zinc/stream.hpp"

namespace zinc
{

class StreamInformationRegion;

class Region
{
protected:
	cmzn_region_id id;

public:

	Region() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit Region(cmzn_region_id in_region_id) : id(in_region_id)
	{  }

	Region(const Region& region) : id(cmzn_region_access(region.id))
	{ }

	Region& operator=(const Region& region)
	{
		cmzn_region_id temp_id = cmzn_region_access(region.id);
		if (0 != id)
		{
			cmzn_region_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~Region()
	{
		if (0 != id)
		{
			cmzn_region_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	cmzn_region_id getId()
	{
		return id;
	}

	int beginChange()
	{
		return cmzn_region_begin_change(id);
	}

	int endChange()
	{
		return cmzn_region_end_change(id);
	}

	int beginHierarchicalChange()
	{
		return cmzn_region_begin_hierarchical_change(id);
	}

	int endHierarchicalChange()
	{
		return cmzn_region_end_hierarchical_change(id);
	}

	Region createChild(const char *name)
	{
		return Region(cmzn_region_create_child(id, name));
	}

	Region createSubregion(const char *path)
	{
		return Region(cmzn_region_create_subregion(id, path));
	}

	Region createRegion()
	{
		return Region(cmzn_region_create_region(id));
	}

	FieldModule getFieldModule()
	{
		return FieldModule(cmzn_region_get_field_module(id));
	}

	int readFile(const char *fileName)
	{
		return cmzn_region_read_file(id, fileName);
	}

	char *getName()
	{
		return cmzn_region_get_name(id);
	}

	int setName(const char *name)
	{
		return cmzn_region_set_name(id, name);
	}

	Region getParent()
	{
		return Region(cmzn_region_get_parent(id));
	}

	Region getFirstChild()
	{
		return Region(cmzn_region_get_first_child(id));
	}

	Region getNextSibling()
	{
		return Region(cmzn_region_get_next_sibling(id));
	}

	Region getPreviousSibling()
	{
		return Region(cmzn_region_get_previous_sibling(id));
	}

	int appendChild(Region newChild)
	{
		return cmzn_region_append_child(id, newChild.id);
	}

	int insertChildBefore(Region& newChild, Region& refChild)
	{
		return cmzn_region_insert_child_before(id, newChild.id, refChild.id);
	}

	int removeChild(Region& oldChild)
	{
		return cmzn_region_remove_child(id, oldChild.id);
	}

	Region findChildByName(const char *name)
	{
		return Region(cmzn_region_find_child_by_name(id, name));
	}

	Region findSubregionAtPath(const char *path)
	{
		return Region(cmzn_region_find_subregion_at_path(id, path));
	}

	int containsSubregion(Region& subregion)
	{
		return cmzn_region_contains_subregion(id, subregion.id);
	}

	int read(StreamInformation& streamInformation)
	{
		return cmzn_region_read(id, streamInformation.getId());
	}

	int write(StreamInformation& streamInformation)
	{
		return cmzn_region_write(id, streamInformation.getId());
	}

	int writeFile(const char *fileName)
	{
		return cmzn_region_write_file(id, fileName);
	}

	StreamInformationRegion createStreamInformation();

};

class StreamInformationRegion : public StreamInformation
{
	// takes ownership of C-style reference
	explicit StreamInformationRegion(StreamInformation& streamInformation) :
		StreamInformation(streamInformation)
	{  }

	friend StreamInformationRegion Region::createStreamInformation();

public:

	// takes ownership of C handle, responsibility for destroying it
	explicit StreamInformationRegion(cmzn_stream_information_region_id stream_information_region_id) :
		StreamInformation(reinterpret_cast<cmzn_stream_information_id>(stream_information_region_id))
	{ }

	enum RegionAttribute
	{
		REGION_ATTRIBUTE_INVALID = CMZN_STREAM_INFORMATION_REGION_ATTRIBUTE_INVALID ,
		REGION_ATTRIBUTE_TIME = CMZN_STREAM_INFORMATION_REGION_ATTRIBUTE_TIME,
	};

	int hasRegionAttribute(RegionAttribute attribute)
	{
		return cmzn_stream_information_region_has_attribute(
			reinterpret_cast<cmzn_stream_information_region_id>(id),
			static_cast<cmzn_stream_information_region_attribute>(attribute));
	}

	double getRegionAttributeReal(RegionAttribute attribute)
	{
		return cmzn_stream_information_region_get_attribute_real(
			reinterpret_cast<cmzn_stream_information_region_id>(id),
			static_cast<cmzn_stream_information_region_attribute>(attribute));
	}

	int setRegionAttributeReal(RegionAttribute attribute, double value)
	{
		return cmzn_stream_information_region_set_attribute_real(
			reinterpret_cast<cmzn_stream_information_region_id>(id),
			static_cast<cmzn_stream_information_region_attribute>(attribute), value);
	}

	int hasRegionResourceAttribute(StreamResource resource, RegionAttribute attribute)
	{
		return cmzn_stream_information_region_has_resource_attribute(
			reinterpret_cast<cmzn_stream_information_region_id>(id), resource.getId(),
			static_cast<cmzn_stream_information_region_attribute>(attribute));
	}

	double getRegionResourceAttributeReal(StreamResource resource,
		RegionAttribute attribute)
	{
		return cmzn_stream_information_region_get_resource_attribute_real(
			reinterpret_cast<cmzn_stream_information_region_id>(id), resource.getId(),
			static_cast<cmzn_stream_information_region_attribute>(attribute));
	}

	int setRegionResourceAttributeReal(StreamResource resource,
		RegionAttribute attribute, double value)
	{
		return cmzn_stream_information_region_set_resource_attribute_real(
			reinterpret_cast<cmzn_stream_information_region_id>(id), resource.getId(),
			static_cast<cmzn_stream_information_region_attribute>(attribute), value);
	}

	int getResourceDomainType(StreamResource resource)
	{
		return static_cast<int>(
			cmzn_stream_information_region_get_resource_domain_type(
				reinterpret_cast<cmzn_stream_information_region_id>(id), resource.getId()));
	}

	int setResourceDomainType(StreamResource resource, int domainType)
	{
		return cmzn_stream_information_region_set_resource_domain_type(
			reinterpret_cast<cmzn_stream_information_region_id>(id), resource.getId(),
			domainType);
	}

};

inline StreamInformationRegion Region::createStreamInformation()
{
	return StreamInformationRegion(reinterpret_cast<cmzn_stream_information_region_id>(
		cmzn_region_create_stream_information(id)));
}

}  // namespace zinc

#endif
