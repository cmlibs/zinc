/*******************************************************************************
FILE : cmiss_core.h

LAST MODIFIED : 12 August 2003

DESCRIPTION :
The public interface to the some of the internal functions of cmiss.
==============================================================================*/
#ifndef __CMISS_CORE_H__
#define __CMISS_CORE_H__

#define Cmiss_ALLOCATE(pointer,type,number) \
   (pointer=(type *)Cmiss_allocate(sizeof(type)*number))

void *Cmiss_allocate(int bytes);
/*******************************************************************************
LAST MODIFIED : 12 August 2003

DESCRIPTION :
Frees the memory associated with the pointer.  Used to clean up when functions
return buffers allocated internally to cmiss.
==============================================================================*/

int Cmiss_deallocate(void *ptr);
/*******************************************************************************
LAST MODIFIED : 12 August 2003

DESCRIPTION :
Frees the memory associated with the pointer.  Used to clean up when functions
return buffers allocated internally to cmiss.
==============================================================================*/
#endif /* __CMISS_CORE_H__ */
