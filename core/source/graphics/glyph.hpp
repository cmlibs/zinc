/**
 * FILE : glyph.hpp
 *
 * Internal header for private glyph functions.
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
 * Portions created by the Initial Developer are Copyright (C) 2005
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
#if !defined (GLYPH_HPP)
#define GLYPH_HPP

#include "zinc/glyph.h"
#include "general/cmiss_set.hpp"
#include "general/enumerator.h"
#include "general/indexed_list_stl_private.hpp"
#include "general/manager.h"
#include "graphics/auxiliary_graphics_types.h"
#include "graphics/graphics_object.h"

PROTOTYPE_OBJECT_FUNCTIONS(Cmiss_glyph);

DECLARE_LIST_TYPES(Cmiss_glyph);

DECLARE_MANAGER_TYPES(Cmiss_glyph);

PROTOTYPE_GET_OBJECT_NAME_FUNCTION(Cmiss_glyph);

PROTOTYPE_LIST_FUNCTIONS(Cmiss_glyph);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Cmiss_glyph,name,const char *);

PROTOTYPE_MANAGER_FUNCTIONS(Cmiss_glyph);
PROTOTYPE_MANAGER_IDENTIFIER_FUNCTIONS(Cmiss_glyph,name,const char *);

struct Cmiss_glyph
{
	const char *name;
	struct MANAGER(Cmiss_glyph) *manager;
	int manager_change_status;
	bool isManagedFlag;
	int access_count;
private:
	Cmiss_glyph_type type;

protected:

	Cmiss_glyph() :
		name(0),
		manager(0),
		manager_change_status(0),
		isManagedFlag(false),
		access_count(1),
		type(CMISS_GLYPH_TYPE_INVALID)
	{
	}

	virtual ~Cmiss_glyph();

public:

	inline Cmiss_glyph *access()
	{
		++access_count;
		return this;
	}

	static inline int deaccess(Cmiss_glyph **glyphAddress)
	{
		int return_code;
		struct Cmiss_glyph *glyph;
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
				(MANAGER_CHANGE_NONE(Cmiss_glyph) != glyph->manager_change_status))))
			{
				return_code = REMOVE_OBJECT_FROM_MANAGER(Cmiss_glyph)(glyph, glyph->manager);
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
			MANAGED_OBJECT_CHANGE(Cmiss_glyph)(this, MANAGER_CHANGE_NOT_RESULT(Cmiss_glyph));
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

	// override if glyph uses the font supplied by graphic
	virtual bool usesFont()
	{
		return false;
	}

	virtual void spectrumChange(struct MANAGER_MESSAGE(Spectrum) *)
	{
	}

	virtual void timeChange()
	{
	}

	void changed()
	{
		MANAGED_OBJECT_CHANGE(Cmiss_glyph)(this, MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Cmiss_glyph));
	}

	/** return ACCESSed pointer to graphics object */
	virtual GT_object *getGraphicsObject(Cmiss_tessellation *, Cmiss_graphics_material *, Cmiss_font *) = 0;

	const char *getName() const
	{
		return this->name;
	}

	int setName(const char *newName);

	Cmiss_glyph_type getType() const
	{
		return this->type;
	}

	/** Set type metadata as alternative means to identify glyph.
	 * Should ensure no two glyphs have the same type. */
	void setType(Cmiss_glyph_type typeIn)
	{
		this->type = typeIn;
	}
};

/* Only to be used from FIND_BY_IDENTIFIER_IN_INDEXED_LIST_STL function
 * Creates a pseudo object with name identifier suitable for finding
 * objects by identifier with Cmiss_set.
 */
class Cmiss_glyph_identifier : private Cmiss_glyph
{
public:
	Cmiss_glyph_identifier(const char *nameIn)
	{
		this->name = nameIn;
	}

	~Cmiss_glyph_identifier()
	{
		this->name = 0;
	}

	virtual GT_object *getGraphicsObject(Cmiss_tessellation *, Cmiss_graphics_material *, Cmiss_font *)
	{
		return 0;
	}

	Cmiss_glyph *getPseudoObject()
	{
		return this;
	}
};

/** functor for ordering Cmiss_set<Cmiss_glyph> by name */
struct Cmiss_glyph_compare_name_functor
{
	bool operator() (const Cmiss_glyph* glyph1, const Cmiss_glyph* glyph2) const
	{
		return strcmp(glyph1->name, glyph2->name) < 0;
	}
};

typedef Cmiss_set<Cmiss_glyph *,Cmiss_glyph_compare_name_functor> Cmiss_set_Cmiss_glyph;

struct Cmiss_glyph_static : public Cmiss_glyph
{
private:
	GT_object *graphicsObject;

	Cmiss_glyph_static(GT_object *graphicsObjectIn) :
		graphicsObject(ACCESS(GT_object)(graphicsObjectIn))
	{
	}

	virtual ~Cmiss_glyph_static()
	{
		DEACCESS(GT_object)(&graphicsObject);
	}

public:

	static Cmiss_glyph_static* create(GT_object *graphicsObjectIn)
	{
		if (graphicsObjectIn)
			return new Cmiss_glyph_static(graphicsObjectIn);
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

	virtual GT_object *getGraphicsObject(Cmiss_tessellation *, Cmiss_graphics_material *, Cmiss_font *)
	{
		return ACCESS(GT_object)(graphicsObject);
	}

	GT_object *getGraphicsObject()
	{
		return ACCESS(GT_object)(graphicsObject);
	}

};

struct Cmiss_glyph_module
{
private:
	Cmiss_graphics_material_module *materialModule;
	struct MANAGER(Cmiss_glyph) *manager;
	Cmiss_glyph *defaultPointGlyph;
	int access_count;

	Cmiss_glyph_module(Cmiss_graphics_material_module *materialModuleIn);
	~Cmiss_glyph_module();

	void defineGlyph(const char *name, Cmiss_glyph *glyph, Cmiss_glyph_type type);

	bool defineGlyphStatic(GT_object*& graphicsObject, Cmiss_glyph_type type);

public:

	static Cmiss_glyph_module *create(Cmiss_graphics_material_module *materialModuleIn)
	{
		return new Cmiss_glyph_module(materialModuleIn);
	}

	Cmiss_glyph_module *access()
	{
		++access_count;
		return this;
	}

	static void deaccess(Cmiss_glyph_module* &glyph_module)
	{
		if (glyph_module)
		{
			--(glyph_module->access_count);
			if (glyph_module->access_count <= 0)
			{
				delete glyph_module;
			}
			glyph_module = 0;
		}
	}

	struct MANAGER(Cmiss_glyph) *getManager()
	{
		return manager;
	}

	int beginChange()
	{
		return MANAGER_BEGIN_CACHE(Cmiss_glyph)(this->manager);
	}

	int endChange()
	{
		return MANAGER_END_CACHE(Cmiss_glyph)(this->manager);
	}

	int defineStandardGlyphs();

	int defineStandardCmguiGlyphs();

	Cmiss_set_Cmiss_glyph *getGlyphListPrivate();

	/** @return non-ACCESSed pointer to glyph, or 0 if no match */
	Cmiss_glyph *findGlyphByName(const char *name)
	{
		return FIND_BY_IDENTIFIER_IN_MANAGER(Cmiss_glyph,name)(name, this->manager);
	}

	/** @return non-ACCESSed pointer to glyph, or 0 if no match */
	Cmiss_glyph *findGlyphByType(enum Cmiss_glyph_type glyph_type);

	/** adds glyph to manager, ensuring it has a unique name */
	void addGlyph(Cmiss_glyph *glyph);

	Cmiss_glyph *getDefaultPointGlyph()
	{
		if (this->defaultPointGlyph)
			return this->defaultPointGlyph->access();
		return 0;
	}

	void setDefaultPointGlyph(Cmiss_glyph *glyph)
	{
		REACCESS(Cmiss_glyph)(&this->defaultPointGlyph, glyph);
	}

};

PROTOTYPE_ENUMERATOR_FUNCTIONS(Cmiss_glyph_repeat_mode);

int Cmiss_glyph_repeat_mode_get_number_of_glyphs(
	enum Cmiss_glyph_repeat_mode glyph_repeat_mode);

bool Cmiss_glyph_repeat_mode_glyph_number_has_label(
	enum Cmiss_glyph_repeat_mode glyph_repeat_mode, int glyph_number);

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
	enum Cmiss_glyph_repeat_mode glyph_repeat_mode, int glyph_number,
	Triple base_size, Triple scale_factors, Triple offset,
	Triple point, Triple axis1, Triple axis2, Triple axis3, Triple scale,
	Triple final_point, Triple final_axis1, Triple final_axis2, Triple final_axis3);

/** internal only */
Cmiss_glyph_module_id Cmiss_glyph_module_create(Cmiss_graphics_material_module *materialModule);

/* internal only */
struct MANAGER(Cmiss_glyph) *Cmiss_glyph_module_get_manager(
	Cmiss_glyph_module_id glyph_module);

/**
 * Create extra glyphs such as line_ticks, diamond, only used in cmgui
 */
int Cmiss_glyph_module_define_standard_cmgui_glyphs(
	Cmiss_glyph_module_id glyph_module);

/**
 * Internal only.
 * @return  Handle to new glyph wrapping graphics object. Up to caller to destroy.
 */
Cmiss_glyph *Cmiss_glyph_module_create_glyph_static(
	Cmiss_glyph_module_id glyphModule, GT_object *graphicsObject);

#endif /* !defined (GLYPH_HPP) */
