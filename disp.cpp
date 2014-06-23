/*   CS580 HW   */
#include    "stdafx.h"  
#include	"Gz.h"
#include	"disp.h"

unsigned char scale12to8(short x){
	unsigned char y;
	y = x>>4;
	return y;
}

int GzNewFrameBuffer(char** framebuffer, int width, int height)
{
/* create a framebuffer:
 -- allocate memory for framebuffer : (sizeof)GzPixel x width x height
 -- pass back pointer 
 -- NOTE: this function is optional and not part of the API, but you may want to use it within the display function.
*/
	*framebuffer = new char[3*sizeof(char)*width*height];

	return GZ_SUCCESS;
}

int GzNewDisplay(GzDisplay	**display, int xRes, int yRes)
{
/* create a display:
  -- allocate memory for indicated resolution
  -- pass back pointer to GzDisplay object in display
*/
	*display = new GzDisplay();
	(*display)->fbuf = new GzPixel[xRes*yRes];
	if(xRes<MAXXRES)
	(*display)->xres = xRes;
	else
		(*display)->xres = MAXXRES;
	if(yRes<MAXYRES)
	(*display)->yres = yRes;
	else
		(*display)->yres = MAXYRES;
	return GZ_SUCCESS;
}


int GzFreeDisplay(GzDisplay	*display)
{
/* clean up, free memory */

	delete(display->fbuf);
	delete(display);

	return GZ_SUCCESS;
}


int GzGetDisplayParams(GzDisplay *display, int *xRes, int *yRes)
{
/* pass back values for a display */
	*xRes = display->xres;
	*yRes = display->yres;
	
	return GZ_SUCCESS;
}


int GzInitDisplay(GzDisplay	*display)
{
/* set everything to some default values - start a new frame */
	if (display == NULL)
	{
		return GZ_FAILURE;
	}
	
	for(int i=0; i<display->xres*display->yres; i++){
		display->fbuf[i].alpha = 0;
		display->fbuf[i].z = MAXINT;
		display->fbuf[i].red = 1000;
		display->fbuf[i].green = 2000;
		display->fbuf[i].blue = 2000;
	}
	
	return GZ_SUCCESS;
}


int GzPutDisplay(GzDisplay *display, int i, int j, GzIntensity r, GzIntensity g, GzIntensity b, GzIntensity a, GzDepth z)
{
/* write pixel values into the display */
	if(i>display->xres||i<0||j>display->yres||j<0)
		return GZ_FAILURE;

	if(r<=4095)
	display->fbuf[ARRAY(i,j)].red =r;
	else
	display->fbuf[ARRAY(i,j)].red =4095;

	if(g<=4095)
	display->fbuf[ARRAY(i,j)].green = g;
	else
	display->fbuf[ARRAY(i,j)].green =4095;

	if(b<=4095)
	display->fbuf[ARRAY(i,j)].blue = b;
	else
	display->fbuf[ARRAY(i,j)].blue =4095;

	display->fbuf[ARRAY(i,j)].alpha = a;

	if(0 < z < display->fbuf[ARRAY(i,j)].z )
	display->fbuf[ARRAY(i,j)].z = z;
	
	return GZ_SUCCESS;
}


int GzGetDisplay(GzDisplay *display, int i, int j, GzIntensity *r, GzIntensity *g, GzIntensity *b, GzIntensity *a, GzDepth *z)
{
	/* pass back pixel value in the display */
	if(i>display->xres||i<0||j>display->yres||j<0)
		return GZ_FAILURE;
	*r = display->fbuf[ARRAY(i,j)].red;
	*g = display->fbuf[ARRAY(i,j)].green;
	*b = display->fbuf[ARRAY(i,j)].blue;
	*a = display->fbuf[ARRAY(i,j)].alpha;
	*z = display->fbuf[ARRAY(i,j)].z;
		return GZ_SUCCESS;
}


int GzFlushDisplay2File(FILE* outfile, GzDisplay *display)
{

	/* write pixels to ppm file -- "P6 %d %d 255\r" */
	  fprintf(outfile, "P6 %d %d 255\r", display->xres, display->yres);
        for(int i=0; i<(display->xres*display->yres); i++){
                        fprintf(outfile,"%c", scale12to8(display->fbuf[i].red));
                        fprintf(outfile,"%c", scale12to8(display->fbuf[i].green));
                        fprintf(outfile,"%c", scale12to8(display->fbuf[i].blue));
        }

	return GZ_SUCCESS;
}

int GzFlushDisplay2FrameBuffer(char* framebuffer, GzDisplay *display)
{

	/* write pixels to framebuffer: 
		- Put the pixels into the frame buffer
		- Caution: store the pixel to the frame buffer as the order of blue, green, and red 
		- Not red, green, and blue !!!
	*/
	 int i;
        for(i=0; i<(display->xres*display->yres); i++){
                        framebuffer[3*i] = scale12to8(display->fbuf[i].blue );
                        framebuffer[3*i+1] = scale12to8(display->fbuf[i].green );
                        framebuffer[3*i+2] = scale12to8(display->fbuf[i].red );
                }
	return GZ_SUCCESS;
}