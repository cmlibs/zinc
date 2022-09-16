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
#include "jsoncpp/json.h"
#include "opencmiss/zinc/types/graphicsid.h"


struct GT_object;

/* generic threejs_export class with many basic methods to export
 * standard surfaces
 */
class Threejs_export
{
protected:

	char *filename;
	cmzn_streaminformation_scene_io_data_type mode;
	bool morphVerticesExported, morphColoursExported, morphNormalsExported;
	int morphVertices, morphColours, morphNormals;
	int number_of_time_steps;
	char *groupName, *regionPath;
	cmzn_graphics *graphics; // not accessed
	double textureSizes[3];
	std::string verticesMorphString;
	std::string colorsMorphString;
	std::string normalMorphString;
	std::string facesString;
	std::string outputString;
	bool isEmpty;
	unsigned int graphicsOrder;

	void writeVertexBuffer(const char *output_variable_name,
		GLfloat *vertex_buffer, unsigned int values_per_vertex,
		unsigned int vertex_count);

	void writeMorphVertexBuffer(const char *output_variable_name,
		std::string *output, GLfloat *vertex_buffer, unsigned int values_per_vertex,
		unsigned int vertex_count, int time_step);

	void writeIntegerBuffer(const char *output_variable_name,
		int *vertex_buffer, unsigned int values_per_vertex,
		unsigned int vertex_count);

	void writeMorphIntegerBuffer(const char *output_variable_name,
		std::string *output, int *vertex_buffer, unsigned int values_per_vertex,
		unsigned int vertex_count, int time_step);

	void writeIndexBuffer(struct GT_object *object, int typeMask,
		int number_of_points, unsigned int offset);

	virtual void writeIndexBufferWithoutIndex(int typeMask, int number_of_points, unsigned int offset);

	void writeSpecialDataBuffer(struct GT_object *object, GLfloat *vertex_buffer,
		unsigned int values_per_vertex, unsigned int vertex_count);

	/* texture coordinates export*/
	void writeUVsBuffer(GLfloat *texture_buffer, unsigned int values_per_vertex,
		unsigned int vertex_count);

public:

	Threejs_export(const char *filename_in, int number_of_time_steps_in,
			cmzn_streaminformation_scene_io_data_type mode_in,
			int morphVerticesIn, int morphColoursIn, int morphNormalsIn, 
			double *textureSizesIn, const char *groupNameIn,
			const char *regionPathIn, cmzn_graphics *graphicsIn,
			unsigned int graphicsOrderIn) :
		filename(duplicate_string(filename_in)),
		mode(mode_in),
		morphVerticesExported(false),
		morphColoursExported(false),
		morphNormalsExported(false),
		morphVertices(morphVerticesIn),
		morphColours(morphColoursIn),
		morphNormals(morphNormalsIn),
		number_of_time_steps(number_of_time_steps_in),
		groupName(groupNameIn ? duplicate_string(groupNameIn) : nullptr),
		regionPath(regionPathIn ? duplicate_string(regionPathIn) : nullptr),
		graphics(graphicsIn),
		graphicsOrder(graphicsOrderIn)
	{
		if (textureSizesIn)
		{
			textureSizes[0] = textureSizesIn[0];
			textureSizes[1] = textureSizesIn[1];
			textureSizes[2] = textureSizesIn[2];
		}
		else
		{
			textureSizes[0] = 0.0;
			textureSizes[1] = 0.0;
			textureSizes[2] = 0.0;
		}
		
		verticesMorphString.clear();
		colorsMorphString.clear();
		normalMorphString.clear();
		facesString.clear();
		outputString.clear();
		isEmpty = false;
	}

	virtual ~Threejs_export();

	virtual int exportGraphicsObject(struct GT_object *object, int time_step);

	void exportMaterial(cmzn_material_id material);

	int beginExport();

	int endExport();

	bool isValid() const
	{
		return !isEmpty;
	}

	const char *getGroupName() const
	{
		return groupName;
	}

	const char *getRegionPath() const
	{
		return regionPath;
	}
	
	/* this return json format describing colours and transformation of the glyph */
	Json::Value getExportJson();

	std::string *getExportString();

	bool isExportForGraphics(cmzn_graphics *graphicsIn) const
	{
		return (this->graphics == graphicsIn);
	}

	bool getMorphVerticesExported() const
	{
		return morphVerticesExported;
	}

	bool getMorphColoursExported() const
	{
		return morphColoursExported;
	}

	bool getMorphNormalsExported() const
	{
		return morphNormalsExported;
	}

	unsigned int getGraphicsOrder() const
	{
		return graphicsOrder;
	}

};

/* class for export glyph into WebGL format, only
 * surface glyphs are supported.
 */
class Threejs_export_glyph : public Threejs_export
{
private:

	void exportStaticGlyphs(struct GT_object *object);

	void exportGlyphsTransformation(struct GT_object *glyph,  int time_step);

	void exportGlyphsLabel(struct GT_object *glyph);

	void writeGlyphIndexBuffer(struct GT_object *object, int typeMask);

	Json::Value metadata, positions_json, axis1_json, axis2_json, axis3_json,
		scale_json, color_json, label_json;

	std::string glyphTransformationString;

	char *glyphGeometriesURLName;

public:

	Threejs_export_glyph(const char *filename_in, int number_of_time_steps_in,
		cmzn_streaminformation_scene_io_data_type mode_in,
		int morphVerticesIn, int morphColoursIn, int morphNormalsIn,
		double *textureSizesIn, const char *groupNameIn,
		const char* regionPathIn, cmzn_graphics *graphicsIn,
		unsigned int graphicsOrderIn) :
		Threejs_export(filename_in, number_of_time_steps_in,
		mode_in, morphVerticesIn, morphColoursIn, morphNormalsIn, textureSizesIn,
		groupNameIn, regionPathIn, graphicsIn, graphicsOrderIn)
	{
		glyphTransformationString.clear();
		glyphGeometriesURLName = 0;
	}

	virtual ~Threejs_export_glyph();

	virtual int exportGraphicsObject(struct GT_object *object, int time_step);

	/* this return json format describing colours and transformation of the glyph */
	Json::Value getGlyphTransformationExportJson();

	/* this return string format describing colours and transformation of the glyph */
	std::string *getGlyphTransformationExportString();

	void setGlyphGeometriesURLName(char *name);

};

/* class for export point into WebGL format.
 */
class Threejs_export_point : public Threejs_export
{
private:

	void exportPointsLabel(struct GT_object *point);

public:

	Threejs_export_point(const char *filename_in, int number_of_time_steps_in,
		cmzn_streaminformation_scene_io_data_type mode_in,
		int morphVerticesIn, int morphColoursIn, int morphNormalsIn,
		double *textureSizesIn, const char *groupNameIn, const char* regionPathIn,
		cmzn_graphics *graphicsIn, unsigned int graphicsOrderIn) :
		Threejs_export(filename_in, number_of_time_steps_in,
		mode_in, morphVerticesIn, morphColoursIn, morphNormalsIn, textureSizesIn,
		groupNameIn, regionPathIn, graphicsIn, graphicsOrderIn)
	{
	}

	virtual int exportGraphicsObject(struct GT_object *object, int time_step);

	virtual void writeIndexBufferWithoutIndex(int typeMask, int number_of_points, unsigned int offset);
};


/* class for export point into WebGL format.
 */
class Threejs_export_line : public Threejs_export_point
{

public:

	Threejs_export_line(const char *filename_in, int number_of_time_steps_in,
		cmzn_streaminformation_scene_io_data_type mode_in,
		int morphVerticesIn, int morphColoursIn, int morphNormalsIn,
		double *textureSizesIn, const char *groupNameIn, const char* regionPathIn,
		cmzn_graphics *graphicsIn, unsigned int graphicsOrderIn) :
			Threejs_export_point(filename_in, number_of_time_steps_in,
		mode_in, morphVerticesIn, morphColoursIn, morphNormalsIn, textureSizesIn,
		groupNameIn,  regionPathIn, graphicsIn, graphicsOrderIn)
	{
	}

	virtual int exportGraphicsObject(struct GT_object *object, int time_step);
};
