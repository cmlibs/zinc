/***********************************************************************
*
*  Name:          XvgMemoryManager.c
*
*  Author:        Paul Charette
*
*  Last Modified: 3 March 1997
*
*  Purpose:       Memory manager rotuines.
*
*                      XvgMM_Alloc()
*                      XvgMM_Free()
*                      XvgMM_FreeAllBlocks()
*                      XvgMM_GetBlockSize()
*                      int XvgMM_GetMemSize();
*                      XvgMM_Init()
*                      XvgMM_MemoryCheck()
*
***********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#if (!defined (_XVGMM_STANDALONE))
#include "UxXt.h"
#include "XvgGlobals.h"
#endif
#include "XvgMemoryManager.h"

/* local definitions */
#define MEMORY_MAX 20000
static struct {
  void *ptr;
  int length;
  int footer;
} memory_blocks[MEMORY_MAX];
static int n_memory_blocks, total_memory_allocated;

/* defintions required for stand-alone operation */
#if defined (_XVGMM_STANDALONE)
#define CBUF_SIZE 1024
char cbuf[CBUF_SIZE];
int VerboseFlag=1, XvgMM_MemoryCheck_Flag=1, MemoryErrorFlag=0;

void ErrorMsg(char *s)
{
  printf("\a%s\n", s);
  MemoryErrorFlag = 1;
}
#endif

#if defined (_INCLUDE_XVG_EXAMPLE_CODE)
/* example code */
static void ExampleCode(void)
{
  double *buffer;
  int n;
  
  /* init memory manager */
  XvgMM_Init();
  
  /* ex: allocate "n" doubles to "buffer". Note that the first argument to */
  /* XvgMM_Alloc() must be an initialized pointer, i.e.: the pointer       */
  /* itself (buffer in this example) if the pointer has been used in a     */
  /* previous call to XvgMM_Alloc() or NULL at the time of the first call, */
  /* as below.                                                             */
  if ((buffer = (double *) XvgMM_Alloc((void *) NULL,
				       n*sizeof(double))) == NULL) {
    printf("Error : Out of memory\n");
    return;
  }
  
  /*
  ..
  ..
  */

  /* optional explicit memory check during program execution */
  XvgMM_MemoryCheck("Error occured in routine ExampleCode()");

  /*
  ..
  ..
  */

  /* re-allocate "2n" doubles to "buffer" */
  if ((buffer = (double *) XvgMM_Alloc((void *) buffer,
				       2*n*sizeof(double))) == NULL) {
    printf("Error : Out of memory\n");
    return;
  }

  /*
  ..
  ..
  */

  /* optional explicit memory check during program execution */
  XvgMM_MemoryCheck("Error occured in routine ExampleCode()");

  /*
  ..
  ..
  */

  /* free memory */
  XvgMM_Free(buffer);
}
#endif


void XvgMM_Init(void)
{
  int i;
  
  /* check that integers are 4 bytes on this machine */
  if (sizeof(int) != 4) {
    sprintf(cbuf, "INTEGER SIZE (%d) WILL CAUSE PROBLEMS WITH XVG MEMORY MGR",
	    sizeof(int));
    ErrorMsg(cbuf);
#if (!defined (_XVGMM_STANDALONE))
    Abort = B_TRUE;
#endif
    return;
  }

  /* init memory manager */
  n_memory_blocks = 0;
  total_memory_allocated = 0;
  for (i = 0; i < MEMORY_MAX; i++) {
    memory_blocks[i].ptr = NULL;
    memory_blocks[i].length = 0;
    memory_blocks[i].footer = -1;
  }

#if defined (_XVGMM_STANDALONE)
  MemoryErrorFlag = 0;
#endif
}

void *XvgMM_Alloc(void *ptr, int nbytes)
{
  int *block, i, BytesToAlloc;
  
  sprintf(cbuf, "XvgMM_Alloc(%d)", nbytes);
  XvgMM_MemoryCheck(cbuf);

  /* round of nbytes to nearest multiple of 4 */
  BytesToAlloc = nbytes & ~((unsigned int) 0x03);
  BytesToAlloc = (((nbytes & 0x03) != 0) ? BytesToAlloc + 4 : BytesToAlloc);
  if (BytesToAlloc < nbytes) {
    ErrorMsg(cbuf);
#if (!defined (_XVGMM_STANDALONE))
    Abort = B_TRUE;
#endif
    return(NULL);
  }
  
  /* find the location of this block definition in the list, if it exists */
  if (ptr) {
    for (i = 0; (i < n_memory_blocks) && (ptr != memory_blocks[i].ptr);
	 i++);
  }
  else {
    i = n_memory_blocks;
  }

  /* if the block definition exists and its already the correct size,return */
  if (i < n_memory_blocks) {
    if (memory_blocks[i].length == BytesToAlloc) {
      if (VerboseFlag)
	printf("XvgMM_Alloc(): %d byte block (#%d) already allocated\n",
	       BytesToAlloc, i);
      return(ptr);
    }
  }
  
  /* if the block definition exists, just realloc */
  if (i < n_memory_blocks) {
    total_memory_allocated -= memory_blocks[i].length;
    block = (int *) realloc(memory_blocks[i].ptr,
			    BytesToAlloc + sizeof(int));
    if (VerboseFlag && block) {
      printf("XvgMM_Alloc(): realloc'd");
      fflush(stdout);
    }
  }
  /* else create a new block definition and use malloc() */
  else {
    /* check first if memory manager block list is full */
    if (n_memory_blocks+1 < MEMORY_MAX) {
      block = (int *) malloc(BytesToAlloc + sizeof(int));
      if (block)
        n_memory_blocks++;
      if (VerboseFlag && block) {
	printf("XvgMM_Alloc(): alloc'd");
	fflush(stdout);
      }
    }
    else {
      sprintf(cbuf, "XvgMM_Alloc() : BLOCK LIST SIZE EXCEEDED!");
      ErrorMsg(cbuf);
#if (!defined (_XVGMM_STANDALONE))
    Abort = B_TRUE;
#endif
      return(NULL);
    }
  }
  
  /* if the allocation was successfull, complete the block defintion */
  if (block) {
    memory_blocks[i].ptr = block;
    memory_blocks[i].length = BytesToAlloc;
/*    memory_blocks[i].footer = rand(); */
    memory_blocks[i].footer = i;
    block[BytesToAlloc/sizeof(int)] = memory_blocks[i].footer;
    total_memory_allocated += memory_blocks[i].length;
    if (VerboseFlag)
      printf(" %d bytes to block #%d (footer:%d, %d blocks allocated, %.3f Mbytes tot)\n",
	     memory_blocks[i].length, i,
	     memory_blocks[i].footer, n_memory_blocks,
	     ((double) total_memory_allocated)/1000000.0);
    return(memory_blocks[i].ptr);
  }
  /* else return NULL and flag error */
  else {
    sprintf(cbuf, "XvgMM_Alloc(): Failure to allocate %d size block!",
	    BytesToAlloc);
    ErrorMsg(cbuf);
#if (!defined (_XVGMM_STANDALONE))
    Abort = B_TRUE;
#endif
    return(NULL);
  }
}

void XvgMM_Free(void *ptr)
{
  int i, footer;
  
  sprintf(cbuf, "XvgMM_Free()");
  XvgMM_MemoryCheck(cbuf);

  /* return in the case of a null pointer */
  if (ptr == NULL)
    return;
  
  /* find the location of this block definition in the list */
  for (i = 0; (i < n_memory_blocks) && (ptr != memory_blocks[i].ptr);
       i++);

  /* if the pointer was not in the list, exit */
  if ((i == n_memory_blocks) && (VerboseFlag)) {
    sprintf(cbuf, "XvgMM_Free(): block not handled by the Xvg memory manager");
    ErrorMsg(cbuf);
#if (!defined (_XVGMM_STANDALONE))
    Abort = B_TRUE;
#endif
    return;
  }

  /* if the block was defined, free the memory and delete its definition */
  if (memory_blocks[i].ptr) {
    /* check the bracketing bytes for corruption */
    footer = ((int *) memory_blocks[i].ptr)
      [(memory_blocks[i].length)/sizeof(int)];
    if (footer != memory_blocks[i].footer) {
      sprintf(cbuf, "XvgMM_Free() : Memory block #%d footer corrupted!", i);
      ErrorMsg(cbuf);
#if (!defined (_XVGMM_STANDALONE))
    Abort = B_TRUE;
#endif
      return;
    }
    free(memory_blocks[i].ptr);
  }
  else {
    sprintf(cbuf, "XvgMM_Free() : Memory block #%d pointer was NULL!", i);
    ErrorMsg(cbuf);
#if (!defined (_XVGMM_STANDALONE))
    Abort = B_TRUE;
#endif
    return;
  }
  
  if (VerboseFlag)
    printf("XvgMM_Free(): freed %d bytes from block #%d (%d blocks, %.3f Mbytes tot)\n",
	   memory_blocks[i].length, i, n_memory_blocks-1,
	   ((double) (total_memory_allocated - memory_blocks[i].length))
	   /1000000.0);

  /* pack memory manager block list */
  total_memory_allocated -= memory_blocks[i].length;
  if (i < n_memory_blocks) {
    for (; (i < (n_memory_blocks-1)); i++)
      memory_blocks[i] = memory_blocks[i+1];
  }
  n_memory_blocks -= 1;
}


void XvgMM_MemoryCheck(char *msg)
{
  char str[CBUF_SIZE];
  int i, footer;
 
  if (XvgMM_MemoryCheck_Flag) {
    for (i = 0; i < n_memory_blocks; i++) {
      footer = ((int *) memory_blocks[i].ptr)
	[(memory_blocks[i].length)/sizeof(int)];
      if (footer != memory_blocks[i].footer) {
	sprintf(str,
		"XvgMM_MemoryCheck(): Memory corrupted at block #%d in %s",
		i, msg);
	ErrorMsg(str);
#if (!defined (_XVGMM_STANDALONE))
	Abort = B_TRUE;
#endif
	return;
      }
    }
  }
}

void XvgMM_FreeAllBlocks(void)
{
  int i;
  
  for (i = 0; i < n_memory_blocks; i++) {
    if (memory_blocks[i].ptr)
      free(memory_blocks[i].ptr);
  }
  XvgMM_Init();
}


int XvgMM_GetBlockSize(void *ptr)
{
  int i;
  
  /* if pointer is null, return an error */
  if (ptr == NULL)
    return(-1);

  /* find the location of this block definition in the list */
  for (i = 0; (i < n_memory_blocks) && (ptr != memory_blocks[i].ptr);
       i++);

  /* if its here, return the block length, else return an error */
  if (i < n_memory_blocks)
    return(memory_blocks[i].length);
  else
    return(-1);
}

void ShowXvgMMStats(double *inpixels, double *outpixels, int height, int width,
		    void *ParmPtrs[])
{
  int i;

  /* show stats */
  printf("XvgMM_Alloc() Xvg Memory Manager State:\n");
  printf("  %d memory blocks allocated.\n", n_memory_blocks);
  printf("  %d Kbytes total allocated.\n\n", total_memory_allocated/1000);

  /* transfer data */
  for (i = 0; i < height*width; i++)
    outpixels[i] = inpixels[i];
}

int XvgMM_GetMemSize(void)
{
  return(total_memory_allocated);
}

