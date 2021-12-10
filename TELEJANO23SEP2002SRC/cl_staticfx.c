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


// Generated from scrach by Tei

#define SFX_EXPLOFIRE	1
#define SFX_FIRE		2
#define SFX_SMOKE		3
#define SFX_EXPLOWAVE	4
#define SFX_ZRINGS		5
#define SFX_LIGHT		6
#define SFX_BLACKSNOKESRC 7
#define SFX_XPLOX		8
#define SFX_XPLOX2		9
#define SFX_SPARK		10
#define SFX_RAY			11


#define LF_FOREVER		1
#define LF_WILLDIE		2
#define LF_SCALE		3
#define LF_SCALEFAST	4
#define LF_ONLYONCE		5


typedef struct
{
	int		type; // entity number
	int		typelife;
	float   scale;
	float	dead;
	vec3_t	origin;
	vec3_t	endpos;
	qboolean present;
	int     key;
} prj_t;


#define MAX_PRJ  340  // Tei.. low becuase is underused

int			cl_prj_num = 0;
prj_t		cl_prj[MAX_PRJ];

/////////////
//
//  FX
//

void R_FireExplo (vec3_t  origin, float scale);

void R_FireExploBlack (vec3_t  origin, float scale);

void R_FireFlama (vec3_t  origin, float scale);
void R_ExploRing (vec3_t  origin, float scale);
void R_ZRing (vec3_t  origin, float scale);
void CL_AddLight( vec3_t pos, float scale );


///////////
//
//  Parsing the PRJ
//

void CL_ParsePRJ ()
{
	vec3_t	start, end;
	prj_t * ent;
	int		i;
	int		type;
	int		typelife;
	float	scale;
	float	dead;
	int		key;
	int		proto;

	/* Reading the net cfg data */

	proto = MSG_ReadByte ();  

	if (proto != 1 )	// Only support for proto=1
	{
		Sys_Error ("StaticFX proto unsuported version %d \n", proto);
		return;
	}


	start[0] = MSG_ReadCoord (); // Origin
	start[1] = MSG_ReadCoord ();
	start[2] = MSG_ReadCoord ();
	
	end[0] = MSG_ReadCoord ();	 // Origin2...	
	end[1] = MSG_ReadCoord ();
	end[2] = MSG_ReadCoord ();

	type		= MSG_ReadByte ();  // Fire, smoke, magic, rays.. etc..
	typelife	= MSG_ReadByte ();  // Temporal, static, etc..
	dead		= MSG_ReadByte ();	// cl.time + deadclock dead (if type need)
	scale		= MSG_ReadByte () * 0.01; //   
	key			= MSG_ReadByte ();	// Key for delete and change


	/* Search for a free slot. */
	i = 0;
	while ( cl_prj[i].present && i<MAX_PRJ )
			i++;

	if ( cl_prj[i].present || i>=MAX_PRJ)
	{
		// TODO: write a subst code
		Con_Printf ("OVERFLOW: PRJ flood\n");	
		return;
	}
	
	/*        Cfg the slot.     */
	ent = &cl_prj[i];
	memset (ent, 0, sizeof(*ent));

	ent->origin[0] = start[0]; 
	ent->origin[1] = start[1];
	ent->origin[2] = start[2];

	ent->endpos[0] = end[0];
	ent->endpos[1] = end[1];
	ent->endpos[2] = end[2];

	ent->scale		= scale;
	ent->typelife	= typelife;
	ent->type		= type;
	ent->key		= key; 
	ent->dead		= cl.time + dead;
	
	//Con_Printf ("PRJ %f time, %f dead\n",cl.time, ent->dead);

	ent->present = true;


	//Con_Printf ("PRJ generated\n");	
	return ;
}



void DefPRJ (	vec3_t  start,vec3_t  end, int type, int typelife, int dead, int scale, int key )
{
	prj_t * ent;
	int		i;
	//int		type;
	//int		typelife;
	//float	scale;
	//float	dead;
	//int		key;
//	int		proto;

	/* Reading the net cfg data */

	/* Search for a free slot. */
	i = 0;
	while ( cl_prj[i].present && i<MAX_PRJ )
			i++;

	if ( cl_prj[i].present || i>=MAX_PRJ)
	{
		// TODO: write a subst code
		Con_Printf ("OVERFLOW: PRJ flood\n");	
		return;
	}
	
	/*        Cfg the slot.     */
	ent = &cl_prj[i];
	memset (ent, 0, sizeof(*ent));

	ent->origin[0] = start[0]; 
	ent->origin[1] = start[1];
	ent->origin[2] = start[2];

	ent->endpos[0] = end[0];
	ent->endpos[1] = end[1];
	ent->endpos[2] = end[2];

	ent->scale		= scale;
	ent->typelife	= typelife;
	ent->type		= type;
	ent->key		= key; 
	ent->dead		= cl.time + dead;
	
	//Con_Printf ("PRJ %f time, %f dead\n",cl.time, ent->dead);

	ent->present = true;


	//Con_Printf ("PRJ generated\n");	
	return ;
}

void CL_ClearPRJ() 
{
	memset (cl_prj, 0, sizeof(cl_prj));
}



//////////////
//
//   Running the prj
//

extern cvar_t r_staticfx;

void R_DrawCorona(vec3_t  pos, float radius , int visible );//Tei
void R_WRing (vec3_t  origin, float scale);//Tei
void R_ParticleTeExplosion (vec3_t origin);
void R_RayFieldParticles (vec3_t org);
void R_ParticleImpSuperSpike (vec3_t origin);
void XR_BlobExplosion3 (vec3_t org);
void R_BeamZing (vec3_t origin, vec3_t end);
void R_SuperZing (vec3_t start, vec3_t end);


void CL_RunPRJ()
{
	int i,k,t;
	prj_t		*prj;
	float deltat, x;
	vec3_t downmove;

	/* Static FX on/off */
	if (!r_staticfx.value)
		return;

	for (i=0 ; i< MAX_PRJ  ; i++)
	{
		prj = &cl_prj[i];

		if ( prj->present ) 
		{
			//Con_Printf ("PRJ:%d\n", prj->key);

			/* Type work */

			switch ( prj->type )
			{
				case SFX_EXPLOFIRE:
					R_FireExplo (prj->origin, prj->scale);

					break;
				case SFX_FIRE:
					R_FireFlama (prj->origin, prj->scale);					

					break;
				case SFX_EXPLOWAVE:
					R_ExploRing (prj->origin, prj->scale);
					break;
				case SFX_ZRINGS:
					R_ZRing (prj->origin, prj->scale);
					break;
				case SFX_LIGHT:
					//CL_AddLight( prj->origin, prj->scale );
					//R_RailTrail (prj->origin, prj->endpos, prj->endpos);
					break;
				case SFX_BLACKSNOKESRC:
					R_FireExploBlack (prj->origin, prj->scale);
					break;
				case SFX_XPLOX:
					//R_FireExploBlack (prj->origin, prj->scale);
					if (rand()&1)
						R_ParticleTeExplosion (prj->origin);
					break;
				case SFX_XPLOX2:
					//R_FireExploBlack (prj->origin, prj->scale);
					if (lhrandom(1,110)>90)
					//	R_ParticleImpSuperSpike (prj->origin);
					//else
						R_RayFieldParticles(prj->origin);

					if (lhrandom(1,1000)>990)
						R_ParticleImpSuperSpike (prj->origin);
					
					if (teirand(1000)>998)
							XR_BlobExplosion3(prj->origin);//Tei QMB!

					break;
				
				case SFX_SPARK:
					if (lhrandom(1,1000)>980)
						R_SparkShower (prj->origin, vec3_origin );


					break;
				case SFX_RAY:

					for (t=0; t<3;t++)
					{
					
					VectorCopy(prj->origin,downmove);
					k = lhrandom(10,100 * prj->scale);
					downmove[0] += lhrandom(-k,k);
					downmove[1] += lhrandom(-k,k);
					downmove[2] += lhrandom(-k,k);
					R_BeamZing(prj->origin, downmove);
					if(teirand(100)<2)
						R_SuperZing(prj->origin, downmove);
					}

					break;
				default:
					break;
			}

			/* Life work */

			switch ( prj->typelife )
			{
				case LF_WILLDIE:
					if (prj->dead < cl.time )
					{
						//Con_Printf ("PRJ:deleted!\n");
						prj->present = false;
					}
					break;
				case LF_SCALE:
					deltat = fabs(cl.time - prj->dead);
					x = ( prj->scale ) / (deltat + 0.001f);
					prj->scale -= x;
					
					if (prj->scale < 0.1f) 
					{
						prj->present = false;
						break;
					}
					break;
				case LF_SCALEFAST:
					prj->scale -= 0.1f;
				
					if (prj->scale < 0.1f) 
					{
						prj->present = false;
						break;
					}
					break;
				case LF_ONLYONCE:
					prj->present = false;
					break;

				case LF_FOREVER:
				default:
					break;		
			}

			/* Continue */
		}
	}
}
