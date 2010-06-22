/*******************************************************************************
FILE : scene_filters.hpp

LAST MODIFIED : 16 October 2008

DESCRIPTION :
==============================================================================*/
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

#ifndef SCENE_FILTERS_HPP
#define SCENE_FILTERS_HPP

#include <map>
#include <string>

template <typename ObjectType>
class SceneFiltersBaseFunctor
{
protected:
  int inclusive;
public:
  virtual int call(ObjectType object) = 0;

  virtual ~SceneFiltersBaseFunctor()
  {
  };
};

template <typename ObjectType>
class SceneFiltersNoValueFunctor :  public SceneFiltersBaseFunctor<ObjectType>
{
private:
  int(*fpt)(ObjectType);
public:

  SceneFiltersNoValueFunctor(int(*fpt_in)(ObjectType), int inclusive_in)
  { fpt=fpt_in; inclusive=inclusive_in;};

  virtual int call(ObjectType object)
  {
  	if (fpt)
  		return inclusive == (*fpt)(object);
  	else
  		return 0;
  }

  ~SceneFiltersNoValueFunctor()
  {
  };
};

template <typename ObjectType, typename ValueType> class SceneFiltersValueFunctor :
  public SceneFiltersBaseFunctor<ObjectType>
{
private:
  int(*fpt)(ObjectType, ValueType);
  int inclusive;
  ValueType value;

public:

  SceneFiltersValueFunctor(int(*fpt_in)(ObjectType, ValueType),
  	ValueType value_in, int inclusive_in)
  { fpt=fpt_in; value=value_in; inclusive=inclusive_in;};

  virtual int call(ObjectType object)
  {
  	if (fpt)
  		return inclusive == (*fpt)(object, value);
  	else
  		return 0;
  };             // execute member function

  ~SceneFiltersValueFunctor()
  {
  };

};

typedef std::multimap<std::string, void *> Filtering_list;
typedef std::multimap<std::string, void *>::iterator Filtering_list_iterator;

#endif /* SCENE_FILTERS_HPP_ */
