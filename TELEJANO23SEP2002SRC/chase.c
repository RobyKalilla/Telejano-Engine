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
// chase.c -- chase camera code

#include "quakedef.h"

cvar_t	chase_back = {"chase_back", "100"};
cvar_t	chase_up = {"chase_up", "16"};
cvar_t	chase_right = {"chase_right", "0"};
cvar_t	chase_active = {"chase_active", "0"};
cvar_t	chase_trans = {"chase_trans", "0"};
cvar_t	firstperson = {"firstperson", "0"};


vec3_t	chase_pos;
vec3_t	chase_angles;

vec3_t	chase_dest;
vec3_t	chase_dest_angles;

//Tei viewmodes
void ToggleModes_f (void)
{
	if (firstperson.value) {
		Cvar_Set("firstperson","0");
		Cvar_Set("scr_ofsx","0");
		Cvar_Set("r_drawviewmodel","1");
		Cvar_Set("chase_active","1");
	}
	else
	if (chase_active.value) {
		Cvar_Set("r_drawviewmodel","1");
		Cvar_Set("chase_active","0");
	}
	else {
		Cvar_Set("firstperson","1");
		Cvar_Set("scr_ofsx","9");
		Cvar_Set("r_drawviewmodel","0");
	}
};
//Tei viewmodes


void Chase_Init (void)
{
	Cvar_RegisterVariable (&chase_back);
	Cvar_RegisterVariable (&chase_up);
	Cvar_RegisterVariable (&chase_right);
	Cvar_RegisterVariable (&chase_active);

	// Tei - chase trans
	Cvar_RegisterVariable (&firstperson);
	// Tei - chase trans
	
	Cmd_AddCommand ("toggleviewmode",		ToggleModes_f);
	
}

//Tei traceline

//Why this work, and why the hell this somethimes not work??!?!?!!
float CL_TraceLine (vec3_t start, vec3_t end, vec3_t impact, vec3_t normal, int contents, int hitbmodels) ;

float TTraceLine (vec3_t start, vec3_t end, vec3_t impact, vec3_t normal)
{
	vec3_t dumy;

	return CL_TraceLine(start,end, dumy, dumy, 0, true);
};

float TTraceLine_Old (vec3_t start, vec3_t end, vec3_t impact, vec3_t normal)
{
	trace_t	trace;
	trace_t	trace2;
//	moveclip_t	clip;

	memset (&trace, 0, sizeof(trace));
	VectorCopy (end, trace.endpos);
	trace.fraction = 1;
	SV_RecursiveHullCheck (cl.worldmodel->hulls, 0, 0, 1, start, end, &trace);
	VectorCopy (trace.endpos, impact);
	VectorCopy (trace.plane.normal, normal);

	if (trace.fraction <1)
		return trace.fraction;
	
	{	
		//Not while demos play
		if ( cls.state != ca_connected || cls.demoplayback )
			return trace.fraction;
	}
	

	trace2 = SV_Move (start, vec3_origin, vec3_origin, end, 0, 0);

	if (trace.fraction != trace2.fraction) {
	
		//Con_Printf(" trac1: %f, trac2: %f\n",trace.fraction, trace2.fraction);
		return trace2.fraction;
	}

	return trace.fraction;
}
//Tei traceline 

float TraceLine (vec3_t start, vec3_t end, vec3_t impact, vec3_t normal)
{
	trace_t	trace;

	memset (&trace, 0, sizeof(trace));
	VectorCopy (end, trace.endpos);
	trace.fraction = 1;
	SV_RecursiveHullCheck (cl.worldmodel->hulls, 0, 0, 1, start, end, &trace);
	VectorCopy (trace.endpos, impact);
	VectorCopy (trace.plane.normal, normal);

	return trace.fraction;
}



void Chase_Update (void)
{
	int		i;
	float	dist;
	vec3_t	forward, up, right, normal;
	vec3_t	dest, stop;


	// if can't see player, reset
	AngleVectors (cl.viewangles, forward, right, up);

	// calc exact destination
	for (i=0 ; i<3 ; i++)
		chase_dest[i] = r_refdef.vieworg[i] 
		- forward[i]*chase_back.value
		- right[i]*chase_right.value;
	chase_dest[2] = r_refdef.vieworg[2] + chase_up.value;

	// find the spot the player is looking at
	VectorMA (r_refdef.vieworg, 4096, forward, dest);
	TraceLine (r_refdef.vieworg, dest, stop, normal);

	// calculate pitch to look at the same spot from camera
	VectorSubtract (stop, r_refdef.vieworg, stop);
	dist = DotProduct (stop, forward);
	if (dist < 1)
		dist = 1;
	r_refdef.viewangles[PITCH] = -atan(stop[2] / dist) / M_PI * 180;

	// Tomaz - Chase Cam Fix Begin
	TraceLine(r_refdef.vieworg, chase_dest, stop, normal);
	if (stop[0] != 0 || stop[1] != 0 || stop[2] != 0)
		VectorCopy(stop, chase_dest);
	// Tomaz - Chase Cam Fix End

	// move towards destination
	VectorCopy (chase_dest, r_refdef.vieworg);
}

