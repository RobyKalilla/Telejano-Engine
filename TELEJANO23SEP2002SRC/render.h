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

// refresh.h -- public interface to refresh functions
#define	TOP_RANGE		16			// soldier uniform colors
#define	BOTTOM_RANGE	96

//=============================================================================

typedef struct efrag_s
{
	struct mleaf_s		*leaf;
	struct efrag_s		*leafnext;
	struct entity_s		*entity;
	struct efrag_s		*entnext;
} efrag_t;


typedef struct entity_s
{
	qboolean				forcelink;		// model changed

	int						update_type;

	entity_state_t			baseline;		// to fill in defaults in updates

	double					msgtime;		// time of last update
	vec3_t					msg_origins[2];	// last two updates (0 is newest)	
	vec3_t					origin;
	vec3_t					msg_angles[2];	// last two updates (0 is newest)
	vec3_t					angles;

	// Tomaz - Quake2 Models Begin
	int						draw_lastpose, draw_pose; 	// for interpolation
	float					draw_lerpstart; 			// for interpolation
	struct model_s			*draw_lastmodel; 			// for interpolation
	// Tomaz - Quake2 Models End

	struct model_s			*model;			// NULL = no model
	struct efrag_s			*efrag;			// linked list of efrags
	int						frame;
	float					syncbase;		// for client-side animations
	byte					*colormap;
	int						effects;		// light, particals, etc
	int						skinnum;		// for Alias models
	int						visframe;		// last frame this entity was
											//  found in an active leaf
											
	int						dlightframe;	// dynamic lighting
	int						dlightbits;
	
// FIXME: could turn these into a union
	int						trivial_accept;
	struct mnode_s			*topnode;		// for bmodels, first world node
											//  that splits bmodel, or NULL if
											//  not split

	// Tomaz - QC Alpha Scale Glow Begin
	qboolean				solid;

	float					alpha;
	float					scale;
	float					glow_size;
	float					glow_red;
	float					glow_green;
	float					glow_blue;
	// Tomaz - QC Alpha Scale Glow End

	int						contents;

	// Tomaz-  Model Animation Interpolation Begin
	float					frame_start_time;
	float					frame_interval;
	int						pose1; 
	int						pose2;
	// Tomaz-  Model Animation Interpolation End

	// Tomaz-  Model Transform Interpolation Begin
	float					translate_start_time;
	vec3_t					origin1;
	vec3_t					origin2;
	float					rotate_start_time;
	vec3_t					angles1;
	vec3_t					angles2;
	// Tomaz-  Model Transform Interpolation End

	float					time_left;
	float					debris_smoke;

	float					last_light[3]; // Tomaz X

	int					effects2;// Tei - more fx
	int					effects3;// Tei - and more fx
	vec3_t				finalpos;// Tei - prj
	//vec3_t				info_velocity;// Tei obj speed
	vec3_t				oldorg; //Tei fire trails

	int					stepdone;//Tei frame with stepsound sone

#if 1 //Tei lh trace


	
	// calculated during R_AddModelEntities
	vec3_t mins, maxs;

#endif 

} entity_t;

typedef struct
{
	vrect_t		vrect;				// subwindow in video for refresh
									// FIXME: not need vrect next field here?
	vrect_t		aliasvrect;			// scaled Alias version
	int			vrectright, vrectbottom;	// right & bottom screen coords
	int			aliasvrectright, aliasvrectbottom;	// scaled Alias versions
	float		vrectrightedge;			// rightmost right edge we care about,
										//  for use in edge list
	float		fvrectx, fvrecty;		// for floating-point compares
	float		fvrectx_adj, fvrecty_adj; // left and top edges, for clamping
	int			vrect_x_adj_shift20;	// (vrect.x + 0.5 - epsilon) << 20
	int			vrectright_adj_shift20;	// (vrectright + 0.5 - epsilon) << 20
	float		fvrectright_adj, fvrectbottom_adj;
										// right and bottom edges, for clamping
	float		fvrectright;			// rightmost edge, for Alias clamping
	float		fvrectbottom;			// bottommost edge, for Alias clamping
	float		horizontalFieldOfView;	// at Z = 1.0, this many X is visible 
										// 2.0 = 90 degrees
	float		xOrigin;			// should probably allways be 0.5
	float		yOrigin;			// between be around 0.3 to 0.5

	vec3_t		vieworg;
	vec3_t		viewangles;
	
	float		fov_x, fov_y;

	int			ambientlight;
} refdef_t;


//
// refresh
//
extern	refdef_t	r_refdef;
extern vec3_t	r_origin, vpn, vright, vup;

extern	struct texture_s	*r_notexture_mip;


void R_Init (void);
void R_InitTextures (void);
void R_InitEfrags (void);
void R_RenderView (void);		// must set r_refdef first
void R_ViewChanged (vrect_t *pvrect, int lineadj, float aspect);
								// called whenever r_refdef or vid change
void R_InitSky (byte *src, int bytesperpixel);	// called at level load

void R_AddEfrags (entity_t *ent);
void R_RemoveEfrags (entity_t *ent);

void R_NewMap (void);

void GL_Set2D (void);
void Draw_MapShots(void);
void Draw_Crosshair (int num);

byte *loadimagepixels (char* filename, qboolean complain);
int  loadtextureimage (char* filename, qboolean complain, qboolean mipmap);
int  loadtextureimage3 (char* filename, qboolean complain, qboolean mipmap, byte *data);


void R_ParseParticleEffect (void);
void R_RunParticleEffect (vec3_t org, vec3_t dir, int color, int count);
void R_RailTrail (vec3_t start, vec3_t end, vec3_t angle);
void R_RocketTrail (vec3_t start, vec3_t end, entity_t *ent);
void R_BloodTrail (vec3_t start, vec3_t end, entity_t *ent);
void R_TracerTrail (vec3_t start, vec3_t end, entity_t *ent, byte color);
void R_VoorTrail (vec3_t start, vec3_t end, entity_t *ent);
void R_Snow (vec3_t min, vec3_t max, int flakes);
void R_SparkShower (vec3_t origin, vec3_t direction);
void R_Rain (vec3_t min, vec3_t max, int drops);
void R_Fire (entity_t *ent, qboolean fire2);
void R_EntityParticles (entity_t *ent);
void R_ParticleExplosion (vec3_t org);
void R_ParticleExplosion2 (vec3_t origin, int colorStart, int colorLength);
void R_LavaSplash (vec3_t org);
void R_TeleportSplash (vec3_t org);

void R_PushDlights (void);

// Tei fire
void R_FireBlue (entity_t *ent, qboolean fire2);
void R_BigFire (entity_t *ent, qboolean fire2);
void R_DowFire (entity_t *ent, qboolean fire2);
void R_FogSplash (vec3_t origin);
void R_FogSplashLite (vec3_t origin);
void R_FireClassic (entity_t *ent, qboolean fire2);
void R_WaterFall (entity_t *ent, qboolean fire2);
void R_CustomFX (vec3_t origin,vec3_t color,vec3_t veloc , int num, int particle, int bounce, int scale, int die, int type );
void R_DarkFieldParticles (entity_t *ent);
// Tei fire


// LordHavoc: was a major time waster
//#define R_CullBox(mins,maxs) (frustum[0].BoxOnPlaneSideFunc(mins, maxs, &frustum[0]) == 2 || frustum[1].BoxOnPlaneSideFunc(mins, maxs, &frustum[1]) == 2 || frustum[2].BoxOnPlaneSideFunc(mins, maxs, &frustum[2]) == 2 || frustum[3].BoxOnPlaneSideFunc(mins, maxs, &frustum[3]) == 2)
//#define R_NotCulledBox(mins,maxs) (frustum[0].BoxOnPlaneSideFunc(mins, maxs, &frustum[0]) != 2 && frustum[1].BoxOnPlaneSideFunc(mins, maxs, &frustum[1]) != 2 && frustum[2].BoxOnPlaneSideFunc(mins, maxs, &frustum[2]) != 2 && frustum[3].BoxOnPlaneSideFunc(mins, maxs, &frustum[3]) != 2)
// LordHavoc: was a major time waster
