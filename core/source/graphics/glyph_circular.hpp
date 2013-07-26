/**
 * FILE : glyph_circular.hpp
 *
 * Internal header for glyphs with circular features which are drawn with
 * tessellation circle divisions.
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
 * Portions created by the Initial Developer are Copyright (C) 2013
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
#if !defined (GLYPH_CIRCULAR_HPP)
#define GLYPH_CIRCULAR_HPP

#include <vector>
#include "graphics/glyph.hpp"

struct Cmiss_glyph_circular : public Cmiss_glyph
{
private:

	struct GraphicsObjectDivisions
	{
		int circleDivisions;
		GT_object *graphicsObject;

	public:
		GraphicsObjectDivisions() :
			circleDivisions(0),
			graphicsObject(0)
		{
		}

		GraphicsObjectDivisions(int circleDivisionsIn, GT_object *graphicsObjectIn) :
			circleDivisions(circleDivisionsIn),
			graphicsObject(ACCESS(GT_object)(graphicsObjectIn))
		{
		}

		GraphicsObjectDivisions(const GraphicsObjectDivisions& source) :
			circleDivisions(source.circleDivisions),
			graphicsObject(ACCESS(GT_object)(source.graphicsObject))
		{
		}

		~GraphicsObjectDivisions()
		{
			DEACCESS(GT_object)(&graphicsObject);
		}

		GraphicsObjectDivisions& operator=(const GraphicsObjectDivisions& source)
		{
			this->circleDivisions = source.circleDivisions;
			REACCESS(GT_object)(&this->graphicsObject, source.graphicsObject);
			return *this;
		}
	};
	std::vector<GraphicsObjectDivisions> objects;

	// derived class (sphere, cone etc.) must implement, return accessed object for divisions
	virtual GT_object *createGraphicsObject(int circleDivisions) = 0;

protected:

	Cmiss_glyph_circular()
	{
	}

	virtual ~Cmiss_glyph_circular()
	{
	}

public:

	virtual bool usesCircleDivisions()
	{
		return true;
	}

	virtual GT_object *getGraphicsObject(Cmiss_tessellation *, Cmiss_graphics_material *, Cmiss_font *);

};

struct Cmiss_glyph_arrow_solid : public Cmiss_glyph_circular
{
private:
	double headLength;
	double shaftThickness;

	Cmiss_glyph_arrow_solid(double headLengthIn, double shaftThicknessIn) :
		headLength(headLengthIn),
		shaftThickness(shaftThicknessIn)
	{
	}

	virtual GT_object *createGraphicsObject(int circleDivisions);

public:
	static Cmiss_glyph_arrow_solid* create(double headLengthIn, double shaftThicknessIn)
	{
		return new Cmiss_glyph_arrow_solid(headLengthIn, shaftThicknessIn);
	}
};

struct Cmiss_glyph_cone : public Cmiss_glyph_circular
{
private:
	virtual GT_object *createGraphicsObject(int circleDivisions);

public:
	static Cmiss_glyph_cone* create()
	{
		return new Cmiss_glyph_cone();
	}
};

struct Cmiss_glyph_cone_solid : public Cmiss_glyph_circular
{
private:
	virtual GT_object *createGraphicsObject(int circleDivisions);

public:
	static Cmiss_glyph_cone_solid* create()
	{
		return new Cmiss_glyph_cone_solid();
	}
};

struct Cmiss_glyph_cylinder : public Cmiss_glyph_circular
{
private:
	virtual GT_object *createGraphicsObject(int circleDivisions);

public:
	static Cmiss_glyph_cylinder* create()
	{
		return new Cmiss_glyph_cylinder();
	}
};

struct Cmiss_glyph_cylinder_solid : public Cmiss_glyph_circular
{
private:
	virtual GT_object *createGraphicsObject(int circleDivisions);

public:
	static Cmiss_glyph_cylinder_solid* create()
	{
		return new Cmiss_glyph_cylinder_solid();
	}
};

struct Cmiss_glyph_sphere : public Cmiss_glyph_circular
{
private:
	virtual GT_object *createGraphicsObject(int circleDivisions);

public:
	static Cmiss_glyph_sphere* create()
	{
		return new Cmiss_glyph_sphere();
	}
};

/**
 * Creates a graphics object named <name> resembling an arrow made from a cone on
 * a cylinder. The base of the arrow is at (0,0,0) while its head lies at (1,0,0).
 * The radius of the cone is <cone_radius>. The cylinder is <shaft_length> long
 * with its radius given by <shaft_radius>. The ends of the arrow and the cone
 * are both closed.  Primary axis is either 1,2 or 3 and indicates the direction
 * the arrow points in.
 */
struct GT_object *create_GT_object_arrow_solid(const char *name, int primary_axis,
	int number_of_segments_around,ZnReal shaft_length,ZnReal shaft_radius,
	ZnReal cone_radius);

/**
 * Creates a graphics object named <name> resembling a sphere with the given
 * <number_of_segments_around> and <number_of_segments_down> from pole to pole.
 * The sphere is centred at (0,0,0) and its poles are on the (1,0,0) line. It
 * fits into the unit cube spanning from -0.5 to +0.5 across all axes. Parameter
 * <number_of_segments_around> should normally be an even number at least 6 and
 * twice <number_of_segments_down> look remotely spherical.
 */
struct GT_object *create_GT_object_sphere(const char *name,int number_of_segments_around,
	int number_of_segments_down);

#endif /* !defined (GLYPH_CIRCULAR_HPP) */
