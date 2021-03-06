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
// cl_parse.c  -- parse a message received from the server

#include "quakedef.h"

char *svc_strings[] =
{
	"svc_bad",
	"svc_nop",
	"svc_disconnect",
	"svc_updatestat",
	"svc_version",		// [long] server version
	"svc_setview",		// [short] entity number
	"svc_sound",		// <see code>
	"svc_time",			// [float] server time
	"svc_print",		// [string] null terminated string
	"svc_stufftext",	// [string] stuffed into client's console buffer
						// the string should be \n terminated
	"svc_setangle",		// [vec3] set the view angle to this absolute value
	
	"svc_serverinfo",	// [long] version
						// [string] signon string
						// [string]..[0]model cache [string]...[0]sounds cache
						// [string]..[0]item cache
	"svc_lightstyle",	// [byte] [string]
	"svc_updatename",	// [byte] [string]
	"svc_updatefrags",	// [byte] [short]
	"svc_clientdata",	// <shortbits + data>
	"svc_stopsound",	// <see code>
	"svc_updatecolors",	// [byte] [byte]
	"svc_particle",		// [vec3] <variable>
	"svc_damage",		// [byte] impact [byte] blood [vec3] from
	
	"svc_spawnstatic",
	"OBSOLETE svc_spawnbinary",
	"svc_spawnbaseline",
	
	"svc_temp_entity",	// <variable>
	"svc_setpause",
	"svc_signonnum",
	"svc_centerprint",
	"svc_killedmonster",
	"svc_foundsecret",
	"svc_spawnstaticsound",
	"svc_intermission",
	"svc_finale",		// [string] music [string] text
	"svc_cdtrack",		// [byte] track [byte] looptrack
	"svc_sellscreen",
	"svc_cutscene"
	"svc_showlmp",	// Tomaz - Show LMP
	"svc_hidelmp",	// Tomaz - Hide LMP
	"svc_skybox",	// Tomaz - Skybox
#if 1
	//TELEJANO
	"svc_changefov",	// Tei - change fov
	"svc_rexec",		// Tei - change fov
	"svc_movelmp",		// Tei - move pic loc
	"svc_updatelmp", 	// Tei - update pic file		
	"svc_spawnstatic",  //Tei ss extended

#endif
};



// Tei client changefov
extern cvar_t scr_fov;
// Tei client changefov

//=============================================================================

/*
===============
CL_EntityNum

This error checks and tracks the total number of entities
===============
*/
entity_t	*CL_EntityNum (int num)
{
	if (num >= cl.num_entities)
	{
		if (num >= MAX_EDICTS)
			Host_Error ("CL_EntityNum: %i is an invalid number",num);
		while (cl.num_entities<=num)
		{
			cl_entities[cl.num_entities].colormap = vid.colormap;
			cl.num_entities++;
		}
	}
		
	return &cl_entities[num];
}



/*
==================
CL_ParseStartSoundPacket
==================
*/
void CL_ParseStartSoundPacket(void)
{
    vec3_t  pos;
    int 	channel, ent;
    int 	sound_num;
    int 	volume;
    int 	field_mask;
    float 	attenuation;  
 	int		i;
	           
    field_mask = MSG_ReadByte(); 

    if (field_mask & SND_VOLUME)
		volume = MSG_ReadByte ();
	else
		volume = DEFAULT_SOUND_PACKET_VOLUME;
	
    if (field_mask & SND_ATTENUATION)
		attenuation = MSG_ReadByte () * 0.015625;	// Tomaz - Speed
	else
		attenuation = DEFAULT_SOUND_PACKET_ATTENUATION;
	
	channel = MSG_ReadShort ();
	sound_num = MSG_ReadByte ();

	ent = channel >> 3;
	channel &= 7;

	if (ent > MAX_EDICTS)
		Host_Error ("CL_ParseStartSoundPacket: ent = %i", ent);
	
	for (i=0 ; i<3 ; i++)
		pos[i] = MSG_ReadCoord ();
 
    S_StartSound (ent, channel, cl.sound_precache[sound_num], pos, volume/255.0, attenuation);
}

/*
==================
CL_KeepaliveMessage

When the client is taking a long time to load stuff, send keepalive messages
so the server doesn't disconnect.
==================
*/
void CL_KeepaliveMessage (void)
{
	float	time;
	static float lastmsg;
	int		ret;
	sizebuf_t	old;
	byte		olddata[8192];
	
	if (sv.active)
		return;		// no need if server is local
	if (cls.demoplayback)
		return;

// read messages from server, should just be nops
	old = net_message;
	memcpy (olddata, net_message.data, net_message.cursize);
	
	do
	{
		ret = CL_GetMessage ();
		switch (ret)
		{
		default:
			Host_Error ("CL_KeepaliveMessage: CL_GetMessage failed");		
		case 0:
			break;	// nothing waiting
		case 1:
			Host_Error ("CL_KeepaliveMessage: received a message");
			break;
		case 2:
			if (MSG_ReadByte() != svc_nop)
				Host_Error ("CL_KeepaliveMessage: datagram wasn't a nop");
			break;
		}
	} while (ret);

	net_message = old;
	memcpy (net_message.data, olddata, net_message.cursize);

// check time
	time = Sys_FloatTime ();
	if (time - lastmsg < 5)
		return;
	lastmsg = time;

// write out a nop
	Con_Printf ("--> client to server keepalive\n");

	MSG_WriteByte (&cls.message, clc_nop);
	NET_SendMessage (cls.netcon, &cls.message);
	SZ_Clear (&cls.message);
}

void R_SetSkyBox (char *sky);
extern char skyname[];// Tomaz - Skybox
extern cvar_t	gl_themefolder;//Tei textures themes


// Tomaz - Qc Parsing & HL Maps Begin
void CL_ParseEntityLump(char *entdata)
{
	char *data;
	char key[128], value[4096], texturefolder[4096];
	char wadname[128];
	int i, j, k;

	skyname[0] = 0;
	data = entdata;
	if (!data)
		return;
	data = COM_Parse(data);

	if (!data)
		return; // valid exit

	if (com_token[0] != '{')
		return; // error

	while (1)
	{
		data = COM_Parse(data);
		if (!data)
			return; // error
		if (com_token[0] == '}')
			return; // since we're just parsing the first ent (worldspawn), exit
		strcpy(key, com_token);
		while (key[strlen(key)-1] == ' ') // remove trailing spaces
			key[strlen(key)-1] = 0;
		data = COM_Parse(data);
		if (!data)
			return; // error
		strcpy(value, com_token);
		if (!strcmp("sky", key))
			R_SetSkyBox(value);
		else if (!strcmp("skyname", key))
			R_SetSkyBox(value);
		else if (!strcmp("wad", key)) // for HalfLife maps
		{
			j = 0;
			for (i = 0;i < 4096;i++)
				if (value[i] != ';' && value[i] != '\\' && value[i] != '/' && value[i] != ':')
					break;
			if (value[i])
			{
				for (;i < 4096;i++)
				{
					// ignore path - the \\ check is for HalfLife... stupid windoze 'programmers'...
					if (value[i] == '\\' || value[i] == '/' || value[i] == ':')
						j = i+1;
					else if (value[i] == ';' || value[i] == 0)
					{
						k = value[i];
						value[i] = 0;
						//Tei texture themes
						sprintf(texturefolder,"%s/",gl_themefolder.string);
						//strcpy(wadname, "textures/");
						strcpy(wadname, texturefolder);
						//Tei texture themes
						strcat(wadname, &value[j]);
						W_LoadTextureWadFile (wadname, false);
						j = i+1;                                                
						if (!k)
							break;
					}
				}
			}
		}
	}
}
// Tomaz - Qc Parsing & HL Maps End

/*
==================
CL_ParseServerInfo
==================
*/
void CL_ParseServerInfo (void)
{
	char	*str;
	int		i;
	int		nummodels, numsounds;
	char	model_precache[MAX_MODELS][MAX_QPATH];
	char	sound_precache[MAX_SOUNDS][MAX_QPATH];
	
	Con_DPrintf ("Serverinfo packet received.\n");
//
// wipe the client_state_t struct
//
	CL_ClearState ();

// parse protocol version number
	i = MSG_ReadLong ();
	if (i != PROTOCOL_VERSION)
	{
		Con_Printf ("Server returned version %i, not %i\n", i, PROTOCOL_VERSION);
		return;
	}

// parse maxclients
	cl.maxclients = MSG_ReadByte ();
	if (cl.maxclients < 1 || cl.maxclients > MAX_SCOREBOARD)
	{
		Con_Printf("Bad maxclients (%u) from server\n", cl.maxclients);
		return;
	}
	cl.scores = Hunk_AllocName (cl.maxclients*sizeof(*cl.scores), "scores");

// parse gametype
	cl.gametype = MSG_ReadByte ();

// parse signon message
	str = MSG_ReadString ();
	strncpy (cl.levelname, str, sizeof(cl.levelname)-1);

// seperate the printfs so the server message can have a color
	Con_Printf("\n\n\35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\37\n\n");
	Con_Printf ("%c%s\n", 2, str);

//
// first we go through and touch all of the precache data that still
// happens to be in the cache, so precaching something else doesn't
// needlessly purge it
//

// precache models
	memset (cl.model_precache, 0, sizeof(cl.model_precache));
	for (nummodels=1 ; ; nummodels++)
	{
		str = MSG_ReadString ();
		if (!str[0])
			break;
		if (nummodels==MAX_MODELS)
		{
			Con_Printf ("Server sent too many model precaches\n");
			return;
		}
		strcpy (model_precache[nummodels], str);
		Mod_TouchModel (str);
	}

// precache sounds
	memset (cl.sound_precache, 0, sizeof(cl.sound_precache));
	for (numsounds=1 ; ; numsounds++)
	{
		str = MSG_ReadString ();
		if (!str[0])
			break;
		if (numsounds==MAX_SOUNDS)
		{
			Con_Printf ("Server sent too many sound precaches\n");
			return;
		}
		strcpy (sound_precache[numsounds], str);
		S_TouchSound (str);
	}

//
// now we try to load everything else until a cache allocation fails
//

	for (i=1 ; i<nummodels ; i++)
	{
		cl.model_precache[i] = Mod_ForName (model_precache[i], false);
		if (cl.model_precache[i] == NULL)
		{
			Con_Printf("Model %s not found\n", model_precache[i]);
			return;
		}
		CL_KeepaliveMessage ();
	}

	S_BeginPrecaching ();
	for (i=1 ; i<numsounds ; i++)
	{
		cl.sound_precache[i] = S_PrecacheSound (sound_precache[i]);
		CL_KeepaliveMessage ();
	}
	S_EndPrecaching ();


// local state
	cl_entities[0].model = cl.worldmodel = cl.model_precache[1];

	R_NewMap ();

	Hunk_Check ();		// make sure nothing is hurt
	
	noclip_anglehack = false;		// noclip is turned off at start	
}


/*
==================
CL_ParseUpdate

Parse an entity update message from the server
If an entities model or origin changes from frame to frame, it must be
relinked.  Other attributes can change without relinking.
==================
*/
int	bitcounts[16];

#ifndef GLQUAKE
#define GLQUAKE 1
#endif

#if 1 //Tei debug
void CL_ParseUpdate (int bits)
{
	int			i;
	model_t		*model;
	int			modnum;
	qboolean	forcelink;
	entity_t	*ent;
	int			num;
	int			skin;

	if (cls.signon == SIGNONS - 1)
	{	// first update is the final signon stage
		cls.signon = SIGNONS;
		CL_SignonReply ();
	}
	
	//Con_DPrintf("CL_ParseUpdate %d\n",bits);//Tei debug

	if (bits & U_MOREBITS)
	{
		i = MSG_ReadByte ();
		bits |= (i<<8);
	}

#ifdef EXTENDQC
	if (bits & U_EXTEND1)
	{
		bits |= MSG_ReadByte() << 16;
	}
#endif

	if (bits & U_LONGENTITY)	
		num = MSG_ReadShort ();
	else
		num = MSG_ReadByte ();

	ent = CL_EntityNum (num);

for (i=0 ; i<16 ; i++)
if (bits&(1<<i))
	bitcounts[i]++;

	if (ent->msgtime != cl.mtime[1])
		forcelink = true;	// no previous frame to lerp from
	else
		forcelink = false;

	ent->msgtime = cl.mtime[0];
	
	if (bits & U_MODEL)
	{
		modnum = MSG_ReadByte ();
		if (modnum >= MAX_MODELS)
			Host_Error ("CL_ParseModel: bad modnum");
	}
	else
		modnum = ent->baseline.modelindex;
		
	model = cl.model_precache[modnum];
	if (model != ent->model)
	{
		ent->model = model;

		Con_DPrintf("model is %s\n",model->name);//Tei debug
#ifdef MULTIMDL
		Mod_RelinkMultiModels(ent);
#endif
	// automatic animation (torches, etc) can be either all together
	// or randomized
		if (model)
		{
			if (model->synctype == ST_RAND)
				ent->syncbase = (float)(rand()&0x7fff) / 0x7fff;
			else
				ent->syncbase = 0.0;
		}
		else
			forcelink = true;	// hack to make null model players work
#ifdef GLQUAKE
		if (num > 0 && num <= cl.maxclients)
			R_TranslatePlayerSkin (num - 1);
#endif
	}
	
	if (bits & U_FRAME)
		ent->frame = MSG_ReadByte ();
	else
		ent->frame = ent->baseline.frame;

	if (bits & U_COLORMAP)
		i = MSG_ReadByte();
	else
		i = ent->baseline.colormap;
	if (!i)
		ent->colormap = vid.colormap;
	else
	{
		if (i > cl.maxclients)
			Sys_Error ("i >= cl.maxclients");
		ent->colormap = cl.scores[i-1].translations;
	}

#ifdef GLQUAKE
	if (bits & U_SKIN)
		skin = MSG_ReadByte();
	else
		skin = ent->baseline.skin;
	if (skin != ent->skinnum) {
		ent->skinnum = skin;
		if (num > 0 && num <= cl.maxclients)
			R_TranslatePlayerSkin (num - 1);
	}

#else

	if (bits & U_SKIN)
		ent->skinnum = MSG_ReadByte();
	else
		ent->skinnum = ent->baseline.skin;
#endif

	if (bits & U_EFFECTS)
		ent->effects = MSG_ReadByte();
	else
		ent->effects = ent->baseline.effects;

// shift the known values for interpolation
	VectorCopy (ent->msg_origins[0], ent->msg_origins[1]);
	VectorCopy (ent->msg_angles[0], ent->msg_angles[1]);

	if (bits & U_ORIGIN1)
		ent->msg_origins[0][0] = MSG_ReadCoord ();
	else
		ent->msg_origins[0][0] = ent->baseline.origin[0];
	if (bits & U_ANGLE1)
		ent->msg_angles[0][0] = MSG_ReadAngle();
	else
		ent->msg_angles[0][0] = ent->baseline.angles[0];

	if (bits & U_ORIGIN2)
		ent->msg_origins[0][1] = MSG_ReadCoord ();
	else
		ent->msg_origins[0][1] = ent->baseline.origin[1];
	if (bits & U_ANGLE2)
		ent->msg_angles[0][1] = MSG_ReadAngle();
	else
		ent->msg_angles[0][1] = ent->baseline.angles[1];

	if (bits & U_ORIGIN3)
		ent->msg_origins[0][2] = MSG_ReadCoord ();
	else
		ent->msg_origins[0][2] = ent->baseline.origin[2];
	if (bits & U_ANGLE3)
		ent->msg_angles[0][2] = MSG_ReadAngle();
	else
		ent->msg_angles[0][2] = ent->baseline.angles[2];

#ifdef EXTENDQC
	if (bits & U_ALPHA)
		ent->alpha = MSG_ReadFloat();
	else
		ent->alpha = 1.0;

	if (bits & U_SCALE)
		ent->scale = MSG_ReadFloat();
	else
		ent->scale = 1.0;

	if (bits & U_GLOW_SIZE)
		ent->glow_size = MSG_ReadFloat();
	else
		ent->glow_size = 0;

	if (bits & U_GLOW_RED)
		ent->glow_red = MSG_ReadFloat();
	else
		ent->glow_red = 0;

	if (bits & U_GLOW_GREEN)
		ent->glow_green = MSG_ReadFloat();
	else
		ent->glow_green = 0;

	if (bits & U_GLOW_BLUE)
		ent->glow_blue = MSG_ReadFloat();
	else
		ent->glow_blue = 0;
#endif

	if ( bits & U_NOLERP )
		ent->forcelink = true;

	if ( forcelink )
	{	// didn't have an update last message
		VectorCopy (ent->msg_origins[0], ent->msg_origins[1]);
		VectorCopy (ent->msg_origins[0], ent->origin);
		VectorCopy (ent->msg_angles[0], ent->msg_angles[1]);
		VectorCopy (ent->msg_angles[0], ent->angles);
		ent->forcelink = true;
	}

	//Con_DPrintf("CL_ParseUpdate %d complete\n",bits);//Tei debug

}

#else
void CL_ParseUpdate (int bits)//Telejano version
{
	int			i;
	model_t		*model;
	int			modnum;
	qboolean	forcelink;
	entity_t	*ent;
	int			num;
	int			skin;

	//Con_Printf("Parse Update\n");//Tei parse update

	if (cls.signon == SIGNONS - 1)
	{	// first update is the final signon stage
		cls.signon = SIGNONS;
		CL_SignonReply ();
	}

	if (bits & U_MOREBITS)
	{
		i = MSG_ReadByte ();
		bits |= (i<<8);
	}

	// Tomaz - QC Control Begin
	if (bits & U_EXTEND1)
	{
		bits |= MSG_ReadByte() << 16;
		if (bits & U_EXTEND2)
			bits |= MSG_ReadByte() << 24;
	}
	// Tomaz - QC Control End

	if (bits & U_LONGENTITY)	
		num = MSG_ReadShort ();
	else
		num = MSG_ReadByte ();

	ent = CL_EntityNum (num);

for (i=0 ; i<16 ; i++)
if (bits&(1<<i))
	bitcounts[i]++;

	if (ent->msgtime != cl.mtime[1])
	{
		forcelink			= true;	// no previous frame to lerp from
		ent->debris_smoke	= 9;
		ent->time_left		= 0;
	}
	else
		forcelink = false;

	ent->msgtime = cl.mtime[0];
	
	if (bits & U_MODEL)
	{
		modnum = MSG_ReadByte ();
		if (modnum >= MAX_MODELS)
			Host_Error ("CL_ParseModel: bad modnum");
	}
	else
		modnum = ent->baseline.modelindex;
		
	model = cl.model_precache[modnum];
	if (model != ent->model)
	{
		ent->model = model;
	// automatic animation (torches, etc) can be either all together
	// or randomized
		if (model)
		{
			if (model->synctype == ST_RAND)
				ent->syncbase = (float)(rand()&0x7fff) / 0x7fff;
			else
				ent->syncbase = 0.0;
		}
		else
			forcelink = true;	// hack to make null model players work
		if (num > 0 && num <= cl.maxclients)
			R_TranslatePlayerSkin (num - 1);
	}

	if (bits & U_FRAME)
		ent->frame = MSG_ReadByte ();
	else
		ent->frame = ent->baseline.frame;

	if (bits & U_COLORMAP)
		i = MSG_ReadByte();
	else
		i = ent->baseline.colormap;

#if 1
	if (!i)
		ent->colormap = vid.colormap;
	else
	{
		// Tei Colormap on monsters // Reactivated by user demand
		if (i > cl.maxclients)
			Sys_Error ("i >= cl.maxclients");
		// Tei Colormap on monsters 
		
		ent->colormap = cl.scores[i-1].translations;
	}
#endif 
	
	if (bits & U_SKIN)
		skin = MSG_ReadByte();
	else
		skin = ent->baseline.skin;
	if (skin != ent->skinnum) 
	{
		ent->skinnum = skin;
		if (num > 0 && num <= cl.maxclients)
			R_TranslatePlayerSkin (num - 1);
	}

	if (bits & U_EFFECTS)
		ent->effects = MSG_ReadByte();
	else
		ent->effects = ent->baseline.effects;

// shift the known values for interpolation
	VectorCopy (ent->msg_origins[0], ent->msg_origins[1]);
	VectorCopy (ent->msg_angles[0], ent->msg_angles[1]);

	if (bits & U_ORIGIN1)
		ent->msg_origins[0][0] = MSG_ReadCoord ();
	else
		ent->msg_origins[0][0] = ent->baseline.origin[0];
	if (bits & U_ANGLE1)
		ent->msg_angles[0][0] = MSG_ReadAngle();
	else
		ent->msg_angles[0][0] = ent->baseline.angles[0];

	if (bits & U_ORIGIN2)
		ent->msg_origins[0][1] = MSG_ReadCoord ();
	else
		ent->msg_origins[0][1] = ent->baseline.origin[1];
	if (bits & U_ANGLE2)
		ent->msg_angles[0][1] = MSG_ReadAngle();
	else
		ent->msg_angles[0][1] = ent->baseline.angles[1];

	if (bits & U_ORIGIN3)
		ent->msg_origins[0][2] = MSG_ReadCoord ();
	else
		ent->msg_origins[0][2] = ent->baseline.origin[2];
	if (bits & U_ANGLE3)
		ent->msg_angles[0][2] = MSG_ReadAngle();
	else
		ent->msg_angles[0][2] = ent->baseline.angles[2];


	// Tei infovel
	//if (bits & U_INFOVEL) {
	//	ent->info_velocity[0] = MSG_ReadCoord ();
	//	ent->info_velocity[1] = MSG_ReadCoord ();
	//	ent->info_velocity[2] = MSG_ReadCoord ();
	//}
	// Tei infovel	


	
	
	// Tomaz - QC Alpha Scale Glow & More Frames Begin
	if (bits & U_ALPHA)
		ent->alpha = MSG_ReadFloat();
	else
		ent->alpha = 1;

	if (bits & U_SCALE)
		ent->scale = MSG_ReadFloat();
	else
		ent->scale = 1;

	if (bits & U_GLOW_SIZE)
		ent->glow_size = MSG_ReadFloat();
	else
		ent->glow_size = 0;

	if (bits & U_GLOW_RED)
		ent->glow_red = MSG_ReadFloat();
	else
		ent->glow_red = 0;

	if (bits & U_GLOW_GREEN)
		ent->glow_green = MSG_ReadFloat();
	else
		ent->glow_green = 0;

	if (bits & U_GLOW_BLUE)
		ent->glow_blue = MSG_ReadFloat();
	else
		ent->glow_blue = 0;
#if 1
	// Tei more fx2
	if (bits & U_EFFECTS2)
		ent->effects2 = MSG_ReadByte();
	else
		ent->effects2 = ent->baseline.effects2;

	if (bits & U_EFFECTS3)
		ent->effects3 = MSG_ReadByte();
	else
		ent->effects3 = ent->baseline.effects3;
	// Tei more fx2
#endif

	if (bits & U_FRAME2)
		ent->frame |= (MSG_ReadByte() <<8);
	else
		ent->frame |= (ent->baseline.frame & 0xFF00);	

	// Tomaz - QC Alpha Scale Glow & More Frames End

	if ( bits & U_NOLERP )
		ent->forcelink = true;

	if ( forcelink )
	{	// didn't have an update last message
		VectorCopy (ent->msg_origins[0], ent->msg_origins[1]);
		VectorCopy (ent->msg_origins[0], ent->origin);
		VectorCopy (ent->msg_angles[0], ent->msg_angles[1]);
		VectorCopy (ent->msg_angles[0], ent->angles);
		ent->forcelink = true;
	}


	//if (ent->model->type == mod_hud)
	//	Con_Print("here is good\n");  Ok, mod_hud arrive here

	//Con_Printf("Parse Update - close\n");//Tei parse update
}
#endif


/*
==================
CL_ParseBaseline
==================
*/
void CL_ParseBaseline (entity_t *ent)
{
	int			i;
	
	ent->baseline.modelindex = MSG_ReadByte ();
	ent->baseline.frame = MSG_ReadByte ();
	ent->baseline.colormap = MSG_ReadByte();
	ent->baseline.skin = MSG_ReadByte();
	for (i=0 ; i<3 ; i++)
	{
		ent->baseline.origin[i] = MSG_ReadCoord ();
		ent->baseline.angles[i] = MSG_ReadAngle ();
	}
}


/*
==================
CL_ParseClientdata

Server information pertaining to this client only
==================
*/
void CL_ParseClientdata (int bits)
{
	int		i, j;
	//Con_Printf("open clientdata\n");

	if (bits & SU_VIEWHEIGHT)
		cl.viewheight = MSG_ReadChar ();
	else
		cl.viewheight = DEFAULT_VIEWHEIGHT;

	if (bits & SU_IDEALPITCH)
		cl.idealpitch = MSG_ReadChar ();
	else
		cl.idealpitch = 0;
	
	VectorCopy (cl.mvelocity[0], cl.mvelocity[1]);
	for (i=0 ; i<3 ; i++)
	{
		if (bits & (SU_PUNCH1<<i) )
			cl.punchangle[i] = MSG_ReadChar();
		else
			cl.punchangle[i] = 0;
		if (bits & (SU_VELOCITY1<<i) )
			cl.mvelocity[0][i] = MSG_ReadChar()*16;
		else
			cl.mvelocity[0][i] = 0;
	}

// [always sent]	if (bits & SU_ITEMS)
		i = MSG_ReadLong ();

	if (cl.items != i)
	{	// set flash times
		for (j=0 ; j<32 ; j++)
			if ( (i & (1<<j)) && !(cl.items & (1<<j)))
				cl.item_gettime[j] = cl.time;
		cl.items = i;
	}
		
	cl.onground = (bits & SU_ONGROUND) != 0;
	cl.inwater = (bits & SU_INWATER) != 0;

	if (bits & SU_WEAPONFRAME)
		cl.stats[STAT_WEAPONFRAME] = MSG_ReadByte ();
	else
		cl.stats[STAT_WEAPONFRAME] = 0;

	if (bits & SU_ARMOR)
		i = MSG_ReadByte ();
	else
		i = 0;
	if (cl.stats[STAT_ARMOR] != i)
		cl.stats[STAT_ARMOR] = i;

	if (bits & SU_WEAPON)
		i = MSG_ReadByte ();
	else
		i = 0;
	if (cl.stats[STAT_WEAPON] != i)
		cl.stats[STAT_WEAPON] = i;
	
	i = MSG_ReadShort ();
	if (cl.stats[STAT_HEALTH] != i)
		cl.stats[STAT_HEALTH] = i;

	i = MSG_ReadByte ();
	if (cl.stats[STAT_AMMO] != i)
		cl.stats[STAT_AMMO] = i;


	for (i=0 ; i<4 ; i++)
	{
		j = MSG_ReadByte ();
		if (cl.stats[STAT_SHELLS+i] != j)
			cl.stats[STAT_SHELLS+i] = j;
	}

	i = MSG_ReadByte ();

	
	if (standard_quake)
	{
		if (cl.stats[STAT_ACTIVEWEAPON] != i)
			cl.stats[STAT_ACTIVEWEAPON] = i;
	}
	else
	{
		if (cl.stats[STAT_ACTIVEWEAPON] != (1<<i))
			cl.stats[STAT_ACTIVEWEAPON] = (1<<i);
	}

	//Con_Printf("closed clientdata\n");
}

/*
=====================
CL_NewTranslation
=====================
*/
void CL_NewTranslation (int slot)
{
	int		i, j;
	int		top, bottom;
	byte	*dest, *source;
	
	if (slot > cl.maxclients)
		Sys_Error ("CL_NewTranslation: slot > cl.maxclients");
	dest = cl.scores[slot].translations;
	source = vid.colormap;
	memcpy (dest, vid.colormap, sizeof(cl.scores[slot].translations));
	top = cl.scores[slot].colors & 0xf0;
	bottom = (cl.scores[slot].colors &15)<<4;

	R_TranslatePlayerSkin (slot);

	for (i=0 ; i<VID_GRADES ; i++, dest += 256, source+=256)
	{
		if (top < 128)	// the artists made some backwards ranges.  sigh.
			memcpy (dest + TOP_RANGE, source + top, 16);
		else
			for (j=0 ; j<16 ; j++)
				dest[TOP_RANGE+j] = source[top+15-j];
				
		if (bottom < 128)
			memcpy (dest + BOTTOM_RANGE, source + bottom, 16);
		else
			for (j=0 ; j<16 ; j++)
				dest[BOTTOM_RANGE+j] = source[bottom+15-j];		
	}
}

/*
=====================
CL_ParseStatic
=====================
*/
void CL_ParseStatic (void)
{
	entity_t *ent;
	int		i;
		
	i = cl.num_statics;
	if (i >= MAX_STATIC_ENTITIES)
		Host_Error ("Too many static entities");
	ent = &cl_static_entities[i];
	cl.num_statics++;
	CL_ParseBaseline (ent);

// copy it to the current state
	ent->model = cl.model_precache[ent->baseline.modelindex];
	ent->frame = ent->baseline.frame;
	ent->colormap = vid.colormap;
	ent->skinnum = ent->baseline.skin;
	ent->effects = ent->baseline.effects;

#if 0 //Tei debug
	// Tei more fx23 static
	if(!old_progsdat.value)
	{
		ent->effects2 = ent->baseline.effects2;
		ent->effects3 = ent->baseline.effects3;
	}
	else
	{
		ent->effects2 = 0;
		ent->effects3 = 0;
	}
	VectorCopy (ent->origin, ent->oldorg);//XFX
	//ent->alpha	  = ent->baseline.alpha;
	// Tei more fx23 static
#endif

	VectorCopy (ent->baseline.origin, ent->origin);
	VectorCopy (ent->baseline.angles, ent->angles);	

	//Tei randomize grass for static entitys
	if(ent && ent->model && ent->model->effect == MFX_GRASS)
		ent->angles[1] = teirand(100);

	R_AddEfrags (ent);
}


/*
=====================
CL_ParseStatic2  PS with extra values: alfa, ...
=====================
*/
void CL_ParseStatic2 (void)
{
	entity_t *ent;
	int		i, alfa ;
		
	i = cl.num_statics;
	if (i >= MAX_STATIC_ENTITIES)
		Host_Error ("Too many static entities");
	ent = &cl_static_entities[i];
	cl.num_statics++;
	CL_ParseBaseline (ent);

// copy it to the current state
	ent->model = cl.model_precache[ent->baseline.modelindex];
	ent->frame = ent->baseline.frame;
	ent->colormap = vid.colormap;
	ent->skinnum = ent->baseline.skin;
	ent->effects = ent->baseline.effects;
	
	alfa = MSG_ReadByte();//Tei extra
	if (!alfa)
		alfa = 1;
	else
		alfa = alfa / 255;

	ent->alpha	 = alfa;

	// Tei more fx23 static
	if(!old_progsdat.value)
	{
		ent->effects2 = ent->baseline.effects2;
		ent->effects3 = ent->baseline.effects3;
	}
	else
	{
		ent->effects2 = 0;
		ent->effects3 = 0;		
	}
	VectorCopy (ent->origin, ent->oldorg);//XFX
	//ent->alpha	  = ent->baseline.alpha;
	// Tei more fx23 static

	VectorCopy (ent->baseline.origin, ent->origin);
	VectorCopy (ent->baseline.angles, ent->angles);	
	R_AddEfrags (ent);
}

/*
===================
CL_ParseStaticSound
===================
*/
void CL_ParseStaticSound (void)
{
	vec3_t		origin;
	int			sound_num, volume, atten;
	
	origin[0]	= MSG_ReadCoord ();
	origin[1]	= MSG_ReadCoord ();
	origin[2]	= MSG_ReadCoord ();
	sound_num	= MSG_ReadByte ();
	volume		= MSG_ReadByte ();
	atten		= MSG_ReadByte ();
	
	S_StaticSound (cl.sound_precache[sound_num], origin, volume, atten);
}


//#define SHOWNET(x) if(cl_shownet.value==2)Con_Printf ("%3i:%s\n", msg_readcount-1, x);
#define SHOWNET(x) ;//Tei: may break dm playing...

extern void HIDELMP();	// Tomaz - Show LMP
extern void SHOWLMP(int origin );// Tomaz - Hide LMP
extern void SHOWCNT(int origin );// Tei - lmp counters
extern void UpdateLmp(void); // Tei update lmp

/*
=====================
CL_ParseServerMessage
=====================
*/

void MoveLmp(void);


#if 0 //Old version
void CL_ParseServerMessage (void)
{
	int			cmd;
	int			i;

	// dummy holders for keeping comms in sync in case of invalid msgs.
	char fredstring[1024];
	long fredlong;
	
#if 0 //Tei debug
	// if recording demos, copy the message out
	if (cl_shownet.value == 1)
		Con_Printf ("%i ",net_message.cursize);
	else if (cl_shownet.value == 2)
		Con_Printf ("------------------\n");
#endif 
	
	//Con_DPrintf ("New Server Frame\n------------------\n");//Tei debug
	
	cl.onground = false;	// unless the server says otherwise	

	// parse the message
	MSG_BeginReading ();
	
	while (1)
	{
		if (msg_badread)
			Host_Error ("CL_ParseServerMessage: Bad server message");

		cmd = MSG_ReadByte ();
		
		//Con_DPrintf("net_message size is %d, cursize %d,readcount:..%d\n",net_message.maxsize,net_message.cursize,msg_readcount);
		
		
		if (cmd == -1)
		{
			//SHOWNET("END OF MESSAGE");
		//	Con_DPrintf("...error or end of data\n");//Tei debug
			return;		// end of message
		}
		
		//Con_DPrintf("...1\n");//Tei debug

		if (msg_badread)
		{			
			Host_Error ("CL_ParseServerMessage: Bad server cmd message");
		}

#if 0		
		if(cmd<0 || cmd > 40)
			Con_DPrintf("svc id is %d\n",cmd);//Tei debug
		else
			Con_DPrintf("svc string is %s\n",svc_strings[cmd]);//Tei debug
#endif 


		// if the high bit of the command byte is set, it is a fast update
		if (cmd & 128)
		{
			//SHOWNET("fast update");
			CL_ParseUpdate (cmd&127);
			continue;
		}

		//SHOWNET(svc_strings[cmd]);
	
		// other commands
		switch (cmd)
		{
		default:
			Host_Error ("CL_ParseServerMessage: Illegible server message\n");
			break;

		case svc_nop:
			 Con_DPrintf ("svc_nop from client\n");//Tei debug
			break;

		case svc_time:
			cl.mtime[1] = cl.mtime[0];
			cl.mtime[0] = MSG_ReadFloat ();			
			break;

		case svc_clientdata:
			i = MSG_ReadShort ();
			CL_ParseClientdata (i);
			break;

		case svc_version:
			i = MSG_ReadLong ();
			if (i != PROTOCOL_VERSION)
				Host_Error ("CL_ParseServerMessage: Server is protocol %i instead of %i\n", i, PROTOCOL_VERSION);
			break;

		case svc_disconnect:
			Host_EndGame ("Server disconnected\n");

		case svc_print:
			Con_Printf ("%s", MSG_ReadString ());
			break;

		case svc_centerprint:
			SCR_CenterPrint (MSG_ReadString ());
			break;

		case svc_stufftext:
			Cbuf_AddText (MSG_ReadString ());
			break;

		case svc_damage:
			V_ParseDamage ();
			break;

		case svc_serverinfo:
			CL_ParseServerInfo ();
			//vid.recalc_refdef = true;	// leave intermission full screen
			break;

		case svc_setangle:
			for (i=0 ; i<3 ; i++)
				cl.viewangles[i] = MSG_ReadAngle ();
			break;

		case svc_setview:
			cl.viewentity = MSG_ReadShort ();
			break;

		case svc_lightstyle:
			i = MSG_ReadByte ();

			if (i >= MAX_LIGHTSTYLES)
			{
				// make this safe by doing nothing instead of crashing - we still have to read
				// the string to keep the communications happily in sync.
				strcpy (fredstring, MSG_ReadString ());
				break;
			}

			strcpy (cl_lightstyle[i].map,  MSG_ReadString());
			cl_lightstyle[i].length = strlen(cl_lightstyle[i].map);
			break;

		case svc_sound:
			CL_ParseStartSoundPacket();
			break;

		case svc_stopsound:
			i = MSG_ReadShort();
			S_StopSound(i>>3, i&7);
			break;

		case svc_updatename:
			i = MSG_ReadByte ();
			if (i >= cl.maxclients)
				Host_Error ("CL_ParseServerMessage: svc_updatename > MAX_SCOREBOARD");
			strcpy (cl.scores[i].name, MSG_ReadString ());
			break;

		case svc_updatefrags:
			i = MSG_ReadByte ();
			if (i >= cl.maxclients)
				Host_Error ("CL_ParseServerMessage: svc_updatefrags > MAX_SCOREBOARD");
			cl.scores[i].frags = MSG_ReadShort ();
			break;			

		case svc_updatecolors:
			i = MSG_ReadByte ();
			if (i >= cl.maxclients)
				Host_Error ("CL_ParseServerMessage: svc_updatecolors > MAX_SCOREBOARD");
			cl.scores[i].colors = MSG_ReadByte ();
			CL_NewTranslation (i);
			break;

		case svc_particle:
			R_ParseParticleEffect ();
			break;

		case svc_spawnbaseline:
			i = MSG_ReadShort ();
			// must use CL_EntityNum() to force cl.num_entities up
			CL_ParseBaseline (CL_EntityNum(i));
			break;

		case svc_spawnstatic:
			CL_ParseStatic ();
			break;

		case svc_temp_entity:
			CL_ParseTEnt ();
			break;

		case svc_setpause:
			{
				cl.paused = MSG_ReadByte ();

				if (cl.paused)
				{
					CDAudio_Pause ();
					//VID_HandlePause (true);
				}
				else
				{
					CDAudio_Resume ();
					//VID_HandlePause (false);
				}
			}
			break;

		case svc_signonnum:
			i = MSG_ReadByte ();
			if (i <= cls.signon)
				Host_Error ("Received signon %i when at %i", i, cls.signon);
			cls.signon = i;
			CL_SignonReply ();
			break;

		case svc_killedmonster:
			cl.stats[STAT_MONSTERS]++;
			break;

		case svc_foundsecret:
			cl.stats[STAT_SECRETS]++;
			break;

		case svc_updatestat:
			i = MSG_ReadByte ();

			if (i < 0 || i >= MAX_CL_STATS)
			{
				// do nothing instead of crashing - read the long to keep comms in sync
				fredlong = MSG_ReadLong ();
				break;
			}

			cl.stats[i] = MSG_ReadLong ();;
			break;

		case svc_spawnstaticsound:
			CL_ParseStaticSound ();
			break;

		case svc_cdtrack:
			cl.cdtrack = MSG_ReadByte ();
			cl.looptrack = MSG_ReadByte ();

			if ( (cls.demoplayback || cls.demorecording) && (cls.forcetrack != -1) )
				CDAudio_Play ((byte)cls.forcetrack, true);
			else
				CDAudio_Play ((byte)cl.cdtrack, true);
			break;

		case svc_intermission:
			cl.intermission = 1;
			cl.completed_time = cl.time;
			//vid.recalc_refdef = true;	// go to full screen
			break;

		case svc_finale:
			cl.intermission = 2;
			cl.completed_time = cl.time;
			//vid.recalc_refdef = true;	// go to full screen
			SCR_CenterPrint (MSG_ReadString ());			
			break;

		case svc_cutscene:
			cl.intermission = 3;
			cl.completed_time = cl.time;
			//vid.recalc_refdef = true;	// go to full screen
			SCR_CenterPrint (MSG_ReadString ());			
			break;

		case svc_sellscreen:
			Cmd_ExecuteString ("help", src_command);
			break;
		}
		
		//Con_DPrintf("svc string is %s complete\n",svc_strings[cmd]);//Tei debug

	}
}

#else 

void CL_ParseServerMessage (void) //New telejano version
{
	int			cmd;
	int			i;
#if 0
	//TELEJANO
	//Lh!
	byte		cmdlog[32];
	char		*cmdlogname[32];//, *temp;
	int			cmdindex, cmdcount = 0;
	//Lh!
#endif

	//Con_Printf("open parserver\n");

//
// if recording demos, copy the message out
//
	if (cl_shownet.value == 1)
		Con_Printf ("%i ",net_message.cursize);
	else if (cl_shownet.value == 2)
		Con_Printf ("------------------\n");
	
	cl.onground = false;	// unless the server says otherwise	
//
// parse the message
//
	MSG_BeginReading ();
	
	while (1)
	{
		if (msg_badread)
			Host_Error ("CL_ParseServerMessage: Bad server message");

		cmd = MSG_ReadByte ();

		if (cmd == -1)
		{
		//	SHOWNET("END OF MESSAGE");
			return;		// end of message
		}
#if 0
		//TELEJANO
		//Lh!
		cmdindex = cmdcount & 31;
		cmdcount++;
		cmdlog[cmdindex] = cmd;
		//Lh!
#endif 

	// if the high bit of the command byte is set, it is a fast update
		if (cmd & 128)
		{
			//SHOWNET("fast update");
			CL_ParseUpdate (cmd&127);
			continue;
		}

		//SHOWNET(svc_strings[cmd]);

		//Con_Printf ("%3i:%s\n", msg_readcount-1, svc_strings[cmd]);//Tei debug
	
	// other commands
		switch (cmd)
		{
		default:
#if 0
			//TELEJANO
			//Lh packet debuging
			{
				char description[32*64], temp[64];
				int count;

				Con_Printf("net debug!\n");

				strcpy(description, "packet dump: ");
				i = cmdcount - 32;
				if (i < 0)
					i = 0;
				count = cmdcount - i;
				i &= 31;
				while(count > 0)
				{
					sprintf(temp, "%3i:%s ", cmdlog[i], cmdlogname[i]);
					strcat(description, temp);
					count--;
					i++;
					i &= 31;
				}
				description[strlen(description)-1] = '\n'; // replace the last space with a newline
				Con_Printf("%s", description);
				Host_Error ("CL_ParseServerMessage: Illegible server message\n");
			}
			//Lh packet debuging
#else
			Host_Error ("CL_ParseServerMessage: Illegible server message\n");
			Con_Printf("message: %i\n", cmd);
#endif
			break;
			
		case svc_nop:
			Con_Printf ("svc_nop\n");//Tei debug
			break;
			
		case svc_time:
			cl.mtime[1] = cl.mtime[0];
			cl.mtime[0] = MSG_ReadFloat ();			
			break;
			
		case svc_clientdata:
			i = MSG_ReadShort ();
			CL_ParseClientdata (i);
			break;
		
		case svc_version:
			i = MSG_ReadLong ();
			if (i != PROTOCOL_VERSION)
				Host_Error ("CL_ParseServerMessage: Server is protocol %i instead of %i\n", i, PROTOCOL_VERSION);
			break;
			
		case svc_disconnect:
			Host_EndGame ("Server disconnected\n");

		case svc_print:
			Con_Printf ("%s", MSG_ReadString ());
			break;
			
		case svc_centerprint:
			SCR_CenterPrint (MSG_ReadString ());
			break;
			
		case svc_stufftext:
			Cbuf_AddText (MSG_ReadString ());
			break;
			
		case svc_damage:
			V_ParseDamage ();
			break;
			
		case svc_serverinfo:
			CL_ParseServerInfo ();
			vid.recalc_refdef = true;	// leave intermission full screen
			break;
			
		case svc_setangle:
			cl.viewangles[0] = MSG_ReadAngle ();
			cl.viewangles[1] = MSG_ReadAngle ();
			cl.viewangles[2] = MSG_ReadAngle ();
			break;
			
		case svc_setview:
			cl.viewentity = MSG_ReadShort ();
			break;
					
		case svc_lightstyle:
			i = MSG_ReadByte ();
			if (i >= MAX_LIGHTSTYLES)
				Sys_Error ("svc_lightstyle > MAX_LIGHTSTYLES");
			Q_strcpy (cl_lightstyle[i].map,  MSG_ReadString());
			cl_lightstyle[i].length = Q_strlen(cl_lightstyle[i].map);
			break;
			
		case svc_sound:
			CL_ParseStartSoundPacket();
			break;
			
		case svc_stopsound:
			i = MSG_ReadShort();
			S_StopSound(i>>3, i&7);
			break;
		
		case svc_updatename:
			i = MSG_ReadByte ();
			if (i >= cl.maxclients)
				Host_Error ("CL_ParseServerMessage: svc_updatename > MAX_SCOREBOARD");
			strcpy (cl.scores[i].name, MSG_ReadString ());
			break;
			
		case svc_updatefrags:
			i = MSG_ReadByte ();
			if (i >= cl.maxclients)
				Host_Error ("CL_ParseServerMessage: svc_updatefrags > MAX_SCOREBOARD");
			cl.scores[i].frags = MSG_ReadShort ();
			break;			

		case svc_updatecolors:
			i = MSG_ReadByte ();
			if (i >= cl.maxclients)
				Host_Error ("CL_ParseServerMessage: svc_updatecolors > MAX_SCOREBOARD");
			cl.scores[i].colors = MSG_ReadByte ();
			CL_NewTranslation (i);
			break;
			
		case svc_particle:
			R_ParseParticleEffect ();
			break;

		case svc_spawnbaseline:
			i = MSG_ReadShort ();
			// must use CL_EntityNum() to force cl.num_entities up
			CL_ParseBaseline (CL_EntityNum(i));
			break;
		case svc_spawnstatic:
			CL_ParseStatic ();
			break;			
		case svc_temp_entity:
			CL_ParseTEnt ();
			break;

		case svc_setpause:
			cl.paused = MSG_ReadByte ();
			break;
			
		case svc_signonnum:
			i = MSG_ReadByte ();
			if (i <= cls.signon)
				Host_Error ("Received signon %i when at %i", i, cls.signon);
			cls.signon = i;
			CL_SignonReply ();
			break;

		case svc_killedmonster:
			cl.stats[STAT_MONSTERS]++;
			break;

		case svc_foundsecret:
			cl.stats[STAT_SECRETS]++;
			break;

		case svc_updatestat:
			i = MSG_ReadByte ();
			if (i < 0 || i >= MAX_CL_STATS)
				Sys_Error ("svc_updatestat: %i is invalid", i);
			cl.stats[i] = MSG_ReadLong ();;
			break;
			
		case svc_spawnstaticsound:
			CL_ParseStaticSound ();
			break;

		case svc_cdtrack:
			cl.cdtrack = MSG_ReadByte ();
			cl.looptrack = MSG_ReadByte ();
			if ( (cls.demoplayback || cls.demorecording) && (cls.forcetrack != -1) )
				CDAudio_Play ((byte)cls.forcetrack, true);
			else
				CDAudio_Play ((byte)cl.cdtrack, true);
			break;

		case svc_intermission:
			cl.intermission = 1;
			cl.completed_time = cl.time;
			vid.recalc_refdef = true;	// go to full screen
			break;

		case svc_finale:
			cl.intermission = 2;
			cl.completed_time = cl.time;
			vid.recalc_refdef = true;	// go to full screen
			SCR_CenterPrint (MSG_ReadString ());			
			break;

		case svc_cutscene:
			cl.intermission = 3;
			cl.completed_time = cl.time;
			vid.recalc_refdef = true;	// go to full screen
			SCR_CenterPrint (MSG_ReadString ());			
			break;


		case svc_sellscreen:
			Cmd_ExecuteString ("help", src_command);
			break;

		// Tomaz - Show Hide LMP Begin
		case svc_hidelmp:
			HIDELMP();
			break;
		case svc_showlmp:
			SHOWLMP(0);
			break;
		// Tomaz - Show Hide LMP End


		// Tei itemsgui
#if 1
			//TELEJANO			
		case svc_showlmp2:			
			SHOWLMP( MSG_ReadByte() );
			break;

		case svc_showlmp3:			
			SHOWCNT( MSG_ReadByte() );
			break;

		case svc_movelmp:			
			MoveLmp();
			break;

		case svc_updatelmp:			
			UpdateLmp();
			break;
#endif 				
		// Tei itemsgui

		// Tomaz - Skybox Begin
		case svc_skybox:
			R_SetSkyBox(MSG_ReadString());
			break;
		// Tomaz - Skybox End

		// Tei change fov svc
		case svc_changefov:
			i = MSG_ReadByte ();
			scr_fov.value = i;
			break;
		// Tei change fov svc

		
#if 0
		//TELEJANO
		// Tei rexec svc
		case svc_rexec:
			Cmd_ExecuteString (MSG_ReadString(),src_command);
			break;
		// Tei rexec svc
#endif

		case svc_spawnstatic2:
			CL_ParseStatic2 ();
			break;			

		}
	}
	Con_Printf("close clientdata\n");
}

#endif
