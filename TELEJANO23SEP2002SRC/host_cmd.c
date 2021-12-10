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

extern cvar_t	pausable;

int	current_skill;

void Mod_Print (void);

/*
==================
Host_Quit_f
==================
*/

extern void M_Menu_Quit_f (void);

void Host_Quit_f (void)
{
	if (key_dest != key_console && cls.state != ca_dedicated)
	{
		M_Menu_Quit_f ();
		return;
	}
	CL_Disconnect ();
	Host_ShutdownServer(false);		

	Sys_Quit ();
}


/*
==================
Host_Status_f
==================
*/
void Host_Status_f (void)
{
	client_t	*client;
	int			seconds;
	int			minutes;
	int			hours = 0;
	int			j;
	void		(*print) (char *fmt, ...);
	
	if (cmd_source == src_command)
	{
		if (!sv.active)
		{
			Cmd_ForwardToServer ();
			return;
		}
		print = Con_Printf;
	}
	else
		print = SV_ClientPrintf;

	print ("host:    %s\n", Cvar_VariableString ("hostname"));
	print ("version: %4.2f\n", VERSION);
	if (tcpipAvailable)
		print ("tcp/ip:  %s\n", my_tcpip_address);
	if (ipxAvailable)
		print ("ipx:     %s\n", my_ipx_address);
	print ("map:     %s\n", sv.name);
	print ("players: %i active (%i max)\n\n", net_activeconnections, svs.maxclients);
	for (j=0, client = svs.clients ; j<svs.maxclients ; j++, client++)
	{
		if (!client->active)
			continue;
		seconds = (int)(net_time - client->netconnection->connecttime);
		minutes = seconds * 0.0166666666;	// Tomaz - Speed
		if (minutes)
		{
			seconds -= (minutes * 60);
			hours = minutes * 0.0166666666;	// Tomaz - Speed
			if (hours)
				minutes -= (hours * 60);
		}
		else
			hours = 0;
		print ("#%-2u %-16.16s  %3i  %2i:%02i:%02i\n", j+1, client->name, (int)client->edict->v.frags, hours, minutes, seconds);
		print ("   %s\n", client->netconnection->address);
	}
}

// Tomaz - QC Exec Begin
/*
==================
Host_QC_ExecExecute QC commands from the console
==================
*/
void Host_QC_Exec (void)
{	
	dfunction_t *f;
	
	if (cmd_source == src_command)	
	{		
		Cmd_ForwardToServer ();		
		return;	
	}
	if (!developer.value)		
		return;	
	f = 0;
	if ((f = (dfunction_t *)ED_FindFunction(Cmd_Argv(1))) != NULL)	
	{		 
		pr_global_struct->self = EDICT_TO_PROG(sv_player);
		PR_ExecuteProgram ((func_t)(f - pr_functions));	
	}	
	else
		Con_Printf("bad function\n");
}

// Tomaz - QC Exec End


// Tei - call QC 

/*
void Host_CallQC (void)
{	
	dfunction_t *f;
	


	if (cmd_source == src_command)	
	{		
		Cmd_ForwardToServer ();		
		return;	
	}

	Cvar_Set("callvar", Cmd_Argv(1));

	f = 0;
	if ((f = ED_FindFunction("Command_Console")) != NULL)	
	{		 
		pr_global_struct->self = EDICT_TO_PROG(sv_player);
		PR_ExecuteProgram ((func_t)(f - pr_functions));	
	}	
	else
		Con_Printf("Unsupported by mod\n");
}
*/

// Tei - call QC 

/*
==================
Host_God_f

Sets client to godmode
==================
*/
void Host_God_f (void)
{
	if (cmd_source == src_command)
	{
		Cmd_ForwardToServer ();
		return;
	}

	if (pr_global_struct->deathmatch)
		return;

	sv_player->v.flags = (int)sv_player->v.flags ^ FL_GODMODE;
	if (!((int)sv_player->v.flags & FL_GODMODE) )
		SV_ClientPrintf ("godmode OFF\n");
	else
		SV_ClientPrintf ("godmode ON\n");
}

void Host_Notarget_f (void)
{
	if (cmd_source == src_command)
	{
		Cmd_ForwardToServer ();
		return;
	}

	if (pr_global_struct->deathmatch)
		return;

	sv_player->v.flags = (int)sv_player->v.flags ^ FL_NOTARGET;
	if (!((int)sv_player->v.flags & FL_NOTARGET) )
		SV_ClientPrintf ("notarget OFF\n");
	else
		SV_ClientPrintf ("notarget ON\n");
}

qboolean noclip_anglehack;

void Host_Noclip_f (void)
{
	if (cmd_source == src_command)
	{
		Cmd_ForwardToServer ();
		return;
	}

	if (pr_global_struct->deathmatch)
		return;

	if (sv_player->v.movetype != MOVETYPE_NOCLIP)
	{
		noclip_anglehack = true;
		sv_player->v.movetype = MOVETYPE_NOCLIP;
		SV_ClientPrintf ("noclip ON\n");
	}
	else
	{
		noclip_anglehack = false;
		sv_player->v.movetype = MOVETYPE_WALK;
		SV_ClientPrintf ("noclip OFF\n");
	}
}

/*
==================
Host_Fly_f

Sets client to flymode
==================
*/
void Host_Fly_f (void)
{
	if (cmd_source == src_command)
	{
		Cmd_ForwardToServer ();
		return;
	}

	if (pr_global_struct->deathmatch)
		return;

	if (sv_player->v.movetype != MOVETYPE_FLY)
	{
		sv_player->v.movetype = MOVETYPE_FLY;
		SV_ClientPrintf ("flymode ON\n");
	}
	else
	{
		sv_player->v.movetype = MOVETYPE_WALK;
		SV_ClientPrintf ("flymode OFF\n");
	}
}


/*
==================
Host_Ping_f

==================
*/
void Host_Ping_f (void)
{
	int		i, j;
	float	total;
	client_t	*client;
	
	if (cmd_source == src_command)
	{
		Cmd_ForwardToServer ();
		return;
	}

	SV_ClientPrintf ("Client ping times:\n");
	for (i=0, client = svs.clients ; i<svs.maxclients ; i++, client++)
	{
		if (!client->active)
			continue;
		total = 0;
		for (j=0 ; j<NUM_PING_TIMES ; j++)
			total+=client->ping_times[j];
		total /= NUM_PING_TIMES;
		SV_ClientPrintf ("%4i %s\n", (int)(total*1000), client->name);
	}
}

/*
===============================================================================

SERVER TRANSITIONS

===============================================================================
*/

typedef struct
{
	char	name[32];
} extralevel_t;

extern	int				extramaps_count;
extern	char			moddir[255];
extern	extralevel_t	extralevels[128];

void Host_Maps_f (void)
{
	int	i,dx,t;

	for (i=0; i<extramaps_count; i++)
	{
		Con_Printf ("\n %s ", extralevels[i].name);
		//Tei dense maps cmd
		if ((i+1)<extramaps_count) {
			dx = 18 - strlen( extralevels[i].name );
			for (t = 0; t< dx ; t++)
				Con_Printf(" ");
			Con_Printf ("%s", extralevels[++i].name);
			if ((i+1)<extramaps_count) {
				dx = 18 - strlen( extralevels[i].name );
				for (t = 0; t< dx ; t++)
					Con_Printf(" ");
				Con_Printf ("%s", extralevels[++i].name);
			}

		}
		//Tei dense maps cmd
	}
	Con_Printf ("\nFound %i extra maps\n", extramaps_count);
}

/*
======================
Host_Map_f

handle a 
map <servername>
command from the console.  Active clients are kicked off.
======================
*/
void Host_Map_f (void)
{
	int		i;
	char	name[MAX_QPATH];

	if (cmd_source != src_command)
		return;

	cls.demonum = -1;		// stop demo loop in case this fails

	CL_Disconnect ();
	Host_ShutdownServer(false);		

	key_dest = key_game;			// remove console or menu
	SCR_BeginLoadingPlaque ();

	cls.mapstring[0] = 0;
	for (i=0 ; i<Cmd_Argc() ; i++)
	{
		strcat (cls.mapstring, Cmd_Argv(i));
		strcat (cls.mapstring, " ");
	}
	strcat (cls.mapstring, "\n");

	svs.serverflags = 0;			// haven't completed an episode yet
	strcpy (name, Cmd_Argv(1));

	SV_SpawnServer (name);

	if (!sv.active)
		return;
	
	if (cls.state != ca_dedicated)
	{
		strcpy (cls.spawnparms, "");

		for (i=2 ; i<Cmd_Argc() ; i++)
		{
			strcat (cls.spawnparms, Cmd_Argv(i));
			strcat (cls.spawnparms, " ");
		}
		
		Cmd_ExecuteString ("connect local", src_command);
	}	
}

/*
==================
Host_Changelevel_f

Goes to a new map, taking all clients along
==================
*/
void Host_Changelevel_f (void)
{
	char	level[MAX_QPATH];

	if (Cmd_Argc() != 2)
	{
		Con_Printf ("changelevel <levelname> : continue game on a new level\n");
		return;
	}
	if (!sv.active || cls.demoplayback)
	{
		Con_Printf ("Only the server may changelevel\n");
		return;
	}
	SV_SaveSpawnparms ();
	strcpy (level, Cmd_Argv(1));

	SV_SpawnServer (level);
}

/*
==================
Host_Restart_f

Restarts the current server for a dead player
==================
*/
void Host_Restart_f (void)
{
	char	mapname[MAX_QPATH];

	if (cls.demoplayback || !sv.active)
		return;

	if (cmd_source != src_command)
		return;
	strcpy (mapname, sv.name);	// must copy out, because it gets cleared
								// in sv_spawnserver
	SV_SpawnServer (mapname);
}

/*
==================
Host_Reconnect_f

This command causes the client to wait for the signon messages again.
This is sent just before a server changes levels
==================
*/
void Host_Reconnect_f (void)
{
	SCR_BeginLoadingPlaque ();
	cls.signon = 0;		// need new connection messages
}

/*
=====================
Host_Connect_f

User command to connect to server
=====================
*/
void Host_Connect_f (void)
{
	char	name[MAX_QPATH];
	
	cls.demonum = -1;		// stop demo loop in case this fails
	if (cls.demoplayback)
	{
		CL_StopPlayback ();
		CL_Disconnect ();
	}
	strcpy (name, Cmd_Argv(1));
	CL_EstablishConnection (name);
	Host_Reconnect_f ();
}


/*
===============================================================================

LOAD / SAVE GAME

===============================================================================
*/

#define	SAVEGAME_VERSION	5

/*
===============
Host_SavegameComment

Writes a SAVEGAME_COMMENT_LENGTH character comment describing the current 
===============
*/
void Host_SavegameComment (char *text)
{
	int		i;
	char	kills[20];

	for (i=0 ; i<SAVEGAME_COMMENT_LENGTH ; i++)
		text[i] = ' ';
	memcpy (text, cl.levelname, strlen(cl.levelname));
	sprintf (kills,"kills:%3i/%3i", cl.stats[STAT_MONSTERS], cl.stats[STAT_TOTALMONSTERS]);
	memcpy (text+22, kills, strlen(kills));
// convert space to _ to make stdio happy
	for (i=0 ; i<SAVEGAME_COMMENT_LENGTH ; i++)
		if (text[i] == ' ')
			text[i] = '_';
	text[SAVEGAME_COMMENT_LENGTH] = '\0';
}


/*
===============
Host_Savegame_f
===============
*/
void Host_Savegame_f (void)
{
	char	name[256];
	FILE	*f;
	int		i;
	char	comment[SAVEGAME_COMMENT_LENGTH+1];

	if (cmd_source != src_command)
		return;

	if (!sv.active)
	{
		Con_Printf ("Not playing a local game.\n");
		return;
	}

	if (cl.intermission)
	{
		Con_Printf ("Can't save in intermission.\n");
		return;
	}

	if (svs.maxclients != 1)
	{
		Con_Printf ("Can't save multiplayer games.\n");
		return;
	}

	if (Cmd_Argc() != 2)
	{
		Con_Printf ("save <savename> : save a game\n");
		return;
	}

	if (strstr(Cmd_Argv(1), ".."))
	{
		Con_Printf ("Relative pathnames are not allowed.\n");
		return;
	}
		
	for (i=0 ; i<svs.maxclients ; i++)
	{
		if (svs.clients[i].active && (svs.clients[i].edict->v.health <= 0) )
		{
			Con_Printf ("Can't savegame with a dead player\n");
			return;
		}
	}

	sprintf (name, "%s/%s", com_gamedir, Cmd_Argv(1));
	COM_DefaultExtension (name, ".sav");
	
	Con_Printf ("Saving game to %s...\n", name);
	f = fopen (name, "w");
	if (!f)
	{
		Con_Printf ("ERROR: couldn't open.\n");
		return;
	}
	
	fprintf (f, "%i\n", SAVEGAME_VERSION);
	Host_SavegameComment (comment);
	fprintf (f, "%s\n", comment);
	for (i=0 ; i<NUM_SPAWN_PARMS ; i++)
		fprintf (f, "%f\n", svs.clients->spawn_parms[i]);
	fprintf (f, "%d\n", current_skill);
	fprintf (f, "%s\n", sv.name);
	fprintf (f, "%f\n",sv.time);

// write the light styles

	for (i=0 ; i<MAX_LIGHTSTYLES ; i++)
	{
		if (sv.lightstyles[i])
			fprintf (f, "%s\n", sv.lightstyles[i]);
		else
			fprintf (f,"m\n");
	}


	ED_WriteGlobals (f);
	for (i=0 ; i<sv.num_edicts ; i++)
	{
		ED_Write (f, EDICT_NUM(i));
		fflush (f);
	}
	fclose (f);
	Con_Printf ("done.\n");
}


/*
===============
Host_Loadgame_f
===============
*/
void Host_Loadgame_f (void)
{
	char	name[MAX_OSPATH];
	FILE	*f;
	char	mapname[MAX_QPATH];
	float	time, tfloat;
	char	str[32768], *start;
	int		i, r;
	edict_t	*ent;
	int		entnum;
	int		version;
	float			spawn_parms[NUM_SPAWN_PARMS];

	if (cmd_source != src_command)
		return;

	if (Cmd_Argc() != 2)
	{
		Con_Printf ("load <savename> : load a game\n");
		return;
	}

	cls.demonum = -1;		// stop demo loop in case this fails

	sprintf (name, "%s/%s", com_gamedir, Cmd_Argv(1));
	COM_DefaultExtension (name, ".sav");
	
// we can't call SCR_BeginLoadingPlaque, because too much stack space has
// been used.  The menu calls it before stuffing loadgame command
//	SCR_BeginLoadingPlaque ();

	Con_Printf ("Loading game from %s...\n", name);
	f = fopen (name, "r");
	if (!f)
	{
		Con_Printf ("ERROR: couldn't open.\n");
		return;
	}

	fscanf (f, "%i\n", &version);
	if (version != SAVEGAME_VERSION)
	{
		fclose (f);
		Con_Printf ("Savegame is version %i, not %i\n", version, SAVEGAME_VERSION);
		return;
	}
	fscanf (f, "%s\n", str);
	for (i=0 ; i<NUM_SPAWN_PARMS ; i++)
		fscanf (f, "%f\n", &spawn_parms[i]);
// this silliness is so we can load 1.06 save files, which have float skill values
	fscanf (f, "%f\n", &tfloat);
	current_skill = (int)(tfloat + 0.1);
	Cvar_SetValue ("skill", (float)current_skill);

	fscanf (f, "%s\n",mapname);
	fscanf (f, "%f\n",&time);

	CL_Disconnect_f ();

	SV_SpawnServer (mapname);

	if (!sv.active)
	{
		Con_Printf ("Couldn't load map\n");
		return;
	}
	sv.paused = true;		// pause until all clients connect
	sv.loadgame = true;

// load the light styles

	for (i=0 ; i<MAX_LIGHTSTYLES ; i++)
	{
		fscanf (f, "%s\n", str);
		sv.lightstyles[i] = Hunk_Alloc (strlen(str)+1);
		strcpy (sv.lightstyles[i], str);
	}

// load the edicts out of the savegame file
	entnum = -1;		// -1 is the globals
	while (!feof(f))
	{
		for (i=0 ; i<sizeof(str)-1 ; i++)
		{
			r = fgetc (f);
			if (r == EOF || !r)
				break;
			str[i] = r;
			if (r == '}')
			{
				i++;
				break;
			}
		}
		if (i == sizeof(str)-1)
			Sys_Error ("Loadgame buffer overflow");
		str[i] = 0;
		start = str;
		start = COM_Parse(str);
		if (!com_token[0])
			break;		// end of file
		if (strcmp(com_token,"{"))
			Sys_Error ("First token isn't a brace");
			
		if (entnum == -1)
		{	// parse the global vars
			ED_ParseGlobals (start);
		}
		else
		{	// parse an edict

			ent = EDICT_NUM(entnum);
			memset (&ent->v, 0, progs->entityfields * 4);
			ent->free = false;
			ED_ParseEdict (start, ent);
	
		// link it into the bsp tree
			if (!ent->free)
				SV_LinkEdict (ent, false);
		}

		entnum++;
	}
	
	sv.num_edicts = entnum;
	sv.time = time;

	fclose (f);

	for (i=0 ; i<NUM_SPAWN_PARMS ; i++)
		svs.clients->spawn_parms[i] = spawn_parms[i];

	if (cls.state != ca_dedicated)
	{
		CL_EstablishConnection ("local");
		Host_Reconnect_f ();
	}
}

//============================================================================

/*
======================
Host_Name_f
======================
*/
void Host_Name_f (void)
{
	char	*newName;

	Con_DPrintf ("\"name\" is \"%s\"\n", cl_name.string);//Tei debug

	if (Cmd_Argc () == 1)
	{
		Con_Printf ("\"name\" is \"%s\"\n", cl_name.string);
		return;
	}
	if (Cmd_Argc () == 2)
		newName = Cmd_Argv(1);	
	else
		newName = Cmd_Args();
	newName[15] = 0;

	if (cmd_source == src_command)
	{
		if (Q_strcmp(cl_name.string, newName) == 0)
			return;
		Cvar_Set ("_cl_name", newName);
		if (cls.state == ca_connected)
			Cmd_ForwardToServer ();
		return;
	}

	if (host_client)
		Con_DPrintf("host_client->name is '%s'\n",host_client->name);//Tei debug
		
	if (host_client->name[0] && strcmp(host_client->name, "unconnected") )
		if (Q_strcmp(host_client->name, newName) != 0)
			Con_Printf ("%s renamed to %s\n", host_client->name, newName);
	Q_strcpy (host_client->name, newName);
	host_client->edict->v.netname = host_client->name - pr_strings;
	
	Con_DPrintf("Sending new name to server...\n");
// send notification to all clients
	
	MSG_WriteByte (&sv.reliable_datagram, svc_updatename);
	MSG_WriteByte (&sv.reliable_datagram, host_client - svs.clients);
	MSG_WriteString (&sv.reliable_datagram, host_client->name);
}

	
void Host_Version_f (void)
{
	Con_Printf ("Version %4.2f\n", VERSION);
	Con_Printf ("Exe: "__TIME__" "__DATE__"\n");
}

void Host_Say(qboolean teamonly)
{
	client_t *client;
	client_t *save;
	int		j;
	char	*p;
	unsigned char	text[64];
	qboolean	fromServer = false;

	if (cmd_source == src_command)
	{
		if (cls.state == ca_dedicated)
		{
			fromServer = true;
			teamonly = false;
		}
		else
		{
			Cmd_ForwardToServer ();
			return;
		}
	}

	if (Cmd_Argc () < 2)
		return;

	save = host_client;

	p = Cmd_Args();
// remove quotes if present
	if (*p == '"')
	{
		p++;
		p[Q_strlen(p)-1] = 0;
	}

// turn on color set 1
	if (!fromServer)
		sprintf (text, "%c%s: ", 1, save->name);
	else
		sprintf (text, "%c<%s> ", 1, hostname.string);

	j = sizeof(text) - 2 - Q_strlen(text);  // -2 for /n and null terminator
	if (Q_strlen(p) > j)
		p[j] = 0;

	strcat (text, p);
	strcat (text, "\n");

	for (j = 0, client = svs.clients; j < svs.maxclients; j++, client++)
	{
		if (!client || !client->active || !client->spawned)
			continue;
		if (teamplay.value && teamonly && client->edict->v.team != save->edict->v.team)
			continue;
		host_client = client;
		SV_ClientPrintf("%s", text);
	}
	host_client = save;

	Sys_Printf("%s", &text[1]);
}


void Host_Say_f(void)
{
	Host_Say(false);
}


void Host_Say_Team_f(void)
{
	Host_Say(true);
}


void Host_Tell_f(void)
{
	client_t *client;
	client_t *save;
	int		j;
	char	*p;
	char	text[64];

	if (cmd_source == src_command)
	{
		Cmd_ForwardToServer ();
		return;
	}

	if (Cmd_Argc () < 3)
		return;

	Q_strcpy(text, host_client->name);
	Q_strcat(text, ": ");

	p = Cmd_Args();

// remove quotes if present
	if (*p == '"')
	{
		p++;
		p[Q_strlen(p)-1] = 0;
	}

// check length & truncate if necessary
	j = sizeof(text) - 2 - Q_strlen(text);  // -2 for /n and null terminator
	if (Q_strlen(p) > j)
		p[j] = 0;

	strcat (text, p);
	strcat (text, "\n");

	save = host_client;
	for (j = 0, client = svs.clients; j < svs.maxclients; j++, client++)
	{
		if (!client->active || !client->spawned)
			continue;
		if (Q_strcasecmp(client->name, Cmd_Argv(1)))
			continue;
		host_client = client;
		SV_ClientPrintf("%s", text);
		break;
	}
	host_client = save;
}


/*
==================
Host_Color_f
==================
*/
void Host_Color_f(void)
{
	int		top, bottom;
	int		playercolor;
	
	if (Cmd_Argc() == 1)
	{
		Con_Printf ("\"color\" is \"%i %i\"\n", ((int)cl_color.value) >> 4, ((int)cl_color.value) & 0x0f);
		Con_Printf ("color <0-13> [0-13]\n");
		return;
	}

	if (Cmd_Argc() == 2)
		top = bottom = atoi(Cmd_Argv(1));
	else
	{
		top = atoi(Cmd_Argv(1));
		bottom = atoi(Cmd_Argv(2));
	}
	
	top &= 15;
	if (top > 13)
		top = 13;
	bottom &= 15;
	if (bottom > 13)
		bottom = 13;
	
	playercolor = top*16 + bottom;

	if (cmd_source == src_command)
	{
		Cvar_SetValue ("_cl_color", playercolor);
		if (cls.state == ca_connected)
			Cmd_ForwardToServer ();
		return;
	}

	host_client->colors = playercolor;
	host_client->edict->v.team = bottom + 1;

// send notification to all clients
	MSG_WriteByte (&sv.reliable_datagram, svc_updatecolors);
	MSG_WriteByte (&sv.reliable_datagram, host_client - svs.clients);
	MSG_WriteByte (&sv.reliable_datagram, host_client->colors);
}

/*
==================
Host_Kill_f
==================
*/
void Host_Kill_f (void)
{
	if (cmd_source == src_command)
	{
		Cmd_ForwardToServer ();
		return;
	}

	if (sv_player->v.health <= 0)
	{
		SV_ClientPrintf ("Can't suicide -- allready dead!\n");
		return;
	}
	
	pr_global_struct->time = sv.time;
	pr_global_struct->self = EDICT_TO_PROG(sv_player);
	PR_ExecuteProgram (pr_global_struct->ClientKill);
}


/*
==================
Host_Pause_f
==================
*/
void Host_Pause_f (void)
{
	
	if (cmd_source == src_command)
	{
		Cmd_ForwardToServer ();
		return;
	}
	if (!pausable.value)
		SV_ClientPrintf ("Pause not allowed.\n");
	else
	{
		sv.paused ^= 1;

		if (sv.paused)
		{
			SV_BroadcastPrintf ("%s paused the game\n", pr_strings + sv_player->v.netname);
		}
		else
		{
			SV_BroadcastPrintf ("%s unpaused the game\n",pr_strings + sv_player->v.netname);
		}

	// send notification to all clients
		MSG_WriteByte (&sv.reliable_datagram, svc_setpause);
		MSG_WriteByte (&sv.reliable_datagram, sv.paused);
	}
}

//===========================================================================


/*
==================
Host_PreSpawn_f
==================
*/
void Host_PreSpawn_f (void)
{
	if (cmd_source == src_command)
	{
		Con_Printf ("prespawn is not valid from the console\n");
		return;
	}

	if (host_client->spawned)
	{
		Con_Printf ("prespawn not valid -- allready spawned\n");
		return;
	}
	
	SZ_Write (&host_client->message, sv.signon.data, sv.signon.cursize);
	MSG_WriteByte (&host_client->message, svc_signonnum);
	MSG_WriteByte (&host_client->message, 2);
	host_client->sendsignon = true;
}

/*
==================
Host_Spawn_f
==================
*/
void Host_Spawn_f (void)
{
	int		i;
	client_t	*client;
	edict_t	*ent;

	if (cmd_source == src_command)
	{
		Con_Printf ("spawn is not valid from the console\n");
		return;
	}

	if (host_client->spawned)
	{
		Con_Printf ("Spawn not valid -- allready spawned\n");
		return;
	}

// send all current names, colors, and frag counts
	SZ_Clear (&host_client->message);

// run the entrance script
	if (sv.loadgame)
	{	// loaded games are fully inited allready
		// if this is the last client to be connected, unpause
		sv.paused = false;
	}
	else
	{
		// set up the edict
		ent = host_client->edict;

		memset (&ent->v, 0, progs->entityfields * 4);
		ent->v.colormap = NUM_FOR_EDICT(ent);
		ent->v.team = (host_client->colors & 15) + 1;
		ent->v.netname = host_client->name - pr_strings;

		// copy spawn parms out of the client_t

		for (i=0 ; i< NUM_SPAWN_PARMS ; i++)
			(&pr_global_struct->parm1)[i] = host_client->spawn_parms[i];

		// call the spawn function

		pr_global_struct->time = sv.time;
		pr_global_struct->self = EDICT_TO_PROG(sv_player);
		PR_ExecuteProgram (pr_global_struct->ClientConnect);

		if ((Sys_FloatTime() - host_client->netconnection->connecttime) <= sv.time)
			Sys_Printf ("%s entered the game\n", host_client->name);

		PR_ExecuteProgram (pr_global_struct->PutClientInServer);	
	}

// send time of update
	MSG_WriteByte (&host_client->message, svc_time);
	MSG_WriteFloat (&host_client->message, sv.time);

	for (i=0, client = svs.clients ; i<svs.maxclients ; i++, client++)
	{
		MSG_WriteByte (&host_client->message, svc_updatename);
		MSG_WriteByte (&host_client->message, i);
		MSG_WriteString (&host_client->message, client->name);
		MSG_WriteByte (&host_client->message, svc_updatefrags);
		MSG_WriteByte (&host_client->message, i);
		MSG_WriteShort (&host_client->message, client->old_frags);
		MSG_WriteByte (&host_client->message, svc_updatecolors);
		MSG_WriteByte (&host_client->message, i);
		MSG_WriteByte (&host_client->message, client->colors);
	}
	
// send all current light styles
	for (i=0 ; i<MAX_LIGHTSTYLES ; i++)
	{
		MSG_WriteByte (&host_client->message, svc_lightstyle);
		MSG_WriteByte (&host_client->message, (char)i);
		MSG_WriteString (&host_client->message, sv.lightstyles[i]);
	}

//
// send some stats
//
	MSG_WriteByte (&host_client->message, svc_updatestat);
	MSG_WriteByte (&host_client->message, STAT_TOTALSECRETS);
	MSG_WriteLong (&host_client->message, pr_global_struct->total_secrets);

	MSG_WriteByte (&host_client->message, svc_updatestat);
	MSG_WriteByte (&host_client->message, STAT_TOTALMONSTERS);
	MSG_WriteLong (&host_client->message, pr_global_struct->total_monsters);

	MSG_WriteByte (&host_client->message, svc_updatestat);
	MSG_WriteByte (&host_client->message, STAT_SECRETS);
	MSG_WriteLong (&host_client->message, pr_global_struct->found_secrets);

	MSG_WriteByte (&host_client->message, svc_updatestat);
	MSG_WriteByte (&host_client->message, STAT_MONSTERS);
	MSG_WriteLong (&host_client->message, pr_global_struct->killed_monsters);

	
//
// send a fixangle
// Never send a roll angle, because savegames can catch the server
// in a state where it is expecting the client to correct the angle
// and it won't happen if the game was just loaded, so you wind up
// with a permanent head tilt
	ent = EDICT_NUM( 1 + (host_client - svs.clients) );
	MSG_WriteByte (&host_client->message, svc_setangle);
	for (i=0 ; i < 2 ; i++)
		MSG_WriteAngle (&host_client->message, ent->v.angles[i] );
	MSG_WriteAngle (&host_client->message, 0 );

	SV_WriteClientdataToMessage (sv_player, &host_client->message);

	MSG_WriteByte (&host_client->message, svc_signonnum);
	MSG_WriteByte (&host_client->message, 3);
	host_client->sendsignon = true;
	

	//Tei comment this 
	//MSG_WriteByte (&host_client->message, svc_stufftext);
	//MSG_WriteString (&host_client->message, "mp3 restart\n");
	//MSG_WriteByte (&host_client->message, svc_stufftext);
	//MSG_WriteString (&host_client->message, "s_restart\n");
	//Tei comment this

	MSG_WriteByte (&host_client->message, svc_stufftext);
	MSG_WriteString (&host_client->message,"echo Running Telejano Server\n");

}

/*
==================
Host_Begin_f
==================
*/
void Host_Begin_f (void)
{
	if (cmd_source == src_command)
	{
		Con_Printf ("begin is not valid from the console\n");
		return;
	}

	Con_DPrintf("Begin a game\n");//Tei debugging
	host_client->spawned = true;
}

//===========================================================================


/*
==================
Host_Kick_f

Kicks a user off of the server
==================
*/
void Host_Kick_f (void)
{
	char		*who;
	char		*message = NULL;
	client_t	*save;
	int			i;
	qboolean	byNumber = false;

	if (cmd_source == src_command)
	{
		if (!sv.active)
		{
			Cmd_ForwardToServer ();
			return;
		}
	}
	else if (pr_global_struct->deathmatch)
		return;

	save = host_client;

	if (Cmd_Argc() > 2 && Q_strcmp(Cmd_Argv(1), "#") == 0)
	{
		i = Q_atof(Cmd_Argv(2)) - 1;
		if (i < 0 || i >= svs.maxclients)
			return;
		if (!svs.clients[i].active)
			return;
		host_client = &svs.clients[i];
		byNumber = true;
	}
	else
	{
		for (i = 0, host_client = svs.clients; i < svs.maxclients; i++, host_client++)
		{
			if (!host_client->active)
				continue;
			if (Q_strcasecmp(host_client->name, Cmd_Argv(1)) == 0)
				break;
		}
	}

	if (i < svs.maxclients)
	{
		if (cmd_source == src_command)
		{
			if (cls.state == ca_dedicated)
				who = "Console";
			else
				who = cl_name.string;
		}
		else
			who = save->name;

		// can't kick yourself!
		if (host_client == save)
			return;

		if (Cmd_Argc() > 2)
		{
			message = COM_Parse(Cmd_Args());
			if (byNumber)
			{
				message++;							// skip the #
				while (*message == ' ')				// skip white space
					message++;
				message += Q_strlen(Cmd_Argv(2));	// skip the number
			}
			while (*message && *message == ' ')
				message++;
		}
		if (message)
			SV_ClientPrintf ("Kicked by %s: %s\n", who, message);
		else
			SV_ClientPrintf ("Kicked by %s\n", who);
		SV_DropClient (false);
	}

	host_client = save;
}

/*
===============================================================================

DEBUGGING TOOLS

===============================================================================
*/

/*
==================
Host_Give_f
==================
*/
void Host_Give_f (void)
{
	char	*t;
	int		v;
	eval_t	*val;

	if (cmd_source == src_command)
	{
		Cmd_ForwardToServer ();
		return;
	}

	if (pr_global_struct->deathmatch)
		return;

	t = Cmd_Argv(1);
	v = atoi (Cmd_Argv(2));
	
	switch (t[0])
	{
   case '0':
   case '1':
   case '2':
   case '3':
   case '4':
   case '5':
   case '6':
   case '7':
   case '8':
   case '9':
      // MED added hipnotic give stuff
      if (hipnotic)
      {
         if (t[0] == '6')
         {
            if (t[1] == 'a')
               sv_player->v.items = (int)sv_player->v.items | HIT_PROXIMITY_GUN;
            else
               sv_player->v.items = (int)sv_player->v.items | IT_GRENADE_LAUNCHER;
         }
         else if (t[0] == '9')
            sv_player->v.items = (int)sv_player->v.items | HIT_LASER_CANNON;
         else if (t[0] == '0')
            sv_player->v.items = (int)sv_player->v.items | HIT_MJOLNIR;
         else if (t[0] >= '2')
            sv_player->v.items = (int)sv_player->v.items | (IT_SHOTGUN << (t[0] - '2'));
      }
      else
      {
         if (t[0] >= '2')
            sv_player->v.items = (int)sv_player->v.items | (IT_SHOTGUN << (t[0] - '2'));
      }
		break;
	
    case 's':
		if (rogue)
		{
	        if (val = GETEDICTFIELDVALUE(sv_player, eval_ammo_shells1));
		        val->_float = v;
		}

        sv_player->v.ammo_shells = v;
        break;		
    case 'n':
		if (rogue)
		{
	        if (val = GETEDICTFIELDVALUE(sv_player, eval_ammo_nails1));
			{
				val->_float = v;
				if (sv_player->v.weapon <= IT_LIGHTNING)
					sv_player->v.ammo_nails = v;
			}
		}
		else
		{
			sv_player->v.ammo_nails = v;
		}
        break;		
    case 'l':
		if (rogue)
		{
	        if (val = GETEDICTFIELDVALUE(sv_player, eval_ammo_lava_nails));
			{
				val->_float = v;
				if (sv_player->v.weapon > IT_LIGHTNING)
					sv_player->v.ammo_nails = v;
			}
		}
        break;
    case 'r':
		if (rogue)
		{
	        if (val = GETEDICTFIELDVALUE(sv_player, eval_ammo_rockets1));
			{
				val->_float = v;
				if (sv_player->v.weapon <= IT_LIGHTNING)
					sv_player->v.ammo_rockets = v;
			}
		}
		else
		{
			sv_player->v.ammo_rockets = v;
		}
        break;		
    case 'm':
		if (rogue)
		{
	        if (val = GETEDICTFIELDVALUE(sv_player, eval_ammo_multi_rockets));
			{
				val->_float = v;
				if (sv_player->v.weapon > IT_LIGHTNING)
					sv_player->v.ammo_rockets = v;
			}
		}
        break;		
    case 'h':
        sv_player->v.health = v;
        break;		
    case 'c':
		if (rogue)
		{
	        if (val = GETEDICTFIELDVALUE(sv_player, eval_ammo_cells1));
			{
				val->_float = v;
				if (sv_player->v.weapon <= IT_LIGHTNING)
					sv_player->v.ammo_cells = v;
			}
		}
		else
		{
			sv_player->v.ammo_cells = v;
		}
        break;		
    case 'p':
		if (rogue)
		{
	        if (val = GETEDICTFIELDVALUE(sv_player, eval_ammo_plasma));
			{
				val->_float = v;
				if (sv_player->v.weapon > IT_LIGHTNING)
					sv_player->v.ammo_cells = v;
			}
		}
        break;		
    }
}

edict_t	*FindViewthing (void)
{
	int		i;
	edict_t	*e;
	
	for (i=0 ; i<sv.num_edicts ; i++)
	{
		e = EDICT_NUM(i);
		if ( !strcmp (pr_strings + e->v.classname, "viewthing") )
			return e;
	}
	Con_Printf ("No viewthing on map\n");
	return NULL;
}

/*
==================
Host_Viewmodel_f
==================
*/
void Host_Viewmodel_f (void)
{
	edict_t	*e;
	model_t	*m;

	e = FindViewthing ();
	if (!e)
		return;

	m = Mod_ForName (Cmd_Argv(1), false);
	if (!m)
	{
		Con_Printf ("Can't load %s\n", Cmd_Argv(1));
		return;
	}
	
	e->v.frame = 0;
	cl.model_precache[(int)e->v.modelindex] = m;
}

/*
==================
Host_Viewframe_f
==================
*/
void Host_Viewframe_f (void)
{
	edict_t	*e;
	int		f;
	model_t	*m;

	e = FindViewthing ();
	if (!e)
		return;
	m = cl.model_precache[(int)e->v.modelindex];

	f = atoi(Cmd_Argv(1));
	if (f >= m->numframes)
		f = m->numframes-1;

	e->v.frame = f;		
}


void PrintFrameName (model_t *m, int frame)
{
	aliashdr_t 			*hdr;
	maliasframedesc_t	*pframedesc;

	hdr = (aliashdr_t *)Mod_Extradata (m);
	if (!hdr)
		return;
	pframedesc = &hdr->frames[frame];
	
	Con_Printf ("frame %i: %s\n", frame, pframedesc->name);
}

/*
==================
Host_Viewnext_f
==================
*/
void Host_Viewnext_f (void)
{
	edict_t	*e;
	model_t	*m;
	
	e = FindViewthing ();
	if (!e)
		return;
	m = cl.model_precache[(int)e->v.modelindex];

	e->v.frame = e->v.frame + 1;
	if (e->v.frame >= m->numframes)
		e->v.frame = m->numframes - 1;

	PrintFrameName (m, e->v.frame);		
}

/*
==================
Host_Viewprev_f
==================
*/
void Host_Viewprev_f (void)
{
	edict_t	*e;
	model_t	*m;

	e = FindViewthing ();
	if (!e)
		return;

	m = cl.model_precache[(int)e->v.modelindex];

	e->v.frame = e->v.frame - 1;
	if (e->v.frame < 0)
		e->v.frame = 0;

	PrintFrameName (m, e->v.frame);		
}

/*
===============================================================================

DEMO LOOP CONTROL

===============================================================================
*/


/*
==================
Host_Startdemos_f
==================
*/
void Host_Startdemos_f (void)
{
	int		i, c;

	if (cls.state == ca_dedicated)
	{
		if (!sv.active)
			Cbuf_AddText ("map start\n");
		return;
	}

	c = Cmd_Argc() - 1;
	if (c > MAX_DEMOS)
	{
		Con_Printf ("Max %i demos in demoloop\n", MAX_DEMOS);
		c = MAX_DEMOS;
	}

	Con_Printf ("\n%i demo(s) in loop\n", c);

	for (i=1 ; i<c+1 ; i++)
		strncpy (cls.demos[i-1], Cmd_Argv(i), sizeof(cls.demos[0])-1);

	if (!sv.active && cls.demonum != -1 && !cls.demoplayback)
	{
		cls.demonum = 0;
		CL_NextDemo ();
	}
	else
		cls.demonum = -1;
}


/*
==================
Host_Demos_f

Return to looping demos
==================
*/
void Host_Demos_f (void)
{
	if (cls.state == ca_dedicated)
		return;
	if (cls.demonum == -1)
		cls.demonum = 1;
	CL_Disconnect_f ();
	CL_NextDemo ();
}

/*
==================
Host_Stopdemo_f

Return to looping demos
==================
*/
void Host_Stopdemo_f (void)
{
	if (cls.state == ca_dedicated)
		return;
	if (!cls.demoplayback)
		return;
	CL_StopPlayback ();
	CL_Disconnect ();
}

// FH! listing
void FileList (char *filestring)
{
	char	spacer[32];
	WIN32_FIND_DATA filedata;
	HANDLE	handle;
	int		width, name_length, i;

	width = 0;
	handle = FindFirstFile(filestring, &filedata);
	if (handle != INVALID_HANDLE_VALUE)
	{
		do
		{
			Con_Printf("%s", filedata.cFileName);
			name_length = strlen(filedata.cFileName);
			width = width + 1;
			if (width >= 3)
			{
				Con_Printf("\n");
				width = 0;
			}
			else
			{
				name_length = 16 - name_length;
				sprintf(spacer, "");
				for (i=0;i<name_length;i++)
					strcat(spacer, " ");
				Con_Printf("%s", spacer);
			}
		} while (FindNextFile( handle, &filedata ) != 0);
	}
	FindClose(handle);
	Con_Printf("\n");
}
// FH! listing


// FH! savemapcfg

extern cvar_t  sv_gravity;
extern cvar_t  r_wateralpha;
//extern cvar_t  r_waterwave;

// Tei custom sky
extern cvar_t sv_skyvalue;
extern cvar_t sv_skydim;
extern cvar_t sv_skyspeedscale;
extern cvar_t sv_skyflattern;

extern char skyname[256];

// Tei custom sky

// Tei autofx
extern cvar_t r_autolava;
extern cvar_t r_autofogwater;
extern cvar_t r_autobubbleglobal;
extern cvar_t r_autotele;
extern cvar_t r_autosnow;
extern cvar_t r_autorain;
extern cvar_t r_autolightday;
extern cvar_t r_autofluor;
extern cvar_t r_autozing;
extern cvar_t r_autobubbles;
extern cvar_t r_autograss;


extern cvar_t gl_sun;
extern cvar_t gl_sun_x;
extern cvar_t gl_sun_y;
extern cvar_t gl_sun_z;
// Tei autofx

// saves different map settings to a cfg file named for the map
void SaveMapConfig (void)
{
	FILE	*f;
	char	filename[128];

	// only if a map is running
	if (sv.active)
	{
		//f = fopen (va("%s/maps/%s.cfg",com_gamedir, sv.name), "w");
		sprintf(filename, "%s/maps/cfgs/%s.cfg", com_gamedir, sv.name);
		Con_Printf("%s--\n", filename);
		f = fopen (filename, "w");
		if (!f)
		{
			Con_Printf ("\nCouldn't save map settings.\n");
			return;
		}
		Con_Printf ("\nSaved map settings.\n");

		//fprintf (f, "worldscale %s\n",	worldscale.string	);

		//fprintf (f, "gl_skytype %s\n",	gl_skytype.string	);
		//fprintf (f, "gl_skybox %s\n",	gl_skybox.string	);
		//fprintf (f, "gl_skysize %s\n",	gl_skysize.string	);
		//fprintf (f, "r_skyspeed %s\n",	r_skyspeed.string	);
		//fprintf (f, "gl_skyred %s\n",	gl_skyred.string	);
		//fprintf (f, "gl_skygreen %s\n",	gl_skygreen.string	);
		//fprintf (f, "gl_skyblue %s\n",	gl_skyblue.string	);

		fprintf (f, "// Autogenerated\n");

		fprintf (f, "gl_fogenable		%s\n",	gl_fogenable.string	);
		fprintf (f, "gl_fogred			%s\n",		gl_fogred.string	);
		fprintf (f, "gl_foggreen		%s\n",		gl_foggreen.string	);
		fprintf (f, "gl_fogblue			%s\n",		gl_fogblue.string	);
		fprintf (f, "gl_fogstart		%s\n",		gl_fogstart.string	);
		fprintf (f, "gl_fogend			%s\n",		gl_fogend.string	);

		fprintf (f, "r_wateralpha		%s\n",	r_wateralpha.string	);

		fprintf (f, "sv_skyvalue		%s\n",		sv_skyvalue.string);
		fprintf (f, "sv_skydim			%s\n",		sv_skydim.string);
		fprintf (f, "sv_skyspeedscale	%s\n",		sv_skyspeedscale.string);
		fprintf (f, "sv_skyflattern		%s\n",		sv_skyflattern.string);

		fprintf (f, "r_autolava		%s\n",		r_autolava.string);
		fprintf (f, "r_autotele		%s\n",		r_autotele.string);
		fprintf (f, "r_autosnow		%s\n",		r_autosnow.string);
		fprintf (f, "r_autorain		%s\n",		r_autorain.string);
		fprintf (f, "r_autofogwater	%s\n",		r_autofogwater.string);
		fprintf (f, "r_autobubbleglobal	%s\n",		r_autobubbleglobal.string);
		fprintf (f, "r_autolightday	%s\n",		r_autolightday.string);
		fprintf (f, "r_autozing		%s\n",		r_autozing.string);
		fprintf (f, "r_autofluor	%s\n",		r_autofluor.string);
		fprintf (f, "r_autobubbles  %s\n",		r_autobubbles.string);


		fprintf (f, "gl_sun			%s\n",		gl_sun.string);
		fprintf (f, "gl_sun_x		%s\n",		gl_sun_x.string);
		fprintf (f, "gl_sun_y		%s\n",		gl_sun_y.string);
		fprintf (f, "gl_sun_z		%s\n",		gl_sun_z.string);

		if (skyname[0])
			fprintf (f, "loadsky %s\n",				skyname);

		fprintf (f, "exec  %s.user.cfg\n",		sv.name);
	

		fclose (f);
	}
	else      
		Con_Printf("No map loaded.\n");
}

// FH! savemapcfg

//Tei for menus

extern cvar_t	menu_0;
extern cvar_t	menu_1;
extern cvar_t	menu_2;
extern cvar_t	menu_3;
extern cvar_t	menu_4 ;
extern cvar_t	menu_5 ;
extern cvar_t	menu_6 ;
extern cvar_t	menu_7 ;
extern cvar_t	menu_8 ;
extern cvar_t	menu_9 ;
extern cvar_t	menu_select ;
extern cvar_t  menu_mode ;
extern cvar_t  menu_items ;

extern cvar_t  menu_scale ;
extern cvar_t  menu_image ;

extern cvar_t	xmenu_0 ;
extern cvar_t	xmenu_1 ;
extern cvar_t	xmenu_2 ;
extern cvar_t	xmenu_3 ;
extern cvar_t	xmenu_4 ;
extern cvar_t	xmenu_5 ;
extern cvar_t	xmenu_6 ;
extern cvar_t	xmenu_7 ;
extern cvar_t	xmenu_8 ;
extern cvar_t	xmenu_9 ;

void SaveMenuConfig (void)
{
	FILE	*f;
	char	filename[128];

		sprintf(filename, "%s/%s.txt", com_gamedir, sv.name);
		Con_Printf("%s--\n", filename);
		f = fopen (filename, "w");
		if (!f)
		{
			Con_Printf ("\nCouldn't save menu settings.\n");
			return;
		}
		Con_Printf ("\nSaved menu settings.\n");

		fprintf (f, "// Autogenerated\n\n menu_mode 1\n menu_clear\n");

		fprintf (f, "menu_0	\"%s\"\n",	menu_0.string	);
		fprintf (f, "xmenu_0	\"%s\"\n",	xmenu_0.string	);

		fprintf (f, "menu_1	\"%s\"\n",	menu_1.string	);
		fprintf (f, "xmenu_1	\"%s\"\n",	xmenu_1.string	);

		fprintf (f, "menu_2	\"%s\"\n",	menu_2.string	);
		fprintf (f, "xmenu_2	\"%s\"\n",	xmenu_2.string	);

		fprintf (f, "menu_3	\"%s\"\n",	menu_3.string	);
		fprintf (f, "xmenu_3	\"%s\"\n",	xmenu_3.string	);

		fprintf (f, "menu_4	\"%s\"\n",	menu_4.string	);
		fprintf (f, "xmenu_4	\"%s\"\n",	xmenu_4.string	);

		fprintf (f, "menu_5	\"%s\"\n",	menu_5.string	);
		fprintf (f, "xmenu_5	\"%s\"\n",	xmenu_5.string	);

		fprintf (f, "menu_6	\"%s\"\n",	menu_6.string	);
		fprintf (f, "xmenu_6	\"%s\"\n",	xmenu_6.string	);

		fprintf (f, "menu_7	\"%s\"\n",	menu_7.string	);
		fprintf (f, "xmenu_7	\"%s\"\n",	xmenu_7.string	);

		fprintf (f, "menu_8	\"%s\"\n",	menu_8.string	);
		fprintf (f, "xmenu_8	\"%s\"\n",	xmenu_8.string	);

		fprintf (f, "menu_9	\"%s\"\n",	menu_9.string	);
		fprintf (f, "xmenu_9	\"%s\"\n",	xmenu_9.string	);

		fprintf (f, "menu_items	\"%s\"\n",	menu_items.string	);

		fprintf (f, "menu_select	\"%s\"\n",	menu_select.string	);

		fprintf (f, "menu_image	\"%s\"\n",	menu_image.string	);

		fclose (f);
}
//Tei for menus



// FH! modlist
void ModList (void)
{
	char filestring[64];

	Con_Printf("\n\nDirectories \35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\37\n\n");
	sprintf (filestring, "*.");
	FileList(filestring);
}
// FH! modlist

// FH! modlist
void CfgList (void)
{
	char filestring[64];

	Con_Printf("\n\nCfg \35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\37\n\n");
	sprintf (filestring, "*.cfg");
	FileList(filestring);
}
// FH! modlist


// FH! maplist
void MapList (void)
{
	char filestring[64];

	Con_Printf("\n\nMap List \35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\37\n\n");
	sprintf (filestring, "%s/maps/*.bsp", com_gamedir);
	FileList(filestring);
}
// FH! maplist


// Tei! demlist
void DemList (void)
{
	char filestring[64];

	Con_Printf("\n\nDem List \35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\37\n\n");
	sprintf (filestring, "%s/*.dem", com_gamedir);
	FileList(filestring);
}
// Tei! demlist

// Tei! hr
void HorizontalBar_f (void)
{
	Con_Printf("\35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\37\n");
}
// Tei! hr


//TEi cheat
char *cheatername;

int ischeat(char * newname)
{
	return (!strcmp (cheatername, newname));
}

void CheatMe (void)
{
	///char	filename[128];
	//char *cheatername;
	edict_t	*ed;
//	dfunction_t	*func;

	if (pr_global_struct->deathmatch)
		return;


	ed = PROG_TO_EDICT(pr_global_struct->self);
	cheatername = ed->v.netname + pr_strings;

	Con_Printf("Cheater! %s\n", cheatername);

	if ( ischeat("GameSpy") )
		ed->v.health = -300;
	else
	if ( ischeat("LordGraga") )
		ed->v.health = 1;
	//PLease, add you here.
	else
	if ( ischeat("Player") || ischeat("player") )
		ed->v.health = ed->v.health + 1;
	else
	{
		ed->v.health = ed->v.health + 1;
		/*
		ed->v.velocity[2] = 500;
		ed->v.weapon = 1|2|4|8|16|32|64|128|256|1024|2048|4096|(4096*2)|(4096*4)|(4096*8);
		*/
		/*
		//Fun, but dont work.
		func = 0;
		func = ED_FindFunction ("T_RadiusDamage");
		if (func)
		{
			G_FLOAT(OFS_PARM2) = 600;
			pr_global_struct->self = EDICT_TO_PROG(ed);
			PR_ExecuteProgram (func - pr_functions);			
		}
		*/
	}

}
//Tei cheat

void CheatJump (void)
{
	edict_t	*ed;
//	dfunction_t	*func;

	if (pr_global_struct->deathmatch)
		return;


	ed = PROG_TO_EDICT(pr_global_struct->self);
	cheatername = ed->v.netname + pr_strings;

	Con_Printf("Cheater! %s\n", cheatername);
	ed->v.velocity[2] = 500;
}
//Tei cheat


//Tei cheat teleport
extern cvar_t temp1;

cvar_t telecheat_x = {"telecheat_x","0",false};
cvar_t telecheat_y = {"telecheat_y","0",false};
cvar_t telecheat_z = {"telecheat_z","0",false};

void CheatTeleport (void)
{
	float x, y, z;
	edict_t	*ed;
//	dfunction_t	*func;


	if (pr_global_struct->deathmatch)
		return;


	ed = PROG_TO_EDICT(pr_global_struct->self);
	cheatername = ed->v.netname + pr_strings;
	
	x = telecheat_x.value;
	y = telecheat_y.value;
	z = telecheat_y.value;


	if ( (x!=0) || (y!=0) || (z!=0) ) 
	{	
		ed->v.origin[0] = telecheat_x.value;
		ed->v.origin[1] = telecheat_y.value;
		ed->v.origin[2] = telecheat_z.value;
		Con_Printf("[%s] Teleported! %f %f %f\n", cheatername, x,y,z);
	}
	else {
		Con_Printf("[%s] Drop teleport landing!\n", cheatername);
		telecheat_x.value = ed->v.origin[0];
		telecheat_y.value = ed->v.origin[1];
		telecheat_z.value = ed->v.origin[2];
	}

}

void ClearTele (void)
{

	if (pr_global_struct->deathmatch)
		return;


		Cvar_SetValue("telecheat_x",		0		);
		Cvar_SetValue("telecheat_y",		0		);
		Cvar_SetValue("telecheat_z",		0		);
}

//Tei cheat teleport


//=============================================================================

/*
==================
Host_InitCommands
==================
*/

//cvar_t callvar = {"callvar",0 };
extern cvar_t demopause;
void Host_InitCommands (void)
{
	Cmd_AddCommand ("status", Host_Status_f);
	Cmd_AddCommand ("quit", Host_Quit_f);
	Cmd_AddCommand ("god", Host_God_f);
	Cmd_AddCommand ("notarget", Host_Notarget_f);
	Cmd_AddCommand ("fly", Host_Fly_f);
	Cmd_AddCommand ("map", Host_Map_f);
	Cmd_AddCommand ("maps", Host_Maps_f);
	Cmd_AddCommand ("restart", Host_Restart_f);
	Cmd_AddCommand ("changelevel", Host_Changelevel_f);
	Cmd_AddCommand ("connect", Host_Connect_f);
	Cmd_AddCommand ("reconnect", Host_Reconnect_f);
	Cmd_AddCommand ("name", Host_Name_f);
	Cmd_AddCommand ("noclip", Host_Noclip_f);
	Cmd_AddCommand ("version", Host_Version_f);
	Cmd_AddCommand ("say", Host_Say_f);
	Cmd_AddCommand ("say_team", Host_Say_Team_f);
	Cmd_AddCommand ("tell", Host_Tell_f);
	Cmd_AddCommand ("color", Host_Color_f);
	Cmd_AddCommand ("kill", Host_Kill_f);
	Cmd_AddCommand ("pause", Host_Pause_f);
	Cmd_AddCommand ("spawn", Host_Spawn_f);
	Cmd_AddCommand ("begin", Host_Begin_f);
	Cmd_AddCommand ("prespawn", Host_PreSpawn_f);
	Cmd_AddCommand ("kick", Host_Kick_f);
	Cmd_AddCommand ("ping", Host_Ping_f);
	Cmd_AddCommand ("load", Host_Loadgame_f);
	Cmd_AddCommand ("save", Host_Savegame_f);
	Cmd_AddCommand ("give", Host_Give_f);

	Cmd_AddCommand ("startdemos", Host_Startdemos_f);
	Cmd_AddCommand ("demos", Host_Demos_f);
	Cmd_AddCommand ("stopdemo", Host_Stopdemo_f);
    Cvar_RegisterVariable (&demopause);//Tei demopause
	

	Cmd_AddCommand ("viewmodel", Host_Viewmodel_f);
	Cmd_AddCommand ("viewframe", Host_Viewframe_f);
	Cmd_AddCommand ("viewnext", Host_Viewnext_f);
	Cmd_AddCommand ("viewprev", Host_Viewprev_f);

	Cmd_AddCommand ("mcache", Mod_Print);
	Cmd_AddCommand ("qcexec", Host_QC_Exec);	// Tomaz - QC Exec

	// FH! savemapcfg
	Cmd_AddCommand ("savemapcfg", SaveMapConfig);

	
	// FH! savemapcfg

	Cmd_AddCommand ("savemenucfg", SaveMenuConfig);//Tei savemenuconfig
	// FH!
	Cmd_AddCommand ("maplist", MapList);
	Cmd_AddCommand ("modlist", ModList);
	Cmd_AddCommand ("cfglist", CfgList);
	Cmd_AddCommand ("demlist", DemList);

	// FH!

	// Tei call
	 //Cvar_RegisterVariable (&callvar);
	 Cmd_AddCommand ("hr", HorizontalBar_f);
	 Cmd_AddCommand ("cheatme", CheatMe);
	 Cmd_AddCommand ("cheat_jump", CheatJump);
	 Cmd_AddCommand ("telecheat", CheatTeleport);
	 Cmd_AddCommand ("teleclear", ClearTele);


	 Cvar_RegisterVariable (&telecheat_x);
	 Cvar_RegisterVariable (&telecheat_y);
	 Cvar_RegisterVariable (&telecheat_z);
  // Tei call

	
}
