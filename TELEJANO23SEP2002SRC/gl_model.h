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

#ifndef __MODEL__
#define __MODEL__

#include "modelgen.h"
#include "spritegn.h"


/*

d*_t structures are on-disk representations
m*_t structures are in-memory

*/

// entity effects

#define	EF_BRIGHTFIELD			1
#define	EF_MUZZLEFLASH 			2
#define	EF_BRIGHTLIGHT 			4
#define	EF_DIMLIGHT 			8


/*
==============================================================================

BRUSH MODELS

==============================================================================
*/


//
// in memory representation
//
typedef struct
{
	vec3_t		position;
} mvertex_t;

#define	SIDE_FRONT	0
#define	SIDE_BACK	1
#define	SIDE_ON		2


// plane_t structure
typedef struct mplane_s
{
	vec3_t	normal;
	float	dist;
	byte	type;			// for texture axis selection and fast side tests
	byte	signbits;		// signx + signy<<1 + signz<<1
	byte	pad[2];
} mplane_t;

typedef struct texture_s
{
	char				name[16];
	unsigned			width, height;
	int					gl_texturenum;
	int					anim_total;			// total tenths in sequence ( 0 = no)
	int					anim_min, anim_max;	// time for this frame min <=time< max
	struct texture_s	*anim_next;			// in the animation sequence
	struct texture_s	*alternate_anims;	// bmodels in frmae 1 use these
	unsigned			offsets[MIPLEVELS];	// four mip maps stored

	int					fullbrights;		// Tomaz - Fullbrights
	int					transparent;		// Tomaz - HL Bsp's
	int					rs;					// Tomaz - Rscripts

	// Tei - surface particle fx

		/* force fx */
	
	qboolean			iswater;		// 
	qboolean			islava;			//
	qboolean			istele;			// 
	qboolean			issky;			// 

		/* force fx */
	int					fwater;		// 
	int					flava;			// 
	int					ftele;			// 
	int					flight;			// 
	int					frain;
	int					fsnow;
	int					fnodraw;		// effect

		/* color fx */
	int					wfx_red;
	int					wfx_green;
	int					wfx_blue;
	
	// Tei - surface particle fx

} texture_t;

#define	SURF_PLANEBACK		2
#define	SURF_DRAWSKY		4
#define SURF_DRAWSPRITE		8
#define SURF_DRAWTURB		16
#define SURF_DRAWTILED		32
#define SURF_DRAWBACKGROUND	64
#define SURF_UNDERWATER		128
#define SURF_ENVMAP			1024	// Tomaz - Enviroment Mapping

//QMB!
#define SURF_SHINY_GLASS	256 //JHL:ADD; New flag for glass effect
#define SURF_SHINY_METAL	512
//QMB!


//Reuse of number for grass
#define SURF_GRASS	256 //JHL:ADD; New flag for glass effect



typedef struct
{
	unsigned short	v[2];
	unsigned int	cachededgeoffset;
} medge_t;

typedef struct
{
	float		vecs[2][4];
	float		mipadjust;
	texture_t	*texture;
	int			flags;
} mtexinfo_t;

#define	VERTEXSIZE	7

typedef struct glpoly_s
{
	struct	glpoly_s	*next;
	struct	glpoly_s	*chain;
	int		numverts;
	int		flags;			// for SURF_UNDERWATER
	float	verts[4][VERTEXSIZE];	// variable sized (xyz s1t1 s2t2)
} glpoly_t;

typedef struct msurface_s
{
	int			visframe;		// should be drawn when node is crossed

	mplane_t	*plane;
	int			flags;

	int			firstedge;	// look up in model->surfedges[], negative numbers
	int			numedges;	// are backwards edges
	
	short		texturemins[2];
	short		extents[2];

	int			light_s, light_t;	// gl lightmap coordinates

	glpoly_t	*polys;				// multiple if warped
	struct	msurface_s	*texturechain;

	mtexinfo_t	*texinfo;
	
// lighting info
	int			dlightframe, lightframe;
	int			dlightbits;

	int			lightmaptexturenum;
	byte		styles[MAXLIGHTMAPS];
	int			cached_light[MAXLIGHTMAPS];	// values currently used in lightmap
	qboolean	cached_dlight;				// true if dynamic light in cache
	byte		*samples;		// [numstyles*surfsize]

	// stain to apply on lightmap (soot/dirt/blood/whatever) //LH!
	byte		*stainsamples;

} msurface_t;

typedef struct mnode_s
{
// common with leaf
	int			contents;		// 0, to differentiate from leafs
	int			visframe;		// node needs to be traversed if current
	
	vec3_t		mins;			// for bounding box culling
	vec3_t		maxs;			// for bounding box culling

	struct mnode_s	*parent;

// node specific
	mplane_t	*plane;
	struct mnode_s	*children[2];	

	unsigned short		firstsurface;
	unsigned short		numsurfaces;
} mnode_t;



typedef struct mleaf_s
{
// common with node
	int			contents;		// wil be a negative contents number
	int			visframe;		// node needs to be traversed if current

	vec3_t		mins;			// for bounding box culling
	vec3_t		maxs;			// for bounding box culling

	struct mnode_s	*parent;

// leaf specific
	byte		*compressed_vis;
	efrag_t		*efrags;

	msurface_t	**firstmarksurface;
	int			nummarksurfaces;
	int			key;			// BSP sequence number for leaf's contents
	byte		ambient_sound_level[NUM_AMBIENTS];
} mleaf_t;

typedef struct
{
	dclipnode_t	*clipnodes;
	mplane_t	*planes;
	int			firstclipnode;
	int			lastclipnode;
	vec3_t		clip_mins;
	vec3_t		clip_maxs;
	vec3_t		clip_size;//Tei for lh trace
} hull_t;

/*
==============================================================================

SPRITE MODELS

==============================================================================
*/


// FIXME: shorten these?

/*
==============================================================================

ALIAS MODELS

Alias models are position independent, so the cache manager can move them.
==============================================================================
*/

typedef struct
{
	int					firstpose;
	int					numposes;
	float				interval;
	trivertx_t			bboxmin;
	trivertx_t			bboxmax;
	int					frame;
	char				name[16];
} maliasframedesc_t;

typedef struct
{
	trivertx_t			bboxmin;
	trivertx_t			bboxmax;
	int					frame;
} maliasgroupframedesc_t;

typedef struct
{
	int						numframes;
	int						intervals;
	maliasgroupframedesc_t	frames[1];
} maliasgroup_t;

typedef struct mtriangle_s {
	int					facesfront;
	int					vertindex[3];
} mtriangle_t;


#define	MAX_SKINS	1024
typedef struct 
{
	int			transparent;
	int			ident;
	int			version;
	vec3_t		scale;
	vec3_t		scale_origin;
	float		boundingradius;
	vec3_t		eyeposition;
	int			numskins;
	int			skinwidth;
	int			skinheight;
	int			numverts;
	int			numtris;
	int			numframes;
	synctype_t	synctype;
	int			flags;
	float		size;

	int					numposes;
	int					poseverts;
	int					posedata;	// numposes*poseverts trivert_t
	int					commands;	// gl command list with embedded s/t
	int					gl_texturenum[MAX_SKINS][4];
	int					fb_texturenum[MAX_SKINS][4];
	int					texels[MAX_SKINS];	// only for player skins
	maliasframedesc_t	frames[1];	// variable sized
} aliashdr_t;

#define	MAXALIASVERTS	4096
#define	MAXALIASFRAMES	2048
#define	MAXALIASTRIS	8192
extern	aliashdr_t	*pheader;
extern	stvert_t	stverts[MAXALIASVERTS];
extern	mtriangle_t	triangles[MAXALIASTRIS];
extern	trivertx_t	*poseverts[MAXALIASFRAMES];

// Tomaz - Quake2 Models Begin
/*
========================================================================
.MD2 triangle model file format
========================================================================
*/

#define MD2IDALIASHEADER	(('2'<<24)+('P'<<16)+('D'<<8)+'I')
#define MD2ALIAS_VERSION	8

#define	MD2MAX_TRIANGLES	4096
#define MD2MAX_VERTS		2048
#define MD2MAX_FRAMES		1024 
#define MD2MAX_SKINS		32
#define	MD2MAX_SKINNAME		64
// sanity checking size
#define MD2MAX_SIZE	(1024*4200)

typedef struct
{
	short	s;
	short	t;
} md2stvert_t;

typedef struct 
{
	short	index_xyz[3];
	short	index_st[3];
} md2triangle_t;

typedef struct
{
	byte	v[3];			// scaled byte to fit in frame mins/maxs
	byte	lightnormalindex;
} md2trivertx_t;

#define MD2TRIVERTX_V0   0
#define MD2TRIVERTX_V1   1
#define MD2TRIVERTX_V2   2
#define MD2TRIVERTX_LNI  3
#define MD2TRIVERTX_SIZE 4

typedef struct
{
	float		scale[3];	// multiply byte verts by this
	float		translate[3];	// then add this
	char		name[16];	// frame name from grabbing
	md2trivertx_t	verts[1];	// variable sized
} md2frame_t;


// the glcmd format:
// a positive integer starts a tristrip command, followed by that many
// vertex structures.
// a negative integer starts a trifan command, followed by -x vertexes
// a zero indicates the end of the command list.
// a vertex consists of a floating point s, a floating point t,
// and an integer vertex index.


typedef struct
{
	int			ident;
	int			version;

	int			skinwidth;
	int			skinheight;
	int			framesize;		// byte size of each frame

	int			num_skins;
	int			num_xyz;
	int			num_st;			// greater than num_xyz for seams
	int			num_tris;
	int			num_glcmds;		// dwords in strip/fan command list
	int			num_frames;

	int			ofs_skins;		// each skin is a MAX_SKINNAME string
	int			ofs_st;			// byte offset from start for stverts
	int			ofs_tris;		// offset for dtriangles
	int			ofs_frames;		// offset for first frame
	int			ofs_glcmds;	
	int			ofs_end;		// end of file

	int			gl_texturenum[MAX_SKINS];
//	int			fb_texturenum[MAX_SKINS];
} md2_t;

#define ALIASTYPE_MDL 1
#define ALIASTYPE_MD2 2
// Tomaz - Quake2 Models End

//===================================================================

//
// Whole model
//

typedef enum {mod_brush, mod_sprite, mod_alias, mod_hud } modtype_t;

#define	EF_ROCKET	1			// leave a trail
#define	EF_GRENADE	2			// leave a trail
#define	EF_GIB		4			// leave a trail
#define	EF_ROTATE	8			// rotate (bonus items)
#define	EF_TRACER	16			// green split trail
#define	EF_ZOMGIB	32			// small blood trail
#define	EF_TRACER2	64			// orange split trail + rotate
#define	EF_TRACER3	128			// purple trail

#define MFX_NONE		0
#define MFX_MISSILE		1
#define MFX_FIRELAMP	2
#define MFX_FIRE		3 
#define MFX_FIRE2		4 
#define MFX_NODRAW		5
#define MFX_BLUEFIRE	6
#define MFX_BLUEFIRE2	7
#define MFX_DOWFIRE     8
#define MFX_ENGINEFIRE2 9  
#define MFX_BIGFIRE     10 
#define MFX_FOGMAKER	11
#define MFX_FOGMAKERLITE	12
#define MFX_WATERFALL	13
#define MFX_SPARKSHOWER 14
#define MFX_GLOW_		15
#define MFX_ALIENBLOOD  16
#define MFX_BOLTFX		17
#define MFX_SUN			18
#define MFX_LASER		19
#define MFX_SPIKE		20
#define MFX_LUX			21
#define MFX_GLOW		22
#define MFX_GIB			23
#define MFX_GRASS		24
#define MFX_LUX2		25
#define MFX_LASERBEAM	26
#define MFX_FOOTSOUND	27


typedef struct model_s
{
	char		name[MAX_QPATH];
	qboolean	needload;		// bmodels and sprites don't cache normally

	modtype_t	type;
	int			aliastype;		// Tomaz - Quake2 Models
	int			numframes;
	synctype_t	synctype;
	
	// Tomaz 148
	float		glow_radius;
	vec3_t		glow_color;
	qboolean	noshadow;
	qboolean	fullbright;
	qboolean	visible;
	// Tomaz 148
	int			flags;

//
// volume occupied by the model graphics
//		
	vec3_t		mins, maxs;
	float		radius;

// Tei dpx flares and others..
	int			effect;
	int			dpxflare;
// Tei dpx flares

// Tei isfx_
	qboolean	isfx_;
// Tei isfx_

// Tei mscript
			qboolean use_FIRE;
			qboolean use_FIRE2;
			qboolean use_MISSILE;
			qboolean use_FIRELAMP;
			qboolean use_BLUEFIRE2;
			qboolean use_BLUEFIRE;
			qboolean use_DOWFIRE;
			qboolean use_ENGINEFIRE2;
			qboolean use_BIGFIRE;
			qboolean use_FOGMAKER;
			qboolean use_FOGMAKERLITE;
			qboolean use_WATERFALL;
			qboolean use_SPARKSHOWER;
			qboolean use_ALIENBLOOD;
			qboolean use_BOLTFX;
			qboolean use_SUN;
			qboolean use_LASER;		
			qboolean use_SPIKE;
			qboolean use_GLOW;
			qboolean use_LUX;
			qboolean use_GIB;
			qboolean use_GRASS;
			qboolean use_GRASS2;
			qboolean use_LUX2;		
			qboolean use_LASERBEAM;
			qboolean use_FOOTSOUND;
// Tei mscript

	int hud;//Tei hud texture
	qpic_t * pic; //Tei hud

#if 1 //Tei for lh traces

	qboolean ishlbsp;

	// volume occupied by the model
	// bounding box at angles '0 0 0'
	vec3_t			normalmins, normalmaxs;
	// bounding box if yaw angle is not 0, but pitch and roll are
	vec3_t			yawmins, yawmaxs;
	// bounding box if pitch or roll are used
	vec3_t			rotatedmins, rotatedmaxs;

#endif 



//
// solid volume for clipping 
//

#if 0 //Tei remarcked because is unused
	//qboolean	clipbox;
	//vec3_t		clipmins, clipmaxs;
#endif 


//
// brush model
//
	int			firstmodelsurface, nummodelsurfaces;

	int			numsubmodels;
	dmodel_t	*submodels;

	int			numplanes;
	mplane_t	*planes;

	int			numleafs;		// number of visible leafs, not counting 0
	mleaf_t		*leafs;

	int			numvertexes;
	mvertex_t	*vertexes;

	int			numedges;
	medge_t		*edges;

	int			numnodes;
	mnode_t		*nodes;

	int			numtexinfo;
	mtexinfo_t	*texinfo;

	int			numsurfaces;
	msurface_t	*surfaces;

	int			numsurfedges;
	int			*surfedges;

	int			numclipnodes;
	dclipnode_t	*clipnodes;

	int			nummarksurfaces;
	msurface_t	**marksurfaces;

	hull_t		hulls[MAX_MAP_HULLS];

	int			numtextures;
	texture_t	**textures;

	byte		*visdata;
	byte		*lightdata;
	char		*entities;

//
// additional model data
//
	cache_user_t	cache;		// only access through Mod_Extradata

} model_t;

//============================================================================

void	Mod_Init (void);
void	Mod_ClearAll (void);
model_t *Mod_ForName (char *name, qboolean crash);
void	*Mod_Extradata (model_t *mod);	// handles caching
void	Mod_TouchModel (char *name);

mleaf_t *Mod_PointInLeaf (float *p, model_t *model);
byte	*Mod_LeafPVS (mleaf_t *leaf, model_t *model);

qboolean Has_Fullbrights (byte *data, int size);
qboolean Has_Fullbrights2 (byte *data, int size);

#endif	// __MODEL__
