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
// cl_main.c  -- client main loop

#include "quakedef.h"

// Tei new fx
// Here, becaus if you change .h all rebuilt..

void R_TempusVivendi (vec3_t min, vec3_t max, int flakes);
void R_TempusFire (vec3_t min, vec3_t max, int flakes);
void R_TempusSmoke(vec3_t min, vec3_t max, int flakes);
void R_ShadowParticles (entity_t *ent);
void R_MagicFire (entity_t *ent, qboolean fire2);
void R_TempusDarkSmoke(vec3_t min, vec3_t max, int flakes);
void R_DarkRocketTrail (vec3_t start, vec3_t end, entity_t *ent, qboolean subtype);
void R_SmallDowFire (entity_t *ent, qboolean fire2);
void R_CircleRocketTrail (vec3_t start, vec3_t end, entity_t *ent);
void R_ParticleImplosion (vec3_t origin);
void R_TrailGray2 (entity_t * e);
void R_TrailGray3 (entity_t * e);
void R_Fire2 (entity_t *ent, qboolean fire2);
void R_FireBack (entity_t *ent, qboolean fire2);
void DefineFlare(vec3_t origin, int radius, int mode, int alfa);//Tei dp flares
// Tei new fx



typedef enum {
	pt_static, pt_grav, pt_slowgrav, pt_rise, pt_slowrise, pt_explode, pt_explode2, pt_blob, pt_blob2, pt_fire
} part_move_t;

typedef enum {
	p_sparks, p_smoke, p_fire, p_blood, p_chunks, p_smoke_fade, p_custom
} p_type_t;


// we need to declare some mouse variables here, because the menu system
// references them even when on a unix system.

// these two are not intended to be set directly
cvar_t	cl_name			= {"_cl_name", "player", true};
cvar_t	cl_color		= {"_cl_color", "0", true};

cvar_t	cl_shownet		= {"cl_shownet","0"};	// can be 0, 1, or 2
cvar_t	cl_nolerp		= {"cl_nolerp","0"};

cvar_t	cl_grenclassic		= {"cl_grenclassic", "0", true};//Tei grensmoke color

cvar_t	lookspring		= {"lookspring","0", true};
cvar_t	lookstrafe		= {"lookstrafe","0", true};
cvar_t	sensitivity		= {"sensitivity","3", true};

cvar_t	m_pitch			= {"m_pitch","0.022", true};
cvar_t	m_yaw			= {"m_yaw","0.022", true};
cvar_t	m_forward		= {"m_forward","1", true};
cvar_t	m_side			= {"m_side","0.8", true};

// Tei autofov
cvar_t	autofovmax		= {"autofovmax","50", true};
// Tei autofov


// Tei mod_
cvar_t	mod_predator			= {"mod_predator","0", false};
cvar_t	mod_lsig			= {"mod_lsig","0", false};
cvar_t	mod_focus			= {"mod_focus","0", false};

cvar_t	mod_progs				= {"mod_progs","game.dat", false};

cvar_t	mod_extendedparticle	= {"mod_extendedparticle","1", true};
cvar_t	mod_showlight			= {"mod_showlight","0", false};
cvar_t	mod_cityofangels			= {"mod_cityofangels","0", false};

cvar_t	gamedir				= {"gamedir","id1", false};//dave

//cvar_t	mod_boltclassic = {"mod_boltclassic","0", true};
// Tei mod_

cvar_t	r_autofluor				= {"r_autofluor","2", false};//Tei autofluor 0/1/2/3/4



client_static_t	cls;
client_state_t	cl;

// FIXME: put these on hunk?
efrag_t			cl_efrags[MAX_EFRAGS];
entity_t		cl_entities[MAX_EDICTS];
entity_t		cl_static_entities[MAX_STATIC_ENTITIES];
lightstyle_t	cl_lightstyle[MAX_LIGHTSTYLES];
dlight_t		cl_dlights[MAX_DLIGHTS];

int				cl_numvisedicts;
entity_t		*cl_visedicts[MAX_VISEDICTS];

/*
=====================
CL_ClearState

=====================
*/
//void CL_ClearTEnts (void);
void CL_ClearState (void)
{
	int			i;

	if (!sv.active)
		Host_ClearMemory ();

// wipe the entire cl structure
	memset (&cl, 0, sizeof(cl));

	SZ_Clear (&cls.message);

// clear other arrays	
	memset (cl_efrags,			0, sizeof(cl_efrags));
	memset (cl_entities,		0, sizeof(cl_entities));
	memset (cl_dlights,			0, sizeof(cl_dlights));
	memset (cl_lightstyle,		0, sizeof(cl_lightstyle));
	memset (cl_temp_entities,	0, sizeof(cl_temp_entities));
	memset (cl_beams,			0, sizeof(cl_beams));
	//Q2!
	//CL_ClearTEnts();
	//Q2!
//
// allocate the efrags and chain together into a free list
//
	cl.free_efrags = cl_efrags;
	for (i=0 ; i<MAX_EFRAGS-1 ; i++)
		cl.free_efrags[i].entnext = &cl.free_efrags[i+1];
	cl.free_efrags[i].entnext = NULL;
}

/*
=====================
CL_Disconnect

Sends a disconnect message to the server
This is also called on Host_Error, so it shouldn't cause any errors
=====================
*/
void CL_Disconnect (void)
{
// stop sounds (especially looping!)
	S_StopAllSounds (true);

// bring the console down and fade the colors back to normal
//	SCR_BringDownConsole ();

// if running a local server, shut it down
	if (cls.demoplayback)
		CL_StopPlayback ();
	else if (cls.state == ca_connected)
	{
		if (cls.demorecording)
			CL_Stop_f ();

		Con_DPrintf ("Sending clc_disconnect\n");
		SZ_Clear (&cls.message);
		MSG_WriteByte (&cls.message, clc_disconnect);
		NET_SendUnreliableMessage (cls.netcon, &cls.message);
		SZ_Clear (&cls.message);
		NET_Close (cls.netcon);

		cls.state = ca_disconnected;
		if (sv.active)
			Host_ShutdownServer(false);
	}

	cls.demoplayback = cls.timedemo = false;
	cls.signon = 0;
}

void CL_Disconnect_f (void)
{
	CL_Disconnect ();
	if (sv.active)
		Host_ShutdownServer (false);
}




/*
=====================
CL_EstablishConnection

Host should be either "local" or a net address to be passed on
=====================
*/
void CL_EstablishConnection (char *host)
{
	if (cls.state == ca_dedicated)
		return;

	if (cls.demoplayback)
		return;

	CL_Disconnect ();

	cls.netcon = NET_Connect (host);
	if (!cls.netcon)
		Host_Error ("CL_Connect: connect failed\n");
	Con_DPrintf ("CL_EstablishConnection: connected to %s\n", host);
	
	cls.demonum = -1;			// not in the demo loop now
	cls.state = ca_connected;
	cls.signon = 0;				// need all the signon messages before playing
}

/*
=====================
CL_SignonReply

An svc_signonnum has been received, perform a client side setup
=====================
*/
void CL_SignonReply (void)
{
	char 	str[8192];

Con_DPrintf ("CL_SignonReply: %i\n", cls.signon);

	switch (cls.signon)
	{
	case 1:
		MSG_WriteByte (&cls.message, clc_stringcmd);
		MSG_WriteString (&cls.message, "prespawn");

		Con_DPrintf("prespawn fase...\n");
		break;
		
	case 2:		
		MSG_WriteByte (&cls.message, clc_stringcmd);
		MSG_WriteString (&cls.message, va("name \"%s\"\n", cl_name.string));
	
		MSG_WriteByte (&cls.message, clc_stringcmd);
		MSG_WriteString (&cls.message, va("color %i %i\n", ((int)cl_color.value)>>4, ((int)cl_color.value)&15));
	
		MSG_WriteByte (&cls.message, clc_stringcmd);
		sprintf (str, "spawn %s", cls.spawnparms);
		MSG_WriteString (&cls.message, str);

		Con_DPrintf("spawn fase...\n");
		break;
		
	case 3:	
		MSG_WriteByte (&cls.message, clc_stringcmd);
		MSG_WriteString (&cls.message, "begin");
		Cache_Report ();		// print remaining memory
		Con_DPrintf("Cache report...\n");//Tei debugging
		break;
		
	case 4:
		SCR_EndLoadingPlaque ();		// allow normal screen updates
		Con_DPrintf("Loading plaque...\n");//Tei debugging
		break;
	}
}

/*
=====================
CL_NextDemo

Called to play the next demo in the demo loop
=====================
*/
void CL_NextDemo (void)
{
	char	str[1024];

	if (cls.demonum == -1)
		return;		// don't play demos

	SCR_BeginLoadingPlaque ();

	if (!cls.demos[cls.demonum][0] || cls.demonum == MAX_DEMOS)
	{
		cls.demonum = 0;
		if (!cls.demos[cls.demonum][0])
		{
			Con_Printf ("No demos listed with startdemos\n");
			cls.demonum = -1;
			return;
		}
	}

	sprintf (str,"playdemo %s\n", cls.demos[cls.demonum]);
	Cbuf_InsertText (str);
	cls.demonum++;
}

/*
==============
CL_PrintEntities_f
==============
*/
void CL_PrintEntities_f (void)
{
	entity_t	*ent;
	int			i;
	
	for (i=0,ent=cl_entities ; i<cl.num_entities ; i++,ent++)
	{
		Con_Printf ("%3i:",i);
		if (!ent->model)
		{
			Con_Printf ("EMPTY\n");
			continue;
		}
		Con_Printf ("%s:%2i  (%5.1f,%5.1f,%5.1f) [%5.1f %5.1f %5.1f]\n"
		,ent->model->name,ent->frame, ent->origin[0], ent->origin[1], ent->origin[2], ent->angles[0], ent->angles[1], ent->angles[2]);
	}
}

/*
===============
CL_AllocDlight
===============
*/
dlight_t *CL_AllocDlight (int key)
{
	int			i;
	dlight_t	*dl;

// first look for an exact key match
	if (key)
	{
		dl = cl_dlights;
		for (i=0 ; i<MAX_DLIGHTS ; i++, dl++)
		{
			if (dl->key == key)
			{
				memset (dl, 0, sizeof(*dl));
				dl->key = key;

				// Tomaz - Lit Support Begin
				dl->color[0] = 1;
				dl->color[1] = 0.5;
				dl->color[2] = 0.25;
				// Tomaz - Lit Support End

				return dl;
			}
		}
	}

// then look for anything else
	dl = cl_dlights;
	for (i=0 ; i<MAX_DLIGHTS ; i++, dl++)
	{
		if (dl->die < cl.time)
		{
			memset (dl, 0, sizeof(*dl));
			dl->key = key;

			// Tomaz - Lit Support Begin
			dl->color[0] = 1;
			dl->color[1] = 0.5;
			dl->color[2] = 0.25;
			// Tomaz - Lit Support End

			return dl;
		}
	}

	dl = &cl_dlights[0];
	memset (dl, 0, sizeof(*dl));
	dl->key = key;
	
	// Tomaz - Lit Support Begin
	dl->color[0] = 1;
	dl->color[1] = 0.5;
	dl->color[2] = 0.25;
	// Tomaz - Lit Support End

	return dl;
}


/*
===============
CL_DecayLights
===============
*/
void CL_DecayLights (void)
{
	int			i;
	dlight_t	*dl;
	float		time;
	
	time = cl.time - cl.oldtime;

	dl = cl_dlights;
	for (i=0 ; i<MAX_DLIGHTS ; i++, dl++)
	{
		if (dl->die < cl.time || !dl->radius)
			continue;
		
		dl->radius -= time*dl->decay;
		if (dl->radius < 0)
			dl->radius = 0;
	}
}

/*
===============
CL_LerpPoint

Determines the fraction between the last two messages that the objects
should be put at.
===============
*/
float	CL_LerpPoint (void)
{
	float	f, frac;

	f = cl.mtime[0] - cl.mtime[1];
	
	if (!f || cl_nolerp.value || cls.timedemo || sv.active)
	{
		cl.time = cl.mtime[0];
		return 1;
	}
		
	if (f > 0.1)
	{	// dropped packet, or start of demo
		cl.mtime[1] = cl.mtime[0] - 0.1;
		f = 0.1f;
	}
	frac = (cl.time - cl.mtime[1]) / f;
//Con_Printf ("frac: %f\n",frac);
	if (frac < 0)
	{
		if (frac < -0.01)
		{
			cl.time = cl.mtime[1];
//				Con_Printf ("low frac\n");
		}
		frac = 0;
	}
	else if (frac > 1)
	{
		if (frac > 1.01)
		{
			cl.time = cl.mtime[0];
//				Con_Printf ("high frac\n");
		}
		frac = 1;
	}
	return frac;
}


/*
===============
CL_RelinkEntities
===============
*/
void R_TrailGray (entity_t * ent);
void R_GrenadeTrail (vec3_t start, vec3_t end, entity_t *ent);
void R_TracerWizard (vec3_t start, vec3_t end, entity_t *ent, byte color);
void R_FireRocketTrailX (entity_t *ent, qboolean fire2);
void R_TracerKTrail (vec3_t start, vec3_t end, entity_t *ent, byte color);
int dxvector (vec3_t *vec1, vec3_t * vec2);
void AddTrail(vec3_t start, vec3_t end, int type, float time);
void R_BeamZing (vec3_t origin, vec3_t end);

extern cvar_t temp1;

void CL_RelinkEntities (void)
{
	entity_t	*ent;
	int			i, j;
	float		frac, f, d;
	vec3_t		delta;
	float		bobjrotate;
	dlight_t	*dl;
	int			effects, effects2, effects3;
	vec3_t		oldorg,downmove,oldorg2;//, color;

// determine partial update time	
	frac = CL_LerpPoint ();

	cl_numvisedicts = 0;

//
// interpolate player info
//
	cl.velocity[0] = cl.mvelocity[1][0] + frac * (cl.mvelocity[0][0] - cl.mvelocity[1][0]);
	cl.velocity[1] = cl.mvelocity[1][1] + frac * (cl.mvelocity[0][1] - cl.mvelocity[1][1]);
	cl.velocity[2] = cl.mvelocity[1][2] + frac * (cl.mvelocity[0][2] - cl.mvelocity[1][2]);

	if (cls.demoplayback)
	{
	// interpolate the angles	
		for (j=0 ; j<3 ; j++)
		{
			d = cl.mviewangles[0][j] - cl.mviewangles[1][j];
			if (d > 180)
				d -= 360;
			else if (d < -180)
				d += 360;
			cl.viewangles[j] = cl.mviewangles[1][j] + frac*d;
		}
	}
	
	bobjrotate = anglemod(100*cl.time);
	
// start on the entity after the world
	for (i=1,ent=cl_entities+1 ; i<cl.num_entities ; i++,ent++)
	{
		if (!ent->model)
		{	// empty slot
			if (ent->forcelink)
				R_RemoveEfrags (ent);	// just became empty
			continue;
		}

// if the object wasn't included in the last packet, remove it
		if (ent->msgtime != cl.mtime[0])
		{

			/*
			//XFX!
			if (ent->alpha ==0)
			{	
				Con_Printf ("more!\n");	
				ent->alpha =1;
				continue;
			}
			else
			{				
				ent->alpha = ent->alpha * 0.6;
				if (ent->alpha > 0.1)
					continue;
			}	
			//XFX!
			*/
			//	Con_Printf ("more!\n");	



			ent->model = NULL;
			// Tomaz - Model Transform Interpolation Begin
			ent->translate_start_time = 0;
			ent->rotate_start_time    = 0;
			VectorClear (ent->last_light);
			// Tomaz - Model Transform Interpolation End

			// Tei
			//if (ent->effects3 == 23 )
			//	Con_Printf ("PRJ deleted\n");	
			// Tei

			continue;
		}


		if (ent->model->effect == MFX_LASER && dxvector ((vec3_t *)ent->origin,(vec3_t *) ent->oldorg)<100)
			R_BeamZing(ent->origin, ent->oldorg);//XFX XFX !!!


#if 0 //Tei pre4 code
		VectorCopy (ent->origin, ent->oldorg);
		VectorCopy (ent->origin, oldorg);

#else //Tei v4 code		
		VectorCopy (ent->oldorg, oldorg2);
		VectorCopy (ent->origin, ent->oldorg);
		VectorCopy (ent->origin, oldorg);//Standard
#endif

		if (ent->forcelink)
		{	// the entity was not updated in the last message
			// so move to the final spot
			VectorCopy (ent->msg_origins[0], ent->origin);
			VectorCopy (ent->msg_angles[0], ent->angles);
		}
		else
		{	// if the delta is large, assume a teleport and don't lerp
			f = frac;
			for (j=0 ; j<3 ; j++)
			{
				delta[j] = ent->msg_origins[0][j] - ent->msg_origins[1][j];
				if (delta[j] > 100 || delta[j] < -100)
					f = 1;		// assume a teleportation, not a motion
			}

			// Tomaz - Model Transform Interpolation Begin
			if (f >= 1)
			{
				ent->translate_start_time = 0;
				ent->rotate_start_time    = 0;
				VectorClear (ent->last_light);
			}
			// Tomaz - Model Transform Interpolation End
		// interpolate the origin and angles
			for (j=0 ; j<3 ; j++)
			{
				ent->origin[j] = ent->msg_origins[1][j] + f*delta[j];

				d = ent->msg_angles[0][j] - ent->msg_angles[1][j];
				if (d > 180)
					d -= 360;
				else if (d < -180)
					d += 360;
				ent->angles[j] = ent->msg_angles[1][j] + f*d;
			}
			
		}

		if (ent->effects)
		{
			effects = ent->effects;//Tei speedup

			if (effects & EF_MUZZLEFLASH)
			{
				vec3_t		fv, rv, uv;

				dl = CL_AllocDlight (i);
				VectorCopy (ent->origin,  dl->origin);
				dl->origin[2] += 16;
				AngleVectors (ent->angles, fv, rv, uv);
				 
				VectorMA (dl->origin, 18, fv, dl->origin);
				dl->radius = 130;
				dl->minlight = 1;
				dl->die = cl.time + 0.01;
				DefineFlare(ent->origin, 200, 0,20);//Tei dp flare
			}
			
			if (effects & EF_BRIGHTLIGHT)
			{			
				dl = CL_AllocDlight (i);
				VectorCopy (ent->origin,  dl->origin);
				dl->origin[2] += 16;
				dl->radius = 300;
				dl->die = cl.time + 0.001;
				dl->color[0] = 1.0f;
				dl->color[1] = 1.0f;
				dl->color[2] = 1.0f;
				DefineFlare(ent->origin, 150, 0,20);//Tei dp flare
			}
			
			if (effects & EF_DIMLIGHT)
			{			
				dl = CL_AllocDlight (i);
				VectorCopy (ent->origin,  dl->origin);
				dl->radius = 230 + (rand()&31);
				dl->die = cl.time + 0.001;
				dl->color[0] = 1.0f;
				dl->color[1] = 1.0f;
				dl->color[2] = 1.0f;
				DefineFlare(ent->origin,190+rand()&31, 0,20);//Tei dp flare
			}

			if (ent->effects & EF_BRIGHTFIELD)
				R_EntityParticles (ent);

			// Tei fx ef1
			//if (ent->effects & EF_DECAL)
			//	R_AutoLowAlpha(ent);

			if (ent->effects & EF_BLUEFIRE)
				R_FireBlue(ent, true);

			if (ent->effects & EF_FIRE)
			{		
				R_Fire(ent, true);
				/*
				if (nomach(oldorg,ent->origin)
				{
					MediumRandom(oldorg,ent->origin);
					R_Fire(ent, true);
				}*/
			}

			if (ent->effects & EF_DOWFIRE)
				R_DowFire(ent, true);
			// Tei fx ef1
		}

		// Tei more fx
		if (ent->effects2)
		{
			effects2 = ent->effects2;//Tei speedup

			if (effects2 & EF2_VOORTRAIL)
				R_VoorTrail (ent->origin1,ent->origin2,ent);

			if (effects2 & EF2_BIGFIRE)
				R_BigFire(ent, true);

			if (effects2 & EF2_FOGSPLASH)
				R_FogSplash(ent->origin);

			if (effects2 & EF2_FOGSPLASHLITE)
				R_FogSplashLite(ent->origin);

			if (effects2 & EF2_SPARKSHOWER){
				downmove[0] = 0;
				downmove[1] = 0;
				downmove[2] = 10;
				R_SparkShower (ent->origin, downmove );
				}

			if (effects2 & EF2_WATERFALL)
				R_WaterFall(ent, true);

			if (effects2 & EF2_DARKFIELD)
				R_DarkFieldParticles(ent);

			if (effects2 &  EF2_ROTATEBIT   )
			{	
				ent->origin[2] += (( sin(bobjrotate/90*M_PI) * 3) + 3 );
			}
		}

		if (ent->effects3)
		{
			effects3 = ent->effects3;//Tei speedup
			
			switch (effects3)
			{
			case EF3_ROTATE1:
				ent->angles[1] = bobjrotate;
				break;
			case EF3_ROTATE2:
				ent->angles[2] = bobjrotate;
				break;
			case EF3_ROTATE0:
				ent->angles[0] = bobjrotate;
				break;
			case EF3_ROTATE012:
				ent->angles[0] = bobjrotate;
				ent->angles[1] = bobjrotate;
				ent->angles[2] = bobjrotate;
				break;
			case EF3_ROTATEZ:
				ent->origin[2] += (( sin(bobjrotate/90*M_PI) * 5) + 5 );
				break;
			case EF3_AUTOSNOW:
				R_Snow (ent->origin1,ent->origin2, 1);
				break;
			case EF3_TEMPUSVIVENDI:
				R_TempusVivendi(ent->origin1,ent->origin2, 1);
				break;
			case EF3_TEMPUSFIRE:
				R_TempusFire(ent->origin1,ent->origin2, 1);
				break;
			case EF3_TEMPUSSMOKE:
				R_TempusSmoke(ent->origin1,ent->origin2, 1);
				break;
			case EF3_SHADOWSHIELD:
				R_ShadowParticles(ent);
				break;
			case EF3_FLUXFIRE:
				R_DowFire(ent, true);
				break;
			case EF3_ROTATEZ10:
				ent->origin[2] += 10 - (( sin(bobjrotate/90*M_PI) * 5) + 5 );
				break;
			case EF3_ROTATEZ5:
				ent->origin[2] += 5 - (( sin(bobjrotate/90*M_PI) * 5) + 5 );
				break;
			case EF3_MAGICFIRE:
				R_MagicFire (ent, true);
				break;
			case EF3_DARKSMOKE:
				R_TempusDarkSmoke(ent->origin1,ent->origin2, 1);
				break;
			case EF3_DARKTRAIL:
				R_DarkRocketTrail (oldorg, ent->origin, ent, false);
				break;
			case EF3_DARKTRAIL2:
				R_DarkRocketTrail (oldorg, ent->origin, ent, true);
				break;
			case EF3_FLUXFIRESMALL:
				R_SmallDowFire (ent, true);
				break;
			case EF3_CIRCLETRAIL:
				R_CircleRocketTrail (oldorg, ent->origin, ent);
				break;
			case EF3_CLASSICDOWNFIRE:
				R_FireClassic(ent, true);
				break;
			case EF3_Q3AUTOGUN:
				if (rand()&1)
						return;
				dl = CL_AllocDlight (i);
				VectorCopy (ent->origin,  dl->origin);
				dl->radius = 250 + (rand()&31);
				dl->die = cl.time;
				dl->color[0] = 1.0f;
				dl->color[1] = 1.0f;
				dl->color[2] = 1.0f;
				break;

				/*
			case 23:
				Con_Printf ("PRJ debug\n");	
				R_MagicFire (ent, true);
				R_Snow (ent->origin1,ent->origin2, 1);

		
				break;*/
			case EF3_GRAYBITS:
				R_TrailGray (ent);
				break;
			case EF3_GRAYBITS2:
				R_TrailGray2 (ent);
				break;
			case EF3_GRAYBITS3:
				R_TrailGray3 (ent);
				break;

			case EF3_FULLUX:
				ent->last_light[0] = 255;
				ent->last_light[1] = 255;
				ent->last_light[2] = 255;
				break;
			case EF3_NOLUX:
				ent->last_light[0] = -255;
				ent->last_light[1] = -255;
				ent->last_light[2] = -255;
				break;

			case EF3_FOXFIRE:
				R_TrailGray3 (ent);
				R_Fire2(ent, true);
				if (rand()&1)
					R_Fire2(ent, true);

				//R_SmallDowFire (ent, true);

				dl = CL_AllocDlight (i);
				VectorCopy (ent->origin,  dl->origin);
				dl->radius = 150 + (rand()&7);
				dl->die = cl.time;
				dl->color[0] = 1.0f;
				dl->color[1] = 1.0f;
				dl->color[2] = 1.0f;

				break;
			case EF3_EFIRE:
				
				//R_CircleRocketTrail (oldorg, ent->origin, ent);
				R_FireBack(ent, true);
				break;

			case EF3_DPXFLARE:
				DefineFlare(ent->origin,200, 0,20);//Tei dp flare
				break;


			case EF3_DPXNUKE:

				DefineFlare(ent->origin,2000, 0,100);//Tei dp flare nuke
				break;

			case EF3_ALIENBLOOD:
				R_TracerWizard (oldorg, ent->origin, ent, 63);
				break;

			default:
				break;
			}			
		}
		// Tei more fx

		if ( mod_predator.value )
			DefineFlare(ent->origin,200, 0,20);//Tei dp flare

		if (ent->model->flags)
		{
			// rotate binary objects locally
			// Tomaz - Bobbing Items
			if (ent->model->flags & EF_ROTATE)
			{	
				ent->angles[1] = bobjrotate;
				if (r_bobbing.value)
					ent->origin[2] += (( sin(bobjrotate/90*M_PI) * 5) + 5 );
			}
			// Tomaz - Bobbing Items

			if (ent->model->flags & EF_ROCKET)
			{
				R_FireRocketTrailX(ent,true);
				//R_RocketTrail (oldorg, ent->origin, ent);
				//AddTrail(oldorg, ent->origin, p_fire, 0.1f);
				//AddTrail(oldorg, ent->origin, p_smoke, 1);
				dl = CL_AllocDlight (i);
				VectorCopy (ent->origin, dl->origin);
				dl->radius = 250;
				dl->die = cl.time + 0.01;
				
				//XR_RocketTrail (ent->origin, oldorg2, 10);//SPX

			}
			else if (ent->model->flags & EF_GRENADE)
				R_GrenadeTrail (oldorg, ent->origin, ent);
			
			else if (ent->model->flags & EF_GIB) {
				R_BloodTrail (oldorg, ent->origin, ent);
				//R_BigBlood(ent->origin);
			}
			
			else if (ent->model->flags & EF_ZOMGIB)
				R_BloodTrail (oldorg, ent->origin, ent);
			
			else if (ent->model->flags & EF_TRACER)
			{
				//R_TracerTrail (oldorg, ent->origin, ent, 63);
				R_TracerWizard (oldorg2, ent->origin, ent, 63);
				dl = CL_AllocDlight (i);
				VectorCopy (ent->origin, dl->origin);
				dl->radius = 250;
				dl->die = cl.time + 0.01;
				dl->color[0] = 0.42f;
				dl->color[1] = 0.42f;
				dl->color[2] = 0.06f;
				//Con_Printf("tracer 1\n");
			}
			
			else if (ent->model->flags & EF_TRACER2)
			{
				R_TracerKTrail (oldorg, ent->origin, ent, 236);
				dl = CL_AllocDlight (i);
				VectorCopy (ent->origin, dl->origin);
				dl->radius = 250;
				dl->die = cl.time + 0.01;
				dl->color[0] = 0.88f;
				dl->color[1] = 0.58f;
				dl->color[2] = 0.31f;
			//	Con_Printf("tracer 2\n");
			}
			
			else if (ent->model->flags & EF_TRACER3)
			{
				R_VoorTrail (oldorg, ent->origin, ent);
				dl = CL_AllocDlight (i);
				VectorCopy (ent->origin, dl->origin);
				dl->radius = 250;
				dl->die = cl.time + 0.01;
				dl->color[0] = 0.73f;
				dl->color[1] = 0.45f;
				dl->color[2] = 0.62f;
			//	Con_Printf("tracer 3\n");
			}
		}
		
		// Tomaz - QC Glow Begin
		if (ent->glow_size)
		{
			dl = CL_AllocDlight (i);
			VectorCopy (ent->origin, dl->origin);
			dl->radius = ent->glow_size;
			dl->die = cl.time + 0.01;
			dl->color[0] = ent->glow_red;
			dl->color[1] = ent->glow_green;
			dl->color[2] = ent->glow_blue;
		}
		// Tomaz - QC Glow End

		ent->forcelink = false;

		if (i == cl.viewentity && !chase_active.value)
			continue;

		if ( ent->effects3 == EF3_NODRAW )
				continue;

		if (cl_numvisedicts < MAX_VISEDICTS)
		{
			cl_visedicts[cl_numvisedicts] = ent;
			cl_numvisedicts++;
		}
	}
}

/*
===============
CL_ReadFromServer

Read all incoming data from the server
===============
*/
int CL_ReadFromServer (void)
{
	int		ret;

	cl.oldtime = cl.time;
	cl.time += host_frametime;
	
	do
	{
		//Con_DPrintf("Get message...\n");//Tei debug

		ret = CL_GetMessage ();
		
		//Con_DPrintf("Get message - Exit ok, ret %d...\n",ret);//Tei debug
		
		if (ret == -1)
			Host_Error ("CL_ReadFromServer: lost server connection");

		//Con_DPrintf("...2\n");//Tei debug

		if (!ret)
			break;
		
		//Con_DPrintf("...ret true\n");//Tei debug

		cl.last_received_message = realtime;
		CL_ParseServerMessage ();
	} while (ret && cls.state == ca_connected);

	///Con_DPrintf("...3\n");//Tei debug

	if (cl_shownet.value)
		Con_Printf ("\n");

	//Con_DPrintf("...4\n");//Tei debug

	CL_RelinkEntities ();

	//Con_DPrintf("...5\n");//Tei debug

	//Addd more entitis here.
	CL_UpdateTEnts ();

	//Con_DPrintf("...Ok - CL_ReadFromServer\n");//Tei debug

//
// bring the links up to date
//
	return 0;
}

/*
=================
CL_SendCmd
=================
*/
void CL_SendCmd (void)
{
	usercmd_t		cmd;

	if (cls.state != ca_connected)
		return;

	if (cls.signon == SIGNONS)
	{
	// get basic movement from keyboard
		CL_BaseMove (&cmd);
	
	// allow mice or other external controllers to add to the move
		IN_Move (&cmd);
	
	// send the unreliable message
		CL_SendMove (&cmd);
	
	}


	if (cls.demoplayback)
	{
		SZ_Clear (&cls.message);
		return;
	}
	
// send the reliable message
	if (!cls.message.cursize)
		return;		// no message at all
	
	if (!NET_CanSendMessage (cls.netcon))
	{
		Con_DPrintf ("CL_WriteToServer: can't send\n");
		return;
	}

	if (NET_SendMessage (cls.netcon, &cls.message) == -1)
		Host_Error ("CL_WriteToServer: lost server connection");

	SZ_Clear (&cls.message);
}


//FH!
void CL_Fog_f (void)
{
	if (Cmd_Argc () == 1)
	{
		Con_Printf ("\"fog\" is \"%f %f %f\"\n", gl_fogred.value, gl_foggreen.value, gl_fogblue.value);
		return;
	}
	gl_fogenable.value = 1;
	
	gl_fogred.value =  atof(Cmd_Argv(1));
	gl_foggreen.value =  atof(Cmd_Argv(2));
	gl_fogblue.value =  atof(Cmd_Argv(3));


}
//FH!


/*
=============
CL_Tracepos_f -- johnfitz

display impact point of trace along VPN
=============
*/

//float TraceLine (vec3_t start, vec3_t end, vec3_t impact, vec3_t normal);

void CL_Tracepos_f (void)
{
//	vec3_t	v, w;

	return;
#if 0 //This cant really work, different traceline function?
	VectorScale(vpn, 8192.0, v);
	TraceLine(r_refdef.vieworg, v, w);

	if (Length(w) == 0)
		Con_Printf ("Tracepos: trace didn't hit anything\n");
	else
		Con_Printf ("Tracepos: (%i %i %i)\n", (int)w[0], (int)w[1], (int)w[2]);
#endif
}


/*
=============
CL_Viewpos_f -- johnfitz

display client's position and angles
=============
*/
void CL_Viewpos_f (void)
{
#if 0
	//camera position
	Con_Printf ("Viewpos: (%i %i %i) %i %i %i\n",
		(int)r_refdef.vieworg[0],
		(int)r_refdef.vieworg[1],
		(int)r_refdef.vieworg[2],
		(int)r_refdef.viewangles[PITCH],
		(int)r_refdef.viewangles[YAW],
		(int)r_refdef.viewangles[ROLL]);
#else
	//player position
	Con_Printf ("Viewpos: (%i %i %i) %i %i %i\n",
		(int)cl_entities[cl.viewentity].origin[0],
		(int)cl_entities[cl.viewentity].origin[1],
		(int)cl_entities[cl.viewentity].origin[2],
		(int)cl.viewangles[PITCH],
		(int)cl.viewangles[YAW],
		(int)cl.viewangles[ROLL]);
#endif
}

/*
=============
CL_Mapname_f -- johnfitz
=============
*/
void CL_Mapname_f (void)
{
	char name[MAX_QPATH];
	COM_StripExtension (cl.worldmodel->name + 5, name);
	Con_Printf ("\"mapname\" is \"%s\"\n", name);
}


/*
=================
CL_Init
=================
*/

void Cvar_List_f (void);//Q2!
void Cvar_Init (void);//Q2!
void CL_LocalMenus_Init(void);//Tei localmenus

void CL_Init (void)
{
	SZ_Alloc (&cls.message, 1024);

	CL_InitInput ();
	CL_InitTEnts ();
	
//
// register our commands
//
	Cvar_RegisterVariable (&cl_name);
	Cvar_RegisterVariable (&cl_color);
	Cvar_RegisterVariable (&cl_upspeed);
	Cvar_RegisterVariable (&cl_forwardspeed);
	Cvar_RegisterVariable (&cl_backspeed);
	Cvar_RegisterVariable (&cl_sidespeed);
	Cvar_RegisterVariable (&cl_movespeedkey);
	Cvar_RegisterVariable (&cl_yawspeed);
	Cvar_RegisterVariable (&cl_pitchspeed);
	Cvar_RegisterVariable (&cl_anglespeedkey);
	Cvar_RegisterVariable (&cl_shownet);
	Cvar_RegisterVariable (&cl_nolerp);

	Cvar_RegisterVariable (&cl_grenclassic);//Tei gren color

	Cvar_RegisterVariable (&lookspring);
	Cvar_RegisterVariable (&lookstrafe);
	Cvar_RegisterVariable (&sensitivity);
	Cvar_RegisterVariable (&in_mlook); // Tomaz - MouseLook
	Cvar_RegisterVariable (&m_pitch);
	Cvar_RegisterVariable (&m_yaw);
	Cvar_RegisterVariable (&m_forward);
	Cvar_RegisterVariable (&m_side);


	// Tei mod support
	Cvar_RegisterVariable (&autofovmax);
	Cvar_RegisterVariable (&mod_predator);	
	Cvar_RegisterVariable (&mod_lsig);
	Cvar_RegisterVariable (&mod_focus);
	Cvar_RegisterVariable (&mod_extendedparticle);
	Cvar_RegisterVariable (&mod_showlight);
	Cvar_RegisterVariable (&mod_cityofangels);

	Cvar_RegisterVariable (&r_autofluor);

	Cvar_RegisterVariable (&mod_progs);
	
	Cvar_RegisterVariable (&gamedir);//dave
	//Cvar_RegisterVariable (&gamedir);//dave



//	Cvar_RegisterVariable (&mod_boltclassic);
	// Tei mod support

//	Cvar_RegisterVariable (&cl_autofire);
	
	Cmd_AddCommand ("entities", CL_PrintEntities_f);
	Cmd_AddCommand ("disconnect", CL_Disconnect_f);
	Cmd_AddCommand ("record", CL_Record_f);
	Cmd_AddCommand ("stop", CL_Stop_f);
	Cmd_AddCommand ("playdemo", CL_PlayDemo_f);
	Cmd_AddCommand ("timedemo", CL_TimeDemo_f);
	Cmd_AddCommand ("fog",CL_Fog_f);//FH!

	
	Cmd_AddCommand ("tracepos", CL_Tracepos_f); //johnfitz
	Cmd_AddCommand ("viewpos", CL_Viewpos_f); //johnfitz
	Cmd_AddCommand ("mapname", CL_Mapname_f); //johnfitz

	// Q2!
	//Cmd_AddCommand ("cvarlist", Cvar_List_f );
	//Cvar_Init();
	// Q2!

	CL_LocalMenus_Init();//Tei localmenus
}

