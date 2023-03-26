/**
 * FILE : material_json_io.cpp
 *
 * Implementation of material serialisation in JSON format.
 *
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "description_io/material_json_io.hpp"
#include "general/message.h"
#include "opencmiss/zinc/core.h"
#include "opencmiss/zinc/changemanager.hpp"
#include "opencmiss/zinc/field.hpp"
#include "opencmiss/zinc/fieldmodule.hpp"
#include "opencmiss/zinc/material.hpp"
#include "opencmiss/zinc/region.hpp"
#include "opencmiss/zinc/result.hpp"
#include <string>

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

/** Get string path/to/region.
 * @param path  On return, contains the path.
 * @param rootRegion  On return, contains root region of path. */
void getRegionPath(const Region& region, std::string &path, Region &rootRegion)
{
	Region parentRegion = region.getParent();
	if (parentRegion.isValid())
	{
		getRegionPath(parentRegion, path, rootRegion);
		if (!(parentRegion == rootRegion))
			path += "/";
		char* name = region.getName();
		path += name;
		cmzn_deallocate(name);
	}
	else
	{
		path = "";
		rootRegion = region;
	}
}

/** Write material attributes into json structure
 * @return Result OK on success, WARNING_PART_DONE if texture
 * fields are not in a single region tree. */
int writeMaterialToJson(const Material& material, Json::Value& materialSettings, Region& rootRegion)
{
	int result = RESULT_OK;
	double rgb[3];
	char* name = material.getName();
	materialSettings["Name"] = name;
	cmzn_deallocate(name);
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
	// write texture fields
	int tLastTexture = 0;
	for (int t = 1; t <= 4; ++t)
	{
		Field textureField = material.getTextureField(t);
		if (textureField.isValid())
		{
			// fill null images between the last and current texture
			for (int u = tLastTexture + 1; u < t; ++u)
				materialSettings["TextureFields"].append(Json::Value());  // null_value
			const Region region = textureField.getFieldmodule().getRegion();
			std::string texturePath;
			Region thisRootRegion;
			getRegionPath(region, texturePath, thisRootRegion);
			if (rootRegion.isValid())
			{
				if (!(thisRootRegion == rootRegion))
				{
					display_message(WARNING_MESSAGE, "Materialmodule writeDescription:  "
						"Material %s references texture field %s from a different region tree. Cannot re-read.",
						materialSettings["Name"].asCString(), texturePath.c_str());
					result = RESULT_WARNING_PART_DONE;
				}
			}
			else
				rootRegion = thisRootRegion;
			if (!(region == rootRegion))
				texturePath += "/";
            name = textureField.getName();
            texturePath += name;
            cmzn_deallocate(name);
			materialSettings["TextureFields"].append(texturePath);
			tLastTexture = t;
		}
	}
	return result;
}

/** Read material attributes into json structure.
 * @param rootRegion  Root of region tree to find region/path/texture fields under.
 * @return Result OK on success, otherwise any error code, such as
 * ERROR_NOT_FOUND if texture fields not found within root region tree. */
int readMaterialFromJson(Material& material, const Json::Value& materialSettings, const Region& rootRegion)
{
	int result = RESULT_OK;
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
	// read texture fields
	if (materialSettings["TextureFields"].isArray())
	{
		unsigned int size = materialSettings["TextureFields"].size();
		// currently up to 4 textures allowed
		for (unsigned int t = 0; t < 4; ++t)
		{
			if ((t < size) && materialSettings["TextureFields"][t].isString())
			{
				Region region(rootRegion);
				std::string texturePath(materialSettings["TextureFields"][t].asCString());
				std::string regionPath;
				size_t pathEnd = texturePath.rfind('/');
				if (pathEnd != std::string::npos)
				{
					regionPath = texturePath.substr(0, pathEnd);
					texturePath.erase(0, pathEnd + 1);
					region = rootRegion.findSubregionAtPath(regionPath.c_str());
					if (!region.isValid())
					{
						display_message(WARNING_MESSAGE, "Materialmodule readDescription:  "
							"Material %s cannot find subregion %s containing texture field %u %s",
							materialSettings["Name"].asCString(), regionPath.c_str(), t + 1, texturePath.c_str());
						result = RESULT_ERROR_NOT_FOUND;
						continue;
					}
				}
				Field textureField = region.getFieldmodule().findFieldByName(texturePath.c_str());
				if (!textureField.isValid())
				{
					display_message(WARNING_MESSAGE, "Materialmodule readDescription:  "
						"Material %s cannot find texture field %u %s in region %s",
						materialSettings["Name"].asCString(), t + 1, texturePath.c_str(), regionPath.c_str());
					result = RESULT_ERROR_NOT_FOUND;
					continue;
				}
				const int result = material.setTextureField(t + 1, textureField);
				if (result != RESULT_OK)
				{
					display_message(WARNING_MESSAGE, "Materialmodule readDescription:  "
						"Material %s cannot set texture field %u %s in region %s.",
						materialSettings["Name"].asCString(), t + 1, texturePath.c_str(), regionPath.c_str());
				}
			}
			else
			{
				material.setTextureField(t + 1, Field());
			}
		}
	}
	return result;
}

};

std::string MaterialmoduleJsonExport::getExportString()
{
	Json::Value root;
	OpenCMISS::Zinc::Region rootRegion;

	Materialiterator materialiterator = this->materialmodule.createMaterialiterator();
	Material material;
	while ((material = materialiterator.next()).isValid())
	{
		Json::Value materialSettings;
		writeMaterialToJson(material, materialSettings, rootRegion);
		root["Materials"].append(materialSettings);
	}
	for (int d = 0; d < 3; ++d)
	{
		MaterialmoduleGetSetMaterialToken& getSetMaterialToken = materialmoduleGetSetDefaultMaterialTokens[d];
		material = (this->materialmodule.*getSetMaterialToken.getMaterial)();
		if (material.isValid())
		{
			char* materialName = material.getName();
			root[getSetMaterialToken.token] = materialName;
			cmzn_deallocate(materialName);
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
	int result = CMZN_RESULT_OK;
	for (unsigned int index = 0; index < materialsJson.size(); ++index)
	{
		const int materialResult = this->importMaterial(materialsJson[index]);
		if ((materialResult != CMZN_RESULT_OK) && (result == CMZN_RESULT_OK))
			result = materialResult;
	}
	for (int d = 0; d < 3; ++d)
	{
		MaterialmoduleGetSetMaterialToken& getSetMaterialToken = materialmoduleGetSetDefaultMaterialTokens[d];
		Material material = (root[getSetMaterialToken.token].isString()) ?
			this->materialmodule.findMaterialByName(root[getSetMaterialToken.token].asCString()) : Material();
		(this->materialmodule.*getSetMaterialToken.setMaterial)(material);
	}
	return result;
}

int MaterialmoduleJsonImport::importMaterial(Json::Value &materialSettings)
{
	const char *materialName = materialSettings["Name"].asCString();
	Material material = this->materialmodule.findMaterialByName(materialName);
	if (!material.isValid())
	{
		material = this->materialmodule.createMaterial();
		material.setName(materialName);
	}
	Region rootRegion = this->materialmodule.getContext().getDefaultRegion();
	return readMaterialFromJson(material, materialSettings, rootRegion);
}
