/**
 * FILE : material_json_io.cpp
 *
 * Implementation of material serialisation in JSON format.
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "description_io/material_json_io.hpp"
#include "general/debug.h"
#include "opencmiss/zinc/changemanager.hpp"
#include "opencmiss/zinc/material.hpp"
#include "opencmiss/zinc/result.hpp"

using namespace OpenCMISS::Zinc;

namespace {

struct AttributeToken
{
	const char* token;
	Material::Attribute attribute;
};
	
const AttributeToken rgbAttributeTokens[4] =
{
	{ "Ambient", Material::ATTRIBUTE_AMBIENT },
	{ "Diffuse", Material::ATTRIBUTE_DIFFUSE },
	{ "Emission", Material::ATTRIBUTE_EMISSION },
	{ "Specular", Material::ATTRIBUTE_SPECULAR }
};

const AttributeToken realAttributeTokens[4] =
{
	{ "Alpha", Material::ATTRIBUTE_ALPHA },
	{ "Shininess", Material::ATTRIBUTE_SHININESS }
};

struct MaterialmoduleGetSetMaterialToken
{
	const char* token;
	Material (Materialmodule::* getMaterial)() const;
	int (Materialmodule::* setMaterial)(const Material&);
};

MaterialmoduleGetSetMaterialToken materialmoduleGetSetDefaultMaterialTokens[3] =
{
	{ "DefaultMaterial", &Materialmodule::getDefaultMaterial, &Materialmodule::setDefaultMaterial },
	{ "DefaultSelectedMaterial", &Materialmodule::getDefaultSelectedMaterial, &Materialmodule::setDefaultSelectedMaterial },
	{ "DefaultSurfaceMaterial", &Materialmodule::getDefaultSurfaceMaterial, &Materialmodule::setDefaultSurfaceMaterial }
};

/** Write material attributes into json structure */
void writeMaterialToJson(const Material& material, Json::Value& materialSettings)
{
	double rgb[3];
	char* name = material.getName();
	materialSettings["Name"] = name;
	DEALLOCATE(name);
	for (int a = 0; a < 4; ++a)
	{
		const AttributeToken& attributeToken = rgbAttributeTokens[a];
		material.getAttributeReal3(attributeToken.attribute, rgb);
		for (int i = 0; i < 3; ++i)
			materialSettings[attributeToken.token].append(rgb[i]);
	}
	for (int a = 0; a < 2; ++a)
	{
		const AttributeToken& attributeToken = realAttributeTokens[a];
		materialSettings[attributeToken.token] = material.getAttributeReal(attributeToken.attribute);
	}
	// serialise image fields
}

void readMaterialFromJson(Material& material, const Json::Value& materialSettings)
{
	double rgb[3];
	// caller guarantees name is already set for existing or new material
	//if (materialSettings["Name"].isString())
	//	material.setName(materialSettings["Name"].asCString());
	for (int a = 0; a < 4; ++a)
	{
		const AttributeToken& attributeToken = rgbAttributeTokens[a];
		if (materialSettings[attributeToken.token].isArray())
		{
			for (unsigned int i = 0; i < 3; ++i)
				rgb[i] = materialSettings[attributeToken.token][i].asDouble();
			material.setAttributeReal3(attributeToken.attribute, rgb);
		}
	}
	for (int a = 0; a < 2; ++a)
	{
		const AttributeToken& attributeToken = realAttributeTokens[a];
		if (materialSettings[attributeToken.token].isDouble())
			material.setAttributeReal(attributeToken.attribute, materialSettings[attributeToken.token].asDouble());
	}
	material.setManaged(true);
	// deserialise image fields
}

};

std::string MaterialmoduleJsonExport::getExportString()
{
	Json::Value root;

	Materialiterator materialiterator = this->materialmodule.createMaterialiterator();
	Material material;
	while ((material = materialiterator.next()).isValid())
	{
		Json::Value materialSettings;
		writeMaterialToJson(material, materialSettings);
		root["Materials"].append(materialSettings);
	}
	for (int d = 0; d < 3; ++d)
	{
		MaterialmoduleGetSetMaterialToken& getSetMaterialToken = materialmoduleGetSetDefaultMaterialTokens[d];
		Material& material = (this->materialmodule.*getSetMaterialToken.getMaterial)();
		if (material.isValid())
		{
			char* materialName = material.getName();
			root[getSetMaterialToken.token] = materialName;
			DEALLOCATE(materialName);
		}
	}
	return Json::StyledWriter().write(root);
}

int MaterialmoduleJsonImport::import(const std::string &jsonString)
{
	Json::Value root;
	if (!(Json::Reader().parse(jsonString, root, true) && root.isObject()))
		return CMZN_RESULT_ERROR_ARGUMENT;
	ChangeManager<Materialmodule> changeMaterial(this->materialmodule);
	Json::Value materialsJson = root["Materials"];
	for (unsigned int index = 0; index < materialsJson.size(); ++index )
		this->importMaterial(materialsJson[index]);
	for (int d = 0; d < 3; ++d)
	{
		MaterialmoduleGetSetMaterialToken& getSetMaterialToken = materialmoduleGetSetDefaultMaterialTokens[d];
		Material& material = (root[getSetMaterialToken.token].isString()) ?
			this->materialmodule.findMaterialByName(root[getSetMaterialToken.token].asCString()) : Material();
		(this->materialmodule.*getSetMaterialToken.setMaterial)(material);
	}
	return CMZN_RESULT_OK;
}

void MaterialmoduleJsonImport::importMaterial(Json::Value &materialSettings)
{
	const char *materialName = materialSettings["Name"].asCString();
	Material material = this->materialmodule.findMaterialByName(materialName);
	if (!material.isValid())
	{
		material = this->materialmodule.createMaterial();
		material.setName(materialName);
	}
	readMaterialFromJson(material, materialSettings);
}
