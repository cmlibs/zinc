#include <stdio.h>

#define N_IMAGES          11
#define N_DIGITIZED_NODES 52

char *fnames[N_IMAGES] = {
  "/usr/people/paulc/speckle/March03/img000.exnode",
  "/usr/people/paulc/speckle/March03/img050.exnode",
  "/usr/people/paulc/speckle/March03/img100.exnode",
  "/usr/people/paulc/speckle/March03/img150.exnode",
  "/usr/people/paulc/speckle/March03/img200.exnode",
  "/usr/people/paulc/speckle/March03/img250.exnode",
  "/usr/people/paulc/speckle/March03/img300.exnode",
  "/usr/people/paulc/speckle/March03/img350.exnode",
  "/usr/people/paulc/speckle/March03/img400.exnode",
  "/usr/people/paulc/speckle/March03/img450.exnode",
  "/usr/people/paulc/speckle/March03/img500.exnode"
};

double xdnodes[N_DIGITIZED_NODES][N_IMAGES];
double ydnodes[N_DIGITIZED_NODES][N_IMAGES];

struct {
  int cn, xn;
} nodes[N_DIGITIZED_NODES] = {
  {37, 52},
  {38, 49},
  {39, 51},
  {40, 48},
  {41, 45},
  {42, 47},
  {43, 44},
  {44, 41},
  {45, 43},
  {46, 40},
  {47, 37},
  {48, 39},
  {49, 36},
  {50, 33},
  {51, 35},
  {52, 32},
  {53, 29},
  {54, 31},
  {55, 28},
  {56, 25},
  {57, 27},
  {58, 24},
  {59, 21},
  {60, 23},
  {61, 20},
  {62, 17},
  {63, 19},
  {64, 16},
  {65, 13},
  {66, 15},
  {67, 12},
  {68, 9},
  {69, 11},
  {70, 8},
  {71, 5},
  {72, 7},
  {73, 4},
  {74, 1},
  {75, 3},
  {3, 2},
  {8, 6},
  {15, 10},
  {22, 14},
  {29, 18},
  {34, 22},
  {35, 26},
  {36, 30},
  {33, 34},
  {28, 38},
  {21, 42},
  {14, 46},
  {7, 50}
};

void main(void)
{
  FILE *fp;
  char s[256];
  int i, nfile, n, go;

  /* loop for all files */
  for (nfile = 0; nfile < N_IMAGES; nfile++) {
    /* open input file */
    if ((fp = fopen(fnames[nfile], "r")) == NULL) {
      printf("Could not open file \"%s\"\n", fnames[nfile]);
      exit(EXIT_FAILURE);
    }
    
    /* find start of node desctiption */
    for (go = 1; go; ) {
      if (fgets(s, 256, fp)) {
/*
	printf("Read in \"%s\" (lenght : %d)\n", s, strlen(s));
*/
	if (strcmp(s, " Node :10000\n") == 0) {
	  go = 0;
	}
      }
      else {
	go = 0;
      }
    }
    
    /* loop for all digitized nodes */
    printf("Reading nodes from file \"%s\"\n", fnames[nfile]);
    for (n = 0; n < N_DIGITIZED_NODES; n++) {
      /* skip over node number */
      if (n > 0)
	fgets(s, 256, fp);
      /* read in x,y coordinates */
      if (fscanf(fp, "%le %le\n", &(xdnodes[n][nfile]), &(ydnodes[n][nfile]))
	  != 2) {
	printf("Error parsing node %d in file %s\n", n, fnames[nfile]);
	exit(EXIT_FAILURE);
      }
    }

    /* close the input file */
    fclose(fp);
  }

  /* write the x data to the output file */
  if ((fp = fopen("xnodes.dat", "w")) == NULL) {
    printf("Could not open file \"ynodes.dat\"\n");
    exit(EXIT_FAILURE);
  }
  for (n = 0; n < N_DIGITIZED_NODES; n++) {
    for (nfile = 0; nfile < N_IMAGES; nfile++) {
      fprintf(fp, "%f ", xdnodes[n][nfile]);
    }
    fprintf(fp, "\n");
  }
  fclose(fp);

  /* write the y data to the output file */
  if ((fp = fopen("ynodes.dat", "w")) == NULL) {
    printf("Could not open file \"ynodes.dat\"\n");
    exit(EXIT_FAILURE);
  }
  for (n = 0; n < N_DIGITIZED_NODES; n++) {
    for (nfile = 0; nfile < N_IMAGES; nfile++) {
      fprintf(fp, "%f ", ydnodes[n][nfile]);
    }
    fprintf(fp, "\n");
  }
  fclose(fp);
}
  


