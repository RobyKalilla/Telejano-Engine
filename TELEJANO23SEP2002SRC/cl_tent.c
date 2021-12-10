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
// cl_tent.c -- client side temporary entities

#include "quakedef.h"

int			num_temp_entities;
entity_t	cl_temp_entities[MAX_TEMP_ENTITIES];
beam_t		cl_beams[MAX_BEAMS];

void R_ParticleImplosionXray (vec3_t origin);//Tei

void R_Decal (vec3_t origin);

extern cvar_t gl_xrayblast;


sfx_t	*cl_sfx_wizhit;
sfx_t	*cl_sfx_knighthit;
sfx_t	*cl_sfx_tink1;
sfx_t	*cl_sfx_ric1;
sfx_t	*cl_sfx_ric2;
sfx_t	*cl_sfx_ric3;
sfx_t	*cl_sfx_r_exp3;
sfx_t	*cl_sfx_dland;//Tei dland

sfx_t	*cl_sfx_foot;//Tei dland


model_t	*cl_bolt1_mod;
model_t	*cl_bolt2_mod;
model_t	*cl_bolt3_mod;

// Tei fx mod
model_t	*cl_spx_mod;
model_t	*cl_star_mod;
// Tei fx mod

//Tei zing rain
sfx_t	*cl_zing_sfx;
sfx_t	*cl_zing2_sfx;
sfx_t	*cl_zing3_sfx;
//Tei zing rain

void R_RayFieldParticles (vec3_t org);
void R_BeamZing (vec3_t origin, vec3_t end);
void R_SparkShowerQ3 (vec3_t origin, vec3_t direction);
void R_SparkShowerQ3Fire (vec3_t origin, vec3_t direction);


/*
=================
CL_ParseTEnt
=================
*/
void CL_InitTEnts (void)
{
	cl_sfx_wizhit	= S_PrecacheSound ("wizard/hit.wav");
	cl_sfx_knighthit= S_PrecacheSound ("hknight/hit.wav");
	cl_sfx_tink1	= S_PrecacheSound ("weapons/tink1.wav");
	cl_sfx_ric1		= S_PrecacheSound ("weapons/ric1.wav");
	cl_sfx_ric2		= S_PrecacheSound ("weapons/ric2.wav");
	cl_sfx_ric3		= S_PrecacheSound ("weapons/ric3.wav");
	cl_sfx_r_exp3	= S_PrecacheSound ("weapons/r_exp3.wav");
#if 1
	//TELEJANO
	cl_zing_sfx			= 0;
	cl_zing2_sfx		= 0;
	cl_zing3_sfx		= 0;

	cl_zing_sfx			= S_PrecacheSound ("weapons/lhit.wav");//Tei zings
	cl_zing2_sfx		= S_PrecacheSound ("weapons/lstart.wav");//Tei zings
	cl_zing3_sfx		= S_PrecacheSound ("ambience/thunder1.wav");//Tei zings
	
	cl_sfx_dland	= S_PrecacheSound ("demon/dland2.wav");//Tei dland
	cl_sfx_foot		= S_PrecacheSound ("autofoot.wav");//Tei foot

#endif

	cl_bolt1_mod	= Mod_ForName ("progs/bolt.mdl", true);
	cl_bolt2_mod	= Mod_ForName ("progs/bolt2.mdl", true);
	cl_bolt3_mod	= Mod_ForName ("progs/bolt3.mdl", true);
	// Tei fx mod
	//cl_spx_mod		= Mod_ForName ("progs/s_explod.spr", true);
	//	cl_star_mod		= Mod_ForName ("progs/star2.mdl", true);// Break backward compatibility
	// Tei fx mod
}



void CL_ParsePRJ ();


/*
=================
CL_ParseBeam
=================
*/
void CL_ParseBeam (model_t *m, int style)
{
	int		ent;
	vec3_t	start, end;
	beam_t	*b;
	int		i;
	
	ent = MSG_ReadShort ();
	
	start[0] = MSG_ReadCoord ();
	start[1] = MSG_ReadCoord ();
	start[2] = MSG_ReadCoord ();
	
	end[0] = MSG_ReadCoord ();
	end[1] = MSG_ReadCoord ();
	end[2] = MSG_ReadCoord ();

// override any beam with the same entity
	for (i=0, b=cl_beams ; i< MAX_BEAMS ; i++, b++)
		if (b->entity == ent)
		{
			b->entity = ent;
			b->model = m;
			b->endtime = cl.time + 0.2;
			// Tei - Beam style
			b->style = style;
			// Tei - Beam style
			VectorCopy (start, b->start);
			VectorCopy (end, b->end);
			return;
		}

// find a free beam
	for (i=0, b=cl_beams ; i< MAX_BEAMS ; i++, b++)
	{
		if (!b->model || b->endtime < cl.time)
		{
			b->entity = ent;
			b->model = m;
			b->endtime = cl.time + 0.2;
			// Tei - Beam style
			b->style = style;
			// Tei - Beam style
			VectorCopy (start, b->start);
			VectorCopy (end, b->end);
			return;
		}
	}
	Con_Printf ("beam list overflow!\n");	
}

// Tei custom beam
void CL_ParseBeamString ()
{
	int		ent, style;
	vec3_t	start, end;
	beam_t	*b;
	int		i;
	char *name;
	model_t *m;
	ent = MSG_ReadShort ();
	
	start[0] = MSG_ReadCoord ();
	start[1] = MSG_ReadCoord ();
	start[2] = MSG_ReadCoord ();
	
	end[0] = MSG_ReadCoord ();
	end[1] = MSG_ReadCoord ();
	end[2] = MSG_ReadCoord ();

	style =	MSG_ReadByte();

	name = MSG_ReadString ();

	m = Mod_ForName (name, true);

// override any beam with the same entity
	for (i=0, b=cl_beams ; i< MAX_BEAMS ; i++, b++)
		if (b->entity == ent)
		{
			b->entity = ent;
			b->model = m;
			b->endtime = cl.time + 0.2;
	
			b->style = style;

			VectorCopy (start, b->start);
			VectorCopy (end, b->end);
			return;
		}

// find a free beam
	for (i=0, b=cl_beams ; i< MAX_BEAMS ; i++, b++)
	{
		if (!b->model || b->endtime < cl.time)
		{
			b->entity = ent;
			b->model = m;
			b->endtime = cl.time + 0.2;

			b->style = style;
			
			VectorCopy (start, b->start);
			VectorCopy (end, b->end);
			return;
		}
	}
	Con_Printf ("beam list overflow!\n");	
}
// Tei custom beam


// Q2! dl interface

void V_AddLight (vec3_t org, float intensity, float r, float g, float b)
{
	dlight_t	*dl;

	dl = CL_AllocDlight (0);
	if (!dl)
		return;

	VectorCopy (org, dl->origin);
	dl->radius = intensity;
	dl->color[0] = r;
	dl->color[1] = g;
	dl->color[2] = b;
	dl->decay = 100;
	dl->die = cl.time + 0.1;
}

// Q2! dl interface



/*
=================
CL_ParseTEnt
=================
*/
// Tei fx
void R_ParticleExplosion3 (vec3_t origin);// Rock dark explo 
void R_ParticleExplosion4 (vec3_t origin);// Misil dark explo
void R_ParticleExplosion5 (vec3_t origin);// "Nuke" dark explo
void R_ZTrail (vec3_t start, vec3_t end, vec3_t angle);
void R_ParticleImplosion (vec3_t origin);
void R_FireStaticFog (vec3_t  origin);
void R_ParticleBlast (vec3_t origin, vec3_t dir, int num,vec3_t color, byte scale);

void R_FireExplo (vec3_t  origin, float scale);
void DefineFlare(vec3_t origin, int radius, int mode, int alfa);//Tei dp flares
void R_ParticleTeExplosion (vec3_t origin);
void R_ParticleFire (vec3_t origin, int dx, int scale);
void R_ParticleExplosionX (vec3_t origin);
// Tei fx


// Tei oriented explosions

//Dont work as expected, FIXME!

void GetExploVector(vec3_t origin, vec3_t dir) 
{
#if 0 //TELEJANO - disabled to enable lan play
	int con;
	vec3_t down;
	vec3_t up; 
	vec3_t est;
	vec3_t west;
	vec3_t sur;
	vec3_t nort;

	int px = lhrandom(5,25);
	int dx = lhrandom(-1,1)* 20;

	


	if (cls.demoplayback)
		return;

	
	//VectorCopy(vec3_origin,dir);


	//Con_Printf("in: %f, %f, %f\n", dir[0],dir[1],dir[2]);

	VectorCopy ( origin, down);
	down[2] = - px;

	VectorCopy ( origin, up);
	up[2]	= px;

	VectorCopy ( origin, west);
	west[0]	= -px;

	VectorCopy ( origin, est);
	est[0]	= px;

	VectorCopy ( origin, sur);
	sur[1]	= -px;

	VectorCopy ( origin, nort);
	nort[1]	= px;



	VectorCopy ( vec3_origin, dir );

	con = SV_PointContents (down);//Mod_PointInLeaf(down, cl.worldmodel)->contents;
	if ( con == CONTENTS_SOLID )
		dir[2] = dir[2] - dx;

	con = SV_PointContents (up);//Mod_PointInLeaf(up, cl.worldmodel)->contents;
	if ( con == CONTENTS_SOLID )
		dir[2] = dir[2] + dx;

	con = SV_PointContents (west);//Mod_PointInLeaf(west, cl.worldmodel)->contents;
	if ( con == CONTENTS_SOLID )
		dir[0] = dir[0] + dx;

	con = SV_PointContents (est);//Mod_PointInLeaf(est, cl.worldmodel)->contents;
	if ( con == CONTENTS_SOLID )
		dir[0] = dir[0] - dx;

	con = SV_PointContents (sur);//Mod_PointInLeaf(sur, cl.worldmodel)->contents;
	if ( con == CONTENTS_SOLID )
		dir[1] = dir[1] + dx;

	con = SV_PointContents (nort);//Mod_PointInLeaf(nort, cl.worldmodel)->contents;
	if ( con == CONTENTS_SOLID )
		dir[1] = dir[1] - dx;

	//Con_Printf("ex: %f, %f, %f, pc:%d\n", dir[0],dir[1],dir[2], px);
#else
	VectorCopy(vec3_origin,dir);
#endif

}
// Tei oriented explosions


void R_SparkShowerQ2 (vec3_t origin, vec3_t direction);//Tei
void R_ParticleExplosionAlien (vec3_t origin);//Tei
void R_ParticleImpSuperSpike (vec3_t origin);
void R_ParticleSuperHazes (vec3_t origin);
void DefPRJ (	vec3_t  start,vec3_t  end, int type, int typelife, int dead, int scale, int key );

void AddParticle(vec3_t org, int count, float size, float time, int type);

extern cvar_t extra_fx;
typedef enum {
	p_sparks, p_smoke, p_fire, p_blood, p_chunks, p_smoke_fade, p_custom
} p_type_t;

void XR_ParticleExplosion (vec3_t org);


void Static_Hell_Here(vec3_t pos)
{
	Con_Printf("more hell..\n");
	DefPRJ(pos, pos, 1,1,43,1,2);//Tei for fun
		DefPRJ(pos, pos, 1,2,43,1,2);//Tei for fun
}

void CL_ParseTEnt (void)
{
	int		amount, type, rnd, colorStart, colorLength,t;
	vec3_t	pos, pos2, angle;//, start;
	dlight_t	*dl;
	// Tei custom FX
	int num		;
	int particle;
	int bounce	;
	int scale	;
	int die		;
	int typepar	;
	// Tei custom FX

	//Con_DPrintf("As arrived data..\n",type);//Tei debug

	type = MSG_ReadByte ();

	//Con_DPrintf("As arrived data..value %d\n",type);//Tei debug

	switch (type)
	{
	case TE_WIZSPIKE:			// spike hitting wall
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_RunParticleEffect (pos, vec3_origin, 20, 30);// Tei ...hummm
		S_StartSound (-1, 0, cl_sfx_wizhit, pos, 1, 1);
		break;
		
	case TE_KNIGHTSPIKE:			// spike hitting wall
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_RunParticleEffect (pos, vec3_origin, 226, 20);// Tei ...hummm
		S_StartSound (-1, 0, cl_sfx_knighthit, pos, 1, 1);
		break;
		
	case TE_SPIKE:			// spike hitting wall
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();

		if (extra_fx.value)
			DefPRJ(pos, pos, 10,3,43,1,2);
	
		//if (teirand(100)>95)
			AddParticle(pos,  3, 100, 1, p_sparks);//Tei QMB!

		GetExploVector( pos, pos2 );//Tei get dir of explosion
		R_SparkShowerQ2(pos, pos2);	// Tei redone "Tomaz - Particle System"

		if (teirand(100)<10)
			R_ParticleImpSuperSpike(pos);

		if (teirand(100)<4)
				R_Decal (pos);


		if ( rand() % 5 )
			S_StartSound (-1, 0, cl_sfx_tink1, pos, 1, 1);
		else
		{
			rnd = rand() & 3;
			if (rnd == 1)
				S_StartSound (-1, 0, cl_sfx_ric1, pos, 1, 1);
			else if (rnd == 2)
				S_StartSound (-1, 0, cl_sfx_ric2, pos, 1, 1);
			else
				S_StartSound (-1, 0, cl_sfx_ric3, pos, 1, 1);
		}
		break;

	case TE_PLASMA:			// spike hitting wall
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		S_StartSound (-1, 0, cl_sfx_wizhit, pos, 1, 1);
		break;

	case TE_SUPERSPIKE:			// super spike hitting wall
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		GetExploVector( pos, pos2 );//Tei get dir of explosion
		R_SparkShowerQ2(pos, pos2);	// Tei redone "Tomaz - Particle System"
		R_ParticleImpSuperSpike(pos);

		if (extra_fx.value)
			DefPRJ(pos, pos, 10,3,43,1,2);
	
		//if (teirand(100)>95)
			AddParticle(pos,  32, 100, 1, p_sparks);//Tei QMB!

		if (teirand(100)<5)
			R_ParticleSuperHazes(pos);
		
		if ( rand() % 5 )
			S_StartSound (-1, 0, cl_sfx_tink1, pos, 1, 1);	
		else
		{
			rnd = rand() & 3;
			if (rnd == 1)
				S_StartSound (-1, 0, cl_sfx_ric1, pos, 1, 1);
			else if (rnd == 2)
				S_StartSound (-1, 0, cl_sfx_ric2, pos, 1, 1);
			else
				S_StartSound (-1, 0, cl_sfx_ric3, pos, 1, 1);
		}
		break;

// Tomaz - Particle System Begin
	case TE_SNOW:
		pos[0] = MSG_ReadCoord (); // mins
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		pos2[0] = MSG_ReadCoord (); // maxs
		pos2[1] = MSG_ReadCoord ();
		pos2[2] = MSG_ReadCoord ();
		amount	= MSG_ReadShort ();
		R_Snow(pos, pos2, amount);
		break;
		
	case TE_RAIN:
		pos[0]	= MSG_ReadCoord (); // mins
		pos[1]	= MSG_ReadCoord ();
		pos[2]	= MSG_ReadCoord ();
		pos2[0] = MSG_ReadCoord (); // maxs
		pos2[1] = MSG_ReadCoord ();
		pos2[2] = MSG_ReadCoord ();
		amount	= MSG_ReadShort ();
		R_Rain(pos, pos2, amount);
		break;			
// Tomaz - Particle System End

	case TE_GUNSHOT:			// bullet hitting wall
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();

		
		//return; //Tei debug
		
		GetExploVector( pos, pos2 );//Tei get dir of explosion
		R_SparkShower(pos, pos2);	// Tei redone "Tomaz - Particle System"

		//if (extra_fx.value)
		//	DefPRJ(pos, pos, 10,3,43,1,2);

		if (teirand(100)>95)
			AddParticle(pos,  4, 100, 1, p_sparks);//Tei QMB!

		//R_ParticleImpSuperSpike(pos);
		if (teirand(100)<4)
			R_ParticleImpSuperSpike(pos);
		break;
		
	case TE_EXPLOSION:			// rocket explosion

		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
#if 1 //TELEJANO


		XR_ParticleExplosion( pos );//Tei QMB!
		
		//Tei xfx
		cl.punchangle[0] += 1;
		cl.cshifts[CSHIFT_BONUS].destcolor[0] = 115;
		cl.cshifts[CSHIFT_BONUS].destcolor[1] = 126;
		cl.cshifts[CSHIFT_BONUS].destcolor[2] = 212;
		cl.cshifts[CSHIFT_BONUS].percent += 10;
		//Tei xfx



		if (extra_fx.value)
			DefPRJ(pos, pos, 10,3,43,1,2);

		if (extra_fx.value > 5)
			R_ParticleExplosion4(pos);//INSANE!

		R_ParticleTeExplosion (pos);
		R_ParticleExplosionX (pos);
		//R_ParticleExplosion4 (pos);

		//R_FireExploHaze(pos,1);

		/*
		GetExploVector( pos, pos2 );//Tei get dir of explosion
		R_SparkShowerQ2(pos, pos2);	// Tei redone "Tomaz - Particle System"
		R_SparkShowerQ2(pos, pos2);	// Tei redone "Tomaz - Particle System"
		R_SparkShowerQ2(pos, pos2);	// Tei redone "Tomaz - Particle System"

		*/
		//R_SparkShower(pos, pos);
		dl = CL_AllocDlight (0);
		VectorCopy (pos, dl->origin);
		dl->radius = 500;//Tei 300->800
		dl->die = cl.time + 0.5;
		dl->decay = 200;//Tei 300 -> 200
		S_StartSound (-1, 0, cl_sfx_r_exp3, pos, 1, 1);
		//R_FireExplo (pos, 2);//Tei
		for (t=0;t<12;t++)
			R_ParticleFire ( pos ,28, lhrandom(3,12));
		DefineFlare(pos,300, 0,320);//Tei dp flare

		//GetExploVector( pos, pos2 );//Tei get dir of explosion
		//R_SparkShowerQ3Fire( pos, pos2 );//XFX



		//void R_XDrawGlows (vec3_t lightorigin, float radius, vec3_t glow_color)
		//Tei glow explosion
		//Tei glow explosion
#endif
		break;

// Tomaz - Particle System Begin		
	case TE_TAREXPLOSION:			// tarbaby explosion
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_ParticleExplosion (pos);
		dl = CL_AllocDlight (0);
		VectorCopy (pos, dl->origin);
		dl->color[0]	= 1;
		dl->color[1]	= 0;
		dl->color[2]	= 1;
		dl->radius		= 300;
		dl->die			= cl.time + 0.5;
		dl->decay		= 300;
		S_StartSound (-1, 0, cl_sfx_r_exp3, pos, 1, 1);
		break;
// Tomaz - Particle System End

	case TE_LIGHTNING1:				// lightning bolts
		CL_ParseBeam (cl_bolt1_mod, BE_NORMAL);
		break;
	
	case TE_LIGHTNING2:				// lightning bolts
		CL_ParseBeam (cl_bolt2_mod, BE_NORMAL);
		break;
	
	case TE_LIGHTNING3:				// lightning bolts
		CL_ParseBeam (cl_bolt3_mod, BE_NORMAL);
		break;
	
	// Tei
	case TE_LIGHTNING4:				// lightning bolts
		CL_ParseBeam (Mod_ForName ("progs/star2.mdl", true), BE_NORMAL);//cl_spx_mod);//cl_bolt2_mod);
		break;
	case TE_LIGHTNINGX:				// lightning bolts		
		CL_ParseBeamString ();//cl_spx_mod);//cl_bolt2_mod);
		break;
	// Tei
		
// PGM 01/21/97 
	case TE_BEAM:				// grappling hook beam
		CL_ParseBeam(Mod_ForName ("progs/beam.mdl", true), BE_NORMAL);
		break;
// PGM 01/21/97

	case TE_LAVASPLASH:	
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_LavaSplash (pos);
		break;
	
	case TE_TELEPORT:
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_TeleportSplash (pos);
		DefineFlare(pos,2000, 0,100);//Tei nuke flash teleport

		if (extra_fx.value)
			DefPRJ(pos, pos, 11,3,43,1,2);//Tei extra fx
		break;

	case TE_EXPLOSION2:				// color mapped explosion
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		colorStart = MSG_ReadByte ();
		colorLength = MSG_ReadByte ();
		R_ParticleExplosion2 (pos, colorStart, colorLength);
		dl = CL_AllocDlight (0);
		VectorCopy (pos, dl->origin);
		dl->radius	= 300;
		dl->die		= cl.time + 0.5;
		dl->decay	= 300;
		S_StartSound (-1, 0, cl_sfx_r_exp3, pos, 1, 1);
		break;

// Tomaz - RailTrail Begin
	case TE_RAILTRAIL: 
		pos[0] = MSG_ReadCoord();
		pos[1] = MSG_ReadCoord();
		pos[2] = MSG_ReadCoord();
		pos2[0] = MSG_ReadCoord();
		pos2[1] = MSG_ReadCoord();
		pos2[2] = MSG_ReadCoord();
		angle[0] = MSG_ReadCoord();
		angle[1] = MSG_ReadCoord();
		angle[2] = MSG_ReadCoord();
		R_RailTrail (pos, pos2, angle);
		break;
// Tomaz - RailTrail End		
// Tei - custom FX
	case TE_CUSTOMFX: //Not work
		pos[0] = MSG_ReadCoord();
		pos[1] = MSG_ReadCoord();
		pos[2] = MSG_ReadCoord();
		pos2[0] = MSG_ReadCoord();
		pos2[1] = MSG_ReadCoord();
		pos2[2] = MSG_ReadCoord();
		angle[0] = MSG_ReadCoord();
		angle[1] = MSG_ReadCoord();
		angle[2] = MSG_ReadCoord();
		num			= MSG_ReadByte ();
		particle	= MSG_ReadByte ();
		bounce		= MSG_ReadByte ();
		scale		= MSG_ReadByte ();
		die			= MSG_ReadByte ();
		typepar		= MSG_ReadByte ();
		//void R_CustomFX (vec3_t origin,vec3_t color,vec3_t veloc , int num, int particle, int bounce, int scale, int die, int type )

		R_CustomFX (pos, pos2, angle, num,particle,bounce,scale,die,type);
		break;
// Tei - custom FX
	case TE_EXPLOSIONSMALL:			// rocket explosion
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_ParticleExplosion3 (pos);//KiU
		//R_BlobExplosion(pos);
		dl = CL_AllocDlight (0);
		VectorCopy (pos, dl->origin);
		dl->radius = 20;
		dl->die = cl.time + 0.1;
		dl->decay = 100;
		S_StartSound (-1, 0, cl_sfx_r_exp3, pos, 1, 1);
		break;
	case TE_EXPLOSIONSMALL2:			// rocket explosion
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_ParticleExplosion4 (pos);
		//R_BlobExplosion(pos);
		dl = CL_AllocDlight (0);
		VectorCopy (pos, dl->origin);
		dl->radius = 80;
		dl->die = cl.time + 0.1;
		dl->decay = 200;
		S_StartSound (-1, 0, cl_sfx_r_exp3, pos, 1, 1);
		break;

	case TE_EXPLOSIONSMALL3:			// rocket explosion2
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_ParticleExplosion5 (pos);
		//R_BlobExplosion(pos);
		dl = CL_AllocDlight (0);
		VectorCopy (pos, dl->origin);
		dl->radius = 280;
		dl->die = cl.time + 0.3;
		dl->decay = 200;
		S_StartSound (-1, 0, cl_sfx_r_exp3, pos, 1, 1);
		break;
	// Tei custom fx

	case TE_ALIENEXPLOSION:			
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_ParticleExplosionAlien (pos);
		//R_BlobExplosion(pos);
		dl = CL_AllocDlight (0);
		VectorCopy (pos, dl->origin);
		dl->radius = 80;
		dl->die = cl.time + 0.1;
		dl->decay = 200;
		S_StartSound (-1, 0, cl_sfx_r_exp3, pos, 1, 1);
		break;


	///////////////////
	// FH!
	/*
	case TE_RADARBLIP:
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		start[0] = MSG_ReadCoord ();
		start[1] = MSG_ReadCoord ();
		start[2] = MSG_ReadCoord ();
		R_Radar(pos, start);
		break;
	*/
	//
	////////////////////

	case TE_TRAILS: 
		pos[0] = MSG_ReadCoord();
		pos[1] = MSG_ReadCoord();
		pos[2] = MSG_ReadCoord();
		pos2[0] = MSG_ReadCoord();
		pos2[1] = MSG_ReadCoord();
		pos2[2] = MSG_ReadCoord();
		angle[0] = MSG_ReadCoord();
		angle[1] = MSG_ReadCoord();
		angle[2] = MSG_ReadCoord();

		R_ZTrail (pos, pos2, angle);// Zoomes trail
		break;
	/*
	case TE_SHOCKWAVE:
		pos[0] = MSG_ReadCoord ();
		pos[1] = MSG_ReadCoord ();
		pos[2] = MSG_ReadCoord ();
		R_Shock1 (pos, 1);
	*/

	// Tei - prj
	case TE_PRJ:
		//Con_Printf ("PRJ init\n");	
		CL_ParsePRJ();
		break;
	// Tei - prj

	case TE_IMPLOSIONFX:
		pos[0] = MSG_ReadCoord();
		pos[1] = MSG_ReadCoord();
		pos[2] = MSG_ReadCoord();
		R_ParticleImplosion (pos);
		break;

	case TE_STATICFOG:
		pos[0] = MSG_ReadCoord();
		pos[1] = MSG_ReadCoord();
		pos[2] = MSG_ReadCoord();

		R_FireStaticFog(pos);
		break;

	
	//Tei particle blast
	case TE_PARTICLEBLAST:
		pos[0] = MSG_ReadCoord();
		pos[1] = MSG_ReadCoord();
		pos[2] = MSG_ReadCoord();

		pos2[0] = MSG_ReadCoord();
		pos2[1] = MSG_ReadCoord();
		pos2[2] = MSG_ReadCoord();

		angle[0] = MSG_ReadCoord();
		angle[1] = MSG_ReadCoord();
		angle[2] = MSG_ReadCoord();

		num			= MSG_ReadByte ();

		scale		= MSG_ReadByte ();
	

		R_ParticleBlast(pos,pos2, num, angle, (byte)scale);
		//void R_ParticleBlast (vec3_t origin, vec3_t dir, int num,vec3_t color)
		break;
	//Tei particle blast
	
	default:
		Sys_Error ("CL_ParseTEnt: bad type");
	}

	Con_DPrintf("As arrived data..value %d, Ok TENT\n",type);//Tei debug
}


/*
=================
CL_NewTempEntity
=================
*/
entity_t *CL_NewTempEntity (void)
{
	entity_t	*ent;

	if (cl_numvisedicts == MAX_VISEDICTS)
		return NULL;
	if (num_temp_entities == MAX_TEMP_ENTITIES)
		return NULL;
	ent = &cl_temp_entities[num_temp_entities];
	memset (ent, 0, sizeof(*ent));
	num_temp_entities++;
	cl_visedicts[cl_numvisedicts] = ent;
	cl_numvisedicts++;

	ent->colormap = vid.colormap;
	return ent;
}

// Tei StaticFX
void CL_RunPRJ();
// Tei StaticFX


void XR_BlobExplosion2 (vec3_t org);

/*
=================
CL_UpdateTEnts
=================
*/

void CL_UpdateTEnts (void)
{
	int			i;
	beam_t		*b;
	vec3_t		dist, org, dxdir,orgold;
	float		d;
	entity_t	*ent;
	float		yaw, pitch;// , distx;
	float		forward;
	int BEAM_DENSITY;
//	trace_t		trace;
	dlight_t	*dl;
	vec3_t		iniorg, iniend;
	int			factorcompress;


	num_temp_entities = 0;
	
	BEAM_DENSITY = 20;//

// update lightning
	for (i=0, b=cl_beams ; i< MAX_BEAMS ; i++, b++)
	{
			
		if (!b->model || b->endtime < cl.time)
			continue;

		if (b->entity == cl.viewentity)
		{
			VectorCopy( b->start, dxdir);//Tei smooth ray
			VectorCopy (cl_entities[cl.viewentity].origin, b->start);
			VectorSubtract (b->start, dxdir, dxdir);//Tei smooth ray
		    VectorAdd( b->end, dxdir, b->end);//Tei smooth ray
		}

		//VectorCopy (cl_entities[b->entity].origin, b->start);
		VectorSubtract (b->end, b->start, dist);
		
 

#if 1 //TELEJANO
		//Tei eyecandy
		
		//GetExploVector(  b->end, dir );//Tei get dir of explosion
		R_SparkShowerQ3( b->end, vec3_origin );
		//DefineFlareColor(b->start,200, 0,130,0.4,0.7,0.9);//Tei dp flare

		if (extra_fx.value)
			if (teirand(100)>95)
				XR_BlobExplosion2(b->end);//Tei QMB!

		//Tei XFX 
		if (gl_xrayblast.value) {
				R_ParticleImplosionXray (b->end);			
				R_RayFieldParticles(b->end);
		dl = CL_AllocDlight (0);
		VectorCopy (b->end, dl->origin);
		dl->radius = 300;//Tei 300->800
		dl->die = cl.time + 0.5;
		dl->decay = 200;//Tei 300 -> 200
		if (extra_fx.value)
		if( cls.state == ca_connected && !cls.demoplayback)
			{
			//if (SV_PointContents(b->end )!= CONTENTS_EMPTY )//Tei will crash a client in lan
				if(lhrandom(0,100)<2)//TODO: this need to be framesincro
							DefPRJ(b->end, b->end, 9,3,43,1,2);
			}
			else
				DefPRJ(b->end, b->end, 9,3,43,1,2);
		}
		//Tei eyecandy ray
#endif

		if (dist[1] == 0 && dist[0] == 0)
		{
			yaw = 0;
			if (dist[2] > 0)
				pitch = 90;
			else
				pitch = 270;
		}
		else
		{
			yaw = (int) (atan2(dist[1], dist[0]) * 57.295779513082320);	// Tomaz Speed
			if (yaw < 0)
				yaw += 360;
	
			forward = sqrt (dist[0]*dist[0] + dist[1]*dist[1]);
			pitch = (int) (atan2(dist[2], forward) * 57.295779513082320);	// Tomaz Speed
			if (pitch < 0)
				pitch += 360;
		}

	// add new entities for the lightning
		VectorCopy (b->start, org);
		d = VectorNormalize(dist);
		
		//R_BeamZing(b->start, b->end);//XFX

		if (rand()&1)
		while (d > 0)
		{

			ent = CL_NewTempEntity ();
			if (!ent)
				return;
			VectorCopy (org, ent->origin);
			ent->model = b->model;
			ent->angles[0] = pitch;
			ent->angles[1] = yaw;

			//ent->alpha = 10;//Tei
			//ent->effects3 = EF3_HIPERTRANS2;//Tei

			// Tei - beam rotate
			if (b->style & BE_ROTATE )
				ent->angles[2] = 0;
			else
				ent->angles[2] = rand()%360;
			// Tei - beam rotate
		
			VectorCopy(org,orgold);//Tei improbe xray

			// Tei - beam multiray
			if (b->style & BE_MULTIRAY) 
			{
				org[0] += dist[0]*(BEAM_DENSITY +(rand()&7)-7);//30;
				org[1] += dist[1]*(BEAM_DENSITY +(rand()&7)-7);//30;
				org[2] += dist[2]*(BEAM_DENSITY +(rand()&7)-7);//30;
			}
			else 
			{
				org[0] += dist[0]*30;
				org[1] += dist[1]*30;
				org[2] += dist[2]*30;
			}
			// Tei - beam multiray
			
				
			// Tei - beam compress
			if (b->style & BE_SPANXIVE)
				d -= BEAM_DENSITY;
			else
				d -= 30;//30 xfx
			// Tei - beam compress

		}



		VectorCopy(b->start, org);
		VectorSubtract (b->end, b->start, dist);
		d = VectorNormalize(dist);

		//R_BeamZing(b->start, b->end);//XFX
		VectorCopy(org,iniend);

		while (d > 0)
		{

			VectorCopy(org,iniorg);

			factorcompress =lhrandom(2,160);

			org[0] += dist[0]*factorcompress;//*lhrandom(1,3);
			org[1] += dist[1]*factorcompress;//*lhrandom(1,3);
			org[2] += dist[2]*factorcompress;//*lhrandom(1,3);

			if (teirand(100)<10)
			{
				R_BeamZing(org, iniend);//XFX
				VectorCopy(org,iniend);
			}

			d -= factorcompress;
		}
		R_BeamZing(org, iniend);//XFX
	
	}
	
	
	// Tei - PRJ
	CL_RunPRJ();
	// Tei - PRJ
	
}



// Tei - prj fx

void CL_AddLight( vec3_t pos, float scale ) 
{
		dlight_t	*dl;

		dl = CL_AllocDlight (0);
		VectorCopy (pos, dl->origin);
		dl->radius = scale;
		dl->die = cl.time + scale / 100;
		dl->decay = 200;
}

// Tei - prj fx


void R_SuperZing (vec3_t start, vec3_t end)
{
	//int			i;
//	beam_t		*b;
	vec3_t		dist, org;//, orgold;
	float		d;
//	trace_t		trace;
	vec3_t		iniorg, iniend;
	int			factorcompress;


	VectorCopy( start, org);
	VectorSubtract (end, start, dist);
	d = VectorNormalize(dist);

	VectorCopy(org,iniend);

	while (d > 0)
	{

		VectorCopy(org,iniorg);

		factorcompress =lhrandom(2,160);

		org[0] += dist[0]*factorcompress;//*lhrandom(1,3);
		org[1] += dist[1]*factorcompress;//*lhrandom(1,3);
		org[2] += dist[2]*factorcompress;//*lhrandom(1,3);
		if (teirand(100)<10)
		{
				R_BeamZing(org, iniend);//XFX
				VectorCopy(org,iniend);
		}

		d -= factorcompress;
	}
	R_BeamZing(org, iniend);//XFX

}








