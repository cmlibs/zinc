void CenterOfMass(double *inpixels, int height, int width, double *buffer,
		  double NodeX, double NodeY, double Radius,
		  double *CMX, double *CMY)
{
  double v, median, cmx, cmy, w;
  int i, j, k, swap, x, y;

  /* if radius is 0.0, no filtering required */
  if (Radius == 0.0) {
    *CMX = NodeX;
    *CMY = NodeY;
    return;
  }

  /* bubble sort the left node neighbourhood (yeah, I know...) */
  for (y = rint(NodeY - Radius), k = 0; y <= rint(NodeY + Radius); y++) {
    for (x = rint(NodeX - Radius); x <= rint(NodeX + Radius); x++) {
      if ((x > 0) && (x < width) && (y > 0) && (y < height)) {
	/* load the next pixel in the array */
	buffer[k] = inpixels[y*width + x];
	/* shift the new value down to keep values ordered from min to max */
	for (i = k, swap = 1; (i > 0) && (swap != 0); i--) {
	  if (buffer[i] < buffer[i-1]) {
	    v = buffer[i-1];
	    buffer[i-1] = buffer[i];
	    buffer[i] = v;
	  }
	  else {
	    swap = 0;
	  }
	}
	/* increment insertion index */
	k++;
      }
    }
  }
  median = buffer[k/2];
  
  /* calculate the center of mass of the left node and write it out */
  cmx = 0.0;
  cmy = 0.0;
  w   = 0.0;
  for (y = rint(NodeY - Radius); y <= rint(NodeY + Radius); y++) {
    for (x = rint(NodeX - Radius); x <= rint(NodeX + Radius); x++) {
      if ((x > 0) && (x < width) && (y > 0) && (y < height)) {
	/* calculate weight */
	v = (median - inpixels[y*width + x]);
	v = (v > 0.0 ? v*v : 0.0);
	/* update running sums */
	cmx += ((double) x) * v;
	cmy += ((double) y) * v;
	w   += v;
      }
    }
  }
  if (w > 0.0) {
    cmx /= w;
    cmy /= w;
  }

  /* return results */
  *CMX = cmx;
  *CMY = cmy;
}

