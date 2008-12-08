/*******************************************************************************
FILE : example_functions.cc

LAST MODIFIED : 8 December 2008

DESCRIPTION :
Three simple functions to be wrapped for python using swig. 
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

#include <iostream>
#include <time.h>
#include "example_functions.h"

double arithmetics(double x, double y, int type)
{
	double answer;
	
	switch(type)
	{
		case 1:
			answer = x+y;
			std::cout << x << " + " << y << " = " << answer << std::endl;
			break;
		case 2:
			answer = x-y;
			std::cout << x << " - " << y << " = " << answer << std::endl;
			break;
		case 3:
			answer = x*y;
			std::cout << x << " * " << y << " = " << answer << std::endl;
			break;
		case 4:
			answer = x/y;
			std::cout << x << " / " << y << " = " << answer << std::endl;
			break;
		default:
			std::cout << "Unknown operation!";
	}
	return answer;
}

int shoe()
{
	int shoes;
	
	std::cout << "number of shoes? ";
	std::cin >> shoes;
	
	if (shoes%2 != 0)
		std::cout << "not in pairs\n";
	else
		std::cout << "thats " << shoes/2 << " pairs\n" << std::endl;
	
	return shoes/2;
}

char *get_time()
{
    time_t ltime;
    time(&ltime);
    return ctime(&ltime);
}

