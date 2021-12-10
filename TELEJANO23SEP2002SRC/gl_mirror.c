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
// FIXME: add this to rscript + fix skybox problem
// Modified for use with tomazquake by BramBo

#include "quakedef.h"

// glmsurface_t == msurface_t


// bloody mirrors
qboolean mirror;
mplane_t *mirror_plane;
cvar_t r_mirroralpha = {"r_mirroralpha","1", true};
msurface_t *mirrorchain = NULL;
qboolean mirror_render;	// true when reflections are being rendered

void Mirror_Scale (void)
{
	if (mirror_plane->normal[2])
		glScalef ( 1,-1, 1);
	else
		glScalef (-1, 1, 1);

	glCullFace(GL_BACK);
}


void Mirror_Clear (void)
{
	if (gl_clear.value)
		glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	else
		glClear (GL_DEPTH_BUFFER_BIT);

	gldepthmin = 0;
	gldepthmax = 0.5;
	glDepthFunc (GL_LEQUAL);
}

/*
=============
R_Mirror

  code written by MH - http://mhquake.quakesrc.org/
=============
*/
extern cvar_t gl_multitexture;
float r_base_world_matrix[16];
void R_RenderScene (void);

void R_Mirror (void)
{
	float d;
	msurface_t *s;
	entity_t *ent;
	float mirror_alpha;
	glpoly_t	*p;
	float		*v;
	int			i;


	// these should stay the same for all mirror planes
	vec3_t oldvieworg;
	vec3_t oldviewangles;
	vec3_t oldvpn;


	// don't have infinite reflections if 2 mirrors are facing each other (looks ugly but
	// better than hanging the engine)
	mirror_render = true;

	ent = &cl_entities[cl.viewentity];

	if (cl_numvisedicts < MAX_VISEDICTS)
	{
		cl_visedicts[cl_numvisedicts] = ent;
		cl_numvisedicts++;
	}

	memcpy (r_base_world_matrix, r_world_matrix, sizeof (r_base_world_matrix));

	VectorCopy (r_refdef.vieworg, oldvieworg);
	VectorCopy (r_refdef.viewangles, oldviewangles);
	VectorCopy (vpn, oldvpn);

	// r_mirroralpha values of more than about 0.65 don't really look well or have any effect
	mirror_alpha = r_mirroralpha.value * (1.0 / 1.5);

	for (s = mirrorchain; s; s = s->texturechain)
	{
		mirror_plane = s->plane;

		d = PlaneDiff (oldvieworg, mirror_plane);
		VectorMA (oldvieworg, -2 * d, mirror_plane->normal, r_refdef.vieworg);

		d = DotProduct (oldvpn, mirror_plane->normal);
		VectorMA (oldvpn, -2 * d, mirror_plane->normal, vpn);

		r_refdef.viewangles[0] = -asin (vpn[2]) / M_PI * 180;
		r_refdef.viewangles[1] = atan2 (vpn[1], vpn[0]) / M_PI * 180;
		r_refdef.viewangles[2] = -oldviewangles[2];

		gldepthmin = 0.5;
		gldepthmax = 1;
		glDepthRange (gldepthmin, gldepthmax);
		glDepthFunc (GL_LEQUAL);

		// everything to do with the render has been moved into r_renderscene, so that the
		// full render reflects properly in the skybox...
		R_RenderScene ();

		gldepthmin = 0;
		gldepthmax = 0.5;
		glDepthRange (gldepthmin, gldepthmax);
		glDepthFunc (GL_LEQUAL);
		glMatrixMode (GL_PROJECTION);

		if (mirror_plane->normal[2])
			glScalef (1, -1, 1);
		else
			glScalef (-1, 1, 1);

		glCullFace (GL_FRONT);
		glMatrixMode (GL_MODELVIEW);
		glLoadMatrixf (r_base_world_matrix);

		glColor4f (1, 1, 1, mirror_alpha);

		glBindTexture (GL_TEXTURE_2D, s->texinfo->texture->gl_texturenum);

		p = s->polys;
		v = p->verts[0];

		glBegin (GL_POLYGON);

		for (i=0 ; i<p->numverts ; i++, v+= VERTEXSIZE)
		{
			glTexCoord2fv (&v[3]);
			glVertex3fv (v);
		}

		glEnd ();
		glColor4f (1, 1, 1, 1);
	}

	mirrorchain = NULL;
	mirror = false;
	mirror_render = false;
}


