/**
 * @file streamregion.hpp
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_STREAMREGION_HPP__
#define CMZN_STREAMREGION_HPP__

#include "zinc/streamregion.h"
#include "zinc/field.hpp"
#include "zinc/region.hpp"
#include "zinc/stream.hpp"

namespace OpenCMISS
{
namespace Zinc
{

class StreaminformationRegion : public Streaminformation
{
public:
	StreaminformationRegion() : Streaminformation()
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit StreaminformationRegion(cmzn_streaminformation_region_id streaminformation_region_id) :
		Streaminformation(reinterpret_cast<cmzn_streaminformation_id>(streaminformation_region_id))
	{ }

	bool isValid() const
	{
		return (0 != id);
	}

	inline cmzn_streaminformation_region_id getDerivedId() const
	{
		return reinterpret_cast<cmzn_streaminformation_region_id>(this->id);
	}

	enum Attribute
	{
		ATTRIBUTE_INVALID = CMZN_STREAMINFORMATION_REGION_ATTRIBUTE_INVALID,
		ATTRIBUTE_TIME = CMZN_STREAMINFORMATION_REGION_ATTRIBUTE_TIME
	};

	enum FileFormat
	{
		FILE_FORMAT_INVALID = CMZN_STREAMINFORMATION_REGION_FILE_FORMAT_INVALID,
		FILE_FORMAT_AUTOMATIC = CMZN_STREAMINFORMATION_REGION_FILE_FORMAT_AUTOMATIC,
		FILE_FORMAT_EX = CMZN_STREAMINFORMATION_REGION_FILE_FORMAT_EX,
		FILE_FORMAT_FIELDML = CMZN_STREAMINFORMATION_REGION_FILE_FORMAT_FIELDML
	};

	int hasAttribute(Attribute attribute)
	{
		return cmzn_streaminformation_region_has_attribute(getDerivedId(),
			static_cast<cmzn_streaminformation_region_attribute>(attribute));
	}

	double getAttributeReal(Attribute attribute)
	{
		return cmzn_streaminformation_region_get_attribute_real(getDerivedId(),
			static_cast<cmzn_streaminformation_region_attribute>(attribute));
	}

	int setAttributeReal(Attribute attribute, double value)
	{
		return cmzn_streaminformation_region_set_attribute_real(getDerivedId(),
			static_cast<cmzn_streaminformation_region_attribute>(attribute), value);
	}

	int hasResourceAttribute(const Streamresource& resource, Attribute attribute)
	{
		return cmzn_streaminformation_region_has_resource_attribute(
			getDerivedId(), resource.getId(),
			static_cast<cmzn_streaminformation_region_attribute>(attribute));
	}

	double getResourceAttributeReal(const Streamresource& resource,
		Attribute attribute)
	{
		return cmzn_streaminformation_region_get_resource_attribute_real(
			getDerivedId(), resource.getId(),
			static_cast<cmzn_streaminformation_region_attribute>(attribute));
	}

	int setResourceAttributeReal(const Streamresource& resource,
		Attribute attribute, double value)
	{
		return cmzn_streaminformation_region_set_resource_attribute_real(
			getDerivedId(), resource.getId(),
			static_cast<cmzn_streaminformation_region_attribute>(attribute), value);
	}

	FileFormat getFileFormat()
	{
		return static_cast<FileFormat>(
			cmzn_streaminformation_region_get_file_format(getDerivedId()));
	}

	int setFileFormat(FileFormat fileFormat)
	{
		return cmzn_streaminformation_region_set_file_format(getDerivedId(),
			static_cast<cmzn_streaminformation_region_file_format>(fileFormat));
	}

	Field::DomainTypes getResourceDomainTypes(const Streamresource& resource)
	{
		return static_cast<Field::DomainTypes>(
			cmzn_streaminformation_region_get_resource_domain_types(
				getDerivedId(), resource.getId()));
	}

	int setResourceDomainTypes(const Streamresource& resource, Field::DomainTypes domainTypes)
	{
		return cmzn_streaminformation_region_set_resource_domain_types(
			getDerivedId(), resource.getId(),
			static_cast<cmzn_field_domain_types>(domainTypes));
	}

};

inline StreaminformationRegion Streaminformation::castRegion()
{
	return StreaminformationRegion(cmzn_streaminformation_cast_region(id));
}

inline StreaminformationRegion Region::createStreaminformationRegion()
{
	return StreaminformationRegion(reinterpret_cast<cmzn_streaminformation_region_id>(
		cmzn_region_create_streaminformation_region(id)));
}

inline int Region::read(const StreaminformationRegion& streaminformationRegion)
{
	return cmzn_region_read(id, streaminformationRegion.getDerivedId());
}

inline int Region::write(const StreaminformationRegion& streaminformationRegion)
{
	return cmzn_region_write(id, streaminformationRegion.getDerivedId());
}

}  // namespace Zinc
}

#endif
