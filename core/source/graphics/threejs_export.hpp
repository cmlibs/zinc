/***************************************************************************//**
 * FILE : threejs_export.hpp
 *
 * File containing a class for exporting threejs vertices and script for rendering.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "general/mystring.h"
#include "graphics/graphics_library.h"
#include "graphics/render_gl.h"
#include <string>

struct GT_object;

class Threejs_export
{
private:
	char *filename;
	int number_of_time_steps;
	cmzn_scene_render_threejs_data_export_mode mode;
	FILE *threejs_file;
	std::string facesString;
	std::string verticesMorphString;
	std::string normalMorphString;
	std::string colorsMorphString;

	void writeVertexBuffer(const char *output_variable_name,
		GLfloat *vertex_buffer, unsigned int values_per_vertex,
		unsigned int vertex_count);

	void writeMorphVertexBuffer(const char *output_variable_name,
		std::string *output,	GLfloat *vertex_buffer, unsigned int values_per_vertex,
		unsigned int vertex_count, int time_step);

	void writeIntegerBuffer(const char *output_variable_name,
		int *vertex_buffer, unsigned int values_per_vertex,
		unsigned int vertex_count);

	void writeMorphIntegerBuffer(const char *output_variable_name,
		std::string *output, int *vertex_buffer, unsigned int values_per_vertex,
		unsigned int vertex_count, int time_step);

	void writeIndexBuffer(struct GT_object *object, int typeMask,
		int number_of_points);

	void writeSpecialDataBuffer(struct GT_object *object, GLfloat *vertex_buffer,
		unsigned int values_per_vertex, unsigned int vertex_count);

public:

	Threejs_export(const char *filename, int number_of_time_steps_in,
		cmzn_scene_render_threejs_data_export_mode mode_in) :
		filename(duplicate_string(filename)), number_of_time_steps(number_of_time_steps_in),
		mode(mode_in),	threejs_file(0)
	{
		verticesMorphString.clear();
		colorsMorphString.clear();
		normalMorphString.clear();
		facesString.clear();
	}

	~Threejs_export();

	int exportGraphicsObject(struct GT_object *object, int time_step);

	int beginExport();

	int endExport();

};
