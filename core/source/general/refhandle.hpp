/**
 * FILE : general/refhandle.hpp
 * 
 * Simple intrusive smart pointer template for classes derived from
 * cmzn::RefCounted, supporting access/deaccess methods.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if !defined (CMZN_GENERAL_REFHANDLE_HPP)
#define CMZN_GENERAL_REFHANDLE_HPP

#include "general/refcounted.hpp"

namespace cmzn
{

template <typename T> class RefHandle
{
private:
	T* object;

public:
	RefHandle() :
		object(0)
	{
	}

	// take ownership of reference; assumes created with access count of 1
	explicit RefHandle(T* objectIn) :
		object(objectIn)
	{
	}

	RefHandle(const RefHandle<T>& refHandleIn) :
		object(cmzn::ACCESS(refHandleIn.object))
	{
	}

	~RefHandle()
	{
		cmzn::DEACCESS(this->object);
	}

	RefHandle<T>& operator=(const RefHandle<T>& refHandleIn)
	{
		cmzn::REACCESS(this->object, refHandleIn.object);
		return *this;
	}

	T& operator*()
	{
		return *this->object;
	}

	T* operator->()
	{
		return this->object;
	}

	T* getObject()
	{
		return this->object;
	}

	operator bool()
	{
		return (this->object != 0);
	}
};

}

#endif /* !defined (CMZN_GENERAL_REFHANDLE_HPP) */
