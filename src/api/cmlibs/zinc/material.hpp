/**
 * @file material.hpp
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_MATERIAL_HPP__
#define CMZN_MATERIAL_HPP__

#include "cmlibs/zinc/material.h"
#include "cmlibs/zinc/context.hpp"
#include "cmlibs/zinc/field.hpp"
#include "cmlibs/zinc/shader.hpp"

namespace CMLibs
{
namespace Zinc
{

class Materialmodulenotifier;

class Material
{

protected:
	cmzn_material_id id;

public:

	Material() : id(0)
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit Material(cmzn_material_id material_id) :
		id(material_id)
	{ }

	Material(const Material& material) :
		id(cmzn_material_access(material.id))
	{ }

	Material& operator=(const Material& material)
	{
		cmzn_material_id temp_id = cmzn_material_access(material.id);
		if (0 != id)
		{
			cmzn_material_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~Material()
	{
		if (0 != id)
		{
			cmzn_material_destroy(&id);
		}
	}

	bool isValid() const
	{
		return (0 != id);
	}

	cmzn_material_id getId() const
	{
		return id;
	}

	enum Attribute
	{
		ATTRIBUTE_INVALID = CMZN_MATERIAL_ATTRIBUTE_INVALID,
		ATTRIBUTE_ALPHA = CMZN_MATERIAL_ATTRIBUTE_ALPHA,
		ATTRIBUTE_AMBIENT = CMZN_MATERIAL_ATTRIBUTE_AMBIENT,
		ATTRIBUTE_DIFFUSE = CMZN_MATERIAL_ATTRIBUTE_DIFFUSE,
		ATTRIBUTE_EMISSION = CMZN_MATERIAL_ATTRIBUTE_EMISSION,
		ATTRIBUTE_SHININESS = CMZN_MATERIAL_ATTRIBUTE_SHININESS,
		ATTRIBUTE_SPECULAR = CMZN_MATERIAL_ATTRIBUTE_SPECULAR
	};

	enum ChangeFlag
	{
		CHANGE_FLAG_NONE = CMZN_MATERIAL_CHANGE_FLAG_NONE,
		CHANGE_FLAG_ADD = CMZN_MATERIAL_CHANGE_FLAG_ADD,
		CHANGE_FLAG_REMOVE = CMZN_MATERIAL_CHANGE_FLAG_REMOVE,
		CHANGE_FLAG_IDENTIFIER = CMZN_MATERIAL_CHANGE_FLAG_IDENTIFIER,
		CHANGE_FLAG_DEFINITION = CMZN_MATERIAL_CHANGE_FLAG_DEFINITION,
		CHANGE_FLAG_FULL_RESULT = CMZN_MATERIAL_CHANGE_FLAG_FULL_RESULT,
		CHANGE_FLAG_FINAL = CMZN_MATERIAL_CHANGE_FLAG_FINAL
	};

	/**
	 * Type for passing logical OR of #ChangeFlag
	 * @see Materialmoduleevent::getMaterialChangeFlags
	 */
	typedef int ChangeFlags;

	bool isManaged() const
	{
		return cmzn_material_is_managed(id);
	}

	int setManaged(bool value)
	{
		return cmzn_material_set_managed(id, value);
	}

	double getAttributeReal(Attribute attribute) const
	{
		return cmzn_material_get_attribute_real(id,
			static_cast<cmzn_material_attribute>(attribute));
	}

	int setAttributeReal(Attribute attribute, double value)
	{
		return cmzn_material_set_attribute_real(id,
			static_cast<cmzn_material_attribute>(attribute), value);
	}

	int getAttributeReal3(Attribute attribute, double *valuesOut3) const
	{
		return cmzn_material_get_attribute_real3(id,
			static_cast<cmzn_material_attribute>(attribute), valuesOut3);
	}

	int setAttributeReal3(Attribute attribute, const double *valuesIn3)
	{
		return cmzn_material_set_attribute_real3(id,
			static_cast<cmzn_material_attribute>(attribute), valuesIn3);
	}

	char *getName() const
	{
		return cmzn_material_get_name(id);
	}

	int setName(const char *name)
	{
		return cmzn_material_set_name(id, name);
	}

	Field getTextureField(int textureNumber) const
	{
		return Field(cmzn_material_get_texture_field(id, textureNumber));
	}

	int setTextureField(int textureNumber, const Field& textureField)
	{
		return cmzn_material_set_texture_field(id, textureNumber, textureField.getId());
	}

	Shaderuniforms getShaderuniforms() const
	{
		return Shaderuniforms(cmzn_material_get_shaderuniforms(id));
	}

	int setShaderuniforms(const Shaderuniforms& shaderuniforms)
	{
		return cmzn_material_set_shaderuniforms(id, shaderuniforms.getId());
	}

	Shaderprogram getShaderprogram() const
	{
		return Shaderprogram(cmzn_material_get_shaderprogram(id));
	}

	int setShaderprogram(const Shaderprogram& shaderprogram)
	{
		return cmzn_material_set_shaderprogram(id, shaderprogram.getId());
	}


};

inline bool operator==(const Material& a, const Material& b)
{
	return a.getId() == b.getId();
}

class Materialiterator
{
private:

	cmzn_materialiterator_id id;

public:

	Materialiterator() : id(0)
	{ }

	// takes ownership of C handle, responsibility for destroying it
	explicit Materialiterator(cmzn_materialiterator_id iterator_id) :
		id(iterator_id)
	{ }

	Materialiterator(const Materialiterator& materialiterator) :
		id(cmzn_materialiterator_access(materialiterator.id))
	{ }

	Materialiterator& operator=(const Materialiterator& materialiterator)
	{
		cmzn_materialiterator_id temp_id = cmzn_materialiterator_access(materialiterator.id);
		if (0 != id)
		{
			cmzn_materialiterator_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~Materialiterator()
	{
		if (0 != id)
		{
			cmzn_materialiterator_destroy(&id);
		}
	}

	bool isValid() const
	{
		return (0 != id);
	}

	Material next()
	{
		return Material(cmzn_materialiterator_next(id));
	}
};

class Materialmodule
{
protected:
	cmzn_materialmodule_id id;

public:

	Materialmodule() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit Materialmodule(cmzn_materialmodule_id in_materialmodule_id) :
		id(in_materialmodule_id)
	{  }

	Materialmodule(const Materialmodule& materialModule) :
		id(cmzn_materialmodule_access(materialModule.id))
	{  }

	Materialmodule& operator=(const Materialmodule& materialModule)
	{
		cmzn_materialmodule_id temp_id = cmzn_materialmodule_access(
			materialModule.id);
		if (0 != id)
		{
			cmzn_materialmodule_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~Materialmodule()
	{
		if (0 != id)
		{
			cmzn_materialmodule_destroy(&id);
		}
	}

	bool isValid() const
	{
		return (0 != id);
	}

	cmzn_materialmodule_id getId() const
	{
		return id;
	}

	Material createMaterial()
	{
		return Material(cmzn_materialmodule_create_material(id));
	}

	Materialiterator createMaterialiterator()
	{
		return Materialiterator(cmzn_materialmodule_create_materialiterator(id));
	}

	Material findMaterialByName(const char *name) const
	{
		return Material(cmzn_materialmodule_find_material_by_name(id, name));
	}

	int beginChange()
	{
		return cmzn_materialmodule_begin_change(id);
	}

	int endChange()
	{
		return cmzn_materialmodule_end_change(id);
	}

	int defineStandardMaterials()
	{
		return cmzn_materialmodule_define_standard_materials(id);
	}

	Context getContext() const
	{
		return Context(cmzn_materialmodule_get_context(id));
	}

	Material getDefaultMaterial() const
	{
		return Material(cmzn_materialmodule_get_default_material(id));
	}

	int setDefaultMaterial(const Material& material)
	{
		return cmzn_materialmodule_set_default_material(id, material.getId());
	}

	Material getDefaultSelectedMaterial() const
	{
		return Material(cmzn_materialmodule_get_default_selected_material(id));
	}

	int setDefaultSelectedMaterial(const Material& material)
	{
		return cmzn_materialmodule_set_default_selected_material(id, material.getId());
	}

	Material getDefaultSurfaceMaterial() const
	{
		return Material(cmzn_materialmodule_get_default_surface_material(id));
	}

	int setDefaultSurfaceMaterial(const Material& material)
	{
		return cmzn_materialmodule_set_default_surface_material(id, material.getId());
	}

	int readDescription(const char* description)
	{
		return cmzn_materialmodule_read_description(this->id, description);
	}

	char* writeDescription() const
	{
		return cmzn_materialmodule_write_description(this->id);
	}

	inline Materialmodulenotifier createMaterialmodulenotifier();

};

class Materialmoduleevent
{
protected:
	cmzn_materialmoduleevent_id id;

public:

	Materialmoduleevent() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit Materialmoduleevent(cmzn_materialmoduleevent_id in_materialmodule_event_id) :
		id(in_materialmodule_event_id)
	{  }

	Materialmoduleevent(const Materialmoduleevent& materialmoduleEvent) :
		id(cmzn_materialmoduleevent_access(materialmoduleEvent.id))
	{  }

	Materialmoduleevent& operator=(const Materialmoduleevent& materialmoduleEvent)
	{
		cmzn_materialmoduleevent_id temp_id = cmzn_materialmoduleevent_access(materialmoduleEvent.id);
		if (0 != id)
			cmzn_materialmoduleevent_destroy(&id);
		id = temp_id;
		return *this;
	}

	~Materialmoduleevent()
	{
		if (0 != id)
		{
			cmzn_materialmoduleevent_destroy(&id);
		}
	}

	bool isValid() const
	{
		return (0 != id);
	}

	cmzn_materialmoduleevent_id getId() const
	{
		return id;
	}

	Material::ChangeFlags getMaterialChangeFlags(const Material& material) const
	{
		return cmzn_materialmoduleevent_get_material_change_flags(id, material.getId());
	}

	Material::ChangeFlags getSummaryMaterialChangeFlags() const
	{
		return cmzn_materialmoduleevent_get_summary_material_change_flags(id);
	}

};

/**
 * @brief Base class functor for material module notifier callbacks
 *
 * Base class functor for material module notifier callbacks:
 * - Derive from this class adding any user data required.
 * - Implement virtual operator()(const Materialmoduleevent&) to handle callback.
 * @see Materialmodulenotifier::setCallback()
 */
class Materialmodulecallback
{
	friend class Materialmodulenotifier;
private:
	Materialmodulecallback(const Materialmodulecallback&); // not implemented
	Materialmodulecallback& operator=(const Materialmodulecallback&); // not implemented

	static void C_callback(cmzn_materialmoduleevent_id materialmoduleevent_id, void *callbackVoid)
	{
		Materialmoduleevent materialmoduleevent(cmzn_materialmoduleevent_access(materialmoduleevent_id));
		Materialmodulecallback *callback = reinterpret_cast<Materialmodulecallback *>(callbackVoid);
		(*callback)(materialmoduleevent);
	}

	virtual void operator()(const Materialmoduleevent &materialmoduleevent) = 0;

protected:
	Materialmodulecallback()
	{ }

public:
	virtual ~Materialmodulecallback()
	{ }
};

class Materialmodulenotifier
{
protected:
	cmzn_materialmodulenotifier_id id;

public:

	Materialmodulenotifier() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit Materialmodulenotifier(cmzn_materialmodulenotifier_id in_materialmodulenotifier_id) :
		id(in_materialmodulenotifier_id)
	{  }

	Materialmodulenotifier(const Materialmodulenotifier& materialmoduleNotifier) :
		id(cmzn_materialmodulenotifier_access(materialmoduleNotifier.id))
	{  }

	Materialmodulenotifier& operator=(const Materialmodulenotifier& materialmoduleNotifier)
	{
		cmzn_materialmodulenotifier_id temp_id = cmzn_materialmodulenotifier_access(materialmoduleNotifier.id);
		if (0 != id)
		{
			cmzn_materialmodulenotifier_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~Materialmodulenotifier()
	{
		if (0 != id)
		{
			cmzn_materialmodulenotifier_destroy(&id);
		}
	}

	bool isValid() const
	{
		return (0 != id);
	}

	cmzn_materialmodulenotifier_id getId() const
	{
		return id;
	}

	int setCallback(Materialmodulecallback& callback)
	{
		return cmzn_materialmodulenotifier_set_callback(id, callback.C_callback, static_cast<void*>(&callback));
	}

	int clearCallback()
	{
		return cmzn_materialmodulenotifier_clear_callback(id);
	}
};

inline Materialmodulenotifier Materialmodule::createMaterialmodulenotifier()
{
	return Materialmodulenotifier(cmzn_materialmodule_create_materialmodulenotifier(id));
}

inline Materialmodule Context::getMaterialmodule() const
{
	return Materialmodule(cmzn_context_get_materialmodule(id));
}

}
}
#endif
