/*******************************************************************************
FILE : userdef_objects.c

LAST MODIFIED : 15 October 2001

DESCRIPTION :
Data structures and functions for user defined graphical objects.

HISTORY :
Used to be in graphics_object.h
==============================================================================*/
#include <stddef.h>
#include <stdio.h>
#include <math.h>
#include "general/debug.h"
#include "general/matrix_vector.h"
#include "graphics/graphics_library.h"
#include "graphics/graphics_object.h"
#include "graphics/userdef_objects.h"
#include "user_interface/message.h"

/*
Module types
------------
*/
enum Userdef_type
/*******************************************************************************
LAST MODIFIED : 26 February 1996

DESCRIPTION :
==============================================================================*/
{
	USERDEF_HAIR=0
}; /* enum Userdef_type */

/* hair/eye edge structure */

struct Userdef_hair
/*******************************************************************************
LAST MODIFIED : 26 February 1996

DESCRIPTION :
==============================================================================*/
{
	enum Userdef_type type;
	int haircnt;
	float hair[100][20][3];
	int hairtypearray[100];
	float t;
	int lod;
}; /* struct Userdef_hair */

/*
Module variables
----------------
*/
#if defined (GL_API) || defined (OPENGL_API)
/*???DB.  For hair */
double seed = 0.5;
float eyecentre[3] = {0.75,2.75,-4.5};
float eyebrownorm[] = {0,1,0};
float eyelashdatahigh[] = {0,-.2,-.1,.3,0.7};
float eyelashdatalow[] = {0,.05,-.05,-.3,-.7};
float eyelashnormh[][3] = {
	{0,.707,.707},
	{0,1,0},
	{0,.447,-.894},
	{0,.707,-.707},
	{0,.83,-.55}};
float eyelashnorml[][3] = {
	{0,0,1},
	{0,0,1},
	{0,.62,.78},
	{0,.707,.707},
	{0,.83,.55}};
float eyebrowl[][3] = {
	{0,0,0},
	{1,2,1},
	{3,2.5,-.5}};
float eyebrowh[][3] = {
	{0,0,0},
	{1,-1,1},
	{3,-1.5,-.5}};
#endif

/*
Module functions
----------------
*/

#if defined (GL_API) || defined (OPENGL_API)

static double myrand(void)
/*******************************************************************************
LAST MODIFIED : 20 February 1993

DESCRIPTION :
==============================================================================*/
{
	if (seed < 0.00001)
	{
		seed += .1;
	}
	seed = cos(1000/seed) * cos(1000/seed);
	return (seed);
} /* myrand */

static void drawfibre(float point1[3],float r1,float point2[3],float r2)
/*******************************************************************************
LAST MODIFIED : 2 November 1995

DESCRIPTION :
==============================================================================*/
{
	int dis = 8 /* 10 */;
	float vert[4][3];
	float norm[3],vec1[3],vec2[3];
	double a,ia;
	int i,k;

	ia = 2*PI/(dis-1);

	for (i=0,a= 0;i<dis;++i,a+=ia)
	{
		vert[0][0] = point1[0] + r1 * cos(a);
		vert[0][1] = point1[1] + r1 * sin(a);
		vert[0][2] = point1[2];
		vert[1][0] = point2[0] + r2 * cos(a);
		vert[1][1] = point2[1] + r2 * sin(a);
		vert[1][2] = point2[2];
		vert[2][0] = point2[0] + r2 * cos(a+ia);
		vert[2][1] = point2[1] + r2 * sin(a+ia);
		vert[2][2] = point2[2];
		vert[3][0] = point1[0] + r1 * cos(a+ia);
		vert[3][1] = point1[1] + r1 * sin(a+ia);
		vert[3][2] = point1[2];
		for (k=0;k<3;k++)
		{
			vec1[k] = vert[1][k] - vert[0][k];
			vec2[k] = vert[3][k] - vert[0][k];
		}
		cross_product_float3(vec2, vec1, norm);
		normalize_float3(norm);
#if defined (GL_API)
		bgnpolygon();
		n3f(norm);
		v3f(vert[0]);
#endif
#if defined (OPENGL_API)
		glBegin(GL_POLYGON);
		/*???RC openGL normals seem to require reversing */
		norm[0] = -norm[0];
		norm[1] = -norm[1];
		norm[2] = -norm[2];
		glNormal3fv(norm);
		glVertex3fv(vert[0]);
#endif
		for (k=1;k<=3;k++)
		{
#if defined (GL_API)
			v3f(vert[k]);
#endif
#if defined (OPENGL_API)
			glVertex3fv(vert[k]);
#endif
		}
#if defined (GL_API)
		endpolygon();
#endif
#if defined (OPENGL_API)
		glEnd();
#endif
	}
} /* drawfibre */

static void draweyelash(float eyelashx[3],float eyelashy[3],float eyelashz[3],
	int lod)
/*******************************************************************************
LAST MODIFIED : 2 November 1995

DESCRIPTION :
==============================================================================*/
{
	float t,tstep,hairx,hairy,hairz,ran1,ran2,ran3;
	float r1=.1,r2=.1,R2 = .1,point1[3],point2[3];
	int i,k;

#if defined (GL_API)
	RGBcolor(0,255,0);
#endif
#if defined (OPENGL_API)
	glColor3ub(0, 255, 0);
#endif
	tstep = 1.0/((float)lod);

	for (i=1;i<3;i++)
	{
		ran1 = 0.8*(myrand() -.5);
		ran2 = 0.8*(myrand() - .5);
		ran3 = (myrand() -.2);
		eyelashx[i] += ran1;
		eyelashy[i] += ran2;
		eyelashz[i] += ran3;
	}

	point1[0] = eyelashx[0];
	point1[1] = eyelashy[0];
	point1[2] = eyelashz[0];
	for (t=0;t<=1;t+=tstep)
	{
		hairx = 2*(1-t)*(.5-t)*eyelashx[0] +
			4*t*(1-t)*eyelashx[1] +
			2*t*(t-.5)*eyelashx[2];
		hairy = 2*(1-t)*(.5-t)*eyelashy[0] +
			4*t*(1-t)*eyelashy[1] +
			2*t*(t-.5)*eyelashy[2];
		hairz = 2*(1-t)*(.5-t)*eyelashz[0] +
			4*t*(1-t)*eyelashz[1] +
			2*t*(t-.5)*eyelashz[2];
		point2[0] = hairx;
		point2[1] = hairy;
		point2[2] = hairz;
		drawfibre(point1,r1,point2,r2);
		r1 = r2;            /* ?? */
		r2 = (1-t) * R2;
		for (k=0;k<3;k++)
		{
			point1[k] = point2[k];
		}
	}
} /* draweyelash */
#endif

static int render_hair(void *hair_struct)
/*******************************************************************************
LAST MODIFIED : 26 February 1996

DESCRIPTION :
Render a hair.
==============================================================================*/
{
	int return_code;
	struct Userdef_hair *hair;
#if defined (GL_API) || defined (OPENGL_API)
	int s,j,k,hairnum,hairoff;
	float eyelashx[3],eyelashy[3],eyelashz[3];
	double t,tt,tstep,ttstep,bend,bend2;
	float hairx,hairy,hairz,browx,browy,browz,hairxprev,hairyprev,hairzprev;
#if defined (GL_API)
	float dummynorm[3] = {0,1,0};
#endif
	float vec1[3],vec2[3],vec3[3];
#endif

	ENTER(render_hair);
	if (hair=(struct Userdef_hair *)hair_struct)
	{
#if defined (GL_API) || defined (OPENGL_API)
		seed = 0.5;
		/*???MS.  I have hardcoded the hair level of detail here */
		hair->lod = 6;
#if defined (GL_API)
		pushmatrix();
		normal(dummynorm);
#endif
#if defined (OPENGL_API)
		glPushMatrix();
		/*glNormal3fv(dummynorm);*/
			/*???Edouard.  Is a normal statement allowed outside a begin/end pair? */
#endif
		for (j=0;j< hair->haircnt;j++)
		{
			switch (hair->hairtypearray[j])
			{
				case 0:
				/*???DB  What do these numbers mean ?  Should be using an enumerated
					type ? */
				{
					tstep = 1.0/10.0 * hair->lod;
#if defined (GL_API)
					move(hair->hair[j][1][0],hair->hair[j][1][1],hair->hair[j][1][2]);
/*          lmbind(MATERIAL,9); */
#endif
#if defined (OPENGL_API)
					glBegin(GL_LINE_STRIP);
					glVertex3f(hair->hair[j][1][0],hair->hair[j][1][1],
						hair->hair[j][1][2]);
					glCallList(9);
						/*???Edouard.  Make sure this number is correct */
#endif
					for (t=0;t<=1;t+=tstep)
					{
						hairx = 2*(1-t)*(.5-t)*hair->hair[j][1][0] +
							4*t*(1-t)*hair->hair[j][2][0] +
							2*t*(t-.5)*hair->hair[j][3][0];
						hairy = 2*(1-t)*(.5-t)*hair->hair[j][1][1] +
							4*t*(1-t)*hair->hair[j][2][1] +
							2*t*(t-.5)*hair->hair[j][3][1];
						hairz = 2*(1-t)*(.5-t)*hair->hair[j][1][2] +
							4*t*(1-t)*hair->hair[j][2][2] +
							2*t*(t-.5)*hair->hair[j][3][2];
#if defined (GL_API)
						draw(hairx,hairy,hairz);
#endif
#if defined (OPENGL_API)
						glVertex3f(hairx, hairy, hairz);
#endif
					}
#if defined (OPENGL_API)
					glEnd();
#endif
				} break;
				case 1:
				{
					tstep = 1.0/55.0;
#if defined (GL_API)
					move(hair->hair[j][1][0],hair->hair[j][1][1],hair->hair[j][1][2]);
#endif
#if defined (OPENGL_API)
					/*???Edouard.  This translation of the IrisGL 'move' statement
						shouldn't be here since the loop below contains a 'move' statement
						as well. It is illegal in OpenGL to call glBegin() while already in
						a glBegin(), so we would have to call glEnd() stright after this
						statement, but this would result in no ouput from the renderer (as
						not enough information would have been passed to OpenGL to draw
						anything).  Therefore the OpenGL translation below is commented out,
						and hopefully the same behaviour will result as did in IrisGL */
/*          glBegin(GL_LINE_STRIP);
					glVertex3f(hair->hair[j][1][0],hair->hair[j][1][1],
						hair->hair[j][1][2]);*/
#endif
					for (t=0;t<=1+tstep/2;t+=tstep)
					{
#if defined (GL_API)
/*            lmbind(MATERIAL,8); */
#endif
#if defined (OPENGL_API)
						glCallList(8);
							/*???Edouard.  Make sure this number is correct */
#endif
						hairxprev = hairx;
						hairyprev = hairy;
						hairzprev = hairz;
						hairx = 2*(1-t)*(.5-t)*hair->hair[j][1][0] +
							4*t*(1-t)*hair->hair[j][2][0] +
							2*t*(t-.5)*hair->hair[j][3][0];
						hairy = 2*(1-t)*(.5-t)*hair->hair[j][1][1] +
							4*t*(1-t)*hair->hair[j][2][1] +
							2*t*(t-.5)*hair->hair[j][3][1];
						hairz = 2*(1-t)*(.5-t)*hair->hair[j][1][2] +
							4*t*(1-t)*hair->hair[j][2][2] +
							2*t*(t-.5)*hair->hair[j][3][2];
/*            bend= drand48()-.5;*/
						bend= myrand()-.5;
						if (hairx > -5)
						{
							/*???Edouard.  I've moved the following rendering code inside the
								if() statement in order to allow OpenGL to have a balanced
								glBegin() and glEnd() without danger of running into another
								glBegin() for the polygon statements in the subsequent if()
								statement */
#if defined (GL_API)
							move(hairx,hairy,hairz);
#endif
#if defined (OPENGL_API)
							glBegin(GL_LINE_STRIP);
							glVertex3f(hairx,hairy,hairz);
#endif
							for (s=0;s<5;s++)
							{
								bend2 = 0.25*(myrand()-.5);
#if defined (GL_API)
								normal(eyelashnormh[s]);
								draw(hairx + s*hairx/11.0+s*bend/3.0+s*bend2/3.0,
									hairy + eyelashdatahigh[s]+s*bend2/3.0,
									hairz + s/3.0+s*bend2/3.0);
#endif
#if defined (OPENGL_API)
								glNormal3fv(eyelashnormh[s]);
								glVertex3f(hairx + s*hairx/11.0+s*bend/3.0+s*bend2/3.0,
									hairy + eyelashdatahigh[s]+s*bend2/3.0,
									hairz + s/3.0+s*bend2/3.0);
#endif
							}
#if defined (OPENGL_API)
							glEnd();
#endif
						}
#if defined (GL_API)
/*            lmbind(MATERIAL,8);*/

#endif
#if defined (OPENGL_API)
						glCallList(8);
							/*???Edouard.  Make sure this number is correct */
#endif
						if (t>0)
						{
							vec1[0] = hairx-hairxprev;
							vec1[1] = hairy-hairyprev;
							vec1[2] = hairz-hairzprev;
							vec2[0] = -(eyecentre[0]-hairxprev)*.2;
							vec2[1] = -(eyecentre[1]-hairyprev)*.2;
							vec2[2] = -(eyecentre[2]-hairzprev)*.2;
							cross_product_float3(vec1, vec2, vec3);
							normalize_float3(vec3);
#if defined (GL_API)
							normal(vec3);
							/* pmv   = polygon move */
							pmv(hairxprev,hairyprev,hairzprev);
							/* pdr   = polygon draw */
							pdr(hairx,hairy,hairz);
							pdr(hairx-vec2[0],hairy-vec2[1],hairz-vec2[2]);
							pdr(hairxprev-vec2[0],hairyprev-vec2[1],hairzprev-vec2[2]);
							/* pclos = polygon close */
							pclos();
#endif
#if defined (OPENGL_API)
							/*???Edouard.  I moved glNormal() inside the begin/end pair */
							glBegin(GL_POLYGON);
							glNormal3fv(vec3);
							glVertex3f(hairxprev, hairyprev, hairzprev);
							glVertex3f(hairx,hairy,hairz);
							glVertex3f(hairx-vec2[0],hairy-vec2[1],hairz-vec2[2]);
							glVertex3f(hairxprev-vec2[0],hairyprev-vec2[1],hairzprev-vec2[2]);
							glEnd();
#endif
						}
					}
				} break;
				case 2:
				{
					tstep = 1.0/20.0;
#if defined (GL_API)
					move(hair->hair[j][1][0],hair->hair[j][1][1],hair->hair[j][1][2]);
#endif
#if defined (OPENGL_API)
					/*???Edouard.  As in the previous section of code above, we are faced
						with the problem of how to translate the IrisGL 'move' statement.  A
						call to glVertex() outside a begin/end pair is said to result in
						"undefined behaviour" (OpenGL specification, version 1.0, page 20),
						so we would have to put a glBegin() before it, and follow it with a
						glEnd() in order to not hit another glBegin() for the move inside
						the for() loop below. At the monent I'll provide a translation, but
						leave it commeted out, as I can't see what it's supposed to be
						doing */
/*          glBegin(GL_LINE_STRIP);
					glVertex3f(hair->hair[j][1][0],hair->hair[j][1][1],
						hair->hair[j][1][2]);
					glEnd();*/
#endif
					for (t=0;t<=1+tstep/2;t+=tstep)
					{
#if defined (GL_API)
/*            lmbind(MATERIAL,9);*/
#endif
#if defined (OPENGL_API)
						glCallList(9);
							/*???Edouard.  Check this number is the correct list index */
#endif
						hairxprev = hairx;
						hairyprev = hairy;
						hairzprev = hairz;
						hairx = 2*(1-t)*(.5-t)*hair->hair[j][1][0] +
							4*t*(1-t)*hair->hair[j][2][0] +
							2*t*(t-.5)*hair->hair[j][3][0];
						hairy = 2*(1-t)*(.5-t)*hair->hair[j][1][1] +
							4*t*(1-t)*hair->hair[j][2][1] +
							2*t*(t-.5)*hair->hair[j][3][1];
						hairz = 2*(1-t)*(.5-t)*hair->hair[j][1][2] +
							4*t*(1-t)*hair->hair[j][2][2] +
							2*t*(t-.5)*hair->hair[j][3][2];
/*            bend= drand48()-.5;*/
						bend= myrand()-.5;
						if (t>.1)
						{
							/*???Edouard.  I moved 'move()' inside the if statement to make
								sure its OpenGL translation (starting with a glBegin() ) doesn't
								run into any other glBegin()s */
#if defined (GL_API)
							move(hairx,hairy,hairz);
#endif
#if defined (OPENGL_API)
							glBegin(GL_LINE_STRIP);
							glVertex3f(hairx,hairy,hairz);
#endif
							for (s=0;s<5;s++)
							{
#if defined (GL_API)
								normal(eyelashnorml[s]);
								draw(hairx + s*hairx/13.0+s*bend/2.0,
									hairy + eyelashdatalow[s],
									hairz + s/4.0);
#endif
#if defined (OPENGL_API)
								glNormal3fv(eyelashnorml[s]);
								glVertex3f(hairx + s*hairx/13.0+s*bend/2.0,
									hairy + eyelashdatalow[s],
									hairz + s/4.0);
#endif
							}
#if defined (OPENGL_API)
							glEnd();
#endif
						}
#if defined (GL_API)
/*            lmbind(MATERIAL,8);*/
#endif
#if defined (OPENGL_API)
						glCallList(8);
							/*???Edouard.  Check this number is the correct list index */
#endif
						if (t>0)
						{
							vec1[0] = hairx-hairxprev;
							vec1[1] = hairy-hairyprev;
							vec1[2] = hairz-hairzprev;
							vec2[0] = (eyecentre[0]-hairxprev)*.2;
							vec2[1] = (eyecentre[1]-hairyprev)*.2;
							vec2[2] = (eyecentre[2]-hairzprev)*.2;
							cross_product_float3(vec1, vec2, vec3);
							normalize_float3(vec3);
#if defined (GL_API)
							normal(vec3);
							pmv(hairxprev,hairyprev,hairzprev);
							pdr(hairx,hairy,hairz);
							pdr(hairx+vec2[0],hairy+vec2[1],hairz+vec2[2]);
							pdr(hairxprev+vec2[0],hairyprev+vec2[1],hairzprev+vec2[2]);
/*              pdr(eyecentre[0],eyecentre[1],eyecentre[2]);*/
							pclos();
#endif
#if defined (OPENGL_API)
							glBegin(GL_POLYGON);
							glNormal3fv(vec3);
							glVertex3f(hairxprev,hairyprev,hairzprev);
							glVertex3f(hairx,hairy,hairz);
							glVertex3f(hairx+vec2[0],hairy+vec2[1],hairz+vec2[2]);
							glVertex3f(hairxprev+vec2[0],hairyprev+vec2[1],hairzprev+vec2[2]);
/*              glVertex3f(eyecentre[0],eyecentre[1],eyecentre[2]);*/
							glEnd();
#endif
						}
					}
				} break;
				case 3:
				{
					tstep = 1.0/60.0;
					ttstep = 1.0/8.0;
#if defined (GL_API)
/*          lmbind(MATERIAL,9);*/
					normal(eyebrownorm);
#endif
#if defined (OPENGL_API)
					glCallList(9);
						/*???Edouard.  Make sure this number is the correct list index */
					/*???Edouard.  I know calling glVertex() outside of a begin/end pair
						is a Bad Thing. But I cannot seem to find a definitive ruling on
						whether calling glNormal() outside of a begin/end has any problems.
						The example code seems to all put glNormal() inside the begin/end
						pairs, but the specification seem to imply the the normal is held in
						state, and so could be specified at any time. If the below call
						turns out to be a problem, then the it will have to be moved inside
						the begin/end pairing */
					glNormal3fv(eyebrownorm);
#endif
					for (t=0;t<=1;t+=tstep)
					{
						hairx = 2*(1-t)*(.5-t)*hair->hair[j][1][0] +
							4*t*(1-t)*hair->hair[j][2][0] +
							2*t*(t-.5)*hair->hair[j][3][0];
						hairy = 2*(1-t)*(.5-t)*hair->hair[j][1][1] +
							4*t*(1-t)*hair->hair[j][2][1] +
							2*t*(t-.5)*hair->hair[j][3][1];
						hairz = 2*(1-t)*(.5-t)*hair->hair[j][1][2] +
							4*t*(1-t)*hair->hair[j][2][2] +
							2*t*(t-.5)*hair->hair[j][3][2];
/*            bend= drand48()-.5;*/
						bend = myrand()-.5;
#if defined (GL_API)
						move(hairx+bend/10.0,hairy+bend/10.0,hairz);
#endif
#if defined (OPENGL_API)
						glBegin(GL_LINE_STRIP);
						glVertex3f(hairx+bend/10.0,hairy+bend/10.0,hairz);
#endif
						for (tt=0;tt<=1;tt+=ttstep)
						{
							browx = 2*(1-tt)*(.5-tt)*eyebrowl[0][0] +
								4*tt*(1-tt)*eyebrowl[1][0] +
								2*tt*(tt-.5)*eyebrowl[2][0];
							browy = 2*(1-tt)*(.5-tt)*eyebrowl[0][1] +
								4*tt*(1-tt)*eyebrowl[1][1] +
								2*tt*(tt-.5)*eyebrowl[2][1];
							browz = 2*(1-tt)*(.5-tt)*eyebrowl[0][2] +
								4*tt*(1-tt)*eyebrowl[1][2] +
								2*tt*(tt-.5)*eyebrowl[2][2];
#if defined (GL_API)
							draw(hairx+browx+tt*bend,hairy+browy*(1-t)+tt*bend,hairz+browz);
#endif
#if defined (OPENGL_API)
							glVertex3f(hairx+browx+tt*bend,hairy+browy*(1-t)+tt*bend,
								hairz+browz);
#endif
						}
#if defined (OPENGL_API)
						glEnd();
#endif
					}
				} break;
				case 4:
				{
					tstep = 1.0/60.0;
					ttstep = 1.0/8.0;
#if defined (GL_API)
/*          lmbind(MATERIAL,9);*/
					normal(eyebrownorm);
#endif
#if defined (OPENGL_API)
					glCallList(9);
						/*???Edouard.  Check this is the correct list index */
					glNormal3fv(eyebrownorm);
						/*???Edouard.  See the comment above about glNormal() placement */
#endif
					for (t=0;t<=1;t+=tstep)
					{
						hairx = 2*(1-t)*(.5-t)*hair->hair[j][1][0] +
							4*t*(1-t)*hair->hair[j][2][0] +
							2*t*(t-.5)*hair->hair[j][3][0];
						hairy = 2*(1-t)*(.5-t)*hair->hair[j][1][1] +
							4*t*(1-t)*hair->hair[j][2][1] +
							2*t*(t-.5)*hair->hair[j][3][1];
						hairz = 2*(1-t)*(.5-t)*hair->hair[j][1][2] +
							4*t*(1-t)*hair->hair[j][2][2] +
							2*t*(t-.5)*hair->hair[j][3][2];
/*            bend = drand48()-.5;*/
						bend = myrand()-.5;
						hairx += bend/10.0;
						hairy += bend/10.0;
#if defined (GL_API)
						move(hairx,hairy,hairz);
#endif
#if defined (OPENGL_API)
						glBegin(GL_LINE_STRIP);
						glVertex3f(hairx,hairy,hairz);
#endif
						for (tt=0;tt<=1;tt+=ttstep)
						{
							browx = 2*(1-tt)*(.5-tt)*eyebrowh[0][0] +
								4*tt*(1-tt)*eyebrowh[1][0] +
								2*tt*(tt-.5)*eyebrowh[2][0];
							browy = 2*(1-tt)*(.5-tt)*eyebrowh[0][1] +
								4*tt*(1-tt)*eyebrowh[1][1] +
								2*tt*(tt-.5)*eyebrowh[2][1];
							browz = 2*(1-tt)*(.5-tt)*eyebrowh[0][2] +
								4*tt*(1-tt)*eyebrowh[1][2] +
								2*tt*(tt-.5)*eyebrowh[2][2];
#if defined (GL_API)
							draw(hairx+browx+tt*bend,hairy+browy*(1-0.1*t)+tt*bend,
								hairz+browz);
#endif
#if defined (OPENGL_API)
							glVertex3f(hairx+browx+tt*bend,hairy+browy*(1-0.1*t)+tt*bend,
								hairz+browz);
#endif
						}
#if defined (OPENGL_API)
						glEnd();
#endif
					}
				} break;
				case 5: case 6:
				/* general eyelash */
				{
					if (5==hair->hairtypearray[j])
					{
						tstep = 1.0/30.0;
					}
					else
					{
						tstep = 1.0/20.0;
					}
					for (t=0;t<=1;t+=tstep)
					{
						hairnum = 0;                            /*!!!*/
						for (k=0,hairoff=0;k<3;k++,hairoff += 5)
						{
							eyelashx[k] = 2*(1-t)*(.5-t)*hair->hair[j][1+hairnum+hairoff][0] +
								4*t*(1-t)*hair->hair[j][2+hairnum+hairoff][0] +
								2*t*(t-.5)*hair->hair[j][3+hairnum+hairoff][0];
							eyelashy[k] =2*(1-t)*(.5-t)*hair->hair[j][1+hairnum+hairoff][1] +
								4*t*(1-t)*hair->hair[j][2+hairnum+hairoff][1] +
								2*t*(t-.5)*hair->hair[j][3+hairnum+hairoff][1];
							eyelashz[k] = 2*(1-t)*(.5-t)*hair->hair[j][1+hairnum+hairoff][2] +
								4*t*(1-t)*hair->hair[j][2+hairnum+hairoff][2] +
								2*t*(t-.5)*hair->hair[j][3+hairnum+hairoff][2];
						}
						draweyelash(eyelashx,eyelashy,eyelashz,hair->lod);
						hairnum = 2;
						for (k=0,hairoff=0;k<3;k++,hairoff += 5)
						{
							eyelashx[k] = 2*(1-t)*(.5-t)*hair->hair[j][1+hairnum+hairoff][0] +
								4*t*(1-t)*hair->hair[j][2+hairnum+hairoff][0] +
								2*t*(t-.5)*hair->hair[j][3+hairnum+hairoff][0];
							eyelashy[k] = 2*(1-t)*(.5-t)*hair->hair[j][1+hairnum+hairoff][1] +
								4*t*(1-t)*hair->hair[j][2+hairnum+hairoff][1] +
								2*t*(t-.5)*hair->hair[j][3+hairnum+hairoff][1];
							eyelashz[k] = 2*(1-t)*(.5-t)*hair->hair[j][1+hairnum+hairoff][2] +
								4*t*(1-t)*hair->hair[j][2+hairnum+hairoff][2] +
								2*t*(t-.5)*hair->hair[j][3+hairnum+hairoff][2];
						}

						draweyelash(eyelashx,eyelashy,eyelashz,hair->lod);
					}
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,"render_hair.  Unknown hairtype");
					return_code=0;
				} break;
			}
		}
#if defined (GL_API)
		popmatrix();
#endif
#if defined (OPENGL_API)
		glPopMatrix();
#endif
		return_code=1;
#else
		return_code=0;
#endif
	}
	else
	{
		display_message(ERROR_MESSAGE,"render_hair.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* render_hair */

#if defined (OLD_CODE)
static int robot_7dof;
#endif /* defined (OLD_CODE) */

static int render_robot_7dof(void *dummy)
/*******************************************************************************
LAST MODIFIED : 26 February 1996

DESCRIPTION :
Render a 7 dof heart surgery robot.
==============================================================================*/
{
	int return_code;

	ENTER(render_robot_7dof);
	USE_PARAMETER(dummy);
#if defined (GL_API)
	callobj(robot_7dof);
	return_code=1;
#else
	return_code=0;
#endif
	LEAVE;

	return (return_code);
} /* render_robot_7dof */

/*
Global functions
----------------
*/
struct GT_userdef *create_robot_7dof(void)
/*******************************************************************************
LAST MODIFIED : 19 June 1998

DESCRIPTION :
Creates a 7 dof heart surgery robot.
==============================================================================*/
{
	struct GT_userdef *robot_7dof_object;
#if defined (GL_API)
	double a,b,cf,ia,ib;
	float norm[3],r,sphere_norm[4][3],sphere_vert[4][3],vert[3];
	int cone,cylinder,discretization=20,i,j,k,l,scalpel_1,scalpel_2,sphere,tool;
	static int first_call=1;
#endif

	ENTER(create_robot_7dof);
	if (ALLOCATE(robot_7dof_object,struct GT_userdef,1))
	{
#if defined (GL_API)
		if (first_call)
		{
			/* make the cylinder */
			cylinder=genobj();
			makeobj(cylinder);
			ia = 2.0*PI/((double) discretization);
			for (i=0,a=0;i<discretization;i++,a+=ia)
			{
				bgnpolygon();
				vert[0] = cos(a);
				vert[1] = sin(a);
				vert[2] = 0.0;
				norm[0] = -vert[0];
				norm[1] = -vert[1];
				norm[2] = -vert[2];
				n3f(norm);
				v3f(vert);
				vert[2] = 1.0;
				v3f(vert);
				vert[0] = cos(a+ia);
				vert[1] = sin(a+ia);
				norm[0] = -vert[0];
				norm[1] = -vert[1];
				n3f(norm);
				v3f(vert);
				vert[2] = 0.0;
				v3f(vert);
				endpolygon();
			}
			for (i=0,a=0;i<discretization;i++,a+=ia)
			{
				bgnpolygon();
				norm[0] = norm[1] = 0.0;
				norm[2] = 1.0;
				vert[0] = cos(a);
				vert[1] = sin(a);
				vert[2] = 0.0;
				n3f(norm);
				v3f(vert);
				vert[0] = cos(a+ia);
				vert[1] = sin(a+ia);
				v3f(vert);
				vert[0] = vert[1] = 0.0;
				v3f(vert);
				endpolygon();
				bgnpolygon();
				norm[0] = norm[1] = 0.0;
				norm[2] = -1.0;
				vert[0] = cos(a);
				vert[1] = sin(a);
				vert[2] = 1.0;
				n3f(norm);
				v3f(vert);
				vert[0] = cos(a+ia);
				vert[1] = sin(a+ia);
				v3f(vert);
				vert[0] = vert[1] = 0.0;
				v3f(vert);
				endpolygon();
			}
			closeobj();
			/* make the sphere */
			sphere=genobj();
			makeobj(sphere);
			r=1.0;
			ia=PI/6;
			ib=2.0*PI/ 6;
			for (i=0,a=PI;i<=6;++i,a+=ia)
			{
				for (j=0,b=0;j<=6;++j,b+=ib)
				{
					sphere_vert[3][0] = r * sin(a) * cos(b);
					sphere_vert[3][1] = r * sin(a) * sin(b);
					sphere_vert[3][2] = r * cos(a);
					sphere_vert[2][0] = r * sin(a) * cos(b + ib);
					sphere_vert[2][1] = r * sin(a) * sin(b + ib);
					sphere_vert[2][2] = r * cos(a);
					sphere_vert[1][0] = r * sin(a + ia) * cos(b + ib);
					sphere_vert[1][1] = r * sin(a + ia) * sin(b + ib);
					sphere_vert[1][2] = r * cos(a + ia);
					sphere_vert[0][0] = r * sin(a + ia) * cos(b);
					sphere_vert[0][1] = r * sin(a + ia) * sin(b);
					sphere_vert[0][2] = r * cos(a + ia);
					for (k = 0;k<=3;k++)
					{
						for (l=0;l<=2;++l)
						{
							sphere_norm[k][l] = -sphere_vert[k][l] / r;
						}
					}
					bgnpolygon();
					for (k=0;k<4;k++)
					{
						n3f(sphere_norm[k]);
						v3f(sphere_vert[k]);
					}
					endpolygon();
				}
			}
			closeobj();
			/* make the cone */
			cone=genobj();
			makeobj(cone);
			cf = 0.6 /* 0.3 */;
			ia = 2.0*PI/((double) discretization);
			for (i=0,a=0;i<discretization;i++,a+=ia)
			{
				bgnpolygon();
				vert[0] = cos(a);
				vert[1] = sin(a);
				vert[2] = 0.0;
				norm[0] = -vert[0];
				norm[1] = -vert[1];
				norm[2] = -vert[2];
				n3f(norm);
				v3f(vert);
				vert[2] = 1.0;
				vert[0] *= cf; vert[1] *= cf;
				v3f(vert);
				vert[0] = cos(a+ia);
				vert[1] = sin(a+ia);
				norm[0] = -vert[0];
				norm[1] = -vert[1];
				vert[0] *= cf;
				vert[1] *= cf;
				n3f(norm);
				v3f(vert);
				vert[0] = cos(a+ia);
				vert[1] = sin(a+ia);
				norm[0] = -vert[0];
				norm[1] = -vert[1];
				vert[2] = 0.0;
				v3f(vert);
				endpolygon();
			}
			for (i=0,a=0;i<discretization;i++,a+=ia)
			{
				bgnpolygon();
				norm[0] = norm[1] = 0.0;
				norm[2] = 1.0;
				vert[0] = cos(a);
				vert[1] = sin(a);
				vert[2] = 0.0;
				n3f(norm);
				v3f(vert);
				vert[0] = cos(a+ia);
				vert[1] = sin(a+ia);
				v3f(vert);
				vert[0] = vert[1] = 0.0;
				v3f(vert);
				endpolygon();
				bgnpolygon();
				norm[0] = norm[1] = 0.0;
				norm[2] = -1.0;
				vert[0] = cf*cos(a);
				vert[1] = cf*sin(a);
				vert[2] = 1.0;
				n3f(norm);
				v3f(vert);
				vert[0] = cf*cos(a+ia);
				vert[1] = cf*sin(a+ia);
				v3f(vert);
				vert[0] = vert[1] = 0.0;
				v3f(vert);
				endpolygon();
			}
			closeobj();
			/* make scalpel */
			scalpel_1=genobj();
			makeobj(scalpel_1);
			norm[0] = 0.0;
			norm[1] = -1.0;
			norm[2] = 0.0;
			bgnpolygon();
			n3f(norm);
			vert[0] = -1.0;
			vert[1] = 1.0; vert[2] = 0;
			v3f(vert);
			vert[1] = 0.0;
			vert[0] = -1.0; vert[2] = 1.8;
			v3f(vert);
			vert[0] = 1.0; vert[2] = 1.35;
			v3f(vert);
			vert[0] = 1.0; vert[2] = 0.0;
			v3f(vert);
			endpolygon();
			norm[0] = 0.0;
			norm[1] = 1.0;
			norm[2] = 0.0;
			bgnpolygon();
			n3f(norm);
			vert[0] = -1.0;
			vert[1] = -1.0; vert[2] = 0;
			v3f(vert);
			vert[1] = 0.0;
			vert[0] = -1.0; vert[2] = 1.9;
			v3f(vert);
			vert[0] = 1.0; vert[2] = 1.35;
			v3f(vert);
			vert[0] = 1.0; vert[2] = 0.0;
			v3f(vert);
			endpolygon();
			norm[0] = 1.0;
			norm[1] = 0.0;
			norm[2] = 0.0;
			bgnpolygon();
			n3f(norm);
			vert[0] = -1.0;
			vert[1] = -1.0; vert[2] = 0;
			v3f(vert);
			vert[1] = 0.0;
			vert[0] = -1.0; vert[2] = 1.9;
			v3f(vert);
			vert[1] = 1.0;
			vert[0] = -1.0; vert[2] = 0.0;
			v3f(vert);
			endpolygon();
			closeobj();
			/* make scalpel */
			scalpel_2=genobj();
			makeobj(scalpel_2);
			norm[0] = 0.0;
			norm[1] = 1.0;
			norm[2] = 0.0;
			bgnpolygon();
			n3f(norm);
			vert[0] = 0.0;
			vert[1] = vert[2] = 0;
			v3f(vert);
			vert[0] = 1.0; vert[2] = 1.0;
			v3f(vert);
			vert[0] = 0.0; vert[2] = 2.0;
			v3f(vert);
			vert[0] = -1.0; vert[2] = 1.0;
			v3f(vert);
			endpolygon();
			closeobj();
			/* make the tool */
			tool=genobj();
			makeobj(tool);
			pushmatrix();
			scale(0.75,0.75,25.0);
			callobj(cylinder);
			popmatrix();
			translate(0,0,25.0);
			pushmatrix();
			scale(0.6,0.6,10.7);
			callobj(cone);
			popmatrix();
			translate(0,0,10.2);
			scale(0.4,1.0,1.5);
			callobj(scalpel_2);
			closeobj();
			/* make the robot */
			robot_7dof=genobj();
			makeobj(robot_7dof);
			pushmatrix();
			translate(0,0,-10.0);
			pushmatrix();
			translate(0,0,8);
			scale(0.5,0.5,0.5);
			scale(0.34,0.175,1.5);
			callobj(scalpel_1);
			popmatrix();
			pushmatrix();
			pushmatrix();
			pushmatrix();
			scale(0.75,0.75,0.75);
			callobj(sphere);
			popmatrix();
			translate(0,0,-8.5);
			pushmatrix();
			scale(0.75,0.75,0.75);
			callobj(sphere);
			popmatrix();
			popmatrix();
			translate(0,0,-10.0);
			scale(0.5,0.5,0.5);
			callobj(tool);
			popmatrix();
			pushmatrix();
			rot(150.0,'x');
			pushmatrix();
			scale(0.4,0.4,56.0);
			callobj(cylinder);
			popmatrix();
			pushmatrix();
			pushmatrix();
			translate(0,0,45.0);
			scale(4,4,5.0);
			callobj(cylinder);
			popmatrix();
			translate(0,0,50.5);
			scale(4,4,5.0);
			callobj(cylinder);
			popmatrix();
			translate(0,0,7);
			rot(45.0,'z');
			rot(90.0,'x');
			pushmatrix();
			translate(0,0,.3);
			scale(0.3,0.3,0.3);
			callobj(sphere);
			popmatrix();
			pushmatrix();
			scale(0.2,0.2,5.0);
			callobj(cylinder);
			popmatrix();
			translate(0,0,5.0);
			pushmatrix();
			scale(0.3,0.3,0.3);
			callobj(sphere);
			popmatrix();
			rot(-90.0,'z');
			rot(120.0,'x');
			scale(0.2,0.2,4.0);
			callobj(cylinder);
			popmatrix();
			pushmatrix();
			rot(120.0,'z');
			rot(150.0,'x');
			pushmatrix();
			scale(0.4,0.4,56.0);
			callobj(cylinder);
			popmatrix();
			pushmatrix();
			pushmatrix();
			translate(0,0,45.0);
			scale(4,4,5.0);
			callobj(cylinder);
			popmatrix();
			translate(0,0,50.5);
			scale(4,4,5.0);
			callobj(cylinder);
			popmatrix();
			translate(0,0,7);
			rot(45.0,'z');
			rot(90.0,'x');
			pushmatrix();
			translate(0,0,.3);
			scale(0.3,0.3,0.3);
			callobj(sphere);
			popmatrix();
			pushmatrix();
			scale(0.2,0.2,5.0);
			callobj(cylinder);
			popmatrix();
			translate(0,0,5.0);
			pushmatrix();
			scale(0.3,0.3,0.3);
			callobj(sphere);
			popmatrix();
			rot(-90.0,'z');
			rot(120.0,'x');
			scale(0.2,0.2,4.0);
			callobj(cylinder);
			popmatrix();
			pushmatrix();
			rot(240,'z');
			rot(150.0,'x');
			pushmatrix();
			scale(0.4,0.4,56.0);
			callobj(cylinder);
			popmatrix();
			pushmatrix();
			pushmatrix();
			translate(0,0,45.0);
			scale(4,4,5.0);
			callobj(cylinder);
			popmatrix();
			translate(0,0,50.5);
			scale(4,4,5.0);
			callobj(cylinder);
			popmatrix();
			translate(0,0,7);
			rot(45.0,'z');
			rot(90.0,'x');
			pushmatrix();
			translate(0,0,.3);
			scale(0.3,0.3,0.3);
			callobj(sphere);
			popmatrix();
			pushmatrix();
			scale(0.2,0.2,5.0);
			callobj(cylinder);
			popmatrix();
			translate(0,0,5.0);
			pushmatrix();
			scale(0.3,0.3,0.3);
			callobj(sphere);
			popmatrix();
			rot(-90.0,'z');
			rot(120.0,'x');
			scale(0.2,0.2,4.0);
			callobj(cylinder);
			popmatrix();
			popmatrix();
			closeobj();
			first_call=0;
		}
#endif
		robot_7dof_object->data=(void *)NULL;
		robot_7dof_object->render_function=render_robot_7dof;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_robot_7dof.  Could not create 7 dof robot");
	}
	LEAVE;

	return (robot_7dof_object);
} /* create_robot_7dof */

int file_read_userdef(FILE *file,void *userdef_address_void)
/*******************************************************************************
LAST MODIFIED : 12 August 1998

DESCRIPTION :
Reads a user defined object from a graphics object <file>. A pointer to the
newly created object is put at <*userdef_address>.
==============================================================================*/
{
	enum Userdef_type type;
	int i,j,k,return_code;
	struct GT_userdef **userdef_address;
	struct Userdef_hair *hair;
	void *userdef_data;
	int (*userdef_destroy_function)(void **);
	int (*userdef_render_function)(void *);
 
	ENTER(file_read_userdef);
	/* check arguments */
	if (file&&(userdef_address=(struct GT_userdef **)userdef_address_void))
	{
		/* make sure the userdef is initially clear */
		*userdef_address=(struct GT_userdef *)NULL;
		/* read the type */
		fscanf(file,"%d",&type);
		switch (type)
		{
			case USERDEF_HAIR:
			{
				if (ALLOCATE(hair,struct Userdef_hair,1))
				{
					hair->t=0.0;
					fscanf(file,"%d",&(hair->haircnt));
					for (i=0;i<hair->haircnt;i++)
					{
						fscanf(file,"%d",&(hair->hairtypearray[i]));
						if (hair->hairtypearray[i]>4)
						{
							for (k=1;k<16;k++)
							{
								for (j=0;j<3;j++)
								{
									fscanf(file,"%f",&(hair->hair[i][k][j]));
								}
							}
						}
					}
					userdef_data=hair;
					/*???RC Should really specify a destroy function */
					userdef_destroy_function=NULL;
					userdef_render_function=render_hair;
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"file_read_userdef.  Could not allocate hair");
					return_code=0;
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"file_read_userdef.  Unknown user defined graphics object");
				return_code=0;
			} break;
		}
		if (return_code)
		{
			if (!(*userdef_address=CREATE(GT_userdef)(userdef_data,
				userdef_destroy_function,userdef_render_function)))
			{
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"file_read_userdef.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* file_read_userdef */
