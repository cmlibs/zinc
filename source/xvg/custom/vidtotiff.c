/***********************************************************************
*
*  Name:          vidtotiff.c
*
*  Author:        Paul Charette
*
*  Last Modified: 20 March 1997
*
*  Purpose:       Grab frames and write to tiff files
*
*  Compile:       cc -O -o vidtotiff vidtotiff.c -lvl
*                 WARNING: do not use the "-n32" flag!
*
*  Usage:         vidtotiff verbose(0/1) <-usage>;
*
***********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <vl/vl.h>
#include <sys/time.h>
#include "ij.h"
#include <gl/image.h>
#include <gl/gl.h>
#include <math.h>

static struct timeval tv0;
static struct timezone tz0;

#define FLUSH_FRAMES    12
#define HL 0x4D4D

static struct IFD_entry {
  unsigned short tag;
  unsigned short type;
  unsigned long length;
  unsigned long value_offset;
};

static struct tiff_header_struct {
  unsigned short byte_order;
  unsigned short version;
  unsigned long IFD_offset;
  unsigned short IFD_entry_count;
};

static struct IFD_struct {
  struct IFD_entry image_width;
  struct IFD_entry image_length;
  struct IFD_entry bits_per_sample;
  struct IFD_entry photometric_int;
  struct IFD_entry image_description;
  struct IFD_entry strip_offsets;
  struct IFD_entry samples_per_pixel;
  struct IFD_entry rows_per_strip;
  struct IFD_entry strip_byte_counts;
  struct IFD_entry colormap;
  unsigned long next_IFD_offset;
};

static struct tiff_header_struct tiff_header;


void rgb_to_hsv(double *rgb, double *hsv)
{
	/* converts rgb [0,1] to hsv [0,1],[0,360] */
	double max, min, delta;
	
	max = rgb[0];
	if (rgb[1] > max)
		max = rgb[1];
	if (rgb[2] > max)
		max = rgb[2];
		
	min = rgb[0];
	if (rgb[1] < min)
		min = rgb[1];
	if (rgb[2] < min)
		min = rgb[2];
		
	hsv[2] = max;
	if (max !=0)
		hsv[1] = (max-min)/max;
	else
		hsv[1] = 0;	
	
	if (hsv[1] == 0)
		hsv[0] = 0; /* hsv[0] should technically = undefined */
	else
		{
			delta = max-min;
			if (rgb[0] == max)
			{
				hsv[0] = (rgb[1]-rgb[2])/delta;
			}
			else if (rgb[1] == max)
			{
				hsv[0] = 2+(rgb[2]-rgb[0])/delta;
			}
			else if (rgb[2] == max)
			{
				hsv[0] = 4+(rgb[0]-rgb[1])/delta;
			}
			
			hsv[0] *= 60;
			if (hsv[0] < 0)
				hsv[0]+=360;			
		}
}




static writetorgb_hf(char *pixels, int *dataPtr,
					int height, int width, char *fname, int *hue)
{
    int i, c, x, y;
    int xsize;
    int ysize;
    long win;
	short *dp, *rbuf, *gbuf, *bbuf, intensity;
	IMAGE *image,*outimage;
	double rgb[3], hsv[3], hsv_hue[3], rgb_hue[3];

		rgb_hue[0] = ((double) hue[0])/255;
		rgb_hue[1] = ((double) hue[1])/255;
		rgb_hue[2] = ((double) hue[2])/255;
		rgb_to_hsv(rgb_hue, hsv_hue);
	
    xsize = width;
    ysize = height;
    printf("writing rgb x, y = %d %d\n", width, height);
    /* Set up and open a GL window to display the data */
	
#ifdef WINDOW
	foreground();
    prefsize(xsize,ysize);
    win = winopen("Simplegrab Window");
    RGBmode();
    pixmode(PM_TTOB, 1);
    gconfig();
	printf("Press <Enter> to exit: ");
    c = getc(stdin);
	
	
	lrectwrite(0,0, xsize-1, ysize-1, (ulong *)dataPtr);
	    /* Wait until the user presses a key */
#endif
	
	if( (outimage=iopen(fname,"w",RLE(1),3,width,height,3))
	 == NULL ) {
                printf("readimage: can't open output file\n");
                exit(1);
            }
	
			
	
/* now write to an image file... */

            rbuf = (short *)malloc(width*sizeof(short));
            gbuf = (short *)malloc(width*sizeof(short));
            bbuf = (short *)malloc(width*sizeof(short));
	/* fill buffers with pixel values */
	
i=0;
			
	for(y=height-1; y>=0; y--) 
	{
		for (x=0;x<	width;x++)
		{
		/* printf("%d \n", dataPtr[i]); */
		rbuf[x] = (unsigned short) (dataPtr[i] & 0x000000ff);
		gbuf[x] = (unsigned short) ((dataPtr[i]  >> 8) & 0x000000ff);
		bbuf[x] = (unsigned short) ((dataPtr[i] >> 16) & 0x000000ff);

#ifdef RGB		
		/* filter hue here */
		intensity = 255 - (unsigned short) sqrt( ((double)(
			(rbuf[x]-hue[0])*(rbuf[x]-hue[0])*(rbuf[x]-hue[0]) + 
			(gbuf[x]-hue[1])*(gbuf[x]-hue[1])*(gbuf[x]-hue[1]) +
			(bbuf[x]-hue[2])*(bbuf[x]-hue[2])*bbuf[x]-hue[2]))/3));

		
#else
		rgb[0] = ((double) rbuf[x])/255;
		rgb[1] = ((double) gbuf[x])/255;
		rgb[2] = ((double) bbuf[x])/255;
		rgb_to_hsv(rgb, hsv);

		intensity =255 - 512*sqrt(
			(((10*(hsv[0] - hsv_hue[0])/360) * ((hsv[0] - hsv_hue[0])/360)
			+ 2*(hsv[1] - hsv_hue[1])*(hsv[1] - hsv_hue[1])+ 2*(hsv[2]-hsv_hue[2])*(hsv[2]-hsv_hue[2]))) / 14
			);	

		if (intensity < 0)
			intensity = 0;
#endif
		if (hue[3] > 0)
		{
			if (intensity > hue[3])
				intensity = 255;
			else
				intensity = 0;
		}	
		rbuf[x]=gbuf[x]=bbuf[x]=intensity;
		
		i++;
		
		}	 
		 
		putrow(outimage,rbuf,y,0);
		putrow(outimage,gbuf,y,1);
		putrow(outimage,bbuf,y,2);
    }
	
	iclose(outimage);

	free(rbuf);
	free(gbuf);
	free(bbuf);


}

static writetorgb(char *pixels, int *dataPtr,
					int height, int width, char *fname)
{
    int i, c, x, y;
    int xsize;
    int ysize;
    long win;
	short *dp, *rbuf, *gbuf, *bbuf;
	IMAGE *image,*outimage;


    xsize = width;
    ysize = height;
    printf("writing rgb x, y = %d %d\n", width, height);
    /* Set up and open a GL window to display the data */
	
#ifdef WINDOW
	foreground();
    prefsize(xsize,ysize);
    win = winopen("Simplegrab Window");
    RGBmode();
    pixmode(PM_TTOB, 1);
    gconfig();
	printf("Press <Enter> to exit: ");
    c = getc(stdin);
	
	
	lrectwrite(0,0, xsize-1, ysize-1, (ulong *)dataPtr);
	    /* Wait until the user presses a key */
#endif
	
	if( (outimage=iopen(fname,"w",RLE(1),3,width,height,3))
	 == NULL ) {
                printf("readimage: can't open output file\n");
                exit(1);
            }
	
			
	
/* now write to an image file... */

            rbuf = (short *)malloc(width*sizeof(short));
            gbuf = (short *)malloc(width*sizeof(short));
            bbuf = (short *)malloc(width*sizeof(short));
	/* fill buffers with pixel values */
	
i=0;
			
	for(y=height-1; y>=0; y--) 
	{
		for (x=0;x<	width;x++)
		{
		/* printf("%d \n", dataPtr[i]); */
		rbuf[x] = (unsigned short) (dataPtr[i] & 0x000000ff);
		gbuf[x] = (unsigned short) ((dataPtr[i]  >> 8) & 0x000000ff);
		bbuf[x] = (unsigned short) ((dataPtr[i] >> 16) & 0x000000ff);
		i++;
		
		 }	 
		 
		putrow(outimage,rbuf,y,0);
		putrow(outimage,gbuf,y,1);
		putrow(outimage,bbuf,y,2);
    }
	
	iclose(outimage);

	free(rbuf);
	free(gbuf);
	free(bbuf);




}



static boolean_t WriteTiffFileGreyLevel(char *pixels, int *dataPtr,
					int height, int width, char *fname)
{
  FILE *fpo;
  struct tiff_header_struct tiff_header;
  struct IFD_struct IFD;
  int tiff_header_size, info_len, i;
  
  /* KLUDGE: fool compiler to adjust structure size */
  tiff_header_size = sizeof(struct tiff_header_struct) - 2;
  info_len = 0;
  
  /* open tiff output file */
  if ((fpo = fopen(fname, "wb")) == NULL) {
    printf("vidtotiff() : Could not open output file: %s",
	   fname);
    return(B_FALSE);
  }
  
  /* fill tiff file header fields and write to the file */
  tiff_header.byte_order = HL;
  tiff_header.version = 42;
  tiff_header.IFD_offset = 8;
  tiff_header.IFD_entry_count = 9;
  if (fwrite(&tiff_header, tiff_header_size, 1, fpo) != 1) {
    printf("vidtotiff() : Failed writing tiff header\n");
    return(B_FALSE);
  }
  
  /* fill IFD entries and write to file */
  IFD.image_width.tag = 256;
  IFD.image_width.type = 3;
  IFD.image_width.length = 1;
  IFD.image_width.value_offset = width << 16;
  
  IFD.image_length.tag = 257;
  IFD.image_length.type = 3;
  IFD.image_length.length = 1;
  IFD.image_length.value_offset = height << 16;
  
  IFD.bits_per_sample.tag = 258;
  IFD.bits_per_sample.type = 3;
  IFD.bits_per_sample.length = 1;
  IFD.bits_per_sample.value_offset = 8 << 16;
  
  IFD.photometric_int.tag = 262;
  IFD.photometric_int.type = 3;
  IFD.photometric_int.length = 1;
  IFD.photometric_int.value_offset = 1 << 16;
  
  IFD.image_description.tag = 270;
  IFD.image_description.type = 2;
  IFD.image_description.length = info_len;
  IFD.image_description.value_offset = tiff_header_size
    +sizeof(struct IFD_struct);
  
  IFD.strip_offsets.tag = 273;
  IFD.strip_offsets.type = 4;
  IFD.strip_offsets.length = 1;
  IFD.strip_offsets.value_offset = tiff_header_size
    +sizeof(struct IFD_struct)+info_len;
  
  IFD.samples_per_pixel.tag = 277;
  IFD.samples_per_pixel.type = 3;
  IFD.samples_per_pixel.length = 1;
  IFD.samples_per_pixel.value_offset = 1 << 16;
  
  IFD.rows_per_strip.tag = 278;
  IFD.rows_per_strip.type = 4;
  IFD.rows_per_strip.length = 1;
  IFD.rows_per_strip.value_offset = height;
  
  IFD.strip_byte_counts.tag = 279;
  IFD.strip_byte_counts.type = 4;
  IFD.strip_byte_counts.length = 1;
  IFD.strip_byte_counts.value_offset = height*width;

  IFD.next_IFD_offset = 0;

  /* write IFD blocks */
  if (fwrite(&IFD, sizeof(struct IFD_struct), 1, fpo) != 1) {
    printf("vidtotiff() : Failed writing tiff IFD\n");
    return(B_FALSE);
  }
  
  /* pack the data from 32 bits to 24 bits, flip the image in y */
  for (i = 0; i < width*height; i++)
    pixels[i] = ((double) (dataPtr[i] & 0x000000ff)
		 + ((dataPtr[i] & 0x0000ff00) >> 8)
		 + ((dataPtr[i] & 0x00ff0000) >> 16)) / 3.0;
  
  /* write image data to the output file */
  if (fwrite(pixels, 1, width*height, fpo) != (height*width)) {
    printf("vidtotiff() : Failed writing image data\n");
    return(B_FALSE);
  }

  /* close and return */
  fclose(fpo);
  return(B_TRUE);
}


static void usage(void)
{
  printf("vidtotiff verbose(0/1) <-usage>\n");
}


void main(int argc, char **argv)
{
    int ret_idx, ret_idx2;
    int node_number, node_number2;
    VLDev device, device2;
    IJhandle h, h2;
	int idx = 2; /* 0 = svideo, 1 = compoiste 2 = camera */
	int idx1 = 0;
	int idx2 = 1;
	char filename1[80], filename2[80], filename3[80], filename4[80], filename5[80];
  VLServer svr, svr2;
  VLNode src, drn, src2, drn2;
  VLPath path, path2, paths[2];
  VLControlValue val, val2;
  VLBuffer buffer, buffer2;
  VLInfoPtr info;
  VLDevList devlist;
  VLTransferDescriptor transferDescriptor;
  double tm0, tm;
  int i, *dataPtr, *dataPtr2,  go, xsize, ysize, verbose;
  unsigned long *temp_buf1, *temp_buf2;
  char inbuf[512], *pixels, *pixels2;
  unsigned long *tempbuf1, *tempbuf2;
  int hue[4];

if (argc != 2 && argc !=6)
{
	printf("usage: vidtotiff filename\n");
	exit(0);
}
if (argc ==6)
{
	hue[0] = atoi(argv[2]);
	hue[1] = atoi(argv[3]);
	hue[2] = atoi(argv[4]);
	hue[3] = atoi(argv[5]);
}
else
{
	hue[0]=hue[1]=hue[2]=hue[3] = 0;
}
printf ("hue filter = %d %d %d %d\n", hue[0], hue[1], hue[2], hue[3]);

sprintf(filename1, "%s_l.tif", argv[1]);
sprintf(filename2, "%s_r.tif", argv[1]);
sprintf(filename3, "%s_l.rgb", argv[1]);
sprintf(filename4, "%s_r.rgb", argv[1]);
sprintf(filename5, "%s_hue.rgb", argv[1]);

  /* load input parameters */
	verbose = 1;

  /* Connect to the video server */
  if ((svr = vlOpenVideo(NULL)) == NULL) {
    vlPerror("on return from vlOpenVideo()");
    exit(EXIT_FAILURE);
  }
 if (verbose)
    printf("vidtotiff() : Opened link to video server.\n");
  
  /* Set up a drain node */ 
  
  if ((drn = vlGetNode(svr, VL_DRN, VL_MEM, VL_ANY)) == -1) {
    vlPerror("getting drain node");
    exit(EXIT_FAILURE);
  }
 
  if (verbose)
    printf("vidtotiff() : Opened video drain node.\n");
	

	

h = ijOpenHandle(svr, VL_ANY, drn); 
node_number = ijGetNodeNumber(h, idx1);
device = ijGetVLDev(h);
src = vlGetNode(svr, VL_SRC, VL_VIDEO, node_number);
path = vlCreatePath(svr, device, src, drn);

    if (node_number2 < 0 || src < 0 || path < 0) 
      printf("error 1\n");
   
    if (vlSetupPaths(svr, (VLPathList)&path, 1, VL_SHARE, VL_SHARE) < 0)
      { vlDestroyPath(svr, path); printf("error 2\n"); }
    
    if (ijConfigurePath(h, idx1, 
                        src, drn, path, 
                        node_number, &ret_idx))
      { vlDestroyPath(svr, path); printf("error 3\n");}


  if (verbose)
    printf("vidtotiff() : Opened path between source and drain nodes.\n");
  
  /* Set the packing format */
  val.intVal = VL_PACKING_RGB_8;
  if ((vlSetControl(svr, path, drn, VL_PACKING, &val)) == -1) {
    vlPerror("Setting packing format");
    exit(EXIT_FAILURE);
  } 
  if (verbose)
    printf("vidtotiff() : Set video packing format.\n");
    
  /* Set the capture mode to frames */
  val.intVal = VL_CAPTURE_INTERLEAVED;
  if ((vlSetControl(svr, path, drn, VL_CAP_TYPE, &val)) == -1) {
    vlPerror("Setting the capture type");
    exit(EXIT_FAILURE);
  }
  if (verbose)
    printf("vidtotiff() : Set video capture mode to frames.\n");
    
  /* setup and register the video memory buffer */
  if ((buffer = vlCreateBuffer(svr, path, drn, 1)) == NULL) {
    vlPerror("creating buffer");
    exit(EXIT_FAILURE);
  }
  if (verbose)
    printf("vidtotiff() : Created video capture buffer.\n");

  /* register buffer */
  if ((vlRegisterBuffer(svr, path, drn, buffer)) != VLSuccess) {
    vlPerror("registering buffer");
    exit(EXIT_FAILURE);
  }
  if (verbose)
    printf("vidtotiff() : Registered video capture buffer.\n");

  /* Get the video size, setup buffer & window sizes */
  if (vlGetControl(svr, path, drn, VL_SIZE, &val) == -1) {
    vlPerror("after vlGetControl()");
    exit(EXIT_FAILURE);
  }
#ifdef TIF
  /* allocate space for the pixel buffer */
  
  if ((pixels = malloc(val.xyVal.x * val.xyVal.y)) == NULL) {
    printf("vidtotiff() : Could not malloc %d bytes\n",
	   val.xyVal.x * val.xyVal.y);
    exit(EXIT_FAILURE);
  }
#endif
 
  if ((temp_buf1 = (int *) malloc(val.xyVal.x * val.xyVal.y * sizeof (unsigned long))) == NULL) {
    printf("vidtotiff() : Could not malloc %d bytes\n",
	   val.xyVal.x * val.xyVal.y);
    exit(EXIT_FAILURE);
  }
	if ((temp_buf2 = (int *) malloc(val.xyVal.x * val.xyVal.y * sizeof (unsigned long))) == NULL) {
    printf("vidtotiff() : Could not malloc %d bytes\n",
	   val.xyVal.x * val.xyVal.y);
    exit(EXIT_FAILURE);
  }

 /*----------------------------------------------------------------------*/ 

  /* Begin the continuous data transfer */
  if (vlBeginTransfer(svr, path, 0, NULL) == -1) {
    vlPerror("after vlBeginTransfer()");
    exit(EXIT_FAILURE);
  }

  if (verbose)
    printf("vidtotiff() : Enter filenames to grab, ctrl-d to stop...\n");

	do {
	info = vlGetNextValid(svr, buffer);
      } while (!info);
      
      /* Get a pointer to the frame */
      if ((dataPtr = vlGetActiveRegion(svr, buffer, info)) == NULL) {
	vlPerror("vlGetActiveRegion()");
	exit(EXIT_FAILURE);
      }
	  
	  for (i=0;i<val.xyVal.x * val.xyVal.y;i++)
		temp_buf1[i]= dataPtr[i];
	  vlPutFree(svr, buffer);
	  
	  
#ifdef TIF	  
      /* save the file to disk */
	        if (WriteTiffFileGreyLevel(pixels, dataPtr, val.xyVal.y, val.xyVal.x,
				 filename1)
	  == B_FALSE) {
	exit(EXIT_FAILURE);
      }	 
#endif 


	  
	    

  /* Close the connection to the video server and cleanup */
  vlEndTransfer(svr, path);
  vlDeregisterBuffer(svr, path, drn, buffer);
   vlDestroyBuffer(svr, buffer);
 vlDestroyPath(svr, path); 
/* try opening next path & buffer here*/
/*----------------------------------------------------*/
  h = ijOpenHandle(svr, VL_ANY, drn); 
node_number = ijGetNodeNumber(h, idx2);
device = ijGetVLDev(h);
src = vlGetNode(svr, VL_SRC, VL_VIDEO, node_number);
path = vlCreatePath(svr, device, src, drn);

    if (node_number2 < 0 || src < 0 || path < 0) 
      printf("error 1\n");
   
    if (vlSetupPaths(svr, (VLPathList)&path, 1, VL_SHARE, VL_SHARE) < 0)
      { vlDestroyPath(svr, path); printf("error 2\n"); }
    
    if (ijConfigurePath(h, idx2, 
                        src, drn, path, 
                        node_number, &ret_idx))
      { vlDestroyPath(svr, path); printf("error 3\n");}


  if (verbose)
    printf("vidtotiff() : Opened path between source and drain nodes.\n");
  
  /* Set the packing format */
  val.intVal = VL_PACKING_RGB_8;
  if ((vlSetControl(svr, path, drn, VL_PACKING, &val)) == -1) {
    vlPerror("Setting packing format");
    exit(EXIT_FAILURE);
  } 
  if (verbose)
    printf("vidtotiff() : Set video packing format.\n");
    
  /* Set the capture mode to frames */
  val.intVal = VL_CAPTURE_INTERLEAVED;
  if ((vlSetControl(svr, path, drn, VL_CAP_TYPE, &val)) == -1) {
    vlPerror("Setting the capture type");
    exit(EXIT_FAILURE);
  }
  if (verbose)
    printf("vidtotiff() : Set video capture mode to frames.\n");
    
  /* setup and register the video memory buffer */
  if ((buffer2 = vlCreateBuffer(svr, path, drn, 1)) == NULL) {
    vlPerror("creating buffer");
    exit(EXIT_FAILURE);
  }
  if (verbose)
    printf("vidtotiff() : Created video capture buffer.\n");

  /* register buffer */
  if ((vlRegisterBuffer(svr, path, drn, buffer2)) != VLSuccess) {
    vlPerror("registering buffer");
    exit(EXIT_FAILURE);
  }
  if (verbose)
    printf("vidtotiff() : Registered video capture buffer.\n");

  /* Get the video size, setup buffer & window sizes */
  if (vlGetControl(svr, path, drn, VL_SIZE, &val) == -1) {
    vlPerror("after vlGetControl()");
    exit(EXIT_FAILURE);
  }

  /* allocate space for the pixel buffer */
#ifdef TIF
  if ((pixels2 = malloc(val.xyVal.x * val.xyVal.y)) == NULL) {
    printf("vidtotiff() : Could not malloc %d bytes\n",
	   val.xyVal.x * val.xyVal.y);
    exit(EXIT_FAILURE);
  }
#endif
 /*----------------------------------------------------------------------*/ 

  /* Begin the continuous data transfer */
  if (vlBeginTransfer(svr, path, 0, NULL) == -1) {
    vlPerror("after vlBeginTransfer()");
    exit(EXIT_FAILURE);
  }

  if (verbose)
    printf("vidtotiff() : Enter filenames to grab, ctrl-d to stop...\n");

	do {
	info = vlGetNextValid(svr, buffer2);
      } while (!info);
      
      /* Get a pointer to the frame */
      if ((dataPtr2 = vlGetActiveRegion(svr, buffer2, info)) == NULL) {
	vlPerror("vlGetActiveRegion()");
	
		
	exit(EXIT_FAILURE);
      }
	  for (i=0;i<val.xyVal.x * val.xyVal.y;i++)
		temp_buf2[i]=dataPtr2[i];
	  vlPutFree(svr, buffer2);
      /* save the file to disk */
#ifdef TIF
      if (WriteTiffFileGreyLevel(pixels2, dataPtr2, val.xyVal.y, val.xyVal.x,
				 filename2)
	  == B_FALSE) {
	exit(EXIT_FAILURE);
	 
      }	 
#endif 
	/*writetorgb(pixels, dataPtr, val.xyVal.y, val.xyVal.x,
				 filename3);  
  
	 writetorgb(pixels2, dataPtr2, val.xyVal.y, val.xyVal.x,
				 filename4);*/
/*----------------------------------------------------*/ 

  vlEndTransfer(svr, path);
  vlDeregisterBuffer(svr, path, drn, buffer2);
   vlDestroyBuffer(svr, buffer2);
 vlDestroyPath(svr, path); 
   

  

  
  vlCloseVideo(svr);
  
  	writetorgb(pixels, temp_buf1, val.xyVal.y, val.xyVal.x,
				 filename3); 
	writetorgb_hf(pixels, temp_buf1, val.xyVal.y, val.xyVal.x,
				 filename5, hue); 
  
	 writetorgb(pixels2, temp_buf2, val.xyVal.y, val.xyVal.x,
				 filename4);

  if (verbose)
    printf("vidtotiff terminated normally.\n");
}



