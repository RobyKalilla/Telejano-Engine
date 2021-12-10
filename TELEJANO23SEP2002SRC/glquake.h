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
// disable data conversion warnings

#pragma warning(disable : 4244)     // MIPS
#pragma warning(disable : 4136)     // X86
#pragma warning(disable : 4051)     // ALPHA
  
#include <windows.h>
#include <GL/gl.h>

void GL_BeginRendering (int *x, int *y, int *width, int *height);
void GL_EndRendering (void);

// Function prototypes for the Texture Object Extension routines
typedef GLboolean (APIENTRY *ARETEXRESFUNCPTR)(GLsizei, const GLuint *,
                    const GLboolean *);
typedef void (APIENTRY *BINDTEXFUNCPTR)(GLenum, GLuint);
typedef void (APIENTRY *DELTEXFUNCPTR)(GLsizei, const GLuint *);
typedef void (APIENTRY *GENTEXFUNCPTR)(GLsizei, GLuint *);
typedef GLboolean (APIENTRY *ISTEXFUNCPTR)(GLuint);
typedef void (APIENTRY *PRIORTEXFUNCPTR)(GLsizei, const GLuint *,
                    const GLclampf *);
typedef void (APIENTRY *TEXSUBIMAGEPTR)(int, int, int, int, int, int, int, int, void *);

extern	BINDTEXFUNCPTR bindTexFunc;
extern	DELTEXFUNCPTR delTexFunc;
extern	TEXSUBIMAGEPTR TexSubImage2DFunc;

extern	int texture_extension_number;
extern	int		texture_mode;
extern	float	gldepthmin, gldepthmax;

void GL_Upload32 (unsigned *data, int width, int height,  qboolean mipmap, qboolean alpha);
void GL_Upload8 (byte *data, int width, int height,  qboolean mipmap, qboolean alpha);
int GL_LoadTexture (char *identifier, int width, int height, byte *data, qboolean mipmap, qboolean alpha, int bytesperpixel);	// Tomaz - Quake2 Models

typedef struct
{
	float	x, y, z;
	float	s, t;
	float	r, g, b;
} glvert_t;

extern glvert_t glv;

extern	int glx, gly, glwidth, glheight;

extern	PROC glArrayElementEXT;
extern	PROC glColorPointerEXT;
extern	PROC glTexturePointerEXT;
extern	PROC glVertexPointerEXT;

// r_local.h -- private refresh defs

#define ALIAS_BASE_SIZE_RATIO	0.0909090909
#define	MAX_LBM_HEIGHT			1024
#define BACKFACE_EPSILON		0.01

void R_TimeRefresh_f (void);
void R_ReadPointFile_f (void);
texture_t *R_TextureAnimation (texture_t *base);

//====================================================


extern	entity_t	r_worldentity;
extern	vec3_t		modelorg;
extern	entity_t	*currententity;
extern	int			r_visframecount;	// ??? what difs?
extern	int			r_framecount;
extern	mplane_t	frustum[4];
extern	int			c_brush_polys, c_alias_polys;


//
// view origin
//
extern	vec3_t	vup;
extern	vec3_t	vpn;
extern	vec3_t	vright;
extern	vec3_t	r_origin;

//
// screen size info
//
extern	refdef_t	r_refdef;
extern	mleaf_t		*r_viewleaf, *r_oldviewleaf;
extern	texture_t	*r_notexture_mip;
extern	int		d_lightstylevalue[256];	// 8.8 fraction of base light value

extern	int	playertextures;

extern	cvar_t	r_norefresh;
extern	cvar_t	r_drawviewmodel;
extern	cvar_t	r_speeds;
extern	cvar_t	r_shadows;
extern	cvar_t	r_wateralpha;
extern	cvar_t	r_dynamic;
extern	cvar_t	r_novis;

extern	cvar_t	gl_clear;
extern	cvar_t	gl_poly;
extern	cvar_t	gl_keeptjunctions;
extern	cvar_t	gl_nocolors;
extern	cvar_t	gl_doubleeyes;

extern	int		gl_solid_format;
extern	int		gl_alpha_format;

extern	cvar_t	gl_max_size;

// Tomaz - Model Interpolation Begin
//tern	cvar_t	r_int_model_anim;
//tern	cvar_t	r_int_model_trans;
// Tomaz - Model Interpolation End

extern	cvar_t	slowmo;		// Tomaz - Slowmo

// Tomaz - Fog Begin
extern  cvar_t  gl_fogenable; 
extern  cvar_t  gl_fogstart;
extern  cvar_t  gl_fogend; 
extern  cvar_t  gl_fogred; 
extern  cvar_t  gl_fogblue; 
extern  cvar_t  gl_foggreen; 
// Tomaz - Fog End

extern  cvar_t  centerfade;		// Tomaz - Fading CenterPrints
extern  cvar_t  sbar_alpha;		// Tomaz - Sbar Alpha
extern  cvar_t  con_alpha;		// Tomaz - Console Alpha
extern  cvar_t  r_wave;			// Tomaz - Water Wave
extern  cvar_t  gl_glows;		// Tomaz - Glow
extern  cvar_t  r_bobbing;		// Tomaz - Bobbing Items
extern  cvar_t  gl_envmap;		// Tomaz - Enviroment Mapping
extern  cvar_t  gl_caustics;	// Tomaz - Underwater Caustics
extern  cvar_t  gl_geocaustics;	// Tomaz - Underwater Caustics

extern  cvar_t  gl_fbr;			// Tomaz - Fullbrights
extern  cvar_t  impaim;			// Tomaz - Improved Aiming
extern  cvar_t  show_fps;		// Tomaz - FPS Counter
extern  cvar_t  skybox_spin;	// Tomaz - Spinning Skyboxes
extern  cvar_t  mapshots;		// Tomaz - MapShots
extern  cvar_t  gl_showpolys;	// Tomaz - Show BSP Polygons
extern  cvar_t  gl_wireframe;	// Tomaz - Draw World as Wireframe and Textures
extern  cvar_t  gl_wireonly;	// Tomaz - Draw World as Wireframe Only
extern  cvar_t  gl_particles;	// Tomaz - Particles
extern  cvar_t  print_center_to_console;	// Tomaz - Prints CenterString's to the console
extern  cvar_t  gl_particle_fire;	// Tomaz - Fire Particles
extern  cvar_t  watermap;			// Tei - watermap
extern  cvar_t  fpscroak;			// Tei - watermap
extern  cvar_t  r_dosky;			// Tei - watermap
extern  cvar_t  gl_cull;			// Q2!

extern	float	r_world_matrix[16];

extern	const char *gl_vendor;
extern	const char *gl_renderer;
extern	const char *gl_version;
extern	const char *gl_extensions;

void R_TranslatePlayerSkin (int playernum);

// Multitexture
#define		TEXTURE0_SGIS		0x835E
#define		TEXTURE1_SGIS		0x835F

#define		TEXTURE0_ARB		0x84C0
#define		TEXTURE1_ARB		0x84C1

extern GLenum TEXTURE0_SGIS_ARB;
extern GLenum TEXTURE1_SGIS_ARB;

typedef void (APIENTRY *lpMTexFUNC) (GLenum, GLfloat, GLfloat);
typedef void (APIENTRY *lpSelTexFUNC) (GLenum);

extern lpMTexFUNC qglMTexCoord2fSGIS_ARB;
extern lpSelTexFUNC qglSelectTextureSGIS_ARB;

void EmitWaterPolys		(msurface_t *s);
void EmitSkyPolys		(msurface_t *s);
void EmitEnvMapPolys	(msurface_t *s);
void EmitCausticsPolys	(msurface_t *s);

void Print(int x, int y, const char *string, ...);

void R_DrawSkyBox (void);
void R_DrawSky(msurface_t *s);
qboolean R_CullBox(vec3_t mins, vec3_t maxs);
void R_MarkLights (dlight_t *light, int bit, model_t *model);
void R_MarkLightsNoVis (dlight_t *light, int bit, mnode_t *node);
void R_BlendedRotateForEntity (entity_t *e, int shadow);
void R_StoreEfrags(efrag_t **ppefrag);

void InitRenderScripts();
void InitModelScripts();//Tei model scripts
void RS_DrawPic (int x, int y, qpic_t *pic);


// FH!
typedef enum	ptype_s		ptype_t;

//void R_CreateParticleFH(int count,int fxtype, int drawtype,int tex_num,float color, ptype_t typeFH,float born, float die, float angle, float v_angle,					  float alpha,					  float scale,					  vec3_t org,					  int org_rand,int vel,int vel_z,int power);
//void R_Radar (vec3_t org, vec3_t dir);

// FH!