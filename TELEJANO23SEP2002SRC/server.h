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
// server.h

typedef struct
{
	int			maxclients;
	int			maxclientslimit;
	struct client_s	*clients;		// [maxclients]
	int			serverflags;		// episode completion information
	qboolean	changelevel_issued;	// cleared when at SV_SpawnServer
} server_static_t;

//=============================================================================

typedef enum {ss_loading, ss_active} server_state_t;

typedef struct
{
	qboolean	active;				// false if only a net client

	qboolean	paused;
	qboolean	loadgame;			// handle connections specially

	double		time;
	
	int			lastcheck;			// used by PF_checkclient
	double		lastchecktime;
	
	char		name[64];			// map name
	char		modelname[64];		// maps/<name>.bsp, for model_precache[0]
	struct model_s 	*worldmodel;
	char		*model_precache[MAX_MODELS];	// NULL terminated
	struct model_s	*models[MAX_MODELS];
	char		*sound_precache[MAX_SOUNDS];	// NULL terminated
	char		*lightstyles[MAX_LIGHTSTYLES];
	int			num_edicts;
	int			max_edicts;
	edict_t		*edicts;			// can NOT be array indexed, because
									// edict_t is variable sized, but can
									// be used to reference the world ent
	server_state_t	state;			// some actions are only valid during load

	sizebuf_t	datagram;
	byte		datagram_buf[MAX_DATAGRAM];

	sizebuf_t	reliable_datagram;	// copied to all clients at end of frame
	byte		reliable_datagram_buf[MAX_DATAGRAM];

	sizebuf_t	signon;

	byte		signon_buf[32768]; // LordHavoc: increased signon message buffer from 8192 to 32768

} server_t;


#define	NUM_PING_TIMES		16
#define	NUM_SPAWN_PARMS		16

typedef struct client_s
{
	qboolean		active;				// false = client is free
	qboolean		spawned;			// false = don't send datagrams
	qboolean		dropasap;			// has been told to go to another level
	qboolean		sendsignon;			// only valid before spawned

	// LordHavoc: to make netquake protocol get through NAT routers, have to wait for client to ack
	qboolean		waitingforconnect;	// waiting for connect from client (stage 1)
	qboolean		sendserverinfo;		// send server info in next datagram (stage 2)
	// LordHavoc: to make netquake protocol get through NAT routers, have to wait for client to ack

	double			last_message;		// reliable messages must be sent
										// periodically

	struct qsocket_s *netconnection;	// communications handle

	usercmd_t		cmd;				// movement
	vec3_t			wishdir;			// intended motion calced from cmd

	sizebuf_t		message;			// can be added to at any time,
										// copied and clear once per frame
	byte			msgbuf[MAX_MSGLEN];
	edict_t			*edict;				// EDICT_NUM(clientnum+1)
	char			name[32];			// for printing to other people
	int				colors;
		
	float			ping_times[NUM_PING_TIMES];
	int				num_pings;			// ping_times[num_pings%NUM_PING_TIMES]

// spawn parms are carried from level to level
	float			spawn_parms[NUM_SPAWN_PARMS];

// client known data for deltas	
	int				old_frags;
} client_t;


//=============================================================================

// edict->movetype values
#define	MOVETYPE_NONE			0		// never moves
#define	MOVETYPE_ANGLENOCLIP	1
#define	MOVETYPE_ANGLECLIP		2
#define	MOVETYPE_WALK			3		// gravity
#define	MOVETYPE_STEP			4		// gravity, special edge handling
#define	MOVETYPE_FLY			5
#define	MOVETYPE_TOSS			6		// gravity
#define	MOVETYPE_PUSH			7		// no clip to world, push and crush
#define	MOVETYPE_NOCLIP			8
#define	MOVETYPE_FLYMISSILE		9		// extra size to monsters
#define	MOVETYPE_BOUNCE			10
#define MOVETYPE_BOUNCEMISSILE	11		// bounce w/o gravity
#define MOVETYPE_FOLLOW			12		// Tomaz - MOVETYPE_FOLLOW
#define MOVETYPE_MAGNETIC		13		// Tei - Movetype magnetic
#define MOVETYPE_MAGNETICFOLLOW 14		// Tei - Movetype magnetic to entity
#define MOVETYPE_RELATIVESTATIC 15		// Tei - Movetype follow without rotations
#define MOVETYPE_PREDATOR		16		// Tei experimental 
#define MOVETYPE_CHASECAM		17		// Tei experimental 
#define MOVETYPE_WATERFLOAT		18		// Tei experimental


// edict->solid values
#define	SOLID_NOT				0		// no interaction with other objects
#define	SOLID_TRIGGER			1		// touch on edge, but not blocking
#define	SOLID_BBOX				2		// touch on edge, block
#define	SOLID_SLIDEBOX			3		// touch on edge, but not an onground
#define	SOLID_BSP				4		// bsp clip, touch on edge, block

// edict->deadflag values
#define	DEAD_NO					0
#define	DEAD_DYING				1
#define	DEAD_DEAD				2

#define	DAMAGE_NO				0
#define	DAMAGE_YES				1
#define	DAMAGE_AIM				2

// edict->flags
#define	FL_FLY					1
#define	FL_SWIM					2
//#define	FL_GLIMPSE				4
#define	FL_CONVEYOR				4
#define	FL_CLIENT				8
#define	FL_INWATER				16
#define	FL_MONSTER				32
#define	FL_GODMODE				64
#define	FL_NOTARGET				128
#define	FL_ITEM					256
#define	FL_ONGROUND				512
#define	FL_PARTIALGROUND		1024	// not all corners are valid
#define	FL_WATERJUMP			2048	// player jumping out of water
#define	FL_JUMPRELEASED			4096	// for jump debouncing

// entity effects

#define	EF_BRIGHTFIELD			1
#define	EF_MUZZLEFLASH 			2
#define	EF_BRIGHTLIGHT 			4
#define	EF_DIMLIGHT 			8

// Tei ultra decals
#define EF_DECAL				16
#define EF_BLUEFIRE				32
#define EF_FIRE					64
#define EF_DOWFIRE				128

#define EF2_VOORTRAIL			1
#define	EF2_BIGFIRE				2
#define	EF2_FOGSPLASH			4
#define	EF2_WATERFALL			8
#define EF2_FOGSPLASHLITE		16
#define EF2_SPARKSHOWER			32
#define EF2_DARKFIELD            64
#define EF2_ROTATEBIT           128

#define EF3_ROTATE1			1
#define EF3_ROTATE2			2
#define EF3_ROTATE0			3
#define EF3_ROTATE012		4
#define EF3_ROTATEZ			5
#define EF3_AUTOSNOW		6
#define EF3_TEMPUSVIVENDI	7
#define EF3_TEMPUSFIRE		8
#define EF3_TEMPUSSMOKE		9
#define EF3_SHADOWSHIELD	10
#define EF3_FLUXFIRE		11
#define EF3_ROTATEZ10		12
#define EF3_ROTATEZ5		13
#define EF3_MAGICFIRE		14
#define EF3_DARKSMOKE		15
#define EF3_DARKTRAIL		16
#define EF3_DARKTRAIL2		17
#define EF3_FLUXFIRESMALL	18
#define EF3_CIRCLETRAIL		19 
#define EF3_NODRAW			20 // No draw
#define EF3_CLASSICDOWNFIRE 21 
#define EF3_NONET			22 // No net to client
#define EF3_AUTOVANISH		23
#define EF3_EBEAM			24 // Q2 ClEngine Beam 
#define EF3_HIPERTRANS		25
#define EF3_HIPERTRANS2		26 
#define EF3_Q3AUTOGUN		27 // Random Flash
#define EF3_GRAYBITS		28 // Walk smoke
#define EF3_FULLUX			29 // Lux = 100%
#define EF3_NOLUX			30 // Lux = 0
#define EF3_NOLERP			31 // 
#define EF3_VISIBLE			32 // 
#define EF3_GRAYBITS2		33 // Walk smoke
#define EF3_GRAYBITS3		34 // Walk smoke
#define EF3_SPRBEAM			35
#define EF3_FOXFIRE			36 // Motor fire
#define EF3_EFIRE			37
#define EF3_ADDITIVE		38 //Additive rendering
#define EF3_CHEAPLENZ       48 // something similir to + lenz
#define EF3_DPXFLARE		49 // something like dp lenz
#define EF3_DPXNUKE			50 // nuke flare!
#define EF3_ISWEAPON		51 // Entity will show as weaponmodel
#define EF3_VISIBLE2		52 // visible is trace from me to it (flare?, hud?, lame?)
#define EF3_ALIENBLOOD		53
#define EF3_NOGRASS			54

// Tei ultra decals


#define	SPAWNFLAG_NOT_EASY			256
#define	SPAWNFLAG_NOT_MEDIUM		512
#define	SPAWNFLAG_NOT_HARD			1024
#define	SPAWNFLAG_NOT_DEATHMATCH	2048

//============================================================================

extern	cvar_t	teamplay;
extern	cvar_t	skill;
extern	cvar_t	deathmatch;
extern	cvar_t	coop;
extern	cvar_t	fraglimit;
extern	cvar_t	timelimit;

extern	server_static_t	svs;				// persistant server info
extern	server_t		sv;					// local server

extern	client_t	*host_client;

extern	jmp_buf 	host_abortserver;

extern	double		host_time;

extern	edict_t		*sv_player;

//===========================================================

void SV_Init (void);

void SV_StartParticle (vec3_t org, vec3_t dir, int color, int count);
void SV_StartSound (edict_t *entity, int channel, char *sample, int volume,
    float attenuation);

void SV_DropClient (qboolean crash);

void SV_SendClientMessages (void);
void SV_ClearDatagram (void);

int SV_ModelIndex (char *name);

void SV_SetIdealPitch (void);

void SV_AddUpdates (void);

void SV_ClientThink (void);
void SV_AddClientToServer (struct qsocket_s	*ret);

void SV_ClientPrintf (char *fmt, ...);
void SV_BroadcastPrintf (char *fmt, ...);

void SV_Physics (void);

qboolean SV_CheckBottom (edict_t *ent);
qboolean SV_movestep (edict_t *ent, vec3_t move, qboolean relink);

void SV_WriteClientdataToMessage (edict_t *ent, sizebuf_t *msg);

void SV_MoveToGoal (void);

void SV_CheckForNewClients (void);
void SV_RunClients (void);
void SV_SaveSpawnparms ();
void SV_SpawnServer (char *server);

