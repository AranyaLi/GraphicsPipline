/* Texture functions for cs580 GzLib	*/
#include    "stdafx.h" 
#include	"stdio.h"
#include	"Gz.h"
#include <cmath>

GzColor	*image=NULL;
int xs, ys;
int reset = 1;
//#define	INDEX(x,y)	(x+(y*xs))
void bilinear_interpolate(GzColor *image, float u, float v, GzColor result, int xs);

/* Image texture function */
int tex_fun(float u, float v, GzColor color)
{
  unsigned char		pixel[3];
  unsigned char     dummy;
  char  		foo[8];
  int   		i, j;
  FILE			*fd;

  if (reset) {          /* open and load texture file */
    fd = fopen ("texture", "rb");
    if (fd == NULL) {
      fprintf (stderr, "texture file not found\n");
      exit(-1);
    }
    fscanf (fd, "%s %d %d %c", foo, &xs, &ys, &dummy);
    image = (GzColor*)malloc(sizeof(GzColor)*(xs+1)*(ys+1));
    if (image == NULL) {
      fprintf (stderr, "malloc for texture image failed\n");
      exit(-1);
    }

    for (i = 0; i < xs*ys; i++) {	/* create array of GzColor values */
      fread(pixel, sizeof(pixel), 1, fd);
      image[i][RED] = (float)((int)pixel[RED]) * (1.0 / 255.0);
      image[i][GREEN] = (float)((int)pixel[GREEN]) * (1.0 / 255.0);
      image[i][BLUE] = (float)((int)pixel[BLUE]) * (1.0 / 255.0);
      }

    reset = 0;          /* init is done */
	fclose(fd);
  }

/* bounds-test u,v to make sure nothing will overflow image array bounds */
  if(u>1)
	  u=1;
  if(u<0)
	  u=0;
  if(v>1)
	  v=1;
  if(v<0)
	  v=0;
/* determine texture cell corner values and perform bilinear interpolation */
  float scaledu, scaledv;
  GzColor result;
  scaledu = u*( xs -1 );
  scaledv = v*( ys -1 );
  bilinear_interpolate(image, scaledu, scaledv, result, xs);
/* set color to interpolated GzColor value and return */
  color[RED] = result[RED];
  color[GREEN] = result[GREEN];
  color[BLUE] = result[BLUE];
  return GZ_SUCCESS;
}

/* Procedural texture function */
int ptex_fun(float u, float v, GzColor color)
{
     int x=floor(u*4);
	 int y=floor(v*4);
	 if((x+y)%2==0){
	 color[0]=0.5;color[1]=0.5;color[2]=0.5;
	 }
	 else{
	  color[0]=1;color[1]=1;color[2]=1;
	 }
	 return GZ_SUCCESS;
}

/* Free texture memory */
int GzFreeTexture()
{
	if(image!=NULL)
		free(image);
	return GZ_SUCCESS;
}

void bilinear_interpolate(GzColor *image, float u, float v, GzColor result, int xs){
	GzColor A, B, C, D;
	GzCoord Av, Bv, Cv, Dv;
	float s, t;
	Av[X] = floorf(u);
	Av[Y] = floorf(v);
	Bv[X] = ceilf(u);
	Bv[Y] = floorf(v);
	Cv[X] = ceilf(u);
	Cv[Y] = ceilf(v);
	Dv[X] = floorf(u);
	Dv[Y] = ceilf(v);
	s = u - floorf(u);
	t = v - floorf(v);
	A[RED] = image[(int)(Av[X]+ Av[Y] *xs)][RED];
	A[GREEN] = image[(int)(Av[X]+ Av[Y] *xs)][GREEN];
	A[BLUE] = image[(int)(Av[X]+ Av[Y] *xs)][BLUE];

	B[RED] = image[(int)(Bv[X]+ Bv[Y] *xs)][RED];
	B[GREEN] = image[(int)(Bv[X]+ Bv[Y] *xs)][GREEN];
	B[BLUE] = image[(int)(Bv[X]+ Bv[Y] *xs)][BLUE];

	C[RED] = image[(int)(Cv[X]+ Cv[Y] *xs)][RED];
	C[GREEN] = image[(int)(Cv[X]+ Cv[Y] *xs)][GREEN];
	C[BLUE] = image[(int)(Cv[X]+ Cv[Y] *xs)][BLUE];

	D[RED] = image[(int)(Dv[X]+ Dv[Y] *xs)][RED];
	D[GREEN] = image[(int)(Dv[X]+ Dv[Y] *xs)][GREEN];
	D[BLUE] = image[(int)(Dv[X]+ Dv[Y] *xs)][BLUE];

	result[RED] =	s *t *C[RED] + (1-s)* t *D[RED] + s *(1-t) *B[RED] + (1-s) *(1-t)* A[RED];
	result[GREEN] =	s *t *C[GREEN] + (1-s)* t *D[GREEN] + s *(1-t) *B[GREEN] + (1-s) *(1-t)* A[GREEN];
	result[BLUE] =	s *t *C[BLUE] + (1-s)* t *D[BLUE] + s *(1-t) *B[BLUE] + (1-s) *(1-t)* A[BLUE];
}