/* cc -O -o RGBDeinterlace RGBDeinterlace.c -limage */
#include <stdio.h>
#include <stdlib.h>
#include <gl/image.h>
#include <gl/gl.h>
void main(int argc, char **argv)
{
  IMAGE *image, *outimage;
  unsigned short *rbuf0, *gbuf0, *bbuf0, *rbuf1, *gbuf1, *bbuf1;
  int x, y, k, height, width, field, ydelta;
  
  /* check input */
  if (argc != 4) {
    printf("Usage : RGBDeinterlace infilename outfilname field\n");
    exit(0);
  }

  /* check the field specifier */
  field = atoi(argv[3]);
  if ((field != 0) && (field != 1)) {
    printf("RGBDeinterlace : Invalid field specifier (%d).\n", field);
    exit(-1);
  }

  /* open the input file */
  if ((image = iopen(argv[1], "r")) == NULL) {
    printf("RGBDeinterlace : Could not open the file \"%s\" for reading.\n", argv[1]);
    exit(-1);
  }

  /* extract the image size parameters */
  height = image->ysize;
  width  = image->xsize;
  
  /* open the output image */
  if((outimage = iopen(argv[2], "w", RLE(1), 3, width, height, 3)) == NULL ) {
    printf("RGBDeinterlace : Could not open the file \"%s\" for writing.\n", argv[2]);
    iclose(image);
    exit(-1);
  }

  /* allocate storage for the row buffers */
  rbuf0 = (unsigned short *) malloc((image->xsize)*sizeof(unsigned short));
  gbuf0 = (unsigned short *) malloc((image->xsize)*sizeof(unsigned short));
  bbuf0 = (unsigned short *) malloc((image->xsize)*sizeof(unsigned short));
  rbuf1 = (unsigned short *) malloc((image->xsize)*sizeof(unsigned short));
  gbuf1 = (unsigned short *) malloc((image->xsize)*sizeof(unsigned short));
  bbuf1 = (unsigned short *) malloc((image->xsize)*sizeof(unsigned short));
  if ((rbuf0 == NULL) || (gbuf0 == NULL) || (bbuf0 == NULL) ||
      (rbuf1 == NULL) || (gbuf1 == NULL) || (bbuf1 == NULL)) {
    printf("Could not malloc(%d,%d) for color buffers\n", width, height);
    iclose(image);   
    iclose(outimage);   
    exit(-1);
  }
  
  /* read in the data, convert to luminance */
  for(y = field, k = 0, ydelta = (field ? -1 : +1);
      y < (image->ysize & 0x01 ? image->ysize -1 : image->ysize);
      y+=2 ) {
    /* read in two rows */
    getrow(image, rbuf0, y, 0);
    getrow(image, gbuf0, y, 1);
    getrow(image, bbuf0, y, 2);

    /* write out the rows */
    putrow(outimage, rbuf0, y, 0);
    putrow(outimage, gbuf0, y, 1);
    putrow(outimage, bbuf0, y, 2);
    putrow(outimage, rbuf0, y+ydelta, 0);
    putrow(outimage, gbuf0, y+ydelta, 1);
    putrow(outimage, bbuf0, y+ydelta, 2);
  }

  /* check last row, if required */
  if (y < image->ysize) {
    getrow(image, rbuf0, y, 0);
    getrow(image, gbuf0, y, 1);
    getrow(image, bbuf0, y, 2);
    putrow(outimage, rbuf0, y, 0);
    putrow(outimage, gbuf0, y, 1);
    putrow(outimage, bbuf0, y, 2);
  }

  /* close the files */
  iclose(image);
  iclose(outimage);

  /* free the storage space */
  free(rbuf0);
  free(gbuf0);
  free(bbuf0);
  free(rbuf1);
  free(gbuf1);
  free(bbuf1);
}
