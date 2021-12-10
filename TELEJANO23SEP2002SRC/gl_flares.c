/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
//
// this file is specific to the crosshair, since we use several crosshairs
//
#include "quakedef.h"


float glowcos[17] = 
{
	 1.000000f,
	 0.923880f,
	 0.707105f,
	 0.382680f,
	 0.000000f,
	-0.382680f,
	-0.707105f,
	-0.923880f,
	-1.000000f,
	-0.923880f,
	-0.707105f,
	-0.382680f,
	 0.000000f,
	 0.382680f,
	 0.707105f,
	 0.923880f,
	 1.000000f
};


float glowsin[17] = 
{
	 0.000000f,
	 0.382680f,
	 0.707105f,
	 0.923880f,
	 1.000000f,
	 0.923880f,
	 0.707105f,
	 0.382680f,
	-0.000000f,
	-0.382680f,
	-0.707105f,
	-0.923880f,
	-1.000000f,
	-0.923880f,
	-0.707105f,
	-0.382680f,
	 0.000000f
};

//Tei flares DP style

void R_DrawCorona(vec3_t  pos, float radius , int visible , int alfa);//Tei
float TraceLine (vec3_t start, vec3_t end, vec3_t impact, vec3_t normal);//
//void R_DrawCoronaColor(flares_t *flare);//vec3_t pos, float radius , int visible, int alfa, int red, int green, int blue);


#define MAX_FLARES 128 //Tei low limits


//Tei flares

typedef struct {
	int					active;
	vec3_t				origin;
	int					radius;
	int					mode;
	int					alfa;
	byte					colorred;
	byte					colorgreen;
	byte					colorblue;
//	qboolean			iscolor;
	int					texnum;//flare tex
} flares_t;

//Tei flares

extern int particle_tex;
extern int flareglow_tex;
extern int flare_tex;
extern int circle_tex;
extern int bolt_tex;

flares_t flares[MAX_FLARES];


#define FLARE_NORMAL  0
#define FLARE_NOSOLID 1


void Clear_Flares(void) {
	memset (flares, 0, sizeof(flares));
}


extern cvar_t developer;

static vec3_t vdx = {25,25,25 };
extern cvar_t gl_dither;  // Tei force dither

extern int focus_tex;


//void R_DrawCoronaColor(vec3_t pos, float radius , int visible, int alfa, int red, int green, int blue)
void R_DrawCoronaColor(flares_t * flare)//vec3_t pos, float radius , int visible, int alfa, int red, int green, int blue)
{
	float viewdist;
	int visible;
//	int	texnum;
	
	vec3_t		up				= { vup[0] * 0.75f, vup[1] * 0.75f, vup[2] * 0.75f };
	vec3_t		right			= { vright[0] * 0.75f, vright[1] * 0.75f, vright[2] * 0.75f };
	vec3_t		coord[4];

	visible = (flare->mode == 666);//I dont remenber the value that make this correct.

	viewdist = DotProduct(r_origin, vpn);
	
	VectorAdd( up, right, coord[0] );
	VectorSubtract( right, up, coord[1] );
	VectorNegate( coord[0], coord[2] );
	VectorNegate( coord[1], coord[3] );

	//texnum		= flareglow_tex;//particle_tex;//lightcorona;//circle_tex;//flare_tex;//
	//texnum		= circle_tex;//lightcorona;//circle_tex;//flare_tex;//

	if (gl_fogenable.value)
	{
		glDisable(GL_FOG);
	}	

//	if (gl_dither.value)
		glEnable(GL_DITHER);//Tei dither

	if (!visible)
		glDisable(GL_DEPTH_TEST);

	glBindTexture( GL_TEXTURE_2D, flare->texnum );
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
		
	//glDepthMask(false);

	//glBindTexture( GL_TEXTURE_2D, flare->texnum );		//extra?
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	
	//glColor4ub( (GLubyte) flare->colorred, (GLubyte)flare->colorgreen,(GLubyte) flare->colorblue,(GLubyte) flare->alfa);
	glColor4ub( flare->colorred, flare->colorgreen, flare->colorblue, (byte)flare->alfa);
	//glColor4f( flare->colorred, flare->colorgreen, flare->colorblue, flare->alfa);//Tei warning fix
	//glColor4ub( 0, 0, 0, alfa);

	
	//FH!
	//glDisable (GL_TEXTURE_2D);
	//glShadeModel (GL_SMOOTH);
	//glEnable (GL_BLEND);
	//glBlendFunc (GL_ONE, GL_ONE);
	//FH!


	glPushMatrix();

	glTranslatef( flare->origin[0] , flare->origin[1], flare->origin[2]   );	

	//radius = radius * 0.1 * viewdist * 0.01f ;

	glScalef( flare->radius , flare->radius, flare->radius);//Tei xfx bug

	glBegin( GL_QUADS );
	{				
		glTexCoord2f(	0,		1	);
		glVertex3fv(	coord[0]	);
			
		glTexCoord2f(	0,		0	);
		glVertex3fv(	coord[1]	);
						
		glTexCoord2f(	1,		0	); 
		glVertex3fv(	coord[2]	);
						
		glTexCoord2f(	1,		1	);
		glVertex3fv(	coord[3]	);
						
		glEnd();
	}
	glPopMatrix();
			

	//FH!
	//glDisable (GL_BLEND);
	//glEnable (GL_TEXTURE_2D);
	//glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//FH!

	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	
	//glDepthMask(true);
	
	glColor4f(1,1,1,1);

//	if (gl_dither.value)
//		glDisable(GL_DITHER);//Tei dither

	if( !visible )
		glEnable(GL_DEPTH_TEST);

//	Con_Printf("hello\n");
	
	if (gl_fogenable.value)
	{
		glEnable(GL_FOG);
	}

}

float TTraceLine (vec3_t start, vec3_t end, vec3_t impact, vec3_t normal);

void DrawFlares (void) {
	vec3_t	stop,normal;//, start ;
	flares_t * flare;
	int i;


	for (i=1;i<MAX_FLARES;i++) {
		flare = &flares[i];
		if ( flare->active ){
			if (flare->mode != 1)
				flare->active = 0;

			switch( flare->mode) 
				{
				case 2:
					//R_DrawCoronaColor(flare->origin, flare->radius  , 0, flare->alfa, flare->colorred,flare->colorgreen, flare->colorblue);
					R_DrawCoronaColor(flare);//flare->origin, flare->radius  , 0, flare->alfa, flare->colorred,flare->colorgreen, flare->colorblue);
					//R_DrawCoronaColorTexnum(flare->origin, flare->radius  , 0, flare->alfa, flare->colorred,flare->colorgreen, flare->colorblue, flareglow_tex);
					break;

				case 3:
					//R_DrawCoronaColor(flare->origin, flare->radius  , 0, flare->alfa, flare->colorred,flare->colorgreen, flare->colorblue);
					//R_DrawCoronaColorTexnum(flare);
					if(TTraceLine (r_refdef.vieworg, flare->origin, stop, normal)==1) {
						//R_DrawCoronaColor(flare->origin, flare->radius  , 0, flare->alfa, flare->colorred,flare->colorgreen, flare->colorblue);
						R_DrawCoronaColor(flare);//flare->origin, flare->radius  , 0, flare->alfa, flare->colorred,flare->colorgreen, flare->colorblue);
					}
					break;

				default:
				if(TTraceLine (r_refdef.vieworg, flare->origin, stop, normal)==1) {
					//R_DrawCoronaColor(flare->origin, flare->radius  , 0, flare->alfa, flare->colorred,flare->colorgreen, flare->colorblue);
					R_DrawCoronaColor(flare);//flare->origin, flare->radius  , 0, flare->alfa, flare->colorred,flare->colorgreen, flare->colorblue);
					//R_DrawCoronaColorTexnum(flare->origin, flare->radius  , 0, flare->alfa, flare->colorred,flare->colorgreen, flare->colorblue, flare->texnum);					
				}
				break;
			}
		}
	}

}

/*
void DrawFlares (void) {
	vec3_t	stop,normal;
	flares_t * flare;
	int i;

	for (i=1;i<MAX_FLARES;i++) {
		flare = &flares[i];
		if ( flares[i].active ){
			if (flares[i].mode == 0)
				flares[i].active = 0;
			else
			i
			if(TraceLine (r_refdef.vieworg, flares[i].origin, stop, normal)==1) {
				//R_DrawCorona(flares[i].origin, flares[i].radius * 0.8 , 0);
				R_DrawCorona(flares[i].origin, flares[i].radius  , 0, flares[i].alfa);
			}
		}
	}

}
*/

int FreeFlare(void){
	int i;
	for( i=MAX_FLARES-1;i>0;i--)
		if ( !(flares[i].active) )
			return i;

	return 0;
}


void DefineFlareColorTexnum(vec3_t origin, int radius, int mode, int alfa, float red, float green, float blue, int texnum) 
{
	int i;
	i = FreeFlare();
	if (!i) return;

	flares[i].active = true;
	flares[i].radius = radius;
	flares[i].mode   = mode;
	flares[i].alfa   = alfa;
	flares[i].colorred		= red;
	flares[i].colorgreen	= green;
	flares[i].colorblue		= blue;
	flares[i].texnum		= texnum;
	//flares[i].iscolor = true;
	VectorCopy( origin, flares[i].origin );
}

extern int focus_tex;

void DefineFlareColor(vec3_t origin, int radius, int mode, int alfa, float red, float green, float blue) 
{
	int i;
	i = FreeFlare();
	if (!i) return;

	flares[i].active = true;
	flares[i].radius = radius;
	flares[i].mode   = mode;
	flares[i].alfa   = alfa;
	flares[i].colorred		= red;
	flares[i].colorgreen	= green;
	flares[i].colorblue		= blue;
	flares[i].texnum		= flareglow_tex;//flare_tex;
	//flares[i].iscolor = true;
	VectorCopy( origin, flares[i].origin );
}



void DefineFlare(vec3_t origin, int radius, int mode, int alfa) 
{
	DefineFlareColor(origin, radius,mode,alfa,255,255,255);
}




//Tei flares DP style






void R_DrawGlows (entity_t *e)
{
	vec3_t	lightorigin;	// Origin of torch.
	vec3_t	v;				// Vector to torch.
	float	radius;			// Radius of torch flare.
	float	distance;		// Vector distance to torch.
	float	intensity;		// Intensity of torch flare.
	int			i;
	model_t		*clmodel;

	if (gl_fogenable.value)
	{
		glDisable(GL_FOG);
	}	

	clmodel = e->model;

	// NOTE: this isn't centered on the model
	VectorCopy(e->origin, lightorigin);	

	// set radius based on what model we are doing here
	radius = clmodel->glow_radius;

	VectorSubtract(lightorigin, r_origin, v);

	// See if view is outside the light.
	distance = Length(v);
	if (distance > radius)
	{
		glDepthMask (false);
		glDisable (GL_TEXTURE_2D);
		glBlendFunc (GL_ONE, GL_ONE);

		glBegin(GL_TRIANGLE_FAN);

		if (clmodel->effect == MFX_GLOW_)
		{	
			lightorigin[2] += 4.0f;
			intensity = (1 - ((1024.0f - distance) * 0.0009765625)) * 0.75;
			glColor3f(1.0f*intensity, 0.7f*intensity, 0.4f*intensity);		
		}
		else
		{
			glColor3fv (clmodel->glow_color);				
		}

		VectorScale (vpn, -radius, v);
		VectorAdd (v, lightorigin, v);

		glVertex3fv(v);
		glColor3f(0.0f, 0.0f, 0.0f);
		for (i=16; i>=0; i--) 
		{
			v[0] = lightorigin[0] + radius * (vright[0] * glowcos[i] + vup[0] * glowsin[i]);
			v[1] = lightorigin[1] + radius * (vright[1] * glowcos[i] + vup[1] * glowsin[i]);
			v[2] = lightorigin[2] + radius * (vright[2] * glowcos[i] + vup[2] * glowsin[i]);
			glVertex3fv(v);
		}
		glEnd();
		glColor3f (1,1,1);
		glEnable (GL_TEXTURE_2D);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthMask (true);
	}
	if (gl_fogenable.value)
	{
		glEnable(GL_FOG);
	}
}
