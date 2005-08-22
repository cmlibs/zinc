/*******************************************************************************
FILE : compare.h

LAST MODIFIED : 15 March 1999

DESCRIPTION :
Prototypes of functions for comparing data types (analogous to strcmp).
Functions used for managers and lists.
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
#if !defined (COMPARE_H)

/*
Global functions
----------------
*/
int compare_int(int int_1,int int_2);
/*******************************************************************************
LAST MODIFIED : 27 April 1995

DESCRIPTION :
Returns -1 if int_1 < int_2, 0 if int_1 = int_2 and 1 if int_1 > int_2.
==============================================================================*/

int compare_pointer(void *pointer_1,void *pointer_2);
/*******************************************************************************
LAST MODIFIED : 15 March 1999

DESCRIPTION :
Returns -1 if pointer_1 < pointer_2, 0 if pointer_1 = pointer_2 
and 1 if pointer_1 > pointer_2.
==============================================================================*/

int compare_double(double double_1,double double_2);
/*******************************************************************************
LAST MODIFIED : 25 February 2005

DESCRIPTION :
Returns -1 if double_1 < double_2, 0 if double_1 = double_2 
and 1 if double_1 > double_2.
==============================================================================*/
#endif
