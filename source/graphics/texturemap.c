/*******************************************************************************
FILE : texturemap.c

LAST MODIFIED : 6 October 1998

DESCRIPTION :
???MS.  Uses SGI gl/image.h library for reading and writing to .rgb files
???DB.  Should this be using general/image_utilities.c ?
==============================================================================*/
#include <math.h>
#include "general/debug.h"
#include "general/image_utilities.h"
#if defined (OLD_CODE)
#include "general/geometry.h"
#include "graphics/graphics_object.h"
#include "graphics/graphics_window.h"
#include "graphics/scene.h"
#endif /* defined (OLD_CODE) */
#include "graphics/texturemap.h"
#include "graphics/volume_texture.h"
#if defined (OLD_CODE)
#include "three_d_drawing/ThreeDDraw.h"
#endif /* defined (OLD_CODE) */
#include "user_interface/message.h"
#include "user_interface/user_interface.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_to_graphics_object.h"

/*
Module functions
----------------
*/
static void destroy_texturemap_image(struct Image_buffer *image)
/*******************************************************************************
LAST MODIFIED : 7 April 1997

DESCRIPTION :
==============================================================================*/
{
	int y;

	ENTER(destroy_texturemap_image);
	if (image)
	{
		for (y=0;y<image->ysize;y++)
		{
			DEALLOCATE(image->rbuf[y]);
			DEALLOCATE(image->gbuf[y]);
			DEALLOCATE(image->bbuf[y]);
			DEALLOCATE(image->abuf[y]);
		}
		DEALLOCATE(image->rbuf);
		DEALLOCATE(image->gbuf);
		DEALLOCATE(image->bbuf);
		DEALLOCATE(image->abuf);
		DEALLOCATE(image);
	}
	LEAVE;
} /* destroy_texturemap_image */

struct Image_buffer *read_texturemap_image(char *infile)
/*******************************************************************************
LAST MODIFIED : 6 March 1997

DESCRIPTION :
Reads in background image to create textures from
==============================================================================*/
{
	char newname[80];
	int number_of_bytes_per_component,number_of_components,x,y,z;
	int long height,width;
	short **rbuf,**gbuf,**bbuf,**abuf;
	struct Image_buffer *in_image;
	unsigned char *pixel;
	unsigned long *image;

	ENTER(read_texturemap_image);
	if (read_rgb_image_file(infile,&number_of_components,&number_of_bytes_per_component,
		&height,&width,&image))
	{
		if (number_of_bytes_per_component == 1)
		{
			/*???debug.  Print a little info about the image */
			printf("Image x and y size in pixels: %ld %ld\n",width,height);
			printf("Image zsize in channels: %ld\n",number_of_components);
			if (ALLOCATE(in_image,struct Image_buffer,1))
			{
				in_image->xsize=width;
				in_image->ysize=height;
				in_image->zsize=4;
				in_image->rbuf=(short **)NULL;
				in_image->gbuf=(short **)NULL;
				in_image->bbuf=(short **)NULL;
				in_image->abuf=(short **)NULL;
				if (ALLOCATE(in_image->rbuf,short *,in_image->ysize)&&
					ALLOCATE(in_image->gbuf,short *,in_image->ysize)&&
					ALLOCATE(in_image->bbuf,short *,in_image->ysize)&&
					ALLOCATE(in_image->abuf,short *,in_image->ysize))
				{
					(in_image->rbuf)[0]=(short *)NULL;
					(in_image->gbuf)[0]=(short *)NULL;
					(in_image->bbuf)[0]=(short *)NULL;
					(in_image->abuf)[0]=(short *)NULL;
					pixel=(unsigned char *)image;
					y=0;
					while ((y<in_image->ysize)&&
						ALLOCATE(in_image->rbuf[y],short,in_image->xsize)&&
						ALLOCATE(in_image->gbuf[y],short,in_image->xsize)&&
						ALLOCATE(in_image->bbuf[y],short,in_image->xsize)&&
						ALLOCATE(in_image->abuf[y],short,in_image->xsize))
					{
						for (x=0;x<in_image->xsize;x++)
						{
							switch (number_of_components)
							{
								case 1:
								{
									/* I */
									(in_image->rbuf)[y][x]=(short)*pixel;
									(in_image->gbuf)[y][x]=(short)*pixel;
									(in_image->bbuf)[y][x]=(short)*pixel;
									(in_image->abuf)[y][x]=(short)*pixel;
									pixel++;
								} break;
								case 2:
								{
									/* IA */
									(in_image->rbuf)[y][x]=(short)*pixel;
									(in_image->gbuf)[y][x]=(short)*pixel;
									(in_image->bbuf)[y][x]=(short)*pixel;
									pixel++;
									(in_image->abuf)[y][x]=(short)*pixel;
									pixel++;
								} break;
								case 3:
								{
									/* RGB */
									(in_image->rbuf)[y][x]=(short)*pixel;
									pixel++;
									(in_image->gbuf)[y][x]=(short)*pixel;
									pixel++;
									(in_image->bbuf)[y][x]=(short)*pixel;
									pixel++;
									(in_image->abuf)[y][x]=(short)255;
								} break;
								case 4:
								{
									/* RGBA */
									(in_image->rbuf)[y][x]=(short)*pixel;
									pixel++;
									(in_image->gbuf)[y][x]=(short)*pixel;
									pixel++;
									(in_image->bbuf)[y][x]=(short)*pixel;
									pixel++;
									(in_image->abuf)[y][x]=(short)*pixel;
									pixel++;
								} break;
							}
						}
						y++;
						if (y<in_image->ysize)
						{
							(in_image->rbuf)[y]=(short *)NULL;
							(in_image->gbuf)[y]=(short *)NULL;
							(in_image->bbuf)[y]=(short *)NULL;
							(in_image->abuf)[y]=(short *)NULL;
						}
					}
					if (y<in_image->ysize)
					{
						display_message(ERROR_MESSAGE,
							"read_texturemap_image.  Alloc failed for component rows");
						while (y>=0)
						{
							DEALLOCATE((in_image->rbuf)[y]);
							DEALLOCATE((in_image->gbuf)[y]);
							DEALLOCATE((in_image->bbuf)[y]);
							DEALLOCATE((in_image->abuf)[y]);
							y--;
						}
						DEALLOCATE(in_image->rbuf);
						DEALLOCATE(in_image->gbuf);
						DEALLOCATE(in_image->bbuf);
						DEALLOCATE(in_image->abuf);
						DEALLOCATE(in_image);
						in_image=(struct Image_buffer *)NULL;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"read_texturemap_image.  Alloc failed for component buffers");
					DEALLOCATE(in_image->rbuf);
					DEALLOCATE(in_image->gbuf);
					DEALLOCATE(in_image->bbuf);
					DEALLOCATE(in_image->abuf);
					DEALLOCATE(in_image);
					in_image=(struct Image_buffer *)NULL;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"read_texturemap_image.  Alloc failed for image buffer");
			}
			DEALLOCATE(image);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"read_texturemap_image."
				"  Not implemented for more than one byte per component");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"read_texturemap_image.  Can't open input file %s\n",infile);
		in_image=(struct Image_buffer *)NULL;
	}
	LEAVE;

	return(in_image);
} /* read_texturemap_image */

static void write_texturemap_image(char *outfile,struct Image_buffer *in_image)
/*******************************************************************************
LAST MODIFIED : 6 March 1997

DESCRIPTION :
Writes generated texture image to file
==============================================================================*/
{
	int height,number_of_components,width,x,y,z;
	unsigned char *component;
	unsigned long *image;

	/* check arguments */
	if (outfile&&in_image)
	{
		width=in_image->xsize;
		height=in_image->ysize;
		number_of_components=in_image->zsize;
		if (ALLOCATE(image,unsigned long,width*height))
		{
			component=(unsigned char *)image;
			for (y=0;y<height;y++)
			{
				for (x=0;x<width;x++)
				{
					*component=(unsigned char)((in_image->rbuf)[y][x]);
					component++;
					*component=(unsigned char)((in_image->gbuf)[y][x]);
					component++;
					*component=(unsigned char)((in_image->bbuf)[y][x]);
					component++;
					*component=(unsigned char)((in_image->abuf)[y][x]);
					component++;
				}
			}
			write_rgb_image_file(outfile,number_of_components,/*bytes_per_component*/1,
				height,width,0,image);
			DEALLOCATE(image);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"write_texturemap_image.  Could not allocate image");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_texturemap_image.  Invalid argument(s)");
	}
	LEAVE;
} /* write_texturemap_image */

/*
Global functions
----------------
*/
int generate_textureimage_from_FE_element(
	struct Graphics_window *graphics_window,char *infile,char *outfile,
	struct FE_element *element,double ximax[3],
	struct Computed_field *coordinate_field)
/*******************************************************************************
LAST MODIFIED : 15 February 1999

DESCRIPTION :
Creates texture map segment (range[0,ximax[3]]) SGI rgb image <out_image> from
<in_image> by interpolating and projecting FE element surface onto normalized
image space
==============================================================================*/
{
	double temp_matrix[16],*vector,xsized,ysized;
	GLdouble model_matrix[16],projection_matrix[16],windowx,windowy,windowz;
	GLint default_viewport[4]={0,0,1,1};
	int i,j,k,return_code,x,xpixel,xsize,y,ypixel,ysize,z,zsize;
	short *rbuf, *gbuf, *bbuf, *abuf, *inbuf, *outbuf;
	struct Image_buffer *in_image,*out_image;
	struct Scene_viewer *scene_viewer;
	struct VT_vector_field vector_field,*warped_field;

	ENTER(generate_textureimage_from_FE_element);
	if (graphics_window&&(scene_viewer=Graphics_window_get_Scene_viewer(
		graphics_window,0))&&infile&&outfile&&element&&ximax&&coordinate_field)
	{
/*???debug */
printf("Create texmap: input image = %s, output = %s:  seed_element = %d <cf = %p>. Extent = %lf %lf %lf\n ",
	infile,outfile,element->cm.number,coordinate_field,ximax[0],ximax[1],ximax[2]);
		if (in_image=read_texturemap_image(infile))
		{
			xsize=in_image->xsize;
			ysize=in_image->ysize;
			zsize=in_image->zsize;
			xsized=(double) in_image->xsize;
			ysized=(double) in_image->ysize;
/*???debug */
printf("x, y size d = %lf %lf\n", xsized, ysized);
			if (ALLOCATE(out_image,struct Image_buffer,1))
			{
				out_image->xsize=xsize;
				out_image->ysize=ysize;
				out_image->zsize=zsize;
				out_image->rbuf=(short **)NULL;
				out_image->gbuf=(short **)NULL;
				out_image->bbuf=(short **)NULL;
				out_image->abuf=(short **)NULL;
/*???debug */
printf("allocated out_image x, y = %d %d\n",out_image->xsize,out_image->ysize);
				ALLOCATE(out_image->rbuf,short *,out_image->ysize);
				ALLOCATE(out_image->gbuf,short *,out_image->ysize);
				ALLOCATE(out_image->bbuf,short *,out_image->ysize);
				ALLOCATE(out_image->abuf,short *,out_image->ysize);
				if ((out_image->rbuf)&&(out_image->gbuf)&&(out_image->bbuf)&&
					(out_image->abuf))
				{
					return_code=1;
					y=0;
					while (return_code&&(y<ysize))
					{
						ALLOCATE(out_image->rbuf[y],short,out_image->xsize);
						ALLOCATE(out_image->gbuf[y],short,out_image->xsize);
						ALLOCATE(out_image->bbuf[y],short,out_image->xsize);
						ALLOCATE(out_image->abuf[y],short,out_image->xsize);
						if ((out_image->rbuf[y])&&(out_image->gbuf[y])&&
							(out_image->bbuf[y])&&(out_image->abuf[y]))
						{
							y++;
						}
						else
						{
							display_message(ERROR_MESSAGE,
			"generate_textureimage_from_FE_element.  Alloc3 failed for image buffer");
							return_code=0;
							while (y>=0)
							{
								DEALLOCATE(out_image->rbuf[y]);
								DEALLOCATE(out_image->gbuf[y]);
								DEALLOCATE(out_image->bbuf[y]);
								DEALLOCATE(out_image->abuf[y]);
								y--;
							}
							DEALLOCATE(out_image->rbuf);
							DEALLOCATE(out_image->gbuf);
							DEALLOCATE(out_image->bbuf);
							DEALLOCATE(out_image->abuf);
							DEALLOCATE(out_image);
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
			"generate_textureimage_from_FE_element.  Alloc2 failed for image buffer");
					return_code=0;
					display_message(ERROR_MESSAGE, "alloc failed for buffer *");
					DEALLOCATE(out_image->rbuf);
					DEALLOCATE(out_image->gbuf);
					DEALLOCATE(out_image->bbuf);
					DEALLOCATE(out_image->abuf);
					DEALLOCATE(out_image);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
			"generate_textureimage_from_FE_element.  Alloc1 failed for image buffer");
				return_code=0;
			}
			if (return_code)
			{
				/* generate vector field over element */
				vector_field.dimension[0]=xsize-1;
				vector_field.dimension[1]=ysize-1;
				vector_field.dimension[2]=1-1;
				if (ALLOCATE(vector,double,(int)xsize*ysize*3))
				{
/*???debug */
printf("allocated %d doubles\n", xsize*ysize*3);
					vector_field.vector=vector;
					/* 1:create xi space mesh over element surface */
					for (i=0;i<xsize;i++)
					{
						for (j=0;j<ysize;j++)
						{
							/*???RC Why swap y and x? */
							vector[3*(i+j*xsize)+1]=((double)i)/((double)(xsize-1))*ximax[1];
							vector[3*(i+j*xsize)+0]=((double)j)/((double)(ysize-1))*ximax[0];
							vector[3*(i+j*xsize)+2]=0.0;
						}
					}
					/* 2:interpolate points (this function will do across blocks if
						ximax > 1 */
					if (warped_field=interpolate_vector_field_on_FE_element(ximax,element,
						coordinate_field,&vector_field))
					{
						/* 3:project warped_field points onto normalized window */
						Scene_viewer_get_modelview_matrix(scene_viewer,temp_matrix);
						/* transpose to OpenGL matrix format */
						for (i=0;i<4;i++)
						{
							for (j=0;j<4;j++)
							{
								model_matrix[i*4+j]=(GLdouble)temp_matrix[j*4+i];
							}
						}
						Scene_viewer_get_window_projection_matrix(scene_viewer,temp_matrix);
						/* transpose to OpenGL matrix format */
						for (i=0;i<4;i++)
						{
							for (j=0;j<4;j++)
							{
								projection_matrix[i*4+j]=(GLdouble)temp_matrix[j*4+i];
							}
						}
#if defined (OLD_CODE)
						/* set active window to 3D window */
						X3dThreeDDrawingMakeCurrent(
							graphics_window->drawing->drawing_widget);
						glGetDoublev(GL_MODELVIEW_MATRIX,model_matrix);
						glGetDoublev(GL_PROJECTION_MATRIX,projection_matrix);
#endif /* defined (OLD_CODE) */
						for (i=0;i<xsize*ysize;i++)
						{
							gluProject(warped_field->vector[3*i+0],
								warped_field->vector[3*i+1],warped_field->vector[3*i+2],
								model_matrix,projection_matrix,default_viewport,&windowx,
								&windowy,&windowz);
							/*???RC Why swap y and x? */
							vector[3*i+1]=(double)(windowx*xsized);
							vector[3*i+0]=(double)(windowy*ysized);
							vector[3*i+2]=(double)(windowz*0);
						}
						/* 4: look up pixel in in_image and store in out_image */
/*???debug */
printf("xsize = %d ysize = %d\n", xsize, ysize);
						for (i=0;i<xsize;i++)
						{
							for (j=0;j<ysize;j++)
							{
								/* look up in_image pixel at vector[3*i] location */
								xpixel=(int)vector[3*(i+j*xsize)+0];
								ypixel=(int)vector[3*(i+j*xsize)+1];
								if (xpixel>=xsize)
								{
									xpixel=xsize-1;
								}
								else
								{
									if (0>xpixel)
									{
										xpixel=0;
									}
								}
								if (ypixel>=ysize)
								{
									ypixel=ysize-1;
								}
								else
								{
									if (0>ypixel)
									{
										ypixel=0;
									}
								}
								/* copy in_image[xpixel,ypixel] to out_image[i,j] */
								/* could do image processing step here (e.g. smoothing) */
								outbuf=out_image->rbuf[i];
								inbuf=in_image->rbuf[xpixel];
								outbuf[j]=inbuf[ypixel];
								outbuf=out_image->gbuf[i];
								inbuf=in_image->gbuf[xpixel];
								outbuf[j]=inbuf[ypixel];
								outbuf=out_image->bbuf[i];
								inbuf=in_image->bbuf[xpixel];
								outbuf[j]=inbuf[ypixel];
								outbuf=out_image->abuf[i];
								inbuf=in_image->abuf[xpixel];
								outbuf[j]=inbuf[ypixel];
							}
						}
						/* 5: write out_image to file */
						write_texturemap_image(outfile,out_image);
						DEALLOCATE(vector);
						DEALLOCATE(warped_field->vector);
						DEALLOCATE(warped_field);
						destroy_texturemap_image(in_image);
						destroy_texturemap_image(out_image);
					}
					else
					{
						display_message(ERROR_MESSAGE,
	"generate_textureimage_from_FE_element.  Could not interpolate vector field");
						return_code=0;
						y=ysize;
						while (y>0)
						{
							y--;
							DEALLOCATE(out_image->rbuf[y]);
							DEALLOCATE(out_image->gbuf[y]);
							DEALLOCATE(out_image->bbuf[y]);
							DEALLOCATE(out_image->abuf[y]);
						}
						DEALLOCATE(out_image->rbuf);
						DEALLOCATE(out_image->gbuf);
						DEALLOCATE(out_image->bbuf);
						DEALLOCATE(out_image->abuf);
						DEALLOCATE(out_image);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
		"generate_textureimage_from_FE_element.  Could not allocate vector field");
					return_code=0;
					y=ysize;
					while (y>0)
					{
						y--;
						DEALLOCATE(out_image->rbuf[y]);
						DEALLOCATE(out_image->gbuf[y]);
						DEALLOCATE(out_image->bbuf[y]);
						DEALLOCATE(out_image->abuf[y]);
					}
					DEALLOCATE(out_image->rbuf);
					DEALLOCATE(out_image->gbuf);
					DEALLOCATE(out_image->bbuf);
					DEALLOCATE(out_image->abuf);
					DEALLOCATE(out_image);
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"generate_textureimage_from_FE_element.  Could not open image file");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"generate_textureimage_from_FE_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* generate_textureimage_from_FE_element */
