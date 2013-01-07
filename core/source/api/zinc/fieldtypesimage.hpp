/***************************************************************************//**
 * FILE : fieldtypesimage.hpp
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
#ifndef __ZN_FIELD_TYPES_IMAGE_HPP__
#define __ZN_FIELD_TYPES_IMAGE_HPP__

#include "zinc/fieldimage.h"
#include "zinc/field.hpp"
#include "zinc/fieldmodule.hpp"
#include "zinc/stream.hpp"


namespace zinc
{
class StreamInformationImage;

class FieldImage : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldImage(Cmiss_field_id field_id) : Field(field_id)
	{	}

	friend FieldImage FieldModule::createImage();
	friend FieldImage FieldModule::createImageFromSource(Field& domain_field, Field& source_field);

public:

	FieldImage() : Field(0)
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit FieldImage(Cmiss_field_image_id field_image_id) :
		Field(reinterpret_cast<Cmiss_field_id>(field_image_id))
	{	}

	// casting constructor: must check isValid()
	FieldImage(Field& field) :
		Field(reinterpret_cast<Cmiss_field_id>(Cmiss_field_cast_image(field.getId())))
	{	}

	enum CombineMode
	{
		COMBINE_INVALID = CMISS_FIELD_IMAGE_COMBINE_INVALID,
		COMBINE_BLEND = CMISS_FIELD_IMAGE_COMBINE_BLEND,
		COMBINE_DECAL = CMISS_FIELD_IMAGE_COMBINE_DECAL,
		COMBINE_MODULATE = CMISS_FIELD_IMAGE_COMBINE_MODULATE,
		COMBINE_ADD = CMISS_FIELD_IMAGE_COMBINE_ADD,
		COMBINE_ADD_SIGNED = CMISS_FIELD_IMAGE_COMBINE_ADD_SIGNED,
		COMBINE_MODULATE_SCALE_4 = CMISS_FIELD_IMAGE_COMBINE_MODULATE_SCALE_4,
		COMBINE_BLEND_SCALE_4 = CMISS_FIELD_IMAGE_COMBINE_BLEND_SCALE_4,
		COMBINE_SUBTRACT = CMISS_FIELD_IMAGE_COMBINE_SUBTRACT,
		COMBINE_ADD_SCALE_4 = CMISS_FIELD_IMAGE_COMBINE_ADD_SCALE_4,
		COMBINE_SUBTRACT_SCALE_4 = CMISS_FIELD_IMAGE_COMBINE_SUBTRACT_SCALE_4,
		COMBINE_INVERT_ADD_SCALE_4 = CMISS_FIELD_IMAGE_COMBINE_INVERT_ADD_SCALE_4,
		COMBINE_INVERT_SUBTRACT_SCALE_4 = CMISS_FIELD_IMAGE_COMBINE_INVERT_SUBTRACT_SCALE_4,
	};

	enum FilterMode
	{
		FILTER_INVALID = CMISS_FIELD_IMAGE_FILTER_INVALID,
		FILTER_NEAREST = CMISS_FIELD_IMAGE_FILTER_NEAREST,
		FILTER_LINEAR = CMISS_FIELD_IMAGE_FILTER_LINEAR,
		FILTER_NEAREST_MIPMAP_NEAREST = CMISS_FIELD_IMAGE_FILTER_NEAREST_MIPMAP_NEAREST,
		FILTER_LINEAR_MIPMAP_NEAREST = CMISS_FIELD_IMAGE_FILTER_LINEAR_MIPMAP_NEAREST,
		FILTER_LINEAR_MIPMAP_LINEAR = CMISS_FIELD_IMAGE_FILTER_LINEAR_MIPMAP_LINEAR,
	};

	enum HardwareCompressionMode
	{
		HARDWARE_COMPRESSION_MODE_INVALID = CMISS_FIELD_IMAGE_HARDWARE_COMPRESSION_MODE_INVALID,
		HARDWARE_COMPRESSION_MODE_UNCOMPRESSED = CMISS_FIELD_IMAGE_HARDWARE_COMPRESSION_MODE_UNCOMPRESSED,
		HARDWARE_COMPRESSION_MODE_AUTOMATIC = CMISS_FIELD_IMAGE_HARDWARE_COMPRESSION_MODE_AUTOMATIC,
		/*!< Allow the hardware to choose the compression */
	};

	enum ImageAttribute
	{
		IMAGE_ATTRIBUTE_INVALID = CMISS_FIELD_IMAGE_ATTRIBUTE_INVALID,
		IMAGE_ATTRIBUTE_RAW_WIDTH_PIXELS = CMISS_FIELD_IMAGE_ATTRIBUTE_RAW_WIDTH_PIXELS,
		IMAGE_ATTRIBUTE_RAW_HEIGHT_PIXELS = CMISS_FIELD_IMAGE_ATTRIBUTE_RAW_HEIGHT_PIXELS,
		IMAGE_ATTRIBUTE_RAW_DEPTH_PIXELS = CMISS_FIELD_IMAGE_ATTRIBUTE_RAW_DEPTH_PIXELS,
		IMAGE_ATTRIBUTE_PHYSICAL_WIDTH_PIXELS = CMISS_FIELD_IMAGE_ATTRIBUTE_PHYSICAL_WIDTH_PIXELS,
		IMAGE_ATTRIBUTE_PHYSICAL_HEIGHT_PIXELS = CMISS_FIELD_IMAGE_ATTRIBUTE_PHYSICAL_HEIGHT_PIXELS,
		IMAGE_ATTRIBUTE_PHYSICAL_DEPTH_PIXELS = CMISS_FIELD_IMAGE_ATTRIBUTE_PHYSICAL_DEPTH_PIXELS,
	};

	int getAttributeInteger(ImageAttribute imageAttribute)
	{
		return Cmiss_field_image_get_attribute_integer(
			reinterpret_cast<Cmiss_field_image_id>(id),
			static_cast<Cmiss_field_image_attribute>(imageAttribute));
	}

	double getAttributeReal(ImageAttribute imageAttribute)
	{
		return Cmiss_field_image_get_attribute_real(
			reinterpret_cast<Cmiss_field_image_id>(id),
			static_cast<Cmiss_field_image_attribute>(imageAttribute));
	}

	int setAttributeReal(ImageAttribute imageAttribute, double value)
	{
		return Cmiss_field_image_set_attribute_real(
			reinterpret_cast<Cmiss_field_image_id>(id),
			static_cast<Cmiss_field_image_attribute>(imageAttribute), value);
	}

	int read(StreamInformation& streamInformation)
	{
		return Cmiss_field_image_read(reinterpret_cast<Cmiss_field_image_id>(id),
			streamInformation.getId());
	}

	int write(StreamInformation& streamInformation)
	{
		return Cmiss_field_image_write(reinterpret_cast<Cmiss_field_image_id>(id),
			streamInformation.getId());
	}

	CombineMode getCombineMode()
	{
		return static_cast<CombineMode>(Cmiss_field_image_get_combine_mode(
			reinterpret_cast<Cmiss_field_image_id>(id)));
	}

	int setCombineMode(CombineMode combineMode)
	{
		return Cmiss_field_image_set_combine_mode(
			reinterpret_cast<Cmiss_field_image_id>(id),
			static_cast<Cmiss_field_image_combine_mode>(combineMode));
	}

	HardwareCompressionMode getHardwareCompressionMode()
	{
		return static_cast<HardwareCompressionMode>(
			Cmiss_field_image_get_hardware_compression_mode(
				reinterpret_cast<Cmiss_field_image_id>(id)));
	}

	int setHardwareCompressionMode(HardwareCompressionMode hardwareCompressionMode)
	{
		return Cmiss_field_image_set_hardware_compression_mode(
			reinterpret_cast<Cmiss_field_image_id>(id),
			static_cast<Cmiss_field_image_hardware_compression_mode>(hardwareCompressionMode));
	}

	FilterMode getFilterMode()
	{
		return static_cast<FilterMode>(Cmiss_field_image_get_filter_mode(
			reinterpret_cast<Cmiss_field_image_id>(id)));
	}

	int setFilterMode(FilterMode filterMode)
	{
		return Cmiss_field_image_set_filter_mode(
			reinterpret_cast<Cmiss_field_image_id>(id),
			static_cast<Cmiss_field_image_filter_mode>(filterMode));
	}

	char *getProperty(const char* property)
	{
		return Cmiss_field_image_get_property(
			reinterpret_cast<Cmiss_field_image_id>(id), property);
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
	explicit StreamInformationImage(Cmiss_stream_information_image_id stream_information_image_id) :
		StreamInformation(reinterpret_cast<Cmiss_stream_information_id>(stream_information_image_id))
	{ }

	enum ImageAttribute
	{
		IMAGE_ATTRIBUTE_RAW_WIDTH_PIXELS = CMISS_STREAM_INFORMATION_IMAGE_ATTRIBUTE_RAW_WIDTH_PIXELS,
		IMAGE_ATTRIBUTE_RAW_HEIGHT_PIXELS = CMISS_STREAM_INFORMATION_IMAGE_ATTRIBUTE_RAW_HEIGHT_PIXELS,
		IMAGE_ATTRIBUTE_BITS_PER_COMPONENT = CMISS_STREAM_INFORMATION_IMAGE_ATTRIBUTE_BITS_PER_COMPONENT,
		IMAGE_ATTRIBUTE_COMPRESSION_QUALITY = CMISS_STREAM_INFORMATION_IMAGE_ATTRIBUTE_COMPRESSION_QUALITY,
	};

	enum ImageFileFormat
	{
		IMAGE_FILE_FORMAT_INVALID = CMISS_STREAM_INFORMATION_IMAGE_FILE_FORMAT_INVALID,
		IMAGE_FILE_FORMAT_BMP = CMISS_STREAM_INFORMATION_IMAGE_FILE_FORMAT_BMP,
		IMAGE_FILE_FORMAT_DICOM = CMISS_STREAM_INFORMATION_IMAGE_FILE_FORMAT_DICOM,
		IMAGE_FILE_FORMAT_JPG = CMISS_STREAM_INFORMATION_IMAGE_FILE_FORMAT_JPG,
		IMAGE_FILE_FORMAT_GIF = CMISS_STREAM_INFORMATION_IMAGE_FILE_FORMAT_GIF,
		IMAGE_FILE_FORMAT_PNG = CMISS_STREAM_INFORMATION_IMAGE_FILE_FORMAT_PNG,
		IMAGE_FILE_FORMAT_SGI = CMISS_STREAM_INFORMATION_IMAGE_FILE_FORMAT_SGI,
		IMAGE_FILE_FORMAT_TIFF = CMISS_STREAM_INFORMATION_IMAGE_FILE_FORMAT_TIFF,
	};

	enum ImagePixelFormat
	{
		IMAGE_PIXEL_FORMAT_INVALID = CMISS_STREAM_INFORMATION_IMAGE_PIXEL_FORMAT_INVALID,
		IMAGE_PIXEL_FORMAT_LUMINANCE = CMISS_STREAM_INFORMATION_IMAGE_PIXEL_FORMAT_LUMINANCE,
		IMAGE_PIXEL_FORMAT_LUMINANCE_ALPHA = CMISS_STREAM_INFORMATION_IMAGE_PIXEL_FORMAT_LUMINANCE_ALPHA,
		IMAGE_PIXEL_FORMAT_RGB = CMISS_STREAM_INFORMATION_IMAGE_PIXEL_FORMAT_RGB,
		IMAGE_PIXEL_FORMAT_RGBA = CMISS_STREAM_INFORMATION_IMAGE_PIXEL_FORMAT_RGBA,
		IMAGE_PIXEL_FORMAT_ABGR = CMISS_STREAM_INFORMATION_IMAGE_PIXEL_FORMAT_ABGR,
		IMAGE_PIXEL_FORMAT_BGR = CMISS_STREAM_INFORMATION_IMAGE_PIXEL_FORMAT_BGR,
	};

	int setAttributeInteger(ImageAttribute imageAttribute, int value)
	{
		return Cmiss_stream_information_image_set_attribute_integer(
			reinterpret_cast<Cmiss_stream_information_image_id>(id),
			static_cast<Cmiss_stream_information_image_attribute>(imageAttribute), value);
	}

	int setAttributeReal(ImageAttribute imageAttribute, double value)
	{
		return Cmiss_stream_information_image_set_attribute_real(
			reinterpret_cast<Cmiss_stream_information_image_id>(id),
			static_cast<Cmiss_stream_information_image_attribute>(imageAttribute), value);
	}

	int setFileFormat(ImageFileFormat imageFileFormat)
	{
		return Cmiss_stream_information_image_set_file_format(
			reinterpret_cast<Cmiss_stream_information_image_id>(id),
			static_cast<Cmiss_stream_information_image_file_format>(imageFileFormat));
	}

	int setPixelFormat(ImagePixelFormat imagePixelFormat)
	{
		return Cmiss_stream_information_image_set_pixel_format(
			reinterpret_cast<Cmiss_stream_information_image_id>(id),
			static_cast<Cmiss_stream_information_image_pixel_format>(imagePixelFormat));
	}

};

inline StreamInformationImage FieldImage::createStreamInformation()
{
	return StreamInformationImage(
		reinterpret_cast<Cmiss_stream_information_image_id>(
			Cmiss_field_image_create_stream_information(
				reinterpret_cast<Cmiss_field_image_id>(id))));
}

inline FieldImage FieldModule::createImage()
{
	return FieldImage(Cmiss_field_module_create_image(id,
		0, 0));
}

inline FieldImage FieldModule::createImageFromSource(Field& domain_field, Field& source_field)
{
	return FieldImage(Cmiss_field_module_create_image(id,
		domain_field.getId(), source_field.getId()));
}

} // namespace Cmiss

#endif /* __ZN_FIELD_TYPES_IMAGE_HPP__ */
