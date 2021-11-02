/**
 * @file fieldimage.hpp
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_FIELDIMAGE_HPP__
#define CMZN_FIELDIMAGE_HPP__

#include "opencmiss/zinc/fieldimage.h"
#include "opencmiss/zinc/field.hpp"
#include "opencmiss/zinc/fieldmodule.hpp"

namespace OpenCMISS
{
namespace Zinc
{

class StreaminformationImage;

class FieldImage : public Field
{
	inline cmzn_field_image_id getDerivedId() const
	{
		return reinterpret_cast<cmzn_field_image_id>(id);
	}

public:

	FieldImage() : Field()
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit FieldImage(cmzn_field_image_id field_image_id) :
		Field(reinterpret_cast<cmzn_field_id>(field_image_id))
	{	}

	enum CombineMode
	{
		COMBINE_MODE_INVALID = CMZN_FIELD_IMAGE_COMBINE_MODE_INVALID,
		COMBINE_MODE_BLEND = CMZN_FIELD_IMAGE_COMBINE_MODE_BLEND,
		COMBINE_MODE_DECAL = CMZN_FIELD_IMAGE_COMBINE_MODE_DECAL,
			/*!< default CombineMode */
		COMBINE_MODE_MODULATE = CMZN_FIELD_IMAGE_COMBINE_MODE_MODULATE,
		COMBINE_MODE_ADD = CMZN_FIELD_IMAGE_COMBINE_MODE_ADD,
		COMBINE_MODE_ADD_SIGNED = CMZN_FIELD_IMAGE_COMBINE_MODE_ADD_SIGNED,
		COMBINE_MODE_MODULATE_SCALE_4 = CMZN_FIELD_IMAGE_COMBINE_MODE_MODULATE_SCALE_4,
		COMBINE_MODE_BLEND_SCALE_4 = CMZN_FIELD_IMAGE_COMBINE_MODE_BLEND_SCALE_4,
		COMBINE_MODE_SUBTRACT = CMZN_FIELD_IMAGE_COMBINE_MODE_SUBTRACT,
		COMBINE_MODE_ADD_SCALE_4 = CMZN_FIELD_IMAGE_COMBINE_MODE_ADD_SCALE_4,
		COMBINE_MODE_SUBTRACT_SCALE_4 = CMZN_FIELD_IMAGE_COMBINE_MODE_SUBTRACT_SCALE_4,
		COMBINE_MODE_INVERT_ADD_SCALE_4 = CMZN_FIELD_IMAGE_COMBINE_MODE_INVERT_ADD_SCALE_4,
		COMBINE_MODE_INVERT_SUBTRACT_SCALE_4 = CMZN_FIELD_IMAGE_COMBINE_MODE_INVERT_SUBTRACT_SCALE_4
	};

	enum FilterMode
	{
		FILTER_MODE_INVALID = CMZN_FIELD_IMAGE_FILTER_MODE_INVALID,
		FILTER_MODE_NEAREST = CMZN_FIELD_IMAGE_FILTER_MODE_NEAREST,
			/*!< default FilterMode */
		FILTER_MODE_LINEAR = CMZN_FIELD_IMAGE_FILTER_MODE_LINEAR,
		FILTER_MODE_NEAREST_MIPMAP_NEAREST = CMZN_FIELD_IMAGE_FILTER_MODE_NEAREST_MIPMAP_NEAREST,
		FILTER_MODE_LINEAR_MIPMAP_NEAREST = CMZN_FIELD_IMAGE_FILTER_MODE_LINEAR_MIPMAP_NEAREST,
		FILTER_MODE_LINEAR_MIPMAP_LINEAR = CMZN_FIELD_IMAGE_FILTER_MODE_LINEAR_MIPMAP_LINEAR
	};

	enum HardwareCompressionMode
	{
		HARDWARE_COMPRESSION_MODE_INVALID = CMZN_FIELD_IMAGE_HARDWARE_COMPRESSION_MODE_INVALID,
		HARDWARE_COMPRESSION_MODE_UNCOMPRESSED = CMZN_FIELD_IMAGE_HARDWARE_COMPRESSION_MODE_UNCOMPRESSED,
			/*!< default HardwareCompressionMode */
		HARDWARE_COMPRESSION_MODE_AUTOMATIC = CMZN_FIELD_IMAGE_HARDWARE_COMPRESSION_MODE_AUTOMATIC
			/*!< Allow the hardware to choose the compression */
	};

	enum PixelFormat
	{
		PIXEL_FORMAT_INVALID = CMZN_FIELD_IMAGE_PIXEL_FORMAT_INVALID,
		PIXEL_FORMAT_LUMINANCE = CMZN_FIELD_IMAGE_PIXEL_FORMAT_LUMINANCE,
		PIXEL_FORMAT_LUMINANCE_ALPHA = CMZN_FIELD_IMAGE_PIXEL_FORMAT_LUMINANCE_ALPHA,
		PIXEL_FORMAT_RGB = CMZN_FIELD_IMAGE_PIXEL_FORMAT_RGB,
		PIXEL_FORMAT_RGBA = CMZN_FIELD_IMAGE_PIXEL_FORMAT_RGBA,
		PIXEL_FORMAT_ABGR = CMZN_FIELD_IMAGE_PIXEL_FORMAT_ABGR,
		PIXEL_FORMAT_BGR = CMZN_FIELD_IMAGE_PIXEL_FORMAT_BGR
	};

	enum WrapMode
	{
		WRAP_MODE_INVALID = CMZN_FIELD_IMAGE_WRAP_MODE_INVALID,
		WRAP_MODE_CLAMP = CMZN_FIELD_IMAGE_WRAP_MODE_CLAMP,
		WRAP_MODE_REPEAT = CMZN_FIELD_IMAGE_WRAP_MODE_REPEAT,
			/*!< default WrapMode */
		WRAP_MODE_EDGE_CLAMP = CMZN_FIELD_IMAGE_WRAP_MODE_EDGE_CLAMP,
		WRAP_MODE_BORDER_CLAMP= CMZN_FIELD_IMAGE_WRAP_MODE_BORDER_CLAMP,
		WRAP_MODE_MIRROR_REPEAT = CMZN_FIELD_IMAGE_WRAP_MODE_MIRROR_REPEAT
	};

	int getWidthInPixels() const
	{
		return cmzn_field_image_get_width_in_pixels(getDerivedId());
	}

	int getHeightInPixels() const
	{
		return cmzn_field_image_get_height_in_pixels(getDerivedId());
	}

	int getDepthInPixels() const
	{
		return cmzn_field_image_get_depth_in_pixels(getDerivedId());
	}

	int getSizeInPixels(int valuesCount, int *valuesOut) const
	{
		return cmzn_field_image_get_size_in_pixels(getDerivedId(), valuesCount, valuesOut);
	}

	int setSizeInPixels(int valuesCount, const int *valuesIn)
	{
		return cmzn_field_image_set_size_in_pixels(getDerivedId(), valuesCount, valuesIn);
	}

	double getTextureCoordinateWidth() const
	{
		return cmzn_field_image_get_texture_coordinate_width(getDerivedId());
	}

	double getTextureCoordinateHeight() const
	{
		return cmzn_field_image_get_texture_coordinate_height(getDerivedId());
	}

	double getTextureCoordinateDepth() const
	{
		return cmzn_field_image_get_texture_coordinate_depth(getDerivedId());
	}

	int getTextureCoordinateSizes(int valuesCount, double *valuesOut) const
	{
		return cmzn_field_image_get_texture_coordinate_sizes(getDerivedId(), valuesCount,
			valuesOut);
	}

	int setTextureCoordinateWidth(double width)
	{
		return cmzn_field_image_set_texture_coordinate_width(getDerivedId(), width);
	}

	int setTextureCoordinateHeight(double height)
	{
		return cmzn_field_image_set_texture_coordinate_height(getDerivedId(), height);
	}

	int setTextureCoordinateDepth(double depth)
	{
		return cmzn_field_image_set_texture_coordinate_depth(getDerivedId(), depth);
	}

	int setTextureCoordinateSizes(int valuesCount, const double *valuesIn)
	{
		return cmzn_field_image_set_texture_coordinate_sizes(getDerivedId(),
			valuesCount, valuesIn);
	}

	inline int read(const StreaminformationImage& streaminformationImage);

	int readFile(const char *fileName)
	{
		return cmzn_field_image_read_file(getDerivedId(), fileName);
	}

	inline int write(const StreaminformationImage& streaminformationImage) const;

	CombineMode getCombineMode() const
	{
		return static_cast<CombineMode>(cmzn_field_image_get_combine_mode(getDerivedId()));
	}

	int setCombineMode(CombineMode combineMode)
	{
		return cmzn_field_image_set_combine_mode(getDerivedId(),
			static_cast<cmzn_field_image_combine_mode>(combineMode));
	}

	Field getDomainField() const
	{
		return Field(cmzn_field_image_get_domain_field(getDerivedId()));
	}

	int setDomainField(const Field& domainField)
	{
		return cmzn_field_image_set_domain_field(getDerivedId(), domainField.getId());
	}

	HardwareCompressionMode getHardwareCompressionMode() const
	{
		return static_cast<HardwareCompressionMode>(
			cmzn_field_image_get_hardware_compression_mode(getDerivedId()));
	}

	int setHardwareCompressionMode(HardwareCompressionMode hardwareCompressionMode)
	{
		return cmzn_field_image_set_hardware_compression_mode(getDerivedId(),
			static_cast<cmzn_field_image_hardware_compression_mode>(hardwareCompressionMode));
	}

	FilterMode getFilterMode() const
	{
		return static_cast<FilterMode>(cmzn_field_image_get_filter_mode(getDerivedId()));
	}

	int setFilterMode(FilterMode filterMode)
	{
		return cmzn_field_image_set_filter_mode(getDerivedId(),
			static_cast<cmzn_field_image_filter_mode>(filterMode));
	}

	WrapMode getWrapMode() const
	{
		return static_cast<WrapMode>(cmzn_field_image_get_wrap_mode(getDerivedId()));
	}

	int setWrapMode(WrapMode wrapMode)
	{
		return cmzn_field_image_set_wrap_mode(getDerivedId(),
			static_cast<cmzn_field_image_wrap_mode>(wrapMode));
	}

	char *getProperty(const char* property) const
	{
		return cmzn_field_image_get_property(getDerivedId(), property);
	}

	int getNumberOfBitsPerComponent() const
	{
		return cmzn_field_image_get_number_of_bits_per_component(
			getDerivedId());
	}

	int setNumberOfBitsPerComponent(int numberOfBits)
	{
		return cmzn_field_image_set_number_of_bits_per_component(
			getDerivedId(), numberOfBits);
	}

	PixelFormat getPixelFormat() const
	{
		return static_cast<PixelFormat>(cmzn_field_image_get_pixel_format(getDerivedId()));
	}

	int setPixelFormat(PixelFormat imagePixelFormat)
	{
		return cmzn_field_image_set_pixel_format(getDerivedId(),
			static_cast<cmzn_field_image_pixel_format>(imagePixelFormat));
	}

	int getBuffer(const void **buffer_out, unsigned int *buffer_length_out) const
	{
		return cmzn_field_image_get_buffer(getDerivedId(), buffer_out,
			buffer_length_out);
	}

	int setBuffer(const void *buffer, unsigned int buffer_length)
	{
		return cmzn_field_image_set_buffer(getDerivedId(), buffer, buffer_length);
	}

	inline StreaminformationImage createStreaminformationImage();

};

inline FieldImage Fieldmodule::createFieldImage()
{
	return FieldImage(reinterpret_cast<cmzn_field_image_id>(
		cmzn_fieldmodule_create_field_image(id)));
}

inline FieldImage Fieldmodule::createFieldImageFromSource(const Field& sourceField)
{
	return FieldImage(reinterpret_cast<cmzn_field_image_id>(
		cmzn_fieldmodule_create_field_image_from_source(id, sourceField.getId())));
}

inline FieldImage Field::castImage()
{
	return FieldImage(cmzn_field_cast_image(id));
}

} // namespace Zinc
}
#endif
