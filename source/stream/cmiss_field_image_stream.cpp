/***************************************************************************//**
 * FILE : cmiss_field_image_stream.cpp
 *
 * The definition to Cmiss_field_image_stream.
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

extern "C" {
#include "api/cmiss_field_image.h"
#include "computed_field/computed_field_image.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "general/image_utilities.h"
#include "graphics/texture.h"
#include "user_interface/message.h"
}
#include "stream/cmiss_field_image_stream.hpp"

int Cmiss_field_image_read(Cmiss_field_image_id image_field,
	Cmiss_stream_information_id stream_information)
{
	int return_code = 1;
	struct Cmgui_image_information *image_information = NULL;
	Cmiss_stream_information_image_id image_stream_information = NULL;
	if (stream_information)
	{
		image_stream_information = dynamic_cast<Cmiss_stream_information_image *>(stream_information);
	}
	if (image_field && image_stream_information &&
		(NULL != (image_information = image_stream_information->getImageInformation())))
	{
		char *field_name = Cmiss_field_get_name(Cmiss_field_image_base_cast(image_field));
		const Cmiss_stream_properties_list streams_list = image_stream_information->getResourcesList();
		if (!(streams_list.empty()))
		{
			Cmiss_stream_properties_list_const_iterator iter;
			Cmiss_resource_properties *stream_properties = NULL;
			Cmiss_stream_resource_id stream = NULL;
			int fileStream = 0;
			int memoryStream = 0;
			for (iter = streams_list.begin(); iter != streams_list.end(); ++iter)
			{
				stream_properties = *iter;
				stream = stream_properties->getResource();
				Cmiss_stream_resource_file_id file_resource = Cmiss_stream_resource_cast_file(stream);
				Cmiss_stream_resource_memory_id memory_resource = NULL;
				if (file_resource)
				{
					char *file_name = file_resource->getFileName();
					if (file_name)
					{
						fileStream = 1;
						if (!memoryStream)
						{
							Cmgui_image_information_add_file_name(image_information, file_name);
						}
						else
						{
							display_message(ERROR_MESSAGE, "Cmiss_field_image_read. Cannot read both file and memory in "
								"one stream information");
							return_code = 0;
						}
						DEALLOCATE(file_name);
					}
					Cmiss_stream_resource_file_destroy(&file_resource);
				}
				else if (NULL != (memory_resource = Cmiss_stream_resource_cast_memory(stream)))
				{
					void *memory_block = NULL;
					unsigned int buffer_size = 0;
					memory_resource->getBuffer(&memory_block, &buffer_size);
					if (memory_block)
					{
						memoryStream = 1;
						if (!fileStream)
						{
							Cmgui_image_information_add_memory_block(image_information, memory_block,
								buffer_size);
						}
						else
						{
							display_message(ERROR_MESSAGE, "Cmiss_field_image_read. Cannot read both file and memory in "
								"one stream information");
							return_code = 0;
						}
					}
					Cmiss_stream_resource_memory_destroy(&memory_resource);
				}
				else
				{
					return_code = 0;
					display_message(ERROR_MESSAGE, "Cmiss_field_image_read. Stream error");
					break;
				}
				if (!return_code)
					break;
			}
			if (return_code)
			{
				struct Cmgui_image *cmgui_image = Cmgui_image_read(image_information);
				if (cmgui_image != NULL)
				{
					char *property, *value;
					Texture *texture= CREATE(Texture)(field_name);
					if (texture && Texture_set_image(texture, cmgui_image,
						field_name, /*file_number_pattern*/NULL,
						/*file_number_series_data.start*/0,
						/*file_number_series_data.stop*/0,
						/*file_number_series_data.increment*/1,
						/*image_data.crop_left_margin*/0,
						/*image_data.crop_bottom_margin*/0,
						/*image_data.crop_width*/0, /*image_data.crop_height*/0))
					{
						/* Calling get_property with wildcard ensures they
						will be available to the iterator, as well as
						any other properties */
						Cmgui_image_get_property(cmgui_image,"exif:*");
						Cmgui_image_reset_property_iterator(cmgui_image);
						while ((property = Cmgui_image_get_next_property(
							cmgui_image)) &&
							(value = Cmgui_image_get_property(cmgui_image,
								property)))
						{
							Texture_set_property(texture, property, value);
							DEALLOCATE(property);
							DEALLOCATE(value);
						}
						DESTROY(Cmgui_image)(&cmgui_image);
						return_code = 1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Cmiss_field_image_read.  Could not create image for field");
						return_code = 0;
					}
					if (return_code)
					{
						return_code = Cmiss_field_image_set_texture(image_field, texture);
						DESTROY(Texture)(&texture);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Cmiss_field_image_read.  Could not read image file");
					return_code = 0;
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_field_image_read.  stream_information does not contain any stream");
			return_code = 0;
		}
		if (field_name)
			DEALLOCATE(field_name);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_field_image_read.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_field_image_read */

int Cmiss_field_image_read_file(Cmiss_field_image_id image_field, const char *file_name)
{
	int return_code = 0;
	if (image_field && file_name)
	{
		Cmiss_stream_information_id stream_information =
			Cmiss_field_image_create_stream_information(image_field);
		Cmiss_stream_resource_id resource = Cmiss_stream_information_create_resource_file(
			stream_information, file_name);
	  return_code = Cmiss_field_image_read(image_field, stream_information);
  	Cmiss_stream_resource_destroy(&resource);
  	Cmiss_stream_information_destroy(&stream_information);
	}
	return return_code;
}

int Cmiss_field_image_write(Cmiss_field_image_id image_field,
	Cmiss_stream_information_id stream_information)
{
	int return_code = 1;
	struct Cmgui_image_information *image_information = NULL;
	Cmiss_stream_information_image_id image_stream_information = NULL;
	if (stream_information)
	{
		image_stream_information = dynamic_cast<Cmiss_stream_information_image *>(stream_information);
	}
	return_code = 1;
	if (image_field && image_stream_information &&
		(NULL != (image_information = image_stream_information->getImageInformation())))
	{
		struct Cmgui_image *cmgui_image = Texture_get_image(Cmiss_field_image_get_texture(image_field));
		const Cmiss_stream_properties_list streams_list = image_stream_information->getResourcesList();
		int number_of_streams = streams_list.size();
		if ((number_of_streams > 0 ) && cmgui_image &&
			(Cmgui_image_get_number_of_images(cmgui_image) == number_of_streams))
		{
			Cmiss_stream_properties_list_const_iterator iter;
			Cmiss_resource_properties *stream_properties = NULL;
			Cmiss_stream_resource_id stream = NULL;
			int set_write_to_memory = 0;
			int fileStream = 0, memoryStream = 0;
			for (iter = streams_list.begin(); iter != streams_list.end(); ++iter)
			{
				stream_properties = *iter;
				stream = stream_properties->getResource();
				Cmiss_stream_resource_file_id file_resource = Cmiss_stream_resource_cast_file(stream);
				Cmiss_stream_resource_memory_id memory_resource = NULL;
				if (file_resource)
				{
					char *file_name = file_resource->getFileName();
					if (file_name)
					{
						fileStream = 1;
						if (!memoryStream)
						{
							Cmgui_image_information_add_file_name(image_information, file_name);
						}
						else
						{
							display_message(ERROR_MESSAGE, "Cmiss_field_image_write. Cannot write to both file and memory in "
								"one stream information");
							return_code = 0;
						}
						DEALLOCATE(file_name);
					}
					Cmiss_stream_resource_file_destroy(&file_resource);
				}
				else if (NULL != (memory_resource = Cmiss_stream_resource_cast_memory(stream)))
				{
					if (fileStream)
					{
						display_message(ERROR_MESSAGE, "Cmiss_field_image_write. Stream information "
							"contains incorrect stream");
						return_code = 0;
					}
					else
					{
						memoryStream = 1;
						set_write_to_memory = 1;
					}
					Cmiss_stream_resource_memory_destroy(&memory_resource);
				}
				else
				{
					return_code = 0;
					display_message(ERROR_MESSAGE, "Cmiss_field_image_write. Stream error");
					break;
				}
				if (!return_code)
					break;
			}
			if (return_code)
			{
				if (set_write_to_memory)
				{
					Cmgui_image_information_set_write_to_memory_block(image_information);
				}
				if (!Cmgui_image_write(cmgui_image, image_information))
				{
					display_message(ERROR_MESSAGE, "Cmiss_field_image_write.  "
						"Error writing image");
					return_code = 0;
				}
				else
				{
					if (set_write_to_memory)
					{
						void **memory_blocks = NULL;
						unsigned int *memory_block_lengths = NULL;
						int number_of_blocks;
						Cmgui_image_information_get_memory_blocks(image_information, &number_of_blocks,
							&memory_blocks, &memory_block_lengths);
						if ((number_of_blocks == number_of_streams)&& memory_blocks && memory_block_lengths)
						{
							int k = 0;
							for (iter = streams_list.begin(); iter != streams_list.end(); ++iter)
							{
								stream_properties = *iter;
								stream = stream_properties->getResource();
								Cmiss_stream_resource_memory_id memory_resource =
									Cmiss_stream_resource_cast_memory(stream);
								if (memory_resource)
								{
									memory_resource->setBuffer(memory_blocks[k], memory_block_lengths[k]);
									Cmiss_stream_resource_memory_destroy(&memory_resource);
								}
								k++;
							}
							DEALLOCATE(memory_block_lengths);
							DEALLOCATE(memory_blocks);
						}
					}
				}
				DESTROY(Cmgui_image)(&cmgui_image);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_field_image_write.  Stream information does not contain the correct"
				"numerb of streams or field does not contain images");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_field_image_write.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_field_image_write */

int Cmiss_field_image_write_file(Cmiss_field_image_id image_field, const char *file_name)
{
	int return_code = 0;
	if (image_field && file_name)
	{
		Cmiss_stream_information_id stream_information =
			Cmiss_field_image_create_stream_information(image_field);
		Cmiss_stream_resource_id resource = Cmiss_stream_information_create_resource_file(
			stream_information, file_name);
	  return_code = Cmiss_field_image_write(image_field, stream_information);
  	Cmiss_stream_resource_destroy(&resource);
  	Cmiss_stream_information_destroy(&stream_information);
	}
	return return_code;
}

Cmiss_stream_information_id Cmiss_field_image_create_stream_information(
	Cmiss_field_image_id image_field)
{
	if (image_field)
	{
		return new Cmiss_stream_information_image(image_field);
	}
	return NULL;
}

int Cmiss_stream_information_image_destroy(
	Cmiss_stream_information_image_id *stream_information_address)
{
	if (stream_information_address && *stream_information_address)
	{
		(*stream_information_address)->deaccess();
		*stream_information_address = NULL;
		return 1;
	}
	return 0;
}

Cmiss_stream_information_image_id Cmiss_stream_information_cast_image(
	Cmiss_stream_information_id stream_information)
{
	if (stream_information &&
		(dynamic_cast<Cmiss_stream_information_image *>(stream_information)))
	{
		stream_information->access();
		return (reinterpret_cast<Cmiss_stream_information_image *>(stream_information));
	}
	else
	{
		return (NULL);
	}
}

int Cmiss_stream_information_image_set_attribute_integer(
	Cmiss_stream_information_image_id stream_information,
	enum Cmiss_stream_information_image_attribute attribute, int value)
{
	struct Cmgui_image_information *image_information = NULL;
	if (stream_information &&
		(NULL != (image_information = stream_information->getImageInformation())))
	{
		switch (attribute)
		{
			case CMISS_STREAM_INFORMATION_IMAGE_ATTRIBUTE_RAW_WIDTH_PIXELS:
			{
				return (Cmgui_image_information_set_width(image_information, value));
			} break;
			case CMISS_STREAM_INFORMATION_IMAGE_ATTRIBUTE_RAW_HEIGHT_PIXELS:
			{
				return (Cmgui_image_information_set_height(image_information, value));
			} break;
			case CMISS_STREAM_INFORMATION_IMAGE_ATTRIBUTE_BITS_PER_COMPONENT:
			{
				int number_of_bytes = 1;
				if (value == 8)
					number_of_bytes = 1;
				else if (value ==16)
					number_of_bytes = 2;
				return (Cmgui_image_information_set_number_of_bytes_per_component(
					image_information, number_of_bytes));
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_stream_information_image_set_attribue_int.  "
					"Invalid attribute");
			} break;
		}
	}
	return 0;
}

int Cmiss_stream_information_image_set_attribute_real(
	Cmiss_stream_information_image_id stream_information,
	enum Cmiss_stream_information_image_attribute attribute, double value)
{
	struct Cmgui_image_information *image_information = NULL;
	if (stream_information &&
		(NULL != (image_information = stream_information->getImageInformation())))
	{
		switch (attribute)
		{
			case CMISS_STREAM_INFORMATION_IMAGE_ATTRIBUTE_COMPRESSION_QUALITY:
			{
				return (Cmgui_image_information_set_quality(image_information, value));
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_stream_information_image_set_attribute_double.  "
					"Invalid attribute");
			} break;
		}
	}
	return 0;
}

int Cmiss_stream_information_image_set_file_format(
	Cmiss_stream_information_image_id stream_information,
	enum Cmiss_stream_information_image_file_format format)
{
	struct Cmgui_image_information *image_information = NULL;
	enum Image_file_format cmgui_file_format = JPG_FILE_FORMAT;
	int return_code = 0;

	if (stream_information &&
		(NULL != (image_information = stream_information->getImageInformation())))
	{
		return_code = 1;
		switch (format)
		{
			case CMISS_STREAM_INFORMATION_IMAGE_FILE_FORMAT_BMP:
			{
				cmgui_file_format = BMP_FILE_FORMAT;
			} break;
			case CMISS_STREAM_INFORMATION_IMAGE_FILE_FORMAT_DICOM:
			{
				cmgui_file_format = DICOM_FILE_FORMAT;
			} break;
			case CMISS_STREAM_INFORMATION_IMAGE_FILE_FORMAT_JPG:
			{
				cmgui_file_format = JPG_FILE_FORMAT;
			} break;
			case CMISS_STREAM_INFORMATION_IMAGE_FILE_FORMAT_GIF:
			{
				cmgui_file_format = GIF_FILE_FORMAT;
			} break;
			case CMISS_STREAM_INFORMATION_IMAGE_FILE_FORMAT_PNG:
			{
				cmgui_file_format = PNG_FILE_FORMAT;
			} break;
			case CMISS_STREAM_INFORMATION_IMAGE_FILE_FORMAT_SGI:
			{
				cmgui_file_format = SGI_FILE_FORMAT;
			} break;
			case CMISS_STREAM_INFORMATION_IMAGE_FILE_FORMAT_TIFF:
			{
				cmgui_file_format = TIFF_FILE_FORMAT;
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_stream_information_image_set_format.  "
					"File format not implemented yet.");
			} break;
		}
		if (return_code)
		{
			return_code = Cmgui_image_information_set_image_file_format(
				image_information, cmgui_file_format);
		}
	}
	return (return_code);
}

int Cmiss_stream_information_image_set_pixel_format(
	Cmiss_stream_information_image_id stream_information,
	enum Cmiss_stream_information_image_pixel_format pixel_format)
{
	struct Cmgui_image_information *image_information = NULL;
	int return_code = 0;
	if (stream_information &&
		(NULL != (image_information = stream_information->getImageInformation())))
	{
		int number_of_components;
		switch(pixel_format)
		{
			case CMISS_STREAM_INFORMATION_IMAGE_PIXEL_FORMAT_LUMINANCE:
			{
				number_of_components = 1;
			} break;
			case CMISS_STREAM_INFORMATION_IMAGE_PIXEL_FORMAT_LUMINANCE_ALPHA:
			{
				number_of_components = 2;
			} break;
			case CMISS_STREAM_INFORMATION_IMAGE_PIXEL_FORMAT_RGB:
			{
				number_of_components = 3;
			} break;
			case CMISS_STREAM_INFORMATION_IMAGE_PIXEL_FORMAT_RGBA:
			{
				number_of_components = 4;
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Texture_set_pixel_format.  Pixel format not implemented yet.");
				number_of_components = 0;
			} break;
		}
		if (number_of_components)
		{
			return_code = Cmgui_image_information_set_number_of_components(
				image_information, number_of_components);
		}
	}
	return (return_code);
}
