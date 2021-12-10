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
// sv_edict.c -- entity dictionary

#include "quakedef.h"

dprograms_t		*progs;
dfunction_t		*pr_functions;
char			*pr_strings;
ddef_t			*pr_fielddefs;
ddef_t			*pr_globaldefs;
dstatement_t	*pr_statements;
globalvars_t	*pr_global_struct;
float			*pr_globals;			// same as pr_global_struct
int				pr_edict_size;	// in bytes

unsigned short		pr_crc;

int		type_size[8] = {1,sizeof(string_t)/4,1,3,1,1,sizeof(func_t)/4,sizeof(void *)/4};

ddef_t *ED_FieldAtOfs (int ofs);
qboolean	ED_ParseEpair (void *base, ddef_t *key, char *s);

cvar_t	nomonsters = {"nomonsters", "0"};
cvar_t	gamecfg = {"gamecfg", "0"};
cvar_t	scratch1 = {"scratch1", "0"};
cvar_t	scratch2 = {"scratch2", "0"};
cvar_t	scratch3 = {"scratch3", "0"};
cvar_t	scratch4 = {"scratch4", "0"};
cvar_t	savedgamecfg = {"savedgamecfg", "0", true};
cvar_t	saved1 = {"saved1", "0", true};
cvar_t	saved2 = {"saved2", "0", true};
cvar_t	saved3 = {"saved3", "0", true};
cvar_t	saved4 = {"saved4", "0", true};

#define	MAX_FIELD_LEN	64
#define GEFV_CACHESIZE	2

typedef struct {
	ddef_t	*pcache;
	char	field[MAX_FIELD_LEN];
} gefv_cache;

static gefv_cache	gefvCache[GEFV_CACHESIZE] = {{NULL, ""}, {NULL, ""}};

ddef_t *ED_FindField (char *name);
ddef_t *ED_FindGlobal (char *name);

int eval_ammo_shells1;
int eval_ammo_nails1;
int eval_ammo_lava_nails;
int eval_ammo_rockets1;
int eval_ammo_multi_rockets;
int eval_ammo_cells1;
int eval_ammo_plasma;
int eval_nodrawtoclient;
int eval_drawonlytoclient;
int eval_alpha;
int eval_renderamt;
int eval_scale;
int eval_glow_size;
int eval_glow_red;
int eval_glow_green;
int eval_glow_blue;
int eval_items2;
int eval_gravity;
int eval_gravityorg;//Tei NewGravOrigin

int eval_autoanimagic; //Tei autoanimation
int eval_autoanimagic_lowframe; //Tei autoanimation

int eval_trigger;//Tei triggers

#if 0 //TELEJANO
// Tei read register
int eval_parse0;
int eval_parse1;
int eval_parse2;
int eval_parse3;
int eval_parse4;
int eval_parse5;
int eval_parse6;
int eval_parse7;
// Tei read register
#endif


int FindFieldOffset(char *field)
{
	ddef_t *d;
	d = ED_FindField(field);
	if (!d)
		return 0;
	return d->ofs*4;
}

int FindGlobalOffset(char *field)
{
	ddef_t *d;
	d = ED_FindGlobal(field);
	if (!d)
		return 0;
	return d->ofs*4;
}


//ED_GlobalAtOfs

void FindEdictFieldOffsets()
{
	eval_ammo_shells1		= FindFieldOffset("ammo_shells1");
	eval_ammo_nails1		= FindFieldOffset("ammo_nails1");
	eval_ammo_lava_nails	= FindFieldOffset("ammo_lava_nails");
	eval_ammo_rockets1		= FindFieldOffset("ammo_rockets1");
	eval_ammo_multi_rockets = FindFieldOffset("ammo_multi_rockets");
	eval_ammo_cells1		= FindFieldOffset("ammo_cells1");
	eval_ammo_plasma		= FindFieldOffset("ammo_plasma");
	eval_nodrawtoclient		= FindFieldOffset("nodrawtoclient");
	eval_drawonlytoclient	= FindFieldOffset("drawonlytoclient");
	eval_alpha				= FindFieldOffset("alpha");
	eval_renderamt			= FindFieldOffset("renderamt");
	eval_scale				= FindFieldOffset("scale");
	eval_glow_size			= FindFieldOffset("glow_size");
	eval_glow_red			= FindFieldOffset("glow_red");
	eval_glow_green			= FindFieldOffset("glow_green");
	eval_glow_blue			= FindFieldOffset("glow_blue");
	eval_items2				= FindFieldOffset("items2");
	eval_gravity			= FindFieldOffset("gravity");
	eval_gravityorg			= FindFieldOffset("gravityorg");//Tei

	eval_autoanimagic			= FindFieldOffset("autoanimagic");//Tei
	eval_autoanimagic_lowframe			= FindFieldOffset("autoanimagic_lowframe");//Tei

	eval_trigger			= FindFieldOffset("targetname");//Tei triggers
	
#if 0 //TELEJANO
	// Tei regparams parsers
	eval_parse0				= FindGlobalOffset("regparse0");//Tei regparams
	eval_parse1				= FindGlobalOffset("regparse1");//Tei regparams
	eval_parse2				= FindGlobalOffset("regparse2");//Tei regparams
	eval_parse3				= FindGlobalOffset("regparse3");//Tei regparams
	eval_parse4				= FindGlobalOffset("regparse4");//Tei regparams
	eval_parse5				= FindGlobalOffset("regparse5");//Tei regparams
	eval_parse6				= FindGlobalOffset("regparse6");//Tei regparams
	eval_parse7				= FindGlobalOffset("regparse7");//Tei regparams
	// Tei regparams parsers
#endif

};

/*
=================
ED_ClearEdict

Sets everything to NULL
=================
*/
void ED_ClearEdict (edict_t *e)
{
	memset (&e->v, 0, progs->entityfields * 4);
	e->free = false;
}

/*
=================
ED_Alloc

Either finds a free edict, or allocates a new one.
Try to avoid reusing an entity that was recently freed, because it
can cause the client to think the entity morphed into something else
instead of being removed and recreated, which can cause interpolated
angles and bad trails.
=================
*/
edict_t *ED_Alloc (void)
{
	int			i, flags, mov;
	edict_t		*e;

	for ( i=svs.maxclients+1 ; i<sv.num_edicts ; i++)
	{
		e = EDICT_NUM(i);
		// the first couple seconds of server time can involve a lot of
		// freeing and allocating, so relax the replacement policy
		if (e->free && ( e->freetime < 2 || sv.time - e->freetime > 0.5 ) )
		{
			ED_ClearEdict (e);
			return e;
		}
	}
	
	if (i == MAX_EDICTS) 		
		{
			//Tei try to avoid syserror on no_free edicts 
			for ( i=svs.maxclients+1 ; i<sv.num_edicts ; i++)
			{
				e = EDICT_NUM(i);
				// the first couple seconds of server time can involve a lot of
				if (e->free && ( e->freetime < 32 || sv.time - e->freetime > 0.05 ) )	
				{
					ED_ClearEdict (e);
					return e;
				}
			}

			for ( i=svs.maxclients+1 ; i<sv.num_edicts ; i++)
			{
				e = EDICT_NUM(i);
				flags = e->v.effects;
				if ( flags & EF_DECAL  )	// Clear first decal
				{
					ED_ClearEdict (e);
					return e;
				}
			}

			//Tei try to delete fly stuff..
			for ( i=svs.maxclients+1 ; i<sv.num_edicts ; i++)
			{
				e = EDICT_NUM(i);
				flags = e->v.effects;
				mov   = e->v.movetype;
				if ( (!(flags & FL_CLIENT)) &&  ((mov & MOVETYPE_FLY)|| (mov & MOVETYPE_FLYMISSILE)|| (mov & MOVETYPE_BOUNCE)|| (mov & MOVETYPE_BOUNCEMISSILE)  ))	// Clear first fly stuff
				{
					Con_DPrintf("Fly entity removed by engine, MAX_EDICTS error\n");
					ED_ClearEdict (e);
					return e;
				}
			}

			//Tei try to avoid syserror on no_free edicts
			if (i == MAX_EDICTS) 				
				Sys_Error ("ED_Alloc: no free edicts");
		}
		
	sv.num_edicts++;
	e = EDICT_NUM(i);
	ED_ClearEdict (e);

	return e;
}

/*
=================
ED_Free

Marks the edict as free
FIXME: walk all entities and NULL out references to this entity
=================
*/
void ED_Free (edict_t *ed)
{
	SV_UnlinkEdict (ed);		// unlink from world bsp

	ed->free = true;
	ed->v.model = 0;
	ed->v.takedamage = 0;
	ed->v.modelindex = 0;
	ed->v.colormap = 0;
	ed->v.skin = 0;
	ed->v.frame = 0;
	VectorCopy (vec3_origin, ed->v.origin);
	VectorCopy (vec3_origin, ed->v.angles);
	ed->v.nextthink = -1;
	ed->v.solid = 0;
	
	ed->freetime = sv.time;
}

//===========================================================================

/*
============
ED_GlobalAtOfs
============
*/
ddef_t *ED_GlobalAtOfs (int ofs)
{
	ddef_t		*def;
	int			i;
	
	for (i=0 ; i<progs->numglobaldefs ; i++)
	{
		def = &pr_globaldefs[i];
		if (def->ofs == ofs)
			return def;
	}
	return NULL;
}

/*
============
ED_FieldAtOfs
============
*/
ddef_t *ED_FieldAtOfs (int ofs)
{
	ddef_t		*def;
	int			i;
	
	for (i=0 ; i<progs->numfielddefs ; i++)
	{
		def = &pr_fielddefs[i];
		if (def->ofs == ofs)
			return def;
	}
	return NULL;
}

/*
============
ED_FindField
============
*/
ddef_t *ED_FindField (char *name)
{
	ddef_t		*def;
	int			i;
	
	for (i=0 ; i<progs->numfielddefs ; i++)
	{
		def = &pr_fielddefs[i];
		if (!strcmp(pr_strings + def->s_name,name) )
			return def;
	}
	return NULL;
}


/*
============
ED_FindGlobal
============
*/
ddef_t *ED_FindGlobal (char *name)
{
	ddef_t		*def;
	int			i;
	
	for (i=0 ; i<progs->numglobaldefs ; i++)
	{
		def = &pr_globaldefs[i];
		if (!strcmp(pr_strings + def->s_name,name) )
			return def;
	}
	return NULL;
}


/*
============
ED_FindFunction
============
*/
dfunction_t *ED_FindFunction (char *name)
{
	dfunction_t		*func;
	int				i;
	
	for (i=0 ; i<progs->numfunctions ; i++)
	{
		func = &pr_functions[i];
		if (!strcmp(pr_strings + func->s_name,name) )
			return func;
	}
	return NULL;
}

/*
============
PR_ValueString

Returns a string describing *data in a type specific manner
=============
*/
char *PR_ValueString (etype_t type, eval_t *val)
{
	static char	line[256];
	ddef_t		*def;
	dfunction_t	*f;
	
	type &= ~DEF_SAVEGLOBAL;

	switch (type)
	{
	case ev_string:
		sprintf (line, "%s", pr_strings + val->string);
		break;
	case ev_entity:	
		sprintf (line, "entity %i", NUM_FOR_EDICT(PROG_TO_EDICT(val->edict)) );
		break;
	case ev_function:
		f = pr_functions + val->function;
		sprintf (line, "%s()", pr_strings + f->s_name);
		break;
	case ev_field:
		def = ED_FieldAtOfs ( val->_int );
		sprintf (line, ".%s", pr_strings + def->s_name);
		break;
	case ev_void:
		sprintf (line, "void");
		break;
	case ev_float:
		sprintf (line, "%5.1f", val->_float);
		break;
	case ev_vector:
		sprintf (line, "'%5.1f %5.1f %5.1f'", val->vector[0], val->vector[1], val->vector[2]);
		break;
	case ev_pointer:
		sprintf (line, "pointer");
		break;
	default:
		sprintf (line, "bad type %i", type);
		break;
	}
	
	return line;
}

/*
============
PR_UglyValueString

Returns a string describing *data in a type specific manner
Easier to parse than PR_ValueString
=============
*/
char *PR_UglyValueString (etype_t type, eval_t *val)
{
	static char	line[4096];
	int i;
	char *s;

	ddef_t		*def;
	dfunction_t	*f;
	
	type &= ~DEF_SAVEGLOBAL;

	switch (type)
	{
	case ev_string:
		sprintf (line, "%s", pr_strings + val->string);
		break;
	case ev_entity:	
		sprintf (line, "%i", NUM_FOR_EDICT(PROG_TO_EDICT(val->edict)));
		break;
	case ev_function:
		f = pr_functions + val->function;
		sprintf (line, "%s", pr_strings + f->s_name);
		break;
#if 0
	case ev_field:
		def = ED_FieldAtOfs ( val->_int );
		sprintf (line, "%s", pr_strings + def->s_name);
		break;
#else //Tei, lh version
	case ev_field:
		def = ED_FieldAtOfs ( val->_int );
		// LordHavoc: parse the string a bit to turn special characters
		// (like newline, specifically) into escape codes,
		// this fixes saving games from various mods
		//sprintf (line, "%s", pr_strings + def->s_name);
		s = pr_strings + def->s_name;
		for (i = 0;i < 4095 && *s;)
		{
			if (*s == '\n')
			{
				line[i++] = '\\';
				line[i++] = 'n';
			}
			else if (*s == '\r')
			{
				line[i++] = '\\';
				line[i++] = 'r';
			}
			else
				line[i] = *s;
			s++;
		}
		line[i++] = 0;
		break;
#endif
	case ev_void:
		sprintf (line, "void");
		break;
	case ev_float:
		sprintf (line, "%f", val->_float);
		break;
	case ev_vector:
		sprintf (line, "%f %f %f", val->vector[0], val->vector[1], val->vector[2]);
		break;
	default:
		sprintf (line, "bad type %i", type);
		break;
	}
	
	return line;
}

/*
============
PR_GlobalString

Returns a string with a description and the contents of a global,
padded to 20 field width
============
*/
char *PR_GlobalString (int ofs)
{
	char	*s;
	int		i;
	ddef_t	*def;
	void	*val;
	static char	line[128];
	
	val = (void *)&pr_globals[ofs];
	def = ED_GlobalAtOfs(ofs);
	if (!def)
		sprintf (line,"%i(???)", ofs);
	else
	{
		s = PR_ValueString (def->type, val);
		sprintf (line,"%i(%s)%s", ofs, pr_strings + def->s_name, s);
	}
	
	i = strlen(line);
	for ( ; i<20 ; i++)
		strcat (line," ");
	strcat (line," ");
		
	return line;
}

char *PR_GlobalStringNoContents (int ofs)
{
	int		i;
	ddef_t	*def;
	static char	line[128];
	
	def = ED_GlobalAtOfs(ofs);
	if (!def)
		sprintf (line,"%i(???)", ofs);
	else
		sprintf (line,"%i(%s)", ofs, pr_strings + def->s_name);
	
	i = strlen(line);
	for ( ; i<20 ; i++)
		strcat (line," ");
	strcat (line," ");
		
	return line;
}


/*
=============
ED_Print

For debugging
=============
*/
void ED_Print (edict_t *ed)
{
	int		l;
	ddef_t	*d;
	int		*v;
	int		i, j;
	char	*name;
	int		type;

	if (ed->free)
	{
		Con_Printf ("FREE\n");
		return;
	}

	Con_Printf("\nEDICT %i:\n", NUM_FOR_EDICT(ed));
	for (i=1 ; i<progs->numfielddefs ; i++)
	{
		d = &pr_fielddefs[i];
		name = pr_strings + d->s_name;
		if (name[strlen(name)-2] == '_')
			continue;	// skip _x, _y, _z vars
			
		v = (int *)((char *)&ed->v + d->ofs*4);

	// if the value is still all 0, skip the field
		type = d->type & ~DEF_SAVEGLOBAL;
		
		for (j=0 ; j<type_size[type] ; j++)
			if (v[j])
				break;
		if (j == type_size[type])
			continue;
	
		Con_Printf ("%s",name);
		l = strlen (name);
		while (l++ < 15)
			Con_Printf (" ");

		Con_Printf ("%s\n", PR_ValueString(d->type, (eval_t *)v));		
	}
}

/*
=============
ED_Write

For savegames
=============
*/
void ED_Write (FILE *f, edict_t *ed)
{
	ddef_t	*d;
	int		*v;
	int		i, j;
	char	*name;
	int		type;

	fprintf (f, "{\n");

	if (ed->free)
	{
		fprintf (f, "}\n");
		return;
	}
	
	for (i=1 ; i<progs->numfielddefs ; i++)
	{
		d = &pr_fielddefs[i];
		name = pr_strings + d->s_name;
		if (name[strlen(name)-2] == '_')
			continue;	// skip _x, _y, _z vars
			
		v = (int *)((char *)&ed->v + d->ofs*4);

	// if the value is still all 0, skip the field
		type = d->type & ~DEF_SAVEGLOBAL;
		for (j=0 ; j<type_size[type] ; j++)
			if (v[j])
				break;
		if (j == type_size[type])
			continue;
	
		fprintf (f,"\"%s\" ",name);
		fprintf (f,"\"%s\"\n", PR_UglyValueString(d->type, (eval_t *)v));		
	}

	fprintf (f, "}\n");
}

void ED_PrintNum (int ent)
{
	ED_Print (EDICT_NUM(ent));
}

/*
=============
ED_PrintEdicts

For debugging, prints all the entities in the current server
=============
*/
void ED_PrintEdicts (void)
{
	int		i;
	
	Con_Printf ("%i entities\n", sv.num_edicts);
	for (i=0 ; i<sv.num_edicts ; i++)
		ED_PrintNum (i);
}

/*
=============
ED_PrintEdict_f

For debugging, prints a single edicy
=============
*/
void ED_PrintEdict_f (void)
{
	int		i;
	
	i = Q_atoi (Cmd_Argv(1));
	if (i >= sv.num_edicts)
	{
		Con_Printf("Bad edict number\n");
		return;
	}
	ED_PrintNum (i);
}

/*
=============
ED_Count

For debugging
=============
*/
void ED_Count (void)
{
	int		i;
	edict_t	*ent;
	int		active, models, solid, step;

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

	Con_Printf ("num_edicts:%3i\n", sv.num_edicts);
	Con_Printf ("active    :%3i\n", active);
	Con_Printf ("view      :%3i\n", models);
	Con_Printf ("touch     :%3i\n", solid);
	Con_Printf ("step      :%3i\n", step);

}

/*
==============================================================================

					ARCHIVING GLOBALS

FIXME: need to tag constants, doesn't really work
==============================================================================
*/

/*
=============
ED_WriteGlobals
=============
*/
void ED_WriteGlobals (FILE *f)
{
	ddef_t		*def;
	int			i;
	char		*name;
	int			type;

	fprintf (f,"{\n");
	for (i=0 ; i<progs->numglobaldefs ; i++)
	{
		def = &pr_globaldefs[i];
		type = def->type;
		if ( !(def->type & DEF_SAVEGLOBAL) )
			continue;
		type &= ~DEF_SAVEGLOBAL;

		if (type != ev_string
		&& type != ev_float
		&& type != ev_entity)
			continue;

		name = pr_strings + def->s_name;		
		fprintf (f,"\"%s\" ", name);
		fprintf (f,"\"%s\"\n", PR_UglyValueString(type, (eval_t *)&pr_globals[def->ofs]));		
	}
	fprintf (f,"}\n");
}

/*
=============
ED_ParseGlobals
=============
*/
void ED_ParseGlobals (char *data)
{
	char	keyname[64];
	ddef_t	*key;

	while (1)
	{	
	// parse key
		data = COM_Parse (data);
		if (com_token[0] == '}')
			break;
		if (!data)
			Sys_Error ("ED_ParseEntity: EOF without closing brace");

		strcpy (keyname, com_token);

	// parse value	
		data = COM_Parse (data);
		if (!data)
			Sys_Error ("ED_ParseEntity: EOF without closing brace");

		if (com_token[0] == '}')
			Sys_Error ("ED_ParseEntity: closing brace without data");

		key = ED_FindGlobal (keyname);
		if (!key)
		{
			Con_Printf ("'%s' is not a global\n", keyname);
			continue;
		}

		if (!ED_ParseEpair ((void *)pr_globals, key, com_token))
			Host_Error ("ED_ParseGlobals: parse error");
	}
}

//============================================================================


/*
=============
ED_NewString
=============
*/
char *ED_NewString (char *string)
{
	char	*new, *new_p;
	int		i,l;
	
	l = strlen(string) + 1;
	new = Hunk_Alloc (l);
	new_p = new;

	for (i=0 ; i< l ; i++)
	{
		if (string[i] == '\\' && i < l-1)
		{
			i++;
			if (string[i] == 'n')
				*new_p++ = '\n';
			else
				*new_p++ = '\\';
		}
		else
			*new_p++ = string[i];
	}
	
	return new;
}


/*
=============
ED_ParseEval

Can parse either fields or globals
returns false if error
=============
*/
qboolean	ED_ParseEpair (void *base, ddef_t *key, char *s)
{
	int		i;
	char	string[128];
	ddef_t	*def;
	char	*v, *w;
	void	*d;
	dfunction_t	*func;
	
	d = (void *)((int *)base + key->ofs);
	
	switch (key->type & ~DEF_SAVEGLOBAL)
	{
	case ev_string:
		*(string_t *)d = ED_NewString (s) - pr_strings;
		break;
		
	case ev_float:
		*(float *)d = atof (s);
		break;
		
	case ev_vector:
		strcpy (string, s);
		v = string;
		w = string;
		for (i=0 ; i<3 ; i++)
		{
			while (*v && *v != ' ')
				v++;
			*v = 0;
			((float *)d)[i] = atof (w);
			w = v = v+1;
		}
		break;
		
	case ev_entity:
		*(int *)d = EDICT_TO_PROG(EDICT_NUM(atoi (s)));
		break;
		
	case ev_field:
		def = ED_FindField (s);
		if (!def)
		{
			Con_Printf ("Can't find field %s\n", s);
			return false;
		}
		*(int *)d = G_INT(def->ofs);
		break;
	
	case ev_function:
		func = ED_FindFunction (s);
		if (!func)
		{
			Con_Printf ("Can't find function %s\n", s);
			return false;
		}
		*(func_t *)d = func - pr_functions;
		break;
		
	default:
		break;
	}
	return true;
}

/*
====================
ED_ParseEdict

Parses an edict out of the given string, returning the new position
ed should be a properly initialized empty edict.
Used for initial level load and for savegames.
====================
*/
char *ED_ParseEdict (char *data, edict_t *ent)
{
	ddef_t		*key;
	qboolean	anglehack;
	qboolean	init;
	char		keyname[256];
	int			n;
	int classnamepased = false;
	init = false;

	// clear it
	if (ent != sv.edicts)	// hack
		memset (&ent->v, 0, progs->entityfields * 4);

	// go through all the dictionary pairs
	while (1)
	{	
	// parse key
		data = COM_Parse (data);
		if (com_token[0] == '}')
			break;
		if (!data)
			Sys_Error ("ED_ParseEntity: EOF without closing brace");
		
		// anglehack is to allow QuakeEd to write single scalar angles
		// and allow them to be turned into vectors. (FIXME...)
		if (!strcmp(com_token, "angle"))
		{
			strcpy (com_token, "angles");
			anglehack = true;
		}
		else
			anglehack = false;

		// FIXME: change light to _light to get rid of this hack
		if (!strcmp(com_token, "light"))
			strcpy (com_token, "light_lev");	// hack for single light def

		strcpy (keyname, com_token);

		// another hack to fix heynames with trailing spaces
		n = strlen(keyname);
		while (n && keyname[n-1] == ' ')
		{
			keyname[n-1] = 0;
			n--;
		}

		// parse value	
		data = COM_Parse (data);
		if (!data)
			Sys_Error ("ED_ParseEntity: EOF without closing brace");

		if (com_token[0] == '}')
			Sys_Error ("ED_ParseEntity: closing brace without data");

		init = true;	

		// keynames with a leading underscore are used for utility comments,
		// and are immediately discarded by quake
		if (keyname[0] == '_')
			continue;
		
		if (!strcmp(com_token, "classname") )
		{
			if (classnamepased)
			{			
				Con_Printf("Double classname!\n");
				continue;
			}
			classnamepased = true;
		}

		key = ED_FindField (keyname);
		if (!key)
		{
			Con_DPrintf ("'%s' is not a field\n", keyname);
			continue;
		}

		if (anglehack)
		{
			char	temp[32];
			strcpy (temp, com_token);
			sprintf (com_token, "0 %s 0", temp);
		}



		if (!ED_ParseEpair ((void *)&ent->v, key, com_token))
			Host_Error ("ED_ParseEdict: parse error");
	}

	if (!init)
		ent->free = true;

	return data;
}


/*
================
ED_LoadFromFile

The entities are directly placed in the array, rather than allocated with
ED_Alloc, because otherwise an error loading the map would have entity
number references out of order.

Creates a server's entity / program execution context by
parsing textual entity definitions out of an ent file.

Used for both fresh maps and savegame loads.  A fresh map would also need
to call ED_CallSpawnFunctions () to let the objects initialize themselves.
================
*/


//Tei mapmodels helper
void PF_precache_model (void);
void StaticBuild (edict_t *ent, char * model)
{
	int i;
	
	//Con_DPrintf("Sending %s\n",model);

	G_INT(OFS_PARM0) = model - pr_strings;
	PF_precache_model();

	MSG_WriteByte (&sv.signon,svc_spawnstatic);

	MSG_WriteByte (&sv.signon, SV_ModelIndex(model));

	MSG_WriteByte (&sv.signon, ent->v.frame);
	MSG_WriteByte (&sv.signon, ent->v.colormap);
	MSG_WriteByte (&sv.signon, ent->v.skin);
	for (i=0 ; i<3 ; i++)
	{
		MSG_WriteCoord(&sv.signon, ent->v.origin[i]);
		MSG_WriteAngle(&sv.signon, ent->v.angles[i]);
	}
};

void StaticBuild2 (edict_t *ent, char * model, float alfa)
{
	int i;
	eval_t  *val;


	//Con_DPrintf("Sending %s\n",model);

	G_INT(OFS_PARM0) = model - pr_strings;
	PF_precache_model();

	MSG_WriteByte (&sv.signon,svc_spawnstatic2);

	MSG_WriteByte (&sv.signon, SV_ModelIndex(model));

	MSG_WriteByte (&sv.signon, ent->v.frame);
	MSG_WriteByte (&sv.signon, ent->v.colormap);
	MSG_WriteByte (&sv.signon, ent->v.skin);
	for (i=0 ; i<3 ; i++)
	{
		MSG_WriteCoord(&sv.signon, ent->v.origin[i]);
		MSG_WriteAngle(&sv.signon, ent->v.angles[i]);
	}

	if (val = GETEDICTFIELDVALUE(ent, eval_alpha))
	{
		MSG_WriteByte (&sv.signon, val->_float * 255 );	
	}
	else
		MSG_WriteByte (&sv.signon, alfa );	
};
//Tei mapmodels helper


extern cvar_t mod_showlight;
extern cvar_t mod_cityofangels;
extern cvar_t temp1;

float TraceLine (vec3_t start, vec3_t end, vec3_t impact, vec3_t normal);


void ED_LoadFromFile (char *data)
{	
	int i;
	edict_t		*ent;
	int			inhibit;
	dfunction_t	*func;
//	eval_t  *val; //Tei
		
//	float		dx; //tei
	//vec3_t		vtemp, normal;//, stop;//Tei


	ent = NULL;
	inhibit = 0;
	pr_global_struct->time = sv.time;
	
// parse ents
	while (1)
	{
// parse the opening brace	
		data = COM_Parse (data);
		if (!data)
			break;
		if (com_token[0] != '{')
			Sys_Error ("ED_LoadFromFile: found %s when expecting {",com_token);

		if (!ent)
			ent = EDICT_NUM(0);
		else
			ent = ED_Alloc ();
		data = ED_ParseEdict (data, ent);

// remove things from different skill levels or deathmatch
		if (deathmatch.value)
		{
			if (((int)ent->v.spawnflags & SPAWNFLAG_NOT_DEATHMATCH))
			{
				ED_Free (ent);	
				inhibit++;
				continue;
			}
		}
		else if ((current_skill == 0 && ((int)ent->v.spawnflags & SPAWNFLAG_NOT_EASY))
				|| (current_skill == 1 && ((int)ent->v.spawnflags & SPAWNFLAG_NOT_MEDIUM))
				|| (current_skill >= 2 && ((int)ent->v.spawnflags & SPAWNFLAG_NOT_HARD)) )
		{
			ED_Free (ent);	
			inhibit++;
			continue;
		}

//
// immediately call spawn function
//
		if (!ent->v.classname)
		{
			Con_DPrintf ("No classname for:\n");
			ED_Print (ent);
			ED_Free (ent);
			continue;
		}

	// look for the spawn function
		func = ED_FindFunction ( pr_strings + ent->v.classname );

		//Tei staticlight	
		if (mod_showlight.value && !strcmp("light",pr_strings + ent->v.classname))
		{
				//Model codified model
				//Con_Printf("Light here\n");
			/*
				VectorCopy(ent->v.origin, vtemp);		
				vtemp[2] += 33;
				dx = TraceLine (ent->v.origin, vtemp, stop, normal);

				if (dx !=1)
				*/
					StaticBuild(ent, "progs/fx_lux.mdl");
		}
		//Tei staticlight


		//Tei mod city of angels
		if (mod_cityofangels.value && !strcmp("light",pr_strings + ent->v.classname))
		{
				i = mod_cityofangels.value;
				if (i==666)
					i = lhrandom(1,8);
				switch(i)
				{
					case 1:
						func = ED_FindFunction ("monster_wizard");
						break;
					case 2:
						func = ED_FindFunction ("monster_demon1");
						break;
					case 3:
						func = ED_FindFunction ("monster_army");
						break;
					case 4:
						func = ED_FindFunction ("monster_hell_knight");
						break;
					case 5:
						func = ED_FindFunction ("monster_enforcer");
						break;
					case 6:
						func = ED_FindFunction ("monster_shambler");
						break;
					case 7:
						func = ED_FindFunction ("monster_dog");
						break;
					case 8:
						func = ED_FindFunction (temp1.string);
						break;
					default:
						func = ED_FindFunction ("monster_wizard");
						break;
				}
			
				if (func)
				{
					pr_global_struct->self = EDICT_TO_PROG(ent);
					PR_ExecuteProgram (func - pr_functions);
				}
		}
		//Tei mod city of angels


		//Tei lightfluor
		if ( !strcmp("light_fluoro",pr_strings + ent->v.classname))
		{
				//Model codified model
				//Con_Printf("Light here\n");
				StaticBuild(ent, "progs/fx_lux.mdl");

		}
		//Tei lightfluor

		/*
		if ( !strcmp("ambient_comp_hum",pr_strings + ent->v.classname))
		{
				//Model codified model
				//Con_Printf("Light here\n");
				StaticBuild(ent, "progs/fx_fm.mdl");
		}
		*/
	
		

		if (!func)
		{

			//Tei mapmodels builtins support
			if (!strcmp("misc_model",pr_strings + ent->v.classname))
			{
				//Model codified model
				StaticBuild(ent, ent->v.model + pr_strings);
				ED_Free (ent);
				continue;
			}
			else
			if (!strcmp("item_mdl",pr_strings + ent->v.classname))
			{
				//Message codified model
				StaticBuild(ent, ent->v.message + pr_strings);
				ED_Free (ent);
				continue;
			}		
			else
			if (!strcmp("light_environment",pr_strings + ent->v.classname))
			{
				//Message codified model
				StaticBuild(ent, "progs/fx_lux.mdl");
				ED_Free (ent);
				continue;
			}
			else
			if (!strcmp("env_glow",pr_strings + ent->v.classname))
			{
				//Message codified model
				StaticBuild(ent, "progs/fx_glow.mdl");
				ED_Free (ent);
				continue;
			}
			else
			if (!strcmp("func_illusionary",pr_strings + ent->v.classname))
			{
				//Message codified model
				StaticBuild2(ent, pr_strings + ent->v.model, 1);
				ED_Free (ent);
				continue;
			}
			else
			if (!strcmp("func_wall",pr_strings + ent->v.classname))
			{
				//Message codified model
				StaticBuild2(ent, pr_strings + ent->v.model, 1);
				ED_Free (ent);
				continue;
			}
			else
			if (!strcmp("hostage_entity",pr_strings + ent->v.classname))
			{
				//Message codified model				
				if (rand()&1)
					func = ED_FindFunction ("monster_army");			
				else
					func = ED_FindFunction ("monster_enforcer");			
			}
			else
			if (!strcmp("func_water",pr_strings + ent->v.classname))
			{
				//Message codified model
				StaticBuild(ent, pr_strings + ent->v.model);
				ED_Free (ent);
				continue;
			}
			else
			if (!strcmp("env_sound",pr_strings + ent->v.classname))
			{
				//Message codified model
				//StaticBuild(ent, "progs/fx_fmlit.mdl");
				//TODO adjust "ambient" to "roomtype"
				
				func = ED_FindFunction ("light_fluoro ");			
				//ED_Free (ent);
				continue;
			}

			//Tei mapmodels builtins support


			//Con_Printf("out: %s, message %s model %s\n",ent->v.classname + pr_strings, ent->v.message + pr_strings,ent->v.model + pr_strings);
			//Tei renaming entitys
			if (func)
			{
				ent->v.skin = 0;
				pr_global_struct->self = EDICT_TO_PROG(ent);
				PR_ExecuteProgram (func - pr_functions);
				continue;
			}
			//Tei renaming entitys
			else
			if (developer.value) // don't confuse non-developers with errors
			{
				Con_Printf ("No spawn function for:\n");
				ED_Print (ent);
			}
			ED_Free (ent);
			continue;
		}

		pr_global_struct->self = EDICT_TO_PROG(ent);
		PR_ExecuteProgram (func - pr_functions);
	}	

	Con_DPrintf ("%i entities inhibited\n", inhibit);
}

// Tei compatibility engine

#define PROGHEADER101_CRC	5927
cvar_t old_progsdat = { "old_progsdat","0" };

// Tei compatibility engine

/*
===============
PR_LoadProgs
===============
*/
extern cvar_t mod_progs;
void PR_LoadProgs (void)
{
	int		i;

// flush the non-C variable lookup cache
	for (i=0 ; i<GEFV_CACHESIZE ; i++)
		gefvCache[i].field[0] = 0;

	CRC_Init (&pr_crc);

	progs = 0;

#if 1 //Telejano custom progs.dat
	progs = (dprograms_t *)COM_LoadHunkFile (mod_progs.string);

	if (!progs)
	{
		Con_Printf("%s not found, searching progs.dat\n", mod_progs.string);
		progs = (dprograms_t *)COM_LoadHunkFile ("progs.dat");
	}
	//Telejano custom progs.dat
#else
	progs = (dprograms_t *)COM_LoadHunkFile ("progs.dat");
#endif

	if (!progs)
		Sys_Error ("PR_LoadProgs: couldn't load progs.dat");

	Con_DPrintf ("Programs occupy %iK.\n", com_filesize * 0.0009765625);	// Tomaz - Speed

	for (i=0 ; i<com_filesize ; i++)
		CRC_ProcessByte (&pr_crc, ((byte *)progs)[i]);

// byte swap the header
	for (i=0 ; i<sizeof(*progs) * 0.25 ; i++)	// Tomaz - Speed
		((int *)progs)[i] = LittleLong ( ((int *)progs)[i] );		

	// Tei - adaptation engine
	Con_Printf("\n\nprogs.dat \35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\37\n\n");

	if (progs->crc == PROGHEADER101_CRC)
	{
		old_progsdat.value = 1;
		Con_Printf ("TomazQuake or Standard Quake Compatible Mod. CRC is %i\n", progs->crc);
	}
	else if (progs->crc == PROGHEADER_CRC )
	{
		old_progsdat.value = 0;
		Con_Printf ("Enhanced Telejano Mod found. CRC is %i.\n", progs->crc);
	}
	else
		Sys_Error ("progs.dat system vars not supported by engine.");

	Con_Printf("\n\35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\37\n\n");

	// Tei - adaptation engine
	
	/*
	if (progs->version != PROG_VERSION)
		Sys_Error ("progs.dat has wrong version number (%i should be %i)", progs->version, PROG_VERSION);
	if (progs->crc != PROGHEADER_CRC)
		//Sys_Error ("progs.dat system vars have been modified, progdefs.h is out of date");
		// Tei message for humans
		Sys_Error ("progs.dat system no mach!. Its a TeiQ compatible mod?");
		// Tei message for humans
	*/

	pr_functions = (dfunction_t *)((byte *)progs + progs->ofs_functions);
	pr_strings = (char *)progs + progs->ofs_strings;
	pr_globaldefs = (ddef_t *)((byte *)progs + progs->ofs_globaldefs);
	pr_fielddefs = (ddef_t *)((byte *)progs + progs->ofs_fielddefs);
	pr_statements = (dstatement_t *)((byte *)progs + progs->ofs_statements);

	pr_global_struct = (globalvars_t *)((byte *)progs + progs->ofs_globals);
	pr_globals = (float *)pr_global_struct;
	
	pr_edict_size = progs->entityfields * 4 + sizeof (edict_t) - sizeof(entvars_t);
	
// byte swap the lumps
	for (i=0 ; i<progs->numstatements ; i++)
	{
		pr_statements[i].op = LittleShort(pr_statements[i].op);
		pr_statements[i].a = LittleShort(pr_statements[i].a);
		pr_statements[i].b = LittleShort(pr_statements[i].b);
		pr_statements[i].c = LittleShort(pr_statements[i].c);
	}

	for (i=0 ; i<progs->numfunctions; i++)
	{
	pr_functions[i].first_statement = LittleLong (pr_functions[i].first_statement);
	pr_functions[i].parm_start = LittleLong (pr_functions[i].parm_start);
	pr_functions[i].s_name = LittleLong (pr_functions[i].s_name);
	pr_functions[i].s_file = LittleLong (pr_functions[i].s_file);
	pr_functions[i].numparms = LittleLong (pr_functions[i].numparms);
	pr_functions[i].locals = LittleLong (pr_functions[i].locals);
	}	

	for (i=0 ; i<progs->numglobaldefs ; i++)
	{
		pr_globaldefs[i].type = LittleShort (pr_globaldefs[i].type);
		pr_globaldefs[i].ofs = LittleShort (pr_globaldefs[i].ofs);
		pr_globaldefs[i].s_name = LittleLong (pr_globaldefs[i].s_name);
	}

	for (i=0 ; i<progs->numfielddefs ; i++)
	{
		pr_fielddefs[i].type = LittleShort (pr_fielddefs[i].type);
		if (pr_fielddefs[i].type & DEF_SAVEGLOBAL)
			Sys_Error ("PR_LoadProgs: pr_fielddefs[i].type & DEF_SAVEGLOBAL");
		pr_fielddefs[i].ofs = LittleShort (pr_fielddefs[i].ofs);
		pr_fielddefs[i].s_name = LittleLong (pr_fielddefs[i].s_name);
	}

	for (i=0 ; i<progs->numglobals ; i++)
		((int *)pr_globals)[i] = LittleLong (((int *)pr_globals)[i]);
	FindEdictFieldOffsets();
}


/*
===============
PR_Init
===============
*/
void PR_Init (void)
{
	Cmd_AddCommand ("edict", ED_PrintEdict_f);
	Cmd_AddCommand ("edicts", ED_PrintEdicts);
	Cmd_AddCommand ("edictcount", ED_Count);
	Cmd_AddCommand ("profile", PR_Profile_f);
	Cvar_RegisterVariable (&nomonsters);
	Cvar_RegisterVariable (&gamecfg);
	Cvar_RegisterVariable (&scratch1);
	Cvar_RegisterVariable (&scratch2);
	Cvar_RegisterVariable (&scratch3);
	Cvar_RegisterVariable (&scratch4);
	Cvar_RegisterVariable (&savedgamecfg);
	Cvar_RegisterVariable (&saved1);
	Cvar_RegisterVariable (&saved2);
	Cvar_RegisterVariable (&saved3);
	Cvar_RegisterVariable (&saved4);
}

//LordHavoc speed

edict_t *EDICT_NUM_ERROR(int n)
{
	Host_Error ("EDICT_NUM: bad number %i", n);
	return NULL;
}
/*
edict_t *EDICT_NUM(int n)
{
	if (n < 0 || n >= sv.max_edicts)
		Sys_Error ("EDICT_NUM: bad number %i", n);
	return (edict_t *)((byte *)sv.edicts+ (n)*pr_edict_size);
}
*/
//LordHavoc speed


int NUM_FOR_EDICT(edict_t *e)
{
	int		b;
	
	b = (byte *)e - (byte *)sv.edicts;
	b = b / pr_edict_size;
	
	if (b < 0 || b >= sv.num_edicts)
		Sys_Error ("NUM_FOR_EDICT: bad pointer");
	return b;
}
