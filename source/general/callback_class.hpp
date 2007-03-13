/*******************************************************************************
FILE : callback_class.hpp

LAST MODIFIED : 21 February 2007

DESCRIPTION :
Template definitions for callbacks.
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
#if !defined (CALLBACK_CLASS_HPP)
#define CALLBACK_CLASS_HPP

template < class Object > class Callback_base
/*****************************************************************************
LAST MODIFIED : 21 February 2007

DESCRIPTION :
============================================================================*/
{
public:
	void operator()(Object object) const
	{
		callback_function(object);
	}
	virtual int callback_function(Object object) = 0;

protected:

	virtual ~Callback_base()
	{
	}
};

template < class Object, class Callee, class MemberFunction > class Callback_member_callback : public Callback_base< Object >
/*****************************************************************************
LAST MODIFIED : 21 February 2007

DESCRIPTION :
============================================================================*/
{
private:
	Callee *callee;
	MemberFunction member_function;

public:
	Callback_member_callback(
		Callee *callee, MemberFunction member_function) :
		callee(callee), member_function(member_function)
	{
	}

	virtual int callback_function(Object object)
	{
		return (callee->*member_function)(object);
	}
		
	virtual ~Callback_member_callback()
	{
	}
};

#endif /* !defined (CALLBACK_CLASS_HPP) */
