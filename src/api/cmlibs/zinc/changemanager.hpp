/**
 * @file changemanager.hpp
 * 
 * Exception-safe change caching class template.
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_CHANGEMANAGER_HPP__
#define CMZN_CHANGEMANAGER_HPP__

/**
 * \brief The Zinc namespace
 *
 * This is the Zinc namespace.
 */
namespace CMLibs
{
/**
 * \brief The CMLibs::Zinc namespace
 *
 * This is the CMLibs::Zinc namespace, all libZinc cpp APIs are in this namespace.
 */
namespace Zinc
{

/**
 * Template class calling beginChange() method on supplied object during
 * construction, and endChange() on it on destruction.
 * For safely making multiple changes to objects in a manager object when
 * exceptions may occur.
 * Usage example for Fieldmodule fieldmodule:
 * CMLibs::Zinc::ChangeManager<Fieldmodule> changeFieldmodule(fieldmodule);
 * // make multiple changes
 * fieldmodule.endChange() is called when leaving scope or on exception.
 * Enclose in { } to force scope to end earlier.
 */
template <class Manager> class ChangeManager
{
	Manager& manager;

public:
	ChangeManager(Manager& managerIn) :
		manager(managerIn)
	{
		this->manager.beginChange();
	}

	~ChangeManager()
	{
		this->manager.endChange();
	}
};

/**
 * Template class calling beginHierarchicalChange() method on supplied object
 * during construction, and endHierarchicalChange() on it on destruction.
 * For safely making multiple changes to objects in a manager object when
 * exceptions may occur.
 * Usage example for Region region:
 * CMLibs::Zinc::HierarchicalChangeManager<Region> changeRegion(region);
 * // make multiple changes
 * region.endHierarchicalChange() is called when leaving scope or on exception.
 * Enclose in { } to force scope to end earlier.
 */
template <class Manager> class HierarchicalChangeManager
{
	Manager &manager;

public:
	HierarchicalChangeManager(Manager& managerIn) :
		manager(managerIn)
	{
		this->manager.beginHierarchicalChange();
	}

	~HierarchicalChangeManager()
	{
		this->manager.endHierarchicalChange();
	}
};

}  // namespace Zinc
}  // namespace CMLibs

#endif /* CMZN_CHANGEMANAGER_HPP__ */
