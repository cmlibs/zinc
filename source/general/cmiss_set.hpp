/***************************************************************************//**
 * FILE : cmiss_set.hpp
 *
 * Template class derived from STL set which handles reference counting and
 * maintains connections between sets of related objects to safely and
 * efficiently manage changing the identifier i.e. sort key of the objects.
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
 * Portions created by the Initial Developer are Copyright (C) 2011
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
#if !defined (CMISS_SET_HPP)
#define CMISS_SET_HPP

#include <set>

/*
Local types
-----------
*/

template<class Key, class Compare > class Cmiss_set :
	private std::set<Key,Compare>
{
private:
	typedef std::set<Key,Compare> Base_class;

	mutable Cmiss_set *next, *prev; // linked list of related sets
	Key temp_removed_object; // removed while changing identifier

	Cmiss_set() :
		next(this),
		prev(this)
	{
	}

	/** copy constructor */
	Cmiss_set(const Cmiss_set& source) :
		Base_class(source),
		next(source.next),
		prev(&source)
	{
		for (iterator iter = begin(); iter != end(); ++iter)
		{
			(*iter)->access();
		}
		source.next = this;
		next->prev = this;
	}

	/** creates a set with the same manager, not a copy constructor */
	Cmiss_set(const Cmiss_set *source) :
		Base_class(),
		next(source->next),
		prev(const_cast<Cmiss_set *>(source))
	{
		source->next = this;
		next->prev = this;
	}

public:

	~Cmiss_set()
	{
		clear();
		prev->next = next;
		next->prev = prev;
	}

	typedef typename Base_class::iterator iterator;
	typedef typename Base_class::const_iterator const_iterator;
	typedef typename Base_class::size_type size_type;

	static Cmiss_set *create_independent()
	{
		return new Cmiss_set();
	}

	Cmiss_set *create_related() const
	{
		return new Cmiss_set(this);
	}

	Cmiss_set *create_copy()
	{
		return new Cmiss_set(*this);
	}

	Cmiss_set& operator=(const Cmiss_set& source)
	{
		if (&source == this)
			return *this;
		const Cmiss_set *related_set = this->next;
		while (related_set != this)
		{
			if (related_set == &source)
			{
				break;
			}
			related_set = related_set->next;
		}
		Base_class::operator=(source);
		if (related_set == this)
		{
			// copy from unrelated set: switch linked-lists
			this->next->prev = this->prev;
			this->prev->next = this->next;
			this->prev = const_cast<Cmiss_set *>(&source);
			this->next = source.next;
			source.next->prev = this;
			source.next = this;
		}
		return *this;
	}

	size_type erase(Key object)
	{
		size_type count = Base_class::erase(object);
		if (count)
		{
			object->deaccess(&object);
		}
		return count;
	}

	void erase(iterator iter)
	{
		Key object = *iter;
		Base_class::erase(iter);
		object->deaccess(&object);
	}

	std::pair<iterator,bool> insert(Key object)
	{
		std::pair<iterator,bool> result = Base_class::insert(object);
		if (result.second)
			object->access();
		return result;
	}

	void clear()
	{
		for (iterator iter = begin(); iter != end(); ++iter)
		{
			Key tmp = *iter;
			tmp->deaccess(&tmp);
		}
		Base_class::clear();
	}

	size_type size() const
	{
		return Base_class::size();
	}

	iterator find(const Key &object) const
	{
		return Base_class::find(object);
	}

   iterator begin()
   {
   	return Base_class::begin();
   }

   const_iterator begin() const
   {
   	return Base_class::begin();
   }

   iterator end()
   {
   	return Base_class::end();
   }

   const_iterator end() const
   {
   	return Base_class::end();
   }

	bool begin_identifier_change(Key object)
	{
		Cmiss_set *related_set = this;
		do
		{
			iterator iter = related_set->find(object);
			if (iter != related_set->end())
			{
				related_set->temp_removed_object = (*iter)->access();
				related_set->erase(iter);
			}
			else
			{
				related_set->temp_removed_object = 0;
			}
			related_set = related_set->next;
		}
		while (related_set != this);
		return true;
	}

	void end_identifier_change()
	{
		Cmiss_set *related_set = this;
		do
		{
			if (related_set->temp_removed_object)
			{
				related_set->insert(related_set->temp_removed_object); // check success?
				related_set->temp_removed_object->deaccess(&related_set->temp_removed_object);
			}
			related_set = related_set->next;
		}
		while (related_set != this);
	}
};

#endif /* !defined (CMISS_SET_HPP) */
