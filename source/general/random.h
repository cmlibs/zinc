/*******************************************************************************
FILE : random.h

LAST MODIFIED : 26 June 2002

DESCRIPTION :
Standard macros for returning random numbers.
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
#if !defined (RANDOM_H)
#define RANDOM_H

#if defined (UNIX) /* switch (OPERATING_SYSTEM) */
/* Must #include <stdlib.h> in calling module to use: */
/* Returns a random number of the given <type> in the range [0.0, 1.0]
	 ie. INCLUDING 0.0 and 1.0 in the possible results */
#define CMGUI_RANDOM(type) ((type)random() / 2147483647.0)

/* Returns a random number of the given <type> in the range (0.0, 1.0)
	 ie. NOT INCLUDING 0.0 and 1.0 in the possible results */
#define CMGUI_RANDOM_NON_INCLUSIVE(type) (((type)random() + 1.0) / 2147483649.0)

/* partner random number seed function for CMGUI_RANDOM */
#define CMGUI_SEED_RANDOM(seed) srandom(seed)

#elif defined (WIN32_SYSTEM) /* switch (OPERATING_SYSTEM) */

/* Must #include <stdlib.h> in calling module to use: */
/* Returns a random number of the given <type> in the range [0.0, 1.0]
	 ie. INCLUDING 0.0 and 1.0 in the possible results */
#define CMGUI_RANDOM(type) ((type)rand() / (type)RAND_MAX)

/* Returns a random number of the given <type> in the range (0.0, 1.0)
	 ie. NOT INCLUDING 0.0 and 1.0 in the possible results */
#define CMGUI_RANDOM_NON_INCLUSIVE(type) (((type)rand() + 1.0) / ((type)RAND_MAX + 2.0))

/* partner random number seed function for CMGUI_RANDOM */
#define CMGUI_SEED_RANDOM(seed) srand(seed)

#endif /* switch (OPERATING_SYSTEM) */
#endif /* !defined (RANDOM_H) */
