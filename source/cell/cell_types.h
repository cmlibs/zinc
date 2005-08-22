/*******************************************************************************
FILE : cell_types.h

LAST MODIFIED : 09 November 2000

DESCRIPTION :
The different types of values used in the Cell interface.
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
#if !defined (CELL_TYPES_H)
#define CELL_TYPES_H

/*
Global types
------------
*/
enum Cell_value_type
/*******************************************************************************
LAST MODIFIED : 02 July 2000

DESCRIPTION :
Enumeration of the various value types used in Cell.
==============================================================================*/
{
  CELL_INTEGER_VALUE,
  CELL_REAL_VALUE,
  CELL_STRING_VALUE,
  CELL_UNKNOWN_VALUE
}; /* enum Cell_value_type */

#define CELL_INTEGER_FORMAT "%d"
typedef int CELL_INTEGER;
/*******************************************************************************
LAST MODIFIED : 02 July 2000

DESCRIPTION :
The integer value type
==============================================================================*/

#define CELL_REAL_FORMAT "%g"
typedef float CELL_REAL;
/*******************************************************************************
LAST MODIFIED : 02 July 2000

DESCRIPTION :
The real value type
==============================================================================*/

#define CELL_DOUBLE_FORMAT "%lf"
typedef double CELL_DOUBLE;
/*******************************************************************************
LAST MODIFIED : 28 October 2000

DESCRIPTION :
The double value type
==============================================================================*/

#define CELL_STRING_FORMAT "%s"
typedef char* CELL_STRING;
/*******************************************************************************
LAST MODIFIED : 02 July 2000

DESCRIPTION :
The string value type
==============================================================================*/

#endif /* !defined (CELL_TYPES_H) */
