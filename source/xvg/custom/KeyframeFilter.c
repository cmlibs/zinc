/***********************************************************************
*
*  Name:          KeyframeFilter.c
*
*  Author:        Paul Charette
*
*  Last Modified: 15 July 1997
*
*  Purpose:       Filter.
*
*
***********************************************************************/
#include <stdio.h>
#include <stlib.h>
#include <math.h>

void main(int argc, char **argv)
{
  FILE *fpi0, *fpi1, *fpo;
  double Threshold, NodeXL0, NodeYL0, NodeXR0, NodeYR0,
  NodeXL1, NodeYL1, NodeXR1, NodeYR1;
  char cbuf[512], buf[512];
  int i, k, nc;

  /* check input arguments */
  if (argc != 5) {
    printf("Usage: KeyframeFilter infilename_0 infilename_1 outfilename threshold\n");
    exit(-1);
  }

  /* load in threshold */
  Threshold = atof(argv[4]);

  /* open first input file */
  if ((fpi0 = fopen(argv[1], "r")) == NULL) {
    printf("Could not open input file \"%s\"\n", argv[1]);
    exit(-1)
  }

  /* open second input file */
  if ((fpi1 = fopen(argv[2], "r")) == NULL) {
    printf("Could not open input file \"%s\"\n", argv[2]);
    exit(-1)
  }

  /* open output file */
  if ((fpo = fopen(argv[3], "w")) == NULL) {
    printf("Could not open output file \"%s\"\n", argv[3]);
    exit(-1)
  }

  
  /* loop to read in node coordinates */
  for (k = 0, nc = 0;
       (fgets(buf0, CBUF_SIZE, fpi0) != NULL)
       && (fgets(buf1, CBUF_SIZE, fpi1) != NULL);
       k++) {
    /* load in the nodes from the first file */
    if (fscanf(fpn, "%le %le %le %le\n",
	       &NodeXL0, &NodeYL0, &NodeXR0, &NodeYR0) != 4) {
      printf("Error parsing file \"%s\" at line %d\n", argv[1], k);
      fclose(fpi0);
      fclose(fpi1);
      fclose(fpo);
      exit(-1);
    }

    /* load in the nodes from the second file */
    if (fscanf(fpn, "%le %le %le %le\n",
	       &NodeXL1, &NodeYL1, &NodeXR1, &NodeYR1) != 4) {
      printf("Error parsing file \"%s\" at line %d\n", argv[2], k);
      fclose(fpi0);
      fclose(fpi1);
      fclose(fpo);
      exit(-1);
    }

    /* compare the node coordinate differences to the threshold */
    if ((fabs(NodeXL0 - NodeXL1) > Threshold) ||
	(fabs(NodeXR0 - NodeXR1) > Threshold) ||
	(fabs(NodeYL0 - NodeYL1) > Threshold) ||
	(fabs(NodeYR0 - NodeYR1) > Threshold)) {
      printf("Coordinate differences exceed threshold at node %d\n", k);
      nc++;

      /* write out the node information to the file */
      fprintf(fpo, "C-%s", buf0);
      fprintf(fpo,  "%8.3f %8.3f %8.3f %8.3f\n",
	      NodeXL1, NodeYL1, NodeXR1, NodeYR1);
    }
    else {
      /* write out the node information to the file */
      fprintf(fpo, "%s", buf0);
      fprintf(fpo,  "%8.3f %8.3f %8.3f %8.3f\n",
	      NodeXL0, NodeYL0, NodeXR0, NodeYR0);
    }
  }

  /* clean up and exit */
  printf("Done, %d node coordinate differences exceed %.3f pixels\n", nc);
  fclose(fpi0);
  fclose(fpi1);
  fclose(fpo);
}


