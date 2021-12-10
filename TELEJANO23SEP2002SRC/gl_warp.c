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
#include "quakedef.h"



//
// gl_warp.c -- sky and water polygons
//

//
// External
//
extern	model_t	*loadmodel;
extern	cvar_t	gl_subdivide_size;
extern int causticstexture[31];

// Tei autofx
extern	cvar_t	r_autolava;
extern	cvar_t	r_autotele;
extern	cvar_t	r_autofogwater;
extern	cvar_t	r_autobubbleglobal;
extern	cvar_t	sv_stepsize;
extern	cvar_t	r_autolightday;//Tei

void R_ParticleFogLava (vec3_t origin); //Dark version
// Tei autofx


//
// Integer
//
int		shinytexture;
int		grasstexture;
int             detailtexture;//CHP detail textures
int             detailtexture2;//xfx geo detail texture


//
// msurface_t
//
msurface_t	*warpface;

//
// Float
//
float	turbsin[] =
{
	#include "gl_warp_sin.h"
};

//
// Define
// 
#define TURBSCALE (40.7436589469704879)


/*
================
BoundPoly
================
*/
void BoundPoly (int numverts, float *verts, vec3_t mins, vec3_t maxs)
{
	int		i, j;
	float	*v;

	mins[0] = mins[1] = mins[2] = 9999;
	maxs[0] = maxs[1] = maxs[2] = -9999;
	v = verts;
	for (i=0 ; i<numverts ; i++)
		for (j=0 ; j<3 ; j++, v++)
		{
			if (*v < mins[j])
				mins[j] = *v;
			if (*v > maxs[j])
				maxs[j] = *v;
		}
}

/*
================
SubdividePolygon
================
*/
void SubdividePolygon (int numverts, float *verts)
{
	int		i, j, k;
	vec3_t	mins, maxs;
	float	m;
	float	*v;
	vec3_t	front[64], back[64];
	int		f, b;
	float	dist[64];
	float	frac;
	glpoly_t	*poly;
	float	s, t;

	if (numverts > 60)
		Sys_Error ("numverts = %i", numverts);

	BoundPoly (numverts, verts, mins, maxs);

	for (i=0 ; i<3 ; i++)
	{
		m = (mins[i] + maxs[i]) * 0.5;
		m = gl_subdivide_size.value * floor (m/gl_subdivide_size.value + 0.5);
		if (maxs[i] - m < 8)
			continue;
		if (m - mins[i] < 8)
			continue;

		// cut it
		v = verts + i;
		for (j=0 ; j<numverts ; j++, v+= 3)
			dist[j] = *v - m;

		// wrap cases
		dist[j] = dist[0];
		v-=i;
		VectorCopy (verts, v);

		f = b = 0;
		v = verts;
		for (j=0 ; j<numverts ; j++, v+= 3)
		{
			if (dist[j] >= 0)
			{
				VectorCopy (v, front[f]);
				f++;
			}
			if (dist[j] <= 0)
			{
				VectorCopy (v, back[b]);
				b++;
			}
			if (dist[j] == 0 || dist[j+1] == 0)
				continue;
			if ( (dist[j] > 0) != (dist[j+1] > 0) )
			{
				// clip point
				frac = dist[j] / (dist[j] - dist[j+1]);
				for (k=0 ; k<3 ; k++)
					front[f][k] = back[b][k] = v[k] + frac*(v[3+k] - v[k]);
				f++;
				b++;
			}
		}

		SubdividePolygon (f, front[0]);
		SubdividePolygon (b, back[0]);
		return;
	}

	poly = Hunk_Alloc (sizeof(glpoly_t) + (numverts-4) * VERTEXSIZE*sizeof(float));
	poly->next = warpface->polys;
	warpface->polys = poly;
	poly->numverts = numverts;
	for (i=0 ; i<numverts ; i++, verts+= 3)
	{
		VectorCopy (verts, poly->verts[i]);
		s = DotProduct (verts, warpface->texinfo->vecs[0]);
		t = DotProduct (verts, warpface->texinfo->vecs[1]);
		poly->verts[i][3] = s;
		poly->verts[i][4] = t;
	}
}

/*
================
GL_SubdivideSurface

Breaks a polygon up along axial 64 unit
boundaries so that turbulent and sky warps
can be done reasonably.
================
*/
void GL_SubdivideSurface (msurface_t *s)
{
	vec3_t		verts[64];
	int			numverts;
	int			i;
	int			lindex;
	float		*vec;

	warpface = s;

	//
	// convert edges back to a normal polygon
	//
	numverts = 0;
	for (i=0 ; i<s->numedges ; i++)
	{
		lindex = loadmodel->surfedges[s->firstedge + i];

		if (lindex > 0)
			vec = loadmodel->vertexes[loadmodel->edges[lindex].v[0]].position;
		else
			vec = loadmodel->vertexes[loadmodel->edges[-lindex].v[1]].position;
		VectorCopy (vec, verts[numverts]);
		numverts++;
	}

	SubdividePolygon (numverts, verts[0]);
}

//=========================================================

/*			
Emit*****Polys - emits polygons with special effects
*/

/*
=============
EmitWaterPolys

Does a water warp on the pre-fragmented glpoly_t chain
=============
*/

// Tei surface fx

void LuxHere (vec3_t pos) 
{
	dlight_t	*dl;
	dl = CL_AllocDlight (0);
	if (!dl)
		return;
	VectorCopy (pos, dl->origin);
	dl->color[0]	= 1;
	dl->color[1]	= 0;
	dl->color[2]	= 1;
	dl->radius	= 300;
	dl->die	  	= cl.time + 0.5;
	dl->decay	= 300;
};







// Tei auto fx
void R_FireExploLava (vec3_t  origin, float scale);
void R_ParticleImplosionSutil (vec3_t origin);
void R_ParticleImplosion (vec3_t origin);
void R_ParticleFog (vec3_t origin);
void R_ParticleFogColor (vec3_t origin, int red, int green, int blue, int alphabase);

// Tei auto fx

//Tei surface fx
void R_ParticleBlast (vec3_t origin, vec3_t dir, int num,vec3_t color, byte scale);
void R_ParticleBlastTimed (vec3_t origin, vec3_t dir, int num,vec3_t color, byte scale, int time);


#define LUX_DAY			'l'
#define LUX_PASTELERO	'p'
#define LUX_RAIN		'r'
#define LUX_SNOW		's'
#define LUX_DUNGEON		'd'

void R_ParticleDay2 (vec3_t origin, int lux);
void R_ParticleRain (vec3_t origin);
void R_ParticleSnow (vec3_t origin);

void FxForLux (int fxvalue , vec3_t  org)
{
	vec3_t dir, color;
	int spd;

	switch ( fxvalue )
	{
	case LUX_DAY:
	    R_ParticleDay2 (org, 0);
		break;
	case LUX_PASTELERO:
		color[0] = 55;
		color[1] = 55;
		color[2] =255;
		spd = 8;
		dir[0]   = (rand() & (spd*2-1)) - spd; 
		dir[1]   = (rand() & (spd*2-1)) - spd; 
		dir[2]   = (rand() & (spd*2-1)) - spd; 
		spd = 32;
		org[0]   = org[0] + (rand() & (spd*2-1)) - spd; 
		org[1]   = org[1] +(rand() & (spd*2-1)) - spd; 
		org[2]   = org[2] +(rand() & (spd*2-1)) - spd; 

		spd = rand()&7;

		R_ParticleBlastTimed (org, dir,1,color,16,spd);
			
		break;
	case LUX_DUNGEON:
		if (	rand()%32000 > (32000/512) ) 
			break;
	case LUX_RAIN:
		spd = 16;
		org[0]   = org[0] + (rand() & (spd*2-1)) - spd; 
		org[1]   = org[1] +(rand() & (spd*2-1)) - spd; 
		org[2]   = org[2] +(rand() & (spd*2-1)) - spd; 

		R_ParticleRain (org);
		break;
	case LUX_SNOW:
		spd = 16;
		org[0]   = org[0] + (rand() & (spd*2-1)) - spd; 
		org[1]   = org[1] +(rand() & (spd*2-1)) - spd; 
		org[2]   = org[2] +(rand() & (spd*2-1)) - spd; 

		R_ParticleSnow (org);
		break;

	default:

		break;
	}	
};


void EmitXFX (msurface_t *s);
void R_ParticleImplosionHaze (vec3_t origin);
void R_ParticleBubble (vec3_t origin);
extern cvar_t mod_zwater; //Tei zwater offset
extern int hell_count;

extern cvar_t r_autohell;
void Static_Hell_Here(vec3_t pos);//Tei HELL


void EmitWaterPolys (msurface_t *s)
{
	glpoly_t	*p;
	float		*v;
	int			i;
	float		r, t, os, ot;
	vec3_t		nv;


	// Tei autolava

	vec3_t		fx;
	int         tx=0;

	
	int			islava, lame, istele, iswater , islux, luxvalue;
	int			flava, ftele, fwater, draw;
	char		* rgbcod;
	int			cred, cgreen, cblue;

	qboolean	extended; // extended colors

	istele = (
		s->texinfo->texture->name[0] == '*'
		&& ( s->texinfo->texture->name[1] == 't' ) 
		&& (s->texinfo->texture->name[2] == 'e' )
		) ;
	iswater = (
		s->texinfo->texture->name[0] == '*'
		&& ( s->texinfo->texture->name[1] == 'w') 
		&& (s->texinfo->texture->name[2] == 'a' )
		&& (s->texinfo->texture->name[3] == 't' )
		&& (s->texinfo->texture->name[4] == 'e' )
		) ;
	islava	= s->texinfo->texture->islava | (
		  s->texinfo->texture->name[0] == '*' 
		  && (s->texinfo->texture->name[1] == 'l' ) 
		  && (s->texinfo->texture->name[2] == 'a' ) 
		) ;
 
	islux   = s->texinfo->texture->islava | (
		  s->texinfo->texture->name[0] == '*' 
		  && (s->texinfo->texture->name[1] == 'l' ) 
		  && (s->texinfo->texture->name[2] == 'u' ) 
		) ; //Lux!

	

	fwater	= s->texinfo->texture->fwater;
	flava	= s->texinfo->texture->flava;
	ftele	= s->texinfo->texture->ftele;
	
	draw	= !(s->texinfo->texture->fnodraw);


	if (!draw)
	{
		iswater = true;
		fwater	= true;
	}

	if (islux) {
		luxvalue = s->texinfo->texture->name[3];		
		draw = false;
	}

	extended = false;

	if (fwater || flava || ftele || !draw ) {
		rgbcod = s->texinfo->texture->name + (sizeof("*waterfx") - 1);
		if (rgbcod[0]>='0' && rgbcod[0]<='9')
		{
			extended = true; //Extended TextureFogColor
			//Con_Printf("Extended TextureFogColor %d %d %d\n", s->texinfo->texture->wfx_red,s->texinfo->texture->wfx_green,s->texinfo->texture->wfx_blue);
			cred	= s->texinfo->texture->wfx_red;
			cgreen	= s->texinfo->texture->wfx_green;
			cblue	= s->texinfo->texture->wfx_blue;

		}
	}

	// Ugly hack FIXME
	/*
	if (iswater)
	{
		if (rand()&1)
			iswater = false; // Less particles hack
		else
		if (rand()&1)
			iswater = false; // Less particles hack
	}
	*/
	// Ugly hack FIXME


	// Tei autolava

	
	//EmitXFX (s);//Tei xfx 

	for (p=s->polys ; p ; p=p->next)
	{
		if (draw) // Tei draw
			glBegin (GL_POLYGON);//GL_TRIANGLE_FAN);//GL_POLYGON);

		tx=0;
		for (i=0,v=p->verts[0] ; i<p->numverts ; i++, v+=VERTEXSIZE)
		{
			os = v[3];
			ot = v[4];

			r = os + turbsin[(int)((ot*0.125+realtime) * TURBSCALE) & 255];
			r *= (0.015f);

			t = ot + turbsin[(int)((os*0.125+realtime) * TURBSCALE) & 255];
			t *= (0.015f);
			
			nv[0] = v[0];
			nv[1] = v[1];
			nv[2] = v[2];
			


			// Tei water/lava with autofire

			/* Randomizer */

			if (tx)
			{	
				if (tx==1) {
					fx[0] = v[0];
					fx[1] = v[1];
					fx[2] = v[2];
				}

				if (rand()&1){
					fx[0] = (fx[0] + v[0]) * 0.5;
				}
				if (rand()&1){
					fx[1] = (fx[1] + v[1]) * 0.5;
				}
				if (rand()&1){
					fx[2] = (fx[2] + v[2]) * 0.5;
				}
			}
			tx++;
			
			if (islux && (teirand(1000)<2))
					FxForLux (luxvalue , fx);



			if (islava)		
			{		
				if (hell_count<r_autohell.value && lhrandom(0,1000*hell_count+2000)<2 )
				{
					hell_count++;
					Static_Hell_Here(fx);//XFX for fun
				}

				if (r_autolava.value || flava)
				if ( teirand(16)<2 )
					{
						if ( r_autolava.value == 2)
						if ( teirand(64)<2 ) 
									{
										if (extended)
											R_ParticleFogColor(fx, cred,cgreen,cblue,r_autolava.value ); 
										else
											R_ParticleFogLava (fx);
										//void Static_Hell_Here(vec3_t pos)
										
									}
					
						if (flava) 
							lame = 7;
						else
							lame = r_autolava.value;//lame int(conversion)
						R_FireExploLava(fx,1+ rand()&(lame));
					}
			}
			else
			if (istele)
			{
			if (r_autotele.value || ftele)
			if (teirand(128)<2  )
				{
					if (r_autotele.value ==1)
						R_ParticleImplosion (fx);
					else {					
						//R_RayFieldParticles(fx);
						R_ParticleImplosionHaze(fx);
					}
				}
			}
			else
			{
			if (r_autofogwater.value ||fwater)
				if (teirand(512)<2  )
				{
					if (extended)
						R_ParticleFogColor(fx, cred, cgreen, cblue ,r_autofogwater.value);
					else
						R_ParticleFog (fx);
				}


			}			

			if (r_autobubbleglobal.value)
			{
				if (teirand(500)<10) {
					fx[2] -= teirand(600)*4;
					R_ParticleBubble(fx);
				}
			}

			// Tei tele with autotele
			
			//Con_Printf ("%d azar\n", rand()%32000);

			if (draw) //Tei draw
			{ 
				
				if (r_wave.value )//&& r_wave.value <8)
					nv[2] = v[2] + r_wave.value *sin(v[0]*0.02+realtime)*sin(v[1]*0.02+realtime)*sin(v[2]*0.02+realtime);

				//Tei zwater offset
				if (mod_zwater.value)
				{
					nv[2] += mod_zwater.value;

				}	
				//Tei zwater offset

				//else
				//{
				//	nv[2] = v[2] +(r_wave.value *sin(v[0]*0.02+realtime)*sin(v[1]*0.02+realtime)*sin(v[2]*0.02+realtime));
				//}

				//glTexCoord2f (r , t);
				glTexCoord2f (r , t);
				glVertex3fv (nv);
			}
		}
		if (draw) // Tei draw
			glEnd ();
	}
}


//CHP
// CheapAlert Code - overt your eyes
extern cvar_t gl_detailscale;//Tei detail scale

void EmitDetailPolys (msurface_t *s)
{
	glpoly_t	*p;
	float		*v;
	int			i;//, tn;
    //vec3_t          vert;
	int scale = gl_detailscale.value;

	//tn = (int)(host_time * 20)%31;

	if (!detailtexture)
		return;

    glBindTexture (GL_TEXTURE_2D, detailtexture);                    // chp - put in the texture data
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);        // chp - how the texture is slapped on the polys
	//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);        // chp - how the texture is slapped on the polys
    //glColor4f (1,1,1,0.3f);                                          // chp - full color, fade off a little
	glColor4f (0,0,1,0.1f);                                          // chp - full color, fade off a little
    glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);                         // chp - this is also known as "modulate blend" in unreal rendering jargon.
	
	//glBlendFunc(GL_SRC_ALPHA_SATURATE, GL_SRC_ALPHA_SATURATE);//lux
	//glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_DST_COLOR);//moon
	//glBlendFunc(GL_SRC_ALPHA_SATURATE, GL_ONE_MINUS_DST_COLOR);//jpg alike
	//glBlendFunc(GL_DST_COLOR, GL_SRC_ALPHA_SATURATE);//lux

	
	for (p=s->polys ; p ; p=p->next)
	{
        glBegin (GL_POLYGON);                                    // chp - duplicate polys cos i don't know how to multitexture this baby yet
		for (i=0,v=p->verts[0] ; i<p->numverts ; i++, v+=VERTEXSIZE)
		{
			/*
			v[0] = (v[0] * r_world_matrix[0]) + (v[1] * r_world_matrix[4]) + (v[2] * r_world_matrix[8] ) + r_world_matrix[12];
			v[1] = (v[0] * r_world_matrix[1]) + (v[1] * r_world_matrix[5]) + (v[2] * r_world_matrix[9] ) + r_world_matrix[13];
			v[2] = (v[0] * r_world_matrix[2]) + (v[1] * r_world_matrix[6]) + (v[2] * r_world_matrix[10]) + r_world_matrix[14];

			VectorNormalize (v);
			*/

            //glTexCoord2f (v[3] * 18, v[4] * 18);             // chp - jam some detail in there, raise these numbers for your viewing pleasure
			glTexCoord2f (v[3] * scale, v[4] * scale);             // chp - jam some detail in there, raise these numbers for your viewing pleasure
			glVertex3fv (v);
		}
		glEnd ();
	}
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f (1,1,1,1);
}

// CheapAlert code - open your eyes
//CHP
void EmitRSDetailPolys (msurface_t *s, int mydetail, int myscale)
{
	glpoly_t	*p;
	float		*v;
	int			i;//, tn;
	int scale = myscale;


	if (!mydetail)
		return;

    glBindTexture (GL_TEXTURE_2D, mydetail);                    // chp - put in the texture data
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);        // chp - how the texture is slapped on the polys
                                         // chp - full color, fade off a little
	glColor4f (0,0,1,0.1f);                                          // chp - full color, fade off a little
    glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);                         // chp - this is also known as "modulate blend" in unreal rendering jargon.
	
	
	for (p=s->polys ; p ; p=p->next)
	{
        glBegin (GL_POLYGON);                                    // chp - duplicate polys cos i don't know how to multitexture this baby yet
		for (i=0,v=p->verts[0] ; i<p->numverts ; i++, v+=VERTEXSIZE)
		{
			glTexCoord2f (v[3] * scale, v[4] * scale);             // chp - jam some detail in there, raise these numbers for your viewing pleasure
			glVertex3fv (v);
		}
		glEnd ();
	}
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f (1,1,1,1);
}
// CheapAlert code - open your eyes
//CHP


void EmitXFX (msurface_t *s)
{
	glpoly_t	*p;
	float		*v;
	int		i;
	vec3_t		vert;

	p = s->polys;

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);//GL_MODULATE);
	glBindTexture (GL_TEXTURE_2D, shinytexture);
	glBlendFunc(GL_ONE, GL_SRC_COLOR);

	for (p=s->polys ; p ; p=p->next)
	{
		glBegin (GL_POLYGON);

		for (i=0,v=p->verts[0] ; i<p->numverts ; i++, v+=VERTEXSIZE)
		{
			vert[0] = (v[0] * r_world_matrix[0]) + (v[1] * r_world_matrix[4]) + (v[2] * r_world_matrix[8] ) + r_world_matrix[12];
			vert[1] = (v[0] * r_world_matrix[1]) + (v[1] * r_world_matrix[5]) + (v[2] * r_world_matrix[9] ) + r_world_matrix[13];
			vert[2] = (v[0] * r_world_matrix[2]) + (v[1] * r_world_matrix[6]) + (v[2] * r_world_matrix[10]) + r_world_matrix[14];

			VectorNormalize (vert);
		
			glTexCoord2f (vert[0], vert[1]);
			glVertex3fv (v);
		}

		glEnd ();
	}

	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glColor4f(1,1,1,1);
}






/*
=============
EmitEnvMapPolys

Enviroment Mapping
=============
*/

void EmitEnvMapPolys (msurface_t *s)
{
	glpoly_t	*p;
	float		*v;
	int		i;
	vec3_t		vert;

	if (!gl_envmap.value)
		return;

	p = s->polys;


	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glBindTexture (GL_TEXTURE_2D, shinytexture);
	glBlendFunc(GL_ZERO, GL_SRC_COLOR);

	for (p=s->polys ; p ; p=p->next)
	{
		glBegin (GL_POLYGON);

		for (i=0,v=p->verts[0] ; i<p->numverts ; i++, v+=VERTEXSIZE)
		{
			vert[0] = (v[0] * r_world_matrix[0]) + (v[1] * r_world_matrix[4]) + (v[2] * r_world_matrix[8] ) + r_world_matrix[12];
			vert[1] = (v[0] * r_world_matrix[1]) + (v[1] * r_world_matrix[5]) + (v[2] * r_world_matrix[9] ) + r_world_matrix[13];
			vert[2] = (v[0] * r_world_matrix[2]) + (v[1] * r_world_matrix[6]) + (v[2] * r_world_matrix[10]) + r_world_matrix[14];

			VectorNormalize (vert);
		
			glTexCoord2f (vert[0], vert[1]);
			glVertex3fv (v);
		}

		glEnd ();
	}

	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glColor4f(1,1,1,1);
}
extern int detailtexture2;

/*
=============
EmitCausticsPolys
=============
*/
void EmitCausticsPolys (msurface_t *s)
{
	glpoly_t	*p;
	float		*v;
	int			i, tn;

	tn = (int)(host_time * 20)%31;
	glBindTexture (GL_TEXTURE_2D, causticstexture[tn]);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glColor4f (1,1,1,0.3f);
	glBlendFunc(GL_ZERO, GL_SRC_COLOR);

	for (p=s->polys ; p ; p=p->next)
	{
		glBegin (GL_POLYGON);
		for (i=0,v=p->verts[0] ; i<p->numverts ; i++, v+=VERTEXSIZE)
		{
			glTexCoord2f (v[3], v[4]);
			glVertex3fv (v);
		}
		glEnd ();
	}

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f (1,1,1,1);
}

//QMB!
void EmitUnderwaterPolys (msurface_t *fa)
{
	glpoly_t	*p;
	float		*v;
	int			i;
	float		s, t, os, ot;
//	int tn;

	//GL_Bind (underwatertexture);
	//tn = (int)(host_time * 20)%31;
	//glBindTexture (GL_TEXTURE_2D, causticstexture[tn]);
	glBindTexture (GL_TEXTURE_2D, shinytexture);

	//glEnable (GL_BLEND);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);//GL_DECAL);
	glColor4f (1,1,1,0.01f);
	glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);

	for (p=fa->polys ; p ; p=p->next)
	{
		glBegin (GL_POLYGON);
		for (i=0,v=p->verts[0] ; i<p->numverts ; i++, v+=VERTEXSIZE)
		{
			os = v[3];
			ot = v[4];

			s = os + turbsin[(int)((ot*0.5+(realtime*0.5)) * TURBSCALE) & 255];
			s *= ((0.5/64)*(-1))*3;

			t = ot + turbsin[(int)((os*0.5+(realtime*0.5)) * TURBSCALE) & 255];
			t *= ((0.5/64)*(-1))*3;

			glTexCoord2f (s, t);
			glVertex3fv (v);
		}
		glEnd();
	}
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glDisable (GL_BLEND);
	glColor4f (1,1,1,1);
}
//QMB!

void EmitUnderwaterPolys2 (msurface_t *fa) //without animation
{
	glpoly_t	*p;
	float		*v;
	int			i;
	float		s, t, os, ot;
//	int tn;
	int factor = 10;

	//GL_Bind (underwatertexture);
	//tn = (int)(host_time * 20)%31;
	//glBindTexture (GL_TEXTURE_2D, causticstexture[tn]);
	glBindTexture (GL_TEXTURE_2D, detailtexture2);

	//glEnable (GL_BLEND);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);//GL_DECAL);
	glColor4f (1,1,1,0.01f);
	glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);


	for (p=fa->polys ; p ; p=p->next)
	{
		glBegin (GL_POLYGON);
		for (i=0,v=p->verts[0] ; i<p->numverts ; i++, v+=VERTEXSIZE)
		{
			os = v[3];
			ot = v[4];

			s = os + turbsin[(int)((ot*0.5+(factor*0.5)) * TURBSCALE) & 255];
			s *= ((0.5/64)*(-1))*3;

			t = ot + turbsin[(int)((os*0.5+(factor*0.5)) * TURBSCALE) & 255];
			t *= ((0.5/64)*(-1))*3;

			glTexCoord2f (s, t);
			glVertex3fv (v);
		}
		glEnd();
	}
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glDisable (GL_BLEND);
	glColor4f (1,1,1,1);
}
//QMB!

//QMB


void AddTrailTrail2(vec3_t start, vec3_t end, float time, float size, vec3_t color);

#if 0
void RecurseGrass( vec3_t origin, int level)
{
	vec3_t		fx,color;
	int         tx=0;
	vec3_t		nv,v2,v3;

	color[0]= 0.2;
	color[1]= 0.6;
	color[2]= 0.3;

	VectorCopy(origin,v2);

	v2[0]+= sin(v2[0])*32 - cos(v2[1])*32;
	v2[1]+= cos(v2[1])*32 - sin(v2[0])*32;

	VectorCopy(v2, v3);

	v3[2]+= 32-level + cos(v2[1])*16 + sin(v2[0])*32;

	
	AddTrailTrail2( origin , v3, 0.1, 3, color);

	level++;

	if (level<5) 
		RecurseGrass( v3, level );	
}
#endif


extern float sint[7], cost[7];
extern int part_tex;



entity_t *CL_NewTempEntity (void);

extern model_t * grassmodel;


extern cvar_t temp1;
extern cvar_t r_autograss;
extern cvar_t r_grassdensity;
extern cvar_t r_grasspeed;

//Tei grass models
void EmitGrassFx (msurface_t *s)
{
	glpoly_t	*p;
	float		*v;
	int			i,t,k;


	entity_t	*ent;

	if (!r_autograss.value)
		return;

	for (p=s->polys ; p ; p=p->next)
	{

		for (i=0,v=p->verts[0] ; i<p->numverts ; i++, v+=VERTEXSIZE)
		{

			if (!grassmodel)
				continue;

			t = v[0]+v[1]+v[2] + (int)grassmodel;//"random" grass seed

			if (sin(t)<r_grassdensity.value)
				continue;

			ent = CL_NewTempEntity ();
			if (!ent)
				continue;
			
			VectorCopy (v, ent->origin);
			ent->model = grassmodel;
			VectorCopy(vec3_origin, ent->angles);

			// Help avoid totally blankmodels and patterns.
			ent->origin[0] += 1;
			ent->origin[1] += 1;
			ent->origin[2] += 1;
			ent->angles[1] = sin(t)*360;

			k =  cl.time *r_grasspeed.value + t% (ent->model->numframes);;
			k = k % (ent->model->numframes);

			ent->frame = k;
		}
	}
}



void EmitRSGrassFx (msurface_t *s, model_t *thegrassmodel, float density)
{
	glpoly_t	*p;
	float		*v;
	int			i,t,k;


	entity_t	*ent;

	if (!r_autograss.value)
		return;

	if (!thegrassmodel)
		return;
	
	for (p=s->polys ; p ; p=p->next)
	{

		for (i=0,v=p->verts[0] ; i<p->numverts ; i++, v+=VERTEXSIZE)
		{
			t = v[0]+v[1]+v[2];
			
			t = (int)thegrassmodel + t*(int)thegrassmodel;

			if (sin(t)<density)
				continue;

			ent = CL_NewTempEntity ();
			if (!ent)
				continue;
			
			VectorCopy (v, ent->origin);
			ent->model = thegrassmodel;
			VectorCopy(vec3_origin, ent->angles);

			ent->origin[0] += 1;
			ent->origin[1] += 1;
			ent->origin[2] += 2;

			ent->angles[1] = sin(t<<1)*360;

			//ent->frame = host_frametime
			k =  cl.time *r_grasspeed.value + t% (ent->model->numframes);;
			k = k % (ent->model->numframes);

			ent->frame = k;
		}
	}
}

//Tei grass models


//Tei smoke rscript

void R_ParticleFogX (vec3_t origin, float density);
void EmitRSSmokeFireFx (msurface_t *s, float smokeheight, float density)
{
	glpoly_t	*p;
	float		*v;
	int			i;//,t;
	vec3_t		vx;

	
	for (p=s->polys ; p ; p=p->next)
	{

		for (i=0,v=p->verts[0] ; i<p->numverts ; i++, v+=VERTEXSIZE)
		{
			//t = v[0]+v[1]+v[2]  ;

			//if (((sin(t)+1)*0.5)<density)
			//	continue;
			
			VectorCopy (v, vx);

			vx[2] += teirand(smokeheight);

            if (teirand((32*density))<2)
            {
                        //R_ParticleFogX (vx,255*density);   
					R_FireExploLava(vx,1+ rand()&7);
					if(teirand(64)<2)
						R_ParticleFogLava (vx);
						
            } 
		}
	}
}
//Tei smoke rscript

//QMB
void EmitChromePoly (msurface_t *fa)
{
	glpoly_t	*p;
	float		*v;
	int			i;
//	float		x, y;
	vec3_t		vert;

	//GL_DisableMultitexture();
	p = fa->polys;
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	//GL_Bind (shinetex_type);

	glBindTexture (GL_TEXTURE_2D, detailtexture2);

	glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
	//glEnable (GL_BLEND);
	glColor4f(1,1,1,0.01f);

	for (p=fa->polys ; p ; p=p->next)
	{
		glBegin (GL_POLYGON);

		for (i=0,v=p->verts[0] ; i<p->numverts ; i++, v+=VERTEXSIZE)
		{
			vert[0] = (v[0] * r_world_matrix[0]) + (v[1] * r_world_matrix[4]) + (v[2] * r_world_matrix[8] ) + r_world_matrix[12];
			vert[1] = (v[0] * r_world_matrix[1]) + (v[1] * r_world_matrix[5]) + (v[2] * r_world_matrix[9] ) + r_world_matrix[13];
			vert[2] = (v[0] * r_world_matrix[2]) + (v[1] * r_world_matrix[6]) + (v[2] * r_world_matrix[10]) + r_world_matrix[14];

			VectorNormalize (vert);
		
			glTexCoord2f (vert[0], vert[1]);
			glVertex3fv (v);
		}

		glEnd ();
	}

	//glDisable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glColor4f(1,1,1,1);
	//GL_EnableMultitexture();
}
//QMB