/**
 * FILE : glyph.hpp
 *
 * Internal header for private glyph functions.
 */
 /* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (GLYPH_HPP)
#define GLYPH_HPP

#include "zinc/glyph.h"
#include "general/cmiss_set.hpp"
#include "general/enumerator.h"
#include "general/indexed_list_stl_private.hpp"
#include "general/manager.h"
#include "graphics/auxiliary_graphics_types.h"
#include "graphics/graphics_object.h"

PROTOTYPE_OBJECT_FUNCTIONS(cmzn_glyph);

DECLARE_LIST_TYPES(cmzn_glyph);

DECLARE_MANAGER_TYPES(cmzn_glyph);

PROTOTYPE_GET_OBJECT_NAME_FUNCTION(cmzn_glyph);

PROTOTYPE_LIST_FUNCTIONS(cmzn_glyph);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(cmzn_glyph,name,const char *);

PROTOTYPE_MANAGER_FUNCTIONS(cmzn_glyph);
PROTOTYPE_MANAGER_IDENTIFIER_FUNCTIONS(cmzn_glyph,name,const char *);

struct cmzn_glyph
{
	const char *name;
	struct MANAGER(cmzn_glyph) *manager;
	int manager_change_status;
	bool isManagedFlag;
	int access_count;
private:
	cmzn_glyph_shape_type type;

protected:

	cmzn_glyph() :
		name(0),
		manager(0),
		manager_change_status(0),
		isManagedFlag(false),
		access_count(1),
		type(CMZN_GLYPH_SHAPE_TYPE_INVALID)
	{
	}

	virtual ~cmzn_glyph();

public:

	inline cmzn_glyph *access()
	{
		++access_count;
		return this;
	}

	static inline int deaccess(cmzn_glyph **glyphAddress)
	{
		int return_code;
		struct cmzn_glyph *glyph;
		if (glyphAddress && (glyph = *glyphAddress))
		{
			--(glyph->access_count);
			if (glyph->access_count <= 0)
			{
				delete glyph;
				return_code = 1;
			}
			else if ((!glyph->isManagedFlag) && (glyph->manager) &&
			((1 == glyph->access_count) || ((2 == glyph->access_count) &&
				(MANAGER_CHANGE_NONE(cmzn_glyph) != glyph->manager_change_status))))
			{
				return_code = REMOVE_OBJECT_FROM_MANAGER(cmzn_glyph)(glyph, glyph->manager);
			}
			else
			{
				return_code = 1;
			}
			*glyphAddress = 0;
		}
		else
		{
			return_code = 0;
		}
		return (return_code);
	}

	bool isManaged()
	{
		return isManagedFlag;
	}

	void setManaged(bool isManagedFlagIn)
	{
		if (isManagedFlagIn != this->isManagedFlag)
		{
			this->isManagedFlag = isManagedFlagIn;
			MANAGED_OBJECT_CHANGE(cmzn_glyph)(this, MANAGER_CHANGE_DEFINITION(cmzn_glyph));
		}
	}

	virtual bool isTimeVarying()
	{
		return false;
	}

	virtual bool usesCircleDivisions()
	{
		return false;
	}

	// override if glyph uses the font supplied by graphics
	virtual bool usesFont()
	{
		return false;
	}

	// override for glyphs that use the font from the graphics
	virtual void fontChange()
	{
	}

	// override for glyphs that use their own materials
	virtual void materialChange(struct MANAGER_MESSAGE(Graphical_material) *)
	{
	}

	// override for glyph using spectrum, e.g. colour_bar
	virtual void spectrumChange(struct MANAGER_MESSAGE(cmzn_spectrum) *)
	{
	}

	virtual void timeChange()
	{
	}

	void changed()
	{
		MANAGED_OBJECT_CHANGE(cmzn_glyph)(this, MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(cmzn_glyph));
	}

	/** return ACCESSed pointer to graphics object */
	virtual GT_object *getGraphicsObject(cmzn_tessellation *, cmzn_material *, cmzn_font *) = 0;

	const char *getName() const
	{
		return this->name;
	}

	int setName(const char *newName);

	cmzn_glyph_shape_type getType() const
	{
		return this->type;
	}

	/** Set type metadata as alternative means to identify glyph.
	 * Should ensure no two glyphs have the same type. */
	void setType(cmzn_glyph_shape_type typeIn)
	{
		this->type = typeIn;
	}
};

/* Only to be used from FIND_BY_IDENTIFIER_IN_INDEXED_LIST_STL function
 * Creates a pseudo object with name identifier suitable for finding
 * objects by identifier with cmzn_set.
 */
class cmzn_glyph_identifier : private cmzn_glyph
{
public:
	cmzn_glyph_identifier(const char *nameIn)
	{
		this->name = nameIn;
	}

	~cmzn_glyph_identifier()
	{
		this->name = 0;
	}

	virtual GT_object *getGraphicsObject(cmzn_tessellation *, cmzn_material *, cmzn_font *)
	{
		return 0;
	}

	cmzn_glyph *getPseudoObject()
	{
		return this;
	}
};

/** functor for ordering cmzn_set<cmzn_glyph> by name */
struct cmzn_glyph_compare_name_functor
{
	bool operator() (const cmzn_glyph* glyph1, const cmzn_glyph* glyph2) const
	{
		return strcmp(glyph1->name, glyph2->name) < 0;
	}
};

typedef cmzn_set<cmzn_glyph *,cmzn_glyph_compare_name_functor> cmzn_set_cmzn_glyph;

struct cmzn_glyph_static : public cmzn_glyph
{
private:
	GT_object *graphicsObject;

	cmzn_glyph_static(GT_object *graphicsObjectIn) :
		graphicsObject(ACCESS(GT_object)(graphicsObjectIn))
	{
	}

	virtual ~cmzn_glyph_static()
	{
		DEACCESS(GT_object)(&graphicsObject);
	}

public:

	static cmzn_glyph_static* create(GT_object *graphicsObjectIn)
	{
		if (graphicsObjectIn)
			return new cmzn_glyph_static(graphicsObjectIn);
		return 0;
	}

	virtual bool isTimeVarying()
	{
		return (1 < GT_object_get_number_of_times(graphicsObject));
	}

	virtual void timeChange()
	{
		GT_object_time_change(graphicsObject);
	}

	virtual GT_object *getGraphicsObject(cmzn_tessellation *, cmzn_material *, cmzn_font *)
	{
		return ACCESS(GT_object)(graphicsObject);
	}

	GT_object *getGraphicsObject()
	{
		return ACCESS(GT_object)(graphicsObject);
	}

	virtual void materialChange(struct MANAGER_MESSAGE(Graphical_material) *message);
};

struct cmzn_glyphmodule
{
private:
	cmzn_materialmodule *materialModule;
	struct MANAGER(cmzn_glyph) *manager;
	cmzn_glyph *defaultPointGlyph;
	int access_count;

	cmzn_glyphmodule(cmzn_materialmodule *materialModuleIn);
	~cmzn_glyphmodule();

	void defineGlyph(const char *name, cmzn_glyph *glyph, cmzn_glyph_shape_type type);

	bool defineGlyphStatic(GT_object*& graphicsObject, cmzn_glyph_shape_type type);

public:

	static cmzn_glyphmodule *create(cmzn_materialmodule *materialModuleIn)
	{
		return new cmzn_glyphmodule(materialModuleIn);
	}

	cmzn_glyphmodule *access()
	{
		++access_count;
		return this;
	}

	static void deaccess(cmzn_glyphmodule* &glyphmodule)
	{
		if (glyphmodule)
		{
			--(glyphmodule->access_count);
			if (glyphmodule->access_count <= 0)
			{
				delete glyphmodule;
			}
			glyphmodule = 0;
		}
	}

	struct MANAGER(cmzn_glyph) *getManager()
	{
		return manager;
	}

	int beginChange()
	{
		return MANAGER_BEGIN_CACHE(cmzn_glyph)(this->manager);
	}

	int endChange()
	{
		return MANAGER_END_CACHE(cmzn_glyph)(this->manager);
	}

	int defineStandardGlyphs();

	int defineStandardCmguiGlyphs();

	cmzn_set_cmzn_glyph *getGlyphListPrivate();

	/** @return non-ACCESSed pointer to glyph, or 0 if no match */
	cmzn_glyph *findGlyphByName(const char *name)
	{
		return FIND_BY_IDENTIFIER_IN_MANAGER(cmzn_glyph,name)(name, this->manager);
	}

	/** @return non-ACCESSed pointer to glyph, or 0 if no match */
	cmzn_glyph *findGlyphByType(enum cmzn_glyph_shape_type glyph_type);

	/** adds glyph to manager, ensuring it has a unique name */
	void addGlyph(cmzn_glyph *glyph);

	cmzn_glyph *getDefaultPointGlyph()
	{
		if (this->defaultPointGlyph)
			return this->defaultPointGlyph->access();
		return 0;
	}

	void setDefaultPointGlyph(cmzn_glyph *glyph)
	{
		REACCESS(cmzn_glyph)(&this->defaultPointGlyph, glyph);
	}

};

PROTOTYPE_ENUMERATOR_FUNCTIONS(cmzn_glyph_repeat_mode);

int cmzn_glyph_repeat_mode_get_number_of_glyphs(
	enum cmzn_glyph_repeat_mode glyph_repeat_mode);

bool cmzn_glyph_repeat_mode_glyph_number_has_label(
	enum cmzn_glyph_repeat_mode glyph_repeat_mode, int glyph_number);

/**
 * @param glyph_repeat_mode  NONE|MIRROR|AXES_2D|AXES_3D
 * @param glyph_number  From 0 to one less than number for glyph_repeat_mode.
 * @param base_size  Fixed minimum size to add to scale (after scale factors)
 * @param scale_factors  Scale factors to apply to scale.
 * @param offset  Offset of base point in axes*scale units.
 * Multiplies the three axes by their <scale> to give the final axes, reversing
 * <final_axis3> if necessary to produce a right handed coordinate system.
 */
void resolve_glyph_axes(
	enum cmzn_glyph_repeat_mode glyph_repeat_mode, int glyph_number,
	Triple base_size, Triple scale_factors, Triple offset,
	Triple point, Triple axis1, Triple axis2, Triple axis3, Triple scale,
	Triple final_point, Triple final_axis1, Triple final_axis2, Triple final_axis3);

/** internal only */
cmzn_glyphmodule_id cmzn_glyphmodule_create(cmzn_materialmodule *materialModule);

/* internal only */
struct MANAGER(cmzn_glyph) *cmzn_glyphmodule_get_manager(
	cmzn_glyphmodule_id glyphmodule);

/**
 * Create extra glyphs such as line_ticks, diamond, only used in cmgui
 */
int cmzn_glyphmodule_define_standard_cmgui_glyphs(
	cmzn_glyphmodule_id glyphmodule);

/**
 * Internal only.
 * @return  Handle to new glyph wrapping graphics object. Up to caller to destroy.
 */
cmzn_glyph *cmzn_glyphmodule_create_glyph_static(
	cmzn_glyphmodule_id glyphModule, GT_object *graphicsObject);

#endif /* !defined (GLYPH_HPP) */
