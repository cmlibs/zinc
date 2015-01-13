/***************************************************************************//**
 * FILE : threejs_export.cpp
 *
 * File containing a class for exporting threejs vertices and script for rendering.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "general/debug.h"
#include "graphics/threejs_export.hpp"
#include "graphics/graphics_object.h"
#include "graphics/graphics_object_private.hpp"
#include <iostream>
#include <string>
#include <math.h>
#include <iostream>
#include <sstream>
#include <fstream>

using namespace std;

enum ThreejsType
{
	THREEJS_TYPE_TRIANGLE = 0,
	THREEJS_TYPE_MATERIAL = 2,
	THREEJS_TYPE_VERTEX_TEX_COORD = 8,
	THREEJS_TYPE_VERTEX_NORMAL = 32,
	THREEJS_TYPE_FACE_COLOR = 64,
	THREEJS_TYPE_VERTEX_COLOR = 128
};

int rgb_to_hex(float r, float g, float b)
{
	int red = (r * 255) + 0.5, green = (g * 255) + 0.5, blue = (b * 255) + 0.5;
	int hex = 0;
	int remainder = 0, quotient = 0;
	if (red > 0)
	{
		quotient = red / 16;
		remainder = red % 16;
		hex += pow((double)16, 5) * quotient + pow((double)16, 4) * remainder;
	}
	if (green > 0)
	{
		quotient = green / 16;
		remainder = green % 16;
		hex += pow((double)16, 3) * quotient + pow((double)16, 2) * remainder;
	}
	if (blue > 0)
	{
		quotient = blue / 16;
		remainder = blue % 16;
		hex += pow((double)16, 1) * quotient + remainder;
	}
	return hex;
}

Threejs_export::~Threejs_export()
{
	DEALLOCATE(filename);
}

int Threejs_export::beginExport()
{

	outputString += "{\n\t\"metadata\" : {\n\t\t\"formatVersion\" : 3,\n";
	outputString += "\t\t\"description\" : \"Exported from LibZinc.\"\n\t},\n\n";
	return 1;
}

int Threejs_export::endExport()
{
	outputString += verticesMorphString;
	outputString += colorsMorphString;
	outputString += normalMorphString;
	outputString += facesString;
	outputString += "}\n";
	return 1;
}

std::string *Threejs_export::getExportString()
{
	return &outputString;
}

void Threejs_export::writeIntegerBuffer(const char *output_variable_name,
	int *vertex_buffer, unsigned int values_per_vertex,
	unsigned int vertex_count)
{
	if (vertex_buffer && (values_per_vertex > 0)  && (vertex_count > 0))
	{
		char temp[200];

		unsigned int number_of_valid_output =  values_per_vertex;
		if (number_of_valid_output > 3)
			number_of_valid_output = 3;
		int *currentVertex = vertex_buffer;
		sprintf(temp, "\t\"%s\" : [", output_variable_name);
		outputString += temp;

		for (unsigned int i = 0; i < vertex_count; i++)
		{
			if (((i % 10) == 0))
				outputString += "\n\t\t";
			for (unsigned int k = 0; k < number_of_valid_output; k++)
			{
				sprintf(temp, "%d", currentVertex[k]);
				outputString += temp;
				if (0 == ((k == number_of_valid_output - 1) && (i == vertex_count - 1)))
				{
					outputString += ",";
				}
			}
			currentVertex+=values_per_vertex;
		}
		outputString += "\n\t],\n\n";
	}
}

void Threejs_export::writeMorphIntegerBuffer(const char *output_variable_name,
	std::string *output, int *vertex_buffer, unsigned int values_per_vertex,
	unsigned int vertex_count, int time_step)
{
	if (vertex_buffer && (values_per_vertex > 0)  && (vertex_count > 0) &&
		(vertex_count > 0) && output)
	{
		if (time_step == 0)
		{
			(*output) += "\t\"morphColors\": [";
		}
		unsigned int number_of_valid_output =  values_per_vertex;
		if (number_of_valid_output > 3)
			number_of_valid_output = 3;
		int *currentVertex = vertex_buffer;
		char temp[300];
		sprintf(temp, "\t{ \"name\": \"%s_color_%03d\", \"%s\": [", filename, time_step, output_variable_name);
		(*output) += temp;
		for (unsigned int i = 0; i < vertex_count; i++)
		{
			if (((i % 10) == 0))
			{
				sprintf(temp,"\n\t\t");
				(*output) += temp;
			}
			for (unsigned int k = 0; k < number_of_valid_output; k++)
			{
				sprintf(temp, "%d", currentVertex[k]);
				(*output) += temp;
				if (0 == ((k == number_of_valid_output - 1) && (i == vertex_count - 1)))
				{
					(*output) += ",";
				}
			}
			currentVertex+=values_per_vertex;
		}
		if (number_of_time_steps - 1 > time_step)
		{
			(*output) += "] },\n";
		}
		else
		{
			(*output) += "] }\n\t],\n\n";
		}
	}
}

void Threejs_export::writeVertexBuffer(const char *output_variable_name,
	GLfloat *vertex_buffer, unsigned int values_per_vertex,
	unsigned int vertex_count)
{
	if (vertex_buffer && (values_per_vertex > 0)  && (vertex_count > 0))
	{
		unsigned int number_of_valid_output = 3;
		GLfloat *currentVertex = vertex_buffer;
		char new_string[100];
		sprintf(new_string, "\t\"%s\" : [", output_variable_name);
		outputString += new_string;
		for (unsigned int i = 0; i < vertex_count; i++)
		{
			if ((i % 10) == 0)
				outputString += "\n\t\t";
			for (unsigned int k = 0; k < number_of_valid_output; k++)
			{
				if ((number_of_valid_output > values_per_vertex) &&
					(k >= values_per_vertex))
					outputString += "0.0";
				else
				{
					sprintf(new_string, "%f", currentVertex[k]);
					outputString += new_string;
				}
				if (0 == ((k == number_of_valid_output - 1) && (i == vertex_count - 1)))
				{
					outputString += ",";
				}
			}
			currentVertex+=values_per_vertex;
		}
		outputString += "\n\t],\n\n";
	}
}

void Threejs_export::writeMorphVertexBuffer(const char *output_variable_name,
	std::string *output,	GLfloat *vertex_buffer, unsigned int values_per_vertex,
	unsigned int vertex_count, int time_step)
{
	if (vertex_buffer && (values_per_vertex > 0) &&
		(vertex_count > 0) && output)
	{
		if (time_step == 0)
		{
			if (!strcmp("vertices", output_variable_name))
			{
				(*output) += "\t\"morphTargets\": [";
			}
			else
			{
				(*output) += "\t\"morphNormals\": [";
			}
		}
		unsigned int number_of_valid_output =  3;
		GLfloat *currentVertex = vertex_buffer;

		char temp[300];
		sprintf(temp, "\t{ \"name\": \"%s_%03d\", \"%s\": [", filename, time_step, output_variable_name);
		(*output) += temp;
		for (unsigned int i = 0; i < vertex_count; i++)
		{
			if (((i % 10) == 0))
			{
				sprintf(temp,"\n\t\t");
				(*output) += temp;
			}
			for (unsigned int k = 0; k < number_of_valid_output; k++)
			{
				if ((number_of_valid_output > values_per_vertex) &&
					(k >= values_per_vertex))
					sprintf(temp, "0.0");
				else
					sprintf(temp, "%f", currentVertex[k]);
				(*output) += temp;
				if (0 == ((k == number_of_valid_output - 1) && (i == vertex_count - 1)))
				{
					(*output) += ",";
				}
			}
			currentVertex+=values_per_vertex;
		}
		if (number_of_time_steps - 1 > time_step)
		{
			(*output) += "] },\n";
		}
		else
		{
			(*output) += "] }\n\t],\n\n";
		}
	}
}


void Threejs_export::writeIndexBuffer(struct GT_object *object, int typeMask, int number_of_points)
{
	if (object)
	{
		facesString += "\t\"faces\": [\n";
		unsigned int *index_vertex_buffer = 0, index_values_per_vertex = 0, index_vertex_count = 0;
		object->vertex_array->get_unsigned_integer_vertex_buffer(
			GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_STRIP_INDEX_ARRAY,
			&index_vertex_buffer, &index_values_per_vertex,
			&index_vertex_count);
		char temp[100];
		unsigned int face_colour_index = 0;
		if (index_vertex_buffer)
		{
			unsigned int *indices = index_vertex_buffer;
			int current_index = 0;
			unsigned int *number_buffer = 0, number_per_vertex = 0, number_count = 0;
			object->vertex_array->get_unsigned_integer_vertex_buffer(
				GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_NUMBER_OF_POINTS_FOR_STRIP,
				&number_buffer, &number_per_vertex, &number_count);
			unsigned int points_per_strip = 0;
			for (unsigned i = 0; i < number_count; i ++)
			{
				points_per_strip = number_buffer[i];
				for (unsigned int j =0; j< points_per_strip - 2; j++)
				{
					sprintf(temp,"\t\t%d", typeMask);
					facesString += temp;
					if (0 == (j % 2))
					{
						sprintf(temp," ,%d,%d,%d",
							indices[current_index+j], indices[current_index+j+1], indices[current_index+j+2]);
						facesString += temp;
						if (typeMask & THREEJS_TYPE_VERTEX_TEX_COORD)
						{
							sprintf(temp," ,%d,%d,%d",
								indices[current_index+j], indices[current_index+j+1], indices[current_index+j+2]);
							facesString += temp;
						}
						if (typeMask & THREEJS_TYPE_VERTEX_NORMAL)
						{
							sprintf(temp," ,%d,%d,%d",
								indices[current_index+j], indices[current_index+j+1], indices[current_index+j+2]);
							facesString += temp;
						}
						if (typeMask & THREEJS_TYPE_FACE_COLOR)
						{
							sprintf(temp," ,%d",	face_colour_index);
							facesString += temp;
							face_colour_index++;
						}
						if (typeMask & THREEJS_TYPE_VERTEX_COLOR)
						{
							sprintf(temp," ,%d,%d,%d",
								indices[current_index+j], indices[current_index+j+1], indices[current_index+j+2]);
							facesString += temp;
						}
					}
					else
					{
						sprintf(temp," ,%d,%d,%d",
							indices[current_index+j+1], indices[current_index+j], indices[current_index+j+2]);
						facesString += temp;
						if (typeMask & THREEJS_TYPE_VERTEX_TEX_COORD)
						{
							sprintf(temp," ,%d,%d,%d",
								indices[current_index+j+1], indices[current_index+j], indices[current_index+j+2]);
							facesString += temp;
						}
						if (typeMask & THREEJS_TYPE_VERTEX_NORMAL)
						{
							sprintf(temp," ,%d,%d,%d",
								indices[current_index+j+1], indices[current_index+j], indices[current_index+j+2]);
							facesString += temp;
						}
						if (typeMask & THREEJS_TYPE_FACE_COLOR)
						{
							sprintf(temp," ,%d",	face_colour_index);
							facesString += temp;
							face_colour_index++;
						}
						if (typeMask & THREEJS_TYPE_VERTEX_COLOR)
						{
							sprintf(temp," ,%d,%d,%d",
								indices[current_index+j+1], indices[current_index+j], indices[current_index+j+2]);
							facesString += temp;
						}
					}
					if (!((i == number_count - 1) && (j == points_per_strip - 3)))
					{
						facesString += ",";
					}
					facesString += "\n";
				}
				current_index += points_per_strip;
			}
		}
		else
		{
			unsigned int number_of_triangles = number_of_points / 3;
			int current_index = 0;
			for (unsigned i = 0; i < number_of_triangles; i++)
			{
				sprintf(temp,"\t\t%d", typeMask);
				facesString += temp;
				sprintf(temp," ,%d,%d,%d", current_index, current_index+1, current_index+2);
				facesString += temp;
				if (typeMask & THREEJS_TYPE_VERTEX_TEX_COORD)
				{
					sprintf(temp," ,%d,%d,%d", current_index, current_index+1, current_index+2);
					facesString += temp;
				}
				if (typeMask & THREEJS_TYPE_VERTEX_NORMAL)
				{
					sprintf(temp," ,%d,%d,%d", current_index, current_index+1, current_index+2);
					facesString += temp;
				}
				if (typeMask & THREEJS_TYPE_FACE_COLOR)
				{
					sprintf(temp," ,%d",	face_colour_index);
					facesString += temp;
					face_colour_index++;
				}
				if (typeMask & THREEJS_TYPE_VERTEX_COLOR)
				{
					sprintf(temp," ,%d,%d,%d", current_index, current_index+1, current_index+2);
					facesString += temp;
				}
				current_index += 3;
				if (i != number_of_triangles - 1)
				{
					facesString += ",";
				}
				facesString += "\n";

			}
		}
		facesString += "\t]\n\n";
	}
}

void Threejs_export::writeSpecialDataBuffer(struct GT_object *object, GLfloat *vertex_buffer,
	unsigned int values_per_vertex, unsigned int vertex_count)
{
	char num_string[100];

	if (vertex_buffer && (values_per_vertex > 0)  && (vertex_count > 0))
	{
		outputString += "\t\"colors\" : [";
		if (mode == CMZN_STREAMINFORMATION_SCENE_IO_DATA_TYPE_PER_VERTEX_VALUE)
		{
			GLfloat *currentVertex = vertex_buffer;
			for (unsigned int i = 0; i < vertex_count; i++)
			{
				if (((i % 10) == 0))
					outputString += "\n\t\t";
				for (unsigned int k = 0; k < values_per_vertex; k++)
				{
					sprintf(num_string, "%f", currentVertex[k]);
					outputString += num_string;
					if (0 == ((k == values_per_vertex - 1) && (i == vertex_count - 1)))
					{
						outputString += ",";
					}
				}
				currentVertex+=values_per_vertex;
			}
		}
		else
		{
			unsigned int *index_vertex_buffer = 0, index_values_per_vertex = 0, index_vertex_count = 0;
			object->vertex_array->get_unsigned_integer_vertex_buffer(
				GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_STRIP_INDEX_ARRAY,
				&index_vertex_buffer, &index_values_per_vertex,
				&index_vertex_count);
			if (index_vertex_buffer)
			{
				unsigned int *indices = index_vertex_buffer;
				int current_index = 0;
				unsigned int *number_buffer = 0, number_per_vertex = 0, number_count = 0;
				object->vertex_array->get_unsigned_integer_vertex_buffer(
					GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_NUMBER_OF_POINTS_FOR_STRIP,
					&number_buffer, &number_per_vertex, &number_count);
				unsigned int points_per_strip = 0;
				unsigned int index[3];
				for (unsigned i = 0; i < number_count; i ++)
				{
					outputString += "\n\t\t";
					points_per_strip = number_buffer[i];
					for (unsigned int j =0; j< points_per_strip - 2; j++)
					{
						if (0 == (j % 2))
						{
							index[0] = indices[current_index+j];
							index[1] = indices[current_index+j+1];
							index[2] = indices[current_index+j+2];
						}
						else
						{
							index[0] = indices[current_index+j+1];
							index[1] = indices[current_index+j];
							index[2] = indices[current_index+j+2];
						}
						GLfloat *currentVertex = vertex_buffer;
						for (unsigned int k = 0; k < values_per_vertex; k++)
						{
							GLfloat average = (currentVertex[index[0] * values_per_vertex + k] +
								currentVertex[index[1] * values_per_vertex + k] +
								currentVertex[index[2] * values_per_vertex + k]) / 3;
							sprintf(num_string, "%f", average);
							outputString += num_string;
							if (!((i == number_count - 1) && (k == values_per_vertex - 1) &&
								 (j == points_per_strip - 3)))
							{
								outputString += ",";
							}
						}
					}
					current_index += points_per_strip;
				}

			}
			else
			{
				/* face colors, get the average vertex colors */
				GLfloat *currentVertex = vertex_buffer;
				for (unsigned int i = 0; i < vertex_count; i += 3)
				{
					if (((i % 10) == 0))
						outputString += "\n\t\t";
					for (unsigned int k = 0; k < values_per_vertex; k++)
					{
						GLfloat average = (currentVertex[k] + currentVertex[k + values_per_vertex] +
							currentVertex[k + values_per_vertex * 2]) / 3;
						char num_string[100];
						sprintf(num_string, "%f", average);
						outputString += num_string;
						if (0 == ((k == values_per_vertex - 1) && (i == vertex_count - 1)))
						{
							outputString += ",";
						}
					}
					currentVertex+=values_per_vertex * 3;
				}
			}
		}
		outputString += "\n\t],\n\n";
	}
}


int Threejs_export::exportGraphicsObject(struct GT_object *object, int time_step)
{
	if (object)
	{
		int typebitmask = 0;
		int buffer_binding = object->buffer_binding;
		object->buffer_binding = 1;

		switch (GT_object_get_type(object))
		{
		case g_SURFACE_VERTEX_BUFFERS:
		{
			GLfloat *position_vertex_buffer = NULL;
			unsigned int position_values_per_vertex, position_vertex_count;
			if (object->vertex_array->get_float_vertex_buffer(
				GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_POSITION,
				&position_vertex_buffer, &position_values_per_vertex,
				&position_vertex_count))
			{
				if (time_step == 0)
				{
					writeVertexBuffer("vertices",
						position_vertex_buffer, position_values_per_vertex,
						position_vertex_count);
				}
				if (number_of_time_steps > 1)
				{
					writeMorphVertexBuffer("vertices", &verticesMorphString,
						position_vertex_buffer, position_values_per_vertex,
						position_vertex_count, time_step);
				}
			}

			if (mode == CMZN_STREAMINFORMATION_SCENE_IO_DATA_TYPE_COLOUR)
			{
				unsigned int colour_values_per_vertex, colour_vertex_count;
				GLfloat *colour_buffer = (GLfloat *)NULL;
				if (Graphics_object_create_colour_buffer_from_data(object,
					&colour_buffer,
					&colour_values_per_vertex, &colour_vertex_count)
					&& (colour_vertex_count == position_vertex_count))
				{
					int *hex_colours = new int[colour_vertex_count];
					GLfloat *colours = colour_buffer;
					for (unsigned int i = 0; i < colour_vertex_count; i++)
					{
						hex_colours[i] = rgb_to_hex(colours[0], colours[1], colours[2]);
						colours += colour_values_per_vertex;
					}
					if (time_step == 0)
					{
						typebitmask |= THREEJS_TYPE_VERTEX_COLOR;
						writeIntegerBuffer("colors",
							hex_colours, 1, colour_vertex_count);
					}
					if (number_of_time_steps > 1)
					{
						writeMorphIntegerBuffer("colors", &colorsMorphString,
							hex_colours, 1, colour_vertex_count, time_step);
					}
					delete[] hex_colours;
					if (colour_buffer)
					{
						DEALLOCATE(colour_buffer);
					}
				}
			}
			else if (mode == CMZN_STREAMINFORMATION_SCENE_IO_DATA_TYPE_PER_VERTEX_VALUE ||
					CMZN_STREAMINFORMATION_SCENE_IO_DATA_TYPE_PER_FACE_VALUE)
			{
				GLfloat *data_buffer = NULL;
				unsigned int data_values_per_vertex, data_vertex_count;
				if (object->vertex_array->get_float_vertex_buffer(
					GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_DATA,
					&data_buffer, &data_values_per_vertex, &data_vertex_count))
				{
					if (time_step == 0)
					{
						if (mode == CMZN_STREAMINFORMATION_SCENE_IO_DATA_TYPE_PER_FACE_VALUE)
						{
							typebitmask |= THREEJS_TYPE_FACE_COLOR;
						}
						else
						{
							typebitmask |= THREEJS_TYPE_VERTEX_COLOR;
						}
						writeSpecialDataBuffer(object, data_buffer, data_values_per_vertex,
							data_vertex_count);
					}
				}
			}

			GLfloat *normal_buffer = NULL;
			unsigned int normal_values_per_vertex, normal_vertex_count;
			if (object->vertex_array->get_float_vertex_buffer(
				GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_NORMAL,
				&normal_buffer, &normal_values_per_vertex, &normal_vertex_count)
				&& (3 == normal_values_per_vertex))
			{
				if (time_step == 0)
				{
					typebitmask |= THREEJS_TYPE_VERTEX_NORMAL;
					writeVertexBuffer("normals",
						normal_buffer, normal_values_per_vertex,
						normal_vertex_count);
				}
				if (number_of_time_steps > 1)
				{
					writeMorphVertexBuffer("normals", &normalMorphString,
						normal_buffer, normal_values_per_vertex,
						normal_vertex_count, time_step);
				}
			}
/*
			GLfloat *texture_coordinate0_buffer = NULL;
			unsigned int texture_coordinate0_values_per_vertex,
			texture_coordinate0_vertex_count;
			if (object->vertex_array->get_float_vertex_buffer(
				GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_TEXTURE_COORDINATE_ZERO,
				&texture_coordinate0_buffer, &texture_coordinate0_values_per_vertex,
				&texture_coordinate0_vertex_count)
				&& (texture_coordinate0_vertex_count == position_vertex_count))
			{
				typebitmask |= THREEJS_TYPE_VERTEX_TEX_COORD;
				writeVertexBuffer("uvs",
					texture_coordinate0_buffer, texture_coordinate0_values_per_vertex,
					texture_coordinate0_vertex_count);
			}
			*/
			if (time_step == 0)
			{
				writeIndexBuffer(object, typebitmask, position_vertex_count);
			}
		} break;
		default:
			break;
		}
		object->buffer_binding = buffer_binding;
		return 1;
	}
	return 0;
}
