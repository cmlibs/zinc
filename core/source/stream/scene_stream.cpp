/***************************************************************************//**
 * FILE : scene_stream.cpp
 *
 * The definition to cmzn_streaminformation_scene.
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "zinc/streamscene.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "general/message.h"
#include "general/enumerator_conversion.hpp"
#include "stream/scene_stream.hpp"

int cmzn_scene_export_scene(cmzn_scene_id scene,
	cmzn_streaminformation_scene_id streaminformation_scene)
{
	int return_code = CMZN_OK;

	if (scene && streaminformation_scene &&
		streaminformation_scene->getExportFormat() == CMZN_STREAMINFORMATION_SCENE_EXPORT_FORMAT_THREEJS)
	{
		const cmzn_stream_properties_list streams_list = streaminformation_scene->getResourcesList();
		if (!(streams_list.empty()))
		{
			cmzn_stream_properties_list_const_iterator iter;
			cmzn_resource_properties *stream_properties = NULL;
			int number_of_entries = 0;
			std::string *output_string = 0;

			cmzn_scenefilter_id scenefilter = streaminformation_scene->getScenefilter();
			cmzn_scene_id scene = streaminformation_scene->getScene();

			return_code = Scene_render_threejs(scene,
				scenefilter, /*file_prefix*/"zinc_scene_export",
				streaminformation_scene->getNumberOfTimeSteps(),
				streaminformation_scene->getInitialTime(),
				streaminformation_scene->getFinishTime(),
				streaminformation_scene->getExportDataType(),
				&number_of_entries, &output_string);
			cmzn_scene_destroy(&scene);
			cmzn_scenefilter_destroy(&scenefilter);
			if (return_code != CMZN_OK)
				return CMZN_ERROR_GENERAL;

			cmzn_streamresource_id stream = NULL;
			int i = 0;
			for (iter = streams_list.begin(); iter != streams_list.end(); ++iter)
			{
				stream_properties = *iter;
				stream = stream_properties->getResource();

				cmzn_streamresource_file_id file_resource = cmzn_streamresource_cast_file(stream);
				cmzn_streamresource_memory_id memory_resource = NULL;
				if (number_of_entries > i)
				{
					if (file_resource)
					{
						char *file_name = file_resource->getFileName();
						if (file_name)
						{
							FILE *threejs_file = fopen(file_name,"w");
							fprintf(threejs_file, "%s", output_string[i].c_str());
							fclose(threejs_file);
							DEALLOCATE(file_name);
							i++;
						}
						cmzn_streamresource_file_destroy(&file_resource);
					}
					else if (NULL != (memory_resource = cmzn_streamresource_cast_memory(stream)))
					{
						char *buffer_out = duplicate_string(output_string[i].c_str());
						unsigned int buffer_size = strlen(buffer_out);
						memory_resource->setBuffer(buffer_out, buffer_size);
						cmzn_streamresource_memory_destroy(&memory_resource);
						i++;
					}
					else
					{
						return_code = 0;
						display_message(ERROR_MESSAGE, "cmzn_scene_export. Stream error");
					}
				}
				if (!return_code)
				{
					break;
				}
			}
			if (output_string)
				delete[] output_string;
		}
	}
	return return_code;
}

cmzn_streaminformation_id cmzn_scene_create_streaminformation_scene(
	cmzn_scene_id scene)
{
	if (scene)
	{
		return new cmzn_streaminformation_scene(scene);
	}
	return 0;
}

int cmzn_streaminformation_scene_destroy(
	cmzn_streaminformation_scene_id *streaminformation_address)
{
	if (streaminformation_address && *streaminformation_address)
	{
		(*streaminformation_address)->deaccess();
		*streaminformation_address = NULL;
		return 1;
	}
	return 0;
}

cmzn_streaminformation_scene_id cmzn_streaminformation_cast_scene(
	cmzn_streaminformation_id streaminformation)
{
	if (streaminformation &&
		(dynamic_cast<cmzn_streaminformation_scene *>(streaminformation)))
	{
		streaminformation->access();
		return (reinterpret_cast<cmzn_streaminformation_scene *>(streaminformation));
	}
	return 0;
}

int cmzn_streaminformation_scene_get_number_of_resources_required(cmzn_streaminformation_scene_id streaminformation)
{
	if (streaminformation)
	{
		return streaminformation->getNumberOfResourcesRequired();
	}
	return 0;
}

double cmzn_streaminformation_scene_get_initial_time(cmzn_streaminformation_scene_id streaminformation)
{
	if (streaminformation)
	{
		return streaminformation->getInitialTime();
	}
	return 0;
}

int cmzn_streaminformation_scene_set_initial_time(cmzn_streaminformation_scene_id streaminformation,
	double initialTime)
{
	if (streaminformation)
	{
		return streaminformation->setInitialTime(initialTime);
	}
	return CMZN_ERROR_ARGUMENT;
}

double cmzn_streaminformation_scene_get_finish_time(cmzn_streaminformation_scene_id streaminformation)
{
	if (streaminformation)
	{
		return streaminformation->getFinishTime();
	}

	return 0;
}

int cmzn_streaminformation_scene_set_finish_time(cmzn_streaminformation_scene_id streaminformation,
	double finishTime)
{
	if (streaminformation)
	{
		return streaminformation->setFinishTime(finishTime);
	}
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_streaminformation_scene_get_number_of_time_steps(cmzn_streaminformation_scene_id streaminformation)
{
	if (streaminformation)
	{
		return streaminformation->getNumberOfTimeSteps();
	}

	return 0;
}

int cmzn_streaminformation_scene_set_number_of_time_steps(cmzn_streaminformation_scene_id streaminformation,
	int numberOfTimeSteps)
{
	if (streaminformation)
	{
		return streaminformation->setNumberOfTimeSteps(numberOfTimeSteps);
	}
	return CMZN_ERROR_ARGUMENT;
}

enum cmzn_streaminformation_scene_export_format
	cmzn_streaminformation_scene_get_export_format(
	cmzn_streaminformation_scene_id streaminformation)
{
	if (streaminformation)
	{
		return streaminformation->getExportFormat();
	}

	return CMZN_STREAMINFORMATION_SCENE_EXPORT_FORMAT_INVALID;
}

int cmzn_streaminformation_scene_set_export_format(
	cmzn_streaminformation_scene_id streaminformation,
	enum cmzn_streaminformation_scene_export_format format)
{
	if (streaminformation)
	{
		streaminformation->setExportFormat(format);
		return CMZN_OK;
	}
	return CMZN_ERROR_ARGUMENT;
}

enum cmzn_streaminformation_scene_export_data_type
	cmzn_streaminformation_scene_get_export_data_type(
	cmzn_streaminformation_scene_id streaminformation)
{
	if (streaminformation)
	{
		return streaminformation->getExportDataType();
	}

	return CMZN_STREAMINFORMATION_SCENE_EXPORT_DATA_TYPE_INVALID;
}

int cmzn_streaminformation_scene_set_export_data_type(
	cmzn_streaminformation_scene_id streaminformation,
	cmzn_streaminformation_scene_export_data_type data_type)
{
	if (streaminformation)
	{
		streaminformation->setExportDataType(data_type);
		return CMZN_OK;
	}
	return CMZN_ERROR_ARGUMENT;
}

class cmzn_streaminformation_scene_export_format_conversion
{
public:
	static const char *to_string(enum cmzn_streaminformation_scene_export_format format)
	{
		const char *enum_string = 0;
		switch (format)
		{
			case CMZN_STREAMINFORMATION_SCENE_EXPORT_FORMAT_THREEJS:
				enum_string = "THREEJS";
				break;
			default:
				break;
		}
		return enum_string;
	}
};

cmzn_scenefilter_id cmzn_streaminformation_scene_get_scenefilter(
	cmzn_streaminformation_scene_id streaminformation)
{
	if (streaminformation)
	{
		return streaminformation->getScenefilter();
	}
	return 0;
}

int cmzn_streaminformation_scene_set_scenefilter(
	cmzn_streaminformation_scene_id streaminformation,
	cmzn_scenefilter_id scenefilter)
{
	if (streaminformation)
	{
		streaminformation->setScenefilter(scenefilter);
		return CMZN_OK;
	}
	return CMZN_ERROR_ARGUMENT;
}

enum cmzn_streaminformation_scene_export_format
	cmzn_streaminformation_scene_export_format_enum_from_string(
		const char *string)
{
	return string_to_enum<enum cmzn_streaminformation_scene_export_format,
		cmzn_streaminformation_scene_export_format_conversion>(string);
}

char *cmzn_streaminformation_scene_export_format_enum_to_string(
	enum cmzn_streaminformation_scene_export_format format)
{
	const char *format_string = cmzn_streaminformation_scene_export_format_conversion::to_string(format);
	return (format_string ? duplicate_string(format_string) : 0);
}
