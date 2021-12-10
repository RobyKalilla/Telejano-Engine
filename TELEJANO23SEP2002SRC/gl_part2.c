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

/*******************************************************
QMB: new particle engine

  almost ready for release!!
  done: just need to make all the old functions call the new ones

  maybe add some more documentation

  work out what needs to be done to add to other quake engine

  needs 32bit texture loading (from www.quakesrc.org)
*******************************************************
Shove this in to glquake insted of whats there at the moment for particles


*******************************************************/

#include "quakedef.h"
//#include "r_local.h"

extern cvar_t temp1;//Tei for debug purposes


typedef enum {
	pt_static, pt_grav, pt_slowgrav, pt_rise, pt_slowrise, pt_explode, pt_explode2, pt_blob, pt_blob2, pt_fire, p_lightning
} part_move_t;


typedef struct xparticle_s
{
// driver-usable fields
	vec3_t		org;
	vec3_t		org2;
	vec3_t		colour; //float color (stops the need for looking up the 8bit to 24bit everytime
// drivers never touch the following fields
	struct xparticle_s	*next;
	vec3_t		vel;
	float		ramp; //sort of diffrent purpose
	float		die;
	float		start;
	int			hit;
	float		size;
	float		delay;
	float		fat; //how fat
	part_move_t			type;
} xparticle_t;


//QMB: particles
//types of particles
typedef enum {
	p_sparks, p_smoke, p_fire, p_blood, p_chunks, p_smoke_fade, p_custom
} p_type_t;

//QMB: particles
//custom movement effects

//QMB: particles

//particle struct
//new particles
//add a trail of any particle type
void AddTrail(vec3_t start, vec3_t end, int type, float time, float size);
void AddTrailColor(vec3_t start, vec3_t end, int type, float time, float size, vec3_t color);
void AddTrailTrail(vec3_t start, vec3_t end, float time, float size, vec3_t color);

void AddFadeSmoke(vec3_t org, int count);

//QMB: end
//add a particle or two
//Times: smoke=3, spark=2, blood=3, chunk=4
void AddParticle(vec3_t org, int count, float size, float time, int type);
void AddParticleColor(vec3_t org, int count, float size, float time, int type, vec3_t colour);

void XR_ClearParticles (void);


//default max # of particles at one time
//#define MAX_PARTICLES			2048
#define MAX_PARTICLES			8192
//no fewer than this no matter what's on the command line
#define ABSOLUTE_MIN_PARTICLES	1024

//colour ramps: need to be changed to 24-bit colour
int		ramp1[8] = {0x6f, 0x6d, 0x6b, 0x69, 0x67, 0x65, 0x63, 0x61};
int		ramp2[8] = {0x6f, 0x6e, 0x6d, 0x6c, 0x6b, 0x6a, 0x68, 0x66};
int		ramp3[8] = {0x6d, 0x6b, 6, 5, 4, 3};

//sin and cos tables for the particle triangle fans
float sint[7], cost[7];
 
//linked lists pointers for the first of the active, free and the a spare list
xparticle_t	*xactive_particles, *xfree_particles, *xparticles;
xparticle_t	*smoke, *fire, *blood, *sparks, *chunks, *env, *trails, *lightning;
//Holder of the particle texture
int			part_tex, smoke_tex, trail_tex, lightning_tex;

//number of particles in total
int			r_xnumParticles;
int			xr_numdelayadds;
int			xnumParticles;
//vec3_t			r_pright, r_pup, r_ppn;
//replaced by the r_particles cvar_t	gl_texpart = {"gl_texpart","1"};
cvar_t r_particles = {"r_particles", "1", true};
cvar_t	gl_trail = {"gl_trail","1"};
cvar_t	r_particle_dist = {"r_particle_dist","20"};

//internal functions
void R_UpdateParticles(void);
void TraceLineN (vec3_t start, vec3_t end, vec3_t impact, vec3_t normal);
void R_UpdateAll(void);
void MakeParticleTexure(void);
void DrawTriFans(void);
void DrawTexQuad(void);
void DelayAddChunk(vec3_t org, int count, vec3_t colour, float time);






/*
===============
R_InitParticles
===============
*/
void XR_InitParticles (void)
{
	int		i, a;

	i = COM_CheckParm ("-particles");
	if (i){
		Con_Printf("Can't set custom amount of particles.");
		r_xnumParticles = (int)(atoi(com_argv[i+1]));
		if (r_xnumParticles < ABSOLUTE_MIN_PARTICLES)
			r_xnumParticles = ABSOLUTE_MIN_PARTICLES;
	}
	else{
		r_xnumParticles = MAX_PARTICLES;
	}

	xparticles = (xparticle_t *)Hunk_AllocName (r_xnumParticles * sizeof(xparticle_t), "particles-all");

	//calculate sin/cos tables
	for (i=0;i<7;i++)
	{
		a = i/7.0 * M_PI*2;
		sint[i]=sin(a);
		cost[i]=cos(a);
	}

	XR_ClearParticles();

	MakeParticleTexure();

	Cvar_RegisterVariable (&r_particles);
	Cvar_RegisterVariable (&gl_trail);
	Cvar_RegisterVariable (&r_particle_dist);
}


/*
===============
R_EntityParticles
===============
*/

#define NUMVERTEXNORMALS	162
extern	float	r_avertexnormals[NUMVERTEXNORMALS][3];
vec3_t	avelocities[NUMVERTEXNORMALS];
float	beamlength = 16;
vec3_t	avelocity = {23, 7, 3};
float	partstep = 0.01f;
float	timescale = 0.01f;


/*
===============
R_ClearParticles
===============
*/
void XR_ClearParticles (void)
{
	int		i;
	
	xfree_particles = &xparticles[0];
	//xactive_particles = smoke = chunks = fire = blood = sparks = env = NULL;
	xactive_particles = smoke = chunks = fire = blood = sparks = env = trails = NULL;

	for (i=0 ;i<r_xnumParticles ; i++)
		xparticles[i].next = &xparticles[i+1];

	xparticles[r_xnumParticles-1].next = NULL;
	xnumParticles = 0;
}

//Reads a point file of particles
//Does this ever get used?


/*
===============
R_ParseParticleEffect

Parse an effect out of the server message
===============
*/
void XR_ParseParticleEffect (void)
{
	vec3_t		org, dir;
	int			i, count, msgcount, color;
	
	for (i=0 ; i<3 ; i++)
		org[i] = MSG_ReadCoord ();
	for (i=0 ; i<3 ; i++)
		dir[i] = MSG_ReadChar () * (1.0/16);
	msgcount = MSG_ReadByte ();
	color = MSG_ReadByte ();

	if (msgcount == 255) //explosion
		count = 1024;
	else if (msgcount == 254) //blood from gibs
		count = 512;
	else
		count = msgcount;
	
	R_RunParticleEffect (org, dir, color, count);
}
	
/*===============
R_ParticleExplosion
===============*/
void XR_ParticleExplosion (vec3_t org)
{
	AddParticle(org,  64, 300, 2, p_sparks);
	AddParticle(org,  32, 200, 2, p_sparks);
	AddParticle(org,  20,  25, 1, p_fire);
}

/*===============
R_ParticleExplosion2
===============*/
//Needs to be made to call new functions so that old ones can be removed
void XR_ParticleExplosion2 (vec3_t org, int colorStart, int colorLength)
{
	vec3_t	colour;
	byte	*colourByte;

	colourByte = (byte *)&d_8to24table[colorStart];
	colour[0] = (float)colourByte[0]/255.0;
	colour[1] = (float)colourByte[1]/255.0;
	colour[2] = (float)colourByte[2]/255.0;
	
	AddParticleColor(org, 128, 200, 2, p_sparks, colour);
	AddParticle(org, 64, 1, 3, p_smoke);
}

/*===============
R_BlobExplosion
===============*/
//allow for colored sparks and then use blue ones
//also do colored fires
void XR_BlobExplosion (vec3_t org)
{
	vec3_t	colour;

	colour[0]=0;
	colour[1]=0;
	colour[2]=1;

	AddParticleColor(org, 128, 200, 2, p_sparks, colour);
}

void XR_BlobExplosion2 (vec3_t org)
{
	vec3_t	colour;

	colour[0]=0.4f;
	colour[1]=0.4f;
	colour[2]=0.9f;

	AddParticleColor(org, 3, 50, 2, p_sparks, colour);
}

void XR_BlobExplosion3 (vec3_t org)
{
	vec3_t	colour;

	colour[0]=0.7f;
	colour[1]=0.7f;
	colour[2]=1;

	AddParticleColor(org, 12, 50, 2, p_sparks, colour);
}

/*===============
R_RunParticleEffect
===============*/
void XR_RunParticleEffect (vec3_t org, vec3_t dir, int color, int count)
{
	byte		*colourByte;
	vec3_t		colour;
	
	colourByte = (byte *)&d_8to24table[color];
	colour[0] = colourByte[0]/255.0;
	colour[1] = colourByte[1]/255.0;
	colour[2] = colourByte[2]/255.0;

	if (count == 20)
	{
		AddParticleColor(org, 10, 2, 4, p_chunks, colour);  
		AddParticle(org, 1, 100, 0.05f, p_sparks);
	}else if (count == 1024)
		R_ParticleExplosion(org);
	else if (count == 512)
		AddParticleColor(org, 10, 2.5, 3, p_blood, colour);
	else
	{
		AddParticleColor(org, count, 1, 2, p_blood, colour);
	}
}


/*===============
R_LavaSplash
===============*/
//Need to find out when this is called
void XR_LavaSplash (vec3_t org)
{
	AddParticle(org, 1500, 1000, 10, p_sparks);
	AddParticle(org, 35, 55, 5, p_fire);
}

/*===============
R_TeleportSplash
===============*/
//Need to be changed so that they spin down into the ground
//whould look very cool
//maybe coloured blood (new type?)
void XR_TeleportSplash (vec3_t org)
{
	vec3_t	colour;

	colour[0]=0.9f;
	colour[1]=0.9f;
	colour[2]=0.9f;

	AddParticleColor(org, 256, 200, 0.1f, p_sparks, colour);
}

//Should be made to call the new functions to keep compatablity
void XR_RocketTrail (vec3_t start, vec3_t end, int type)
{
	vec3_t		colour;
	
	switch (type)
	{
		case 0:	// rocket trail
			AddTrail(start, end, p_fire, 0.1f, 6);
			AddTrail(start, end, p_smoke, 1, 1);
			break;

		case 1:	// smoke smoke
			AddTrail(start, end, p_smoke, 1, 2);
			break;

		case 2:	// blood
			AddTrail(start, end, p_blood, 2, 3);
			break;

		case 3:	//tracer 1
			colour[0]=0;	colour[1]=1;	colour[2]=0;
			AddTrailColor (start, end, p_blood, 2, 2, colour);
			break;

		case 5:	// tracer 2
			colour[0]=1;	colour[1]=0.85f;	colour[2]=0;
			AddTrailColor (start, end, p_blood, 2, 2, colour);
			break;

		case 4:	// slight blood
			AddParticle(end, 20, 1, 2, p_blood);
			break;

		case 6:	// voor trail
			colour[0]=1;	colour[1]=0;	colour[2]=0.75f;
			AddTrailColor (start, end, p_blood, 2, 2, colour);
			break;
		//SPX
		case 7:	// ship trail
			//AddTrail(start, end, p_fire, 0.1f, 1);
			//AddTrail(start, end, p_smoke, 0.1, 0.1);
			colour[0]= 0.5f;	colour[1]=0.5f;	colour[2]=0.75f;
			AddTrailTrail(start, end, 2.5 , 1.5, colour);

			//colour[0]= 0.9f;	colour[1]=0.9f;	colour[2]=1;
			//AddTrailTrail(start, end, 2 , 2, colour);

			//R_ParticlePuffSmoke(end, 20);//SPX?

			break;
		case 8:	// shiprocket trail
			//AddTrail(start, end, p_fire, 0.1f, 1);
			//AddTrail(start, end, p_smoke, 0.1, 0.1);
			colour[0]= 1;	colour[1]=0.7f;	colour[2]=0.7f;
			AddTrailTrail(start, end, 1, 1, colour);
			//AddTrail(start, end, p_smoke_fade, 1, 6);
			break;
		case 9:	// ship laser trail
			//AddTrail(start, end, p_fire, 0.1f, 1);
			//AddTrail(start, end, p_smoke, 0.1, 0.1);
			colour[0]= 0.5f;	colour[1]=0.1f;	colour[2]=0.1f;
			AddTrailTrail(start, end, 3, 0.1f, colour);
			break;
		//SPX
		case 10:	// ship trail2
			//AddTrail(start, end, p_fire, 0.1f, 1);
			//AddTrail(start, end, p_smoke, 0.1, 0.1);
			colour[0]= 1;	colour[1]=1;	colour[2]=1;
			AddTrailTrail(start, end, 0.5 , 2, colour);

			//colour[0]= 0.9f;	colour[1]=0.9f;	colour[2]=1;
			//AddTrailTrail(start, end, 2 , 2, colour);

			//R_ParticlePuffSmoke(end, 20);//SPX?
			//Con_Printf("hello\n");
			break;



	}
}

/*
===============
R_DrawParticles
===============
*/
extern	cvar_t	sv_gravity;

void XR_DrawParticles (void)
{
	//See if they can be compressed or made neater
	if (cl.time!=cl.oldtime)
		R_UpdateAll();


	if (gl_fogenable.value)
	{
		glDisable(GL_FOG);
	}

	glDepthMask(0);
	glEnable (GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glShadeModel (GL_SMOOTH);

	//if (r_particles.value)
	//	DrawTexQuad();
	//else
		DrawTriFans();

	glDepthMask(1);
	//glDisable (GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	if (gl_fogenable.value)
	{
		glEnable(GL_FOG);
	}
}

void DrawTriFans(void)
{
	xparticle_t		*p;

	vec3_t			v;
	float			rad;
	int				i;
	float			alphaF, size;

	glDisable (GL_TEXTURE_2D);

	rad=1;
	//Draw other particles
	/*
	for (p=xactive_particles ; p ; p=p->next)
	{
		glBegin (GL_TRIANGLE_FAN);
		alpha = (1-(cl.time-p->start)/(p->die-p->start))*255;
		colour = (byte *)&d_8to24table[(int)p->color];
		glColor4ub (colour[0],colour[1],colour[2],alpha);
		glVertex3fv (p->org);
		glColor4f (0,0,0,0);
		for (i=7 ; i>=0 ; i--)
		{
			v[0] = p->org[0] + vright[0]*cost[i%7]*rad + vup[0]*sint[i%7]*rad;
			v[1] = p->org[1] + vright[1]*cost[i%7]*rad + vup[1]*sint[i%7]*rad;
			v[2] = p->org[2] + vright[2]*cost[i%7]*rad + vup[2]*sint[i%7]*rad;
			glVertex3fv (v);
		}
		glEnd ();
	}*/

	//Draw smoke particles
	for (p=smoke ; p ; p=p->next)
	{
		glBegin (GL_TRIANGLE_FAN);
		glColor4f (p->colour[0]*p->ramp,p->colour[1]*p->ramp,p->colour[2]*p->ramp,p->ramp);
		glVertex3fv (p->org);
		glColor4f (0,0,0,0);
		for (i=7 ; i>=0 ; i--)
		{
			v[0] = p->org[0] + vright[0]*cost[i%7]*p->size + vup[0]*sint[i%7]*p->size;
			v[1] = p->org[1] + vright[1]*cost[i%7]*p->size + vup[1]*sint[i%7]*p->size;
			v[2] = p->org[2] + vright[2]*cost[i%7]*p->size + vup[2]*sint[i%7]*p->size;

			glVertex3fv (v);
		}
		glEnd ();


		
	}

	//Draw blood particles
	for (p=blood ; p ; p=p->next)
	{
		glBegin (GL_TRIANGLE_FAN);
		//alphaF = (1-(cl.time-p->start)/(p->die-p->start));
		glColor4f (p->colour[0]*p->ramp,p->colour[1]*p->ramp,p->colour[2]*p->ramp,p->ramp);//alphaF);
		glVertex3fv (p->org);
		glColor4f (0.25,0,0,0);
		for (i=7 ; i>=0 ; i--)
		{
			v[0] = p->org[0] + vright[0]*cost[i%7]*p->size + vup[0]*sint[i%7]*p->size;
			v[1] = p->org[1] + vright[1]*cost[i%7]*p->size + vup[1]*sint[i%7]*p->size;
			v[2] = p->org[2] + vright[2]*cost[i%7]*p->size + vup[2]*sint[i%7]*p->size;

			glVertex3fv (v);
		}
		glEnd ();
	}

	//Draw fire particles
	size = 25;
	for (p=fire ; p ; p=p->next)
	{
		glBegin (GL_TRIANGLE_FAN);
		//alphaF = (1-((cl.time-p->start)/(p->die-p->start))*0.5);
		glColor4f (p->colour[0]*p->ramp,p->colour[1]*p->ramp,p->colour[2]*p->ramp,p->ramp);
		glVertex3fv (p->org);
		glColor4f (0.25,0,0,0);
		for (i=7 ; i>=0 ; i--)
		{
			v[0] = p->org[0] + vright[0]*cost[i%7]*p->size + vup[0]*sint[i%7]*p->size;
			v[1] = p->org[1] + vright[1]*cost[i%7]*p->size + vup[1]*sint[i%7]*p->size;
			v[2] = p->org[2] + vright[2]*cost[i%7]*p->size + vup[2]*sint[i%7]*p->size;

			glVertex3fv (v);
		}
		glEnd ();


		

	}

	//Draw sparks particles
	for (p=sparks ; p ; p=p->next)
	{
		glBegin (GL_TRIANGLE_FAN);
		//alphaF = (1-(cl.time-p->start)/(p->die-p->start));
		glColor4f (p->colour[0]*p->ramp,p->colour[1]*p->ramp,p->colour[2]*p->ramp,p->ramp);
		glVertex3fv (p->org);
		glColor4f (p->colour[0]*p->ramp/2,p->colour[1]*p->ramp/2,p->colour[2]*p->ramp/2,0);
		for (i=7 ; i>=0 ; i--)
		{
			v[0] = p->org[0] - p->vel[0]/4 + vright[0]*cost[i%7]*p->size + vup[0]*sint[i%7]*p->size;
			v[1] = p->org[1] - p->vel[1]/4 + vright[1]*cost[i%7]*p->size + vup[1]*sint[i%7]*p->size;
			v[2] = p->org[2] - p->vel[2]/4 + vright[2]*cost[i%7]*p->size + vup[2]*sint[i%7]*p->size;
			glVertex3fv (v);
		}
		glEnd ();

		

	}

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//Draw chunks particles
	for (p=chunks ; p ; p=p->next)
	{
		glBegin (GL_TRIANGLE_FAN);
		alphaF = (1-(cl.time-p->start)/(p->die-p->start));
		glColor4f (p->colour[0]*p->ramp,p->colour[1]*p->ramp,p->colour[2]*p->ramp,p->ramp);
		glVertex3fv (p->org);
		glColor4f (p->colour[0]*p->ramp/2,p->colour[1]*p->ramp/2,p->colour[2]*p->ramp/2,0);
		for (i=7 ; i>=0 ; i--)
		{
			v[0] = p->org[0] + vright[0]*cost[i%7]*rad + vup[0]*sint[i%7]*rad;
			v[1] = p->org[1] + vright[1]*cost[i%7]*rad + vup[1]*sint[i%7]*rad;
			v[2] = p->org[2] + vright[2]*cost[i%7]*rad + vup[2]*sint[i%7]*rad;

			glVertex3fv (v);
		}
		glEnd ();

		
	}

	glEnable (GL_TEXTURE_2D);
}

//uncomment the next line for mhquake
//extern int bloodtexture;
void DrawTexQuad(void)
{
	xparticle_t		*p;
	float			sz, szz;

	vec3_t			v;
	int				i;
	float			alphaF;

	glDisable (GL_TEXTURE_2D);
	//Draw sparks particles
	for (p=sparks ; p ; p=p->next)
	{
		glBegin (GL_TRIANGLE_FAN);
		alphaF = (1-(cl.time-p->start)/(p->die-p->start));
		glColor4f (p->colour[0],p->colour[1],p->colour[2],alphaF);
		glVertex3fv (p->org);
		glColor4f (p->colour[0]/2,p->colour[1]/2,p->colour[2]/2,0);
		for (i=7 ; i>=0 ; i--)
		{
			//v[0] = p->org[0] - p->vel[0]/8 + vright[0]*cost[i%7]*p->size + vup[0]*sint[i%7]*p->size;
			//v[1] = p->org[1] - p->vel[1]/8 + vright[1]*cost[i%7]*p->size + vup[1]*sint[i%7]*p->size;
			//v[2] = p->org[2] - p->vel[2]/8 + vright[2]*cost[i%7]*p->size + vup[2]*sint[i%7]*p->size;
			if (!p->fat)
				p->fat = 0.5;
			sz  = cost[i%7] * p->size * p->fat;
			szz = sint[i%7] * p->size * p->fat;
			v[0] = p->org[0] - p->vel[0]/8 + vright[0] * sz + vup[0] * szz;
			v[1] = p->org[1] - p->vel[1]/8 + vright[1] * sz + vup[1] * szz;
			v[2] = p->org[2] - p->vel[2]/8 + vright[2] * sz + vup[2] * szz;

			glVertex3fv (v);
		}
		glEnd ();
	}
	glEnable (GL_TEXTURE_2D);

	/*
	//Draw other particles
	for (p=xactive_particles ; p ; p=p->next)
	{
		colour = (byte *)&d_8to24table[(int)p->color];
		glColor4ub (colour[0],colour[1],colour[2],(byte)(p->ramp*255));

		glTexCoord2f (0, 1);
		VectorMA (p->org, -size, vup, v);
		VectorMA (v, -size, vright, v);
		glVertex3fv (v);

		glTexCoord2f (0, 0);
		VectorMA (p->org, size, vup, v);
		VectorMA (v, -size, vright, v);
		glVertex3fv (v);

		glTexCoord2f (1, 0);
		VectorMA (p->org, size, vup, v);
		VectorMA (v, size, vright, v);
		glVertex3fv (v);

		glTexCoord2f (1, 1);
		VectorMA (p->org, -size, vup, v);
		VectorMA (v, size, vright, v);
		glVertex3fv (v);
	}*/

	//glBind(part_tex);//XFX
	glBindTexture (GL_TEXTURE_2D, part_tex);
	//GL_Bind(bloodtexture); //uncomment for MHquake
	glBegin (GL_QUADS);

	//Draw blood particles
	for (p=blood ; p ; p=p->next)
	{
		glColor4f (p->colour[0]*p->ramp,p->colour[1]*p->ramp,p->colour[2]*p->ramp,p->ramp);

		glTexCoord2f (0, 1);
		VectorMA (p->org, -p->size*p->ramp, vup, v);
		VectorMA (v, -p->size*p->ramp, vright, v);
		glVertex3fv (v);

		glTexCoord2f (0, 0);
		VectorMA (p->org, p->size*p->ramp, vup, v);
		VectorMA (v, -p->size*p->ramp, vright, v);
		glVertex3fv (v);

		glTexCoord2f (1, 0);
		VectorMA (p->org, p->size*p->ramp, vup, v);
		VectorMA (v, p->size*p->ramp, vright, v);
		glVertex3fv (v);

		glTexCoord2f (1, 1);
		VectorMA (p->org, -p->size*p->ramp, vup, v);
		VectorMA (v, p->size*p->ramp, vright, v);
		glVertex3fv (v);
	}
	glEnd ();

	//GL_Bind(part_tex);
	glBindTexture (GL_TEXTURE_2D, part_tex);
	glBegin (GL_QUADS);

	//Draw fire particles
	for (p=fire ; p ; p=p->next)
	{
		VectorSubtract (p->org, r_origin, v);
		//if (VLength (v) > r_particle_dist.value*3)
		if (Length (v) > r_particle_dist.value*3)
		{
			glColor4f (p->colour[0]*p->ramp,p->colour[1]*p->ramp,p->colour[2]*p->ramp,p->ramp);

			glTexCoord2f (0, 1);
			VectorMA (p->org, -p->size, vup, v);
			VectorMA (v, -p->size, vright, v);
			glVertex3fv (v);

			glTexCoord2f (0, 0);
			VectorMA (p->org, p->size, vup, v);
			VectorMA (v, -p->size, vright, v);
			glVertex3fv (v);

			glTexCoord2f (1, 0);
			VectorMA (p->org, p->size, vup, v);
			VectorMA (v, p->size, vright, v);
			glVertex3fv (v);

			glTexCoord2f (1, 1);
			VectorMA (p->org, -p->size, vup, v);
			VectorMA (v, p->size, vright, v);
			glVertex3fv (v);
		}
	}
	
	//Draw chunks particles
	for (p=chunks ; p ; p=p->next)
	{
		glColor4f (p->colour[0]*p->ramp,p->colour[1]*p->ramp,p->colour[2]*p->ramp,p->ramp);

		glTexCoord2f (0, 1);
		VectorMA (p->org, -p->size, vup, v);
		VectorMA (v, -p->size, vright, v);
		glVertex3fv (v);

		glTexCoord2f (0, 0);
		VectorMA (p->org, p->size, vup, v);
		VectorMA (v, -p->size, vright, v);
		glVertex3fv (v);

		glTexCoord2f (1, 0);
		VectorMA (p->org, p->size, vup, v);
		VectorMA (v, p->size, vright, v);
		glVertex3fv (v);

		glTexCoord2f (1, 1);
		VectorMA (p->org, -p->size, vup, v);
		VectorMA (v, p->size, vright, v);
		glVertex3fv (v);
	}
	glEnd ();

	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBindTexture (GL_TEXTURE_2D, part_tex);
	//GL_Bind(smoke_tex);
	glBegin (GL_QUADS);
	//Draw smoke particles
	for (p=smoke ; p ; p=p->next)
	{
		VectorSubtract (p->org, r_origin, v);
		if (Length (v) > r_particle_dist.value)
		//if (VLength (v) > r_particle_dist.value)
		{
			glColor4f (p->colour[0],p->colour[1],p->colour[2],p->ramp);
			glTexCoord2f (0, 1);
			VectorMA (p->org, -p->size*(2-p->ramp), vup, v);
			VectorMA (v, -p->size*(2-p->ramp), vright, v);
			glVertex3fv (v);

			glTexCoord2f (0, 0);
			VectorMA (p->org, p->size*(2-p->ramp), vup, v);
			VectorMA (v, -p->size*(2-p->ramp), vright, v);
			glVertex3fv (v);

			glTexCoord2f (1, 0);
			VectorMA (p->org, p->size*(2-p->ramp), vup, v);
			VectorMA (v, p->size*(2-p->ramp), vright, v);
			glVertex3fv (v);

			glTexCoord2f (1, 1);
			VectorMA (p->org, -p->size*(2-p->ramp), vup, v);
			VectorMA (v, p->size*(2-p->ramp), vright, v);
			glVertex3fv (v);
		}
	}
	glEnd();


	//GL_Bind(trail_tex);
	glBindTexture (GL_TEXTURE_2D, trail_tex);

	glBegin (GL_QUADS);
	//Draw trails
	for (p=trails ; p ; p=p->next)
	{
		glColor4f (p->colour[0],p->colour[1],p->colour[2],p->ramp);

		glTexCoord2f (0, 1);
		VectorMA (p->org, p->size, vup, v);
		VectorMA (p->org, p->size, vright, v);
		glVertex3fv (v);

		glTexCoord2f (0, 0);
		//VectorMA (p->org, p->size, vup, v);
		VectorMA (p->org, -p->size, vright, v);
		glVertex3fv (v);

		glTexCoord2f (1, 0);
		//VectorMA (p->org2, -p->size, vup, v);
		VectorMA (p->org2, -p->size, vright, v);
		glVertex3fv (v);

		glTexCoord2f (1, 1);
		VectorMA (p->org2, p->size, vup, v);
		VectorMA (p->org2, p->size, vright, v);
		glVertex3fv (v);

	// reverse texture - for upside down trails, backwards textures are culled neway.
		glTexCoord2f (0, 1);
		VectorMA (p->org, p->size, vup, v);
		VectorMA (p->org2, p->size, vright, v);
		glVertex3fv (v);

		glTexCoord2f (0, 0);
		//VectorMA (p->org, p->size, vup, v);
		VectorMA (p->org2, -p->size, vright, v);
		glVertex3fv (v);

		glTexCoord2f (1, 0);
		//VectorMA (p->org2, -p->size, vup, v);
		VectorMA (p->org, -p->size, vright, v);
		glVertex3fv (v);

		glTexCoord2f (1, 1);
		VectorMA (p->org2, p->size, vup, v);
		VectorMA (p->org, p->size, vright, v);
		glVertex3fv (v);
	}
	glEnd();

}

int GetTex(int type );

void XDrawTexQuad(void)
{
	xparticle_t		*p;

	vec3_t			v;
	int				i;
	float			alphaF;
	int				size;

	glDisable (GL_TEXTURE_2D);
	//Draw sparks particles
	for (p=sparks ; p ; p=p->next)
	{
		glBegin (GL_TRIANGLE_FAN);
		alphaF = (1-(cl.time-p->start)/(p->die-p->start));
		glColor4f (p->colour[0],p->colour[1],p->colour[2],alphaF);
		glVertex3fv (p->org);
		glColor4f (p->colour[0]/2,p->colour[1]/2,p->colour[2]/2,0);
		for (i=7 ; i>=0 ; i--)
		{
			v[0] = p->org[0] - p->vel[0]/8 + vright[0]*cost[i%7]*p->size + vup[0]*sint[i%7]*p->size;
			v[1] = p->org[1] - p->vel[1]/8 + vright[1]*cost[i%7]*p->size + vup[1]*sint[i%7]*p->size;
			v[2] = p->org[2] - p->vel[2]/8 + vright[2]*cost[i%7]*p->size + vup[2]*sint[i%7]*p->size;
			glVertex3fv (v);
		}
		glEnd ();
	}

	glEnable (GL_TEXTURE_2D);

	//GL_Bind(part_tex);
	glBindTexture (GL_TEXTURE_2D, part_tex);
	glBegin (GL_QUADS);

	/*
	//Draw other particles
	for (p=active_particles ; p ; p=p->next)
	{
		colour = (byte *)&d_8to24table[(int)p->color];
		glColor4ub (colour[0],colour[1],colour[2],(byte)(p->ramp*255));

		glTexCoord2f (0, 1);
		VectorMA (p->org, -size, vup, v);
		VectorMA (v, -size, vright, v);
		glVertex3fv (v);

		glTexCoord2f (0, 0);
		VectorMA (p->org, size, vup, v);
		VectorMA (v, -size, vright, v);
		glVertex3fv (v);

		glTexCoord2f (1, 0);
		VectorMA (p->org, size, vup, v);
		VectorMA (v, size, vright, v);
		glVertex3fv (v);

		glTexCoord2f (1, 1);
		VectorMA (p->org, -size, vup, v);
		VectorMA (v, size, vright, v);
		glVertex3fv (v);
	}*/

	//Draw blood particles
	size = 1.2;
	for (p=blood ; p ; p=p->next)
	{
		glColor4f (p->colour[0]*p->ramp,p->colour[1]*p->ramp,p->colour[2]*p->ramp,p->ramp);

		glTexCoord2f (0, 1);
		VectorMA (p->org, -p->size*p->ramp, vup, v);
		//VectorMA (v, -p->size*p->ramp, vright, v);
		glVertex3fv (v);

		glTexCoord2f (0, 0);
		VectorMA (p->org, p->size*p->ramp, vup, v);
		//VectorMA (v, -p->size*p->ramp, vright, v);
		glVertex3fv (v);

		glTexCoord2f (1, 0);
		VectorMA (p->org, p->size*p->ramp, vup, v);
		//VectorMA (v, p->size*p->ramp, vright, v);
		glVertex3fv (v);

		glTexCoord2f (1, 1);
		VectorMA (p->org, -p->size*p->ramp, vup, v);
		//VectorMA (v, p->size*p->ramp, vright, v);
		glVertex3fv (v);
	}

	//Draw fire particles
	//size = 25;
	for (p=fire ; p ; p=p->next)
	{
		VectorSubtract (p->org, r_origin, v);
		if (Length (v) > r_particle_dist.value*3)
		{
			glColor4f (p->colour[0]*p->ramp,p->colour[1]*p->ramp,p->colour[2]*p->ramp,p->ramp);

			glTexCoord2f (0, 1);
			VectorMA (p->org, -p->size, vup, v);
			//VectorMA (v, -p->size, vright, v);
			glVertex3fv (v);

			glTexCoord2f (0, 0);
			VectorMA (p->org, p->size, vup, v);
			//VectorMA (v, -p->size, vright, v);
			glVertex3fv (v);

			glTexCoord2f (1, 0);
			VectorMA (p->org, p->size, vup, v);
			//VectorMA (v, p->size, vright, v);
			glVertex3fv (v);

			glTexCoord2f (1, 1);
			VectorMA (p->org, -p->size, vup, v);
			//VectorMA (v, p->size, vright, v);
			glVertex3fv (v);
		}
	}

	//Draw chunks particles
	size=1;
	for (p=chunks ; p ; p=p->next)
	{
		glColor4f (p->colour[0]*p->ramp,p->colour[1]*p->ramp,p->colour[2]*p->ramp,p->ramp);

		glTexCoord2f (0, 1);
		VectorMA (p->org, -size, vup, v);
		//VectorMA (v, -size, vright, v);
		glVertex3fv (v);

		glTexCoord2f (0, 0);
		VectorMA (p->org, size, vup, v);
		//VectorMA (v, -size, vright, v);
		glVertex3fv (v);

		glTexCoord2f (1, 0);
		VectorMA (p->org, size, vup, v);
		//VectorMA (v, size, vright, v);
		glVertex3fv (v);

		glTexCoord2f (1, 1);
		VectorMA (p->org, -size, vup, v);
		//VectorMA (v, size, vright, v);
		glVertex3fv (v);
	}
	glEnd ();

	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//GL_Bind(smoke_tex);
	glBindTexture (GL_TEXTURE_2D, smoke_tex);
	glBegin (GL_QUADS);
	//Draw smoke particles
	for (p=smoke ; p ; p=p->next)
	{
		VectorSubtract (p->org, r_origin, v);
		if (Length (v) > r_particle_dist.value)
		{
			glColor4f (p->colour[0],p->colour[1],p->colour[2],p->ramp/4);
			glTexCoord2f (0, 1);
			VectorMA (p->org, -p->size*(2-p->ramp), vup, v);
			//VectorMA (v, -p->size*(2-p->ramp), vright, v);
			glVertex3fv (v);

			glTexCoord2f (0, 0);
			VectorMA (p->org, p->size*(2-p->ramp), vup, v);
			//VectorMA (v, -p->size*(2-p->ramp), vright, v);
			glVertex3fv (v);

			glTexCoord2f (1, 0);
			VectorMA (p->org, p->size*(2-p->ramp), vup, v);
			//VectorMA (v, p->size*(2-p->ramp), vright, v);
			glVertex3fv (v);

			glTexCoord2f (1, 1);
			VectorMA (p->org, -p->size*(2-p->ramp), vup, v);
			//VectorMA (v, p->size*(2-p->ramp), vright, v);
			glVertex3fv (v);
		}
	}
	glEnd ();

	//GL_Bind(trail_tex);
	glBindTexture (GL_TEXTURE_2D, GetTex(4 ));//trail_tex);//ZING

	//GL_Bind(particle_tex);
	glBegin (GL_QUADS);
	//Draw trails
	for (p=trails ; p ; p=p->next)
	{
		glColor4f (p->colour[0],p->colour[1],p->colour[2],p->ramp);

		glTexCoord2f (0, 1);
		VectorMA (p->org, p->size, vup, v);
		VectorMA (p->org, p->size, vright, v);
		glVertex3fv (v);

		glTexCoord2f (0, 0);
		//VectorMA (p->org, p->size, vup, v);
		VectorMA (p->org, -p->size, vright, v);
		glVertex3fv (v);

		glTexCoord2f (1, 0);
		//VectorMA (p->org2, -p->size, vup, v);
		VectorMA (p->org2, -p->size, vright, v);
		glVertex3fv (v);

		glTexCoord2f (1, 1);
		VectorMA (p->org2, p->size, vup, v);
		VectorMA (p->org2, p->size, vright, v);
		glVertex3fv (v);

	// reverse texture - for upside down trails, backwards textures are culled neway.
		glTexCoord2f (0, 1);
		VectorMA (p->org, p->size, vup, v);
		VectorMA (p->org2, p->size, vright, v);
		glVertex3fv (v);

		glTexCoord2f (0, 0);
		//VectorMA (p->org, p->size, vup, v);
		VectorMA (p->org2, -p->size, vright, v);
		glVertex3fv (v);

		glTexCoord2f (1, 0);
		//VectorMA (p->org2, -p->size, vup, v);
		VectorMA (p->org, -p->size, vright, v);
		glVertex3fv (v);

		glTexCoord2f (1, 1);
		VectorMA (p->org2, p->size, vup, v);
		VectorMA (p->org, p->size, vright, v);
		glVertex3fv (v);
	}
	glEnd();

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_COLOR);

	/*
	GL_Bind(lightning_tex);
	glBegin (GL_QUADS);
	//Draw trails
	for (p=lightning ; p ; p=p->next)
	{
		glColor4f (p->colour[0],p->colour[1],p->colour[2],p->ramp);

		glTexCoord2f (0, 1);
		VectorMA (p->org, p->size, vup, v);
		VectorMA (p->org, p->size, vright, v);
		glVertex3fv (v);

		glTexCoord2f (0, 0);
		//VectorMA (p->org, p->size, vup, v);
		VectorMA (p->org, -p->size, vright, v);
		glVertex3fv (v);

		glTexCoord2f (1, 0);
		//VectorMA (p->org2, -p->size, vup, v);
		VectorMA (p->org2, -p->size, vright, v);
		glVertex3fv (v);

		glTexCoord2f (1, 1);
		VectorMA (p->org2, p->size, vup, v);
		VectorMA (p->org2, p->size, vright, v);
		glVertex3fv (v);

	// reverse texture - for upside down trails, backwards textures are culled neway.
		glTexCoord2f (0, 1);
		VectorMA (p->org, p->size, vup, v);
		VectorMA (p->org2, p->size, vright, v);
		glVertex3fv (v);

		glTexCoord2f (0, 0);
		//VectorMA (p->org, p->size, vup, v);
		VectorMA (p->org2, -p->size, vright, v);
		glVertex3fv (v);

		glTexCoord2f (1, 0);
		//VectorMA (p->org2, -p->size, vup, v);
		VectorMA (p->org, -p->size, vright, v);
		glVertex3fv (v);

		glTexCoord2f (1, 1);
		VectorMA (p->org2, p->size, vup, v);
		VectorMA (p->org, p->size, vright, v);
		glVertex3fv (v);
	}
	glEnd();
	*/
}


//p_sparks, p_smoke, p_smoke_fade, p_fire, p_blood, p_chunks

extern cvar_t temp1;

extern cvar_t r_autowindx;
extern cvar_t r_autowindy;
extern cvar_t r_autowindz;


#define OFFSET \
	p->org[0] += (r_autowindx.value * 0.5);\
	p->org[1] += (r_autowindy.value * 0.5);\
	p->org[2] += (r_autowindz.value * 0.5);


void R_UpdateAll(void)
{
	xparticle_t		*p, *kill;
	float			grav, frametime, dist;
	int				j;

	vec3_t			oldOrg, stop, normal;

	//Work out gravity and time
	frametime = (cl.time - cl.oldtime)*1.5;
	grav = frametime * 9.8*(sv_gravity.value/800);

	for (j=p_sparks;j<=p_chunks;j++)
	{
		if (j==p_smoke_fade) 
			j++;

		switch (j)
		{
		case(p_smoke):
			p=smoke;
			break;
		case(p_sparks):
			p=sparks;
			break;
		case(p_fire):
			p=fire;
			break;
		case(p_blood):
			p=blood;
			break;
		case(p_chunks):
			p=chunks;
			break;
		}

		for (; p; p=p->next){
			//Kill off dead particles
			for (kill = p->next ;kill; kill = p->next)
			{
				OFFSET;

				if (kill->die <= cl.time|| kill->size > 150|| kill->size <= 0) // || CONTENTS_SOLID == SV_PointContents (kill->org)
				{
					p->next = kill->next;
					kill->next = xfree_particles;
					xfree_particles = kill;
					//if (j==p_fire)
					//	AddParticle(kill->org,1,4,1,p_smoke);
					xnumParticles--;
					continue;
				}
				break;
			}
		}
	}

	for (p=smoke; p; p=p->next){
	//Update smoke movement
		VectorCopy(p->org, oldOrg);
		
		p->org[0] += p->vel[0]*frametime + (rand()%4-1.5)/16*frametime;
		p->org[1] += p->vel[1]*frametime + (rand()%4-1.5)/16*frametime;
		p->org[2] += p->vel[2]*frametime + (rand()%4-1.5)/16*frametime;
		//p->vel[2] += grav;

		TraceLineN(oldOrg, p->org, stop, normal);
		//if ((stop != p->org)&&(VLength(stop)!=0))
		if ((stop != p->org)&&(Length(stop)!=0))
			p->die = 0;

		if (p->type) 
		{
			if (cl.time > p->start)
				p->type=0;
			p->ramp = 1+(1-(cl.time-p->delay)/(p->start-p->delay));
		}else{
			p->ramp = (1-(cl.time-p->start)/(p->die-p->start));
			p->size += p->ramp/16;
		}
		
		OFFSET;

		//Kill off dead particles
		for (kill = p->next ;kill; kill = p->next)
		{
			if (kill->die <= cl.time || kill->size > 150|| kill->size <= 0)
			{
				p->next = kill->next;
				kill->next = xfree_particles;
				xfree_particles = kill;
				xnumParticles--;
				continue;
			}
			break;
		}
	}

	for (p=fire; p; p=p->next){
	//Update fire movement
		VectorCopy(p->org, oldOrg);
		
		p->org[0] += p->vel[0]*frametime;
		p->org[1] += p->vel[1]*frametime;
		p->org[2] += p->vel[2]*frametime;

		TraceLineN(oldOrg, p->org, stop, normal);
		//if ((stop != p->org)&&(VLength(stop)!=0))
		if ((stop != p->org)&&(Length(stop)!=0))
		{
			dist = DotProduct(p->vel, normal) * -2;
			
			VectorMA(p->vel, dist, normal, p->vel);
			VectorCopy(stop, p->org);
		}

		p->ramp = (1-((cl.time-p->start)/(p->die-p->start))*0.8);

		OFFSET;


	//Kill off dead particles
		for (kill = p->next ;kill; kill = p->next)
		{
			if (kill->die <= cl.time)
			{
				p->next = kill->next;
				kill->next = xfree_particles;
				xfree_particles = kill;
				AddParticle(kill->org,1,4,1,p_smoke);
				xnumParticles--;
				continue;
			}
			break;
		}
	}

	for (p=chunks; p; p=p->next){
	//Update chunk movement
		VectorCopy(p->org, oldOrg);
		p->org[0] += p->vel[0]*frametime;
		p->org[1] += p->vel[1]*frametime;
		p->org[2] += p->vel[2]*frametime;
		p->vel[2] -= grav*8;

		TraceLineN(oldOrg, p->org, stop, normal);
		//if ((stop != p->org)&&(VLength(stop)!=0))
		if ((stop != p->org)&&(Length(stop)!=0))
		{
			dist = DotProduct(p->vel, normal) * -1.3;
			
			VectorMA(p->vel, dist, normal, p->vel);
			VectorCopy(stop, p->org);
		}

		p->ramp = (1-(cl.time-p->start)/(p->die-p->start));

		OFFSET;


	//Kill off dead particles
		for (kill = p->next;kill;kill = p->next)
		{
			if (kill->die <= cl.time)
			{
				p->next = kill->next;
				kill->next = xfree_particles;
				xfree_particles = kill;
				xnumParticles--;
				continue;
			}
			break;
		}
	}

	for (p=sparks; p; p=p->next){
	//Update spark movement
		VectorCopy(p->org, oldOrg);
		p->org[0] += p->vel[0]*frametime;
		p->org[1] += p->vel[1]*frametime;
		p->org[2] += p->vel[2]*frametime;
		p->vel[2] -= grav*8;

		TraceLineN(oldOrg, p->org, stop, normal);
		//if ((stop != p->org)&&(VLength(stop)!=0))
		if ((stop != p->org)&&(Length(stop)!=0))
		{
			dist = DotProduct(p->vel, normal) * -1.5;
			
			VectorMA(p->vel, dist, normal, p->vel);
			VectorCopy(stop, p->org);
		}

		OFFSET;

		p->ramp = (1-(cl.time-p->start)/(p->die-p->start));

	//Kill off dead particles
		for (kill = p->next ;kill; kill = p->next)
		{
			if (kill->die <= cl.time)
			{
				p->next = kill->next;
				kill->next = xfree_particles;
				xfree_particles = kill;
				xnumParticles--;
				continue;
			}
			break;
		}
	}

	for (p=blood; p; p=p->next){
	//Update blood movement
		if (p->hit == 0){
			VectorCopy(p->org, oldOrg);
			p->org[0] += p->vel[0]*frametime;
			p->org[1] += p->vel[1]*frametime;
			p->org[2] += p->vel[2]*frametime;
			p->vel[2] -= grav/4;

			TraceLineN(oldOrg, p->org, stop, normal);
			//if ((stop != p->org)&&(VLength(stop)!=0))

			if ((stop != p->org)&&(Length(stop)!=0))
			{
				dist = DotProduct(p->vel, normal) * -0.1;
				
				VectorMA(p->vel, dist, normal, p->vel);
				VectorCopy(stop, p->org);
				//if (VLength(p->vel)<0.2)
				if (Length(p->vel)<0.2)
					p->hit=1;
			}
			OFFSET;

		}

		p->ramp = (1-(cl.time-p->start)/(p->die-p->start));

	//Kill off dead particles
		for (kill = p->next ;kill; kill = p->next)
		{
			if (kill->die <= cl.time)
			{
				p->next = kill->next;
				kill->next = xfree_particles;
				xfree_particles = kill;
				xnumParticles--;
				continue;
			}
			break;
		}
	}

	for (p=trails; p; p=p->next){
		if (p->type == pt_grav){
			p->vel[2] -= grav*10;
		}
		VectorCopy(p->org, oldOrg);
		p->org[0] += p->vel[0]*frametime;
		p->org[1] += p->vel[1]*frametime;
		p->org[2] += p->vel[2]*frametime;
		TraceLineN(oldOrg, p->org, stop, normal);
		if ((stop != p->org)&&(Length(stop)!=0))
		{
			dist = DotProduct(p->vel, normal) * -0.1;
			
			VectorMA(p->vel, dist, normal, p->vel);
			VectorCopy(stop, p->org);
			if (Length(p->vel)<0.2)
				p->hit=1;
		}
		
		VectorCopy(p->org2, oldOrg);
		p->org2[0] += p->vel[0]*frametime;
		p->org2[1] += p->vel[1]*frametime;
		p->org2[2] += p->vel[2]*frametime;
		TraceLineN(oldOrg, p->org2, stop, normal);
		if ((stop != p->org2)&&(Length(stop)!=0))
		{
			dist = DotProduct(p->vel, normal) * -0.1;
			
			VectorMA(p->vel, dist, normal, p->vel);
			VectorCopy(stop, p->org2);
			if (Length(p->vel)<0.2)
				p->hit=1;
		}
		p->ramp = (1-(cl.time-p->start)/(p->die-p->start));

				OFFSET;


	//Kill off dead particles
		for (kill = p->next ;kill; kill = p->next)
		{

			if (kill->size > 200 || kill->size < 0)
				Con_Printf("bad particle %d\n", kill->size);//XFX

			if (kill->die <= cl.time)
			{
				p->next = kill->next;
				kill->next = xfree_particles;
				xfree_particles = kill;
				xnumParticles--;
				continue;
			}
			break;
		}
	}

}

extern qboolean SV_RecursiveHullCheck (hull_t *hull, int num, float p1f, float p2f, vec3_t p1, vec3_t p2, trace_t *trace);
/** TraceLineN
 * same as the TraceLine but returns the normal as well
 * which is needed for bouncing particles
 */

extern cvar_t gl_particledumb;
void TraceLineN (vec3_t start, vec3_t end, vec3_t impact, vec3_t normal)
{
	trace_t	trace;

	memset (&trace, 0, sizeof(trace));
	if (gl_particledumb.value) {
		VectorCopy(end,impact);
		VectorCopy(vec3_origin,normal);
	}
	else	
	{
		SV_RecursiveHullCheck (cl.worldmodel->hulls, 0, 0, 1, start, end, &trace);

		VectorCopy (trace.endpos, impact);
		VectorCopy (trace.plane.normal, normal);
	}
}

/*
Fire: Explosion, Rocket, Flame
Smoke: Rocket, Gren trail, Explosion smoke
Sparks: Explosion, Gunshot
Chunks: Gunshot
Blood: Gunshot, trail
Enviromental: Snow, Rain
Trails: Rail trail, Lightning(?)
*/

void R_FlameSmokes (vec3_t org)
{
	AddParticle(org, 1, 0.2f, 4, p_smoke);
}

/** AddParticle
 * Just calls AddParticleColor with the default color variable for that particle
 */
void AddParticle(vec3_t org, int count, float size, float time, int type)
{
	//Times: smoke=3, spark=2, blood=3, chunk=4
	vec3_t	colour;

	switch (type)
	{
	case (p_smoke):
		colour[0] = colour[1] = colour[2] = (rand()%255)/255.0;
		break;
	case (p_sparks):
		colour[0] = 1;
		colour[1] = 0.5;
		colour[2] = 0;
		break;
	case (p_fire):
		colour[0] = 1;
		colour[1] = 0.55f;
		colour[2] = 0;
		break;
	case (p_blood):
		colour[0] = (rand()%64)/256.0;
		colour[1] = 0;
		colour[2] = 0;
		break;
	case (p_chunks):
		colour[0] = colour[1] = colour[2] = (rand()%182)/255.0;
	}		

	AddParticleColor(org, count, size, time, type, colour);
}

//same as addparticle with a colour var

/** AddParticleColor
 * This is where it all happends, well not really this is where
 * most of the particles (and soon all) are added execpt for trails
 */
void AddParticleColor(vec3_t org, int count, float size, float time, int type, vec3_t colour)
{
	xparticle_t		*p;
	vec3_t			stop, normal;
	int				i;

	// don't draw if paused or in the console, or otherwise
	if (key_dest != key_game) return;

	for (i=0 ; (i<count)&&(xfree_particles) ; i++)
	{
		//get the next free particle off the list
		p = xfree_particles;
		//reset the next free particle to the next on the list
		xfree_particles = p->next;
		//add the particle to the correct one
		p->size = size + (((rand()%50)*size)/50-(size/2));
		switch (type)
		{
		case (p_fire):
			p->next = fire;
			fire = p;

			//pos
			p->org[0] = org[0] + ((rand()%80)-40)*(size/25);
			p->org[1] = org[1] + ((rand()%80)-40)*(size/25);
			p->org[2] = org[2] + ((rand()%80)-40)*(size/25);
			TraceLineN(org, p->org, stop, normal);
			//if (VLength(stop) != 0)
			if (Length(stop) != 0)
				VectorCopy(stop, p->org);

			//velocity
			p->vel[0] = ((rand()%40)-20)*(size/25);
			p->vel[1] = ((rand()%40)-20)*(size/25);
			p->vel[2] = ((rand()%40)-20)*(size/25);
			break;

		case (p_sparks):
			p->next = sparks;
			sparks = p;

			p->size = 1;
			//pos
			VectorCopy(org, p->org);
			//velocity
			p->vel[0] = (rand()%(int)size)-((int)size/2);
			p->vel[1] = (rand()%(int)size)-((int)size/2);
			p->vel[2] = (rand()%(int)size)-((int)size/2);
			break;

		case (p_smoke):
			p->next = smoke;
			smoke = p;

			//pos
			p->org[0] = org[0] + lhrandom(-3,3);
			p->org[1] = org[1] + lhrandom(-3,3);
			p->org[2] = org[2] + lhrandom(-3,3);
			TraceLineN(org, p->org, stop, normal);
			//if (VLength(stop) != 0)
			if (Length(stop) != 0)
				VectorCopy(stop, p->org);

			//velocity
			p->vel[0] = lhrandom(-3,3);
			p->vel[1] = lhrandom(-3,3);
			p->vel[2] = lhrandom(-3,3);
			break;

		case (p_blood):
			p->next = blood;
			blood = p;
			//pos
			p->org[0] = org[0] + ((rand()%30)-15)/2;
			p->org[1] = org[1] + ((rand()%30)-15)/2;
			p->org[2] = org[2] + ((rand()%30)-15)/2;
			TraceLineN(org, p->org, stop, normal);
			//if (VLength(stop) != 0)
			if (Length(stop) != 0)
				VectorCopy(stop, p->org);

			//velocity
			p->vel[0] = (rand()%40)-20;
			p->vel[1] = (rand()%40)-20;
			p->vel[2] = (rand()%40)-20;
			break;

		case (p_chunks):
			p->next = chunks;
			chunks = p;
			//pos
			VectorCopy(org, p->org);
			//velocity
			p->vel[0] = (rand()%20)-10;
			p->vel[1] = (rand()%20)-10;
			p->vel[2] = (rand()%20)-15;
			break;
		}

		p->hit = 0;
		p->start = cl.time;
		p->die = cl.time + time + (rand()%25)/25.0;
		p->type = 0;
		VectorCopy(colour, p->colour);

		xnumParticles++;
	}
}

/** DelayAddChunk
 * This is to do effects like max payne where the chunks from
 * a bullet hole are created from there for a few seconds
 * basicly it will slowly add chunks over a period of time
 */
void DelayAddChunk(vec3_t org, int count, vec3_t colour, float time)
{
	//as you can see its not here yet
	//i hope to get it in by the next version
}

// use particles for zerstorer rain rather than sprites (faster)
void R_ZerstorerRain (vec3_t org)
{
	vec3_t colour;
	colour[0]=colour[1]=colour[2]=1.0f;
	AddParticleColor(org, 16, 0, 1, p_chunks, colour);
}




/** AddEnv
 * This is ment to add enviroment effects when its
 * finished but currently its not working ill add it soon
 * -note there is the psudo code of how it will work there
 * if you do implemnt it send me the code
 */
void AddEnv(vec3_t org, vec3_t org2, int count)
{
	//From top corner (org) to bottom corner (org2)
	//(org2 - org)*(1,0,0)->A
	//(org2 - org)*(0,1,0)->B
	//for i=0 to count
		//add effeect at: org + rand()*A + rand()*B
	//set take away effect when it gets below org2[3]

}

/** AddTrail
 * Calls AddTrailColor with the default colours
 */
void AddTrail(vec3_t start, vec3_t end, int type, float time, float size)
{
	vec3_t colour;

	//colour set when first update called (check if needs to be fixed)
	switch (type)
	{
	case (p_smoke):
		colour[0] = colour[1] = colour[2] = (rand()%128)/128.0+128;
		break;
	case (p_sparks):
		colour[0] = 1;
		colour[1] = 0.5;
		colour[2] = 0;
		break;
	case (p_fire):
		colour[0] = 1;
		colour[1] = 0.55f;
		colour[2] = 0;
		break;
	case (p_blood):
		colour[0] = 0.8f;
		colour[1] = 0;
		colour[2] = 0;
		break;
	case (p_chunks):
		colour[0] = colour[1] = colour[2] = (rand()%182)/255.0;
	}
	AddTrailColor(start, end, type, time, size, colour);
}

/** AddTrailColor
 * This will add a trail of particles of the specified type.
 * The particles may need thining but ill get some feed back
 * on it first
 */
void AddTrailColor(vec3_t start, vec3_t end, int type, float time, float size, vec3_t color)
{
	xparticle_t		*p;
	int				i;
	float			count;
	vec3_t			point;

	//work out vector for trail
	VectorSubtract(start, end, point);
	//work out the length and therefore the amount of particles
	count = Length(point);
	//make sure its at least 1 long
	//quater the amount of particles (speeds it up a bit)
	if (count == 0)
		return;
	else
		count = count/4;

	if (((type == p_blood)||(type == p_smoke)/*||(type == p_lightning)*/)&&(xfree_particles)){
		
		p = xfree_particles;
		xfree_particles = p->next;
		p->next = trails;
		trails = p;

		//Con_Printf("hello world\n");

		VectorCopy(start, p->org);
		VectorCopy(end, p->org2);

		p->die = cl.time + time + 0.1;
		p->size = 2;

		VectorCopy(color, p->colour);
		p->hit = 0;
		p->start = cl.time;
		p->vel[0] = 0;
		p->vel[1] = 0;
		p->vel[2] = 0;

		xnumParticles++;
		count /= 2;
		if ((type == p_blood)||(type == p_lightning)){
			p->type = pt_grav;
			return;
		} else
			p->type = pt_static;
	}

	//the vector from the current pos to the next particle
	VectorScale(point, 1.0/count, point);

	for (i=0 ; (i<count)&&(xfree_particles) ; i++)
	{
		//set the current particle to the next free particle
		p = xfree_particles;
		//reset the free particle pointer to the now first free particle
		xfree_particles = p->next;

		//small starting velocity
		p->vel[0]=((rand()%16)-8)/2;
		p->vel[1]=((rand()%16)-8)/2;
		p->vel[2]=((rand()%16)-8)/2;

		//add the particle to the correct one
		switch (type)
		{
		case (p_fire):
			p->next = fire;
			fire = p;
			p->size = size;
			break;
		case (p_sparks):
			//need bigger starting velocity (affected by grav)
			p->vel[0]=((rand()%32)-16)*2;
			p->vel[1]=((rand()%32)-16)*2;
			p->vel[2]=((rand()%32))*2;

			p->next = sparks;
			sparks = p;
			p->size = size;
			break;
		case (p_smoke):
			p->next = smoke;
			smoke = p;
			p->size = size;
			break;
		case (p_blood):
			p->next = blood;
			blood = p;
			p->size = size; //1.5 * (rand()%20)/10;
			break;
		case (p_chunks):
			p->next = chunks;
			chunks = p;
			p->size = size;
			break;
		}

		//reset the particle vars
		p->hit = 0;
		p->start = cl.time;
		p->die = cl.time + time;
		

		VectorCopy(color, p->colour);

		//work out the pos
		VectorMA (start, i, point, p->org);

		//make it a bit more random
		p->org[0] += ((rand()%16)-8)/8;
		p->org[1] += ((rand()%16)-8)/8;
		p->org[2] += ((rand()%16)-8)/8;

		xnumParticles++;
	}
}

#define MAXDISTANCE 30 //default for SPX is 30

void AddTrailTrail(vec3_t start, vec3_t end, float time, float size, vec3_t color)
{
	xparticle_t		*p;
//	int				i;
	float			count;
	vec3_t			point;

	//work out vector for trail
	VectorSubtract(start, end, point);
	
	//work out the length and therefore the amount of particles
	count = Length(point);
	
	//make sure its at least 1 long
	//quater the amount of particles (speeds it up a bit)
	if (count == 0)
		return;
	else
		count = count/4;

	if (count>MAXDISTANCE)
		return;

	if (!xfree_particles)
		return;

	p = xfree_particles;
	xfree_particles = p->next;
	p->next = trails;
	trails = p;

		//Con_Printf("hello world\n");

	VectorCopy(start, p->org);
	VectorCopy(end, p->org2);

	p->die = cl.time + time;
	p->size = size;

	VectorCopy(color, p->colour);
	p->hit = 0;
	p->start = cl.time;
	
	p->vel[0] = 0;
	p->vel[1] = 0;
	p->vel[2] = 0;


	xnumParticles++;
	count /= 2;
	p->type = pt_static;

	//the vector from the current pos to the next particle
	VectorScale(point, 1.0/count, point);

}


void AddTrailTrail2(vec3_t start, vec3_t end, float time, float size, vec3_t color)
{
	xparticle_t		*p;
//	int				i;
	float			count;
	vec3_t			point;

	//work out vector for trail
	VectorSubtract(start, end, point);
	
	//work out the length and therefore the amount of particles
	count = Length(point);
	
	//make sure its at least 1 long
	//quater the amount of particles (speeds it up a bit)
	if (count == 0)
		return;
	else
		count = count/4;


	if (!xfree_particles)
		return;

	p = xfree_particles;
	xfree_particles = p->next;
	p->next = trails;
	trails = p;

		//Con_Printf("hello world\n");

	VectorCopy(start, p->org);
	VectorCopy(end, p->org2);

	p->die = cl.time + time;
	p->size = size;

	VectorCopy(color, p->colour);
	p->hit = 0;
	p->start = cl.time;
	
	p->vel[0] = 0;
	p->vel[1] = 0;
	p->vel[2] = 0;


	xnumParticles++;
	count /= 2;
	p->type = pt_static;

	//the vector from the current pos to the next particle
	VectorScale(point, 1.0/count, point);

}


void AddFadeSmoke(vec3_t org, int count)
{
	xparticle_t		*p;
	int				i;

	for (i=0 ; (i<count)&&(xfree_particles) ; i++)
	{
		p = xfree_particles;
		xfree_particles = p->next;
		p->next = smoke;
		smoke = p;

		p->hit = 0;
		p->delay = cl.time;
		p->start = cl.time + 0.1;
		p->die = cl.time + 1.1;
		p->type = 1;
		p->size = 4;

		p->ramp = 2;

		p->colour[0] = p->colour[1] = p->colour[2] = (rand()%255)/255.0;

		p->org[0] = org[0];
		p->org[1] = org[1];
		p->org[2] = org[2];
		p->vel[0] = ((rand()%10)-5)/20;
		p->vel[1] = ((rand()%10)-5)/20;
		p->vel[2] = ((rand()%10)-5)/20;
	}
}


/** MakeParticleTexture
 * Makes the particle textures only 2 the 
 * smoke texture (which could do with some work and
 * the others which is just a round circle.
 *
 * I should really make it an alpha texture which would save 32*32*3
 * bytes of space but *shrug* it works so i wont stuff with it
 */


void MakeParticleTexure(void)
{
    int i, j, k, centreX, centreY, separation, max;
    byte	data[128][128][4];

	//Normal texture (0->256 in a circle)
	//If you change this texture you will change the textures of
	//all particles except for smoke (other texture) and the
	//sparks which dont use textures
    for(j=0;j<128;j++){
        for(i=0;i<128;i++){
			data[i][j][0]	= 255;
			data[i][j][1]	= 255;
			data[i][j][2]	= 255;
            separation = (int) sqrt((64-i)*(64-i) + (64-j)*(64-j));
            if(separation<63)
                data[i][j][3] = 255 - separation * 256/63;
            else data[i][j][3] =0;
        }
    }
	//Load the texture into vid mem and save the number for later use
	part_tex = GL_LoadTexture ("particlezz", 128, 128, &data[0][0][0], true, true, 4);

	//Smoke texture (32 * 4 unit circles)
	//This needs a bit of work its still a bit square but it will do for now

	//Clear the data
	max=(int)(sqrt((64-0)*(64-0) + (64-0)*(64-0)));
    for(j=0;j<128;j++){
        for(i=0;i<128;i++){
			data[i][j][0]	= 255;
			data[i][j][1]	= 255;
			data[i][j][2]	= 255;
			separation = (int) sqrt((64-i)*(64-i) + (64-j)*(64-j));
			data[i][j][3] = (max - separation)*2;
        }
    }
	
	//Add 128 random 4 unit circles
	for(k=0;k<128;k++){
		centreX = rand()%122+3;
		centreY = rand()%122+3;
		for(j=-3;j<3;j++){
			for(i=-3;i<3;i++){
				separation = (int) sqrt((i*i) + (j*j));
				if (separation <= 5)
					data[i+centreX][j+centreY][3]	+= (5-separation);
			}
	    }
	}
	smoke_tex = GL_LoadTexture ("smoke_part", 128, 128, &data[0][0][0], false, true, 4);

	//Load the texture and save the number for later use
	//smoke_tex = loadtextureimage ("textures/smoke", 128, 128, false, true);

	//Clear the data
	max=64;
    for(j=0;j<128;j++){
        for(i=0;i<128;i++){
			data[i][j][0]	= 255;
			data[i][j][1]	= 255;
			data[i][j][2]	= 255;
			separation = (int) sqrt((i - 64)*(i - 64));
			data[i][j][3] = (max - separation)*2; 
		}
    }
	
	//Add 128 random 4 unit circles
	for(k=0;k<128;k++){
		centreX = rand()%122+3;
		centreY = rand()%122+3;
		for(j=-3;j<3;j++){
			for(i=-3;i<3;i++){
				separation = (int) sqrt((i*i) + (j*j));
				if (separation <= 5)
					data[i+centreX][j+centreY][3]	+= (5-separation);
			}
	    }
	}
	trail_tex = GL_LoadTexture ("trail_part", 128, 128, &data[0][0][0], false, true, 4);

//	lightning_tex = loadtextureimage ("textures/lightning", 0, 0, false, true);

	free(data);
}
