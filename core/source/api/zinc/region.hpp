/**
 * FILE : region.hpp
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_REGION_HPP__
#define CMZN_REGION_HPP__

#include "zinc/region.h"
#include "zinc/fieldmodule.hpp"
#include "zinc/stream.hpp"

namespace OpenCMISS
{
namespace Zinc
{

class StreaminformationRegion : public Streaminformation
{
	// takes ownership of C-style reference
	explicit StreaminformationRegion(Streaminformation& streamInformation) :
		Streaminformation(streamInformation)
	{  }

public:

	// takes ownership of C handle, responsibility for destroying it
	explicit StreaminformationRegion(cmzn_streaminformation_region_id streaminformation_region_id) :
		Streaminformation(reinterpret_cast<cmzn_streaminformation_id>(streaminformation_region_id))
	{ }

	bool isValid()
	{
		return (0 != reinterpret_cast<cmzn_streaminformation_region_id>(id));
	}

	cmzn_streaminformation_region_id getId()
	{
		return reinterpret_cast<cmzn_streaminformation_region_id>(id);
	}

	enum RegionAttribute
	{
		REGION_ATTRIBUTE_INVALID = CMZN_STREAMINFORMATION_REGION_ATTRIBUTE_INVALID ,
		REGION_ATTRIBUTE_TIME = CMZN_STREAMINFORMATION_REGION_ATTRIBUTE_TIME,
	};

	int hasRegionAttribute(RegionAttribute attribute)
	{
		return cmzn_streaminformation_region_has_attribute(
			reinterpret_cast<cmzn_streaminformation_region_id>(id),
			static_cast<cmzn_streaminformation_region_attribute>(attribute));
	}

	double getRegionAttributeReal(RegionAttribute attribute)
	{
		return cmzn_streaminformation_region_get_attribute_real(
			reinterpret_cast<cmzn_streaminformation_region_id>(id),
			static_cast<cmzn_streaminformation_region_attribute>(attribute));
	}

	int setRegionAttributeReal(RegionAttribute attribute, double value)
	{
		return cmzn_streaminformation_region_set_attribute_real(
			reinterpret_cast<cmzn_streaminformation_region_id>(id),
			static_cast<cmzn_streaminformation_region_attribute>(attribute), value);
	}

	int hasRegionResourceAttribute(Streamresource resource, RegionAttribute attribute)
	{
		return cmzn_streaminformation_region_has_resource_attribute(
			reinterpret_cast<cmzn_streaminformation_region_id>(id), resource.getId(),
			static_cast<cmzn_streaminformation_region_attribute>(attribute));
	}

	double getRegionResourceAttributeReal(Streamresource resource,
		RegionAttribute attribute)
	{
		return cmzn_streaminformation_region_get_resource_attribute_real(
			reinterpret_cast<cmzn_streaminformation_region_id>(id), resource.getId(),
			static_cast<cmzn_streaminformation_region_attribute>(attribute));
	}

	int setRegionResourceAttributeReal(Streamresource resource,
		RegionAttribute attribute, double value)
	{
		return cmzn_streaminformation_region_set_resource_attribute_real(
			reinterpret_cast<cmzn_streaminformation_region_id>(id), resource.getId(),
			static_cast<cmzn_streaminformation_region_attribute>(attribute), value);
	}

	Field::DomainTypes getResourceDomainTypes(Streamresource resource)
	{
		return static_cast<Field::DomainTypes>(
			cmzn_streaminformation_region_get_resource_domain_types(
				reinterpret_cast<cmzn_streaminformation_region_id>(id), resource.getId()));
	}

	int setResourceDomainTypes(Streamresource resource, Field::DomainTypes domainTypes)
	{
		return cmzn_streaminformation_region_set_resource_domain_types(
			reinterpret_cast<cmzn_streaminformation_region_id>(id), resource.getId(),
			static_cast<cmzn_field_domain_types>(domainTypes));
	}

};

class Scene;

class Region
{
friend bool operator==(const Region& a, const Region& b);

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

	Fieldmodule getFieldmodule()
	{
		return Fieldmodule(cmzn_region_get_fieldmodule(id));
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

	bool containsSubregion(Region& subregion)
	{
		return cmzn_region_contains_subregion(id, subregion.id);
	}

	int read(StreaminformationRegion& streaminformationRegion)
	{
		return cmzn_region_read(id, streaminformationRegion.getId());
	}

	int write(StreaminformationRegion& streaminformationRegion)
	{
		return cmzn_region_write(id, streaminformationRegion.getId());
	}

	int writeFile(const char *fileName)
	{
		return cmzn_region_write_file(id, fileName);
	}

	Scene getScene();

	StreaminformationRegion createStreaminformation()
	{
		return StreaminformationRegion(reinterpret_cast<cmzn_streaminformation_region_id>(
			cmzn_region_create_streaminformation(id)));
	}

};

inline bool operator==(const Region& a, const Region& b)
{
	return a.id == b.id;
}

}  // namespace Zinc
}

#endif
