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
// REDO: recode skybox
// gl_sky.c all sky related stuff
//
#include "quakedef.h"

int		solidskytexture;
int		alphaskytexture;
float	speedscale;		// for top sky and bottom sky


char	skyname[256];
char	*suf[6] = {"rt", "bk", "lf", "ft", "up", "dn"};
int		skytexture[6];


// Tei sky customs
extern cvar_t sv_skyvalue;
extern cvar_t sv_skydim;
extern cvar_t sv_skyspeedscale;
extern cvar_t sv_skyflattern;
// Tei sky customs

// Tei autofx
void R_ParticleSnow (vec3_t origin);
void R_ParticleRain (vec3_t origin);
extern	cvar_t	r_autosnow;
extern	cvar_t	r_autorain;
extern	cvar_t	r_autolightday;
void R_ParticleDay (vec3_t origin);
// Tei autofx

void DefineFlare(vec3_t origin, int radius, int mode, int alfa) ;

extern sfx_t *cl_zing_sfx;//Tei rain zing
extern sfx_t *cl_zing2_sfx;//Tei rain zing
extern sfx_t *cl_zing3_sfx;//Tei rain zing

/*
=============
EmitSkyPolys
=============
*/

extern cvar_t sv_stepsize; //Tei
extern cvar_t r_autozing;
void R_SuperZing (vec3_t start, vec3_t end);

void EmitSkyPolys (msurface_t *s)
{
	glpoly_t	*p;
	float		*v;
	int			i;
	float		r, t;
	vec3_t		dir, hit;
	float		length;
	qboolean	dummy;


	int frain, fsnow, flight; //Tei wheater fx


	// Tei fix for skybox
	//No draw if skybox
	if (skyname[0])
		dummy	= true;
	else
		dummy	= false;
	// Tei fix for skybox


	if(!dummy)
		glBindTexture (GL_TEXTURE_2D, solidskytexture);

	// Tei slow sky
	//speedscale = realtime*8;
	speedscale = realtime * sv_skyvalue.value * 0.1;//1
	// Tei slow sky
	

	// Tei wheater particle fx

	// Force fx with name   skysnow --> force snow

	frain	= s->texinfo->texture->frain;
	fsnow	= s->texinfo->texture->fsnow;
	flight	= s->texinfo->texture->flight;

	// Tei wheater particle fx


	speedscale -= (int)speedscale & ~127;

	for (p=s->polys ; p ; p=p->next)
	{
		if(!dummy)
			glBegin (GL_POLYGON);

		for (i=0,v=p->verts[0] ; i<p->numverts ; i++, v+=VERTEXSIZE)
		{
			VectorSubtract (v, r_origin, dir);
			//Tei More flat sky
			dir[2] *= sv_skydim.value;//32;	// flatten the sphere //tei2 16
			//Tei More flat sky
			
			length = 378/Length (dir);	// Tomaz - Speed

			dir[0] *= length;
			dir[1] *= length;

			r = (speedscale + dir[0]) * (0.0078125);	// Tomaz - Speed
			t = (speedscale + dir[1]) * (0.0078125);	// Tomaz - Speed

			// Tei autofx
			if (r_autorain.value || frain)
			{
				//if (rand()&1) // fixme
				//if ( rand() & 4095) 
				//if (	rand()%32000 < (32000/8) ) // RAND_MAX?
				if (	teirand(100)<(9 + r_autorain.value) )
				{
				
					R_ParticleRain (v);

				}
			}

			if (r_autozing.value)
						if (	teirand(2200)<r_autozing.value )
						if (	teirand(2200)*100<r_autozing.value )
						{
							/* Generating a random Zing */

							VectorCopy(v,hit);
							//DefineFlare();
							hit[2] -= 5;

							DefineFlare(hit, 600, 0, 90);//FLASH!

							hit[2] -= 1000;
							hit[0] += lhrandom(-100,100);
							hit[1] += lhrandom(-100,100);
							R_SuperZing(v,hit);//Ray!
							hit[0] += lhrandom(-100,100);
							hit[1] += lhrandom(-100,100);
							R_SuperZing(v,hit);//Ray!
							hit[0] += lhrandom(-100,100);
							hit[1] += lhrandom(-100,100);
							R_SuperZing(v,hit);//Ray!
							if (cl_zing_sfx && cl_zing2_sfx && cl_zing3_sfx)
							{								
								VectorCopy(v,hit);
								hit[2] -= 128;
								if (rand()&1)
									S_StartSound (-1, 0, cl_zing_sfx,hit , 1, 1);
								if (rand()&1)
									S_StartSound (-1, 1, cl_zing2_sfx,hit , 1, 1);
								//if (rand()&1)
								S_StartSound (-1, 2, cl_zing3_sfx,hit , 1, 1);
							}

							/* with 3 the effect is realistic */
						}


			if (r_autosnow.value || fsnow)
			{
				///if (rand()&127>62) // fixme
				//if (rand() & 4095 )
				if (	teirand(100)<(1+r_autosnow.value) )
				//if (	rand()%32000 < (32000/256) ) // RAND_MAX?
					R_ParticleSnow (v);

			}
			if (r_autolightday.value || flight) 
			{
				//if (rand()&1)
				//if (rand()&231>230) 
				//if (	rand()%32000 < (32000/64) ) 
				if (	teirand(100)<(2+r_autolightday.value) )
				        R_ParticleDay (v);
			}
			/*
			else
			if (r_autolightday.value == 2)
			{
				VectorCopy(v, k);
				k[2] -= 3 + rand()&1;
				if (rand()&1)
				if (rand()&231>230) 
						R_ParticleDream (k);
				        //R_ParticleDay (v);

			}*/

			// Tei autofx
			if(!dummy){

				glTexCoord2f (r, t);
				glVertex3fv (v);
			}
		}
		if(!dummy)
			glEnd ();
	}

	if(dummy)
		return;//Tei nodraw while skybox
	
	glBindTexture (GL_TEXTURE_2D, alphaskytexture);
	//  Tei slow sky
	//speedscale = realtime*16;
	speedscale = realtime*sv_skyspeedscale.value *  0.1;//2
	// Tei slow sky

	speedscale -= (int)speedscale & ~127;

	for (p=s->polys ; p ; p=p->next)
	{
		glBegin (GL_POLYGON);
		for (i=0,v=p->verts[0] ; i<p->numverts ; i++, v+=VERTEXSIZE)
		{
			VectorSubtract (v, r_origin, dir);
		
			//dir[2] *= 3;	// flatten the sphere
			dir[2] *= sv_skyflattern.value;//16;	// tei2

			length = 378/Length (dir);	// Tomaz - Speed

			dir[0] *= length;
			dir[1] *= length;

			r = (speedscale + dir[0]) * (0.0078125);	// Tomaz - Speed
			t = (speedscale + dir[1]) * (0.0078125);	// Tomaz - Speed

			glTexCoord2f (r, t);
			glVertex3fv (v);

		}
		glEnd ();
	}
}


/*
==================
R_LoadSkys
==================
*/
void R_LoadSkys (void)
{
	int		i;
	char	name[64];

	for (i=0 ; i<6 ; i++)
	{
		sprintf (name, "gfx/env/%s%s", skyname, suf[i]);
		skytexture[i] = loadtextureimage(name, false ,true);
		if (skytexture[i] == 0)
		{
			sprintf (name, "gfx/env/tomazsky%s", suf[i]);
			skytexture[i] = loadtextureimage(name, true ,true);
		}
	}
}
void R_SetSkyBox (char *sky)
{
	strcpy(skyname, sky);
	R_LoadSkys ();
}

void LoadSky_f (void)
{
	switch (Cmd_Argc())
	{
	case 1:
		if (skyname[0])
			Con_Printf("Current sky: %s\n", skyname);
		else
			Con_Printf("Error: No skybox has been set\n");
		break;
	case 2:
		R_SetSkyBox(Cmd_Argv(1));
		break;
	default:
		Con_Printf("Usage: loadsky skyname\n");
		break;
	}
}
// Tomaz - Skybox End

/*
=================
R_DrawSky
=================
*/
void R_DrawSky (msurface_t *s)
{
	for ( ; s ; s=s->texturechain)
		EmitSkyPolys (s);
}


/*
==============
R_DrawSkyBox
==============
*/
int	skytexorder[6] = {0,2,1,3,4,5};

#define R_SkyBoxPolyVec(s,t,x,y,z) \
	glTexCoord2f(s, t);\
	glVertex3f((x) + r_refdef.vieworg[0], (y) + r_refdef.vieworg[1], (z) + r_refdef.vieworg[2]);

void R_DrawSkyBox (void)
{
	glBindTexture(GL_TEXTURE_2D, skytexture[skytexorder[3]]); // front
	glBegin(GL_QUADS);
	R_SkyBoxPolyVec(0.998047f, 0.001953f,  3072, -3072,  3072);
	R_SkyBoxPolyVec(0.998047f, 0.998047f,  3072, -3072, -3072);
	R_SkyBoxPolyVec(0.001953f, 0.998047f,  3072,  3072, -3072);
	R_SkyBoxPolyVec(0.001953f, 0.001953f,  3072,  3072,  3072);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, skytexture[skytexorder[2]]); // back
	glBegin(GL_QUADS);
	R_SkyBoxPolyVec(0.998047f, 0.001953f, -3072,  3072,  3072);
	R_SkyBoxPolyVec(0.998047f, 0.998047f, -3072,  3072, -3072);
	R_SkyBoxPolyVec(0.001953f, 0.998047f, -3072, -3072, -3072);
	R_SkyBoxPolyVec(0.001953f, 0.001953f, -3072, -3072,  3072);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, skytexture[skytexorder[0]]); // right
	glBegin(GL_QUADS);
	R_SkyBoxPolyVec(0.998047f, 0.001953f,  3072,  3072,  3072);
	R_SkyBoxPolyVec(0.998047f, 0.998047f,  3072,  3072, -3072);
	R_SkyBoxPolyVec(0.001953f, 0.998047f, -3072,  3072, -3072);
	R_SkyBoxPolyVec(0.001953f, 0.001953f, -3072,  3072,  3072);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, skytexture[skytexorder[1]]); // left
	glBegin(GL_QUADS);
	R_SkyBoxPolyVec(0.998047f, 0.001953f, -3072, -3072,  3072);
	R_SkyBoxPolyVec(0.998047f, 0.998047f, -3072, -3072, -3072);
	R_SkyBoxPolyVec(0.001953f, 0.998047f,  3072, -3072, -3072);
	R_SkyBoxPolyVec(0.001953f, 0.001953f,  3072, -3072,  3072);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, skytexture[skytexorder[4]]); // up
	glBegin(GL_QUADS);
	R_SkyBoxPolyVec(0.998047f, 0.001953f,  3072, -3072,  3072);
	R_SkyBoxPolyVec(0.998047f, 0.998047f,  3072,  3072,  3072);
	R_SkyBoxPolyVec(0.001953f, 0.998047f, -3072,  3072,  3072);
	R_SkyBoxPolyVec(0.001953f, 0.001953f, -3072, -3072,  3072);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, skytexture[skytexorder[5]]); // down
	glBegin(GL_QUADS);
	R_SkyBoxPolyVec(0.998047f, 0.001953f,  3072,  3072, -3072);
	R_SkyBoxPolyVec(0.998047f, 0.998047f,  3072, -3072, -3072);
	R_SkyBoxPolyVec(0.001953f, 0.998047f, -3072, -3072, -3072);
	R_SkyBoxPolyVec(0.001953f, 0.001953f, -3072,  3072, -3072);
	glEnd();
}



//===============================================================

/*
=============
R_InitSky

A sky texture is 256*128, with the right side being a masked overlay
==============
*/
void R_InitSky (byte *src, int bytesperpixel)
{
	int			i, j, p;
	unsigned	trans[128*128];
	unsigned	transpix;
	int			r, g, b;
	unsigned	*rgba;

//	if (isDedicated)
//		return;

	//	Con_Printf("heaven\n");//xfx
	
	if (bytesperpixel == 4)
	{
		for (i = 0;i < 128;i++)
			for (j = 0;j < 128;j++)
				trans[(i*128) + j] = src[i*256+j+128];
	}
	else
	{
		// make an average value for the back to avoid
		// a fringe on the top level
		r = g = b = 0;
		for (i=0 ; i<128 ; i++)
			for (j=0 ; j<128 ; j++)
			{
				p = src[i*256 + j + 128];
				rgba = &d_8to24table[p];
				trans[(i*128) + j] = *rgba;
				r += ((byte *)rgba)[0];
				g += ((byte *)rgba)[1];
				b += ((byte *)rgba)[2];
			}

		((byte *)&transpix)[0] = r/(128*128);
		((byte *)&transpix)[1] = g/(128*128);
		((byte *)&transpix)[2] = b/(128*128);
		((byte *)&transpix)[3] = 0;
	}

	if (!solidskytexture)
		solidskytexture = texture_extension_number++;

	glBindTexture (GL_TEXTURE_2D, solidskytexture );
	glTexImage2D (GL_TEXTURE_2D, 0, gl_solid_format, 128, 128, 0, GL_RGBA, GL_UNSIGNED_BYTE, trans);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


	if (bytesperpixel == 4)
	{
		for (i = 0;i < 128;i++)
			for (j = 0;j < 128;j++)
				trans[(i*128) + j] = src[i*256+j];
	}
	else
	{
		for (i=0 ; i<128 ; i++)
			for (j=0 ; j<128 ; j++)
			{
				p = src[i*256 + j];
				if (p == 0)
					trans[(i*128) + j] = transpix;
				else
					trans[(i*128) + j] = d_8to24table[p];
			}
	}

	if (!alphaskytexture)
		alphaskytexture = texture_extension_number++;

	glBindTexture (GL_TEXTURE_2D, alphaskytexture);
	glTexImage2D (GL_TEXTURE_2D, 0, gl_alpha_format, 128, 128, 0, GL_RGBA, GL_UNSIGNED_BYTE, trans);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}
