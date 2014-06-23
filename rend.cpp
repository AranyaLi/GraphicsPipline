/* CS580 Homework 3 */

#include	"stdafx.h"
#include	"stdio.h"
#include	"math.h"
#include	"Gz.h"
#include	"rend.h"
#include    "float.h"


//#define COLORED 1
//#define NOCOLOR 0
#define REGULAR 0
#define IRREGULAR 1
#define TWOLEFT 2
#define TWORIGHT 3
#define X 0
#define Y 1
#define Z 2
#define MAXVALUE 9999999999999.999
#define PI 3.141592653589793

//float xl = MAXVALUE, xr = 0, yl = MAXVALUE, yr = 0;

short	ctoi(float color);
void computer_shade_equation(GzRender *render, GzCoord normal, GzColor color);
float into_perspective(float affineP,float screenZ);
float back_affine( float screenP,float screenZ);

typedef struct{
float x, y, z;
}vector;
typedef struct{
	GzCoord v;
	GzCoord n;
	GzTextureIndex uv;
}vertex;
//
float ftoradian( float a){
	return PI * a / 180 ;
}

typedef struct{
	GzCoord start;
	GzCoord end;
	GzCoord current;
	float slope_x;
	float slope_z;
	//int flag;
}edgeDDA;

typedef struct{
	GzCoord start;
	GzCoord end;
	GzCoord current;
	float slope_z;
}spanDDA;

typedef struct{
	edgeDDA el;
	edgeDDA er;
	edgeDDA e3;
	int flag;
}triangle;

typedef struct{
	float A, B, C;
}line_coefficient;

typedef struct{
	float A, B, C, D;
}plane_coefficient;

plane_coefficient interpolate_w(float x1, float y1, float w1, float x2, float y2, float w2, float x3, float y3, float w3){
	//float A, B, C, D, w;
	plane_coefficient p;
	p.A = y1 * (w2 - w3) + y2 * (w3 - w1) + y3 * (w1 - w2);
	p.B = w1 * (x2 - x3) + w2 * (x3 - x1) + w3 * (x1 - x2);
	p.C = x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2);
	p.D = -(x1 * (y2 * w3 - y3 * w2) + x2 * (y3 * w1 - y1 * w3) + x3 * (y1 * w2 - y2 * w1));
	//w = - (A*i + B*j + D) / C;
	return p;
}

void Sortvert(GzCoord *a )// by j, increase order
{
	int i, j, k;
	for(k=0;k<2;k++){
		for(i=0;i<2-k;i++){
			if(a[i][1]>a[i+1][1]){
				float temp[3][3];
				for(j=0;j<3;j++){
					temp[i][j]=a[i][j];
					a[i][j]=a[i+1][j];
					a[i+1][j]=temp[i][j];
				}
			}
		}
	}	
}

void swapvertex(vertex *a, vertex *b){
	vertex *temp = new vertex();
	(*temp).v[X] = (*a).v[X];
	(*temp).v[Y] = (*a).v[Y];
	(*temp).v[Z] = (*a).v[Z];
	(*temp).n[X] = (*a).n[X];
	(*temp).n[Y] = (*a).n[Y];
	(*temp).n[Z] = (*a).n[Z];
	(*temp).uv[U] = (*a).uv[U];
	(*temp).uv[V] = (*a).uv[V];

	(*a).v[X] = (*b).v[X];
	(*a).v[Y] = (*b).v[Y];
	(*a).v[Z] = (*b).v[Z];
	(*a).n[X] = (*b).n[X];
	(*a).n[Y] = (*b).n[Y];
	(*a).n[Z] = (*b).n[Z];
	(*a).uv[U] = (*b).uv[U];
	(*a).uv[V] = (*b).uv[V];
	
	(*b).v[X] = (*temp).v[X];
	(*b).v[Y] = (*temp).v[Y];
	(*b).v[Z] = (*temp).v[Z];
	(*b).n[X] = (*temp).n[X];
	(*b).n[Y] = (*temp).n[Y];
	(*b).n[Z] = (*temp).n[Z];
	(*b).uv[U] = (*temp).uv[U];
	(*b).uv[V] = (*temp).uv[V];

}
void sortvertby_y(vertex *vn1, vertex *vn2, vertex *vn3 ){
	if((*vn1).v[Y] >= (*vn2).v[Y])
		swapvertex(vn1, vn2);

	if((*vn1).v[Y] >= (*vn3).v[Y])
		swapvertex(vn1, vn3);

	if((*vn2).v[Y] >= (*vn3).v[Y])
		swapvertex(vn2, vn3);

}

edgeDDA initedgeDDA(GzCoord start,GzCoord end){
	GzCoord  *nstart = new GzCoord[3];
	GzCoord	 *nend = new GzCoord[3];
	GzCoord	 *nc = new GzCoord[3];
	(*nstart)[0]=start[0];
	(*nstart)[1]=start[1];
	(*nstart)[2]=start[2];
	(*nend)[0]=end[0];
	(*nend)[1]=end[1];
	(*nend)[2]=end[2];
	(*nc)[0]=start[0];
	(*nc)[1]=start[1];
	(*nc)[2]=start[2];

	edgeDDA E;
		E.start[0] = (*nstart)[0];
		E.start[1] = (*nstart)[1];
		E.start[2] = (*nstart)[2];
		E.current[0] = (*nstart)[0];
		E.current[1] = (*nstart)[1];
		E.current[2] = (*nstart)[2];
		E.end[0] = (*nend)[0];
		E.end[1] = (*nend)[1];
		E.end[2] = (*nend)[2];
	
	if(end[Y]!=start[Y]){
		E.slope_x = (end[X]-start[X])/(end[Y]-start[Y]);
		E.slope_z = (end[Z]-start[Z])/(end[Y]-start[Y]);
	}
	else{
		E.slope_x = MAXVALUE;
		E.slope_z = MAXVALUE;
	}
	//time(&timer2);
	return E;
	
}

spanDDA initspanDDA(GzCoord *L,GzCoord *R){
	spanDDA E;
	E.start[0] = (*L)[0];
	E.start[1] = (*L)[1];
	E.start[2] = (*L)[2];
	E.end[0] = (*R)[0];
	E.end[1] = (*R)[1];
	E.end[2] = (*R)[2];
	E.current[0] = (*L)[0];
	E.current[1] = (*L)[1];
	E.current[2] = (*L)[2];
	if(*L[X]!=(*R)[X])
		E.slope_z = ((*R)[Z]-(*L)[Z])/((*R)[X]-(*L)[X]);
	else
		E.slope_z = MAXVALUE;
	return E;

}

line_coefficient calcu_line_equation(GzCoord start, GzCoord end ){
	line_coefficient p;
	p.A= end[Y] - start[Y];
	p.B= start[X] - end[X];
	p.C= (end[X] * start[Y]) - (start[X]*end[Y]);
	return p;
}

triangle settriangle(GzCoord start, GzCoord end, GzCoord mid, line_coefficient p){

	triangle T;
	float x_e = (-(p.B*(mid[Y]))-p.C)/p.A;

	if(fabs(mid[Y]-start[Y]) < 1e-9){
		
		 T.e3 = initedgeDDA(start, mid);
		 T.el = initedgeDDA(mid, end);
		 T.er = initedgeDDA(start, end);
		 T.flag = IRREGULAR;
	}
	if(fabs(mid[Y] - end[Y]) < 1e-9){
		T.er = initedgeDDA(start, mid);
		T.e3 = initedgeDDA(mid, end);
		T.el = initedgeDDA(start, end);
		T.flag = REGULAR;
	}
	if(x_e < mid[X]){// need modify
		T.er = initedgeDDA(start, mid);
		T.e3 = initedgeDDA(mid, end);
		T.el = initedgeDDA(start, end);
		T.flag = TWORIGHT;
	}
	if(x_e > mid[X]){
		T.el = initedgeDDA(start, mid);
		T.e3 = initedgeDDA(mid, end);
		T.er = initedgeDDA(start, end);	
		T.flag = TWOLEFT;
	}
	/*if(T.flag == REGULAR || T.flag == IRREGULAR)
		int i =1;*/

	return T;
}

void scanline(edgeDDA  el, edgeDDA  er,  edgeDDA  e3, GzRender *render, int flag, vertex vn1, vertex vn2, vertex vn3){
int i, j;
GzIntensity r =0, g =0, b = 0, a = 0;
int FBZ = MAXVALUE;
spanDDA span;
GzColor clor, uv_color;
//GzCoord normalresult;
//GzColor texture_clor1, texture_clor2, texture_clor3;
GzTextureIndex uv_result;
			//uv 
			vn1.uv[U] = into_perspective(vn1.uv[U], vn1.v[Z] );	
			vn1.uv[V] = into_perspective(vn1.uv[V], vn1.v[Z] );
			vn2.uv[U] = into_perspective(vn2.uv[U], vn2.v[Z] );	
			vn2.uv[V] = into_perspective(vn2.uv[V], vn2.v[Z] );
			vn3.uv[U] = into_perspective(vn3.uv[U], vn3.v[Z] );	
			vn3.uv[V] = into_perspective(vn3.uv[V], vn3.v[Z] );
			plane_coefficient pu, pv;
			pu = interpolate_w(vn1.v[X], vn1.v[Y], vn1.uv[U], vn2.v[X], vn2.v[Y], vn2.uv[U],vn3.v[X], vn3.v[Y], vn3.uv[U]);
			pv = interpolate_w(vn1.v[X], vn1.v[Y], vn1.uv[V], vn2.v[X], vn2.v[Y], vn2.uv[V],vn3.v[X], vn3.v[Y], vn3.uv[V]);

			GzColor clor1, clor2, clor3;
			plane_coefficient pred, pgreen, pblue;
			GzCoord normalresult;
			plane_coefficient pnx, pny, pnz;

			if(render->interp_mode == GZ_COLOR){
			//Gouraud
						
						computer_shade_equation(render, vn1.n, clor1);
						computer_shade_equation(render, vn2.n, clor2);
						computer_shade_equation(render, vn3.n, clor3);
	
						pred = interpolate_w(vn1.v[X], vn1.v[Y], clor1[RED], vn2.v[X], vn2.v[Y], clor2[RED],vn3.v[X], vn3.v[Y], clor3[RED]);
						pgreen = interpolate_w(vn1.v[X], vn1.v[Y], clor1[GREEN], vn2.v[X], vn2.v[Y], clor2[GREEN],vn3.v[X], vn3.v[Y], clor3[GREEN]);
						pblue = interpolate_w(vn1.v[X], vn1.v[Y], clor1[BLUE], vn2.v[X], vn2.v[Y], clor2[BLUE],vn3.v[X], vn3.v[Y], clor3[BLUE]);
			}
			if(render->interp_mode == GZ_NORMALS){
			
			//phong
						pnx = interpolate_w(vn1.v[X], vn1.v[Y], vn1.n[X], vn2.v[X], vn2.v[Y], vn2.n[X],vn3.v[X], vn3.v[Y], vn3.n[X]);
						pny = interpolate_w(vn1.v[X], vn1.v[Y], vn1.n[Y], vn2.v[X], vn2.v[Y], vn2.n[Y],vn3.v[X], vn3.v[Y], vn3.n[Y]);
						pnz = interpolate_w(vn1.v[X], vn1.v[Y], vn1.n[Z], vn2.v[X], vn2.v[Y], vn2.n[Z],vn3.v[X], vn3.v[Y], vn3.n[Z]);
						
			}
//----------------begin scanline-----------
	float delta_y = ceilf((el.start)[Y])-(el.start)[Y];
	float delta_x = 0;

	((el.current))[X] = ((el.start))[X] + (el.slope_x)*delta_y;
	((el.current))[Y] = ((el.start))[Y] + delta_y;
	((el.current))[Z] = ((el.start))[Z] + (el.slope_z)*delta_y;//advance current for el
	((er.current))[X] = ((er.start))[X] + (er.slope_x)*delta_y;
	((er.current))[Y] = ((er.start))[Y] + delta_y;
	((er.current))[Z] = ((er.start))[Z] + (er.slope_z)*delta_y;//advance current for er

	// tri with two left edges marked el, e3, one right edges marked er

	if(flag == TWOLEFT ){
		while( (el.current)[Y] <= (el.end)[Y] ){	
			span = initspanDDA(&el.current, &er.current);
 			delta_x = ceilf((el.current)[X]) - (el.current)[X];
			(span.current)[X] =  (el.current)[X] + delta_x;
			span.current[Y] = el.current[Y];
			(span.current)[Z] = (el.current)[Z] + span.slope_z * delta_x;

			while((span.current)[X] <= (er.current)[X]){
				i =(int) (span.current)[X];
				j = (int)(span.current)[Y] ;

				/*if(i==166 && j==89)
					int bugg =1;*/

				if(render->interp_mode == GZ_COLOR){
							clor[RED] = (-pred.A * i - pred.B * j - pred.D) /( pred.C);
							clor[GREEN] =(-pgreen.A * i - pgreen.B * j - pgreen.D) /( pgreen.C);
							clor[BLUE] =(-pblue.A * i - pblue.B * j - pblue.D) /( pblue.C);
							if(render->tex_fun!= NULL){
							uv_result[U] = (-pu.A * i - pu.B * j - pu.D) /( pu.C);
							uv_result[V] = (-pv.A * i - pv.B * j - pv.D) /( pv.C);
							uv_result[U]= back_affine(uv_result[U], span.current[Z]);
							uv_result[V]= back_affine(uv_result[V], span.current[Z]);
							render->tex_fun(uv_result[U], uv_result[V], uv_color);
							clor[RED]= uv_color[RED]*clor[RED];
							clor[GREEN]= uv_color[GREEN]*clor[GREEN];
							clor[BLUE]= uv_color[BLUE]*clor[BLUE];
							}
						}
				if(render->interp_mode == GZ_NORMALS){
							normalresult[X] = (-pnx.A * i - pnx.B * j - pnx.D) /( pnx.C);
							normalresult[Y] = (-pny.A * i - pny.B * j - pny.D) /( pny.C);
							normalresult[Z] = (-pnz.A * i - pnz.B * j - pnz.D) /( pnz.C);
							uv_result[U] = (-pu.A * i - pu.B * j - pu.D) /( pu.C);
							uv_result[V] = (-pv.A * i - pv.B * j - pv.D) /( pv.C);
							uv_result[U]= back_affine(uv_result[U], span.current[Z]);
							uv_result[V]= back_affine(uv_result[V], span.current[Z]);
							if(render->tex_fun!= NULL){
							render->tex_fun(uv_result[U], uv_result[V], uv_color);
							memcpy(render->Kd, uv_color, sizeof(GzColor));
							memcpy(render->Ka, uv_color, sizeof(GzColor));
							computer_shade_equation(render,normalresult, clor);
							}
							else{
							computer_shade_equation(render,normalresult, clor);
							}
						}

				if((span.current)[Z] < 0){
					continue;
				}

				GzGetDisplay(render->display, i, j, &r, &g, &b, &a, &FBZ );

				if((span.current)[Z] < FBZ){
				GzPutDisplay(render->display, i, j, ctoi(clor[0]),ctoi(clor[1]),  ctoi(clor[2]), 0, (int)(span.current)[Z]);
				}
				((span.current)[X])++;
				((span.current))[Z] = ((span.current))[Z] + span.slope_z;
			}

			(((el.current))[Y])++;
			(((el.current))[X]) += el.slope_x;
			(((el.current))[Z]) += el.slope_z;
			(((er.current))[X]) += er.slope_x;
			(((er.current))[Y])++;
			(((er.current))[Z]) += er.slope_z;
		}

		delta_y = ceilf(((e3.start))[Y])-((e3.start))[Y];
		((e3.current))[X] = ((e3.start))[X] + e3.slope_x*delta_y;
		((e3.current))[Y] = ((e3.start))[Y] + delta_y;
		((e3.current))[Z] = ((e3.start))[Z] + e3.slope_z*delta_y;//advance current for e3
		int bug=0;
		while(((e3.current))[Y] <=((e3.end))[Y]){
			bug++;
			span = initspanDDA(&(e3.current), &(er.current));
			delta_x = ceilf(((e3.current))[X]) - ((e3.current))[X];
			((span.current))[X] =  ((e3.current))[X] + delta_x;
			span.current[Y] = e3.current[Y];
			((span.current))[Z] = ((e3.current))[Z] + span.slope_z * delta_x;

			while(((span.current))[X] <=((er.current))[X]){
				i =(int) ((span.current))[X];
				j = (int)((e3.current))[Y] ;

				if(render->interp_mode == GZ_COLOR){
							clor[RED] = (-pred.A * i - pred.B * j - pred.D) /( pred.C);
							clor[GREEN] =(-pgreen.A * i - pgreen.B * j - pgreen.D) /( pgreen.C);
							clor[BLUE] =(-pblue.A * i - pblue.B * j - pblue.D) /( pblue.C);
							if(render->tex_fun!= NULL){
							uv_result[U] = (-pu.A * i - pu.B * j - pu.D) /( pu.C);
							uv_result[V] = (-pv.A * i - pv.B * j - pv.D) /( pv.C);
							uv_result[U]= back_affine(uv_result[U], span.current[Z]);
							uv_result[V]= back_affine(uv_result[V], span.current[Z]);
							render->tex_fun(uv_result[U], uv_result[V], uv_color);
							clor[RED]= uv_color[RED]*clor[RED];
							clor[GREEN]= uv_color[GREEN]*clor[GREEN];
							clor[BLUE]= uv_color[BLUE]*clor[BLUE];
							}
						}
				if(render->interp_mode == GZ_NORMALS){
							normalresult[X] = (-pnx.A * i - pnx.B * j - pnx.D) /( pnx.C);
							normalresult[Y] = (-pny.A * i - pny.B * j - pny.D) /( pny.C);
							normalresult[Z] = (-pnz.A * i - pnz.B * j - pnz.D) /( pnz.C);
							uv_result[U] = (-pu.A * i - pu.B * j - pu.D) /( pu.C);
							uv_result[V] = (-pv.A * i - pv.B * j - pv.D) /( pv.C);
							uv_result[U]= back_affine(uv_result[U], span.current[Z]);
							uv_result[V]= back_affine(uv_result[V], span.current[Z]);
							if(render->tex_fun!= NULL){
							render->tex_fun(uv_result[U], uv_result[V], uv_color);
							memcpy(render->Kd, uv_color, sizeof(GzColor));
							memcpy(render->Ka, uv_color, sizeof(GzColor));
							computer_shade_equation(render,normalresult, clor);
							}
							else{
							computer_shade_equation(render,normalresult, clor);
							}
						}

				if(((span.current))[Z] < 0){
					continue;
				}

				GzGetDisplay(render->display, i, j, &r, &g, &b, &a, &FBZ );
				if(((span.current)[Z]) < FBZ){
				GzPutDisplay(render->display, i, j, ctoi(clor[0]),ctoi(clor[1]),  ctoi(clor[2]), 0, (int)(span.current)[Z]);
				}
				(((span.current))[X])++;
				((span.current))[Z] = ((span.current))[Z] + span.slope_z;
	}


			(((e3.current))[Y])++;
			(((e3.current))[X]) += e3.slope_x;
			(((e3.current))[Z]) += e3.slope_z;
			(((er.current))[X]) += er.slope_x;
			(((er.current))[Y])++;
			(((er.current))[Z]) += er.slope_z;
		}

	}//end TWOLEFT


	//begin TWORIGHT
	// tri with two right edges marked er, e3, one left edges marked el
	if(flag == TWORIGHT ){
		while(((er.current))[Y] <= ((er.end))[Y]){	
			span = initspanDDA(&(el.current), &er.current);
 			float delta_x = ceilf(((el.current))[X]) - ((el.current))[X];
			((span.current))[X] =  ((el.current))[X] + delta_x;
			span.current[Y] = el.current[Y];
			((span.current))[Z] = ((el.current))[Z] + span.slope_z * delta_x;

			while(((span.current))[X] <= ((er.current))[X]){
				i =(int) ((span.current))[X];
				j = (int)((er.current))[Y] ;
				

				if(render->interp_mode == GZ_COLOR){
							clor[RED] = (-pred.A * i - pred.B * j - pred.D) /( pred.C);
							clor[GREEN] =(-pgreen.A * i - pgreen.B * j - pgreen.D) /( pgreen.C);
							clor[BLUE] =(-pblue.A * i - pblue.B * j - pblue.D) /( pblue.C);
							if(render->tex_fun!= NULL){
							uv_result[U] = (-pu.A * i - pu.B * j - pu.D) /( pu.C);
							uv_result[V] = (-pv.A * i - pv.B * j - pv.D) /( pv.C);
							uv_result[U]= back_affine(uv_result[U], span.current[Z]);
							uv_result[V]= back_affine(uv_result[V], span.current[Z]);
							render->tex_fun(uv_result[U], uv_result[V], uv_color);
							clor[RED]= uv_color[RED]*clor[RED];
							clor[GREEN]= uv_color[GREEN]*clor[GREEN];
							clor[BLUE]= uv_color[BLUE]*clor[BLUE];
							}
							
						}
				if(render->interp_mode == GZ_NORMALS){
							normalresult[X] = (-pnx.A * i - pnx.B * j - pnx.D) /( pnx.C);
							normalresult[Y] = (-pny.A * i - pny.B * j - pny.D) /( pny.C);
							normalresult[Z] = (-pnz.A * i - pnz.B * j - pnz.D) /( pnz.C);
							uv_result[U] = (-pu.A * i - pu.B * j - pu.D) /( pu.C);
							uv_result[V] = (-pv.A * i - pv.B * j - pv.D) /( pv.C);
							uv_result[U]= back_affine(uv_result[U], span.current[Z]);
							uv_result[V]= back_affine(uv_result[V], span.current[Z]);
							if(render->tex_fun!= NULL){
							render->tex_fun(uv_result[U], uv_result[V], uv_color);
							memcpy(render->Kd, uv_color, sizeof(GzColor));
							memcpy(render->Ka, uv_color, sizeof(GzColor));
							computer_shade_equation(render,normalresult, clor);
							}
							else{
							computer_shade_equation(render,normalresult, clor);
							}
						}

				if(((span.current))[Z] < 0){
					continue;
				}

				GzGetDisplay(render->display, i, j, &r, &g, &b, &a, &FBZ );
				if(((span.current)[Z]) < FBZ){
				GzPutDisplay(render->display, i, j, ctoi(clor[0]),ctoi(clor[1]),  ctoi(clor[2]), 0, (int)(span.current)[Z]);
				}
				(((span.current))[X])++;
				((span.current))[Z] = ((span.current))[Z] + span.slope_z;
			}

			(((el.current))[Y])++;
			(((el.current))[X]) += el.slope_x;
			(((el.current))[Z]) += el.slope_z;
			(((er.current))[X]) += er.slope_x;
			(((er.current))[Y])++;
			(((er.current))[Z]) += er.slope_z;
		}

		delta_y = ceilf(((e3.start))[Y])-((e3.start))[Y];
		((e3.current))[X] = ((e3.start))[X] + e3.slope_x*delta_y;
		((e3.current))[Y] = ((e3.start))[Y] + delta_y;
		((e3.current))[Z] = ((e3.start))[Z] + e3.slope_z*delta_y;//advance current for e3

		while(((e3.current))[Y] <=((e3.end))[Y]){
			span = initspanDDA(&(el.current), &(e3.current));
			delta_x = ceilf(((el.current))[X]) - ((el.current))[X];
			((span.current))[X] =  ((el.current))[X] + delta_x;
			span.current[Y] = el.current[Y];
			((span.current))[Z] = ((el.current))[Z] + span.slope_z * delta_x;

			while(((span.current))[X] <= ((e3.current))[X]){
				i =(int) ((span.current))[X];
				j = (int)((e3.current))[Y] ;

				if(render->interp_mode == GZ_COLOR){
							clor[RED] = (-pred.A * i - pred.B * j - pred.D) /( pred.C);
							clor[GREEN] =(-pgreen.A * i - pgreen.B * j - pgreen.D) /( pgreen.C);
							clor[BLUE] =(-pblue.A * i - pblue.B * j - pblue.D) /( pblue.C);
							if(render->tex_fun!= NULL){
							uv_result[U] = (-pu.A * i - pu.B * j - pu.D) /( pu.C);
							uv_result[V] = (-pv.A * i - pv.B * j - pv.D) /( pv.C);
							uv_result[U]= back_affine(uv_result[U], span.current[Z]);
							uv_result[V]= back_affine(uv_result[V], span.current[Z]);
							render->tex_fun(uv_result[U], uv_result[V], uv_color);
							clor[RED]= uv_color[RED]*clor[RED];
							clor[GREEN]= uv_color[GREEN]*clor[GREEN];
							clor[BLUE]= uv_color[BLUE]*clor[BLUE];
							}
						}
				if(render->interp_mode == GZ_NORMALS){
							normalresult[X] = (-pnx.A * i - pnx.B * j - pnx.D) /( pnx.C);
							normalresult[Y] = (-pny.A * i - pny.B * j - pny.D) /( pny.C);
							normalresult[Z] = (-pnz.A * i - pnz.B * j - pnz.D) /( pnz.C);
							uv_result[U] = (-pu.A * i - pu.B * j - pu.D) /( pu.C);
							uv_result[V] = (-pv.A * i - pv.B * j - pv.D) /( pv.C);
							uv_result[U]= back_affine(uv_result[U], span.current[Z]);
							uv_result[V]= back_affine(uv_result[V], span.current[Z]);
							if(render->tex_fun!= NULL){
							render->tex_fun(uv_result[U], uv_result[V], uv_color);
							memcpy(render->Kd, uv_color, sizeof(GzColor));
							memcpy(render->Ka, uv_color, sizeof(GzColor));
							computer_shade_equation(render,normalresult, clor);
							}
							else{
							computer_shade_equation(render,normalresult, clor);
							}
						}

				if(((span.current))[Z] < 0){
					continue;
				}

				GzGetDisplay(render->display, i, j, &r, &g, &b, &a, &FBZ );
				if(((span.current)[Z]) < FBZ){
				GzPutDisplay(render->display, i, j, ctoi(clor[0]),ctoi(clor[1]),  ctoi(clor[2]), 0, (int)(span.current)[Z]);
				}
				(((span.current))[X])++;
				((span.current))[Z] = ((span.current))[Z] + span.slope_z;
	}


			(((e3.current))[Y])++;
			(((e3.current))[X]) += e3.slope_x;
			(((e3.current))[Z]) += e3.slope_z;
			(((el.current))[X]) += el.slope_x;
			(((el.current))[Y])++;
			(((el.current))[Z]) += el.slope_z;
		}

	}// end TWORIGHT
	

	//TBB or TTB
	//tri with bottom/top, marked e3, one left el, one right er
	if(flag == IRREGULAR || flag == REGULAR){
		while(((el.current))[Y] <= ((el.end))[Y]){	
			span = initspanDDA(&(el.current), &er.current);
 			float delta_x = ceilf(((el.current))[X]) - ((el.current))[X];
			((span.current))[X] =  ((el.current))[X] + delta_x;
			span.current[Y] = el.current[Y];
			((span.current))[Z] = ((el.current))[Z] + span.slope_z * delta_x;

			while(((span.current))[X] <= ((er.current))[X]){
				i =(int) ((span.current))[X];
				j = (int)((el.current))[Y] ;


				if(render->interp_mode == GZ_COLOR){
							clor[RED] = (-pred.A * i - pred.B * j - pred.D) /( pred.C);
							clor[GREEN] =(-pgreen.A * i - pgreen.B * j - pgreen.D) /( pgreen.C);
							clor[BLUE] =(-pblue.A * i - pblue.B * j - pblue.D) /( pblue.C);
							if(render->tex_fun!= NULL){
							uv_result[U] = (-pu.A * i - pu.B * j - pu.D) /( pu.C);
							uv_result[V] = (-pv.A * i - pv.B * j - pv.D) /( pv.C);
							uv_result[U]= back_affine(uv_result[U], span.current[Z]);
							uv_result[V]= back_affine(uv_result[V], span.current[Z]);
							render->tex_fun(uv_result[U], uv_result[V], uv_color);
							clor[RED]= uv_color[RED]*clor[RED];
							clor[GREEN]= uv_color[GREEN]*clor[GREEN];
							clor[BLUE]= uv_color[BLUE]*clor[BLUE];
							}
						}
				if(render->interp_mode == GZ_NORMALS){
							normalresult[X] = (-pnx.A * i - pnx.B * j - pnx.D) /( pnx.C);
							normalresult[Y] = (-pny.A * i - pny.B * j - pny.D) /( pny.C);
							normalresult[Z] = (-pnz.A * i - pnz.B * j - pnz.D) /( pnz.C);
							uv_result[U] = (-pu.A * i - pu.B * j - pu.D) /( pu.C);
							uv_result[V] = (-pv.A * i - pv.B * j - pv.D) /( pv.C);
							uv_result[U]= back_affine(uv_result[U], span.current[Z]);
							uv_result[V]= back_affine(uv_result[V], span.current[Z]);
							if(render->tex_fun!= NULL){
							render->tex_fun(uv_result[U], uv_result[V], uv_color);
							memcpy(render->Kd, uv_color, sizeof(GzColor));
							memcpy(render->Ka, uv_color, sizeof(GzColor));
							computer_shade_equation(render,normalresult, clor);
							}
							else{
							computer_shade_equation(render,normalresult, clor);
							}
						}

				if(((span.current))[Z] < 0){
					continue;
				}

				GzGetDisplay(render->display, i, j, &r, &g, &b, &a, &FBZ );
				if(((span.current)[Z]) < FBZ){
				GzPutDisplay(render->display, i, j, ctoi(clor[0]),ctoi(clor[1]),  ctoi(clor[2]), 0, (int)(span.current)[Z]);
				}
				(((span.current))[X])++;
				((span.current))[Z] = ((span.current))[Z] + span.slope_z;
			}

			(((el.current))[Y])++;
			(((el.current))[X]) += el.slope_x;
			(((el.current))[Z]) += el.slope_z;
			(((er.current))[X]) += er.slope_x;
			(((er.current))[Y])++;
			(((er.current))[Z]) += er.slope_z;
		}
	}

}	

//------------------------ END SCANLINE， HW2---------------------------
vector unit_vector(vector tail, vector head){
	vector vector;
	float mo = sqrt(pow(head.x - tail.x, 2) + pow(head.y - tail.y, 2) + pow(head.z - tail.z, 2));
	vector.x = (head.x - tail.x)/ mo;
	vector.y = (head.y - tail.y)/ mo;
	vector.z = (head.z - tail.z)/ mo;
	return vector;
}

vector cross_product(vector A, vector B){

    vector vector;
    vector.x = A.y*B.z - B.y*A.z;
    vector.y = -A.x*B.z + B.x*A.z;
    vector.z = A.x*B.y - A.y*B.x;
    return vector;
}

float dot_product(vector A, vector B){
	float t;
	t= A.x * B.x + A.y * B.y + A.z * B.z;
	return t;
}

void matrix_multiply(GzMatrix left, GzMatrix right, GzMatrix result){
	GzMatrix temp;
	memset(temp, 0, sizeof(GzMatrix));
		for(int i=0; i<4; i++)
			for(int j=0; j<4; j++){
				for(int k=0; k<4; k++){
				temp[i][j] += left[i][k] * right[k][j];
				}
			}
			memcpy(result, temp, sizeof(GzMatrix));
}

void do_xform(GzMatrix stack, GzCoord *v){
	vector res;
	float temp[4]={0, 0, 0, 0};
	float vertice1[4];
	vertice1[0] = (*v)[0];
	vertice1[1] = (*v)[1];
	vertice1[2] = (*v)[2];
	vertice1[3] = 1;

	for(int i=0; i<3; i++){
		temp[0] += stack[0][i] * vertice1[i];
	}
	temp[0]+=stack[0][3];

	for(int i=0; i<3; i++){
		temp[1] += stack[1][i] * vertice1[i];
	}
	temp[1]+=stack[1][3];

	for(int i=0; i<3; i++){
		temp[2] += stack[2][i] * vertice1[i];
	}
	temp[2]+=stack[2][3];

	for(int i=0; i<3; i++){
		temp[3] += stack[3][i]* vertice1[i];
	}
	temp[3]+=stack[3][3];

	if(temp[3]!=0){
		res.x= temp[0]/temp[3];
		res.y = temp[1]/temp[3];
		res.z = temp[2]/temp[3];
	}
	else{
		res.x = temp[0];
		res.y = temp[1];
		res.z = temp[2];
	}
	(*v)[X] = res.x;
	(*v)[Y] = res.y;
	(*v)[Z] = res.z;

	
}

int GzRotXMat(float degree, GzMatrix mat)
{
// Create rotate matrix : rotate along x axis
// Pass back the matrix using mat value
	mat[0][0] = 1;
	mat[0][1] = 0;
	mat[0][2] = 0;
	mat[0][3] = 0;

	mat[1][0] = 0;
	mat[1][1] = cosf(ftoradian(degree));
	mat[1][2] = -sinf(ftoradian(degree));
	mat[1][3] = 0;

	mat[2][0] = 0;
	mat[2][1] = sinf(ftoradian(degree));
	mat[2][2] = cosf(ftoradian(degree));
	mat[2][3] = 0;

	mat[3][0] = 0;
	mat[3][1] = 0;
	mat[3][2] = 0;
	mat[3][3] = 1;
	

	return GZ_SUCCESS;
}


int GzRotYMat(float degree, GzMatrix mat)
{
	mat[0][0] = cosf(ftoradian(degree));
	mat[0][1] = 0;
	mat[0][2] = sinf(ftoradian(degree));
	mat[0][3] = 0;

	mat[1][0] = 0;
	mat[1][1] = 1;
	mat[1][2] = 0;
	mat[1][3] = 0;

	mat[2][0] = -sinf(ftoradian(degree));
	mat[2][1] = 0;
	mat[2][2] = cosf(ftoradian(degree));
	mat[2][3] = 0;

	mat[3][0] = 0;
	mat[3][1] = 0;
	mat[3][2] = 0;
	mat[3][3] = 1;
// Create rotate matrix : rotate along y axis
// Pass back the matrix using mat value

	return GZ_SUCCESS;
}


int GzRotZMat(float degree, GzMatrix mat)
{
// Create rotate matrix : rotate along z axis
// Pass back the matrix using mat value
	mat[0][0] = cosf(ftoradian(degree));
	mat[0][1] = -sinf(ftoradian(degree));
	mat[0][2] = 0;
	mat[0][3] = 0;

	mat[1][0] = sinf(ftoradian(degree));
	mat[1][1] = cosf(ftoradian(degree));
	mat[1][2] = 0;
	mat[1][3] = 0;

	mat[2][0] = 0;
	mat[2][1] = 0;
	mat[2][2] = 1;
	mat[2][3] = 0;

	mat[3][0] = 0;
	mat[3][1] = 0;
	mat[3][2] = 0;
	mat[3][3] = 1;

	return GZ_SUCCESS;
}


int GzTrxMat(GzCoord translate, GzMatrix mat)
{
// Create translation matrix
// Pass back the matrix using mat value
	mat[0][0] = 1;
	mat[0][1] = 0;
	mat[0][2] = 0;
	mat[0][3] = translate[X];

	mat[1][0] = 0;
	mat[1][1] = 1;
	mat[1][2] = 0;
	mat[1][3] = translate[Y];

	mat[2][0] = 0;
	mat[2][1] = 0;
	mat[2][2] = 1;
	mat[2][3] = translate[Z];

	mat[3][0] = 0;
	mat[3][1] = 0;
	mat[3][2] = 0;
	mat[3][3] = 1;

	return GZ_SUCCESS;
}


int GzScaleMat(GzCoord scale, GzMatrix mat)
{
// Create scaling matrix
// Pass back the matrix using mat value
	mat[0][0] = scale[X];
	mat[0][1] = 0;
	mat[0][2] = 0;
	mat[0][3] = 0;

	mat[1][0] = 0;
	mat[1][1] = scale[Y];
	mat[1][2] = 0;
	mat[1][3] = 0;

	mat[2][0] = 0;
	mat[2][1] = 0;
	mat[2][2] = scale[Z];
	mat[2][3] = 0;

	mat[3][0] = 0;
	mat[3][1] = 0;
	mat[3][2] = 0;
	mat[3][3] = 1;

	return GZ_SUCCESS;
}
//--------------------------------END HW3---------------------------
vector normalize( vector unnormal){
	vector normalized;
	float mo = sqrt( pow(unnormal.x, 2) + pow(unnormal.y, 2) + pow(unnormal.z, 2));
	normalized.x = unnormal.x / mo;
	normalized.y = unnormal.y / mo;
	normalized.z = unnormal.z / mo;
	return normalized;
}

void normalize( GzCoord v){
	//vector normalized;
	float mo = sqrt( pow(v[X], 2) + pow(v[Y], 2) + pow(v[Z], 2));
	v[X] = v[X] / mo;
	v[Y] = v[Y] / mo;
	v[Z] = v[Z] / mo;
	//return normalized;
}
void computer_shade_equation(GzRender *render, GzCoord normal, GzColor color){
	vector Rd[MAX_LIGHTS], Ld[MAX_LIGHTS],Rc[MAX_LIGHTS], Lc[MAX_LIGHTS], N, E, clookat, corigin, sum_ks = {0}, sum_kd = {0};
	float RE, NL, NE;

	for(int i=0; i< render->numlights; i++){
		Ld[i].x = render->lights[i].direction[X]; //direction
		Ld[i].y = render->lights[i].direction[Y];
		Ld[i].z = render->lights[i].direction[Z];
		Lc[i].x = render->lights[i].color[RED];  //color
		Lc[i].y = render->lights[i].color[GREEN];
		Lc[i].z = render->lights[i].color[BLUE];
	}//L
	N.x = normal[X];
	N.y = normal[Y];
	N.z = normal[Z];
	//N
	/*corigin.x = render->camera.position[X];
	corigin.y = render->camera.position[Y];
	corigin.z = render->camera.position[Z];

	clookat.x = render->camera.lookat[X];
	clookat.y = render->camera.lookat[Y];
	clookat.z = render->camera.lookat[Z];
	E = normalize2(corigin, clookat);*/
	E.x = 0;
	E.y = 0;
	E.z = -1;
	//E

 for(int i=0; i< render->numlights; i++){
		NE = dot_product(N, E);
		NL = dot_product(N, Ld[i]);
	

		if(NL * NE <0){
			continue;
		}
		else if(NL < 0 && NE < 0){
			N.x = - N.x;
			N.y = -N.y;
			N.z = -N.z;
			NL = - NL;
		}
	
		Rd[i].x = 2* NL * N.x - Ld[i].x;
		Rd[i].y = 2* NL * N.y - Ld[i].y;
		Rd[i].z = 2* NL * N.z - Ld[i].z;
		Rd[i] = normalize(Rd[i]);
		RE =  dot_product(Rd[i], E);
		if(RE < 0){
			RE = 0;
		}
		else if(RE > 1){
		RE =1;
		}
		RE = pow(RE, render->spec);
		sum_ks.x += Lc[i].x * RE;
		sum_ks.y += Lc[i].y * RE;
		sum_ks.z += Lc[i].z * RE;

		sum_kd.x += Lc[i].x * NL;
		sum_kd.y += Lc[i].y * NL;
		sum_kd.z += Lc[i].z * NL;
	}
		if(render->interp_mode ==GZ_COLOR && render->tex_fun != NULL){
		color[RED] =  sum_ks.x +  sum_kd.x + render->ambientlight.color[RED];
		color[GREEN] =sum_ks.y +  sum_kd.y + render->ambientlight.color[GREEN];
		color[BLUE] = sum_ks.z +  sum_kd.z + render->ambientlight.color[BLUE];
	
		}
		else if(render->interp_mode ==GZ_NORMALS || render->tex_fun ==NULL){
		color[RED] = render->Ks[RED]* sum_ks.x + render->Kd[RED] * sum_kd.x + render->Ka[RED]* render->ambientlight.color[RED];
		color[GREEN] = render->Ks[GREEN]* sum_ks.y + render->Kd[GREEN] * sum_kd.y + render->Ka[GREEN]* render->ambientlight.color[GREEN];
		color[BLUE] = render->Ks[BLUE]* sum_ks.z + render->Kd[BLUE] * sum_kd.z + render->Ka[BLUE]* render->ambientlight.color[BLUE];
			
		}

	if(color[RED]<0)
		color[RED] = 0;
	if(color[RED] > 1)
		color[RED] = 1;

	if(color[GREEN]<0)
		color[GREEN] = 0;
	if(color[GREEN] > 1)
		color[GREEN] = 1;

	if(color[BLUE]<0)
		color[BLUE] = 0;
	if(color[BLUE] > 1)
		color[BLUE] = 1;

}

//----------------------------------------------------------
// Begin main functions

//--------------------------------END HW4--------------------------------

float into_perspective(float affineP,float screenZ){
	float screenP, termV;
	termV = screenZ/(MAXINT-screenZ);
	screenP = affineP/(termV + 1);
	return screenP;
}
float back_affine( float screenP,float screenZ){
	float affineP, termV;
	termV = screenZ/(MAXINT - screenZ);
	affineP = screenP*(termV + 1);
	return affineP;
}

int GzPutAttribute(GzRender	*render, int numAttributes, GzToken	*nameList,
				   GzPointer *valueList) /* void** valuelist */
{
	/*
	- set renderer attribute states (e.g.: GZ_RGB_COLOR default color)
	- later set shaders, interpolaters, texture maps, and lights
	*/
	GzColor *temp_color, *temp_ka, *temp_ks, *temp_kd;
	GzLight *temp_light[MAX_LIGHTS], *temp_ambiant;
	float *temp_spec,* temp_tx, * temp_ty;
	int *shadetype, *interpstyle;
	GzTexture *texture;

	for (int i=0; i<numAttributes;i++){
		switch(nameList[i]){
		case GZ_RGB_COLOR: {
			temp_color = (GzColor*)valueList[i];
			render->flatcolor[0] = (*temp_color)[0];
			render->flatcolor[1] = (*temp_color)[1];
			render->flatcolor[2] = (*temp_color)[2];
		}
		break;

		case GZ_DIRECTIONAL_LIGHT:{
			temp_light[i] = (GzLight*)valueList[i];
			render->lights[i].color[0] = (*temp_light[i]).color[0];
			render->lights[i].color[1] = (*temp_light[i]).color[1];
			render->lights[i].color[2] = (*temp_light[i]).color[2];
			render->lights[i].direction[0] = (*temp_light[i]).direction[0];
			render->lights[i].direction[1] = (*temp_light[i]).direction[1];
			render->lights[i].direction[2] = (*temp_light[i]).direction[2];
			render->numlights++;
		}
		break;

		case GZ_AMBIENT_LIGHT: {
			temp_ambiant = (GzLight*)valueList[i];			   
			render->ambientlight.color[0] = (*temp_ambiant).color[0];
			render->ambientlight.color[1] = (*temp_ambiant).color[1];
			render->ambientlight.color[2] = (*temp_ambiant).color[2];
			render->ambientlight.direction[0] = (*temp_ambiant).direction[0];
			render->ambientlight.direction[1] = (*temp_ambiant).direction[1];
			render->ambientlight.direction[2] = (*temp_ambiant).direction[2];
			}
		break;

		case GZ_DIFFUSE_COEFFICIENT:{
			temp_kd = (GzColor*)valueList[i];
			render->Kd[0] = (*temp_kd)[0];
			render->Kd[1] = (*temp_kd)[1];
			render->Kd[2] = (*temp_kd)[2];
			}
		break;

		case GZ_AMBIENT_COEFFICIENT:{
			temp_ka	= (GzColor*)valueList[i];
			render->Ka[0] = (*temp_ka)[0];
			render->Ka[1] = (*temp_ka)[1];
			render->Ka[2] = (*temp_ka)[2];
			}
		break;

		case GZ_SPECULAR_COEFFICIENT:{
			temp_ks	= (GzColor*)valueList[i];
			render->Ks[0] = (*temp_ks)[0];
			render->Ks[1] = (*temp_ks)[1];
			render->Ks[2] = (*temp_ks)[2];
									 }
		break;

		case GZ_DISTRIBUTION_COEFFICIENT:{
			temp_spec = (float*)valueList[i];
			render->spec = *temp_spec;
								 }
		break;

		case GZ_INTERPOLATE:{
			interpstyle = (int*)valueList[i];
			render->interp_mode = *interpstyle;		
		}
		break;

		case GZ_TEXTURE_MAP:{
			//texture = (GzTexture*)valueList[i];
			//render->tex_fun = 0;
			render->tex_fun = (GzTexture)valueList[i];
							}

		case GZ_SHEFT_X:{
			temp_tx = (float*)valueList[i];
			render->xsheft = *temp_tx;
						}
		case GZ_SHEFT_Y:{
			temp_ty = (float*)valueList[i];
			render->ysheft = *temp_ty;
						}

		}
	}
	return GZ_SUCCESS;
}

int GzPutTriangle(GzRender *render, int	numParts, GzToken *nameList,
				  GzPointer *valueList)
				  /* numParts - how many names and values */
{
	/*
	- pass in a triangle description with tokens and values corresponding to
	GZ_NULL_TOKEN:		do nothing - no values
	GZ_POSITION:		3 vert positions in model space
	- Invoke the scan converter and return an error code
- Xform positions of verts using matrix on top of stack
- Clip - just discard any triangle with any vert(s) behind view plane
       - optional: test for triangles with all three verts off-screen (trivial frustum cull)
- invoke triangle rasterizer
	*/
	GzCoord *tempv, *tempn;
	GzTextureIndex *tempuv;
	vertex vn1 = {0}, vn2= {0}, vn3={0};
	triangle t;
	line_coefficient p;
	//line_coefficient ll, lr, l3;
	

	for (int i=0; i<numParts; i++){
		switch(nameList[i]){
		case GZ_NULL_TOKEN: break;
		case GZ_POSITION:{
			tempv = (GzCoord*)(valueList[i]);
			vn1.v[X]=tempv[0][0];
			vn1.v[Y]=tempv[0][1];
			vn1.v[Z]=tempv[0][2];
			vn2.v[X]=tempv[1][0];
			vn2.v[Y]=tempv[1][1];
			vn2.v[Z]=tempv[1][2];
			vn3.v[X]=tempv[2][0];
			vn3.v[Y]=tempv[2][1];
			vn3.v[Z]=tempv[2][2];
			do_xform(render->Ximage[render->matlevel -1], &vn1.v);
			do_xform(render->Ximage[render->matlevel -1], &vn2.v);
			do_xform(render->Ximage[render->matlevel -1], &vn3.v);
			vn1.v[X] += render->xsheft;
			vn1.v[Y] += render->ysheft;
			vn2.v[X] +=  render->xsheft;
			vn2.v[Y] += render->ysheft;
			vn3.v[X] += render->xsheft;
			vn3.v[Y] += render->ysheft;
			break;
		}
			case GZ_NORMAL:{
			tempn = (GzCoord*)(valueList[i]);
			vn1.n[X]=tempn[0][0];
			vn1.n[Y]=tempn[0][1];
			vn1.n[Z]=tempn[0][2];
			vn2.n[X]=tempn[1][0];
			vn2.n[Y]=tempn[1][1];
			vn2.n[Z]=tempn[1][2];
			vn3.n[X]=tempn[2][0];
			vn3.n[Y]=tempn[2][1];
			vn3.n[Z]=tempn[2][2];
			do_xform(render->Xnorm[render->matlevel -1], &vn1.n);
			do_xform(render->Xnorm[render->matlevel -1], &vn2.n);
			do_xform(render->Xnorm[render->matlevel -1], &vn3.n);
			normalize(vn1.n);
			normalize(vn2.n);
			normalize(vn3.n);			
			break;
		}
			case GZ_TEXTURE_INDEX:{
				tempuv = (GzTextureIndex*)(valueList[i]);
				vn1.uv[U] = tempuv[0][U];
				vn1.uv[V] = tempuv[0][V];
				vn2.uv[U] = tempuv[1][U];
				vn2.uv[V] = tempuv[1][V];
				vn3.uv[U] = tempuv[2][U];
				vn3.uv[V] = tempuv[2][V];	  
				int stupid=1;
			}

		}
		}
			sortvertby_y(&vn1, &vn2, &vn3);
			
						
		p = calcu_line_equation(vn1.v, vn3.v);
		t = settriangle(vn1.v, vn3.v, vn2.v, p);
		
		if(render->interp_mode == GZ_COLOR){
		scanline(t.el, t.er, t.e3, render, t.flag, vn1, vn2, vn3);//gouraud
		}
		if(render->interp_mode == GZ_NORMALS){
		scanline(t.el, t.er, t.e3, render, t.flag, vn1, vn2, vn3);//phong
		}

	return GZ_SUCCESS;
}

int GzNewRender(GzRender **render, GzDisplay	*display)
{
/*
- malloc a renderer struct
- setup Xsp and anything only done once
- save the pointer to display
- init default camera
*/
	*render = new GzRender();
	(*render)->display = new GzDisplay();
	(*render)->display = display;

	for(int i = 0; i < 4 ; i++)
		for (int j = 0; j < 4; j++){
			(*render)->Xsp[i][j] = 0;
			(*render)->camera.Xiw[i][j] = 0;
			(*render)->camera.Xpi[i][j] = 0;
		}
		(*render)->camera.FOV = DEFAULT_FOV;
		(*render)->camera.lookat[X] = 0;
		(*render)->camera.lookat[Y] = 0;
		(*render)->camera.lookat[Z] = 0;
		(*render)->camera.position[X] = DEFAULT_IM_X;
		(*render)->camera.position[Y] = DEFAULT_IM_Y;
		(*render)->camera.position[Z] = DEFAULT_IM_Z;
		(*render)->camera.worldup[X] = 0;
		(*render)->camera.worldup[Y] = 1;
		(*render)->camera.worldup[Z] = 0;
		(*render)->matlevel = 0;

		(*render)->interp_mode= GZ_FLAT;
		(*render)->numlights = 0;
		GzColor ka = DEFAULT_AMBIENT, kd = DEFAULT_DIFFUSE, ks = DEFAULT_SPECULAR;
		(*render)->Ka[X] = ka[X];
		(*render)->Ka[Y] = ka[Y];
		(*render)->Ka[Z] = ka[Z];
		(*render)->Kd[X] = kd[X];
		(*render)->Kd[Y] = kd[Y];
		(*render)->Kd[Z] = kd[Z];
		(*render)->Ks[X] = ks[X];
		(*render)->Ks[Y] = ks[Y];
		(*render)->Ks[Z] = ks[Z];
		(*render)->spec = DEFAULT_SPEC;
		//(*render)->ambientlight = DEFAULT_AMBIENT;
		(*render)->xsheft = 0;
		(*render)->ysheft =0;

	return GZ_SUCCESS;

}


int GzFreeRender(GzRender *render)
{
/*
-free all renderer resources
*/
	delete(render);
	return GZ_SUCCESS;
}

int GzBeginRender(GzRender *render)
{
/*
- setup for start of each frame - init frame buffer color,alpha,z
- compute Xiw and projection xform Xpi from camera definition
- init Ximage - put Xsp at base of stack, push on Xpi and Xiw
- now stack contains Xsw and app can push model Xforms when needed
*/
	GzInitDisplay(render->display);
	//---------xpi------
	render->camera.Xpi[0][0] = 1;
	render->camera.Xpi[0][1] = 0;
	render->camera.Xpi[0][2] = 0;
	render->camera.Xpi[0][3] = 0;

	render->camera.Xpi[1][0] = 0;
	render->camera.Xpi[1][1] = 1;
	render->camera.Xpi[1][2] = 0;
	render->camera.Xpi[1][3] = 0;

	render->camera.Xpi[2][0] = 0;
	render->camera.Xpi[2][1] = 0;
	render->camera.Xpi[2][2] = 1;
	render->camera.Xpi[2][3] = 0;

	render->camera.Xpi[3][0] = 0;
	render->camera.Xpi[3][1] = 0;
	render->camera.Xpi[3][2] = tanf(ftoradian(render->camera.FOV/2));
	render->camera.Xpi[3][3] = 1;

	//-----------xiw-----------
	vector Z_axis, X_axis, Y_axis, up1, corigin, clookat, cworldup, origin={0, 0, 0};
	float upZ;
	corigin.x = render->camera.position[X];
	corigin.y = render->camera.position[Y];
	corigin.z = render->camera.position[Z];

	clookat.x = render->camera.lookat[X];
	clookat.y = render->camera.lookat[Y];
	clookat.z = render->camera.lookat[Z];

	cworldup.x = render->camera.worldup[X];
	cworldup.y = render->camera.worldup[Y];
	cworldup.z = render->camera.worldup[Z];
	
	Z_axis = unit_vector(corigin, clookat);

	upZ = dot_product(cworldup, Z_axis);
	up1.x = -(render->camera.worldup[X] - upZ * Z_axis.x);
	up1.y = -(render->camera.worldup[Y] - upZ * Z_axis.y);
	up1.z = -(render->camera.worldup[Z] - upZ * Z_axis.z);

	Y_axis = unit_vector(up1, origin);
	X_axis = cross_product(Y_axis, Z_axis);

	render->camera.Xiw[0][0] = X_axis.x;
	render->camera.Xiw[0][1] = X_axis.y;
	render->camera.Xiw[0][2] = X_axis.z;
	render->camera.Xiw[0][3] = -dot_product(X_axis, corigin);

	render->camera.Xiw[1][0] = Y_axis.x;
	render->camera.Xiw[1][1] = Y_axis.y;
	render->camera.Xiw[1][2] = Y_axis.z;
	render->camera.Xiw[1][3] = -dot_product(Y_axis, corigin);

	render->camera.Xiw[2][0] = Z_axis.x;
	render->camera.Xiw[2][1] = Z_axis.y;
	render->camera.Xiw[2][2] = Z_axis.z;
	render->camera.Xiw[2][3] = -dot_product(Z_axis, corigin);

	render->camera.Xiw[3][0] = 0;
	render->camera.Xiw[3][1] = 0;
	render->camera.Xiw[3][2] = 0;
	render->camera.Xiw[3][3] = 1;

//--------	init Ximage - put Xsp at base of stack, push on Xpi and Xiw------
	render->Xsp[0][0] = render->display->xres/2;
	render->Xsp[0][1] = 0;
	render->Xsp[0][2] = 0;
	render->Xsp[0][3] = render->display->xres/2;

	render->Xsp[1][0] = 0;
	render->Xsp[1][1] = -render->display->yres/2;
	render->Xsp[1][2] = 0;
	render->Xsp[1][3] = render->display->yres/2;

	render->Xsp[2][0] = 0;
	render->Xsp[2][1] = 0;
	render->Xsp[2][2] = MAXINT*tanf(ftoradian(render->camera.FOV/2));
	render->Xsp[2][3] = 0;

	render->Xsp[3][0] = 0;
	render->Xsp[3][1] = 0;
	render->Xsp[3][2] = 0;
	render->Xsp[3][3] = 1;

	GzPushMatrix(render, render->Xsp);
	GzPushMatrix(render, render->camera.Xpi);
	GzPushMatrix(render, render->camera.Xiw);

	return GZ_SUCCESS;
}

int GzPutCamera(GzRender *render, GzCamera *camera)
{
/*
- overwrite renderer camera structure with new camera definition
*/
	render->camera.FOV = camera->FOV;
	render->camera.lookat[X] = camera->lookat[X];
	render->camera.lookat[Y] = camera->lookat[Y];
	render->camera.lookat[Z] = camera->lookat[Z];

	render->camera.position[X] = camera->position[X];
	render->camera.position[Y] = camera->position[Y];
	render->camera.position[Z] = camera->position[Z];

	render->camera.worldup[X] = camera->worldup[X];
	render->camera.worldup[Y] = camera->worldup[Y];
	render->camera.worldup[Z] = camera->worldup[Z];
	
	return GZ_SUCCESS;
}

int GzPushMatrix(GzRender *render, GzMatrix	matrix)
{
/*
- push a matrix onto the Ximage stack
- check for stack overflow
*/
	GzMatrix result;
	memset(result, 0, sizeof(GzMatrix));

	if(render->matlevel >= MATLEVELS )
		return GZ_FAILURE;

	else if(render->matlevel ==0){
		memcpy(render->Ximage[render->matlevel], matrix, sizeof(GzMatrix));

		for(int i =0; i<4; i++)
			for(int j=0; j<4; j++){
				(render->Xnorm[render->matlevel])[i][j] = 0;
			}
			(render->Xnorm[render->matlevel])[0][0] = 1;
			(render->Xnorm[render->matlevel])[1][1] = 1;
			(render->Xnorm[render->matlevel])[2][2] = 1;
			(render->Xnorm[render->matlevel])[3][3] = 1;
	}

	else {
		//ximage
		matrix_multiply(render->Ximage[render->matlevel -1], matrix, render->Ximage[render->matlevel]);
		//xnorm
		if(render->matlevel == 1){
		for(int i =0; i<4; i++)
			for(int j=0; j<4; j++){
				(render->Xnorm[render->matlevel])[i][j] = 0;
			}
			(render->Xnorm[render->matlevel])[0][0] = 1;
			(render->Xnorm[render->matlevel])[1][1] = 1;
			(render->Xnorm[render->matlevel])[2][2] = 1;
			(render->Xnorm[render->matlevel])[3][3] = 1;
		}
		else{
		float k;
		GzMatrix temp = {0};
		k = 1 / sqrtf( pow(matrix[0][0], 2) + pow(matrix[0][1], 2) + pow(matrix[0][2], 2) );
		for(int i =0; i<4; i++){
			for(int j=0; j<4; j++){
			temp[i][j] = k * matrix[i][j];
			}
			temp[i][3]=0;
			temp[3][i]=0;
		}
		temp[3][3] =1;
		matrix_multiply(render->Xnorm[render->matlevel -1], temp, render->Xnorm[render->matlevel]);
		}
	}
	render->matlevel++;
	return GZ_SUCCESS;
}


int GzPopMatrix(GzRender *render)
{
/*
- pop a matrix off the Ximage stack
- check for stack underflow
*/
	if(render->matlevel ==0)
		return GZ_FAILURE;
	else
		render->matlevel--;
	return GZ_SUCCESS;
}



short	ctoi(float color)		/* convert float color to GzIntensity short */
{
  return(short)((int)(color * ((1 << 12) - 1)));
}

