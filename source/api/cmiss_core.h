/*******************************************************************************
FILE : cmiss_core.h

LAST MODIFIED : 12 September 2002

DESCRIPTION :
The public interface to the some of the internal functions of cmiss.
==============================================================================*/
#ifndef __CMISS_CORE_H__
#define __CMISS_CORE_H__

int Cmiss_deallocate(void *ptr);
/*******************************************************************************
LAST MODIFIED : 12 September 2002

DESCRIPTION :
Frees the memory associated with the pointer.  Used to clean up when functions
return buffers allocated internally to cmiss.
==============================================================================*/
#endif /* __CMISS_CORE_H__ */
