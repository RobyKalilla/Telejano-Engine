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

//
// all MD2 code goes into this file
//

#include "quakedef.h"


char	loadname[32];	// for hunk tags

// precalculated dot products for quantized angles
float	r_avertexnormal_dots_md2[16][256] =
#include "anorm_dots.h"
;

float			*shadedots_md2	= r_avertexnormal_dots_md2[0];
float			*shadedots2_md2	= r_avertexnormal_dots_md2[0];
extern float	lightlerpoffset;

extern vec3_t	lightcolor;
extern vec3_t	lightspot;

int				md2bboxmins[3], md2bboxmaxs[3];

/*
=================
Mod_LoadQ2AliasModel
=================
*/
void Mod_LoadQ2AliasModel (model_t *mod, void *buffer)
{
	int					i, j, version, numframes, size, *pinglcmd, *poutglcmd, start, end, total;
	md2_t				*pinmodel, *pheader;
	md2triangle_t		*pintriangles, *pouttriangles;
	md2frame_t			*pinframe, *poutframe;
	char				*pinskins;
	vec3_t				temp;
	//Tei hack that load a texture easy (fuck pinload points!)
	int					texnum;
	char				hackname[MAX_PATH];
	//Tei hack that load a texture


	start = Hunk_LowMark ();

	pinmodel = (md2_t *)buffer;

	version = LittleLong (pinmodel->version);
	if (version != MD2ALIAS_VERSION)
		Sys_Error ("%s has wrong version number (%i should be %i)",
				 mod->name, version, MD2ALIAS_VERSION);

	mod->type = mod_alias;
	mod->aliastype = ALIASTYPE_MD2;

	size = LittleLong(pinmodel->ofs_end) + sizeof(md2_t);

	if (size <= 0 || size >= MD2MAX_SIZE)
		Sys_Error ("%s is not a valid model", mod->name);
	pheader = Hunk_AllocName (size, loadname);
	
	mod->flags = 0; // there are no MD2 flags

// endian-adjust and copy the data, starting with the alias model header
	for (i = 0;i < 17;i++)
		((int*)pheader)[i] = LittleLong(((int *)pinmodel)[i]);
	mod->numframes = numframes = pheader->num_frames;
	mod->synctype = ST_RAND;

	if (pheader->ofs_skins <= 0 || pheader->ofs_skins >= pheader->ofs_end)
		Sys_Error ("%s is not a valid model", mod->name);
	if (pheader->ofs_st <= 0 || pheader->ofs_st >= pheader->ofs_end)
		Sys_Error ("%s is not a valid model", mod->name);
	if (pheader->ofs_tris <= 0 || pheader->ofs_tris >= pheader->ofs_end)
		Sys_Error ("%s is not a valid model", mod->name);
	if (pheader->ofs_frames <= 0 || pheader->ofs_frames >= pheader->ofs_end)
		Sys_Error ("%s is not a valid model", mod->name);
	if (pheader->ofs_glcmds <= 0 || pheader->ofs_glcmds >= pheader->ofs_end)
		Sys_Error ("%s is not a valid model", mod->name);

	if (pheader->num_tris < 1 || pheader->num_tris > MD2MAX_TRIANGLES)
		Sys_Error ("%s has invalid number of triangles: %i", mod->name, pheader->num_tris);
	if (pheader->num_xyz < 1 || pheader->num_xyz > MD2MAX_VERTS)
		Sys_Error ("%s has invalid number of vertices: %i", mod->name, pheader->num_xyz);
	if (pheader->num_frames < 1 || pheader->num_frames > MD2MAX_FRAMES)
		Sys_Error ("%s has invalid number of frames: %i", mod->name, pheader->num_frames);
	if (pheader->num_skins < 0 || pheader->num_skins > MD2MAX_SKINS)
		Sys_Error ("%s has invalid number of skins: %i", mod->name, pheader->num_skins);

	for (i = 0;i < 7;i++)
        ((int*)&pheader->ofs_skins)[i] += sizeof(pheader);

// load the skins
	if (pheader->num_skins)
	{
		pinskins = (void*)((int) pinmodel + LittleLong(pinmodel->ofs_skins));
		for (i = 0;i < pheader->num_skins;i++)
		{

			texnum = loadtextureimage (pinskins, true, true);//Tei
		
			//Tei hack that load a texture easy (fuck pinload points!)
			if (!texnum) {
				sprintf(hackname, "%s.pcx", mod->name);
				texnum = loadtextureimage (hackname, true, true);//Tei
				if (!texnum)
				{
					COM_StripExtension(mod->name, hackname);
					texnum = loadtextureimage (hackname, true, true);//Tei
				}
				//Con_Printf("Intentado %s en %d\n",hackname, texnum);
			}
			//Tei hack that load a texture easy (fuck pinload points!)

			pheader->gl_texturenum[i] = texnum;		
			pinskins += MD2MAX_SKINNAME;
		}
	}

// load triangles
	pintriangles = (void*)((int) pinmodel + LittleLong(pinmodel->ofs_tris));
	pouttriangles = (void*)((int) pheader + pheader->ofs_tris);
	// swap the triangle list
	for (i=0 ; i < pheader->num_tris ; i++)
	{
		for (j=0 ; j<3 ; j++)
		{
			pouttriangles->index_xyz[j] = LittleShort (pintriangles->index_xyz[j]);
			pouttriangles->index_st[j] = LittleShort (pintriangles->index_st[j]);
			if (pouttriangles->index_xyz[j] >= pheader->num_xyz)
				Sys_Error ("%s has invalid vertex indices", mod->name);
			if (pouttriangles->index_st[j] >= pheader->num_st)
				Sys_Error ("%s has invalid vertex indices", mod->name);
		}
		pintriangles++;
		pouttriangles++;
	}

//
// load the frames
//
	pinframe = (void*) ((int) pinmodel + LittleLong(pinmodel->ofs_frames));
	poutframe = (void*) ((int) pheader + pheader->ofs_frames);
	for (i=0 ; i < numframes ; i++)
	{
		for (j = 0;j < 3;j++)
		{
			poutframe->scale[j] = LittleFloat(pinframe->scale[j]);
			poutframe->translate[j] = LittleFloat(pinframe->translate[j]);
		}

		strcpy (poutframe->name, pinframe->name);

		for (j = 0;j < pheader->num_xyz;j++)
		{
			VectorCopy (pinframe->verts[j].v, poutframe->verts[j].v);
			poutframe->verts[j].lightnormalindex = pinframe->verts[j].lightnormalindex;

			temp[0] = poutframe->verts[j].v[0] * poutframe->scale[0] + poutframe->translate[0];
			temp[1] = poutframe->verts[j].v[1] * poutframe->scale[1] + poutframe->translate[1];
			temp[2] = poutframe->verts[j].v[2] * poutframe->scale[2] + poutframe->translate[2];

			// update bounding box
			if (temp[0] < md2bboxmins[0]) md2bboxmins[0] = temp[0];
			if (temp[1] < md2bboxmins[1]) md2bboxmins[1] = temp[1];
			if (temp[2] < md2bboxmins[2]) md2bboxmins[2] = temp[2];
			if (temp[0] > md2bboxmaxs[0]) md2bboxmaxs[0] = temp[0];
			if (temp[1] > md2bboxmaxs[1]) md2bboxmaxs[1] = temp[1];
			if (temp[2] > md2bboxmaxs[2]) md2bboxmaxs[2] = temp[2];
		}

		pinframe = (void*) &pinframe->verts[j].v[0];
		poutframe = (void*) &poutframe->verts[j].v[0];
	}

	VectorCopy (md2bboxmaxs, mod->maxs);
	VectorCopy (md2bboxmins, mod->mins);

	// load the draw list
	pinglcmd = (void*) ((int) pinmodel + LittleLong(pinmodel->ofs_glcmds));
	poutglcmd = (void*) ((int) pheader + pheader->ofs_glcmds);
	for (i = 0;i < pheader->num_glcmds;i++)
		*poutglcmd++ = LittleLong(*pinglcmd++);

// move the complete, relocatable alias model to the cache
	end = Hunk_LowMark ();
	total = end - start;
	
	Cache_Alloc (&mod->cache, total, loadname);
	if (!mod->cache.data)
		return;
	memcpy (mod->cache.data, pheader, total);

	Hunk_FreeToLowMark (start);
}

/*
=============
GL_DrawQ2AliasFrame
=============
*/

extern cvar_t r_ambient2;//Tei r_ambient for md2 files
void GL_DrawQ2AliasFrame (entity_t *e, md2_t *pheader, int lastpose, int pose, float lerp)
{
	float uplight;
	float	ilerp;
	float 	l;
	int		*order, count;
	md2trivertx_t	*verts1, *verts2;
	vec3_t	scale1, translate1, scale2, translate2;
	md2frame_t *frame1, *frame2;

	ilerp = 1.0 - lerp;


	frame1 = (md2frame_t *)((int) pheader + pheader->ofs_frames + (pheader->framesize * lastpose)); 
	frame2 = (md2frame_t *)((int) pheader + pheader->ofs_frames + (pheader->framesize * pose)); 


	VectorCopy(frame1->scale, scale1);
	VectorCopy(frame1->translate, translate1);
	VectorCopy(frame2->scale, scale2);
	VectorCopy(frame2->translate, translate2);
	verts1 = &frame1->verts[0];
	verts2 = &frame2->verts[0];
	order = (int *)((int)pheader + pheader->ofs_glcmds);

	while (1)
	{
		// get the vertex count and primitive type
		count = *order++;
		if (!count)
			break;		// done
		if (count < 0)
		{
			count = -count;
			glBegin (GL_TRIANGLE_FAN);
		}
		else
			glBegin (GL_TRIANGLE_STRIP);

		do
		{
			if (!e->model->fullbright)
			{
				float d1, d2, l1, l2, diff;
				d1 = shadedots_md2[verts2->lightnormalindex] - shadedots_md2[verts1->lightnormalindex];
				d2 = shadedots2_md2[verts2->lightnormalindex] - shadedots2_md2[verts1->lightnormalindex];
				l1 = shadedots_md2[verts1->lightnormalindex] + (lerp * d1);
				l2 = shadedots2_md2[verts1->lightnormalindex] + (lerp * d2);
				if (l1 != l2)
				{
					if (l1 > l2) {
						diff = l1 - l2;
						diff *= lightlerpoffset;
						l = l1 - diff;
					} else {
						diff = l2 - l1;
						diff *= lightlerpoffset;
						l = l1 + diff;
					}
				}
				else
				{
					l = l1;
				}
				//Tei up light for md2 files
				uplight = r_ambient2.value;

#if 1 //Telejano RC2
				glColor4f (l * lightcolor[0] *uplight , l * lightcolor[1] * uplight, l * lightcolor[2]* uplight, e->alpha);//Tei md2 file less black
#else
				glColor4f (l * lightcolor[0] , l * lightcolor[1], l * lightcolor[2], e->alpha);
#endif

				//Tei up light for md2 files

			}
			else {
				glColor4f (1, 1, 1, e->alpha);
			}

			glTexCoord2f(((float *)order)[0], ((float *)order)[1]);
			glVertex3f((verts1[order[2]].v[0]*scale1[0]+translate1[0])*ilerp+(verts2[order[2]].v[0]*scale2[0]+translate2[0])*lerp,
					   (verts1[order[2]].v[1]*scale1[1]+translate1[1])*ilerp+(verts2[order[2]].v[1]*scale2[1]+translate2[1])*lerp,
					   (verts1[order[2]].v[2]*scale1[2]+translate1[2])*ilerp+(verts2[order[2]].v[2]*scale2[2]+translate2[2])*lerp);
				
			order+=3;
		} while (--count);

		glEnd ();
	}
	glColor4f (1,1,1,1);
}


/*
=============
GL_DrawQ2AliasShadow
=============
*/


#if 0
void GL_DrawQ2AliasShadow (entity_t *e, md2_t *pheader, int lastpose, int pose, float lerp)
{
	float	ilerp, height, lheight;
	int		*order, count;
	md2trivertx_t	*verts1, *verts2;
	vec3_t	scale1, translate1, scale2, translate2, point;
	md2frame_t *frame1, *frame2;
	// Tomaz - New Shadow Begin
	trace_t		downtrace;
	vec3_t		downmove;
	float		s1,c1;
	// Tomaz - New Shadow End

	lheight = e->origin[2] - lightspot[2];

	height = 0;

	ilerp = 1.0 - lerp;

	
	e->alpha = 0.5;//Tei

	// Tomaz - New Shadow Begin
	VectorCopy (e->origin, downmove);
	downmove[2] = downmove[2] - 4096;
	memset (&downtrace, 0, sizeof(downtrace));
	SV_RecursiveHullCheck (cl.worldmodel->hulls, 0, 0, 1, e->origin, downmove, &downtrace);

	s1 = sin( e->angles[1]/180*M_PI);
	c1 = cos( e->angles[1]/180*M_PI);
	// Tomaz - New Shadow End

	frame1 = (md2frame_t *)((int) pheader + pheader->ofs_frames + (pheader->framesize * lastpose)); 
	frame2 = (md2frame_t *)((int) pheader + pheader->ofs_frames + (pheader->framesize * pose)); 

	VectorCopy(frame1->scale, scale1);
	VectorCopy(frame1->translate, translate1);
	VectorCopy(frame2->scale, scale2);
	VectorCopy(frame2->translate, translate2);
	verts1 = &frame1->verts[0];
	verts2 = &frame2->verts[0];
	order = (int *)((int) pheader + pheader->ofs_glcmds);

	height = -lheight + 1.0;

	while (1)
	{
		// get the vertex count and primitive type
		count = *order++;
		if (!count)
			break;		// done
		if (count < 0)
		{
			count = -count;
			glBegin (GL_TRIANGLE_FAN);
		}
		else
			glBegin (GL_TRIANGLE_STRIP);

		do
		{
			point[0] = (verts1[order[2]].v[0]*scale1[0]+translate1[0])*ilerp+(verts2[order[2]].v[0]*scale2[0]+translate2[0])*lerp;
			point[1] = (verts1[order[2]].v[1]*scale1[1]+translate1[1])*ilerp+(verts2[order[2]].v[1]*scale2[1]+translate2[1])*lerp;
			point[2] = (verts1[order[2]].v[2]*scale1[2]+translate1[2])*ilerp+(verts2[order[2]].v[2]*scale2[2]+translate2[2])*lerp;

			// Tomaz - New shadow Begin
			point[2] =  - (e->origin[2] - downtrace.endpos[2]) ;

			point[2] += ((point[1] * (s1 * downtrace.plane.normal[0])) -
						  (point[0] * (c1 * downtrace.plane.normal[0])) -
						  (point[0] * (s1 * downtrace.plane.normal[1])) - 
						  (point[1] * (c1 * downtrace.plane.normal[1]))) +  
						  ((1.0 - downtrace.plane.normal[2])*20) + 0.2 ;

			glVertex3fv (point);
			// Tomaz - New shadow Begin

			order+=3;
		} while (--count);

		glEnd ();
	}

}
#else
extern cvar_t sv_stepsize;

void GL_DrawQ2AliasShadow (entity_t *e, md2_t *pheader, int lastpose, int pose, float lerp)
{
	float	ilerp, height, lheight;
	int		*order, count;
	md2trivertx_t	*verts1, *verts2;
	vec3_t	scale1, translate1, scale2, translate2, point;
	md2frame_t *frame1, *frame2;
	// Tomaz - New Shadow Begin
	trace_t		downtrace;
	vec3_t		downmove;
	float		s1,c1;
	// Tomaz - New Shadow End

	// Tei shadevector
	float an;
	vec3_t	shadevector;
	// Tei shadevector

	lheight = e->origin[2] - lightspot[2];

	height = 0;

	ilerp = 1.0 - lerp;

	
	//e->alpha = 0.5;//Tei

	// Tomaz - New Shadow Begin
	VectorCopy (e->origin, downmove);
	downmove[2] = downmove[2] - 4096;
	memset (&downtrace, 0, sizeof(downtrace));
	SV_RecursiveHullCheck (cl.worldmodel->hulls, 0, 0, 1, e->origin, downmove, &downtrace);

	s1 = sin( e->angles[1]/180*M_PI);
	c1 = cos( e->angles[1]/180*M_PI);
	// Tomaz - New Shadow End

	// Tei shadevector
	an = e->angles[1]/180*M_PI;
	shadevector[0] = cos(-an);
	shadevector[1] = sin(-an);
	shadevector[2] = 1;
	VectorNormalize (shadevector);
	// Tei shadevector

	frame1 = (md2frame_t *)((int) pheader + pheader->ofs_frames + (pheader->framesize * lastpose)); 
	frame2 = (md2frame_t *)((int) pheader + pheader->ofs_frames + (pheader->framesize * pose)); 

	VectorCopy(frame1->scale, scale1);
	VectorCopy(frame1->translate, translate1);
	VectorCopy(frame2->scale, scale2);
	VectorCopy(frame2->translate, translate2);
	verts1 = &frame1->verts[0];
	verts2 = &frame2->verts[0];
	order = (int *)((int) pheader + pheader->ofs_glcmds);

	height = -lheight + 1.0;

	while (1)
	{
		// get the vertex count and primitive type
		count = *order++;
		if (!count)
			break;		// done
		if (count < 0)
		{
			count = -count;
			glBegin (GL_TRIANGLE_FAN);
		}
		else
			glBegin (GL_TRIANGLE_STRIP);

		do
		{

			point[0] = (verts1[order[2]].v[0]*scale1[0]+translate1[0])*ilerp+(verts2[order[2]].v[0]*scale2[0]+translate2[0])*lerp;
			point[1] = (verts1[order[2]].v[1]*scale1[1]+translate1[1])*ilerp+(verts2[order[2]].v[1]*scale2[1]+translate2[1])*lerp;
			point[2] = (verts1[order[2]].v[2]*scale1[2]+translate1[2])*ilerp+(verts2[order[2]].v[2]*scale2[2]+translate2[2])*lerp;

		

			//point[2] = height - point[0] * e->angles[0] * 0.02f - point[1] * e->angles[2] * 0.02f;
			point[2] = height - point[0] * e->angles[0] * 0.02f - point[1] * e->angles[2] * 0.02;

			glVertex3fv (point);

			order+=3;
		} while (--count);

		glEnd ();
	}

}
#endif





/*
=================
R_SetupQ2AliasFrame

=================
*/


void R_SetupQ2AliasFrame (int frame, md2_t *pheader, entity_t *e)
{
	float			lerp;

	if ((frame >= pheader->num_frames) || (frame < 0))
	{
		Con_DPrintf ("R_SetupQ2AliasFrame: no such frame %d\n", frame);
		frame = 0;
	}

	if (e->draw_lastmodel == e->model)
	{
		if (frame != e->draw_pose)
		{
			e->draw_lastpose = e->draw_pose;
			e->draw_pose = frame;
			e->draw_lerpstart = cl.time;
			lerp = 0;
		}
		else
			lerp = (cl.time - e->draw_lerpstart) * 10.0;
	}
	else // uninitialized
	{
		e->draw_lastmodel = e->model;
		e->draw_lastpose = e->draw_pose = frame;
		e->draw_lerpstart = cl.time;
		lerp = 0;
	}
	if (lerp > 1) lerp = 1;

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	GL_DrawQ2AliasFrame (e, pheader, e->draw_lastpose, frame, lerp);


	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	if (r_shadows.value)
	{
		if (!e->model->noshadow)
		{
			trace_t		downtrace;
			vec3_t		downmove;

			glPushMatrix ();

			VectorCopy (e->origin, downmove);
			
			downmove[2] = downmove[2] - 4096;
			memset (&downtrace, 0, sizeof(downtrace));
			SV_RecursiveHullCheck (cl.worldmodel->hulls, 0, 0, 1, e->origin, downmove, &downtrace);

			glDisable (GL_TEXTURE_2D);
			glDepthMask(false); // disable zbuffer updates
			//if (e->model->effect != MFX_GRASS)
				glColor4f (0,0,0,(e->alpha - ((e->origin[2] + e->model->mins[2]-downtrace.endpos[2])/20)));//Tei 60->20 
			//else
			//	glColor4f (0,0,0,0);//Tei 60->20 
			GL_DrawQ2AliasShadow (e, pheader, e->draw_lastpose, frame, lerp);
			glDepthMask(true); // enable zbuffer updates
			glEnable (GL_TEXTURE_2D);
			glColor4f (1,1,1,1);
			glPopMatrix ();
		}
	}
}