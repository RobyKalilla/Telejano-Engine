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
// drquake.h
#ifdef DRQUAKE
#ifndef DRQUAKE_H
#define DRQUAKE_H


#include "dr_gldefs.h" //OpenGL renderer
#include "dr_d3ddefs.h" //Direct3D renderer
#include "dr_softdefs.h" //Software renderer
#include "texloader.h" //Texture management
typedef enum {DR_OPENGL, DR_DIRECT3D, DR_SOFTWARE} rendermode_t;
extern rendermode_t rendermode;

//Texture management
extern int (*TX_LoadTexture)(int texnum, char* filename, qboolean complain, int matchwidth, int matchheight);
extern int (*TX_UploadTexture)(char *identifier, int width, int height, byte *data, qboolean mipmap, qboolean alpha, int bytesperpixel);

extern void (*R_EnableMultitexture)(void);
extern void (*R_DisableMultitexture)(void);
extern void (*R_Bind)(int texnum);

//r_part.c
#ifdef NEWPARTS
extern void (*R_InitParticles)(void);
#endif
extern void (*R_DrawParticles)(void);

//r_main
extern void (__stdcall *R_DepthRange)(double zNear, double zFar); //Directly linked to the renderer
extern void (__stdcall *R_Finish)(void); //Directly linked to the renderer

extern void (*R_Setup)(void);
extern void (*R_Mirror)(void);
extern void (*R_Clear)(void);
extern void (*R_RotateForEntity)(entity_t *e);
extern void (*R_DrawSpriteModel)(entity_t *e);
extern void (*R_PolyBlend)(void);
extern void (*R_DrawAliasModel)(entity_t *e);

//r_light
extern void (*R_RenderDlights)(void);
extern void (*R_RenderDlight)(dlight_t *light);

//gl_warp.c
extern void (*EmitSkyPolys)(msurface_t *fa);
extern void (*EmitWaterPolys)(msurface_t *fa);
extern void (*EmitBothSkyLayers)(msurface_t *fa);

//screen.c
extern void (*SCR_ScreenShot_f)(void);

#endif
#endif
