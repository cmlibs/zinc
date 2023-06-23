/**
 * @file fieldsceneviewerprojection.hpp
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_FIELDSCENEVIEWERPROJECTION_HPP__
#define CMZN_FIELDSCENEVIEWERPROJECTION_HPP__

#include "cmlibs/zinc/fieldsceneviewerprojection.h"
#include "cmlibs/zinc/field.hpp"
#include "cmlibs/zinc/fieldmodule.hpp"
#include "cmlibs/zinc/sceneviewer.hpp"
#include "cmlibs/zinc/types/scenecoordinatesystem.hpp"

namespace CMLibs
{
namespace Zinc
{

class FieldSceneviewerProjection : public Field
{
private:
	// takes ownership of C handle, responsibility for destroying it
	explicit FieldSceneviewerProjection(cmzn_field_id field_id) : Field(field_id)
	{	}

	friend FieldSceneviewerProjection Fieldmodule::createFieldSceneviewerProjection(
		const Sceneviewer& sceneviewer, Scenecoordinatesystem fromCoordinateSystem,
		Scenecoordinatesystem toCoordinateSystem);

public:

	FieldSceneviewerProjection() : Field(0)
	{	}

};

inline FieldSceneviewerProjection Fieldmodule::createFieldSceneviewerProjection(
	const Sceneviewer& sceneviewer, Scenecoordinatesystem fromCoordinateSystem,
			Scenecoordinatesystem toCoordinateSystem)
{
	return FieldSceneviewerProjection(cmzn_fieldmodule_create_field_sceneviewer_projection(id,
		sceneviewer.getId(), static_cast<cmzn_scenecoordinatesystem>(fromCoordinateSystem),
		static_cast<cmzn_scenecoordinatesystem>(toCoordinateSystem)));
}

}
}

#endif

