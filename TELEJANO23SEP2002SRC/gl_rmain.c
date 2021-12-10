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
// r_main.c

#include "quakedef.h"
#include "gl_mirror.h" // add mh's mirror header file

// precalculated dot products for quantized angles
extern float	r_avertexnormal_dots_mdl[16][256];
extern float	r_avertexnormal_dots_md2[16][256];

extern float	*shadedots_mdl, *shadedots2_mdl;
extern float	*shadedots_md2, *shadedots2_md2;

float   lightlerpoffset;

int		lastposenum;
int		lastposenum0;
void	R_LightPoint (vec3_t p);
int		playertextures;		// up to 16 color translated skins
int		c_brush_polys, c_alias_polys;
int		r_visframecount;	// bumped when going to a new PVS
int		r_framecount;		// used for dlight push checking
int		d_lightstylevalue[256];	// 8.8 fraction of base light value

extern	vec3_t	lightcolor;
extern	vec3_t	lightspot;

entity_t	r_worldentity;
entity_t	*currententity;

// view origin
vec3_t	vup;
vec3_t	vpn;
vec3_t	vright;
vec3_t	r_origin;
vec3_t	modelorg;

// screen size info
refdef_t	r_refdef;
mleaf_t		*r_viewleaf, *r_oldviewleaf;
mplane_t	frustum[4];
texture_t	*r_notexture_mip;
void R_DrawHudModel(entity_t * self);//Tei

// these are the only functions which are used in this file, rest is in gl_md2 and gl_mdl

void	GL_DrawAliasBlendedFrame (int frame, aliashdr_t *paliashdr, entity_t* e);
void	GL_DrawAliasBlendedFrame2 (int frame, aliashdr_t *paliashdr, entity_t* e);
void	R_SetupQ2AliasFrame (int frame, md2_t *pheader, entity_t *e);
void	GL_DrawAliasBlendedShadow (aliashdr_t *paliashdr, int pose1, int pose2, entity_t* e);
void	R_SetupBrushPolys (entity_t *e);
void	R_AnimateLight (void);
void	V_CalcBlend (void);
void	R_DrawSpriteModel (entity_t *e);
void	R_DrawWorld (void);
void	R_DrawWaterSurfaces (void);
void	R_DrawParticles (qboolean waterpart);
void	R_MarkLeaves (void);
void	R_DrawGlows(entity_t *e);
void DefineFlareColor(vec3_t origin, int radius, int mode, int alfa, float red, float green, float blue) ;//Tei
void R_ParticleImplosionXray (vec3_t origin);//Tei

void RenderLsigEnt (entity_t *e  );//Tei lsigh
void AddTrailTrail2(vec3_t start, vec3_t end, float time, float size, vec3_t color);
void AddTrailTrail(vec3_t start, vec3_t end, float time, float size, vec3_t color);

//void R_DrawNullModel (void);//Q2MAX!

cvar_t	r_norefresh			= {"r_norefresh","0"};
cvar_t	r_drawviewmodel		= {"r_drawviewmodel","1"};
cvar_t	r_speeds			= {"r_speeds","0"};
cvar_t	r_shadows			= {"r_shadows","1", true};// Tei default
cvar_t	r_wateralpha		= {"r_wateralpha","1", true};
cvar_t	r_dynamic			= {"r_dynamic","1"};
cvar_t	r_novis				= {"r_novis","0"};
cvar_t	gl_finish			= {"gl_finish","0", true};
cvar_t	gl_clear			= {"gl_clear","1", true};//Tei default (this help r_wave look)
cvar_t	gl_nocolors			= {"gl_nocolors","0"};
cvar_t	gl_keeptjunctions	= {"gl_keeptjunctions","1"};
cvar_t	gl_doubleeyes		= {"gl_doubleeys", "1"};
cvar_t  gl_fogenable		= {"gl_fogenable", "0", true};//Tei default
cvar_t  gl_fogstart			= {"gl_fogstart", "50.0", true}; 
cvar_t  gl_fogend			= {"gl_fogend", "800.0", true}; 
cvar_t  gl_fogred			= {"gl_fogred","0.6", true};
cvar_t  gl_foggreen			= {"gl_foggreen","0.5", true}; 
cvar_t  gl_fogblue			= {"gl_fogblue","0.4", true}; 
cvar_t	centerfade			= {"centerfade", "0", true};	// Tomaz - Fading CenterPrints
cvar_t	sbar_alpha			= {"sbar_alpha", "0", true};	// Tomaz - Sbar Alpha

cvar_t	sbar_weaponsalpha				= {"sbar_weaponsalpha	", "0.5", true};	// Tomaz - Sbar Alpha

														// Tei new default 0
cvar_t	con_alpha			= {"con_alpha", "0.5", true};	// Tomaz - Console Alpha
cvar_t	r_wave				= {"r_wave", "8", true};		// Tomaz - Water Wave
cvar_t	gl_glows			= {"gl_glows", "1", true};		// Tomaz - Glow
cvar_t	r_bobbing			= {"r_bobbing", "1", true};		// Tomaz - Bobbing Items
cvar_t	gl_envmap			= {"gl_envmap", "0", true};		// Tomaz - Enviroment Mapping
cvar_t	gl_caustics			= {"gl_caustics", "1", true};	// Tomaz - Underwater Caustics

cvar_t	gl_geocaustics			= {"gl_geocaustics", "1", true};	//Tei - Geo Caustics

cvar_t	gl_fbr				= {"gl_fbr", "1", true};		// Tomaz - Fullbrights
cvar_t	impaim				= {"impaim", "0", true};		// Tomaz - Improved Aiming
cvar_t	skybox_spin			= {"skybox_spin", "0", true};	// Tomaz - Spinning Skyboxes
cvar_t	mapshots			= {"mapshots", "0", true};		// Tomaz - MapShots
cvar_t	gl_showpolys		= {"gl_showpolys", "0"};		// Tomaz - Show BSP Polygons
cvar_t	gl_wireframe		= {"gl_wireframe", "0"};		// Tomaz - Draw World as Wireframe and Textures
cvar_t	gl_wireonly			= {"gl_wireonly", "0"};			// Tomaz - Draw World as Wireframe Only
cvar_t	gl_particles		= {"gl_particles", "1", true};	// Tomaz - Particles
cvar_t	print_center_to_console		= {"print_center_to_console", "0", true};		// Tomaz - Prints CenerString's to the console
cvar_t	gl_particle_fire	= {"gl_particle_fire", "1", true};		// Tomaz - Fire Particles
cvar_t	gl_particledumb	= {"gl_particledumb", "1", true};		// Tomaz - Fire Particles

cvar_t	watermap			= {"watermap", "0"};						// Tei - watermap
cvar_t	fpscroak			= {"fpscroak", "0"};						// Tei - fps control
cvar_t	r_staticfx			= {"r_staticfx", "1", false, false};		// Tei - staticfx
cvar_t	r_dosky				= {"r_dosky",	"1"};						//Tei fo sky
cvar_t	gl_cull				= {"gl_cull", "0", true};					//Q2!

cvar_t	gl_sun_z				= {"gl_sun_z", "4000", true};			//Tei sun
cvar_t	gl_sun_x				= {"gl_sun_x", "140", true};			//Tei sun
cvar_t	gl_sun_y				= {"gl_sun_y", "200", true};			//Tei sun
cvar_t	gl_sun				= {"gl_sun", "2", true};					//Tei sun

cvar_t  gl_emulatesoftwaremode = {"gl_emulatesoftwaremode","0",false};	//Tei emulated softwaremode															

cvar_t  gl_xrayblast		= {"gl_xrayblast","1",false};				//Tei x-ray blast for lightinggun
cvar_t  gl_smallparticles	= {"gl_smallparticles","1",false};			//Tei small particles (with nearest)
												
cvar_t gl_themefolder		= {"gl_themefolder","textures",false};	//Tei textures themes
cvar_t gl_decalfolder		= {"gl_decalfolder","mapmodel",false};	//Tei decal themes

cvar_t	gl_detail	= {"gl_detail", "0", true};		// CHP detail texture


cvar_t  gl_dither				= {"gl_dither","1", true}; //Tei force dither
cvar_t  gl_detailscale			= {"gl_detailscale","4", true}; //Tei detail scale
					   

cvar_t  mod_zwater			= {"mod_zwater","0", true}; //Tei zwater offset

   
/*
=================
R_CullBox

Returns true if the box is completely outside the frustom
=================
*/
qboolean R_CullBox (vec3_t mins, vec3_t maxs)
{
	if ((BoxOnPlaneSide (mins, maxs, &frustum[0]) == 2) ||
		(BoxOnPlaneSide (mins, maxs, &frustum[1]) == 2) ||
		(BoxOnPlaneSide (mins, maxs, &frustum[2]) == 2) ||
		(BoxOnPlaneSide (mins, maxs, &frustum[3]) == 2))
		return true;
	return false;
}



/*
=============
R_BlendedRotateForEntity
=============
*/
void R_BlendedRotateForEntity (entity_t *e, int shadow)	// Tomaz - New Shadow
{
	float timepassed;
	float blend;
	vec3_t d;
	int i;

	// positional interpolation

	timepassed = realtime - e->translate_start_time; 

	if (e->translate_start_time == 0 || timepassed > 1)
	{
		e->translate_start_time = realtime;
		VectorCopy (e->origin, e->origin1);
		VectorCopy (e->origin, e->origin2);
	}

	if (!VectorCompare (e->origin, e->origin2))
	{
		e->translate_start_time = realtime;
		VectorCopy (e->origin2, e->origin1);
		VectorCopy (e->origin,  e->origin2);
		blend = 0;
	}
	else
	{
		blend =  (timepassed * 10) * slowmo.value; 	// Tomaz - Speed

		if (cl.paused || blend > 1) blend = 1;
	}

	VectorSubtract (e->origin2, e->origin1, d);

	glTranslatef (
		e->origin1[0] + (blend * d[0]),
		e->origin1[1] + (blend * d[1]),
		e->origin1[2] + (blend * d[2]));

	// orientation interpolation (Euler angles, yuck!)

	timepassed = realtime - e->rotate_start_time; 

	if (e->rotate_start_time == 0 || timepassed > 1)
	{
		e->rotate_start_time = realtime;
		VectorCopy (e->angles, e->angles1);
		VectorCopy (e->angles, e->angles2);
	}

	if (!VectorCompare (e->angles, e->angles2))
	{
		e->rotate_start_time = realtime;
		VectorCopy (e->angles2, e->angles1);
		VectorCopy (e->angles,  e->angles2);
		blend = 0;
	}
	else
	{
		blend = (timepassed * 10) * slowmo.value;	// Tomaz - Speed
 
		if (cl.paused || blend > 1) blend = 1;
	}

	VectorSubtract (e->angles2, e->angles1, d);

	// always interpolate along the shortest path
	for (i = 0; i < 3; i++) 
	{
		if (d[i] > 180)
		{
			d[i] -= 360;
		}
		else if (d[i] < -180)
		{
			d[i] += 360;
		}
	}

	// Tomaz - New Shadow Begin
	glRotatef ( e->angles1[1] + ( blend * d[1]),  0, 0, 1);

	if (shadow==0)
	{
              glRotatef (-e->angles1[0] + (-blend * d[0]),  0, 1, 0);
              glRotatef ( e->angles1[2] + ( blend * d[2]),  1, 0, 0);
	}
	// Tomaz - New Shadow End

	glScalef  (e->scale, e->scale, e->scale);	// Tomaz - QC Scale
}
// Tomaz - Model Transform Interpolation End
/*
=================
R_RotateForEntity
functions used to draw the weapon models, so we don't have that nasty effect
=================
*/
void R_RotateForEntity (entity_t *e, int shadow)
{
    glTranslatef (e->origin[0],  e->origin[1],  e->origin[2]);

    glRotatef (e->angles[1],  0, 0, 1);

	if (shadow == 0)
	{
		glRotatef (-e->angles[0],  0, 1, 0);
		glRotatef (e->angles[2],  1, 0, 0);
	}

	glScalef  (e->scale, e->scale, e->scale);	// Tomaz - QC Scale
}

qboolean weaponmodel = false;

/*
=================
R_DrawAliasModel
Common function used both by mdl and md2, so we put it in gl_rmain
=================
*/

void DefineFlare(vec3_t origin, int radius, int mode, int alfa);//Tei flares
void R_DarkRocket (vec3_t start, int subtype);//Tei rocket 

void R_AlienBlood (vec3_t origin);
void R_AlienBloodExplosion (vec3_t origin);
void R_AlienBoltExplosion (vec3_t origin);
void R_DarkFire (entity_t *ent, qboolean fire2);


void R_RayFieldParticles ( vec3_t end);
void R_BeamSpike (vec3_t origin, vec3_t end);
void R_ParticleIstar (vec3_t origin);
//extern int bolt_tex;//tei 
extern int focus_tex;
extern int GetTex(int type);//Tei
extern int particle_tex;
extern cvar_t r_autofluor;//Tei autofluorsparks..

float TraceLine (vec3_t start, vec3_t end, vec3_t impact, vec3_t normal);
extern int GetTex(int type);//Tei get type
void DefineFlareColorTexnum(vec3_t origin, int radius, int mode, int alfa, float red, float green, float blue, int texnum);
void R_ParticleFire (vec3_t origin, int dx, int scale);

extern cvar_t extra_fx;

void R_ParticleBubbleGib (vec3_t origin);
int GetTex (int type ) ;

void MFX_Apply( entity_t * e)
{

	int i;
//	int			lnum;//, anim;
//	int			ef;//Tei speed
	float		add;
//	md2_t		*pheader; // Tomaz - Quake2 Models
	vec3_t	  downmove ,dumy;
	vec3_t	stop,normal,color,dest;//, start ;
	extern sfx_t *cl_sfx_foot;
	extern cvar_t temp1;
	entity_t * self;

//	model_t		*clmodel;
//	aliashdr_t	*paliashdr;

	dlight_t	*dl;

	float		dx;

			if (e->model->use_FIRE || e->model->effect == MFX_FIRE) {
				R_Fire(currententity, true);
				//Con_Printf("fire1\n");

				};
			if (e->model->use_FIRE2 || e->model->effect == MFX_FIRE2) {
				R_Fire(e, true);
				downmove[0] = e->origin[0] + lhrandom(-9,9);;
				downmove[1] = e->origin[1] + lhrandom(-9,9);;
				downmove[2] = e->origin[2] + lhrandom(-10,13);		
			
				i = lhrandom(6,16);
				R_ParticleFire(downmove,0,i);

				downmove[0] = e->origin[0] + lhrandom(-6,6);;
				downmove[1] = e->origin[1] + lhrandom(-6,6);;
				downmove[2] = e->origin[2] + lhrandom(-6,6);		
			
				i = lhrandom(6,10);
				R_ParticleFire(downmove,0,i);

				DefineFlareColorTexnum(e->origin, 40, 0, 10,199,139,139,GetTex(1));

				downmove[0] = 0;
				downmove[1] = 0;
				downmove[2] = 4;
				VectorAdd( e->origin, downmove, downmove);

				DefineFlareColor(downmove,60,0,80,150,0,0);
			
				
				};
			if (e->model->use_MISSILE || e->model->effect == MFX_MISSILE) {
				if (extra_fx.value > 1)
					R_DarkFire(e, true);// ( e->origin , 1 );
				
				if (extra_fx.value > 2)
				{
					downmove[0] = 0;
					downmove[1] = 0;
					downmove[2] = - 1;
					R_SparkShower (e->origin, downmove );
				}
				};
			if (e->model->use_FIRELAMP || e->model->effect == MFX_FIRELAMP) {
				//Con_Printf("fire3\n");
				downmove[0] = 0;
				downmove[1] = 0;
				downmove[2] = 4;
				VectorAdd( e->origin, downmove, downmove);
				//R_SparkShower (e->origin, downmove );
				R_Fire(e, false);

				if (lhrandom(0,250)<4)
					R_DarkRocket ( downmove , 1 );

				downmove[0] = 0;
				downmove[1] = 0;
				downmove[2] = 4;
				
				//Con_Printf("here\n");

				if (lhrandom(0,400)<4)
					R_SparkShower (e->origin, downmove );

				DefineFlareColor(e->origin,28,0,90,250,0,0);

				downmove[0] = 0;
				downmove[1] = 0;
				downmove[2] = 10;

				VectorAdd( e->origin, downmove, downmove);

				DefineFlareColor(downmove,90, 0,50,245,210,65);//Tei dp flare


				};
			if (e->model->use_BLUEFIRE2 || e->model->effect == MFX_BLUEFIRE2) {
				if (rand()&1)
					R_FireBlue(e, true);
				};//Yes this is correct, make bluefire2 blink
			if (e->model->use_BLUEFIRE || e->model->effect == MFX_BLUEFIRE) {
				R_FireBlue(e, true);
				};

			if (e->model->use_DOWFIRE || e->model->effect == MFX_DOWFIRE) {
				R_DowFire(e, true);	
				};

			if (e->model->use_ENGINEFIRE2 || e->model->effect == MFX_ENGINEFIRE2) {
				R_FireClassic(e, true);
				};

			if (e->model->use_BIGFIRE || e->model->effect == MFX_BIGFIRE) {
				R_BigFire(e, true);
				};

			if (e->model->use_FOGMAKER || e->model->effect == MFX_FOGMAKER) {
				if (rand()&1)
					R_FogSplash(e->origin);
				downmove[0] = 0;
				downmove[1] = 0;
				downmove[2] = 10;
				R_SparkShower (e->origin, downmove );
				};

			if (e->model->use_FOGMAKERLITE || e->model->effect == MFX_FOGMAKERLITE) {
				R_FogSplashLite(e->origin);
				};

			if (e->model->use_WATERFALL || e->model->effect == MFX_WATERFALL) {
				R_WaterFall(e, true);
				};

			if (e->model->use_SPARKSHOWER || e->model->effect == MFX_SPARKSHOWER) {
				downmove[0] = 0;
				downmove[1] = 0;
				downmove[2] = - 1;
				R_SparkShower (currententity->origin, downmove );
				};

			if (e->model->use_ALIENBLOOD || e->model->effect == MFX_ALIENBLOOD) {
				R_AlienBloodExplosion(e->origin);
				};

			if (e->model->use_BOLTFX || e->model->effect == MFX_BOLTFX) {
				
				//R_LightParticles(e->origin);

				if (teirand(100)<4)
					R_AlienBoltExplosion(e->origin);				
				//DefineFlare(e->origin,10, 0,50);
				//e->alpha = 0.1;
				};

			if (e->model->use_SUN || e->model->effect == MFX_SUN) {
				R_ParticleImplosionXray(e->origin);//Tei x-flare sun
				};

			if (e->model->use_LASER || e->model->effect == MFX_LASER) {		
				//R_Fire(currententity, true);
				VectorCopy(e->origin, downmove);
				downmove[0] += lhrandom(-2,2);
				downmove[1] += lhrandom(-2,2);
				downmove[2] += lhrandom(-2,2);

				DefineFlareColor(downmove,6, 0,90,255,100,100);
				DefineFlareColor(e->origin,18,0,90,250,0,0);
				R_ParticleIstar(e->origin);
				//return;
				//e->effects3 = EF3_HIPERTRANS;

				R_RayFieldParticles(e->origin);
				};

			if (e->model->use_SPIKE || e->model->effect == MFX_SPIKE) {
				//R_ParticleIstar(e->origin);
				//R_Fire(e, true);
				//DefineFlareColor(e->origin,18,0,90,250,0,0);
				//R_SparkShower (e->origin, vec3_origin);
				//R_BeamZing(r_origin,e->origin);
				//R_EntityParticles(e);//XFX

					if (teirand(1000)<5)
					{
					
						AngleVectors(e->angles,  downmove,dumy,dumy);

						VectorNormalize(downmove);
						downmove[0] *= 425;
						downmove[1] *= 425;
						downmove[2] *= -425;

						VectorAdd(e->origin,downmove,downmove);
						R_BeamSpike(e->origin,downmove);
					
					}
	
				};
			if (e->model->use_LUX || e->model->use_GLOW || e->model->effect == MFX_LUX) {
			
				if (r_autofluor.value)
				{				
					VectorCopy(e->origin, downmove);		
					downmove[2] += 33;
					dx = TraceLine (e->origin, downmove, stop, normal);
					if(dx!=1) {
						if(teirand(100)<2)
							R_SparkShower (stop, vec3_origin );

						stop[2] -= 1;
						//Con_Printf("%f dirt\n",dx);

						DefineFlareColor(stop,lhrandom(70,90), 0,125,100,100,255);
						stop[2] -= 5;
						if (r_autofluor.value >1)
							DefineFlareColor(stop,lhrandom(170,290), 0,25,120,120,225);
						stop[2] -= 10;
						//DefineFlareColor(stop,lhrandom(290,400), 0,15,180,180,205);
						if (r_autofluor.value >2)
							DefineFlareColor(stop,lhrandom(290,400), 0,115,250,180,105);
						if (r_autofluor.value >3)
						{					
							dl = CL_AllocDlight (0);
							stop[2] -= 15;
							VectorCopy (stop, dl->origin);
							dl->color[0]	= 1;
							dl->color[1]	= 0.7f;
							dl->color[2]	= 0.6f;
							dl->radius		= 300;
							dl->die			= cl.time + 3;
							dl->decay		= 200;
						}

					}
					if (r_autofluor.value >1)
						DefineFlareColor(e->origin,lhrandom(50,100), 0,10,200,200,240);
					DefineFlareColor(e->origin,lhrandom(100,200), 0,5,200,200,240);
				}				
				
				//VectorCopy(e->origin,downmove);
				//i = lhrandom(25,300);
				//	downmove[0] += lhrandom(-i,i);
				//	downmove[1] += lhrandom(-i,i);
				//	downmove[2] += lhrandom(-i,i);
				//R_BeamZing(e->origin, downmove);
				//if(teirand(100)<2)
				//	R_SuperZing(e->origin, downmove);
				//DefineFlareColorTexnum(e->origin,120,3,250,199,199,199,focus_tex);
				//DefineFlareColorTexnum(e->origin,100, 0,250,200,200,240,GetTex(1));
				//DefineFlareColor(e->origin,100, 0,250,200,200,240);

				};

			if (e->model->use_GIB || e->model->effect == MFX_GIB) {
				//Underwater bloodbubbles
				R_ParticleBubbleGib(e->origin);//BubbleBlood, only underwater
				};

			if (e->model->use_GRASS || e->model->effect == MFX_GRASS) {

					VectorSubtract(e->origin, r_origin, downmove );
					add =  Length(downmove);

					add = -add/400 + 3;

					e->alpha = add;

					if (add<0) {
						e->alpha = 0.1f;
						e->effects3 = EF3_NOGRASS;
					}
					else
					if (add>1) {
						e->alpha = 1;
						e->effects3 = 0;
						}
					else
						e->effects3 = 0;
				};

			if (e->model->use_LUX2 || e->model->effect == MFX_LUX2) {		
   				DefineFlareColor(e->origin,lhrandom(70,90), 0,95,200,200,200);		
				//DefineFlareColor(e->origin,lhrandom(170,290), 0,25,225,220,220);
				//DefineFlareColor(e->origin,lhrandom(100,200), 0,5,200,200,200);
				};

			if (e->model->use_LASERBEAM || e->model->effect == MFX_LASERBEAM) {
					AngleVectors(e->angles,  downmove,dumy,dumy);

					VectorNormalize(downmove);
					downmove[0] *= 9096;
					downmove[1] *= 9096;
					downmove[2] *= -9096;//forward

					dx = TraceLine (e->origin, downmove, stop, normal);

					color[0] = 255;
					color[1] = 255;
					color[2] = 1;

					
					//AddTrailTrail2(e->origin,stop,1,3,color);
					AddTrailTrail( e->origin,stop, 0.1f, 3, color);

					VectorCopy(e->origin,downmove);
					VectorCopy(stop,dumy);


					//R_BeamSpike(e->origin,stop);
					glDisable(GL_TEXTURE_2D);
					//glBlendFunc (GL_ONE, GL_ONE);
					//glBindTexture (GL_TEXTURE_2D, GetTex(4));//4->ZING
					//glBlendFunc(GL_SRC_ALPHA, GL_ONE);

					glBegin(GL_QUADS);
		
						downmove[2]-=lhrandom(2,3);
						glColor4f (0.7f,0,0,1);
						glVertex3fv (downmove);			
						
						dumy[2]-=lhrandom(2,3);
						glColor4f (0.7f,0,0,0);
						glVertex3fv (stop);

					VectorCopy(e->origin,downmove);
					VectorCopy(stop,dumy);

						downmove[2]+=lhrandom(2,3);
						dumy[2]+=lhrandom(2,3);

						glColor4f (0.7f,0,0,0);
						glVertex3fv (dumy);								
						glColor4f (0.7f,0,0,1);
						glVertex3fv (downmove);

					glEnd ();
					glEnable(GL_TEXTURE_2D);
				};
			if (e->model->use_FOOTSOUND || e->model->effect == MFX_FOOTSOUND) {
					self = e;									
					if( (self->frame == 1||self->frame == 4||self->frame == 7 || self->frame ==10) && (self->stepdone!=self->frame)) {
						
						VectorCopy(self->origin,dest);

						dest[2] -= 33;//32 also work

						if ( TraceLine (self->origin, dest, stop, normal)<1)
						{
							S_StartSound (-1, 3, cl_sfx_foot, self->origin, 1, 1);
							self->stepdone = self->frame;
						}
					}			

			}
}

extern cvar_t r_autobubbles;
void R_ParticleBubble (vec3_t origin);


void R_DrawAliasModel (entity_t *e, int cull)
{
	int			i, lnum, anim;
//	int			ef;//Tei speed
	int			contents;//Tei autobubbles
	float		add;
	md2_t		*pheader; // Tomaz - Quake2 Models
	vec3_t		dist, mins, maxs;//, downmove ,dumy;
//	vec3_t	normal;//, start ;

	model_t		*clmodel;
	aliashdr_t	*paliashdr;

//	dlight_t	*dl;

//	float		dx;

	clmodel = currententity->model;

	VectorAdd (currententity->origin, clmodel->mins, mins);
	VectorAdd (currententity->origin, clmodel->maxs, maxs);


	//Tei speed
	if (e->model->effect)
	{
		MFX_Apply(e);			

		//if(e->model->effect == MFX_MISSILE  || e->model->effect == MFX_SPIKE)
		//						RenderLsigEnt(e);
	}
	//Tei mfx

	if (e->effects3	== EF3_NOGRASS)
		return;//XFX tei grass


	if ( e->model->dpxflare  ) 
			DefineFlare(e->origin,e->model->dpxflare, 0,50);//Tei dp flare
	

	//Entity dont need more than this, is a fx entity
	if (e->model->isfx_)
	{
			if (e->model->glow_radius)
				R_DrawGlows(e);
			return;
	}


	//Tei underwater bubbles
	if (r_autobubbles.value && (teirand(100)<r_autobubbles.value))
	{
		contents	= Mod_PointInLeaf(e->origin, cl.worldmodel)->contents;
		if (contents == CONTENTS_WATER)
			R_ParticleBubble(e->origin);
	}
	//Tei underwater bubbles
	


	//Tei speedup

	if (cull && R_CullBox (mins, maxs))
		return;

	VectorSubtract (r_origin, currententity->origin, modelorg);

	// HACK HACK HACK -- no fullbright colors, so make torches full light
	if (clmodel->fullbright)
	{
		lightcolor[0] = lightcolor[1] = lightcolor[2] = 256 / 200;	// Tomaz - Lit Support
	}
	else
	{
		float ang_ceil, ang_floor;

		//
		// get lighting information
		//

		R_LightPoint(currententity->origin); // Tomaz - Lit Support

		for (lnum = 0; lnum < MAX_DLIGHTS; lnum++)
		{
			if (cl_dlights[lnum].die >= cl.time)
			{
				VectorSubtract (currententity->origin,
								cl_dlights[lnum].origin,
								dist);
				add = cl_dlights[lnum].radius - Length(dist);

				// Tomaz - Lit Support Begin
				if (add > 0)
				{
					lightcolor[0] += add * cl_dlights[lnum].color[0];
					lightcolor[1] += add * cl_dlights[lnum].color[1];
					lightcolor[2] += add * cl_dlights[lnum].color[2];
				}
				// Tomaz - Lit Support End
			}
		}

		// add pitch angle so lighting changes when looking up/down (mainly for viewmodel)
		lightlerpoffset = e->angles[1] * (16 / 360.0);
		ang_ceil = ceil(lightlerpoffset);
		ang_floor = floor(lightlerpoffset);

		lightlerpoffset = ang_ceil - lightlerpoffset;

		shadedots_mdl	= r_avertexnormal_dots_mdl[(int)ang_ceil & (16 - 1)];
		shadedots_md2	= r_avertexnormal_dots_md2[(int)ang_ceil & (16 - 1)];
		shadedots2_mdl	= r_avertexnormal_dots_mdl[(int)ang_floor & (16 - 1)];
		shadedots2_md2	= r_avertexnormal_dots_md2[(int)ang_floor & (16 - 1)];

		// Tomaz - Lit Support Begin
		lightcolor[0] = lightcolor[0] * 0.005;
		lightcolor[1] = lightcolor[1] * 0.005;
		lightcolor[2] = lightcolor[2] * 0.005;
		// Tomaz - Lit Support End


		if (!lightcolor[0] && !lightcolor[1] && !lightcolor[2])
		{
			VectorCopy  (lightcolor, currententity->last_light);
		}
		else 
		{
			VectorAdd   (lightcolor, currententity->last_light, lightcolor);
			VectorScale (lightcolor, 0.5f, lightcolor);
			VectorCopy  (lightcolor, currententity->last_light);
		}
	}

	
	//
	// locate the proper data
	//
	if (clmodel->aliastype == ALIASTYPE_MDL)
	{
		paliashdr = (aliashdr_t *)Mod_Extradata (currententity->model);
		c_alias_polys += paliashdr->numtris;
	}
	else
	{
		pheader = (md2_t *)Mod_Extradata (currententity->model);
		c_alias_polys += pheader->num_tris;
	}

	//
	// draw all the triangles
	//
	//Tei softwaremode emulation
#if 0
	if (gl_emulatesoftwaremode.value ==1)
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	else
	if (gl_emulatesoftwaremode.value ==2)
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#endif
	//Tei softwaremode emulation

	glPushMatrix ();

	// prevent viemodels from messing up
	if (!weaponmodel || e->effects3 == EF3_ISWEAPON)
	{
		R_BlendedRotateForEntity (e, 0);
	}
	else
	{
		R_RotateForEntity (e, 0);

	}

	if (clmodel->aliastype == ALIASTYPE_MDL)
	{

		// Tei normal eyes
		/*
		if (!strcmp (clmodel->name, "progs/eyes.mdl") && gl_doubleeyes.value) 
		{
			glTranslatef (paliashdr->scale_origin[0], paliashdr->scale_origin[1], paliashdr->scale_origin[2] - (22 + 8));
			// double size of eyes, since they are really hard to see in gl
			glScalef (paliashdr->scale[0]*2, paliashdr->scale[1]*2, paliashdr->scale[2]*2);
		}
		else
		{*/
			glTranslatef (paliashdr->scale_origin[0], paliashdr->scale_origin[1], paliashdr->scale_origin[2]);
			glScalef (paliashdr->scale[0], paliashdr->scale[1], paliashdr->scale[2]);
		//}
		// Tei normal eyes

		anim = (int)(cl.time*10) & 3;
	
	    glBindTexture (GL_TEXTURE_2D, paliashdr->gl_texturenum[currententity->skinnum][anim]);
		glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		// we can't dynamically colormap textures, so they are cached
		// seperately for the players.  Heads are just uncolored.
		if (currententity->colormap != vid.colormap && !gl_nocolors.value)
		{
			i = currententity - cl_entities;
			if (i >= 1 && i<=cl.maxclients /* && !strcmp (currententity->model->name, "progs/player.mdl") */)
				glBindTexture (GL_TEXTURE_2D, playertextures - 1 + i);
		}
		
		//glShadeModel (GL_SMOOTH);//FH!

		// Tei fx no draw
		//if(strncmp (clmodel->name, "progs/fx_",9))
		if ( (e->model->effect != MFX_NODRAW) && clmodel->visible )
			GL_DrawAliasBlendedFrame (currententity->frame, paliashdr, currententity);	
		//else
		//	R_DrawNullModel();
		// Tei fx no draw

		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

		if (paliashdr->fb_texturenum[currententity->skinnum][anim] && gl_fbr.value)
		{
			glBindTexture (GL_TEXTURE_2D, paliashdr->fb_texturenum[currententity->skinnum][anim]);
			GL_DrawAliasBlendedFrame (currententity->frame, paliashdr, currententity);
		}
	
		//FH!
		//glShadeModel (GL_FLAT);
		//glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
		//FH!


		glPopMatrix ();		



		// Glow flare begin - see gl_flares.c for more info
		if (gl_glows.value)
		{
			if (clmodel->glow_radius)
				R_DrawGlows(currententity);
		}	

		if (r_shadows.value)
		{
			if (!clmodel->noshadow && clmodel->visible)//invisible no shadow
			{
				// Tomaz - New Shadow Begin
				trace_t		downtrace;
				vec3_t		downmove;
				// Tomaz - New Shadow End

				glPushMatrix ();

				R_BlendedRotateForEntity (e, 1);

				VectorCopy (e->origin, downmove);

				downmove[2] = downmove[2] - 4096;
				memset (&downtrace, 0, sizeof(downtrace));
				SV_RecursiveHullCheck (cl.worldmodel->hulls, 0, 0, 1, e->origin, downmove, &downtrace);

				glDisable (GL_TEXTURE_2D);
				glDepthMask(false); // disable zbuffer updates
				glColor4f (0,0,0,(currententity->alpha - ((mins[2]-downtrace.endpos[2])/60)));
				GL_DrawAliasBlendedShadow (paliashdr, lastposenum0, lastposenum, currententity);
				glDepthMask(true); // disable zbuffer updates
				glEnable (GL_TEXTURE_2D);
				glColor4f (1,1,1,1);
				glPopMatrix ();


			}
		}
	}
	else
	{
	    glBindTexture (GL_TEXTURE_2D, pheader->gl_texturenum[currententity->skinnum]);
		R_SetupQ2AliasFrame (currententity->frame, pheader, currententity);
	}

	glPopMatrix ();	

	//Tei gl softwaremode emulation
#if 0
	if (gl_emulatesoftwaremode.value ==2)
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	else
	if (gl_emulatesoftwaremode.value ==1)
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#endif
	//Tei gl softwaremode emulation

}
//==================================================================================

/*
=============
R_SetupTransEntities
=============
*/
void R_SetupTransEntities (void)
{
	int		i;

	for (i=0 ; i<cl_numvisedicts ; i++)
	{
		currententity = cl_visedicts[i];
	
		currententity->contents = Mod_PointInLeaf(currententity->origin, cl.worldmodel)->contents;

		if ((currententity->model->type == mod_alias) &&
			(currententity == &cl_entities[cl.viewentity]))
			 currententity->angles[0] *= 0.3f;

		if ((currententity->alpha > 1) ||
			(currententity->alpha <= 0))
			 currententity->alpha = 1;

		if ((currententity->scale > 4) ||
			(currententity->scale <= 0))
			 currententity->scale = 1;

		currententity->solid = false;
	}
}

qboolean wireframe;

/*
=============
R_SortTransEntities
=============
*/



void R_SortTransEntities (qboolean inwater)
{
	int			i;
	float		bestdist, dist;
	entity_t	*bestent;
	vec3_t		start, test;
	
	VectorCopy(r_refdef.vieworg, start);

transgetent:
	bestdist = 0;
	for (i=0 ; i<cl_numvisedicts ; i++)
	{
		currententity = cl_visedicts[i];

		if (currententity->solid)
			continue;

		if(!inwater && currententity->contents != CONTENTS_EMPTY)
			continue;
		
		if(inwater && currententity->contents == CONTENTS_EMPTY)
			continue;

		VectorCopy(currententity->origin, test);

		dist = (((test[0] - start[0]) * (test[0] - start[0])) +
				((test[1] - start[1]) * (test[1] - start[1])) +
				((test[2] - start[2]) * (test[2] - start[2])));

		if (dist > bestdist)
		{
			bestdist	= dist;
			bestent		= currententity;
		}
	}

	if (bestdist == 0)
		return;

	bestent->solid = true;

	currententity = bestent;
	
	switch (currententity->model->type)
	{
	case mod_alias:
		R_DrawAliasModel  (currententity, true);
		break;
	case mod_brush:
		{
			R_SetupBrushPolys (currententity);
			if (gl_wireframe.value)
			{
				wireframe = true;
				glDisable (GL_DEPTH_TEST);
				R_SetupBrushPolys (currententity);
				glEnable (GL_DEPTH_TEST);
				wireframe = false;
			}
			break;
		}
	case mod_sprite:
		R_DrawSpriteModel (currententity);

		break;

	case mod_hud:
		R_DrawHudModel( currententity );
		//Con_Printf("hello mod_hud %s\n", currententity->model->name); Ok, arrive here
		break;

	default:
		//Con_Printf("hello unknom %s\n", currententity->model->name);
		break;
	}

	goto transgetent;
}

/*
=============
R_DrawViewModel
=============
*/
void R_DrawViewModel (void)
{

	if (!r_drawviewmodel.value)
		return;

	if (chase_active.value)
		return;

	if (cl.items & IT_INVISIBILITY)
		return;

	if (cl.stats[STAT_HEALTH] <= 0)
		return;

	currententity = &cl.viewent;
	if (!currententity->model)
		return;

	// Tomaz - QC Alpha Scale Begin
	currententity->alpha = cl_entities[cl.viewentity].alpha;
	currententity->scale = cl_entities[cl.viewentity].scale;

	if ((currententity->alpha > 1) ||
		(currententity->alpha <= 0))
		currententity->alpha = 1;

	if ((currententity->scale > 4) ||
		(currententity->scale <= 0))
		currententity->scale = 1;
	// Tomaz - QC Alpha Scale End

	// hack the depth range to prevent view model from poking into walls
	glDepthRange (gldepthmin, gldepthmin + 0.3*(gldepthmax-gldepthmin));
	weaponmodel = true;

	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);	
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//GL_ResetTextures_f();

	R_DrawAliasModel (currententity, false);

	
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	//Changed
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	//Changed

	weaponmodel = false;
	glDepthRange (gldepthmin, gldepthmax);

}

extern cvar_t brightness; // muff
// idea originally nicked from LordHavoc
// re-worked + extended - muff 5 Feb 2001
// called inside polyblend

extern cvar_t dbrightness;

void DoGamma(void)
{
	if (brightness.value < 0.2f)
		brightness.value = 0.2f;
	if (brightness.value >= 1)
	{
		brightness.value = 1;
		return;
	}

	// believe it or not this actually does brighten the picture!!
	glBlendFunc (GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f (1, 1, 1, brightness.value + dbrightness.value );//Tei dymlight. No safe version!
	//glColor4f (1, 1, 1, bound(0,brightness.value + dbrightness.value,1);
	glBegin (GL_QUADS);
	glVertex3f (10, 100, 100);
	glVertex3f (10, -100, 100);
	glVertex3f (10, -100, -100);
	glVertex3f (10, 100, -100);
	
	// if we do this twice, we double the brightening effect for a wider range of gamma's
	glVertex3f (11, 100, 100);
	glVertex3f (11, -100, 100);
	glVertex3f (11, -100, -100);
	glVertex3f (11, 100, -100);
	glEnd ();
}

/*
============
R_PolyBlend
============
*/
void R_PolyBlend (void)
{
	if (brightness.value == 1 && !v_blend[3])
		return;

	glDisable (GL_DEPTH_TEST);
	glDisable (GL_TEXTURE_2D);

	glLoadIdentity ();
	glRotatef (-90,  1, 0, 0);    // put Z going up
	glRotatef (90,  0, 0, 1);    // put Z going up

	if (v_blend[3])
	{
		glColor4fv (v_blend);
		glBegin (GL_QUADS);
		glVertex3f (10, 100, 100);
		glVertex3f (10, -100, 100);
		glVertex3f (10, -100, -100);
		glVertex3f (10, 100, -100);
		glEnd ();
	}

	if (brightness.value != 1)
	{
		DoGamma();
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	glEnable (GL_TEXTURE_2D);
	glEnable (GL_DEPTH_TEST);
}

// Q2!
void R_Flash( void )
{
	R_PolyBlend ();
}
// Q2!



int SignbitsForPlane (mplane_t *out)
{
	int	bits, j;

	// for fast box on planeside test

	bits = 0;
	for (j=0 ; j<3 ; j++)
	{
		if (out->normal[j] < 0)
			bits |= 1<<j;
	}
	return bits;
}

void R_SetFrustum (void)
{
	int		i;

	
	// LordHavoc: note to all quake engine coders, the special case for 90
	// degrees assumed a square view (wrong), so I removed it, Quake2 has it
	// disabled as well.
	if (r_refdef.fov_x == 90) 
	{
		// front side is visible

		VectorAdd (vpn, vright, frustum[0].normal);
		VectorSubtract (vpn, vright, frustum[1].normal);

		VectorAdd (vpn, vup, frustum[2].normal);
		VectorSubtract (vpn, vup, frustum[3].normal);
	}
	else
	{
	
		

		RotatePointAroundVector( frustum[0].normal, vup, vpn, -(90-r_refdef.fov_x * 0.5 ) );		// Tomaz - Speed
		RotatePointAroundVector( frustum[1].normal, vup, vpn, 90-r_refdef.fov_x * 0.5 );			// Tomaz - Speed
		RotatePointAroundVector( frustum[2].normal, vright, vpn, 90-r_refdef.fov_y * 0.5 );			// Tomaz - Speed
		RotatePointAroundVector( frustum[3].normal, vright, vpn, -( 90 - r_refdef.fov_y * 0.5 ) );	// Tomaz - Speed
	}

	for (i=0 ; i<4 ; i++)
	{
		frustum[i].type = PLANE_ANYZ;
		frustum[i].dist = DotProduct (r_origin, frustum[i].normal);
		frustum[i].signbits = SignbitsForPlane (&frustum[i]);
	}
}

/*
===============
R_SetupFrame
===============
*/
void R_SetupFrame (void)
{
	R_AnimateLight ();

	r_framecount++;

// build the transformation matrix for the given view angles
	VectorCopy (r_refdef.vieworg, r_origin);

	AngleVectors (r_refdef.viewangles, vpn, vright, vup);

// current viewleaf
	r_oldviewleaf = r_viewleaf;
	r_viewleaf = Mod_PointInLeaf (r_origin, cl.worldmodel);

	V_SetContentsColor (r_viewleaf->contents);
	V_CalcBlend ();

	c_brush_polys = 0;
	c_alias_polys = 0;

}


void MYgluPerspective( GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar )
{
   GLdouble xmin, xmax, ymin, ymax;

   ymax = zNear * tan( fovy * 0.008726646259971);	// Tomaz Speed
   ymin = -ymax;

   xmin = ymin * aspect;
   xmax = ymax * aspect;

   glFrustum( xmin, xmax, ymin, ymax, zNear, zFar );
}

//extern cvar_t r_waterwarp;//Tei friz waw

/*
=============
R_SetupGL
=============
*/


void R_SetupGL (void)
{

	float	screenaspect;
	extern	int glwidth, glheight;
	int		x, x2, y2, y, w, h;
	//float fovx, fovy; //johnfitz
	//int contents; //johnfitz

	//
	// set up viewpoint
	//
	glMatrixMode(GL_PROJECTION);
    glLoadIdentity ();
	x = r_refdef.vrect.x * glwidth/vid.width;
	x2 = (r_refdef.vrect.x + r_refdef.vrect.width) * glwidth/vid.width;
	y = (vid.height-r_refdef.vrect.y) * glheight/vid.height;
	y2 = (vid.height - (r_refdef.vrect.y + r_refdef.vrect.height)) * glheight/vid.height;

	if (mirror)
		Mirror_Scale ();
	else 
	{
		glCullFace(GL_FRONT);
	}

	// fudge around because of frac screen scale
	if (x > 0)
		x--;
	if (x2 < glwidth)
		x2++;
	if (y2 < 0)
		y2--;
	if (y < glheight)
		y++;

	w = x2 - x;
	h = y - y2;

	glViewport (glx + x, gly + y2, w, h);
    screenaspect = (float)r_refdef.vrect.width/r_refdef.vrect.height;
    MYgluPerspective (r_refdef.fov_y,  screenaspect,  4, 8193);



	//Q2!
	glCullFace(GL_FRONT);
	//Q2!

	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity ();

    glRotatef (-90,  1, 0, 0);	    // put Z going up
    glRotatef (90,  0, 0, 1);	    // put Z going up
    glRotatef (-r_refdef.viewangles[2],  1, 0, 0);
    glRotatef (-r_refdef.viewangles[0],  0, 1, 0);
    glRotatef (-r_refdef.viewangles[1],  0, 0, 1);
    glTranslatef (-r_refdef.vieworg[0],  -r_refdef.vieworg[1],  -r_refdef.vieworg[2]);

	glGetFloatv (GL_MODELVIEW_MATRIX, r_world_matrix);

	//
	// set drawing parms
	//

	// Q2!
	//
	// set drawing parms
	//
	if (gl_cull.value)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);
	// Q2!

	glEnable(GL_DEPTH_TEST);
}

/*
=============
R_Clear
=============
*/
void R_Clear (void)
{
	if (r_mirroralpha.value < 1.0)
	{
		Mirror_Clear ();
	}
	else
	{
		if (gl_clear.value || gl_wireonly.value)
		{
			glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}
		else
		{
			glClear (GL_DEPTH_BUFFER_BIT);
		}
		gldepthmin = 0;
		gldepthmax = 1;
		glDepthFunc (GL_LEQUAL);
	}

	glDepthRange (gldepthmin, gldepthmax);
}

/*
=============
R_RenderScene
renders the current screen, without mirrors, thus can be used in mirror code
=============
*/


void R_MoveParticles ();
void DrawFlares();//Tei dp style flares support
int SV_HullPointContents (hull_t *hull, int num, vec3_t p);
//void	R_DrawCoronas();//Tei				
extern cvar_t dbrightness;

vec3_t sunorigin = { 30, 40, 800};
vec3_t bitup = { 0,0, 3 };


//Tei general trace fracion 
int TraceFraction (vec3_t start, vec3_t end)
{
	//int i;
	trace_t	trace;

	memset (&trace, 0, sizeof(trace));
	VectorCopy (end, trace.endpos);
	trace.fraction = 1;
	SV_RecursiveHullCheck (cl.worldmodel->hulls, 0, 0, 1, start, end, &trace);

	return trace.fraction;
}
//Tei general trace fraction




//Check that sky is content solid... aRRGGG!!!
int TraceSky (vec3_t start, vec3_t end)
{
	int i;
	trace_t	trace;
	vec3_t	loc;

	memset (&trace, 0, sizeof(trace));
	VectorCopy (end, trace.endpos);
	trace.fraction = 1;
	SV_RecursiveHullCheck (cl.worldmodel->hulls, 0, 0, 1, start, end, &trace);

	VectorAdd(trace.endpos, bitup, loc );

	//trace.endpos
	i = SV_PointContents ( loc );

	//if (i != -1)
	//	Con_Printf("A: %d is %f fract and %d\n", i, trace.fraction, trace.insky );

	
	//i = (int )SV_HullPointContents (&sv.worldmodel->hulls[0], 0, loc);

	//if (i != -1)
	//	Con_Printf("B: %d is %f fract and %d\n", i, trace.fraction, trace.insky );

	//i = Mod_PointInLeaf(loc, cl.worldmodel)->contents;
	//
	//if (i != -1)
	//	Con_Printf("C: %d is %f fract and %d\n", i, trace.fraction, trace.insky );

	return i == CONTENTS_SKY;
}

#if 1
	//TELEJANO
cvar_t deathmatch, coop;
void R_DefineSun(void)
{
	vec3_t		zone;//tei

	if ( gl_sun.value && cls.state == ca_connected && !cls.demoplayback && !coop.value && !deathmatch.value ) {
		VectorAdd( r_origin, sunorigin, zone);
		if ( TraceSky (r_refdef.vieworg,zone) ) {
			DefineFlare(zone, 1800,2,40); 
			DefineFlare(zone, 400,2,80); 
			DefineFlare(zone, 2000,2,10);
			if (gl_sun.value == 2 ) 
				if (dbrightness.value>(-0.2))
					dbrightness.value -= 0.001f;
		}
		else
			if (gl_sun.value == 2 )
				if (dbrightness.value<0)
					dbrightness.value += 0.001f;
	}
}
#endif

extern cvar_t  zone_foggreen; 
extern cvar_t  zone_fogblue;
extern cvar_t  zone_fogred;
extern cvar_t  zone_speed;
extern cvar_t  zone_enable;
extern cvar_t  zone_start;
extern cvar_t  zone_end;

void AddTrailTrail2(vec3_t start, vec3_t end, float time, float size, vec3_t color);


void RenderLsig (void)
{


	vec3_t	forward, up, right;
	vec3_t	dest, colour, origin;


	// if can't see player, reset
	AngleVectors (cl.viewangles, forward, right, up);

	// find the spot the player is looking at
	VectorMA (r_refdef.vieworg, 4096, forward, dest);

	colour[0]= 0.1f;	colour[1]=0;	colour[2]=0;

	VectorCopy(r_refdef.vieworg, origin);

	//VectorSubtract( dest,forward, dest);

	VectorMA( origin, 16,right,origin);
	
	origin[2] -= 8;


	
	//AddTrailTrail2(origin, dest, 0.1 , 1,colour );
	
	//DefineFlare(dest,40,0,250);
	//DefineFlare(dest, 40,2,180); 
	DefineFlareColor(dest,80, 2,155,250,100,100);

	//glDisable(GL_DEPTH_TEST);

	glDisable(GL_TEXTURE_2D);
	glBegin(GL_LINES);
		
		glColor3f (0.7f,0,0);
		glVertex3fv (origin);
		glVertex3fv (dest);

	glEnd ();
	glEnable(GL_TEXTURE_2D);
	
	//glEnable(GL_DEPTH_TEST);

}


void RenderFocus (void)
{

	float	dist;
	vec3_t	forward, up, right, normal;
	vec3_t	dest, stop,colour, origin;
	dlight_t	*dl;


	// if can't see player, reset
	AngleVectors (cl.viewangles, forward, right, up);

	// find the spot the player is looking at
	VectorMA (r_refdef.vieworg, 4096, forward, dest);

	colour[0]= 0.1f;	colour[1]=0;	colour[2]=0;

	VectorCopy(r_refdef.vieworg, origin);

	dist = TraceLine(origin, dest, stop, normal);

	dl = CL_AllocDlight (0);
	VectorCopy (stop, dl->origin);
	dl->radius = 100 + 400*dist;
	dl->die = cl.time + 0.1;
	dl->decay = 200;
	
}



void RenderLsigEnt (entity_t *e  )
{


	vec3_t	forward, up, right;//, normal;
	vec3_t	dest,colour, origin;


	// if can't see player, reset
	AngleVectors (e->angles, forward, right, up);

	// find the spot the player is looking at
	VectorMA (e->origin, 4096, forward, dest);

	colour[0]= 0.1f;	colour[1]=0;	colour[2]=0;

	VectorCopy(e->origin, origin);

	//VectorSubtract( dest,forward, dest);

	VectorMA( origin, 16,right,origin);
	
	//origin[2] -= 8;


	
	AddTrailTrail2(origin, dest, 0.1f , 1,colour );
	
	//DefineFlare(dest,40,0,250);
	//DefineFlare(dest, 40,2,180); 
	DefineFlareColor(dest,80, 2,155,250,100,100);

	//glDisable(GL_DEPTH_TEST);

	glDisable(GL_TEXTURE_2D);
	glBegin(GL_LINES);
		
		glColor3f (1,0,0);
		glVertex3fv (origin);
		glVertex3fv (dest);

	glEnd ();
	glEnable(GL_TEXTURE_2D);
	
	//glEnable(GL_DEPTH_TEST);

}


extern cvar_t mod_lsig;
extern cvar_t mod_focus;
void XR_DrawParticles (void);


void R_RenderScene (void)
{
	vec3_t		colors;
	int			mask;//Tei zones


	double	time1, time2;

	//Con_DPrintf("..rs 1\n" );//Tei debug


	//Tei sun stuff
#if 0 //Sun stuff disabled
	if (gl_sun.value) {
		sunorigin[0] = gl_sun_x.value;
		sunorigin[1] = gl_sun_y.value;
		sunorigin[2] = gl_sun_z.value;
	}
#endif 
	//Tei sun stuff

	
	//Con_DPrintf("..rs 2\n" );//Tei debug

#if 1 //Telejano autozone	
	if (zone_enable.value)
	{


		//red
		if(gl_fogred.value < zone_fogred.value)
			gl_fogred.value += zone_speed.value * 0.01;
		else
			gl_fogred.value -= zone_speed.value * 0.01;

		//green
		if(gl_foggreen.value < zone_foggreen.value)
			gl_foggreen.value += zone_speed.value * 0.01;
		else
			gl_foggreen.value -= zone_speed.value * 0.01;
	
		//blue		
		if (gl_fogblue.value < zone_fogblue.value)
			gl_fogblue.value += zone_speed.value * 0.01;
		else
			gl_fogblue.value -= zone_speed.value * 0.01;

		//start
		if (gl_fogstart.value < zone_start.value)
			gl_fogstart.value += zone_speed.value ;
		else
			gl_fogstart.value -= zone_speed.value ;

		//end
		if (gl_fogend.value < zone_end.value)
			gl_fogend.value += zone_speed.value ;
		else
			gl_fogend.value -= zone_speed.value ;
		
		//Reset
		mask = 0;

		if ( fabs(gl_fogred.value - zone_fogred.value)< (zone_speed.value * 0.05) )
		{
			gl_fogred.value = zone_fogred.value; mask |= 1;
		}

		if ( fabs(gl_foggreen.value - zone_foggreen.value)< (zone_speed.value * 0.05) )
		{
			gl_foggreen.value = zone_foggreen.value; mask |= 2;
		}

		if ( fabs(gl_fogblue.value - zone_fogblue.value)< (zone_speed.value *0.05) )
		{
			gl_fogblue.value = zone_fogblue.value; mask |= 4;
		}

		if ( fabs(gl_fogstart.value - zone_start.value)< (zone_speed.value*2) )
		{		
			gl_fogstart.value = zone_start.value; mask |= 8;
		}

		if ( fabs(gl_fogend.value - zone_end.value)< (zone_speed.value*2) )
		{
			gl_fogend.value = zone_end.value; mask |= 16;
		}
		
		Cvar_SetValue("gl_fogend",		gl_fogend.value		);
		Cvar_SetValue("gl_fogstart",	gl_fogstart.value	);
		Cvar_SetValue("gl_fogred",		gl_fogred.value		);
		Cvar_SetValue("gl_foggreen",	gl_foggreen.value	);
		Cvar_SetValue("gl_fogblue",		gl_fogblue.value	);

		if ( mask & (1|2|4|8|16) )  //If all active, then disable zone
			zone_enable.value = 0;

	}
#endif

	//Con_DPrintf("..rs 3\n" );//Tei debug


	if (r_norefresh.value)
		return;

	if (r_speeds.value)
	{
		glFinish ();
		time1 = Sys_FloatTime ();
		c_brush_polys = 0;
		c_alias_polys = 0;
	}

	//Con_DPrintf("..rs 4\n" );//Tei debug

	R_SetupFrame ();

	R_PushDlights ();

	//Con_DPrintf("..rs 5\n" );//Tei debug

	R_SetFrustum ();

	R_SetupGL ();

	//Con_DPrintf("..rs 6\n" );//Tei debug

	R_MarkLeaves ();	// done here so we know if we're in water

	//Con_DPrintf("..rs 7\n" );//Tei debug

	R_DrawWorld ();		// adds static entities to the list

	//Con_DPrintf("..rs 8\n" );//Tei debug
	//R_RenderDecals ();	//QW! decals

	
	if( mod_lsig.value)
		RenderLsig();//XFX

	if (mod_focus.value)
		RenderFocus();//XFX

	//Con_DPrintf("..rs 9\n" );//Tei debug

	S_ExtraUpdate ();	// don't let sound get messed up if going slow

	//Con_DPrintf("..rs 10\n" );//Tei debug

	R_SetupTransEntities ();


	//Con_DPrintf("..rs 11\n" );//Tei debug

	if (!mirror_render)
		R_MoveParticles ();

	//Con_DPrintf("..rs 11 - particles\n" );//Tei debug

#if 0 //Not compatible with the NET stuff.. (trace is forbiden in server time)
	//TELEJANO
	//Tei sun
	R_DefineSun();

	//Tei sun
#endif

	//Tei here fix view trought solid walls 
	//DrawFlares();//Tei here flares dp and.. why here? is this a bug? a cheap hack?
	//Tei here fix view trought solid walls 

	//Con_DPrintf("..rs 12 - sun\n" );//Tei debug


	if (cl.cshifts[CSHIFT_CONTENTS].percent == 0)
	{
		R_SortTransEntities (true);
		if (gl_particles.value)
			R_DrawParticles (true);	// Tomaz - fixing particle / water bug

	//	XR_DrawParticles();//Tei QMB!

		R_DrawWaterSurfaces (); // Tomaz - fixing particle / water bug

#if 1
		//TELEJANO		
		DrawFlares();//Tei here flares dp and.. why here? is this a bug? a cheap hack?
#endif

		R_SortTransEntities (false);
		if (gl_particles.value)
			R_DrawParticles (false);// Tomaz - fixing particle / water bug
	}
	else
	{
		R_SortTransEntities (false);
		if (gl_particles.value)
			R_DrawParticles (false);// Tomaz - fixing particle / water bug
	//	XR_DrawParticles();//Tei QMB!

		R_DrawWaterSurfaces ();	// Tomaz - fixing particle / water bug

#if 1
		//TELEJANO		
		DrawFlares();//Tei here flares dp and.. why here? is this a bug? a cheap hack?
#endif
		R_SortTransEntities (true);
		if (gl_particles.value)
			R_DrawParticles (true);	// Tomaz - fixing particle / water bug
	}

	//Con_DPrintf("..rs 13\n" );//Tei debug

	XR_DrawParticles();//Tei QMB!


		//R_DrawCoronas();//Tei				


	if (r_speeds.value)
	{
//		glFinish ();
		time2 = Sys_FloatTime ();
		Con_Printf ("%3i ms  %4i wpoly %4i epoly\n", (int)((time2-time1)*1000), c_brush_polys, c_alias_polys); 
	}

	glDisable(GL_FOG);

	if (gl_fogenable.value)
	{
		glFogi(GL_FOG_MODE, GL_LINEAR);
			colors[0] = gl_fogred.value;
			colors[1] = gl_foggreen.value;
			colors[2] = gl_fogblue.value; 
		glFogfv(GL_FOG_COLOR, colors); 
		glFogf(GL_FOG_START, gl_fogstart.value); 
		glFogf(GL_FOG_END, gl_fogend.value); 
		glEnable(GL_FOG);
	}
	
	//Con_DPrintf("..rs 14 - RenderScene ok\n" );//Tei debug

}

extern cvar_t firstperson;

/*
================
R_RenderView
================
*/
;
#include <time.h>

void R_RenderView (void)
{

	//Tei randomize stuff now.
	//if (mod_srand.value)
	//	srand(time(NULL));

	//Con_DPrintf("..r 1\n" );//Tei debug

	if (!r_worldentity.model || !cl.worldmodel)
	{
		Host_Error ("R_RenderView: NULL worldmodel");
	}

	if (r_mirroralpha.value >= 1.0)
	{
		r_mirroralpha.value = 1.0;
	}
	else if (r_mirroralpha.value <= 0)
	{
		r_mirroralpha.value = 0;
	}

	//Con_DPrintf("..r 2\n" );//Tei debug

	mirror = false;
	mirror_render = false;

	if (gl_finish.value)
	{
		glFinish ();
	}

	//Con_DPrintf("..r 3\n" );//Tei debug

	R_Clear ();

	//Tei fx
#if 0
	self = &cl_entities[cl.viewentity];
	
	if( (self->frame == 1||self->frame == 4||self->frame == 7 || self->frame ==10) && (self->stepdone!=self->frame)) {
		
		VectorCopy(self->origin,dest);

		dest[2] -= 33;//32 also work

		if ( TraceLine (self->origin, dest, stop, normal)<1)
		{
			S_StartSound (-1, 3, cl_sfx_foot, self->origin, 0.5f, 1);
			self->stepdone = self->frame;
		}
	}
#endif	
//	Con_Printf("Frame: %d\n",self->frame);

	//Con_DPrintf("..r 4\n" );//Tei debug

	if (firstperson.value)
	{
		
		if (cl_numvisedicts < MAX_VISEDICTS)
		{
			cl_visedicts[cl_numvisedicts] = &cl_entities[cl.viewentity];
			cl_numvisedicts++;
		}
	}
	//Tei fx


	//Con_DPrintf("..r 5\n" );//Tei debug

	R_RenderScene();//Bug is here

	//Con_DPrintf("..r 6\n" );//Tei debug

	if (mirror)
	{
		R_Mirror ();
	}

	//Con_DPrintf("..r 7\n" );//Tei debug

	R_DrawViewModel ();

	//Con_DPrintf("..r 8\n" );//Tei debug

	R_PolyBlend ();

	//Con_DPrintf("..R_Renderview - Ok\n" );//Tei debug
}


#if 0
void R_DrawNullModel (void)
{
	vec3_t	shadelight;
	int		i;
/*
	if ( currententity->flags & RF_FULLBRIGHT )
		shadelight[0] = shadelight[1] = shadelight[2] = 1.0F;
	else
		R_LightPoint (currententity->origin, shadelight, true);
*/
    glPushMatrix ();
	R_RotateForEntity (currententity, true);

	glDisable (GL_TEXTURE_2D);
	glColor3fv (shadelight);

	glBegin (GL_TRIANGLE_FAN);
	glVertex3f (0, 0, -16);
	for (i=0 ; i<=4 ; i++)
		glVertex3f (16*cos(i*M_PI/2), 16*sin(i*M_PI/2), 0);
	glEnd ();

	glBegin (GL_TRIANGLE_FAN);
	glVertex3f (0, 0, 16);
	for (i=4 ; i>=0 ; i--)
		glVertex3f (16*cos(i*M_PI/2), 16*sin(i*M_PI/2), 0);
	glEnd ();

	glColor3f (1,1,1);
	glPopMatrix ();
	glEnable (GL_TEXTURE_2D);
}
#endif


