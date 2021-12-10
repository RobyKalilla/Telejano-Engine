/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free softwasre; you can redistribute it and/or
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

#define	RETURN_EDICT(e) (((int *)pr_globals)[OFS_RETURN] = EDICT_TO_PROG(e))

char	pr_string_temp[128];

int SV_HullPointContents (hull_t *hull, int num, vec3_t p);


//Tei guislots

#define RETURNFLOAT(o)	{Con_Printf("res:%f\n",o);G_FLOAT(OFS_RETURN) = o;return;}

// Tei new lmp 
#define ORG_NW 0
#define ORG_NE 1
#define ORG_SW 2
#define ORG_SE 3
#define ORG_CC 4
#define ORG_CN 5
#define ORG_CS 6
#define ORG_CW 7
#define ORG_CE 8

#define CNT_ARRAY		1
#define CNT_STRING		2
#define CNT_ALPHA		3
#define CNT_DEFAULT		0
// Tei new lmp 

// Tei regparsing

// Tei read register
extern int eval_parse0;
extern int eval_parse1;
extern int eval_parse2;
extern int eval_parse3;
extern int eval_parse4;
extern int eval_parse5;
extern int eval_parse6;
extern int eval_parse7;
// Tei read register



// Tei regparsing


void ParamNegateFix ( float * xx, float * yy, int Zone )
{
	float x,y;
	x = xx[0];
	y = yy[0];

	if (Zone == ORG_CC ||   ORG_CW == Zone || ORG_CE == Zone )
		y = y + 8000;

	if (Zone == ORG_CC || ORG_CN == Zone || ORG_CS == Zone  )
		x = x + 8000;

	xx[0] = x;
	yy[0] = y;
}

void PR_ShowPic(void) 
{
	char *slot	= G_STRING(OFS_PARM0);
	char *picname = G_STRING(OFS_PARM1);
	float x		= G_FLOAT(OFS_PARM2);
	float y		= G_FLOAT(OFS_PARM3);
	float zone	= G_FLOAT(OFS_PARM4);

	ParamNegateFix( &x, &y, zone );

	if (sv.datagram.cursize > MAX_DATAGRAM-16)
		return;	

	MSG_WriteByte  (&sv.datagram, svc_showlmp2);
	MSG_WriteByte  (&sv.datagram, zone);//zone
	MSG_WriteString(&sv.datagram, slot);//lmp label
	MSG_WriteString(&sv.datagram, picname);//picname

	MSG_WriteShort (&sv.datagram, x);
	MSG_WriteShort (&sv.datagram, y);
};

void PR_ShowPicEnt(void) 
{
	char *slot	= G_STRING(OFS_PARM0);
	char *picname = G_STRING(OFS_PARM1);
	float x		= G_FLOAT(OFS_PARM2);
	float y		= G_FLOAT(OFS_PARM3);
	float zone	= G_FLOAT(OFS_PARM4);
	edict_t	* to= G_EDICT(OFS_PARM5);
	int entnum;//, ent;


	ParamNegateFix( &x, &y, zone );

	if (sv.datagram.cursize > MAX_DATAGRAM-16)
		return;	
	
	//ent = PROG_TO_EDICT(to);
	entnum = NUM_FOR_EDICT(to);//ent);

	if (entnum < 1 || entnum > svs.maxclients)
		PR_RunError ("WriteDest: not a client");

	MSG_WriteByte  (&svs.clients[entnum-1].message, svc_showlmp2);
	MSG_WriteByte  (&svs.clients[entnum-1].message, zone);//zone
	MSG_WriteString(&svs.clients[entnum-1].message, slot);//lmp label
	MSG_WriteString(&svs.clients[entnum-1].message, picname);//picname

	MSG_WriteShort (&svs.clients[entnum-1].message, x);
	MSG_WriteShort (&svs.clients[entnum-1].message, y);
};

void PR_HidePic(void) 
{
	char *slot	= G_STRING(OFS_PARM0);

	if (sv.datagram.cursize > MAX_DATAGRAM-16)
		return;	

	MSG_WriteByte  (&sv.datagram, svc_hidelmp);
	MSG_WriteString(&sv.datagram, slot);//lmp label
};

void PR_HidePicEnt(void) 
{
	char *slot	= G_STRING(OFS_PARM0);
	edict_t	* to= G_EDICT(OFS_PARM1);
	int entnum  =  NUM_FOR_EDICT(to);

	if (sv.datagram.cursize > MAX_DATAGRAM-16)
		return;	

	MSG_WriteByte  (&svs.clients[entnum-1].message, svc_hidelmp);
	MSG_WriteString(&svs.clients[entnum-1].message, slot);//lmp label

};


void PR_MovePic(void) 
{
	char *slot	= G_STRING(OFS_PARM0);
	float x		= G_FLOAT(OFS_PARM1);
	float y		= G_FLOAT(OFS_PARM2);
	float zone	= G_FLOAT(OFS_PARM3);

	ParamNegateFix( &x, &y, zone );

	if (sv.datagram.cursize > MAX_DATAGRAM-16)
		return;	

	MSG_WriteByte  (&sv.datagram, svc_movelmp);
	MSG_WriteString(&sv.datagram, slot);
	MSG_WriteByte  (&sv.datagram, zone);
	MSG_WriteShort (&sv.datagram, x);
	MSG_WriteShort (&sv.datagram, y);
};

void PR_MovePicEnt(void) 
{
	char *slot	= G_STRING(OFS_PARM0);
	float x		= G_FLOAT(OFS_PARM1);
	float y		= G_FLOAT(OFS_PARM2);
	float zone	= G_FLOAT(OFS_PARM3);
	edict_t	* to= G_EDICT(OFS_PARM4);
	int entnum  =  NUM_FOR_EDICT(to);

	ParamNegateFix( &x, &y, zone );

	if (sv.datagram.cursize > MAX_DATAGRAM-16)
		return;	

	MSG_WriteByte  (&svs.clients[entnum-1].message, svc_movelmp);
	MSG_WriteString(&svs.clients[entnum-1].message, slot);
	MSG_WriteByte  (&svs.clients[entnum-1].message, zone);
	MSG_WriteShort (&svs.clients[entnum-1].message, x);
	MSG_WriteShort (&svs.clients[entnum-1].message, y);
};


void PR_ChangePic(void)
{
	char *slot	= G_STRING(OFS_PARM0);
	char *newpic= G_STRING(OFS_PARM1);

	if (sv.datagram.cursize > MAX_DATAGRAM-16)
		return;	

	MSG_WriteByte  (&sv.datagram, svc_updatelmp);
	MSG_WriteString(&sv.datagram, slot);
	MSG_WriteString(&sv.datagram, newpic);

};


void PR_ChangePicEnt(void)
{
	
	char *slot	= G_STRING(OFS_PARM0);
	char *newpic= G_STRING(OFS_PARM1);
	edict_t	* to= G_EDICT(OFS_PARM2);
	int entnum  = NUM_FOR_EDICT(to);

	if (sv.datagram.cursize > MAX_DATAGRAM-16)
		return;	

	MSG_WriteByte  (&svs.clients[entnum-1].message, svc_updatelmp);
	MSG_WriteString(&svs.clients[entnum-1].message, slot);
	MSG_WriteString(&svs.clients[entnum-1].message, newpic);

};



//Tei guislots

/*
===============================================================================

						BUILT-IN FUNCTIONS

===============================================================================
*/

char *PF_VarString (int	first)
{
	int		i;
	static char out[256];
	
	out[0] = 0;
	for (i=first ; i<pr_argc ; i++)
	{
		strcat (out, G_STRING((OFS_PARM0+i*3)));
	}
	return out;
}

#if 0 //Complete Implementation
char *QSG_EXTENSIONS = "\
TQ_DLIGHTS \
TQ_RAIN \
TQ_SNOW \
TQ_RAILTRAIL \
TQ_PLASMA \
TQ_SLOWMO \
DP_ENT_ALPHA \
DP_ENT_SCALE \
DP_QUAKE2_MODEL \
DP_HALFLIFE_MAP \
DP_SV_DRAWONLYTOCLIENT \
DP_SV_NODRAWTOCLIENT \
TEI_EF_EXTRA \
TEI_EF2_EXTRA \
TEI_EF3_EXTRA \
TEI_MOVMAGNETIC \
TEI_MOVMAGNETICFOLLOW \
TEI_MOVRELATIVESTATIC \
TEI_MOD_PROGS \
TEI_AUTORAIN \
TEI_AUTOZING \
TEI_AUTOSNOW \
TEI_AUTOLIGHTDAY \
TEI_AUTOLAVA \
TEI_AUTOFLUOR \
TEI_AUTOFOGWATER \
TEI_BOB2 \
TEI_STATICFX \
TEI_MODPREDATOR \
TEI_TPAKLOAD \
TEI_MAPMODELS \
TEI_THEMEFOLDER \
TEI_SVSKY \
TEI_MENU_MISC2 \
TEI_STEPSIZE \
TEI_AUTOANIMATION \
TEI_AUTOVANISH \
TEI_TE_PARTICLEBLAST \
TEI_TE_IMPLOSIONFX \
TEI_TE_STATICFOG \
TEI_TE_LIGHTNINGX \
TEI_TE_LIGHTNING4 \
TEI_TE_TRAILS \
TEI_TE_ALIENEXPLOSION \
TEI_TE_EXPLOSIONSMALL3 \
TEI_TE_EXPLOSIONSMALL2 \
TEI_TE_EXPLOSIONSMALL \
TEI_TE_CUSTOMFX \
TEI_SHOWLMP2 \
TEI_SHOWLMP3 \
TEI_STATCOUNT \
TEI_HUDMANAGER \
TEI_ZONEFOGS \
TEI_LOCALMENUS1 \
TEI_LOCALMENUS2 \
\0";
#else //actual broken implementation (underscore to deactivate)
char *QSG_EXTENSIONS = "\
TQ_DLIGHTS \
TQ_RAIN \
TQ_SNOW \
TQ_RAILTRAIL \
TQ_PLASMA \
TQ_SLOWMO \
_DP_ENT_ALPHA \
_DP_ENT_SCALE \
DP_QUAKE2_MODEL \
DP_HALFLIFE_MAP \
_DP_SV_DRAWONLYTOCLIENT \
_DP_SV_NODRAWTOCLIENT \
_TEI_EF_EXTRA \
_TEI_EF2_EXTRA \
_TEI_EF3_EXTRA \
TEI_MOVMAGNETIC \
TEI_MOVMAGNETICFOLLOW \
TEI_MOVRELATIVESTATIC \
TEI_MOD_PROGS \
TEI_AUTORAIN \
TEI_AUTOZING \
TEI_AUTOSNOW \
TEI_AUTOLIGHTDAY \
TEI_AUTOLAVA \
TEI_AUTOFLUOR \
TEI_AUTOFOGWATER \
TEI_BOB2 \
TEI_STATICFX \
TEI_MODPREDATOR \
TEI_TPAKLOAD \
TEI_MAPMODELS \
TEI_THEMEFOLDER \
TEI_SVSKY \
TEI_MENU_MISC2 \
TEI_STEPSIZE \
TEI_AUTOANIMATION \
TEI_AUTOVANISH \
TEI_TE_PARTICLEBLAST \
TEI_TE_IMPLOSIONFX \
TEI_TE_STATICFOG \
TEI_TE_LIGHTNINGX \
TEI_TE_LIGHTNING4 \
TEI_TE_TRAILS \
TEI_TE_ALIENEXPLOSION \
TEI_TE_EXPLOSIONSMALL3 \
TEI_TE_EXPLOSIONSMALL2 \
TEI_TE_EXPLOSIONSMALL \
TEI_TE_CUSTOMFX \
TEI_SHOWLMP2 \
TEI_SHOWLMP3 \
TEI_STATCOUNT \
TEI_HUDMANAGER \
TEI_ZONEFOGS \
TEI_LOCALMENUS1 \
TEI_LOCALMENUS2 \
\0";

#endif

/*
TEI_SUN \
TEI_MOVPREDATOR \
TEI_MOVCHASECAM \
TEI_MOVWATERFLOAT \
TEI_ENGINEVERSION \
*/

qboolean checkextension(char *name)
{
	int len;
	char *e;
	len = strlen(name);
	for (e = QSG_EXTENSIONS;*e;e++)
	{
		while (*e == ' ')
			e++;
		if (!*e)
			break;
		if (!Q_strncasecmp(e, name, len))
			return true;
		while (*e && *e != ' ')
			e++;
	}
	return false;
}

/*
=================
PF_checkextension

returns true if the extension is supported by the server

checkextension(extensionname)
=================
*/
void PF_checkextension (void)
{
	G_FLOAT(OFS_RETURN) = checkextension(G_STRING(OFS_PARM0));
}

/*
=================
PF_errror

This is a TERMINAL error, which will kill off the entire server.
Dumps self.

error(value)
=================
*/
void PF_error (void)
{
	char	*s;
	edict_t	*ed;
	
	s = PF_VarString(0);
	Con_Printf ("======SERVER ERROR in %s:\n%s\n"
	,pr_strings + pr_xfunction->s_name,s);
	ed = PROG_TO_EDICT(pr_global_struct->self);
	ED_Print (ed);

	Host_Error ("Program error");
}

/*
=================
PF_objerror

Dumps out self, then an error message.  The program is aborted and self is
removed, but the level can continue.

objerror(value)
=================
*/
void PF_objerror (void)
{
	char	*s;
	edict_t	*ed;
	
	s = PF_VarString(0);
	Con_Printf ("======OBJECT ERROR in %s:\n%s\n"
	,pr_strings + pr_xfunction->s_name,s);
	ed = PROG_TO_EDICT(pr_global_struct->self);
	ED_Print (ed);
	ED_Free (ed);
	
	Host_Error ("Program error");
}



/*
==============
PF_makevectors

Writes new values for v_forward, v_up, and v_right based on angles
makevectors(vector)
==============
*/
void PF_makevectors (void)
{
	AngleVectors (G_VECTOR(OFS_PARM0), pr_global_struct->v_forward, pr_global_struct->v_right, pr_global_struct->v_up);
}

/*
=================
PF_setorigin

This is the only valid way to move an object without using the physics of the world (setting velocity and waiting).  Directly changing origin will not set internal links correctly, so clipping would be messed up.  This should be called when an object is spawned, and then only if it is teleported.

setorigin (entity, origin)
=================
*/
void PF_setorigin (void)
{
	edict_t	*e;
	float	*org;
	
	e = G_EDICT(OFS_PARM0);
	org = G_VECTOR(OFS_PARM1);
	VectorCopy (org, e->v.origin);
	SV_LinkEdict (e, false);
}


void SetMinMaxSize (edict_t *e, float *min, float *max, qboolean rotate)
{
	float	*angles;
	vec3_t	rmin, rmax;
	float	bounds[2][3];
	float	xvector[2], yvector[2];
	float	a;
	vec3_t	base, transformed;
	int		i, j, k, l;
	
	for (i=0 ; i<3 ; i++)
		if (min[i] > max[i])
			PR_RunError ("backwards mins/maxs");

	rotate = false;		// FIXME: implement rotation properly again

	if (!rotate)
	{
		VectorCopy (min, rmin);
		VectorCopy (max, rmax);
	}
	else
	{
	// find min / max for rotations
		angles = e->v.angles;
		
		a = angles[1]/180 * M_PI;
		
		xvector[0] = cos(a);
		xvector[1] = sin(a);
		yvector[0] = -sin(a);
		yvector[1] = cos(a);
		
		VectorCopy (min, bounds[0]);
		VectorCopy (max, bounds[1]);
		
		rmin[0] = rmin[1] = rmin[2] = 9999;
		rmax[0] = rmax[1] = rmax[2] = -9999;
		
		for (i=0 ; i<= 1 ; i++)
		{
			base[0] = bounds[i][0];
			for (j=0 ; j<= 1 ; j++)
			{
				base[1] = bounds[j][1];
				for (k=0 ; k<= 1 ; k++)
				{
					base[2] = bounds[k][2];
					
				// transform the point
					transformed[0] = xvector[0]*base[0] + yvector[0]*base[1];
					transformed[1] = xvector[1]*base[0] + yvector[1]*base[1];
					transformed[2] = base[2];
					
					for (l=0 ; l<3 ; l++)
					{
						if (transformed[l] < rmin[l])
							rmin[l] = transformed[l];
						if (transformed[l] > rmax[l])
							rmax[l] = transformed[l];
					}
				}
			}
		}
	}
	
// set derived values
	VectorCopy (rmin, e->v.mins);
	VectorCopy (rmax, e->v.maxs);
	VectorSubtract (max, min, e->v.size);
	
	SV_LinkEdict (e, false);
}

/*
=================
PF_setsize

the size box is rotated by the current angle

setsize (entity, minvector, maxvector)
=================
*/
void PF_setsize (void)
{
	edict_t	*e;
	float	*min, *max;
	
	e = G_EDICT(OFS_PARM0);
	min = G_VECTOR(OFS_PARM1);
	max = G_VECTOR(OFS_PARM2);
	SetMinMaxSize (e, min, max, false);
}


/*
=================
PF_setmodel

setmodel(entity, model)
=================
*/

	


void PF_setmodel (void)
{
	edict_t	*e;
	char	*m, **check;
	model_t	*mod;
	int		i;

	e = G_EDICT(OFS_PARM0);
	m = G_STRING(OFS_PARM1);

	//Fix no existant fire model

	if (gl_particle_fire.value)
	{
		if (!strcmp (m, "progs/flame.mdl")) 
		{
			for (i=0, check = sv.model_precache ; *check ; i++, check++)
				if (!strcmp(*check, "progs/fire.mdl")) 
				{
					m = "progs/fire.mdl";
					break;
				}

		}

		if (!strcmp (m, "progs/flame2.mdl") ||
			!strcmp (m, "progs/flame3.mdl"))
			for (i=0, check = sv.model_precache ; *check ; i++, check++) 
			{
				if (!strcmp(*check, "progs/fire2.mdl")) 
				{
					m = "progs/fire2.mdl";
					break;
				}
			}
	}
	//Fix no existant fire model

// check to see if model was properly precached
	for (i=0, check = sv.model_precache ; *check ; i++, check++)
		if (!strcmp(*check, m))
			break;
			
	if (!*check)
		PR_RunError ("no precache: %s\n", m);
		

	e->v.model = m - pr_strings;
	e->v.modelindex = i; //SV_ModelIndex (m);

	mod = sv.models[ (int)e->v.modelindex];  // Mod_ForName (m, true);
	
	if (mod)
		SetMinMaxSize (e, mod->mins, mod->maxs, true);
	else
		SetMinMaxSize (e, vec3_origin, vec3_origin, true);
}

/*
=================
PF_bprint

broadcast print to everyone on server

bprint(value)
=================
*/
void PF_bprint (void)
{
	char		*s;

	s = PF_VarString(0);

	//Tei speed
	if (!deathmatch.value && !coop.value)
		SV_BroadcastPrintf ("%s", s);
	else
		Con_Printf ("%s", s);
}

/*
=================
PF_sprint

single print to a specific client

sprint(clientent, value)
=================
*/
void PF_sprint (void)
{
	char		*s;
	client_t	*client;
	int			entnum;
	
	entnum = G_EDICTNUM(OFS_PARM0);
	s = PF_VarString(1);
	
	if (entnum < 1 || entnum > svs.maxclients)
	{
		Con_Printf ("tried to sprint to a non-client\n");
		return;
	}
		
	client = &svs.clients[entnum-1];

 //Tei SPE stuff 
	if (!deathmatch.value && !coop.value)
	{
		Con_Printf ("%s", s);
	}
	else
	{
		MSG_WriteChar (&client->message,svc_print);
		MSG_WriteString (&client->message, s );
	}
}


/*
=================
PF_centerprint

single print to a specific client

centerprint(clientent, value)
=================
*/
void PF_centerprint (void)
{
	char		*s;
	client_t	*client;
	int			entnum;
	
	entnum = G_EDICTNUM(OFS_PARM0);
	s = PF_VarString(1);
	
	if (entnum < 1 || entnum > svs.maxclients)
	{
		Con_Printf ("tried to sprint to a non-client\n");
		return;
	}
		
	client = &svs.clients[entnum-1];

	if (!deathmatch.value && !coop.value) //Tei spe-for telejano
		SCR_CenterPrint (s);
	else
	{
		MSG_WriteChar (&client->message,svc_centerprint);
		MSG_WriteString (&client->message, s );
	}
}


/*
=================
PF_normalize

vector normalize(vector)
=================
*/
void PF_normalize (void)
{
	float	*value1;
	vec3_t	newvalue;
	float	new;
	
	value1 = G_VECTOR(OFS_PARM0);

	new = value1[0] * value1[0] + value1[1] * value1[1] + value1[2]*value1[2];
	new = sqrt(new);
	
	if (new == 0)
		newvalue[0] = newvalue[1] = newvalue[2] = 0;
	else
	{
		new = 1/new;
		newvalue[0] = value1[0] * new;
		newvalue[1] = value1[1] * new;
		newvalue[2] = value1[2] * new;
	}
	
	VectorCopy (newvalue, G_VECTOR(OFS_RETURN));	
}

/*
=================
PF_vlen

scalar vlen(vector)
=================
*/
void PF_vlen (void)
{
	float	*value1;
	float	new;
	
	value1 = G_VECTOR(OFS_PARM0);

	new = value1[0] * value1[0] + value1[1] * value1[1] + value1[2]*value1[2];
	new = sqrt(new);
	
	G_FLOAT(OFS_RETURN) = new;
}

/*
=================
PF_vectoyaw

float vectoyaw(vector)
=================
*/
void PF_vectoyaw (void)
{
	float	*value1;
	float	yaw;
	
	value1 = G_VECTOR(OFS_PARM0);

	if (value1[1] == 0 && value1[0] == 0)
		yaw = 0;
	else
	{
		yaw = (int) (atan2(value1[1], value1[0]) * 57.295779513082320);	// Tomaz Speed
		if (yaw < 0)
			yaw += 360;
	}

	G_FLOAT(OFS_RETURN) = yaw;
}


/*
=================
PF_vectoangles

vector vectoangles(vector)
=================
*/
void PF_vectoangles (void)
{
	float	*value1;
	float	forward;
	float	yaw, pitch;
	
	value1 = G_VECTOR(OFS_PARM0);

	if (value1[1] == 0 && value1[0] == 0)
	{
		yaw = 0;
		if (value1[2] > 0)
			pitch = 90;
		else
			pitch = 270;
	}
	else
	{
		yaw = (int) (atan2(value1[1], value1[0]) * 57.295779513082320);	// Tomaz Speed
		if (yaw < 0)
			yaw += 360;

		forward = sqrt (value1[0]*value1[0] + value1[1]*value1[1]);
		pitch = (int) (atan2(value1[2], forward) * 57.295779513082320);	// Tomaz Speed
		if (pitch < 0)
			pitch += 360;
	}

	G_FLOAT(OFS_RETURN+0) = pitch;
	G_FLOAT(OFS_RETURN+1) = yaw;
	G_FLOAT(OFS_RETURN+2) = 0;
}

/*
=================
PF_Random

Returns a number from 0<= num < 1

random()
=================
*/
void PF_random (void)
{
	float		num;
		
	num = (rand ()&0x7fff) / ((float)0x7fff);
	
	G_FLOAT(OFS_RETURN) = num;
}

/*
=================
PF_particle

particle(origin, color, count)
=================
*/
void PF_particle (void)
{
	float		*org, *dir;
	float		color;
	float		count;
			
	org = G_VECTOR(OFS_PARM0);
	dir = G_VECTOR(OFS_PARM1);
	color = G_FLOAT(OFS_PARM2);
	count = G_FLOAT(OFS_PARM3);

	if (!deathmatch.value && !coop.value) //Tei spe-for telejano
		R_RunParticleEffect (org, dir, color, count);
	else
		SV_StartParticle (org, dir, color, count);
}


/*
=================
PF_ambientsound

=================
*/

int IsPrecacheSound( char * samp)
{
	char		**check;
	int soundnum;

	// check to see if samp was properly precached
	for (soundnum=0, check = sv.sound_precache ; *check ; check++, soundnum++)
		if (!strcmp(*check,samp))
			return soundnum;

	return false;
}

void PF_ambientsound (void)
{
	char		**check;
	char		*samp;
	float		*pos;
	float 		vol, attenuation;
	int			i, soundnum;

	pos = G_VECTOR (OFS_PARM0);			
	samp = G_STRING(OFS_PARM1);
	vol = G_FLOAT(OFS_PARM2);
	attenuation = G_FLOAT(OFS_PARM3);
	
// check to see if samp was properly precached
	for (soundnum=0, check = sv.sound_precache ; *check ; check++, soundnum++)
		if (!strcmp(*check,samp))
			break;
			
	if (!*check)
	{
		Con_Printf ("no precache: %s\n", samp);
		return;
	}

// add an svc_spawnambient command to the level signon packet
//	if (!deathmatch.value && !coop.value) //Tei spe-for telejano
//	S_StaticSound (cl.sound_precache[soundnum], pos, vol*255, attenuation*64);
//	else
//	{
		MSG_WriteByte (&sv.signon,svc_spawnstaticsound);
		for (i=0 ; i<3 ; i++)
			MSG_WriteCoord(&sv.signon, pos[i]);

		MSG_WriteByte (&sv.signon, soundnum);

		MSG_WriteByte (&sv.signon, vol*255);
		MSG_WriteByte (&sv.signon, attenuation*64);
	//}

}

/*
=================
PF_sound

Each entity can have eight independant sound sources, like voice,
weapon, feet, etc.

Channel 0 is an auto-allocate channel, the others override anything
allready running on that entity/channel pair.

An attenuation of 0 will play full volume everywhere in the level.
Larger attenuations will drop off.

=================
*/
void PF_sound (void)
{
	char		*sample;
	int			channel;
	edict_t		*entity;
	int 		volume, sound_num;
	float attenuation;
	vec3_t pos;
	int   ent;
	
		
	entity = G_EDICT(OFS_PARM0);
	channel = G_FLOAT(OFS_PARM1);
	sample = G_STRING(OFS_PARM2);
	volume = G_FLOAT(OFS_PARM3) * 255;
	attenuation = G_FLOAT(OFS_PARM4);
	
	channel %= 8;
	volume %= 256;

	if (attenuation < 0 || attenuation > 4)
		Sys_Error ("SV_StartSound: attenuation = %f", attenuation);

	if (!deathmatch.value && !coop.value) //Tei spe-for telejano
	{
		for (sound_num=1 ; sound_num<MAX_SOUNDS
			&& sv.sound_precache[sound_num] ; sound_num++)
			if (!strcmp(sample, sv.sound_precache[sound_num]))
				break;
    
		if ( sound_num == MAX_SOUNDS || !sv.sound_precache[sound_num] )
		{
			Con_Printf ("SV_StartSound: %s not precacheed\n", sample);
			return;
		}
		pos[0] = entity->v.origin[0]+0.5*(entity->v.mins[0]+entity->v.maxs[0]);
		pos[1] = entity->v.origin[1]+0.5*(entity->v.mins[1]+entity->v.maxs[1]);
		pos[2] = entity->v.origin[2]+0.5*(entity->v.mins[2]+entity->v.maxs[2]);

		ent = NUM_FOR_EDICT(entity);
		S_StartSound (ent, channel, cl.sound_precache[sound_num], pos, volume/255.0, attenuation);//SPE
	}	
	else
	{
		SV_StartSound (entity, channel, sample, volume, attenuation);//from server
	}
}

/*
=================
PF_break

break()
=================
*/
void PF_break (void)
{
Con_Printf ("break statement\n");
*(int *)-4 = 0;	// dump to debugger
//	PR_RunError ("break statement");
}

/*
=================
PF_traceline

Used for use tracing and shot targeting
Traces are blocked by bbox and exact bsp entityes, and also slide box entities
if the tryents flag is set.

traceline (vector1, vector2, tryents)
=================
*/
void PF_traceline (void)
{
	float	*v1, *v2;
	trace_t	trace;
	int		nomonsters;
	edict_t	*ent;

	v1 = G_VECTOR(OFS_PARM0);
	v2 = G_VECTOR(OFS_PARM1);
	nomonsters = G_FLOAT(OFS_PARM2);
	ent = G_EDICT(OFS_PARM3);

	trace = SV_Move (v1, vec3_origin, vec3_origin, v2, nomonsters, ent);

	pr_global_struct->trace_allsolid = trace.allsolid;
	pr_global_struct->trace_startsolid = trace.startsolid;
	pr_global_struct->trace_fraction = trace.fraction;
	pr_global_struct->trace_inwater = trace.inwater;
	pr_global_struct->trace_inopen = trace.inopen;
	VectorCopy (trace.endpos, pr_global_struct->trace_endpos);
	VectorCopy (trace.plane.normal, pr_global_struct->trace_plane_normal);
	pr_global_struct->trace_plane_dist =  trace.plane.dist;	
	if (trace.ent)
		pr_global_struct->trace_ent = EDICT_TO_PROG(trace.ent);
	else
		pr_global_struct->trace_ent = EDICT_TO_PROG(sv.edicts);
}

// Tei traceline
void PR_traceline ( vec3_t v1, vec3_t v2, float nomonsters, edict_t * ent)
{
	trace_t	trace;


	trace = SV_Move (v1, vec3_origin, vec3_origin, v2, nomonsters, ent);

	pr_global_struct->trace_allsolid = trace.allsolid;
	pr_global_struct->trace_startsolid = trace.startsolid;
	pr_global_struct->trace_fraction = trace.fraction;
	pr_global_struct->trace_inwater = trace.inwater;
	pr_global_struct->trace_inopen = trace.inopen;
	VectorCopy (trace.endpos, pr_global_struct->trace_endpos);
	VectorCopy (trace.plane.normal, pr_global_struct->trace_plane_normal);
	pr_global_struct->trace_plane_dist =  trace.plane.dist;	
	if (trace.ent)
		pr_global_struct->trace_ent = EDICT_TO_PROG(trace.ent);
	else
		pr_global_struct->trace_ent = EDICT_TO_PROG(sv.edicts);
}
// Tei traceline

/*
float (entity targ) visible =
{
	local vector	spot1, spot2;
	
	spot1 = self.origin + self.view_ofs;
       spot2 = targ.origin + (targ.mins + targ.maxs) * 0.5 + targ.view_ofs;

	traceline (spot1, spot2, TRUE, self);	// see through other monsters
	
	if (trace_inopen && trace_inwater)
  {
		return FALSE;			// sight line crossed contents
  }

  if (trace_ent == targ)
    return TRUE;

	if (trace_fraction == 1)
		return TRUE;
	return FALSE;
};
*/
//*

//Tei visible

void PF_VisibleEnt (void)
{
	vec3_t	v1, v2;
	trace_t	trace;
	//int		out;
	edict_t	*ent;
	edict_t	*eself;


	

	ent   = G_EDICT(OFS_PARM0);
	eself = G_EDICT(OFS_PARM1);//PROG_TO_EDICT(pr_global_struct->self);


	

	VectorCopy ( eself->v.origin, v1 );
    VectorCopy ( ent->v.origin, v2 );
	
	//return;
	//trace = SV_Move (v1, vec3_origin, vec3_origin, v2, 1, ent);
	trace = SV_Move ( v1, vec3_origin, vec3_origin,v2, 1, ent);

	//out = 1;

	if ( trace.inwater && trace.inopen ) {
		G_FLOAT(OFS_RETURN) = 0;
		return;
	}

	
	if ( trace.ent == ent ) {
		G_FLOAT(OFS_RETURN) = 1;
		return;
	}

	if  (trace.fraction > 0.99999) {
		G_FLOAT(OFS_RETURN) = 1;
		return;
	}
	G_FLOAT(OFS_RETURN) = 0;
	return;
}



void PF_Visible (void)
{
	vec3_t	v1, v2;
	trace_t	trace;
	//int		out;
	edict_t	*ent;
	edict_t	*eself;


	

	ent   = G_EDICT(OFS_PARM0);
	eself = PROG_TO_EDICT(pr_global_struct->self);


	

	VectorCopy ( eself->v.origin, v1 );
    VectorCopy ( ent->v.origin, v2 );
	
	//return;
	//trace = SV_Move (v1, vec3_origin, vec3_origin, v2, 1, ent);
	trace = SV_Move ( v1, vec3_origin, vec3_origin,v2, 1, ent);

	//out = 1;

	if ( trace.inwater && trace.inopen ) {
		G_FLOAT(OFS_RETURN) = 0;
		return;
	}

	
	if ( trace.ent == ent ) {
		G_FLOAT(OFS_RETURN) = 1;
		return;
	}

	if  (trace.fraction > 0.99999) {
		G_FLOAT(OFS_RETURN) = 1;
		return;
	}
	G_FLOAT(OFS_RETURN) = 0;
	return;
}

// Tei visible

/*

float(float v) anglemod =
{
	while (v >= 360)
		v = v - 360;
	while (v < 0)
		v = v + 360;
	return v;
};

*/

void PF_AngleMod (void)
{
	float	v;
	
	v = G_FLOAT(OFS_PARM0);
	
	v = anglemod( v );

	G_FLOAT(OFS_RETURN) = v;
}




//============================================================================

byte	checkpvs[MAX_MAP_LEAFS/8];

int PF_newcheckclient (int check)
{
	int		i;
	byte	*pvs;
	edict_t	*ent;
	mleaf_t	*leaf;
	vec3_t	org;

// cycle to the next one

	if (check < 1)
		check = 1;
	if (check > svs.maxclients)
		check = svs.maxclients;

	if (check == svs.maxclients)
		i = 1;
	else
		i = check + 1;

	for ( ;  ; i++)
	{
		if (i == svs.maxclients+1)
			i = 1;

		ent = EDICT_NUM(i);

		if (i == check)
			break;	// didn't find anything else

		if (ent->free)
			continue;
		if (ent->v.health <= 0)
			continue;
		if ((int)ent->v.flags & FL_NOTARGET)
			continue;

	// anything that is a client, or has a client as an enemy
		break;
	}

// get the PVS for the entity
	VectorAdd (ent->v.origin, ent->v.view_ofs, org);
	leaf = Mod_PointInLeaf (org, sv.worldmodel);
	pvs = Mod_LeafPVS (leaf, sv.worldmodel);
	memcpy (checkpvs, pvs, (sv.worldmodel->numleafs+7)>>3 );

	return i;
}

/*
=================
PF_checkclient

Returns a client (or object that has a client enemy) that would be a
valid target.

If there are more than one valid options, they are cycled each frame

If (self.origin + self.viewofs) is not in the PVS of the current target,
it is not returned at all.

name checkclient ()
=================
*/
#define	MAX_CHECK	16
int c_invis, c_notvis;
void PF_checkclient (void)
{
	edict_t	*ent, *self;
	mleaf_t	*leaf;
	int		l;
	vec3_t	view;
	
// find a new check if on a new frame
	if (sv.time - sv.lastchecktime >= 0.1)
	{
		sv.lastcheck = PF_newcheckclient (sv.lastcheck);
		sv.lastchecktime = sv.time;
	}

// return check if it might be visible	
	ent = EDICT_NUM(sv.lastcheck);
	if (ent->free || ent->v.health <= 0)
	{
		RETURN_EDICT(sv.edicts);
		return;
	}

// if current entity can't possibly see the check entity, return 0
	self = PROG_TO_EDICT(pr_global_struct->self);
	VectorAdd (self->v.origin, self->v.view_ofs, view);
	leaf = Mod_PointInLeaf (view, sv.worldmodel);
	l = (leaf - sv.worldmodel->leafs) - 1;
	if ( (l<0) || !(checkpvs[l>>3] & (1<<(l&7)) ) )
	{
c_notvis++;
		RETURN_EDICT(sv.edicts);
		return;
	}

// might be able to see it
c_invis++;
	RETURN_EDICT(ent);
}

//============================================================================


/*
=================
PF_stuffcmd

Sends text over to the client's execution buffer

stuffcmd (clientent, value)
=================
*/
void PF_stuffcmd (void)
{
	int		entnum;
	char	*str;
	client_t	*old;
	
	entnum = G_EDICTNUM(OFS_PARM0);
	if (entnum < 1 || entnum > svs.maxclients)
		PR_RunError ("Parm 0 not a client");
	str = G_STRING(OFS_PARM1);	
	
	old = host_client;
	host_client = &svs.clients[entnum-1];
	Host_ClientCommands ("%s", str);
	host_client = old;
}

// Tei rexec good
void Host_ClientCommands2 (char *fmt, ...);

void PF_stuffcmd2 (void)
{
	int		entnum;
	char	*str;
	client_t	*old;
	
	entnum = G_EDICTNUM(OFS_PARM0);
	if (entnum < 1 || entnum > svs.maxclients)
		PR_RunError ("Parm 0 not a client");
	str = G_STRING(OFS_PARM1);	
	
	old = host_client;
	host_client = &svs.clients[entnum-1];
	Host_ClientCommands2 ("%s", str);
	host_client = old;
}
// Tei rexec good

// Tei changefov
void PF_SetFov (void)
{
	int		entnum;
	char	val;
	client_t	*old;
	
	entnum = G_EDICTNUM(OFS_PARM0);
	if (entnum < 1 || entnum > svs.maxclients)
		PR_RunError ("Parm 0 not a client");
	val = G_FLOAT(OFS_PARM1);	
	
	old = host_client;
	host_client = &svs.clients[entnum-1];

	MSG_WriteByte (&host_client->message, svc_changefov);
	MSG_WriteByte (&host_client->message, val);

	host_client = old;
}
// Tei changefov



/*
=================
PF_localcmd

Sends text over to the client's execution buffer

localcmd (string)
=================
*/
void PF_localcmd (void)
{
	char	*str;
	
	str = G_STRING(OFS_PARM0);	
	Cbuf_AddText (str);
}

/*
=================
PF_cvar

float cvar (string)
=================
*/
void PF_cvar (void)
{
	char	*str;
	
	str = G_STRING(OFS_PARM0);
	
	G_FLOAT(OFS_RETURN) = Cvar_VariableValue (str);
}


void PF_cvarstring (void)
{
	char	*str;
	char * d;

	d = pr_string_temp;
	memset(d, 0, 127);

	str = G_STRING(OFS_PARM0);	


	Q_strcpy(pr_string_temp, Cvar_VariableString (str));

	G_INT(OFS_RETURN) = pr_string_temp - pr_strings;
}



/*
=================
PF_cvar_set

float cvar (string)
=================
*/
void PF_cvar_set (void)
{
	char	*var, *val;
	
	var = G_STRING(OFS_PARM0);
	val = G_STRING(OFS_PARM1);
	
	Cvar_Set (var, val);
}

/*
=================
PF_findradius

Returns a chain of entities that have origins within a spherical area

findradius (origin, radius)
=================
*/
void PF_findradius (void)
{
	edict_t	*ent, *chain;
	float	rad;
	float	*org;
	vec3_t	eorg;
	int		i, j;

	chain = (edict_t *)sv.edicts;
	
	org = G_VECTOR(OFS_PARM0);
	rad = G_FLOAT(OFS_PARM1);

	ent = NEXT_EDICT(sv.edicts);
	for (i=1 ; i<sv.num_edicts ; i++, ent = NEXT_EDICT(ent))
	{
		if (ent->free)
			continue;
		if (ent->v.solid == SOLID_NOT)
			continue;
		for (j=0 ; j<3 ; j++)
			eorg[j] = org[j] - (ent->v.origin[j] + (ent->v.mins[j] + ent->v.maxs[j])*0.5);			
		if (Length(eorg) > rad)
			continue;
			
		ent->v.chain = EDICT_TO_PROG(chain);
		chain = ent;
	}

	RETURN_EDICT(chain);
}


/*
=========
PF_dprint
=========
*/
void PF_dprint (void)
{
	Con_DPrintf ("%s",PF_VarString(0));
}


void PF_ftos (void)
{
	float	v;
	int		i;
	
	v = G_FLOAT(OFS_PARM0);
	
	if (v == (int)v)
		sprintf (pr_string_temp, "%d",(int)v);
	else
	{
		sprintf (pr_string_temp, "%1f", v);
		for (i=Q_strlen(pr_string_temp)-1 ; i>0 && pr_string_temp[i]=='0' && pr_string_temp[i-1]!='.' ; i--)
		{
			pr_string_temp[i] = 0;
		}
	}
	G_INT(OFS_RETURN) = pr_string_temp - pr_strings;
}

void PF_fabs (void)
{
	float	v;
	v = G_FLOAT(OFS_PARM0);
	G_FLOAT(OFS_RETURN) = fabs(v);
}

void PF_vtos (void)
{
	sprintf (pr_string_temp, "'%5.1f %5.1f %5.1f'", G_VECTOR(OFS_PARM0)[0], G_VECTOR(OFS_PARM0)[1], G_VECTOR(OFS_PARM0)[2]);
	G_INT(OFS_RETURN) = pr_string_temp - pr_strings;
}

void PF_Spawn (void)
{
	edict_t	*ed;
	ed = ED_Alloc();
	RETURN_EDICT(ed);
}

void PF_Remove (void)
{
	edict_t	*ed;
	
	ed = G_EDICT(OFS_PARM0);
	ED_Free (ed);
}


// entity (entity start, .string field, string match) find = #5;
void PF_Find (void)
{
	int		e;	
	int		f;
	char	*s, *t;
	edict_t	*ed;

	e = G_EDICTNUM(OFS_PARM0);
	f = G_INT(OFS_PARM1);
	s = G_STRING(OFS_PARM2);
	if (!s)
		PR_RunError ("PF_Find: bad search string");
		
	for (e++ ; e < sv.num_edicts ; e++)
	{
		ed = EDICT_NUM(e);
		if (ed->free)
			continue;
		t = E_STRING(ed,f);
		if (!t)
			continue;
		if (!strcmp(t,s))
		{
			RETURN_EDICT(ed);
			return;
		}
	}

	RETURN_EDICT(sv.edicts);
}

void PR_CheckEmptyString (char *s)
{
	if (s[0] <= ' ')
		PR_RunError ("Bad string");
}

void PF_precache_file (void)
{	// precache_file is only used to copy files with qcc, it does nothing
	G_INT(OFS_RETURN) = G_INT(OFS_PARM0);
}

void PF_precache_sound (void)
{
	char	*s;
	int		i;
	
	if (sv.state != ss_loading)
		PR_RunError ("PF_Precache_*: Precache can only be done in spawn functions");
		
	s = G_STRING(OFS_PARM0);
	G_INT(OFS_RETURN) = G_INT(OFS_PARM0);
	PR_CheckEmptyString (s);
	
	for (i=0 ; i<MAX_SOUNDS ; i++)
	{
		if (!sv.sound_precache[i])
		{
			sv.sound_precache[i] = s;
			return;
		}
		if (!strcmp(sv.sound_precache[i], s))
			return;
	}
	PR_RunError ("PF_precache_sound: overflow");
}

void PF_precache_model (void)
{
	char	*s;
	int		i;
	char	**check;
	
	if (sv.state != ss_loading)
		PR_RunError ("PF_Precache_*: Precache can only be done in spawn functions");
		
	s = G_STRING(OFS_PARM0);
	G_INT(OFS_RETURN) = G_INT(OFS_PARM0);

//	Con_Printf("Receive: %s\n",s);//XFX

	PR_CheckEmptyString (s);

#if 1
	//Fix no existant fire model
	if (gl_particle_fire.value)
	{
		if (!strcmp (s, "progs/flame.mdl")) 
		{
			for (i=0, check = sv.model_precache ; *check ; i++, check++)
				if (!strcmp(*check, "progs/fire.mdl")) 
				{
					s = "progs/fire.mdl";
					break;
				}

		}

		if (!strcmp (s, "progs/flame2.mdl") ||
			!strcmp (s, "progs/flame3.mdl"))
			for (i=0, check = sv.model_precache ; *check ; i++, check++) 
			{
				if (!strcmp(*check, "progs/fire2.mdl")) 
				{
					s = "progs/fire2.mdl";
					break;
				}
			}
	}
	//Fix no existant fire model
#else	
	if (gl_particle_fire.value)
	{
		if (!strcmp (s, "progs/flame.mdl"))
			s = "progs/fire.mdl";

		if (!strcmp (s, "progs/flame2.mdl") ||
			!strcmp (s, "progs/flame3.mdl"))
			s = "progs/fire2.mdl";
	}
#endif

	for (i=0 ; i<MAX_MODELS ; i++)
	{
		if (!sv.model_precache[i])
		{
			sv.model_precache[i] = s;
			sv.models[i] = Mod_ForName (s, true);
			return;
		}
		if (!strcmp(sv.model_precache[i], s))
			return;
	}

	PR_RunError ("PF_precache_model: overflow");
}


void PF_coredump (void)
{
	ED_PrintEdicts ();
}

void PF_traceon (void)
{
	pr_trace = true;
}

void PF_traceoff (void)
{
	pr_trace = false;
}

void PF_eprint (void)
{
	ED_PrintNum (G_EDICTNUM(OFS_PARM0));
}

/*
===============
PF_walkmove

float(float yaw, float dist) walkmove
===============
*/
void PF_walkmove (void)
{
	edict_t	*ent;
	float	yaw, dist;
	vec3_t	move;
	dfunction_t	*oldf;
	int 	oldself;
	
	ent = PROG_TO_EDICT(pr_global_struct->self);
	yaw = G_FLOAT(OFS_PARM0);
	dist = G_FLOAT(OFS_PARM1);
	
	if ( !( (int)ent->v.flags & (FL_ONGROUND|FL_FLY|FL_SWIM) ) )
	{
		G_FLOAT(OFS_RETURN) = 0;
		return;
	}

	yaw = yaw*6.283186307179586 / 360;	// Tomaz Speed
	
	move[0] = cos(yaw)*dist;
	move[1] = sin(yaw)*dist;
	move[2] = 0;

// save program state, because SV_movestep may call other progs
	oldf = pr_xfunction;
	oldself = pr_global_struct->self;
	
	G_FLOAT(OFS_RETURN) = SV_movestep(ent, move, true);
	
	
// restore program state
	pr_xfunction = oldf;
	pr_global_struct->self = oldself;
}

/*
===============
PF_droptofloor

void() droptofloor
===============
*/
void PF_droptofloor (void)
{
	edict_t		*ent;
	vec3_t		end;
	trace_t		trace;
	
	ent = PROG_TO_EDICT(pr_global_struct->self);

	VectorCopy (ent->v.origin, end);
	end[2] -= 256;
	
	trace = SV_Move (ent->v.origin, ent->v.mins, ent->v.maxs, end, false, ent);

	if (trace.fraction == 1 || trace.allsolid)
		G_FLOAT(OFS_RETURN) = 0;
	else
	{
		VectorCopy (trace.endpos, ent->v.origin);
		SV_LinkEdict (ent, false);
		ent->v.flags = (int)ent->v.flags | FL_ONGROUND;
		ent->v.groundentity = EDICT_TO_PROG(trace.ent);
		G_FLOAT(OFS_RETURN) = 1;
	}
}

/*
===============
PF_lightstyle

void(float style, string value) lightstyle
===============
*/
void PF_lightstyle (void)
{
	int		style;
	char	*val;
	client_t	*client;
	int			j;
	
	style = G_FLOAT(OFS_PARM0);
	val = G_STRING(OFS_PARM1);

// change the string in sv
	sv.lightstyles[style] = val;
	
// send message to all clients on this server
	if (sv.state != ss_active)
		return;
	
	for (j=0, client = svs.clients ; j<svs.maxclients ; j++, client++)
		if (client->active || client->spawned)
		{
			MSG_WriteChar (&client->message, svc_lightstyle);
			MSG_WriteChar (&client->message,style);
			MSG_WriteString (&client->message, val);
		}
}

void PF_rint (void)
{
	float	f;
	f = G_FLOAT(OFS_PARM0);
	if (f > 0)
		G_FLOAT(OFS_RETURN) = (int)(f + 0.5);
	else
		G_FLOAT(OFS_RETURN) = (int)(f - 0.5);
}
void PF_floor (void)
{
	G_FLOAT(OFS_RETURN) = floor(G_FLOAT(OFS_PARM0));
}
void PF_ceil (void)
{
	G_FLOAT(OFS_RETURN) = ceil(G_FLOAT(OFS_PARM0));
}


/*
=============
PF_checkbottom
=============
*/
void PF_checkbottom (void)
{
	edict_t	*ent;
	
	ent = G_EDICT(OFS_PARM0);

	G_FLOAT(OFS_RETURN) = SV_CheckBottom (ent);
}

/*
=============
PF_pointcontents
=============
*/
void PF_pointcontents (void)
{
	float	*v;
	
	v = G_VECTOR(OFS_PARM0);

	G_FLOAT(OFS_RETURN) = SV_PointContents (v);	
}


/*
=============
PF_nextent

entity nextent(entity)
=============
*/
void PF_nextent (void)
{
	int		i;
	edict_t	*ent;
	
	i = G_EDICTNUM(OFS_PARM0);
	while (1)
	{
		i++;
		if (i == sv.num_edicts)
		{
			RETURN_EDICT(sv.edicts);
			return;
		}
		ent = EDICT_NUM(i);
		if (!ent->free)
		{
			RETURN_EDICT(ent);
			return;
		}
	}
}

/*
=============
PF_aim

Pick a vector for the player to shoot along
vector aim(entity, missilespeed)
=============
*/
cvar_t	sv_aim = {"sv_aim", "1000"};
void PF_aim (void)
{
	edict_t	*ent, *check, *bestent;
	vec3_t	start, dir, end, bestdir;
	int		i, j;
	trace_t	tr;
	float	dist, bestdist;
	float	speed;
	
	ent = G_EDICT(OFS_PARM0);
	speed = G_FLOAT(OFS_PARM1);

	VectorCopy (ent->v.origin, start);
	start[2] += 20;

// try sending a trace straight
	VectorCopy (pr_global_struct->v_forward, dir);
	VectorMA (start, 2048, dir, end);
	tr = SV_Move (start, vec3_origin, vec3_origin, end, false, ent);
	if (tr.ent && tr.ent->v.takedamage == DAMAGE_AIM
	&& (!teamplay.value || ent->v.team <=0 || ent->v.team != tr.ent->v.team) )
	{
		VectorCopy (pr_global_struct->v_forward, G_VECTOR(OFS_RETURN));
		return;
	}


// try all possible entities
	VectorCopy (dir, bestdir);
	bestdist = sv_aim.value;
	bestent = NULL;
	
	check = NEXT_EDICT(sv.edicts);
	for (i=1 ; i<sv.num_edicts ; i++, check = NEXT_EDICT(check) )
	{
		if (check->v.takedamage != DAMAGE_AIM)
			continue;
		if (check == ent)
			continue;
		if (teamplay.value && ent->v.team > 0 && ent->v.team == check->v.team)
			continue;	// don't aim at teammate
		for (j=0 ; j<3 ; j++)
			end[j] = check->v.origin[j]
			+ 0.5*(check->v.mins[j] + check->v.maxs[j]);
		VectorSubtract (end, start, dir);
		VectorNormalize (dir);
		dist = DotProduct (dir, pr_global_struct->v_forward);
		if (dist < bestdist)
			continue;	// to far to turn
		tr = SV_Move (start, vec3_origin, vec3_origin, end, false, ent);
		if (tr.ent == check)
		{	// can shoot at this one
			bestdist = dist;
			bestent = check;
		}
	}
	
	if (bestent)
	{
		VectorSubtract (bestent->v.origin, ent->v.origin, dir);
		dist = DotProduct (dir, pr_global_struct->v_forward);
		VectorScale (pr_global_struct->v_forward, dist, end);
		end[2] = dir[2];
		VectorNormalize (end);
		VectorCopy (end, G_VECTOR(OFS_RETURN));	
	}
	else
	{
		VectorCopy (bestdir, G_VECTOR(OFS_RETURN));
	}
}

/*
==============
PF_changeyaw

This was a major timewaster in progs, so it was converted to C
==============
*/
void PF_changeyaw (void)
{
	edict_t		*ent;
	float		ideal, current, move, speed;
	
	ent = PROG_TO_EDICT(pr_global_struct->self);
	current = anglemod( ent->v.angles[1] );
	ideal = ent->v.ideal_yaw;
	speed = ent->v.yaw_speed;
	
	if (current == ideal)
		return;
	move = ideal - current;
	if (ideal > current)
	{
		if (move >= 180)
			move = move - 360;
	}
	else
	{
		if (move <= -180)
			move = move + 360;
	}

	if (move > 0)
	{
		if (move > speed)
			move = speed;
	}
	else
	{
		if (move < -speed)
			move = -speed;
	}
	
	ent->v.angles[1] = anglemod (current + move);
}

/*
===============================================================================

MESSAGE WRITING

===============================================================================
*/

#define	MSG_BROADCAST	0		// unreliable to all
#define	MSG_ONE			1		// reliable to one (msg_entity)
#define	MSG_ALL			2		// reliable to all
#define	MSG_INIT		3		// write to the init string

sizebuf_t *WriteDest (void)
{
	int		entnum;
	int		dest;
	edict_t	*ent;

	dest = G_FLOAT(OFS_PARM0);
	switch (dest)
	{
	case MSG_BROADCAST:
		return &sv.datagram;
	
	case MSG_ONE:
		ent = PROG_TO_EDICT(pr_global_struct->msg_entity);
		entnum = NUM_FOR_EDICT(ent);
		if (entnum < 1 || entnum > svs.maxclients)
			PR_RunError ("WriteDest: not a client");
		return &svs.clients[entnum-1].message;
		
	case MSG_ALL:
		return &sv.reliable_datagram;
	
	case MSG_INIT:
		return &sv.signon;

	default:
		PR_RunError ("WriteDest: bad destination");
		break;
	}
	
	return NULL;
}

void PF_WriteByte (void)
{
	MSG_WriteByte (WriteDest(), G_FLOAT(OFS_PARM1));
}

void PF_WriteChar (void)
{
	MSG_WriteChar (WriteDest(), G_FLOAT(OFS_PARM1));
}

void PF_WriteShort (void)
{
	MSG_WriteShort (WriteDest(), G_FLOAT(OFS_PARM1));
}

void PF_WriteLong (void)
{
	MSG_WriteLong (WriteDest(), G_FLOAT(OFS_PARM1));
}

void PF_WriteAngle (void)
{
	MSG_WriteAngle (WriteDest(), G_FLOAT(OFS_PARM1));
}

void PF_WriteCoord (void)
{
	MSG_WriteCoord (WriteDest(), G_FLOAT(OFS_PARM1));
}

void PF_WriteString (void)
{
	MSG_WriteString (WriteDest(), G_STRING(OFS_PARM1));
}


void PF_WriteEntity (void)
{
	MSG_WriteShort (WriteDest(), G_EDICTNUM(OFS_PARM1));
}

//=============================================================================

int SV_ModelIndex (char *name);

void PF_makestatic (void)
{
	edict_t	*ent;
	int		i;
	

	ent = G_EDICT(OFS_PARM0);

	MSG_WriteByte (&sv.signon,svc_spawnstatic);

	MSG_WriteByte (&sv.signon, SV_ModelIndex(pr_strings + ent->v.model));

	MSG_WriteByte (&sv.signon, ent->v.frame);
	MSG_WriteByte (&sv.signon, ent->v.colormap);
	MSG_WriteByte (&sv.signon, ent->v.skin);
	for (i=0 ; i<3 ; i++)
	{
		MSG_WriteCoord(&sv.signon, ent->v.origin[i]);
		MSG_WriteAngle(&sv.signon, ent->v.angles[i]);
	}

// throw the entity away now
	ED_Free (ent);
}

//=============================================================================

/*
==============
PF_setspawnparms
==============
*/
void PF_setspawnparms (void)
{
	edict_t	*ent;
	int		i;
	client_t	*client;

	ent = G_EDICT(OFS_PARM0);
	i = NUM_FOR_EDICT(ent);
	if (i < 1 || i > svs.maxclients)
		PR_RunError ("Entity is not a client");

	// copy spawn parms out of the client_t
	client = svs.clients + (i-1);

	for (i=0 ; i< NUM_SPAWN_PARMS ; i++)
		(&pr_global_struct->parm1)[i] = client->spawn_parms[i];
}

/*
==============
PF_changelevel
==============
*/
void PF_changelevel (void)
{
	char	*s;

// make sure we don't issue two changelevels
	if (svs.changelevel_issued)
		return;
	svs.changelevel_issued = true;
	
	s = G_STRING(OFS_PARM0);
	Cbuf_AddText (va("changelevel %s\n",s));
}

void PF_Fixme (void)
{
	//PR_RunError ("unimplemented bulitin");
	PR_RunError ("unimplemented builtin");//Tei typo fix.
}

// Tomaz - QuakeC String Manipulation Begin
void PF_zone (void)
{
	char *m, *p;
	m = G_STRING(OFS_PARM0);
	p = Z_Malloc(Q_strlen(m) + 1);
	Q_strcpy(p, m);

	G_INT(OFS_RETURN) = p - pr_strings;	
}


void PF_unzone (void)
{

	Z_Free(G_STRING(OFS_PARM0));
	G_INT(OFS_PARM0) = OFS_NULL; // empty the def
};

void PF_strlen (void)
{	

	char *p = G_STRING(OFS_PARM0);
	G_FLOAT(OFS_RETURN) = strlen(p);
}

char pr_strcat_buf [128]; // need this becuase pr_string_temp sucks

void PF_strcat (void)
{
	char *s1, *s2;

	memset(pr_strcat_buf, 0, 127);
	s1 = G_STRING(OFS_PARM0);
	s2 = PF_VarString(1);
	strcpy(pr_strcat_buf, s1);
	strcat(pr_strcat_buf, s2);
	G_INT(OFS_RETURN) = pr_strcat_buf - pr_strings;
}

void PF_substring (void)
{

	int ltwo, start;
	char *p, *d;
	d = pr_string_temp;

	p = G_STRING(OFS_PARM0);	
	start = (int)G_FLOAT(OFS_PARM1); // for some reason, Quake doesn't like G_INT
	ltwo = (int)G_FLOAT(OFS_PARM2);
	if ((unsigned)start > strlen(p))
		start = strlen(p) - 1;

	// cap values
	if (start < 0)
		start = 0;
	if (ltwo < 0)
		ltwo = 0;

	p += start;
	Q_strncpy(d, p, ltwo);
	G_INT(OFS_RETURN) = pr_string_temp - pr_strings;
		
}

// thanks zoid
void PF_stof (void)
{
	char	*s;

	s = G_STRING(OFS_PARM0);
	G_FLOAT(OFS_RETURN) = atof(s);
}

void PF_stov (void)
{
	char *v;
	int i;
	vec3_t d;
	
	v = G_STRING(OFS_PARM0);

	for (i=0; i<3; i++)
	{
		while(v && (v[0] == ' ' || v[0] == '\'')) //skip unneeded data
			v++;
		d[i] = atof(v);
		while (v && v[0] != ' ') // skip to next space
			v++;
	}
	VectorCopy (d, G_VECTOR(OFS_RETURN));
}
// Tomaz - QuakeC String Manipulation End

// Tomaz - QuakeC File System Begin
void PF_open (void)
{
	char *p = G_STRING(OFS_PARM0);
	char *ftemp;
	int fmode = G_FLOAT(OFS_PARM1);
	int h = 0, fsize = 0;

	switch (fmode)
	{  
		case 0: // read
			Sys_FileOpenRead (va("%s/%s",com_gamedir, p), &h);
			G_FLOAT(OFS_RETURN) = (float) h;
			return;
		case 1: // append -- this is nasty
			// copy whole file into the zone
			fsize = Sys_FileOpenRead(va("%s/%s",com_gamedir, p), &h);
			if (h == -1)
			{
				h = Sys_FileOpenWrite(va("%s/%s",com_gamedir, p));
				G_FLOAT(OFS_RETURN) = (float) h;
				return;
			}
			ftemp = Z_Malloc(fsize + 1);
			Sys_FileRead(h, ftemp, fsize);
			Sys_FileClose(h);
			// spit it back out
			h = Sys_FileOpenWrite(va("%s/%s",com_gamedir, p));
			Sys_FileWrite(h, ftemp, fsize);
			Z_Free(ftemp); // free it from memory
			G_FLOAT(OFS_RETURN) = (float) h;  // return still open handle
			return;
		default: // write
			h = Sys_FileOpenWrite (va("%s/%s", com_gamedir, p));
			G_FLOAT(OFS_RETURN) = (float) h; 
			return;
	}

}

void PF_close (void)
{
	int h = (int)G_FLOAT(OFS_PARM0);
	Sys_FileClose(h);
}

void PF_read (void)
{
	// reads one line (to a \n) into a string
	int h = (int)G_FLOAT(OFS_PARM0);
	int test;
	char *p;

	memset(pr_string_temp, 0, 127);
	p = pr_string_temp;
	Sys_FileRead(h, p, 1);
	while (p && p[0] != '\n')
	{
		*p++;
		test = Sys_FileRead(h, p, 1);
		if (p[0] == 13) // carriage return
			Sys_FileRead(h, p, 1); // skip
		if (!test)
			break;
	};
	p[0] = 0;
	if (strlen(pr_string_temp) == 0)
		G_INT(OFS_RETURN) = OFS_NULL;
	else
		G_INT(OFS_RETURN) = pr_string_temp - pr_strings;
}

void PF_write (void)
{
	// writes to file, like bprint
	float handle = G_FLOAT(OFS_PARM0);
	char *str = PF_VarString(1);
	float len;
	len = strlen(str);
	if (!len)
		Sys_FileWrite (handle, "", 0); 
	else
		Sys_FileWrite (handle, str, len); 
}
// Tomaz - QuakeC File System End

// Tei random vector PF
/*
=================
PF_rvector

Returns a vector from 0<= num < 1

rvector()
=================
*/
void PF_rvector (void)
{
	float		num1,num2,num3;
		
	num1 = ((rand ()&0x7fff) / ((float)0x7fff))*2 - 1;
	num2 = ((rand ()&0x7fff) / ((float)0x7fff))*2 - 1;
	num3 = ((rand ()&0x7fff) / ((float)0x7fff))*2 - 1;

	G_FLOAT(OFS_RETURN+0) = num1;
	G_FLOAT(OFS_RETURN+1) = num2;
	G_FLOAT(OFS_RETURN+2) = num3;

}
// Tei random vector PF


// Tei stat counter PF
/*
=================
PF_statcount

Returns stats of entities

statcount()
=================
*/
void PF_statcount (void)
{
	int		i;
	edict_t	*ent;
	int		active, models, solid, step;
	int option = (int)G_FLOAT(OFS_PARM0);


	active = models = solid = step = 0;
	for (i=0 ; i<sv.num_edicts ; i++)
	{
		ent = EDICT_NUM(i);
		if (ent->free)
			continue;
		active++;
		if (ent->v.solid)
			solid++;
		if (ent->v.model)
			models++;
		if (ent->v.movetype == MOVETYPE_STEP)
			step++;
	}

	switch (option)
	{  
		case 0: 
			G_FLOAT(OFS_RETURN+0) = sv.num_edicts;
			return;
		case 1:
			G_FLOAT(OFS_RETURN+0) = active;
			return;
		case 2:
			G_FLOAT(OFS_RETURN+0) = models;
			return;
		case 3:
			G_FLOAT(OFS_RETURN+0) = solid;
		default:
			G_FLOAT(OFS_RETURN+0) = step;
	}
}
// Tei stat counter PF



// Tei keytest manipulation
/*
=================
PF_keytest

Returns existence of string in "begin" string

statcount()
=================
*/

void PF_keytest (void)
{
	char *str;
	char *key;
	int equ, lenkey, offset,i;
		
	str = G_STRING(OFS_PARM0);
	key = G_STRING(OFS_PARM1);
	lenkey = strlen(key);
	offset = 0;


	while(str[offset] && (str[offset] == ' ' || str[offset] == '\t')) //skip unneeded data
			offset++;

	equ = 1;
	for ( i=0; i<lenkey; i++)
		if (str[offset+i] != key[i] ) equ = 0;
		
		
	
	G_FLOAT(OFS_RETURN) = equ;
}
// Tei keytest manipulation

// Tei PF_sscanf 
/*
=================
PF_sscanf

Scan string

vector sscanf(string src, string format)
=================
*/
void PF_sscanf (void)
{
	int e1 = 0;
	int e2 = 0;
	int e3 = 0;

	char * src		= G_STRING(OFS_PARM0);
	char * format	= G_STRING(OFS_PARM1);

	sscanf(src,format,e1,e2,e3,e1,e2,e3,e1,e2,e3,e1,e2,e3,e1,e2,e3);

	G_INT(OFS_RETURN + 0) = (float)e1;
	G_INT(OFS_RETURN + 1) = (float)e2;
	G_INT(OFS_RETURN + 2) = (float)e3;
}
// Tei PF_sscanf 

// Tei hvstring
void PF_hvstring (void)
{

	int l,i;
	char *p, *d;


	d = pr_string_temp;
	memset(d, 0, 127);

	p = G_STRING(OFS_PARM0);	

	l = strlen(p);
	for (i=0;i<l;i++){
		d[i*2+0]=p[i];
		d[i*2+1]='\n';
		d[i*2+2]=0;
	}

	G_INT(OFS_RETURN) = pr_string_temp - pr_strings;
}
// Tei hv string


ddef_t *ED_FindGlobal (char *name);
qboolean	ED_ParseEpair (void *base, ddef_t *key, char *s);

// Tei setarray
void PF_SetArray (void)
{
	//char *s1, *s2;
	ddef_t	*key;
	int val, len;
	float ret;
	char *p, *d, *valstring;
	//char tmp[200];


	d = pr_string_temp;
	memset(d, 0, 127);

	p = G_STRING(OFS_PARM0);	
	val = floor(G_FLOAT(OFS_PARM1));	
	valstring = G_STRING(OFS_PARM2);	

	len = strlen(p);
	Q_strncpy(pr_string_temp, p, len);

	d[len+0 ] = (val/   10)%10 + '0'; 
	d[len+1] = (val      )%10 + '0'; 			
	d[len+2] = 0;

	//ED_FindGlobal (new)
	key = ED_FindGlobal (d);
	if (key!=NULL)
		ret = ED_ParseEpair ((void *)pr_globals, key, valstring);
	
	G_FLOAT(OFS_RETURN) = (key!=NULL);
}
// Tei array


// Tei getarray
void PF_GetArray (void)
{
	//To do
}
// Tei getarray



// Tei fx
/*
=================
PF_setfx

Set render fx flags.

setfx(entity,flags)
=================
*/
/*
void PF_setfx (void)
{
	edict_t	*ent;
	int		i;
	
	ent = G_EDICT(OFS_PARM0);
	i =   G_INT	 (OFS_PARM1);

	ent->v.fx = i;
}
// Tei fx
*/
/*
float(entity targ, entity inflictor) CanDamage =
{
// bmodels need special checking because their origin is 0,0,0
	if (targ.movetype == MOVETYPE_PUSH)
	{
		traceline(inflictor.origin, 0.5 * (targ.absmin + targ.absmax), TRUE, self);
		if (trace_fraction == 1)
			return TRUE;
		if (trace_ent == targ)
			return TRUE;
		return FALSE;
	}
	
	traceline(inflictor.origin, targ.origin, TRUE, self);
	if (trace_fraction == 1)
		return TRUE;
	traceline(inflictor.origin, targ.origin + '15 15 0', TRUE, self);
	if (trace_fraction == 1)
		return TRUE;
	traceline(inflictor.origin, targ.origin + '-15 -15 0', TRUE, self);
	if (trace_fraction == 1)
		return TRUE;
	traceline(inflictor.origin, targ.origin + '-15 15 0', TRUE, self);
	if (trace_fraction == 1)
		return TRUE;
	traceline(inflictor.origin, targ.origin + '15 -15 0', TRUE, self);
	if (trace_fraction == 1)
		return TRUE;

	return FALSE;
};
*/

int PF_CanDamage ( edict_t * targ, edict_t * inflictor ) 
{
	trace_t	trace;
	vec3_t v1,v2;
	edict_t * self = PROG_TO_EDICT(pr_global_struct->self);
	
	VectorCopy( targ->v.origin		, v1 );
	VectorCopy( inflictor->v.origin	, v2 );


	if (targ->v.movetype == MOVETYPE_PUSH )
	{
		trace = SV_Move (v1, vec3_origin, vec3_origin, v2 , 1 , self);
		if (trace.fraction == 1 )
			return true;
		if (trace.ent == targ  )
			return true;
	}

	trace = SV_Move ( v1, targ->v.mins, targ->v.maxs, v2, 1, self );
	
	if (trace.fraction == 1)
		return true;

	return false;
}

/*
void(entity inflictor, entity attacker, float damage, entity ignore) T_RadiusDamage =
{
	local	float 	points;
	local	entity	head;
	local	vector	org;

	head = findradius(inflictor.origin, damage+40);
	
	while (head)
	{
		if (head != ignore)
		{
			if (head.takedamage)
			{
				org = head.origin + (head.mins + head.maxs)*0.5;
				points = 0.5*vlen (inflictor.origin - org);
				if (points < 0)
					points = 0;
				points = damage - points;
				if (head == attacker)
					points = points * 0.5;
				if (points > 0)
				{
					if (CanDamage (head, inflictor))
					{	// shambler takes half damage from all explosions
						if (head.classname == "monster_shambler")						
							T_Damage (head, inflictor, attacker, points*0.5);
						else
							T_Damage (head, inflictor, attacker, points);
					}
				}
			}
		}
		head = head.chain;
	}
};
*/



void PR_T_RadiusDamage (void)
{
	// For TRD

	edict_t	* inflictor	= G_EDICT(OFS_PARM0);
	edict_t	* attacker	= G_EDICT(OFS_PARM1);
	int		  damage	= G_FLOAT(OFS_PARM2);
	edict_t	* ignore	= G_EDICT(OFS_PARM3);
	float   T_Damage	= G_FLOAT(OFS_PARM4);

	int dist;
	int points;

	// For RD

	edict_t	*ent, *chain;
	float	rad;
	vec3_t	org;
	vec3_t	eorg;
	int		i, j;

	chain = (edict_t *)sv.edicts;
	
	VectorCopy (inflictor->v.origin, org);
	rad = damage + 40;

	ent = NEXT_EDICT(sv.edicts);
	for (i=1 ; i<sv.num_edicts ; i++, ent = NEXT_EDICT(ent))
	{
		if (ent->free)
			continue;
		if (ent->v.solid == SOLID_NOT)
			continue;
		for (j=0 ; j<3 ; j++)
			eorg[j] = org[j] - (ent->v.origin[j] + (ent->v.mins[j] + ent->v.maxs[j])*0.5);			
		dist =  Length(eorg);
		if ( dist > rad)
			continue;

		points = damage - (0.5 * dist);
		
		if ( ent == attacker )	
			points *= 0.5;
		
		if (points > 0) {
			if ( PF_CanDamage( ent, inflictor ) ) {
				if (!strcmp( (char *)(pr_strings + ent->v.classname), "monster_shambler" ) )
					points *= 0.5;
				// head inflictor attacker points
				//G_EDICT(OFS_PARM0) = ent;
				//G_EDICT(OFS_PARM1) = inflictor;
				//G_EDICT(OFS_PARM2) = attacker;
				//G_FLOAT(OFS_PARM3) = points;
				Con_Printf("T_Damage %f\n", T_Damage);

				pr_globals[OFS_PARM0] = EDICT_TO_PROG(ent);
				pr_globals[OFS_PARM1] = EDICT_TO_PROG(inflictor);
				pr_globals[OFS_PARM2] = EDICT_TO_PROG(attacker);
				pr_globals[OFS_PARM3] = points;

				PR_ExecuteProgram (T_Damage);					
			}
		}
		ent->v.chain = EDICT_TO_PROG(chain);
		chain = ent;
	}
}

/*
 * Unoptimized but handy function that read a chain, then put values in 
 * pr_tempreg1 arrays.
 *
 */



//Tei ini support

void PF_ReadRegister (void)
{

	FILE*				f;
	char *errormsg = "error";
	

	qboolean sectionok	= false;

	char *p				= G_STRING	(OFS_PARM0);
	char *section		= G_STRING	(OFS_PARM1); 
	char *key			= G_STRING	(OFS_PARM2); 


	COM_FOpenFile(p, &f);

	if (!f || feof(f))
	{
		G_INT(OFS_RETURN) =	errormsg - pr_strings;
		return;
	}

	do
	{
		fscanf(f,"%s",pr_string_temp);
		//Con_Printf("%s det %s busca\n",pr_string_temp, key);

		if ( !_stricmp(pr_string_temp,section) && !feof(f))
		{
			sectionok = true;
		}	
		else
		if ( !_stricmp(pr_string_temp,key) && !feof(f) && sectionok)
		{
			fscanf(f,"%s",pr_string_temp);
			//Con_Printf("%s final\n",pr_string_temp);
			break;
		}	

		//Mark for end of section
		if ( !_stricmp(pr_string_temp,"[EndSection]") )
		{
			if( sectionok ) 
			{
				G_INT(OFS_RETURN) = errormsg - pr_strings;
				return;
			}
		}
		//else 
		//	Con_Printf("%s != %s, %d,%d \n",pr_string_temp, key, strlen(pr_string_temp), strlen(key));

	}
	while( !feof(f) );

	//Con_Printf("Endhere..\n");
	G_INT(OFS_RETURN) = pr_string_temp - pr_strings;
	
	fclose(f);//????????????????
}
//Tei ini support

#if 0

void PF_LoadLexer (void) {};

#else

//TELEJANO
#define		MAX_LEXER	16536*32

char		lexer[MAX_LEXER];
long		sizeoflexer;

void PF_LoadLexer (void)
{
	FILE*				f;
	char *p				= G_STRING	(OFS_PARM0);

	COM_FOpenFile(p, &f);

	if (!f || feof(f))
	{
		G_INT(OFS_RETURN) = 0.0f; //Error!
		return;
	}
	
	sizeoflexer = fread( (void*) lexer,1,MAX_LEXER, f);

	if (lexer)
		Con_Printf("%d : %s\n",sizeoflexer, lexer);

	fclose(f);
};


#endif

void PF_IndexFromLexer (void)
{
	int		offset;
	int			  i = 0;
	qboolean  nofind = true;
	char	*findtxt	= G_STRING	(OFS_PARM0);
	int		lenfind		= strlen( findtxt );

	G_INT(OFS_RETURN) = 0;//No found

	Con_Printf("%s, %d \n", findtxt, lenfind);

	offset = &lexer[0] - pr_strings;

	while(i < MAX_LEXER ) {

		if (!Q_strncmp( findtxt, lexer+i, lenfind ) ){
				Con_Printf("Lo he encontrado?.. %d - %d\n", i, offset);
				G_INT(OFS_RETURN) = offset + i ;//Found!
				break;
			}
		i++;
	}
};







//Tei particleevent
void PR_RunParticleEvent(void) 
{
/*
void (vector org, vector end, float type, float typelife, float dead, float fxscale, float key ) StaticFX =
{
	WriteByte (MSG_BROADCAST, SVC_TEMPENTITY);	
	WriteByte ( MSG_BROADCAST,  49 ); 			
	WriteByte ( MSG_BROADCAST,  1 ); 	// Static TempEntity protocol 1.0
	WriteVec  ( org );				
	WriteVec  ( end );	
	WriteByte ( MSG_BROADCAST,  type ); 			
	WriteByte ( MSG_BROADCAST,  typelife ); 			
	WriteByte ( MSG_BROADCAST,  dead ); 			
	WriteByte ( MSG_BROADCAST, fxscale * 100 ); 			
	WriteByte ( MSG_BROADCAST,  key ); 			
};
  */

	float   *org	= G_VECTOR(OFS_PARM0);
	float   *end	= G_VECTOR(OFS_PARM1);
	float	type	= G_FLOAT(OFS_PARM3);	
	float	tlife	= G_FLOAT(OFS_PARM4);	
	float	dead	= G_FLOAT(OFS_PARM5);	
	float scale		= G_FLOAT(OFS_PARM6);	
	float	key		= G_FLOAT(OFS_PARM7);	

	if (sv.datagram.cursize > MAX_DATAGRAM-16)
		return;	

	MSG_WriteByte  (&sv.datagram, svc_temp_entity	);
	MSG_WriteByte  (&sv.datagram, 49);
	MSG_WriteByte  (&sv.datagram, 1);		//Static protocol v1.0

	MSG_WriteCoord	(&sv.datagram, org[0]);
	MSG_WriteCoord	(&sv.datagram, org[1]);
	MSG_WriteCoord	(&sv.datagram, org[2]);

	MSG_WriteCoord	(&sv.datagram, end[0]);
	MSG_WriteCoord	(&sv.datagram, end[1]);
	MSG_WriteCoord	(&sv.datagram, end[2]);
	
	MSG_WriteByte  (&sv.datagram, type);
	MSG_WriteByte  (&sv.datagram, tlife);
	MSG_WriteByte  (&sv.datagram, dead);
	MSG_WriteByte  (&sv.datagram, scale);
	MSG_WriteByte  (&sv.datagram, key);
};
//Tei particle event

void MSG_WriteCoord3 (vec3_t org)
{
	MSG_WriteCoord	(&sv.datagram, org[0]);
	MSG_WriteCoord	(&sv.datagram, org[1]);
	MSG_WriteCoord	(&sv.datagram, org[2]);
}

void PR_TempEntity( void)
{

	float	type	= G_FLOAT(OFS_PARM0);	
	float   *org	= G_VECTOR(OFS_PARM1);
	float   *end;//	= G_VECTOR(OFS_PARM2);
	float   *vec;//	= G_VECTOR(OFS_PARM2);
	float   param1;//	= G_VECTOR(OFS_PARM2);
	float   param2;//	= G_VECTOR(OFS_PARM2);
	char *	string;

	if (sv.datagram.cursize > MAX_DATAGRAM-16)
		return;	

	MSG_WriteByte  (&sv.datagram, svc_temp_entity	);
	MSG_WriteByte  (&sv.datagram, type);

	MSG_WriteCoord3(org);
	
	switch( (int)type )
	{
	case TE_RAIN:
	case TE_SNOW:
		vec		= G_VECTOR(OFS_PARM2);
		param1	= G_FLOAT(OFS_PARM3);
		MSG_WriteCoord3 (vec);
		MSG_WriteShort	(&sv.datagram, param1);
		break;

	case TE_LIGHTNING2 :
	case TE_LIGHTNING3 :
	case TE_LIGHTNING4:
	case TE_LIGHTNING1 :
	case TE_BEAM:
		vec		= G_VECTOR(OFS_PARM2);
		MSG_WriteCoord3 (vec);
		break;		
	case TE_LIGHTNINGX :
		vec		= G_VECTOR(OFS_PARM2);
		param1 = G_FLOAT(OFS_PARM3);
		string = G_STRING(OFS_PARM4);
		MSG_WriteCoord3 (vec);//end
		MSG_WriteByte  (&sv.datagram, param1);//style
		MSG_WriteString(&sv.datagram, string);//name
		break;		
	case TE_EXPLOSION2:
		/*
		colorStart = MSG_ReadByte ();
		colorLength = MSG_ReadByte ();*/
		param1 = G_FLOAT(OFS_PARM2);
		param2 = G_FLOAT(OFS_PARM3);
		MSG_WriteByte  (&sv.datagram, param1);
		MSG_WriteByte  (&sv.datagram, param2);
		break;
	case TE_TRAILS:
	case TE_RAILTRAIL: 
		/*...
		pos2[0] = MSG_ReadCoord();
		pos2[1] = MSG_ReadCoord();
		pos2[2] = MSG_ReadCoord();
		angle[0] = MSG_ReadCoord();
		angle[1] = MSG_ReadCoord();
		angle[2] = MSG_ReadCoord();*/

		end = G_VECTOR(OFS_PARM2);
		vec = G_VECTOR(OFS_PARM3);
		MSG_WriteCoord3(end);
		MSG_WriteCoord3(vec);
		break;
	case TE_PARTICLEBLAST:
		/*
		pos2[0] = MSG_ReadCoord();
		pos2[1] = MSG_ReadCoord();
		pos2[2] = MSG_ReadCoord();

		angle[0] = MSG_ReadCoord();
		angle[1] = MSG_ReadCoord();
		angle[2] = MSG_ReadCoord();

		num			= MSG_ReadByte ();

		scale		= MSG_ReadByte ();*/
		end = G_VECTOR(OFS_PARM2);
		vec = G_VECTOR(OFS_PARM3);
		param1 = G_FLOAT(OFS_PARM4);
		param2 = G_FLOAT(OFS_PARM5);

		MSG_WriteCoord3(end);
		MSG_WriteCoord3(vec);
		MSG_WriteByte  (&sv.datagram, param1);
		MSG_WriteByte  (&sv.datagram, param2);
		break;
	case TE_IMPLOSIONFX:
	case TE_STATICFOG:		
	case TE_ALIENEXPLOSION:	
	case TE_EXPLOSIONSMALL3:
	case TE_EXPLOSIONSMALL2:
	case TE_EXPLOSIONSMALL:
	case TE_TELEPORT:	
	case TE_LAVASPLASH:			
	case TE_TAREXPLOSION:
	case TE_EXPLOSION:
	case TE_GUNSHOT:
	case TE_SUPERSPIKE:	
	case TE_PLASMA:
	case TE_SPIKE:
	case TE_KNIGHTSPIKE:
	case TE_WIZSPIKE:
		//Implemented.
		break;

	default:
		//Do nothing
		Con_Printf("Unimplemented Builtin handling\n");
		break;
	}
}



builtin_t pr_builtin[] =
{
PF_Fixme,
PF_makevectors,		// #1	void(entity e)	makevectors
PF_setorigin,		// #2	void(entity e, vector o) setorigin
PF_setmodel,		// #3	void(entity e, string m) setmodel
PF_setsize,			// #4	void(entity e, vector min, vector max) setsize
PF_Fixme,			// #5
PF_break,			// #6	void() break
PF_random,			// #7	float() random
PF_sound,			// #8	void(entity e, float chan, string samp) sound
PF_normalize,		// #9	vector(vector v) normalize

PF_error,			// #10	void(string e) error
PF_objerror,		// #11	void(string e) objerror
PF_vlen,			// #12	float(vector v) vlen
PF_vectoyaw,		// #13	float(vector v) vectoyaw
PF_Spawn,			// #14	entity() spawn
PF_Remove,			// #15	void(entity e) remove
PF_traceline,		// #16	float(vector v1, vector v2, float tryents) traceline
PF_checkclient,		// #17	entity() clientlist
PF_Find,			// #18	entity(entity start, .string fld, string match) find
PF_precache_sound,	// #19	void(string s) precache_sound

PF_precache_model,	// #20	void(string s) precache_model
PF_stuffcmd,		// #21	void(entity client, string s)stuffcmd
PF_findradius,		// #22	entity(vector org, float rad) findradius
PF_bprint,			// #23	void(string s) bprint
PF_sprint,			// #24	void(entity client, string s) sprint
PF_dprint,			// #25	void(string s) dprint
PF_ftos,			// #26	void(string s) ftos
PF_vtos,			// #27	void(string s) vtos
PF_coredump,		// #28
PF_traceon,			// #29

PF_traceoff,		// #30
PF_eprint,			// #31	void(entity e) debug print an entire entity
PF_walkmove,		// #32	float(float yaw, float dist) walkmove
PF_Fixme,			// #33	float(float yaw, float dist) walkmove
PF_droptofloor,		// #34
PF_lightstyle,		// #35
PF_rint,			// #36
PF_floor,			// #37
PF_ceil,			// #38
PF_Fixme,			// #39

PF_checkbottom,		// #40
PF_pointcontents,	// #41
PF_Fixme,			// #42
PF_fabs,			// #43
PF_aim,				// #44
PF_cvar,			// #45
PF_localcmd,		// #46
PF_nextent,			// #47
PF_particle,		// #48
PF_changeyaw,		// #49

PF_Fixme,			// #50
PF_vectoangles,		// #51
PF_WriteByte,		// #52
PF_WriteChar,		// #53
PF_WriteShort,		// #54
PF_WriteLong,		// #55
PF_WriteCoord,		// #56
PF_WriteAngle,		// #57
PF_WriteString,		// #58
PF_WriteEntity,		// #59

PF_Fixme,			// #60
PF_Fixme,			// #61
PF_Fixme,			// #62
PF_Fixme,			// #63
PF_Fixme,			// #64
PF_Fixme,			// #65
PF_Fixme,			// #66
SV_MoveToGoal,		// #67
PF_precache_file,	// #68
PF_makestatic,		// #69

PF_changelevel,		// #70
PF_Fixme,			// #71
PF_cvar_set,		// #72
PF_centerprint,		// #73
PF_ambientsound,	// #74
PF_precache_model,	// #75
PF_precache_sound,	// #76
PF_precache_file,	// #77
PF_setspawnparms,	// #78

// Tomaz - QuakeC String Manipulation Begin
PF_zone,			// #79

PF_unzone,			// #80
PF_strlen,			// #81
PF_strcat,			// #82
PF_substring,		// #83
PF_stof,			// #84
PF_stov,			// #85
// Tomaz - QuakeC String Manipulation End

// Tomaz - QuakeC File System Begin
PF_open,			// #86
PF_close,			// #87
PF_read,			// #88
PF_write,			// #89
// Tomaz - QuakeC File System End

// Tei - QC extras
PF_rvector,			// #90 random vector
PF_statcount,		// #91 stats counter
PF_keytest,			// #92 key parsing
PF_sscanf,			// #93 scan string
PF_hvstring,		// #94  hello -> h\ne\nl\nl\no
PF_SetArray,		// #95
PF_stuffcmd2,		// #96 tei good stuff

PF_cvarstring,			// #97 cvarstring 

// Tei - QC extras

PF_Fixme,			// #98
PF_checkextension,	// #99
PF_VisibleEnt,			// #100
PF_Visible,			// #101
PF_AngleMod,			// #102
PR_T_RadiusDamage,			// #103
PR_ShowPic,			// #104
PR_HidePic,			// #105
PR_MovePic,			// #106
PR_ChangePic,			// #107
PR_ShowPicEnt,			// #108
PR_HidePicEnt,			// #109
PR_MovePicEnt,			// #110
PR_ChangePicEnt,			// #111
PF_ReadRegister,			// #112
PR_RunParticleEvent,			// #113
PF_LoadLexer,			// #114
PF_IndexFromLexer,			// #115
PR_TempEntity,			// #116
PF_Fixme,			// #117
PF_Fixme,			// #118
PF_Fixme,			// #119

};

builtin_t *pr_builtins = pr_builtin;
int pr_numbuiltins = sizeof(pr_builtin)/sizeof(pr_builtin[0]);

