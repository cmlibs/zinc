/*******************************************************************************
FILE : heapsort.c

LAST MODIFIED : 2 November 1995

DESCRIPTION :
Created because qsort didn't seem to be working properly in TurboC.
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
#include <stddef.h>
#include "general/heapsort.h"
#include "general/debug.h"

int heapsort(void *base,size_t count,size_t size,
	int (*compare)(const void *,const void *))
/*******************************************************************************
LAST MODIFIED : 2 November 1995

DESCRIPTION :
An implementation of heapsort as described in "Numerical Recipes.  The Art of
Scientific Computing" by W.H. Press, B.P. Flannery, S.A. Teukolsky and W.T.
Vetterling.
==============================================================================*/
{
	int l,ir,i,j,k;
	char *raa=NULL;

	ENTER(heapsort);
	if (ALLOCATE(raa,char,size))
	{
		l=count/2+1;
		ir=count;
		/* The index l will be decremented from its initial value down to 1 during
			the heap createion phase.  Once l reaches 1, the index ir will be
			decremented from its initial value down to 1 during the heap selection
			phase. */
		while (ir>1)
		{
			if (l>1)
			{
				l--;
				for (k=size;k>0;)
				{
					k--;
					*(raa+k)= *(((char *)base)+(l-1)*size+k);
				}
			}
			else
			{
				for (k=size;k>0;)
				{
					k--;
					*(raa+k)= *(((char *)base)+(ir-1)*size+k);
					*(((char *)base)+(ir-1)*size+k)= *(((char *)base)+k);
				}
				ir--;
			}
			i=l;
			j=l+l;
			while (j<=ir)
			{
				if (j<ir)
				{
					if (compare((void *)(((char *)base)+(j-1)*size),
						(void *)(((char *)base)+j*size))<0)
					{
						j++;
					}
				}
				if (compare((void *)raa,(void *)(((char *)base)+(j-1)*size))<0)
				{
					for (k=size;k>0;)
					{
						k--;
						*(((char *)base)+(i-1)*size+k)= *(((char *)base)+(j-1)*size+k);
					}
					i=j;
					j=j+j;
				}
				else
				{
					j=ir+1;
				}
			}
			for (k=size;k>0;)
			{
				k--;
				*(((char *)base)+(i-1)*size+k)= *(raa+k);
			}
		}
		DEALLOCATE(raa);
	}
	LEAVE;

	return(1);
} /* heapsort */
