/***********************************************************************
*
*  Name:          SGIGrabIPOPs.c
*
*  Author:        Paul Charette
*
*  Last Modified: 9 December 1997
*
*  Purpose:       Frame grabbing on the SGI O2
*
*
***********************************************************************/
#include <stdio.h>
#include "UxXt.h"
#include "XvgGlobals.h"
#include <unistd.h>
#ifndef SGI
#include <dmedia/vl.h>
#endif
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

#define MSG_BUF_LEN    128
#define INTERNET_PORT 2000
#define FLUSH_FRAMES    12

/**************************** FrameGrab ************************************/
/*                                                                         */
/*  function: Grab frames via the SGI video server.                        */
/*                                                                         */
/*  Parameters:                                                            */
/*    - None.                                                              */
/*                                                                         */
/***************************************************************************/
void FrameGrab(double *inpixels, double *outpixels,
	       int height, int width, void *ParmPtrs[])
{
  extern Widget RawImageTBG, ProcessedImageTBG, RawImageShell,
  FilenameLG, SizeLG;
  FILE *fp;
  int rc, i, lwidth, lheight;
  double *pixels;
  char fname[256];

  /* init parameters */
	fp=(FILE *)NULL;
  sprintf(fname, "/tmp/tmp.tif");

  /* open the pipe to vidtotiff */
  sprintf(cbuf, "/usr/people/paulc/xvg/cmgui/vidtotiff %d", VerboseFlag);
#ifdef SGI
	Abort = B_TRUE;
	draw_mesg("Abort : Could not open the pipe in FrameGrab() not ANSI");
	return;
#else
  if ((fp = popen(cbuf, "w")) == NULL) {
    Abort = B_TRUE;
    draw_mesg("Abort : Could not open the pipe in FrameGrab()");
    return;
  }
#endif
  
  /* write the filename to signal the frame grab */
  if ((rc = fprintf(fp, "%s\n", fname)) != (strlen(fname) + 1)) {
    Abort = B_TRUE;
    draw_mesg("Abort : Could not write to the pipe in FrameGrab()");
  }
  
  /* close the pipe */
  pclose(fp);

  /* load the file from disk */
  if (GetTiffFile(fname, &RawPixels, &lheight, &lwidth, &RawMin, &RawMax,
		  cmap, UserCells, &TrueColorImage, VerboseFlag) == B_FALSE) {
    Abort = B_TRUE;
    sprintf(cbuf, "Error reading file %s in FrameGrab()", fname);
    draw_mesg(cbuf);
    return;
  }

  /* resize windows & buffers if required */
  if ((lheight != height) || (lwidth != width)) {
    ImgHeight = lheight;
    ImgWidth = lwidth;

    if (IsTrue(UxGetSet(RawImageTBG)) == B_FALSE) {
      RawWinHOffset = 0;
      RawWinVOffset = 0;
      RawWinWidth = ImgWidth;
      RawWinHeight = ImgHeight;
    }
    if (IsTrue(UxGetSet(ProcessedImageTBG)) == B_FALSE) {
      ProcessedWinHOffset = 0;
      ProcessedWinVOffset = 0;
      ProcessedWinWidth = ImgWidth;
      ProcessedWinHeight = ImgHeight;
    }

    /* create the raw Image widgets if they don't exist already, resize them */
    if (RawImageShellCreated == B_FALSE)
      RawImageShell = create_RawImageShell();
    else {
      ResizeRawImageWidgets(B_FALSE);
      CreateRawImage();
    }
    
    /* resize the processed Image widgets */
    InitProcBuffers();
    ResizeProcessedImageWidgets(B_FALSE);
    CreateProcImage();
  }

  /* show image size */
  if (InteractiveFlag) {
    UxPutLabelString(FilenameLG, "Grabbing");
    sprintf(cbuf, "Size: %d x %d", ImgWidth, ImgHeight);
    UxPutLabelString(SizeLG, cbuf);
  }
    
  /* copy pixels */
  pixels = GetCurrentOutputIPBuffer();
  for (i = 0; i < ImgHeight*ImgWidth; i++)
    pixels[i] = RawPixels[i];
  
  /* delete the file */
  unlink(fname);
}


/************************** NetworkGrab ************************************/
/*                                                                         */
/*  function: Grab frames via the SGI video server, on commands            */
/*            received across the network.                                 */
/*                                                                         */
/*  Parameters:                                                            */
/*    - ParmPtrs[0] : Directory to save output files.                      */
/*                                                                         */
/***************************************************************************/
#define COMMAND_LEN 4
boolean_t SendMsg(int remoteClient, char *com, char *args, int n)
{
  /* write command */
  if (write(remoteClient, com, COMMAND_LEN) != COMMAND_LEN)
    return(B_FALSE);

  /* write args if required */
  if ((n > 0) && (args != NULL)) {
    if (write(remoteClient, &n, sizeof(int)) != sizeof(int))
      return(B_FALSE);
    if (write(remoteClient, args, n) != n)
      return(B_FALSE);
  }
  else {
    n = 0;
    if (write(remoteClient, &n, sizeof(int)) != sizeof(int))
      return(B_FALSE);
  }

  /* verbose if required */
  if (VerboseFlag) {
    if (n > 0)
      printf("NetworkGrab() : SendMsg(\"%s\", %d arg bytes, args:\"%s\"...)\n",
	     com, n, args);
    else
      printf("NetworkGrab() : SendMsg(\"%s\", no argument bytes)...\n", com);
  }
  
  /* debug */
  XvgMM_MemoryCheck("in FEComputeDispNodesXCorr()");

  /* return success */
  return(B_TRUE);
}

boolean_t GetMsg(int remoteClient, char *InCom, int *ArgsLen, char **args,
		 int *LongestArgsLen)
{
  /* read 4 byte command */
  read(remoteClient, InCom, COMMAND_LEN);
  InCom[COMMAND_LEN] = '\0';
  
  /* read in the length of the arguments to follow */
  read(remoteClient, ArgsLen, sizeof(int));
  
  /* if the ArgsLen count is non-zero, read in the arguments */
  if ((*ArgsLen) > 0) {
    /* allocate storage if more is required */
    if (*ArgsLen > *LongestArgsLen) {
      if (((*args) = (char *) XvgMM_Alloc((void *) (*args), (*ArgsLen)))
	  == NULL) {
	SendMsg(remoteClient, "NOK ", "Error allocating memory",
		strlen("Error allocating memory")+1);
	return(B_FALSE);
      }
      *LongestArgsLen = *ArgsLen;
    }
    /* read in the data */
    read(remoteClient, (*args), (*ArgsLen));
  }
  
  /* verbose if required */
  if (VerboseFlag) {
    if (*ArgsLen > 0) {
      if (((*args)[*ArgsLen -1] == '\0') && (*ArgsLen < 64))
	printf("GetMsg(\"%s\", %d argument bytes, args:\"%s\")...\n",
	       InCom, *ArgsLen, *args);
      else
	printf("NetworkGrab() : GetMsg(\"%s\", %d argument bytes)...\n",
	       InCom, *ArgsLen);
    }
    else
      printf("NetworkGrab() : GetMsg(\"%s\", no argument bytes)...\n", InCom);
  }
  
  /* debug */
  XvgMM_MemoryCheck("in FEComputeDispNodesXCorr()");

  /* return success */
  return(B_TRUE);
}
    


void NetworkGrab(double *inpixels, double *outpixels,
		 int height, int width, void *ParmPtrs[])
{
  FILE *fpp, *fp;
  int i, j, go, remoteClient, addressLength, optionValue, theSocket, ArgsLen,
  rc, LongestArgsLen, nframes;
  struct sockaddr_in socketDescriptor;
  char InCom[COMMAND_LEN + 1], fname[128], rhost[64],
  ffname[512], FnameMant[128], fname_nodirpath[128], *args=NULL;
  unsigned long theAddressNumber;

  /* Create the (internet, stream) socket */
  if ((theSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1 ) {
    Abort = B_TRUE;
    draw_mesg("Abort : error creating socket in NetworkGrab()");
    return;
  }
  if (VerboseFlag)
    printf("NetworkGrab() : Created socket.\n");
   
  /* Indicate that socket re-use is allowed */
  optionValue = 1;
  if (setsockopt(theSocket, SOL_SOCKET, SO_REUSEADDR,
		&optionValue, sizeof(optionValue)) == -1 ) {
    Abort = B_TRUE;
    draw_mesg("Abort : error returning from setsockopt() in NetworkGrab()");
    return;
  }
  
  /* Bind this socket to all incoming addresses on the port */
  memset(&socketDescriptor, 0, sizeof(socketDescriptor));
  socketDescriptor.sin_family      = AF_INET;           /* Protocol Family */
  socketDescriptor.sin_port        = htons(INTERNET_PORT);         /* Port */
  socketDescriptor.sin_addr.s_addr = htonl(INADDR_ANY);   /* listen to all */
  if (bind(theSocket, (struct sockaddr *) &socketDescriptor,
	   sizeof(struct sockaddr)) == -1 ) {
    Abort = B_TRUE;
    draw_mesg("Abort: error binding internet stream in NetworkGrab()");
    return;
  }
  
  /* Indicate willingness to accept connections */
  if (listen(theSocket, 1) == -1 ) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort: error on return from listen() in NetworkGrab()");
    draw_mesg(cbuf);
    close(theSocket);
    return;
  }
  
  /* wait for the remote host to connect */ 
  draw_mesg("NetworkGrab() : Waiting for the remote host to conect...");
  addressLength = sizeof(socketDescriptor);
  memset(&socketDescriptor, 0, addressLength);
  if ((remoteClient = accept(theSocket,
			     (struct sockaddr *) &socketDescriptor,
			     &addressLength )) == -1) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort: error on return from accept() in NetworkGrab()");
    draw_mesg(cbuf);
    close(theSocket);
    return;
  }
  sprintf(rhost, "%d.%d.%d.%d", 
	  (socketDescriptor.sin_addr.s_addr & 0xff000000) >> 24,
	  (socketDescriptor.sin_addr.s_addr & 0x00ff0000) >> 16,
	  (socketDescriptor.sin_addr.s_addr & 0x0000ff00) >> 8,
	  (socketDescriptor.sin_addr.s_addr & 0x000000ff));
  if (VerboseFlag) 
    printf("NetworkGrab(): host at IP address %s has connected...\n", rhost);

  /* loop waiting for messages from remote host */
  go = 1;
  LongestArgsLen = 0;
  nframes = 0;
  do {
    /* wait for command input */
    sprintf(cbuf, "Waiting for input from %s (%d frames grabbed)...",
	    rhost, nframes);
    draw_mesg(cbuf);
    if (GetMsg(remoteClient, InCom, &ArgsLen, &args, &LongestArgsLen)
	== FALSE) {
      Abort = B_TRUE;
      sprintf(cbuf,
	      "Abort : On return from GetMsg(ArgsLen: %d) in NetworkGrab()",
	      ArgsLen);
      draw_mesg(cbuf);
    }
    
    /********* decode incoming message **********/
    if (Abort == FALSE) {
      
      /* end communication */
      if (strcmp("STOP", InCom) == 0) {
	go = 0;
	SendMsg(remoteClient, "OK  ", NULL, 0);
      }
      
      /* set outfile mantissa name */
      else if (strcmp("FNAM", InCom) == 0) {
	/* extract filename mantissa */
	sprintf(FnameMant, "%s", args);
	
	/* open the list of files file */
	sprintf(ffname, "%s/%s_files.doc", ParmPtrs[0], FnameMant);
	if ((fp = fopen(ffname, "w")) == NULL) {
	  sprintf(cbuf, "Could not open %s", ffname);
	  SendMsg(remoteClient, "NOK ", cbuf, strlen(cbuf)+1);
	  go = 0;
	  Abort = B_TRUE;
	  sprintf(cbuf, "NetworkGrab() : Could not open file \"%s\"", ffname);
	  draw_mesg(cbuf);
	  sprintf(ffname, "");
	}
	else {
	  if (VerboseFlag)
	    printf("NetworkGrab() : Opened file \"%s\"\n", ffname);
	  SendMsg(remoteClient, "OK  ", NULL, 0);
	}
      }
      
      /* read in forces and save values to disk */
      else if (strcmp("FRCS", InCom) == 0) {
	if (ArgsLen == 16*sizeof(double)) {
	  /* compose the filename */
	  sprintf(fname, "%s/%s%02d_forces.mat",
		  ParmPtrs[0], FnameMant, nframes++);
	  if (WriteArrayToMatlabFile(16, 1, "forces", (double *) args,
				     fname)) {
	    if (VerboseFlag)
	      printf("NetworkGrab() : Wrote forces to file \"%s\"\n", fname);
	    sprintf(cbuf, "Wrote forces to file \"%s\"", fname);
	    SendMsg(remoteClient, "OK  ", cbuf, strlen(cbuf)+1);
	  }
	  else {
	    sprintf(cbuf, "Error writing forces to file %s", fname);
	    SendMsg(remoteClient, "NOK ", cbuf, sizeof(cbuf)+1);
	    Abort = B_TRUE;
	    sprintf(cbuf, "NetworkGrab() : Error writing forces in %s", fname);
	    draw_mesg(cbuf);
	  }
	}
	else {
	  sprintf(cbuf, "Incorrect number of force measurements (%d)",
		  ArgsLen/sizeof(double));
	  SendMsg(remoteClient, "NOK ", cbuf, sizeof(cbuf)+1);
	  Abort = B_TRUE;
	  sprintf(cbuf, "NetworkGrab() : Incorrect number of force meas. (%d)",
		  ArgsLen/sizeof(double));
	  draw_mesg(cbuf);
	}
      }

      /* grab a frame */
      else if (strcmp("GRAB", InCom) == 0) {
	/* check that file of filenames was opened */
	if (strcmp(ffname, "") != 0) {
	  /* compose the filenames */
	  sprintf(fname, "%s/%s%02d.tif",
		  ParmPtrs[0], FnameMant, nframes);
	  sprintf(fname_nodirpath, "%s%02d.tif", FnameMant, nframes);
	  /* delete the image file if it exists already */
	  if (access(fname, F_OK) == 0)
	    unlink(fname);

	  /* open the pipe to vidtotiff */
	  sprintf(cbuf, "/usr/people/paulc/xvg/cmgui/vidtotiff %d",
		  VerboseFlag);
#ifdef SGI
		Abort = B_TRUE;
		draw_mesg("Abort : Could not open the pipe in NetworkGrab() - not ANSI");
		return;
#else
	  if ((fpp = popen(cbuf, "w")) == NULL) {
	    Abort = B_TRUE;
	    draw_mesg("Abort : Could not open the pipe in NetworkGrab()");
	    return;
	  }
#endif
  
	  /* write the filename to the pipe to signal the frame grab */
	  if (fprintf(fpp, "%s\n", fname) != (strlen(fname) + 1)) {
	    SendMsg(remoteClient, "NOK ", "Could not write to pipe",
			 strlen("Could not write to pipe")+1);
	    Abort = B_TRUE;
	    draw_mesg("Abort : Could not write to the pipe in NetworkGrab()");
	    pclose(fpp);
	  }
	  /* wait for the file to be written, send back OK */
	  else {
	    /* close the pipe */
	    pclose(fpp);

	    /* verbose if required */
	    if (VerboseFlag)
	      printf("NetworkGrab() : Waiting for \"%s\" to be written...\n",
		     fname);
	    /* wait for file to be written to disk */
	    do {
	      sginap(0);
	      rc = access(fname, F_OK);
	    } while((rc != 0) && (Abort == B_FALSE));
	    /* send acknowledgement */
	    if (Abort == FALSE) {
	      sprintf(cbuf, "Wrote file %s to disk", fname);
	      SendMsg(remoteClient, "OK  ", cbuf, strlen(cbuf)+1);
	    }
	    else {
	      SendMsg(remoteClient, "NOK ", "Error waiting for file write",
		      strlen("Error waiting for file write")+1);
	    }
	    /* print this filename in the output list file */
	    fprintf(fp, "%s\n", fname_nodirpath);
	  }
	}
	
	/* reply that filename file must be opened first */
	else {
	  ErrorMsg("NetworkGrab() : GRAB command received before FNAM!\n");
	  SendMsg(remoteClient,
		       "NOK ", "GRAB command received before FNAM",
		       strlen("GRAB command received before FNAM")+1);
	}
      }
      
      /* unsupported message */
      else {
	sprintf(cbuf, "unknown message \"%s\" in NetworkGrab()", InCom);
	SendMsg(remoteClient, "NOK ", cbuf, strlen(cbuf)+1);
	ErrorMsg(cbuf);
      }
    }
  } while (go && (Abort == FALSE));
  
  /* close socket connection & files */
  if (strcmp(ffname, "") != 0)
    fclose(fp);
  close(remoteClient);    
  close(theSocket);
  
  /* transfer pixels */
  for (i = 0; i < width*height; i++)
    outpixels[i] = inpixels[i];
}







