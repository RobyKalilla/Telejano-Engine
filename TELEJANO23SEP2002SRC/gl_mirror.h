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

// bloody mirrors
void Mirror_Scale (void);
void Mirror_Clear (void);
void R_Mirror (void);
extern cvar_t r_mirroralpha;
extern qboolean mirror;
extern mplane_t *mirror_plane;
extern msurface_t *mirrorchain;
extern qboolean mirror_render;

#define SURF_MIRROR 32768

