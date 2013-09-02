/***************************************************************************//**
 * FILE : fieldimage.hpp
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
#ifndef CMZN_FIELDIMAGE_HPP__
#define CMZN_FIELDIMAGE_HPP__

#include "zinc/fieldimage.h"
#include "zinc/field.hpp"
#include "zinc/fieldmodule.hpp"
#include "zinc/stream.hpp"


namespace OpenCMISS
{
namespace Zinc
{
class StreamInformationImage;

class FieldImage : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldImage(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldImage FieldModule::createImage();
	friend FieldImage FieldModule::createImageWithDomain(Field& domain_field);
	friend FieldImage FieldModule::createImageFromSource(Field& domain_field, Field& source_field);

public:

	FieldImage() : Field(0)
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit FieldImage(cmzn_field_image_id field_image_id) :
		Field(reinterpret_cast<cmzn_field_id>(field_image_id))
	{	}

	// casting constructor: must check isValid()
	FieldImage(Field& field) :
		Field(reinterpret_cast<cmzn_field_id>(cmzn_field_cast_image(field.getId())))
	{	}

	enum CombineMode
	{
		COMBINE_INVALID = CMZN_FIELD_IMAGE_COMBINE_INVALID,
		COMBINE_BLEND = CMZN_FIELD_IMAGE_COMBINE_BLEND,
		COMBINE_DECAL = CMZN_FIELD_IMAGE_COMBINE_DECAL,
		COMBINE_MODULATE = CMZN_FIELD_IMAGE_COMBINE_MODULATE,
		COMBINE_ADD = CMZN_FIELD_IMAGE_COMBINE_ADD,
		COMBINE_ADD_SIGNED = CMZN_FIELD_IMAGE_COMBINE_ADD_SIGNED,
		COMBINE_MODULATE_SCALE_4 = CMZN_FIELD_IMAGE_COMBINE_MODULATE_SCALE_4,
		COMBINE_BLEND_SCALE_4 = CMZN_FIELD_IMAGE_COMBINE_BLEND_SCALE_4,
		COMBINE_SUBTRACT = CMZN_FIELD_IMAGE_COMBINE_SUBTRACT,
		COMBINE_ADD_SCALE_4 = CMZN_FIELD_IMAGE_COMBINE_ADD_SCALE_4,
		COMBINE_SUBTRACT_SCALE_4 = CMZN_FIELD_IMAGE_COMBINE_SUBTRACT_SCALE_4,
		COMBINE_INVERT_ADD_SCALE_4 = CMZN_FIELD_IMAGE_COMBINE_INVERT_ADD_SCALE_4,
		COMBINE_INVERT_SUBTRACT_SCALE_4 = CMZN_FIELD_IMAGE_COMBINE_INVERT_SUBTRACT_SCALE_4
	};

	enum FilterMode
	{
		FILTER_INVALID = CMZN_FIELD_IMAGE_FILTER_INVALID,
		FILTER_NEAREST = CMZN_FIELD_IMAGE_FILTER_NEAREST,
		FILTER_LINEAR = CMZN_FIELD_IMAGE_FILTER_LINEAR,
		FILTER_NEAREST_MIPMAP_NEAREST = CMZN_FIELD_IMAGE_FILTER_NEAREST_MIPMAP_NEAREST,
		FILTER_LINEAR_MIPMAP_NEAREST = CMZN_FIELD_IMAGE_FILTER_LINEAR_MIPMAP_NEAREST,
		FILTER_LINEAR_MIPMAP_LINEAR = CMZN_FIELD_IMAGE_FILTER_LINEAR_MIPMAP_LINEAR
	};

	enum HardwareCompressionMode
	{
		HARDWARE_COMPRESSION_MODE_INVALID = CMZN_FIELD_IMAGE_HARDWARE_COMPRESSION_MODE_INVALID,
		HARDWARE_COMPRESSION_MODE_UNCOMPRESSED = CMZN_FIELD_IMAGE_HARDWARE_COMPRESSION_MODE_UNCOMPRESSED,
		HARDWARE_COMPRESSION_MODE_AUTOMATIC = CMZN_FIELD_IMAGE_HARDWARE_COMPRESSION_MODE_AUTOMATIC
		/*!< Allow the hardware to choose the compression */
	};

	enum WrapMode
	{
		WRAP_INVALID = CMZN_FIELD_IMAGE_WRAP_INVALID,
		WRAP_CLAMP = CMZN_FIELD_IMAGE_WRAP_CLAMP,
		WRAP_REPEAT = CMZN_FIELD_IMAGE_WRAP_REPEAT,
		WRAP_EDGE_CLAMP = CMZN_FIELD_IMAGE_WRAP_EDGE_CLAMP,
		WRAP_BORDER_CLAMP= CMZN_FIELD_IMAGE_WRAP_BORDER_CLAMP,
		WRAP_MIRROR_REPEAT = CMZN_FIELD_IMAGE_WRAP_MIRROR_REPEAT
		/*!< Allow the hardware to choose the wrap mode for texture */
	};

	enum ImageAttribute
	{
		IMAGE_ATTRIBUTE_INVALID = CMZN_FIELD_IMAGE_ATTRIBUTE_INVALID,
		IMAGE_ATTRIBUTE_RAW_WIDTH_PIXELS = CMZN_FIELD_IMAGE_ATTRIBUTE_RAW_WIDTH_PIXELS,
		IMAGE_ATTRIBUTE_RAW_HEIGHT_PIXELS = CMZN_FIELD_IMAGE_ATTRIBUTE_RAW_HEIGHT_PIXELS,
		IMAGE_ATTRIBUTE_RAW_DEPTH_PIXELS = CMZN_FIELD_IMAGE_ATTRIBUTE_RAW_DEPTH_PIXELS,
		IMAGE_ATTRIBUTE_PHYSICAL_WIDTH_PIXELS = CMZN_FIELD_IMAGE_ATTRIBUTE_PHYSICAL_WIDTH_PIXELS,
		IMAGE_ATTRIBUTE_PHYSICAL_HEIGHT_PIXELS = CMZN_FIELD_IMAGE_ATTRIBUTE_PHYSICAL_HEIGHT_PIXELS,
		IMAGE_ATTRIBUTE_PHYSICAL_DEPTH_PIXELS = CMZN_FIELD_IMAGE_ATTRIBUTE_PHYSICAL_DEPTH_PIXELS
	};

	int getAttributeInteger(ImageAttribute imageAttribute)
	{
		return cmzn_field_image_get_attribute_integer(
			reinterpret_cast<cmzn_field_image_id>(id),
			static_cast<cmzn_field_image_attribute>(imageAttribute));
	}

	double getAttributeReal(ImageAttribute imageAttribute)
	{
		return cmzn_field_image_get_attribute_real(
			reinterpret_cast<cmzn_field_image_id>(id),
			static_cast<cmzn_field_image_attribute>(imageAttribute));
	}

	int setAttributeReal(ImageAttribute imageAttribute, double value)
	{
		return cmzn_field_image_set_attribute_real(
			reinterpret_cast<cmzn_field_image_id>(id),
			static_cast<cmzn_field_image_attribute>(imageAttribute), value);
	}

	int read(StreamInformation& streamInformation)
	{
		return cmzn_field_image_read(reinterpret_cast<cmzn_field_image_id>(id),
			streamInformation.getId());
	}

	int write(StreamInformation& streamInformation)
	{
		return cmzn_field_image_write(reinterpret_cast<cmzn_field_image_id>(id),
			streamInformation.getId());
	}

	CombineMode getCombineMode()
	{
		return static_cast<CombineMode>(cmzn_field_image_get_combine_mode(
			reinterpret_cast<cmzn_field_image_id>(id)));
	}

	int setCombineMode(CombineMode combineMode)
	{
		return cmzn_field_image_set_combine_mode(
			reinterpret_cast<cmzn_field_image_id>(id),
			static_cast<cmzn_field_image_combine_mode>(combineMode));
	}

	HardwareCompressionMode getHardwareCompressionMode()
	{
		return static_cast<HardwareCompressionMode>(
			cmzn_field_image_get_hardware_compression_mode(
				reinterpret_cast<cmzn_field_image_id>(id)));
	}

	int setHardwareCompressionMode(HardwareCompressionMode hardwareCompressionMode)
	{
		return cmzn_field_image_set_hardware_compression_mode(
			reinterpret_cast<cmzn_field_image_id>(id),
			static_cast<cmzn_field_image_hardware_compression_mode>(hardwareCompressionMode));
	}

	FilterMode getFilterMode()
	{
		return static_cast<FilterMode>(cmzn_field_image_get_filter_mode(
			reinterpret_cast<cmzn_field_image_id>(id)));
	}

	int setFilterMode(FilterMode filterMode)
	{
		return cmzn_field_image_set_filter_mode(
			reinterpret_cast<cmzn_field_image_id>(id),
			static_cast<cmzn_field_image_filter_mode>(filterMode));
	}

	WrapMode getWrapMode()
	{
		return static_cast<WrapMode>(cmzn_field_image_get_wrap_mode(
			reinterpret_cast<cmzn_field_image_id>(id)));
	}

	int setWrapMode(WrapMode wrapMode)
	{
		return cmzn_field_image_set_wrap_mode(
			reinterpret_cast<cmzn_field_image_id>(id),
			static_cast<cmzn_field_image_wrap_mode>(wrapMode));
	}

	char *getProperty(const char* property)
	{
		return cmzn_field_image_get_property(
			reinterpret_cast<cmzn_field_image_id>(id), property);
	}

	StreamInformationImage createStreamInformation();

};

class StreamInformationImage : public StreamInformation
{
private:
	StreamInformationImage(StreamInformation& streamInformation) :
		StreamInformation(streamInformation)
	{ }

	friend StreamInformationImage FieldImage::createStreamInformation();

public:

	// takes ownership of C handle, responsibility for destroying it
	explicit StreamInformationImage(cmzn_stream_information_image_id stream_information_image_id) :
		StreamInformation(reinterpret_cast<cmzn_stream_information_id>(stream_information_image_id))
	{ }

	enum ImageAttribute
	{
		IMAGE_ATTRIBUTE_RAW_WIDTH_PIXELS = CMZN_STREAM_INFORMATION_IMAGE_ATTRIBUTE_RAW_WIDTH_PIXELS,
		IMAGE_ATTRIBUTE_RAW_HEIGHT_PIXELS = CMZN_STREAM_INFORMATION_IMAGE_ATTRIBUTE_RAW_HEIGHT_PIXELS,
		IMAGE_ATTRIBUTE_BITS_PER_COMPONENT = CMZN_STREAM_INFORMATION_IMAGE_ATTRIBUTE_BITS_PER_COMPONENT,
		IMAGE_ATTRIBUTE_COMPRESSION_QUALITY = CMZN_STREAM_INFORMATION_IMAGE_ATTRIBUTE_COMPRESSION_QUALITY
	};

	enum ImageFileFormat
	{
		IMAGE_FILE_FORMAT_INVALID = CMZN_STREAM_INFORMATION_IMAGE_FILE_FORMAT_INVALID,
		IMAGE_FILE_FORMAT_BMP = CMZN_STREAM_INFORMATION_IMAGE_FILE_FORMAT_BMP,
		IMAGE_FILE_FORMAT_DICOM = CMZN_STREAM_INFORMATION_IMAGE_FILE_FORMAT_DICOM,
		IMAGE_FILE_FORMAT_JPG = CMZN_STREAM_INFORMATION_IMAGE_FILE_FORMAT_JPG,
		IMAGE_FILE_FORMAT_GIF = CMZN_STREAM_INFORMATION_IMAGE_FILE_FORMAT_GIF,
		IMAGE_FILE_FORMAT_PNG = CMZN_STREAM_INFORMATION_IMAGE_FILE_FORMAT_PNG,
		IMAGE_FILE_FORMAT_SGI = CMZN_STREAM_INFORMATION_IMAGE_FILE_FORMAT_SGI,
		IMAGE_FILE_FORMAT_TIFF = CMZN_STREAM_INFORMATION_IMAGE_FILE_FORMAT_TIFF
	};

	enum ImagePixelFormat
	{
		IMAGE_PIXEL_FORMAT_INVALID = CMZN_STREAM_INFORMATION_IMAGE_PIXEL_FORMAT_INVALID,
		IMAGE_PIXEL_FORMAT_LUMINANCE = CMZN_STREAM_INFORMATION_IMAGE_PIXEL_FORMAT_LUMINANCE,
		IMAGE_PIXEL_FORMAT_LUMINANCE_ALPHA = CMZN_STREAM_INFORMATION_IMAGE_PIXEL_FORMAT_LUMINANCE_ALPHA,
		IMAGE_PIXEL_FORMAT_RGB = CMZN_STREAM_INFORMATION_IMAGE_PIXEL_FORMAT_RGB,
		IMAGE_PIXEL_FORMAT_RGBA = CMZN_STREAM_INFORMATION_IMAGE_PIXEL_FORMAT_RGBA,
		IMAGE_PIXEL_FORMAT_ABGR = CMZN_STREAM_INFORMATION_IMAGE_PIXEL_FORMAT_ABGR,
		IMAGE_PIXEL_FORMAT_BGR = CMZN_STREAM_INFORMATION_IMAGE_PIXEL_FORMAT_BGR
	};

	int setAttributeInteger(ImageAttribute imageAttribute, int value)
	{
		return cmzn_stream_information_image_set_attribute_integer(
			reinterpret_cast<cmzn_stream_information_image_id>(id),
			static_cast<cmzn_stream_information_image_attribute>(imageAttribute), value);
	}

	int setAttributeReal(ImageAttribute imageAttribute, double value)
	{
		return cmzn_stream_information_image_set_attribute_real(
			reinterpret_cast<cmzn_stream_information_image_id>(id),
			static_cast<cmzn_stream_information_image_attribute>(imageAttribute), value);
	}

	int setFileFormat(ImageFileFormat imageFileFormat)
	{
		return cmzn_stream_information_image_set_file_format(
			reinterpret_cast<cmzn_stream_information_image_id>(id),
			static_cast<cmzn_stream_information_image_file_format>(imageFileFormat));
	}

	int setPixelFormat(ImagePixelFormat imagePixelFormat)
	{
		return cmzn_stream_information_image_set_pixel_format(
			reinterpret_cast<cmzn_stream_information_image_id>(id),
			static_cast<cmzn_stream_information_image_pixel_format>(imagePixelFormat));
	}

};

inline StreamInformationImage FieldImage::createStreamInformation()
{
	return StreamInformationImage(
		reinterpret_cast<cmzn_stream_information_image_id>(
			cmzn_field_image_create_stream_information(
				reinterpret_cast<cmzn_field_image_id>(id))));
}

inline FieldImage FieldModule::createImage()
{
	return FieldImage(cmzn_field_module_create_image(id,
		0, 0));
}

inline FieldImage FieldModule::createImageWithDomain(Field& domain_field)
{
	return FieldImage(cmzn_field_module_create_image(id,
		domain_field.getId(), 0));
}

inline FieldImage FieldModule::createImageFromSource(Field& domain_field, Field& source_field)
{
	return FieldImage(cmzn_field_module_create_image(id,
		domain_field.getId(), source_field.getId()));
}

} // namespace Zinc
}
#endif
