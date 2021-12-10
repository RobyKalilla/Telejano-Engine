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
// sv_phys.c

#include "quakedef.h"

/*


pushmove objects do not obey gravity, and do not interact with each other or trigger fields, but block normal movement and push normal objects when they move.

onground is set for toss objects when they come to a complete rest.  it is set for steping or walking objects 

doors, plats, etc are SOLID_BSP, and MOVETYPE_PUSH
bonus items are SOLID_TRIGGER touch, and MOVETYPE_TOSS
corpses are SOLID_NOT and MOVETYPE_TOSS
crates are SOLID_BBOX and MOVETYPE_TOSS
walking monsters are SOLID_SLIDEBOX and MOVETYPE_STEP
flying/floating monsters are SOLID_SLIDEBOX and MOVETYPE_FLY

solid_edge items only clip against bsp models.

*/

void R_ParticleExplosionSmoke (vec3_t origin);
extern cvar_t extra_fx;

cvar_t	sv_friction			= {"sv_friction","4",false,true};
cvar_t	sv_stopspeed		= {"sv_stopspeed","100"};
cvar_t	sv_gravity			= {"sv_gravity","800",true,true};//Tei gamers request
cvar_t	sv_maxvelocity		= {"sv_maxvelocity","2000"};
cvar_t	sv_nostep			= {"sv_nostep","0"};

// Tei step size
cvar_t	sv_stepsize			= {"sv_stepsize","18"};
// Tei step size

// Tei sky custom
cvar_t	sv_skyvalue			= {"sv_skyvalue","1"};
cvar_t	sv_skydim			= {"sv_skydim","32"};
cvar_t	sv_skyspeedscale	= {"sv_skyspeedscale","2"};
cvar_t	sv_skyflattern		= {"sv_skyflattern","16"};
// Tei sky custom

// Tei autofx
cvar_t	r_autolava		= {"r_autolava","2"};
cvar_t	r_autohell		= {"r_autohell","0"};//hummm... for fun
cvar_t	r_autotele		= {"r_autotele","1"};
cvar_t	r_autosnow		= {"r_autosnow","0"};
cvar_t	r_autorain		= {"r_autorain","0"};
cvar_t	r_autofogwater	= {"r_autofogwater","1"};
cvar_t	r_autobubbleglobal	= {"r_autobubbleglobal","1"};
cvar_t	r_autolightday	= {"r_autolightday","0"};//Fixed typo 
cvar_t	r_autozing		= {"r_autozing","0"};
cvar_t	r_autobubbles		= {"r_autobubbles","40"};

cvar_t	r_autograss		= {"r_autograss","1"};
cvar_t	r_grassdensity		= {"r_grassdensity","0.8"};
cvar_t	r_grasspeed		= {"r_grasspeed","10"};

cvar_t	r_autofogfog	= {"r_autofogfog","1"};
cvar_t	r_autowindx	= {"r_autowindx","0"};
cvar_t	r_autowindy	= {"r_autowindy","0"};
cvar_t	r_autowindz = {"r_autowindz","0"};


// Tei autofx
//cvar_t	r_waterwarp		= {"r_waterwarp","1"};//Tei / Fritz


#define	MOVE_EPSILON	0.01

void SV_Physics_Toss (edict_t *ent);

// LordHavoc: added light checking to the server
/*
void SV_LightPoint (vec3_t color, vec3_t p)
{
	Mod_CheckLoaded(sv.worldmodel);
	if (!sv.worldmodel->lightdata)
	{
		color[0] = color[1] = color[2] = 255;
		return;
	}

	color[0] = color[1] = color[2] = 0;
	SV_RecursiveLightPoint (color, sv.worldmodel->nodes, p[0], p[1], p[2], p[2] - 65536);
}
*/
// LordHavoc: added light checking to the server


/*
================
SV_CheckAllEnts
================
*/
void SV_CheckAllEnts (void)
{
	int			e;
	edict_t		*check;

// see if any solid entities are inside the final position
	check = NEXT_EDICT(sv.edicts);
	for (e=1 ; e<sv.num_edicts ; e++, check = NEXT_EDICT(check))
	{
		if (check->free)
			continue;
		if (check->v.movetype == MOVETYPE_PUSH
		||  check->v.movetype == MOVETYPE_NONE
		||  check->v.movetype == MOVETYPE_FOLLOW	// Tomaz - MOVETYPE_FOLLOW
		||  check->v.movetype == MOVETYPE_NOCLIP)
			continue;

		if (SV_TestEntityPosition (check))
			Con_Printf ("entity in invalid position\n");
	}
}

/*
================
SV_CheckVelocity
================
*/
void SV_CheckVelocity (edict_t *ent)
{
	int		i;
	float	wishspeed;

//
// bound velocity
//
	for (i=0 ; i<3 ; i++)
	{
		if (IS_NAN(ent->v.velocity[i]))
		{
			Con_Printf ("Got a NaN velocity on %s\n", pr_strings + ent->v.classname);
			ent->v.velocity[i] = 0;
		}
		if (IS_NAN(ent->v.origin[i]))
		{
			Con_Printf ("Got a NaN origin on %s\n", pr_strings + ent->v.classname);
			ent->v.origin[i] = 0;
		}
		// LordHavoc: maxvelocity fix, see below
/*
		if (ent->v.velocity[i] > sv_maxvelocity.value)
			ent->v.velocity[i] = sv_maxvelocity.value;
		else if (ent->v.velocity[i] < -sv_maxvelocity.value)
			ent->v.velocity[i] = -sv_maxvelocity.value;
*/
	}

	// LordHavoc: max velocity fix, inspired by Maddes's source fixes, but this is faster
	wishspeed = DotProduct(ent->v.velocity, ent->v.velocity);
	if (wishspeed > sv_maxvelocity.value * sv_maxvelocity.value)
	{
		wishspeed = sv_maxvelocity.value / sqrt(wishspeed);
		ent->v.velocity[0] *= wishspeed;
		ent->v.velocity[1] *= wishspeed;
		ent->v.velocity[2] *= wishspeed;
	}
	// LordHavoc: max velocity fix, inspired by Maddes's source fixes, but this is faster
}

/*
=============
SV_RunThink

Runs thinking code if time.  There is some play in the exact time the think
function will be called, because it is called before any movement is done
in a frame.  Not used for pushmove objects, because they must be exact.
Returns false if the entity removed itself.
=============
*/
qboolean SV_RunThink (edict_t *ent)
{
	float	thinktime;

	thinktime = ent->v.nextthink;
	if (thinktime <= 0 || thinktime > sv.time + host_frametime)
		return true;
		
	if (thinktime < sv.time)
		thinktime = sv.time;	// don't let things stay in the past.
								// it is possible to start that way
								// by a trigger with a local time.
	ent->v.nextthink = 0;
	pr_global_struct->time = thinktime;
	pr_global_struct->self = EDICT_TO_PROG(ent);
	pr_global_struct->other = EDICT_TO_PROG(sv.edicts);
	PR_ExecuteProgram (ent->v.think);
	return !ent->free;
}

/*
==================
SV_Impact

Two entities have touched, so run their touch functions
==================
*/
	extern sfx_t * cl_sfx_tink1;
	extern sfx_t * cl_sfx_ric1;
	extern sfx_t * cl_sfx_ric2;
	extern sfx_t * cl_sfx_ric3;

#define ParamSum(v,w) (v[0] + w[0] + v[1] + w[1] + v[2] + w[2]))

void SV_Impact (edict_t *e1, edict_t *e2)
{
	int		old_self, old_other, f, f2,f3, speed;
	vec3_t org3;
	int rnd;
	
	old_self = pr_global_struct->self;
	old_other = pr_global_struct->other;
	
	pr_global_struct->time = sv.time;
	if (e1->v.touch && e1->v.solid != SOLID_NOT)
	{
		pr_global_struct->self = EDICT_TO_PROG(e1);
		pr_global_struct->other = EDICT_TO_PROG(e2);
		PR_ExecuteProgram (e1->v.touch);
	}
	
	if (e2->v.touch && e2->v.solid != SOLID_NOT)
	{
		pr_global_struct->self = EDICT_TO_PROG(e2);
		pr_global_struct->other = EDICT_TO_PROG(e1);
		PR_ExecuteProgram (e2->v.touch);
	}
	
	//Tei
	if (extra_fx.value)
	{
		f = e1->v.flags;
		f2 = e1->v.solid;
		f3 = e2->v.solid;
		if (e1->v.movetype && e2->v.movetype && !( f & FL_ONGROUND)&& !( f & FL_CLIENT)&& !( f & FL_INWATER) && !(f &FL_NOTARGET) && !(f2 & SOLID_TRIGGER) && !(f3 & SOLID_TRIGGER))
		{	

			speed = e1->v.velocity[0] +  e1->v.velocity[1]+e1->v.velocity[2];
			speed += e2->v.velocity[0] +  e2->v.velocity[1]+e2->v.velocity[2];
			if (speed>0)
			{
				//Con_Printf("hello imp\n");
				org3[0] = (e1->v.origin[0] + e2->v.origin[0]) * 0.5;
				org3[1] = (e1->v.origin[1] + e2->v.origin[1]) * 0.5;
				org3[2] = (e1->v.origin[2] + e2->v.origin[2]) * 0.5;
				R_ParticleExplosionSmoke(org3);
				R_ParticleExplosionSmoke(e1->v.origin);
				R_ParticleExplosionSmoke(e2->v.origin);
				if (!strcmp("spike",pr_strings + e1->v.classname ) )
				{
						//Con_Printf("Hello\n");
						if ( rand() % 5 )
							S_StartSound (-1, 0, cl_sfx_tink1, e1->v.origin, 1, 1);	
						else
						{
							rnd = rand() & 3;
							if (rnd == 1)
								S_StartSound (-1, 0, cl_sfx_ric1, e1->v.origin, 1, 1);
							else if (rnd == 2)
								S_StartSound (-1, 0, cl_sfx_ric2, e1->v.origin, 1, 1);
							else
								S_StartSound (-1, 0, cl_sfx_ric3, e1->v.origin, 1, 1);
						}
				}
			}

		}

	}
	//Tei

	pr_global_struct->self = old_self;
	pr_global_struct->other = old_other;
}


/*
==================
ClipVelocity

Slide off of the impacting object
returns the blocked flags (1 = floor, 2 = step / wall)
==================
*/
#define	STOP_EPSILON	0.1

int ClipVelocity (vec3_t in, vec3_t normal, vec3_t out, float overbounce)
{
	float	backoff;
	float	change;
	int		i, blocked;
	
	blocked = 0;
	if (normal[2] > 0)
		blocked |= 1;		// floor
	if (!normal[2])
		blocked |= 2;		// step
	
	backoff = DotProduct (in, normal) * overbounce;

	for (i=0 ; i<3 ; i++)
	{
		change = normal[i]*backoff;
		out[i] = in[i] - change;
		if (out[i] > -STOP_EPSILON && out[i] < STOP_EPSILON)
			out[i] = 0;
	}
	
	return blocked;
}


/*
============
SV_FlyMove

The basic solid body movement clip that slides along multiple planes
Returns the clipflags if the velocity was modified (hit something solid)
1 = floor
2 = wall / step
4 = dead stop
If steptrace is not NULL, the trace of any vertical wall hit will be stored
============
*/
//LoardHavoc increase this to 20
#define	MAX_CLIP_PLANES	20
int SV_FlyMove (edict_t *ent, float time, trace_t *steptrace)
{
	int			bumpcount, numbumps;
	vec3_t		dir;
	float		d;
	int			numplanes;
	vec3_t		planes[MAX_CLIP_PLANES];
	vec3_t		primal_velocity, original_velocity, new_velocity;
	int			i, j;
	trace_t		trace;
	vec3_t		end;
	float		time_left;
	int			blocked;
	
	numbumps = 4;
	
	blocked = 0;
	VectorCopy (ent->v.velocity, original_velocity);
	VectorCopy (ent->v.velocity, primal_velocity);
	numplanes = 0;
	
	time_left = time;

	for (bumpcount=0 ; bumpcount<numbumps ; bumpcount++)
	{
		if (!ent->v.velocity[0] && !ent->v.velocity[1] && !ent->v.velocity[2])
			break;

		for (i=0 ; i<3 ; i++)
			end[i] = ent->v.origin[i] + time_left * ent->v.velocity[i];

		trace = SV_Move (ent->v.origin, ent->v.mins, ent->v.maxs, end, false, ent);

		if (trace.allsolid)
		{	// entity is trapped in another solid
			VectorCopy (vec3_origin, ent->v.velocity);
			return 3;
		}

		if (trace.fraction > 0)
		{	// actually covered some distance
			VectorCopy (trace.endpos, ent->v.origin);
			VectorCopy (ent->v.velocity, original_velocity);
			numplanes = 0;
		}

		if (trace.fraction == 1)
			 break;		// moved the entire distance

		if (!trace.ent)
			Sys_Error ("SV_FlyMove: !trace.ent");

		if (trace.plane.normal[2] > 0.7)
		{
			blocked |= 1;		// floor
#if 0
			if (trace.ent->v.solid == SOLID_BSP) //Tei original
#else
			//if (trace.ent->v.solid == SOLID_BSP) //johnfitz -- no sliding on entity bboxes
#endif
			{
				ent->v.flags =	(int)ent->v.flags | FL_ONGROUND;
				ent->v.groundentity = EDICT_TO_PROG(trace.ent);
			}
		}
		if (!trace.plane.normal[2])
		{
			blocked |= 2;		// step
			if (steptrace)
				*steptrace = trace;	// save for player extrafriction
		}

//
// run the impact function
//
		SV_Impact (ent, trace.ent);
		if (ent->free)
			break;		// removed by the impact function

		
		time_left -= time_left * trace.fraction;
		
	// cliped to another plane
		if (numplanes >= MAX_CLIP_PLANES)
		{	// this shouldn't really happen
			VectorCopy (vec3_origin, ent->v.velocity);
			return 3;
		}

		VectorCopy (trace.plane.normal, planes[numplanes]);
		numplanes++;

//
// modify original_velocity so it parallels all of the clip planes
//
		for (i=0 ; i<numplanes ; i++)
		{
			ClipVelocity (original_velocity, planes[i], new_velocity, 1);
			for (j=0 ; j<numplanes ; j++)
				if (j != i)
				{
					if (DotProduct (new_velocity, planes[j]) < 0)
						break;	// not ok
				}
			if (j == numplanes)
				break;
		}
		
		if (i != numplanes)
		{	// go along this plane
			VectorCopy (new_velocity, ent->v.velocity);
		}
		else
		{	// go along the crease
			if (numplanes != 2)
			{
//				Con_Printf ("clip velocity, numplanes == %i\n",numplanes);
				VectorCopy (vec3_origin, ent->v.velocity);
				return 7;
			}
			CrossProduct (planes[0], planes[1], dir);
			d = DotProduct (dir, ent->v.velocity);
			VectorScale (dir, d, ent->v.velocity);
		}

//
// if original velocity is against the original velocity, stop dead
// to avoid tiny occilations in sloping corners
//
		if (DotProduct (ent->v.velocity, primal_velocity) <= 0)
		{
			VectorCopy (vec3_origin, ent->v.velocity);
			return blocked;
		}
	}

	return blocked;
}


/*
============
SV_AddGravity

============
*/
void SV_AddGravity (edict_t *ent)
{
	float	ent_gravity;
	eval_t	*val;

	val = GETEDICTFIELDVALUE(ent, eval_gravity);

	if (val && val->_float)
		ent_gravity = val->_float;
	else
		ent_gravity = 1.0;

	ent->v.velocity[2] -= ent_gravity * sv_gravity.value * host_frametime;
	
}

// Tei - addmagnetic beta code - movetype magnetic

/*
============
SV_AddMagnetic

============
*/
void SV_AddMagnetic (edict_t *ent)
{
	vec3_t	movedir;
	
	VectorSubtract (ent->v.movedir,ent->v.origin, movedir);

	ent->v.velocity[0] +=  movedir[0] * host_frametime * ent->v.magmultiplier;
	ent->v.velocity[1] +=  movedir[1] * host_frametime * ent->v.magmultiplier;
	ent->v.velocity[2] +=  movedir[2] * host_frametime * ent->v.magmultiplier;
}

void SV_AddMagnetic2 (edict_t *ent)
{
	vec3_t	movedir;
	edict_t *e;	

	e = PROG_TO_EDICT(ent->v.aiment);

	VectorSubtract (e->v.origin,ent->v.origin, movedir);

	ent->v.velocity[0] +=  movedir[0] * host_frametime * ent->v.magmultiplier;
	ent->v.velocity[1] +=  movedir[1] * host_frametime * ent->v.magmultiplier;
	ent->v.velocity[2] +=  movedir[2] * host_frametime * ent->v.magmultiplier;
}

// Tei - addmagnetic beta code - movetype magnetic


/*
===============================================================================

PUSHMOVE

===============================================================================
*/

/*
============
SV_PushEntity

Does not change the entities velocity at all
============
*/
trace_t SV_PushEntity (edict_t *ent, vec3_t push)
{
	trace_t	trace;
	vec3_t	end;
		
	VectorAdd (ent->v.origin, push, end);

	if (ent->v.movetype == MOVETYPE_FLYMISSILE)
		trace = SV_Move (ent->v.origin, ent->v.mins, ent->v.maxs, end, MOVE_MISSILE, ent);
	else if (ent->v.solid == SOLID_TRIGGER || ent->v.solid == SOLID_NOT)
	// only clip against bmodels
		trace = SV_Move (ent->v.origin, ent->v.mins, ent->v.maxs, end, MOVE_NOMONSTERS, ent);
	else
		trace = SV_Move (ent->v.origin, ent->v.mins, ent->v.maxs, end, MOVE_NORMAL, ent);	
	
	VectorCopy (trace.endpos, ent->v.origin);
	SV_LinkEdict (ent, true);

	if (trace.ent)
		SV_Impact (ent, trace.ent);		

	return trace;
}					


/*
============
SV_PushMove

============
*/
void SV_PushMove (edict_t *pusher, float movetime)
{
	int			i, e, solid_backup;
	edict_t		*check, *block;
	vec3_t		mins, maxs, move;
	vec3_t		entorig, pushorig;
	int			num_moved;
	edict_t		*moved_edict[MAX_EDICTS];
	vec3_t		moved_from[MAX_EDICTS];

	if (!pusher->v.velocity[0] && !pusher->v.velocity[1] && !pusher->v.velocity[2])
	{
		pusher->v.ltime += movetime;
		return;
	}

	for (i=0 ; i<3 ; i++)
	{
		move[i] = pusher->v.velocity[i] * movetime;
		mins[i] = pusher->v.absmin[i] + move[i];
		maxs[i] = pusher->v.absmax[i] + move[i];
	}

	VectorCopy (pusher->v.origin, pushorig);
	
// move the pusher to it's final position

	VectorAdd (pusher->v.origin, move, pusher->v.origin);
	pusher->v.ltime += movetime;
	SV_LinkEdict (pusher, false);


// see if any solid entities are inside the final position
	num_moved = 0;
	check = NEXT_EDICT(sv.edicts);
	for (e=1 ; e<sv.num_edicts ; e++, check = NEXT_EDICT(check))
	{
		if (check->free)
			continue;
		if (check->v.movetype == MOVETYPE_PUSH
		||  check->v.movetype == MOVETYPE_NONE
		||  check->v.movetype == MOVETYPE_FOLLOW	// Tomaz - MOVETYPE_FOLLOW
		||  check->v.movetype == MOVETYPE_NOCLIP)
			continue;

	// if the entity is standing on the pusher, it will definately be moved
		if ( ! ( ((int)check->v.flags & FL_ONGROUND)
		&& PROG_TO_EDICT(check->v.groundentity) == pusher) )
		{
			if (check->v.absmin[0] >= maxs[0]
			||  check->v.absmin[1] >= maxs[1]
			||  check->v.absmin[2] >= maxs[2]
			||  check->v.absmax[0] <= mins[0]
			||  check->v.absmax[1] <= mins[1]
			||  check->v.absmax[2] <= mins[2] )
				continue;

		// see if the ent's bbox is inside the pusher's final position
			if (!SV_TestEntityPosition (check))
				continue;
		}

	// remove the onground flag for non-players
		if (check->v.movetype != MOVETYPE_WALK)
			check->v.flags = (int)check->v.flags & ~FL_ONGROUND;
		
		VectorCopy (check->v.origin, entorig);
		VectorCopy (check->v.origin, moved_from[num_moved]);
		moved_edict[num_moved] = check;
		num_moved++;

		// try moving the contacted entity 
		solid_backup = pusher->v.solid;

		if (solid_backup == SOLID_BSP				// everything that blocks: bsp models = map brushes = doors, plats, etc.
			|| solid_backup == SOLID_BBOX			// normally boxes
			|| solid_backup == SOLID_SLIDEBOX)		// normally monsters
		{
			pusher->v.solid = SOLID_NOT;
			SV_PushEntity (check, move);
			pusher->v.solid = solid_backup;

			// if it is still inside the pusher, block
			block = SV_TestEntityPosition (check);
		}
		else {
			block = NULL;
		}

		if (block)
		{	// fail the move
			if (check->v.mins[0] == check->v.maxs[0])
				continue;
			if (check->v.solid == SOLID_NOT || check->v.solid == SOLID_TRIGGER)
			{	// corpse
				check->v.mins[0] = check->v.mins[1] = 0;
				VectorCopy (check->v.mins, check->v.maxs);
				continue;
			}
			
			VectorCopy (entorig, check->v.origin);
			SV_LinkEdict (check, true);

			VectorCopy (pushorig, pusher->v.origin);
			SV_LinkEdict (pusher, false);
			pusher->v.ltime -= movetime;

			// if the pusher has a "blocked" function, call it
			// otherwise, just stay in place until the obstacle is gone
			if (pusher->v.blocked)
			{
				pr_global_struct->self = EDICT_TO_PROG(pusher);
				pr_global_struct->other = EDICT_TO_PROG(check);
				PR_ExecuteProgram (pusher->v.blocked);
			}
			
		// move back any entities we already moved
			for (i=0 ; i<num_moved ; i++)
			{
				VectorCopy (moved_from[i], moved_edict[i]->v.origin);
				SV_LinkEdict (moved_edict[i], false);
			}
			return;
		}	
	}
}

// Tomaz - Rotating BModels Begin 
/*
============
SV_PushRotate
============
*/
void SV_PushRotate (edict_t *pusher, float movetime)
{
	int			i, e;
	edict_t		*check, *block;
	vec3_t		move, a, amove;
	vec3_t		entorig, pushorig;
	int			num_moved;
	edict_t		*moved_edict[MAX_EDICTS];
	vec3_t		moved_from[MAX_EDICTS];
	vec3_t		org, org2;
	vec3_t		forward, right, up;

	if (!pusher->v.avelocity[0] && !pusher->v.avelocity[1] && !pusher->v.avelocity[2])
	{
		pusher->v.ltime += movetime;
		return;
	}

	for (i=0 ; i<3 ; i++)
		amove[i] = pusher->v.avelocity[i] * movetime;

	VectorSubtract (vec3_origin, amove, a);
	AngleVectors (a, forward, right, up);

	VectorCopy (pusher->v.angles, pushorig);
	
// move the pusher to it's final position

	VectorAdd (pusher->v.angles, amove, pusher->v.angles);
	pusher->v.ltime += movetime;
	SV_LinkEdict (pusher, false);


// see if any solid entities are inside the final position
	num_moved = 0;
	check = NEXT_EDICT(sv.edicts);
	for (e=1 ; e<sv.num_edicts ; e++, check = NEXT_EDICT(check))
	{
		if (check->free)
			continue;
		if (check->v.movetype == MOVETYPE_PUSH		||
			check->v.movetype == MOVETYPE_NONE		|| 
			check->v.movetype == MOVETYPE_FOLLOW	||
			check->v.movetype == MOVETYPE_NOCLIP)
			continue;

	// if the entity is standing on the pusher, it will definately be moved
		if ( ! ( ((int)check->v.flags & FL_ONGROUND)
		&& PROG_TO_EDICT(check->v.groundentity) == pusher) )
		{
			if (check->v.absmin[0] >= pusher->v.absmax[0]	||
				check->v.absmin[1] >= pusher->v.absmax[1]	||
				check->v.absmin[2] >= pusher->v.absmax[2]	||
				check->v.absmax[0] <= pusher->v.absmin[0]	||
				check->v.absmax[1] <= pusher->v.absmin[1]	||
				check->v.absmax[2] <= pusher->v.absmin[2] )
				continue;

		// see if the ent's bbox is inside the pusher's final position
			if (!SV_TestEntityPosition (check))
				continue;
		}

	// remove the onground flag for non-players
		if (check->v.movetype != MOVETYPE_WALK)
			check->v.flags = (int)check->v.flags & ~FL_ONGROUND;
		
		VectorCopy (check->v.origin, entorig);
		VectorCopy (check->v.origin, moved_from[num_moved]);
		moved_edict[num_moved] = check;
		num_moved++;

		// calculate destination position
		VectorSubtract (check->v.origin, pusher->v.origin, org);
		org2[0] = DotProduct (org, forward);
		org2[1] = -DotProduct (org, right);
		org2[2] = DotProduct (org, up);
		VectorSubtract (org2, org, move);

		// try moving the contacted entity 
		pusher->v.solid = SOLID_NOT;
		SV_PushEntity (check, move);
		pusher->v.solid = SOLID_BSP;

	// if it is still inside the pusher, block
		block = SV_TestEntityPosition (check);
		if (block)
		{	// fail the move
			if (check->v.mins[0] == check->v.maxs[0])
				continue;
			if (check->v.solid == SOLID_NOT || check->v.solid == SOLID_TRIGGER)
			{	// corpse
				check->v.mins[0] = check->v.mins[1] = 0;
				VectorCopy (check->v.mins, check->v.maxs);
				continue;
			}
			
			VectorCopy (entorig, check->v.origin);
			SV_LinkEdict (check, true);

			VectorCopy (pushorig, pusher->v.angles);
			SV_LinkEdict (pusher, false);
			pusher->v.ltime -= movetime;

			// if the pusher has a "blocked" function, call it
			// otherwise, just stay in place until the obstacle is gone
			if (pusher->v.blocked)
			{
				pr_global_struct->self = EDICT_TO_PROG(pusher);
				pr_global_struct->other = EDICT_TO_PROG(check);
				PR_ExecuteProgram (pusher->v.blocked);
			}
			
		// move back any entities we already moved
			for (i=0 ; i<num_moved ; i++)
			{
				VectorCopy (moved_from[i], moved_edict[i]->v.origin);
				VectorSubtract (moved_edict[i]->v.angles, amove, moved_edict[i]->v.angles);
				SV_LinkEdict (moved_edict[i], false);
			}
			return;
		}
		else
		{
			VectorAdd (check->v.angles, amove, check->v.angles);
		}
	}
}
// Tomaz - Rotating BModels End

/*
================
SV_Physics_Pusher
================
*/
void SV_Physics_Pusher (edict_t *ent)
{
	float	thinktime;
	float	oldltime;
	float	movetime;

	oldltime = ent->v.ltime;
	
	thinktime = ent->v.nextthink;
	if (thinktime < ent->v.ltime + host_frametime)
	{
		movetime = thinktime - ent->v.ltime;
		if (movetime < 0)
			movetime = 0;
	}
	else
		movetime = host_frametime;

	// Tomaz - Rotating BModels Begin
	if (strcmp (pr_strings + ent->v.classname, "misc_teleporttrain") && (ent->v.avelocity[0] || ent->v.avelocity[1] || ent->v.avelocity[2]))
		SV_PushRotate (ent, host_frametime);
	// Tomaz - Rotating BModels End
	else if (movetime)
		SV_PushMove (ent, movetime);
	
	if (thinktime > oldltime && thinktime <= ent->v.ltime)
	{
		ent->v.nextthink = 0;
		pr_global_struct->time = sv.time;
		pr_global_struct->self = EDICT_TO_PROG(ent);
		pr_global_struct->other = EDICT_TO_PROG(sv.edicts);
		PR_ExecuteProgram (ent->v.think);
		if (ent->free)
			return;
	}
}

/*
===============================================================================

CLIENT MOVEMENT

===============================================================================
*/

/*
=============
SV_CheckStuck

This is a big hack to try and fix the rare case of getting stuck in the world
clipping hull.
=============
*/

// Tei crouch fx


void SV_HipeUnStuck (edict_t *ent)
{
	int		i, j;
	int		z;
	vec3_t	org;

	if (!SV_TestEntityPosition(ent))
	{
		VectorCopy (ent->v.origin, ent->v.oldorigin);
		return;
	}

	VectorCopy (ent->v.origin, org);
	VectorCopy (ent->v.oldorigin, ent->v.origin);
	if (!SV_TestEntityPosition(ent))
	{
		Con_DPrintf ("Unstuck - TestEntityPos.\n");
		SV_LinkEdict (ent, true);
		return;
	}
	
	if (rand()&1)
	for (z=-32 ; z< 64 ; z++)
		for (i=-1 ; i <= 1 ; i++)
			for (j=-1 ; j <= 1 ; j++)
			{
				ent->v.origin[0] = org[0] + i;
				ent->v.origin[1] = org[1] + j;
				ent->v.origin[2] = org[2] + z;
				if (rand()&1)
				if (!SV_TestEntityPosition(ent))
				{
					Con_DPrintf ("Unstuck - ReTest.\n");
					SV_LinkEdict (ent, true);
					return;
				}
			}
			
	VectorCopy (org, ent->v.origin);
	Con_DPrintf ("player is stuck.\n");
}

// Tei crouch fx

void SV_CheckStuck (edict_t *ent)
{
	int		i, j;
	int		z;
	vec3_t	org;

	SV_HipeUnStuck(ent);
	return;

	if (!SV_TestEntityPosition(ent))
	{
		VectorCopy (ent->v.origin, ent->v.oldorigin);
		return;
	}

	VectorCopy (ent->v.origin, org);
	VectorCopy (ent->v.oldorigin, ent->v.origin);
	if (!SV_TestEntityPosition(ent))
	{
		Con_DPrintf ("Unstuck - TestEntityPos.\n");
		SV_LinkEdict (ent, true);
		return;
	}
	
	for (z=0 ; z< 18 ; z++)
		for (i=-1 ; i <= 1 ; i++)
			for (j=-1 ; j <= 1 ; j++)
			{
				ent->v.origin[0] = org[0] + i;
				ent->v.origin[1] = org[1] + j;
				ent->v.origin[2] = org[2] + z;
				if (!SV_TestEntityPosition(ent))
				{
					Con_DPrintf ("Unstuck - ReTest.\n");
					SV_LinkEdict (ent, true);
					return;
				}
			}
			
	VectorCopy (org, ent->v.origin);
	Con_DPrintf ("player is stuck.\n");
}







/*
=============
SV_CheckWater
=============
*/
int SV_HullPointContents (hull_t *hull, int num, vec3_t p);

qboolean SV_CheckWater (edict_t *ent)
{
	vec3_t	point;
	int		cont;

	point[0] = ent->v.origin[0];
	point[1] = ent->v.origin[1];
	point[2] = ent->v.origin[2] + ent->v.mins[2] + 1;	
	
	
	ent->v.waterlevel = 0;
	ent->v.watertype = CONTENTS_EMPTY;
	//ent->v.waterlevel = 2;
#if 0
	//Tei water float
	if (ent->v.movetype == MOVETYPE_WATERFLOAT)
	{
			if (ent->v.velocity[2]>(-200))
					ent->v.velocity[2] -= 5;
	}
	//Tei water float
#endif

	cont = SV_PointContents (point);

	if (cont <= CONTENTS_WATER)
	{
		ent->v.watertype = cont;
		ent->v.waterlevel = 1;
		point[2] = ent->v.origin[2] + (ent->v.mins[2] + ent->v.maxs[2])*0.5;
		cont = SV_PointContents (point);

		if (cont <= CONTENTS_WATER) 
		{
			ent->v.waterlevel = 2;
#if 0
			//Tei water float
			if (ent->v.movetype == MOVETYPE_WATERFLOAT)
			{
					ent->v.velocity[2] += 5;
			}
			//Tei water float
#endif
			point[2] = ent->v.origin[2] + ent->v.view_ofs[2];
			cont = SV_PointContents (point);

			if (cont <= CONTENTS_WATER)
			{
				ent->v.waterlevel = 3;
#if 0
				//Tei water float
				if (ent->v.movetype == MOVETYPE_WATERFLOAT)
				{
					ent->v.velocity[2] += 15;
				}
				//Tei water float
#endif
			}
		}
	}
	return ent->v.waterlevel > 1;
}

/*
============
SV_WallFriction

============
*/
void SV_WallFriction (edict_t *ent, trace_t *trace)
{
	vec3_t		forward;
	float		d, i;
	vec3_t		into, side;
	
	AngleVectors (ent->v.v_angle, forward, NULL, NULL);
	d = DotProduct (trace->plane.normal, forward);
	
	d += 0.5;
	if (d >= 0)
		return;
		
// cut the tangential velocity
	i = DotProduct (trace->plane.normal, ent->v.velocity);
	VectorScale (trace->plane.normal, i, into);
	VectorSubtract (ent->v.velocity, into, side);
	
	ent->v.velocity[0] = side[0] * (1 + d);
	ent->v.velocity[1] = side[1] * (1 + d);
}

/*
=====================
SV_TryUnstick

Player has come to a dead stop, possibly due to the problem with limited
float precision at some angle joins in the BSP hull.

Try fixing by pushing one pixel in each direction.

This is a hack, but in the interest of good gameplay...
======================
*/
int SV_TryUnstick (edict_t *ent, vec3_t oldvel)
{
	int		i;
	vec3_t	oldorg;
	vec3_t	dir;
	int		clip;
	trace_t	steptrace;
	
	VectorCopy (ent->v.origin, oldorg);
	VectorCopy (vec3_origin, dir);

	for (i=0 ; i<8 ; i++)
	{
// try pushing a little in an axial direction
		switch (i)
		{
			case 0:	dir[0] = 2; dir[1] = 0; break;
			case 1:	dir[0] = 0; dir[1] = 2; break;
			case 2:	dir[0] = -2; dir[1] = 0; break;
			case 3:	dir[0] = 0; dir[1] = -2; break;
			case 4:	dir[0] = 2; dir[1] = 2; break;
			case 5:	dir[0] = -2; dir[1] = 2; break;
			case 6:	dir[0] = 2; dir[1] = -2; break;
			case 7:	dir[0] = -2; dir[1] = -2; break;
		}
		
		SV_PushEntity (ent, dir);

// retry the original move
		ent->v.velocity[0] = oldvel[0];
		ent->v. velocity[1] = oldvel[1];
		ent->v. velocity[2] = 0;
		clip = SV_FlyMove (ent, 0.1f, &steptrace);

		if ( fabs(oldorg[1] - ent->v.origin[1]) > 4
		|| fabs(oldorg[0] - ent->v.origin[0]) > 4 )
		{
//Con_DPrintf ("unstuck!\n");
			return clip;
		}
			
// go back to the original pos and try again
		VectorCopy (oldorg, ent->v.origin);
	}
	
	VectorCopy (vec3_origin, ent->v.velocity);
	return 7;		// still not moving
}

/*
=====================
SV_WalkMove

Only used by players
======================
*/
//#define	STEPSIZE	18
void SV_WalkMove (edict_t *ent)
{
	vec3_t		upmove, downmove;
	vec3_t		oldorg, oldvel;
	vec3_t		nosteporg, nostepvel;
	int			clip;
	int			oldonground;
	trace_t		steptrace, downtrace;
	
//
// do a regular slide move unless it looks like you ran into a step
//
	oldonground = (int)ent->v.flags & FL_ONGROUND;
	ent->v.flags = (int)ent->v.flags & ~FL_ONGROUND;
	
	VectorCopy (ent->v.origin, oldorg);
	VectorCopy (ent->v.velocity, oldvel);
	
	clip = SV_FlyMove (ent, host_frametime, &steptrace);

	if ( !(clip & 2) )
		return;		// move didn't block on a step

	if (!oldonground && ent->v.waterlevel == 0)
		return;		// don't stair up while jumping
	
	if (ent->v.movetype != MOVETYPE_WALK)
		return;		// gibbed by a trigger
	
	if (sv_nostep.value)
		return;
	
	if ( (int)sv_player->v.flags & FL_WATERJUMP )
		return;

	VectorCopy (ent->v.origin, nosteporg);
	VectorCopy (ent->v.velocity, nostepvel);

//
// try moving up and forward to go up a step
//
	VectorCopy (oldorg, ent->v.origin);	// back to start pos

	VectorCopy (vec3_origin, upmove);
	VectorCopy (vec3_origin, downmove);
	upmove[2] = sv_stepsize.value;
	downmove[2] = -sv_stepsize.value + oldvel[2]*host_frametime;

// move up
	SV_PushEntity (ent, upmove);	// FIXME: don't link?

// move forward
	ent->v.velocity[0] = oldvel[0];
	ent->v. velocity[1] = oldvel[1];
	ent->v. velocity[2] = 0;
	clip = SV_FlyMove (ent, host_frametime, &steptrace);

// check for stuckness, possibly due to the limited precision of floats
// in the clipping hulls
	if (clip)
	{
		if ( fabs(oldorg[1] - ent->v.origin[1]) < 0.03125
		&& fabs(oldorg[0] - ent->v.origin[0]) < 0.03125 )
		{	// stepping up didn't make any progress
			clip = SV_TryUnstick (ent, oldvel);
		}
	}
	
// extra friction based on view angle
	if ( clip & 2 )
		SV_WallFriction (ent, &steptrace);

// move down
	downtrace = SV_PushEntity (ent, downmove);	// FIXME: don't link?

	if (downtrace.plane.normal[2] > 0.7)
	{
		if (ent->v.solid == SOLID_BSP)
		{
			ent->v.flags =	(int)ent->v.flags | FL_ONGROUND;
			ent->v.groundentity = EDICT_TO_PROG(downtrace.ent);
		}
	}
	else
	{
// if the push down didn't end up on good ground, use the move without
// the step up.  This happens near wall / slope combinations, and can
// cause the player to hop up higher on a slope too steep to climb	
		VectorCopy (nosteporg, ent->v.origin);
		VectorCopy (nostepvel, ent->v.velocity);
	}
}

void NewPredatorGravity(edict_t *ent)
{

	int i;
	trace_t	trace;
	vec3_t end, normal,start;

	memset (&trace, 0, sizeof(trace));

	VectorCopy (ent->v.origin, start);

	if ( !ent->v.movedir[0] && !ent->v.movedir[1] &&  !ent->v.movedir[2]) {
			ent->v.movedir[0] = 0;
			ent->v.movedir[1] = 0;
			ent->v.movedir[2] = 100;
		}

	VectorAdd( start, ent->v.movedir, end );

	VectorCopy (end, trace.endpos);
	trace.fraction = 1;

	//VectorAdd( ent->v.origin, start );

	start[0] = ent->v.origin[0];
	start[1] = ent->v.origin[1];
	start[2] = ent->v.origin[2];


	SV_RecursiveHullCheck (cl.worldmodel->hulls, 0, 0, 1, start, end, &trace);


	if ( trace.fraction <1  ) {
		VectorCopy( trace.plane.normal, normal );


		normal[0] *= -100;
		normal[1] *= -100;
		normal[2] *= -100;
		
		normal[2] += 10;


		VectorCopy( normal, ent->v.movedir );	
		i = ent->v.flags;

		Con_Printf(" %f %f %f vec\n", normal[0],normal[1],normal[2]);

		if ( i & FL_ONGROUND) {
			i -= FL_ONGROUND;
			ent->v.flags = i;
			}
	}

	

}

/*
================
SV_Physics_Client

Player character actions
================
*/
void SV_Physics_Client (edict_t	*ent, int num)
{
	if ( ! svs.clients[num-1].active )
		return;		// unconnected slot

//
// call standard client pre-think
//	
	pr_global_struct->time = sv.time;
	pr_global_struct->self = EDICT_TO_PROG(ent);
	PR_ExecuteProgram (pr_global_struct->PlayerPreThink);
	
//
// do a move
//
	SV_CheckVelocity (ent);

//
// decide which move function to call
//
	switch ((int)ent->v.movetype)
	{
	case MOVETYPE_NONE:
		if (!SV_RunThink (ent))
			return;
		break;

	case MOVETYPE_PREDATOR:
		if (!SV_RunThink (ent))
			return;
		
		NewPredatorGravity(ent);

		if (!SV_CheckWater (ent) && ! ((int)ent->v.flags & FL_WATERJUMP) )
				SV_AddMagnetic(ent);

		SV_AddGravity (ent);

		SV_CheckStuck (ent);
		SV_WalkMove (ent);
		break;

	case MOVETYPE_WALK:
		if (!SV_RunThink (ent))
			return;
		if (!SV_CheckWater (ent) && ! ((int)ent->v.flags & FL_WATERJUMP) )
			SV_AddGravity (ent);
		SV_CheckStuck (ent);
		SV_WalkMove (ent);

		break;
		
	case MOVETYPE_TOSS:
	case MOVETYPE_BOUNCE:
		SV_Physics_Toss (ent);
		break;

	case MOVETYPE_FLY:
		if (!SV_RunThink (ent))
			return;
		SV_FlyMove (ent, host_frametime, NULL);
		break;
		
	case MOVETYPE_NOCLIP:
		if (!SV_RunThink (ent))
			return;
		VectorMA (ent->v.origin, host_frametime, ent->v.velocity, ent->v.origin);
		break;
		
	default:
		Sys_Error ("SV_Physics_client: bad movetype %i", (int)ent->v.movetype);
	}

//
// call standard player post-think
//		
	SV_LinkEdict (ent, true);

	pr_global_struct->time = sv.time;
	pr_global_struct->self = EDICT_TO_PROG(ent);
	PR_ExecuteProgram (pr_global_struct->PlayerPostThink);
}

//============================================================================

/*
=============
SV_Physics_None

Non moving objects can only think
=============
*/
void SV_Physics_None (edict_t *ent)
{
// regular thinking
	SV_RunThink (ent);
}

// Tomaz - MOVETYPE_FOLLOW Begin
/*
=============
SV_Physics_Follow

Entities that are "stuck" to another entity
=============
*/
void SV_Physics_Follow (edict_t *ent)
{
	vec3_t	vf, vr, vu, angles, v;
	edict_t *e;

// regular thinking
	if (!SV_RunThink (ent))
		return;

	e = PROG_TO_EDICT(ent->v.aiment);
	if (e->v.angles[0] == ent->v.punchangle[0] && e->v.angles[1] == ent->v.punchangle[1] && e->v.angles[2] == ent->v.punchangle[2])
	{
		// quick case for no rotation
		VectorAdd(e->v.origin, ent->v.view_ofs, ent->v.origin);
	}
	else
	{
		angles[0] = -ent->v.punchangle[0];
		angles[1] =  ent->v.punchangle[1];
		angles[2] =  ent->v.punchangle[2];
		AngleVectors (angles, vf, vr, vu);
		v[0] = ent->v.view_ofs[0] * vf[0] + ent->v.view_ofs[1] * vr[0] + ent->v.view_ofs[2] * vu[0];
		v[1] = ent->v.view_ofs[0] * vf[1] + ent->v.view_ofs[1] * vr[1] + ent->v.view_ofs[2] * vu[1];
		v[2] = ent->v.view_ofs[0] * vf[2] + ent->v.view_ofs[1] * vr[2] + ent->v.view_ofs[2] * vu[2];
		angles[0] = -e->v.angles[0];
		angles[1] =  e->v.angles[1];
		angles[2] =  e->v.angles[2];
		AngleVectors (angles, vf, vr, vu);
		ent->v.origin[0] = v[0] * vf[0] + v[1] * vf[1] + v[2] * vf[2] + e->v.origin[0];
		ent->v.origin[1] = v[0] * vr[0] + v[1] * vr[1] + v[2] * vr[2] + e->v.origin[1];
		ent->v.origin[2] = v[0] * vu[0] + v[1] * vu[1] + v[2] * vu[2] + e->v.origin[2];
	}
	VectorAdd (e->v.angles, ent->v.v_angle, ent->v.angles);
	SV_LinkEdict (ent, true);
}
// Tomaz - MOVETYPE_FOLLOW End


// Tei relative static

 // Simple version of follow without rotations

void SV_Physics_Relative (edict_t *ent)
{
	//vec3_t	vf, vr, vu, angles, v;
	edict_t *e;

	// regular thinking
	if (!SV_RunThink (ent))
		return;

	e = PROG_TO_EDICT(ent->v.aiment);

	// Pos is relative to other + view_ofs
	VectorAdd( e->v.origin, ent->v.view_ofs, ent->v.origin );

	SV_LinkEdict (ent, true);
}
// Tei relative static









/*
=============
SV_Physics_Noclip

A moving object that doesn't obey physics
=============
*/
void SV_Physics_Noclip (edict_t *ent)
{
// regular thinking
	if (!SV_RunThink (ent))
		return;
	
	VectorMA (ent->v.angles, host_frametime, ent->v.avelocity, ent->v.angles);
	VectorMA (ent->v.origin, host_frametime, ent->v.velocity, ent->v.origin);

	SV_LinkEdict (ent, false);
}

/*
==============================================================================

TOSS / BOUNCE

==============================================================================
*/

/*
=============
SV_CheckWaterTransition

=============
*/

void 		R_FireStaticFog(vec3_t pos);
void SV_CheckWaterTransition (edict_t *ent)
{
	int		cont;

	cont = SV_PointContents (ent->v.origin);

	if (!ent->v.watertype)
	{	// just spawned here
		ent->v.watertype = cont;
		ent->v.waterlevel = 1;
		return;
	}




	if (cont <= CONTENTS_WATER)
	{
		if (ent->v.watertype == CONTENTS_EMPTY)
		{	// just crossed into water
			SV_StartSound (ent, 0, "misc/h2ohit1.wav", 255, 1);
			//R_FireStaticFog(ent->v.origin);//Tei xfx
			//Con_Printf("hello frm empty!\n");
		}		
		ent->v.watertype = cont;
		ent->v.waterlevel = 1;
	}
	else
	{
		if (ent->v.watertype != CONTENTS_EMPTY)
		{	// just crossed into water
			SV_StartSound (ent, 0, "misc/h2ohit1.wav", 255, 1);
			//Con_Printf("hello from water!\n");
		}	

		ent->v.watertype = CONTENTS_EMPTY;
		ent->v.waterlevel = cont;
	}
}

/*
=============
SV_Physics_Toss

Toss, bounce, and fly movement.  When onground, do nothing.
=============
*/
void SV_Physics_Toss (edict_t *ent)
{
	trace_t	trace;
	vec3_t	move;
	float	backoff;

	// regular thinking
	if (!SV_RunThink (ent))
		return;

// if onground, return without moving
	if ( ((int)ent->v.flags & FL_ONGROUND) )
		return;

	SV_CheckVelocity (ent);

// add gravity
	if (ent->v.movetype != MOVETYPE_FLY
	&& ent->v.movetype != MOVETYPE_BOUNCEMISSILE // LordHavoc: enabled MOVETYPE_BOUNCEMISSILE
	&& ent->v.movetype != MOVETYPE_FLYMISSILE)
		SV_AddGravity (ent);

// move angles
	VectorMA (ent->v.angles, host_frametime, ent->v.avelocity, ent->v.angles);

// move origin
	VectorScale (ent->v.velocity, host_frametime, move);
	trace = SV_PushEntity (ent, move);

	if (trace.fraction == 1)
		return;
	if (ent->free)
		return;
	
	if (ent->v.movetype == MOVETYPE_BOUNCE)
		backoff = 1.5;
	else
		backoff = 1;

	ClipVelocity (ent->v.velocity, trace.plane.normal, ent->v.velocity, backoff);

// stop if on ground
	if (trace.plane.normal[2] > 0.7)
	{		
		if (ent->v.velocity[2] < 60 || ent->v.movetype != MOVETYPE_BOUNCE)
		{
			ent->v.flags = (int)ent->v.flags | FL_ONGROUND;
			ent->v.groundentity = EDICT_TO_PROG(trace.ent);
			VectorCopy (vec3_origin, ent->v.velocity);
			VectorCopy (vec3_origin, ent->v.avelocity);
		}
	}
	
// check for in water
	SV_CheckWaterTransition (ent);
}

/*
===============================================================================

STEPPING MOVEMENT

===============================================================================
*/

/*
=============
SV_Physics_Step

Monsters freefall when they don't have a ground entity, otherwise
all movement is done with discrete steps.

This is also used for objects that have become still on the ground, but
will fall if the floor is pulled out from under them.
=============
*/



void SV_Physics_Step (edict_t *ent)
{
	qboolean	hitsound;
	vec3_t pos2;

// freefall if not onground
	if ( ! ((int)ent->v.flags & (FL_ONGROUND | FL_FLY | FL_SWIM) ) )
	{
		if (ent->v.velocity[2] < sv_gravity.value*-0.1)
			hitsound = true;
		else
			hitsound = false;

		SV_AddGravity (ent);
		SV_CheckVelocity (ent);
		SV_FlyMove (ent, host_frametime, NULL);
		SV_LinkEdict (ent, true);

		if ( (int)ent->v.flags & FL_ONGROUND )	// just hit ground
		{
			if (hitsound) 
			{
				SV_StartSound (ent, 0, "demon/dland2.wav", 255, 1);
				//Tei smoke on landing
				if (extra_fx.value >1)
				{				
					VectorCopy(ent->v.origin,pos2);
					pos2[2] -= ent->v.maxs[2];
					R_ParticleExplosionSmoke(pos2);//Tei xfx 
				}
				//Tei smoke on landing
			}
		}
	}

// regular thinking
	SV_RunThink (ent);
	
	SV_CheckWaterTransition (ent);
}

//============================================================================

/*
================
SV_Physics

================
*/
void SV_Physics (void)
{
	int		i;
	edict_t	*ent;

// let the progs know that a new frame has started
	pr_global_struct->self = EDICT_TO_PROG(sv.edicts);
	pr_global_struct->other = EDICT_TO_PROG(sv.edicts);
	pr_global_struct->time = sv.time;
	PR_ExecuteProgram (pr_global_struct->StartFrame);

//
// treat each object in turn
//
	ent = sv.edicts;
	for (i=0 ; i<sv.num_edicts ; i++, ent = NEXT_EDICT(ent))
	{
		if (ent->free)
			continue;

		if (pr_global_struct->force_retouch)
		{
			SV_LinkEdict (ent, true);	// force retouch even for stationary
		}

		if (i > 0 && i <= svs.maxclients)
		{
			SV_Physics_Client (ent, i);
			continue;
		}

		switch ((int) ent->v.movetype)
		{
		case MOVETYPE_PUSH:
			SV_Physics_Pusher (ent);
			break;


		case MOVETYPE_NONE:
			//LordHavoc: speed
			//			SV_Physics_None (ent);
			// LH: manually inlined the thinktime check here because MOVETYPE_NONE is used on so many objects
			if (ent->v.nextthink > 0 && ent->v.nextthink <= sv.time + host_frametime)
				SV_RunThink (ent);

			//LordHavoc: speed
				

			break;

		// Tomaz - MOVETYPE_FOLLOW Begin
		case MOVETYPE_FOLLOW:
			SV_Physics_Follow (ent);
			break;
		// Tomaz - MOVETYPE_FOLLOW End

		case MOVETYPE_NOCLIP:
			SV_Physics_Noclip (ent);
			break;

		case MOVETYPE_STEP:
			SV_Physics_Step (ent);
			break;

		// Tomaz - MOVETYPE_WALK On Non-Players Begin
		/*
		case MOVETYPE_PREDATOR:
			if (SV_RunThink (ent))
			{
				if (!SV_CheckWater (ent) && ! ((int)ent->v.flags & FL_WATERJUMP) )
					SV_AddGravity (ent);
				SV_CheckStuck (ent);
				SV_WalkMove (ent);
			}
			break;
		*/
		case MOVETYPE_WALK:
			if (SV_RunThink (ent))
			{
				if (!SV_CheckWater (ent) && ! ((int)ent->v.flags & FL_WATERJUMP) )
					SV_AddGravity (ent);
				SV_CheckStuck (ent);
				SV_WalkMove (ent);
			}
			break;
		// Tomaz - MOVETYPE_WALK On Non-Players End

		//case MOVETYPE_PREDATOR:
		case MOVETYPE_TOSS:
		case MOVETYPE_BOUNCE:
		case MOVETYPE_BOUNCEMISSILE:
		case MOVETYPE_FLY:
		case MOVETYPE_FLYMISSILE:
			SV_Physics_Toss (ent);
			break;

		// Tei - Movetype Magnetic 
		case MOVETYPE_MAGNETIC:
			if (SV_RunThink (ent)) 
			{
				SV_AddMagnetic (ent);
				SV_Physics_Toss(ent);
			}
			break;

		case MOVETYPE_MAGNETICFOLLOW:
			if (SV_RunThink (ent)) 
			{
				SV_AddMagnetic2 (ent);
				SV_Physics_Toss(ent);
			}
			break;
		// Tei - Movetype Magnetic

		// Tei - Movetype Relative Static
		case MOVETYPE_RELATIVESTATIC:
			SV_Physics_Relative (ent);
			break;
		// Tei - Movetype Relative Static
	

		default:
			Host_Error ("SV_Physics: bad movetype %i", (int)ent->v.movetype);
			break;
		}			
	}
	
	if (pr_global_struct->force_retouch)
		pr_global_struct->force_retouch--;	

	sv.time += host_frametime;
}