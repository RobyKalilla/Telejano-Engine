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
// r_surf.c: surface-related refresh code

#include "quakedef.h"
#include "bsp_render.h"
#include "gl_mirror.h"

extern  cvar_t  r_dosky;			// Tei - sky
extern	qboolean gl_mtexable;

int			lightmap_textures;
int			lightmap_bytes = 4;
int			lightmap_format = GL_RGBA;

unsigned	blocklights[18*18*3]; // Tomaz - Lit Support

int			active_lightmaps;

glpoly_t	*lightmap_polys[MAX_LIGHTMAPS];
qboolean	lightmap_modified[MAX_LIGHTMAPS];
glRect_t	lightmap_rectchange[MAX_LIGHTMAPS];
byte		lightmaps[4*MAX_LIGHTMAPS*BLOCK_WIDTH*BLOCK_HEIGHT];

int			allocated[MAX_LIGHTMAPS][BLOCK_WIDTH];

msurface_t  *skychain		= NULL;
msurface_t  *waterchain		= NULL;

/*
===============
R_AddDynamicLights
===============
*/

void R_AddDynamicLights (msurface_t *surf)
{
	int			lnum;
	int			sd, td;
	float		dist, rad, minlight;
	vec3_t		impact, local;
	int			s, t;
	int			i;
	int			smax, tmax;
	mtexinfo_t	*tex;
	// Tomaz - Lit Support Begin
	int			cred, cgreen, cblue, brightness;
	unsigned	*bl;
	// Tomaz - Lit Support End



	smax = (surf->extents[0]>>4)+1;
	tmax = (surf->extents[1]>>4)+1;
	tex = surf->texinfo;

	for (lnum=0 ; lnum<MAX_DLIGHTS ; lnum++)
	{
		if ( !(surf->dlightbits & (1<<lnum) ) )
			continue;		// not lit by this light

		rad = cl_dlights[lnum].radius;
		dist = PlaneDiff (cl_dlights[lnum].origin, surf->plane);
		rad -= fabs(dist);
		minlight = cl_dlights[lnum].minlight;
		if (rad < minlight)
			continue;
		minlight = rad - minlight;

		for (i=0 ; i<3 ; i++)
		{
			impact[i] = cl_dlights[lnum].origin[i] -
					surf->plane->normal[i]*dist;
		}

		local[0] = DotProduct (impact, tex->vecs[0]) + tex->vecs[0][3];
		local[1] = DotProduct (impact, tex->vecs[1]) + tex->vecs[1][3];

		local[0] -= surf->texturemins[0];
		local[1] -= surf->texturemins[1];

		// Tomaz - Lit Support Begin
		bl		= blocklights;
		cred	= (int)(cl_dlights[lnum].color[0] * 256.0f);
		cgreen	= (int)(cl_dlights[lnum].color[1] * 256.0f);
		cblue	= (int)(cl_dlights[lnum].color[2] * 256.0f);
		// Tomaz - Lit Support End	

		for (t = 0 ; t<tmax ; t++)
		{
			td = local[1] - t*16;
			if (td < 0)
				td = -td;
			for (s=0 ; s<smax ; s++)
			{
				sd = local[0] - s*16;
				if (sd < 0)
					sd = -sd;
				if (sd > td)
					dist = sd + (td>>1);
				else
					dist = td + (sd>>1);
				if (dist < minlight)
				// Tomaz - Lit Support Begin
				{
					brightness = rad - dist;
					bl[0] += brightness * cred;
					bl[1] += brightness * cgreen;
					bl[2] += brightness * cblue;
				}
				bl += 3;
				// Tomaz - Lit Support End
			}
		}
	}
}


static int dlightdivtable[32768];


void GL_Surf_Init(void)
{
#if STAINS
	int i;
	dlightdivtable[0] = 4194304;
	for (i = 1;i < 32768;i++)
		dlightdivtable[i] = 4194304 / (i << 7);

	/*Cvar_RegisterVariable(&r_ambient);
	Cvar_RegisterVariable(&r_vertexsurfaces);
	Cvar_RegisterVariable(&r_dlightmap);
	Cvar_RegisterVariable(&r_drawportals);
	Cvar_RegisterVariable(&r_testvis);

	R_RegisterModule("GL_Surf", gl_surf_start, gl_surf_shutdown, gl_surf_newmap);
	*/
#endif
}


void R_StainNode (mnode_t *node, model_t *model, vec3_t origin, float radius, int icolor[8])
{
#if STAINS
	float ndist;
	msurface_t *surf, *endsurf;
	int sdtable[256], td, maxdist, maxdist2, maxdist3, i, s, t, smax, tmax, smax3, dist2, impacts, impactt, subtract, a, stained, cr, cg, cb, ca, ratio;
	byte *bl;
	vec3_t impact;
	// LordHavoc: use 64bit integer...  shame it's not very standardized...
#if _MSC_VER || __BORLANDC__
	__int64     k;
#else
	long long   k;
#endif


	// for comparisons to minimum acceptable light
	// compensate for 4096 offset
	maxdist = radius * radius + 4096;

	// clamp radius to avoid exceeding 32768 entry division table
	if (maxdist > 4194304)
		maxdist = 4194304;

	subtract = (int) ((1.0f / maxdist) * 4194304.0f);

loc0:
	if (node->contents < 0)
		return;
	ndist = PlaneDiff(origin, node->plane);
	if (ndist > radius)
	{
		node = node->children[0];
		goto loc0;
	}
	if (ndist < -radius)
	{
		node = node->children[1];
		goto loc0;
	}

	dist2 = ndist * ndist;
	dist2 += 4096.0f;
	if (dist2 < maxdist)
	{
		maxdist3 = maxdist - dist2;

		impact[0] = origin[0] - node->plane->normal[0] * ndist;
		impact[1] = origin[1] - node->plane->normal[1] * ndist;
		impact[2] = origin[2] - node->plane->normal[2] * ndist;

		for (surf = model->surfaces + node->firstsurface, endsurf = surf + node->numsurfaces;surf < endsurf;surf++)
		{
			if (surf->stainsamples)
			{
				smax = (surf->extents[0] >> 4) + 1;
				tmax = (surf->extents[1] >> 4) + 1;

				impacts = DotProduct (impact, surf->texinfo->vecs[0]) + surf->texinfo->vecs[0][3] - surf->texturemins[0];
				impactt = DotProduct (impact, surf->texinfo->vecs[1]) + surf->texinfo->vecs[1][3] - surf->texturemins[1];

				s = bound(0, impacts, smax * 16) - impacts;
				t = bound(0, impactt, tmax * 16) - impactt;
				i = s * s + t * t + dist2;
				if (i > maxdist)
					continue;

				// reduce calculations
				for (s = 0, i = impacts; s < smax; s++, i -= 16)
					sdtable[s] = i * i + dist2;

				// convert to 8.8 blocklights format
				bl = surf->stainsamples;
				smax3 = smax * 3;
				stained = false;

				i = impactt;
				for (t = 0;t < tmax;t++, i -= 16)
				{
					td = i * i;
					// make sure some part of it is visible on this line
					if (td < maxdist3)
					{
						maxdist2 = maxdist - td;
						for (s = 0;s < smax;s++)
						{
							if (sdtable[s] < maxdist2)
							{
								k = dlightdivtable[(sdtable[s] + td) >> 7] - subtract;
								if (k > 0)
								{
									ratio = rand() & 255;
									ca = (((icolor[7] - icolor[3]) * ratio) >> 8) + icolor[3];
									a = (ca * k) >> 8;
									if (a > 0)
									{
										a = bound(0, a, 256);
										cr = (((icolor[4] - icolor[0]) * ratio) >> 8) + icolor[0];
										cg = (((icolor[5] - icolor[1]) * ratio) >> 8) + icolor[1];
										cb = (((icolor[6] - icolor[2]) * ratio) >> 8) + icolor[2];
										bl[0] = (byte) ((((cr - (int) bl[0]) * a) >> 8) + (int) bl[0]);
										bl[1] = (byte) ((((cg - (int) bl[1]) * a) >> 8) + (int) bl[1]);
										bl[2] = (byte) ((((cb - (int) bl[2]) * a) >> 8) + (int) bl[2]);
										stained = true;
									}
								}
							}
							bl += 3;
						}
					}
					else // skip line
						bl += smax3;
				}
				// force lightmap upload
				if (stained)
					surf->cached_dlight = true;
			}
		}
	}

	if (node->children[0]->contents >= 0)
	{
		if (node->children[1]->contents >= 0)
		{
			R_StainNode(node->children[0], model, origin, radius, icolor);
			node = node->children[1];
			goto loc0;
		}
		else
		{
			node = node->children[0];
			goto loc0;
		}
	}
	else if (node->children[1]->contents >= 0)
	{
		node = node->children[1];
		goto loc0;
	}
#endif
}


void R_Stain (vec3_t origin, float radius, int cr1, int cg1, int cb1, int ca1, int cr2, int cg2, int cb2, int ca2)
{
#if STAINS
	int n, icolor[8];
	entity_render_t *ent;
	model_t *model;
	vec3_t org;
	icolor[0] = cr1;
	icolor[1] = cg1;
	icolor[2] = cb1;
	icolor[3] = ca1;
	icolor[4] = cr2;
	icolor[5] = cg2;
	icolor[6] = cb2;
	icolor[7] = ca2;

	model = cl.worldmodel;
	softwaretransformidentity();
	R_StainNode(model->nodes + model->hulls[0].firstclipnode, model, origin, radius, icolor);

	// look for embedded bmodels
	for (n = 1;n < MAX_EDICTS;n++)
	{
		ent = &cl_entities[n].render;
		model = ent->model;
		if (model && model->name[0] == '*')
		{
			Mod_CheckLoaded(model);
			if (model->type == mod_brush)
			{
				softwaretransformforentity(ent);
				softwareuntransform(origin, org);
				R_StainNode(model->nodes + model->hulls[0].firstclipnode, model, org, radius, icolor);
			}
		}
	}
#endif
}



/*
===============
R_BuildLightMap

Combine and scale multiple lightmaps into the 8.8 format in blocklights
===============
*/
void R_BuildLightMap (msurface_t *surf, byte *dest, int stride)
{
	int			smax, tmax;
	int			t;
	int			i, j, size, blocksize;
	byte		*lightmap;
	unsigned	scale;
	int			maps;
	unsigned	*bl;

	surf->cached_dlight = (surf->dlightframe == r_framecount);

	smax = (surf->extents[0]>>4)+1;
	tmax = (surf->extents[1]>>4)+1;
	size = smax*tmax;
	blocksize = size*3;
	lightmap = surf->samples;

// set to full bright if no light data
	if (!cl.worldmodel->lightdata)
	{
		// Tomaz - Lit Support Begin
		//memset (blocklights, 255, blocksize*sizeof(int));
		memset (blocklights, 127, blocksize*sizeof(int));//Standard is black
		// Tomaz - Lit Support End
		goto store;
	}

// clear to no light
	// Tomaz - Lit Support Begin
	memset (blocklights, 0, blocksize*sizeof(int));
	// Tomaz - Lit Support End

// add all the lightmaps
	if (lightmap)
		for (maps = 0 ; maps < MAXLIGHTMAPS && surf->styles[maps] != 255 ;
			 maps++)
		{
			scale = d_lightstylevalue[surf->styles[maps]];
			surf->cached_light[maps] = scale;	// 8.8 fraction
			// Tomaz - Lit Support Begin
			bl = blocklights;
			for (i=0 ; i<size ; i++)
			{
				bl[0]		+= lightmap[0] * scale;
				bl[1]		+= lightmap[1] * scale;
				bl[2]		+= lightmap[2] * scale;

				bl			+= 3;
				lightmap	+= 3;
			}
			// Tomaz - Lit Support End
		}

// add all the dynamic lights
	if (surf->dlightframe == r_framecount)
		R_AddDynamicLights (surf);

// bound, invert, and shift
store:
	bl = blocklights;
	stride -= smax * lightmap_bytes;
	for (i=0 ; i<tmax ; i++, dest += stride)
	{
		for (j=0 ; j<smax ; j++)
		{
			// Tomaz - Lit Support Begin
			t = bl[0] >> 7;if (t > 255) t = 255;dest[0] = t;
			t = bl[1] >> 7;if (t > 255) t = 255;dest[1] = t;
			t = bl[2] >> 7;if (t > 255) t = 255;dest[2] = t;
			if (lightmap_bytes > 3)	dest[3] = 255;

			bl		+= 3;
			dest	+= lightmap_bytes;
			// Tomaz - Lit Support End
		}
	}
}

/*
===============
R_TextureAnimation

Returns the proper texture for a given time and base texture
===============
*/
texture_t *R_TextureAnimation (texture_t *base)
{
	int		reletive;
	int		count;

	if (currententity->frame)
	{
		if (base->alternate_anims)
			base = base->alternate_anims;
	}
	
	if (!base->anim_total)
		return base;

	reletive = (int)(cl.time*10) % base->anim_total;

	count = 0;	
	while (base->anim_min > reletive || base->anim_max <= reletive)
	{
		base = base->anim_next;
		if (!base)
			Sys_Error ("R_TextureAnimation: broken cycle");
		if (++count > 100)
			Sys_Error ("R_TextureAnimation: infinite cycle");
	}

	return base;
}


/*
=============================================================

	BRUSH MODELS

=============================================================
*/

int oldtexture;
extern	int		solidskytexture;
extern	int		alphaskytexture;
extern	float	speedscale;		// for top sky and bottom sky

lpMTexFUNC qglMTexCoord2fSGIS_ARB = NULL;
lpSelTexFUNC qglSelectTextureSGIS_ARB = NULL;

#include "gl_rscript.h"

void R_DrawBrushMTexScript (msurface_t *s);

/*
================
R_DrawLinePolys
================
*/
void R_DrawLinePolys (msurface_t *s)
{
	int			i;
	float		*v;
	glpoly_t	*p;

	p = s->polys;
	v = p->verts[0];

	glDisable(GL_TEXTURE_2D);
	glBegin(GL_LINE_LOOP);

	for (i=0 ; i<p->numverts ; i++, v+= VERTEXSIZE)
		glVertex3fv (v);

	glEnd ();
	glEnable(GL_TEXTURE_2D);
}

/*
====================
R_DrawBrushMTex
====================
*/
int		causticstexture[32];	// Tomaz - Underwater Caustics
extern cvar_t gl_detail; //CHP detail textures
extern cvar_t gl_dither; //Tei force dither
extern cvar_t temp1;
extern int weaponmodel;

void EmitUnderwaterPolys (msurface_t *fa);
void EmitDetailPolys (msurface_t *s);
void EmitGrassFx (msurface_t *s);

extern cvar_t gl_geocaustics; //Tei geocastics
void EmitUnderwaterPolys2 (msurface_t *fa); //without animation


void R_DrawBrushMTex (msurface_t *s)
{
	glpoly_t	*p;
	float		*v;
	int			i;//,k;
	texture_t	*t;
	glRect_t	*theRect;
	vec3_t		w;
	float * wp = w;

	p = s->polys;

	t = R_TextureAnimation (s->texinfo->texture);

	if(t->rs)
	{
		R_DrawBrushMTexScript (s);
		return;
	}

	if (gl_dither.value)
		glEnable(GL_DITHER);//Tei dither

	glBindTexture (GL_TEXTURE_2D, t->gl_texturenum);

	qglSelectTextureSGIS_ARB(TEXTURE1_SGIS_ARB);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEnable(GL_TEXTURE_2D);

	glBindTexture (GL_TEXTURE_2D, lightmap_textures + s->lightmaptexturenum);
	i = s->lightmaptexturenum;
	if (lightmap_modified[i])
	{
		lightmap_modified[i] = false;
		theRect = &lightmap_rectchange[i];
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, theRect->t, BLOCK_WIDTH, theRect->h, lightmap_format, GL_UNSIGNED_BYTE, lightmaps+(i* BLOCK_HEIGHT + theRect->t) *BLOCK_WIDTH*lightmap_bytes);
		theRect->l = BLOCK_WIDTH;
		theRect->t = BLOCK_HEIGHT;
		theRect->h = 0;
		theRect->w = 0;
	}

	glBegin(GL_POLYGON);
	v = p->verts[0];
	for (i=0 ; i<p->numverts ; i++, v+= VERTEXSIZE)
	{
		qglMTexCoord2fSGIS_ARB (TEXTURE0_SGIS_ARB, v[3] , v[4]);//randomize this->comic draw
		qglMTexCoord2fSGIS_ARB (TEXTURE1_SGIS_ARB, v[5], v[6]);
		glVertex3fv (v);

#if 0
		VectorCopy(v,w);
		k =temp1.value;

		w[0] += sin(w[0])*k;
		w[1] += sin(w[1])*k;
		w[2] += sin(w[2])*k;
		glVertex3fv (wp);
#endif
	}
	glEnd ();

	glDisable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	qglSelectTextureSGIS_ARB(TEXTURE0_SGIS_ARB);


#if 1 //Tei QMB underwater
	if(gl_caustics.value)
		if (s->flags & SURF_UNDERWATER)
			EmitUnderwaterPolys(s);//Tei QMB!
#endif

	if(gl_geocaustics.value)		
		EmitUnderwaterPolys2(s);//XFX

#if 0 //Tei QMB! metal, ugly-ulgy
	if (s->flags & SURF_SHINY_GLASS)
			EmitChromePoly (s);//, shinetex_glass);
	else if (s->flags & SURF_SHINY_METAL)
			EmitChromePoly (s);//, shinetex_chrome);
#endif
	
	//Tei grass support
	if (s->flags & SURF_GRASS)
		EmitGrassFx (s);
	//Tei grass support

	if (gl_detail.value)
		EmitDetailPolys(s); // CHP - ranger starts to wear prescription glasses

	if (t->fullbrights != -1 && gl_fbr.value)
	{
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);	

		glBindTexture (GL_TEXTURE_2D, t->fullbrights);

		p = s->polys;
		v = p->verts[0];

		glBegin (GL_POLYGON);

		for (i=0 ; i<p->numverts ; i++, v+= VERTEXSIZE)
		{
			glTexCoord2f (v[3], v[4]);
			glVertex3fv (v);
		}

		glEnd ();

		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	}

	//if (gl_dither.value)
	//	glDisable(GL_DITHER);//Tei dither
}

int RS_AnimTexture(int rs);

void EmitRSGrassFx (msurface_t *s, model_t *thegrassmodel, float density);
void EmitRSDetailPolys (msurface_t *s, int mydetail, int myscale);
void EmitRSSmokeFireFx (msurface_t *s, float smokeheight, float density);

/*
====================
R_DrawBrushMTexScript
====================
*/
void R_DrawBrushMTexScript (msurface_t *s)
{
	glpoly_t	*p;
	float		*v, vt[3], os, ot;
	int			i, rs;
	texture_t	*t;
	glRect_t	*theRect;
	qboolean	stage;
	float		txm, tym;
	vec3_t		nv;

	p = s->polys;

	t = R_TextureAnimation (s->texinfo->texture);

	stage = true;

	rs = t->rs;

	while (stage)
	{
		
		
		
		if (!rscripts[rs].texexist)
			glBindTexture (GL_TEXTURE_2D, t->gl_texturenum);
		else if (!rscripts[rs].useanim)
			glBindTexture (GL_TEXTURE_2D, rscripts[rs].texnum);
		else
			glBindTexture (GL_TEXTURE_2D, RS_AnimTexture(rs));
		
		if (gl_envmap.value)
		{
			glDepthMask (false);
		}
		
		qglSelectTextureSGIS_ARB(TEXTURE1_SGIS_ARB);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glEnable(GL_TEXTURE_2D);

		glBindTexture (GL_TEXTURE_2D, lightmap_textures + s->lightmaptexturenum);
		i = s->lightmaptexturenum;
		if (lightmap_modified[i])
		{
			lightmap_modified[i] = false;
			theRect = &lightmap_rectchange[i];
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, theRect->t, BLOCK_WIDTH, theRect->h, lightmap_format, GL_UNSIGNED_BYTE, lightmaps+(i* BLOCK_HEIGHT + theRect->t) *BLOCK_WIDTH*lightmap_bytes);
			theRect->l = BLOCK_WIDTH;
			theRect->t = BLOCK_HEIGHT;
			theRect->h = 0;
			theRect->w = 0;
		}
		

		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		glBegin(GL_POLYGON);
		v = p->verts[0];
		for (i=0 ; i<p->numverts ; i++, v+= VERTEXSIZE)
		{
			txm		= 0; 
			tym		= 0;
			nv[0]	= v[0];
			nv[1]	= v[1];
			nv[2]	= v[2];
			vt[0]	= v[3];
			vt[1]	= v[4];

			if (rscripts[rs].usescroll) 
			{
				txm = realtime*rscripts[rs].scroll.xspeed;
				while (txm > 1 && (1-txm) > 0) txm=1-txm;
				while (txm < 0 && (1+txm) > 1) txm=1+txm;
				tym = realtime*rscripts[rs].scroll.yspeed;
				while (tym > 1 && (1-tym) > 0) tym=1-tym;
				while (tym < 0 && (1+tym) > 1) tym=1+tym;
			}
			if (rscripts[rs].useturb) 
			{
				float power, movediv;

				power	= rscripts[rs].turb.power * 0.05;	// Tomaz - Speed
				movediv = rscripts[rs].turb.movediv;
				os		= v[3]; 
				ot		= v[4];
				vt[0]	= os + sin((os * 0.1 + realtime) * power) * sin((ot * 0.1 + realtime)) / movediv;
				vt[1]	= ot + sin((ot * 0.1 + realtime) * power) * sin((os * 0.1 + realtime)) / movediv;
			} 

			if (rscripts[rs].usevturb)
			{
				float power;

				power	= rscripts[rs].vturb.power;
				nv[0]	= v[0] + sin(v[1] * 0.1 + realtime) * sin(v[2] * 0.1 + realtime) * power;
				nv[1]	= v[1] + sin(v[0] * 0.1 + realtime) * sin(v[2] * 0.1 + realtime) * power;
				nv[2]	= v[2];
			}

			qglMTexCoord2fSGIS_ARB (TEXTURE0_SGIS_ARB, vt[0]+txm, vt[1]+tym);
			qglMTexCoord2fSGIS_ARB (TEXTURE1_SGIS_ARB, v[5], v[6]);

			glVertex3fv (nv);
		}
		
		glEnd ();




		glDisable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		qglSelectTextureSGIS_ARB(TEXTURE0_SGIS_ARB);

		if(gl_envmap.value)
		{
			glDepthMask (true);

			if (rscripts[rs].flags.envmap)
			{
				EmitEnvMapPolys(s);
			}
		}

		//Tei rsgrass
		if (rscripts[rs].usegrass) {

			EmitRSGrassFx (s,rscripts[rs].grassmodel,rscripts[rs].grassdensity );			
			//EmitUnderwaterPolys(s);//
			//EmitDetailPolys(s);
		}
		if (rscripts[rs].usegrass2) {

			EmitRSGrassFx (s,rscripts[rs].grassmodel2,rscripts[rs].grassdensity2 );			
			//EmitUnderwaterPolys(s);//
			//EmitDetailPolys(s);
		}

		//Tei rsgrass

		if (rscripts[rs].usewater) {
			EmitUnderwaterPolys(s);//
		}
		//Tei rsgrass


		//Tei rsgrass
		if (rscripts[rs].usedetail) {

			EmitRSDetailPolys (s,rscripts[rs].mydetail,rscripts[rs].mydetailscale);			
		}
		//Tei rsgrass


		//Tei smoke rscripts
		if (rscripts[rs].usesmoke) {

			EmitRSSmokeFireFx (s,rscripts[rs].smokeheight,rscripts[rs].smokescale);			
		}
		//Tei smokerscripts

		//



		rs = rscripts[rs].nextstage;

		if (!rs)
			stage = false;
	}
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

}

/*
================
R_DrawBrushNoMTex
================
*/
void R_DrawBrushNoMTex (msurface_t *s)
{
	texture_t	*t;
	int			i;
	float		*v;
	glpoly_t	*p;
	int			j;
	glRect_t	*theRect;

	p = s->polys;

	t = R_TextureAnimation (s->texinfo->texture);
	glBindTexture (GL_TEXTURE_2D, t->gl_texturenum);

	glBegin (GL_POLYGON);
	v = p->verts[0];
	for (i=0 ; i<p->numverts ; i++, v+= VERTEXSIZE)
	{
		glTexCoord2f (v[3], v[4]);
		glVertex3fv (v);
	}
	glEnd ();

	glBindTexture (GL_TEXTURE_2D, lightmap_textures + s->lightmaptexturenum);
	i = s->lightmaptexturenum;
	if (lightmap_modified[i])
	{
		lightmap_modified[i] = false;
		theRect = &lightmap_rectchange[i];
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, theRect->t, BLOCK_WIDTH, theRect->h, lightmap_format, GL_UNSIGNED_BYTE, lightmaps+(i* BLOCK_HEIGHT + theRect->t) *BLOCK_WIDTH*lightmap_bytes);
		theRect->l = BLOCK_WIDTH;
		theRect->t = BLOCK_HEIGHT;
		theRect->h = 0;
		theRect->w = 0;
	}

	glBlendFunc(GL_ZERO, GL_SRC_COLOR);

	glBegin (GL_POLYGON);
	v = p->verts[0];
	for (j=0 ; j<p->numverts ; j++, v+= VERTEXSIZE)
	{
		glTexCoord2f (v[5], v[6]);
		glVertex3fv (v);
	}
	glEnd ();

	if (t->fullbrights != -1 && gl_fbr.value)
	{
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);

		glBindTexture (GL_TEXTURE_2D, t->fullbrights);

		p = s->polys;
		v = p->verts[0];

		glBegin (GL_POLYGON);

		for (i=0 ; i<p->numverts ; i++, v+= VERTEXSIZE)
		{
			glTexCoord2f (v[3], v[4]);
			glVertex3fv (v);
		}

		glEnd ();

		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	}

	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

extern qboolean hl_map;

/*
================
R_DrawBrushMTexTrans
================
*/
void R_DrawBrushMTexTrans (msurface_t *s, float alpha)
{
	glpoly_t	*p;
	float		*v;
	int			i;
	texture_t	*t;
	glRect_t	*theRect;

	glColor4f (1,1,1,alpha);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	if (hl_map)
		glEnable(GL_ALPHA_TEST);

	p = s->polys;

	t = R_TextureAnimation (s->texinfo->texture);

	glBindTexture (GL_TEXTURE_2D, t->gl_texturenum);

	qglSelectTextureSGIS_ARB(TEXTURE1_SGIS_ARB);
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glBindTexture (GL_TEXTURE_2D, lightmap_textures + s->lightmaptexturenum);

	i = s->lightmaptexturenum;
	if (lightmap_modified[i])
	{
		lightmap_modified[i] = false;
		theRect = &lightmap_rectchange[i];
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, theRect->t, BLOCK_WIDTH, theRect->h, lightmap_format, GL_UNSIGNED_BYTE, lightmaps+(i* BLOCK_HEIGHT + theRect->t) *BLOCK_WIDTH*lightmap_bytes);
		theRect->l = BLOCK_WIDTH;
		theRect->t = BLOCK_HEIGHT;
		theRect->h = 0;
		theRect->w = 0;
	}

	glBegin(GL_POLYGON);
	v = p->verts[0];
	for (i=0 ; i<p->numverts ; i++, v+= VERTEXSIZE)
	{
		qglMTexCoord2fSGIS_ARB (TEXTURE0_SGIS_ARB, v[3], v[4]);
		qglMTexCoord2fSGIS_ARB (TEXTURE1_SGIS_ARB, v[5], v[6]);
		glVertex3fv (v);
	}
	glEnd ();

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glDisable(GL_TEXTURE_2D);
	qglSelectTextureSGIS_ARB(TEXTURE0_SGIS_ARB);

	if (hl_map)
		glDisable(GL_ALPHA_TEST);

	glColor4f (1,1,1,1);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}

/*
================
R_DrawBrushNoMTexTrans
================
*/
void R_DrawBrushNoMTexTrans (msurface_t *s, float alpha)
{
	texture_t	*t;
	int			i;
	float		*v;
	glpoly_t	*p;
	glRect_t	*theRect;

	glColor4f (1,1,1,alpha);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	p = s->polys;

	t = R_TextureAnimation (s->texinfo->texture);
	glBindTexture (GL_TEXTURE_2D, t->gl_texturenum);
	glBegin (GL_POLYGON);
	v = p->verts[0];
	for (i=0 ; i<p->numverts ; i++, v+= VERTEXSIZE)
	{
		glTexCoord2f (v[3], v[4]);
		glVertex3fv (v);
	}
	glEnd ();
	glBindTexture (GL_TEXTURE_2D, lightmap_textures + s->lightmaptexturenum);
	i = s->lightmaptexturenum;
	if (lightmap_modified[i])
	{
		lightmap_modified[i] = false;
		theRect = &lightmap_rectchange[i];
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, theRect->t, BLOCK_WIDTH, theRect->h, lightmap_format, GL_UNSIGNED_BYTE, lightmaps+(i* BLOCK_HEIGHT + theRect->t) *BLOCK_WIDTH*lightmap_bytes);
		theRect->l = BLOCK_WIDTH;
		theRect->t = BLOCK_HEIGHT;
		theRect->h = 0;
		theRect->w = 0;
	}

	glBlendFunc(GL_ZERO, GL_SRC_COLOR);

	glBegin (GL_POLYGON);
	v = p->verts[0];
	for (i=0 ; i<p->numverts ; i++, v+= VERTEXSIZE)
	{
		glTexCoord2f (v[5], v[6]);
		glVertex3fv (v);
	}
	glEnd ();

	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f (1,1,1,1);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}

/*
================
R_RenderDynamicLightmaps
Multitexture
================
*/
void R_RenderDynamicLightmaps (msurface_t *s)
{
	byte		*base;
	int			maps;
	glRect_t    *theRect;
	int			smax, tmax;

	c_brush_polys++;

	if (s->flags & ( SURF_DRAWSKY | SURF_DRAWTURB ) )
		return;//XFX
		
	s->polys->chain = lightmap_polys[s->lightmaptexturenum];
	lightmap_polys[s->lightmaptexturenum] = s->polys;

	// check for lightmap modification
	for (maps = 0; maps < MAXLIGHTMAPS && s->styles[maps] != 255; maps++)
		if (d_lightstylevalue[s->styles[maps]] != s->cached_light[maps])
			goto dynamic;

	if (s->dlightframe == r_framecount	// dynamic this frame
		|| s->cached_dlight)			// dynamic previously
	{
dynamic:
		if (r_dynamic.value)
		{
			lightmap_modified[s->lightmaptexturenum] = true;
			theRect = &lightmap_rectchange[s->lightmaptexturenum];

			if (s->light_t < theRect->t) 
			{
				if (theRect->h)
					theRect->h += theRect->t - s->light_t;

				theRect->t = s->light_t;
			}

			if (s->light_s < theRect->l) 
			{
				if (theRect->w)
					theRect->w += theRect->l - s->light_s;

				theRect->l = s->light_s;
			}

			smax = (s->extents[0]>>4)+1;
			tmax = (s->extents[1]>>4)+1;

			if ((theRect->w + theRect->l) < (s->light_s + smax))
				theRect->w = (s->light_s-theRect->l)+smax;

			if ((theRect->h + theRect->t) < (s->light_t + tmax))
				theRect->h = (s->light_t-theRect->t)+tmax;

			base = lightmaps + s->lightmaptexturenum*lightmap_bytes*BLOCK_WIDTH*BLOCK_HEIGHT;
			base += s->light_t * BLOCK_WIDTH * lightmap_bytes + s->light_s * lightmap_bytes;

			R_BuildLightMap (s, base, BLOCK_WIDTH*lightmap_bytes);
		}
	}
}

/*
================
R_DrawWaterSurfaces
================
*/
void R_DrawWaterSurfaces (void)
{
	msurface_t	*s;

	if (!waterchain)
		return;

    glLoadMatrixf (r_world_matrix);

	if (r_wateralpha.value < 1.0) 
	{
		glColor4f (1,1,1,r_wateralpha.value);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	}

	for ( s = waterchain ; s ; s=s->texturechain) 
	{
		glBindTexture (GL_TEXTURE_2D, s->texinfo->texture->gl_texturenum);
		EmitWaterPolys (s);
	}

	waterchain = NULL;

	if (r_wateralpha.value < 1.0) 
	{
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glColor4f (1,1,1,1);
	}
}

float	r_world_matrix[16];
extern qboolean wireframe;

/*
=================
R_SetupBrushPolys
=================
*/
void R_Snow (vec3_t min, vec3_t max, int flakes);


void R_SetupBrushPolys (entity_t *e)
{
	int			k;
	vec3_t		mins, maxs;
	int			i;
	msurface_t	*psurf;
	float		dot;
	mplane_t	*pplane;
	model_t		*clmodel;
	qboolean	rotated;

	clmodel = e->model;

	// Tei autosnow
	//if (!clmodel) 
	//	R_Snow (clmodel->mins, clmodel->maxs, 10);
	//  Tei autosnow


	if (e->angles[0] || e->angles[1] || e->angles[2])
	{
		rotated = true;
		for (i=0 ; i<3 ; i++)
		{
			mins[i] = e->origin[i] - clmodel->radius;
			maxs[i] = e->origin[i] + clmodel->radius;
		}
	}
	else
	{
		rotated = false;
		VectorAdd (e->origin, clmodel->mins, mins);
		VectorAdd (e->origin, clmodel->maxs, maxs);
	}

	if (R_CullBox (mins, maxs))
		return;

	memset (lightmap_polys, 0, sizeof(lightmap_polys));

	VectorSubtract (r_refdef.vieworg, e->origin, modelorg);

	if (rotated)
	{
		vec3_t	temp;
		vec3_t	forward, right, up;

		VectorCopy (modelorg, temp);
		AngleVectors (e->angles, forward, right, up);
		modelorg[0] = DotProduct (temp, forward);
		modelorg[1] = -DotProduct (temp, right);
		modelorg[2] = DotProduct (temp, up);
	}

	psurf = &clmodel->surfaces[clmodel->firstmodelsurface];

// calculate dynamic lighting for bmodel if it's not an
// instanced model
	if (clmodel->firstmodelsurface != 0)
	{
		for (k=0 ; k<MAX_DLIGHTS ; k++)
		{
			if ((cl_dlights[k].die < cl.time) ||
				(!cl_dlights[k].radius))
				continue;

			R_MarkLightsNoVis (&cl_dlights[k], 1<<k, clmodel->nodes + clmodel->hulls[0].firstclipnode);
		}
	}

    glPushMatrix ();

	glTranslatef (e->origin[0], e->origin[1], e->origin[2]);
	glRotatef (e->angles[1], 0, 0, 1);
	glRotatef (e->angles[0], 0, 1, 0);	// stupid quake bug
	glRotatef (e->angles[2], 1, 0, 0);

	//
	// draw texture
	//
	for (i=0 ; i<clmodel->nummodelsurfaces ; i++, psurf++)
	{
	// find which side of the node we are on
		pplane = psurf->plane;

		dot = PlaneDiff (modelorg, pplane);

	// draw the polygon
		if (((psurf->flags & SURF_PLANEBACK) && (dot < -BACKFACE_EPSILON)) ||
			(!(psurf->flags & SURF_PLANEBACK) && (dot > BACKFACE_EPSILON)))
		{
			if (wireframe)
			{
				R_DrawLinePolys (psurf);
				continue;
			}

			if (gl_wireonly.value)
			{
				R_DrawLinePolys (psurf);
				continue;
			}

			R_RenderDynamicLightmaps(psurf);

			if (gl_mtexable)
			{
				if ((e->alpha != 1) || (psurf->texinfo->texture->transparent))
					R_DrawBrushMTexTrans (psurf, e->alpha);
				else
					R_DrawBrushMTex (psurf);

			}
			else
			{
				if ((e->alpha != 1) || (psurf->texinfo->texture->transparent))
				{
					R_DrawBrushNoMTexTrans (psurf, e->alpha);
				}
				else
				{
					R_DrawBrushNoMTex (psurf);
				}
			}
			if (gl_showpolys.value)
			{
				R_DrawLinePolys (psurf);
			}
		}
	}

	glPopMatrix ();
}

/*
=============================================================

	WORLD MODEL

=============================================================
*/

/*
================
R_RecursiveWorldNode
================
*/
void R_RecursiveWorldNode (mnode_t *node)
{
	int			c, side;
	mplane_t	*plane;
	msurface_t	*surf, **mark;
	mleaf_t		*pleaf;
	double		dot;

	if (node->contents == CONTENTS_SOLID)
		return;

	if (node->visframe != r_visframecount)
		return;
	if (R_CullBox (node->mins, node->maxs))
		return;

// if a leaf node, draw stuff
	if (node->contents < 0)
	{
		pleaf = (mleaf_t *)node;

		mark = pleaf->firstmarksurface;
		c = pleaf->nummarksurfaces;

		if (c)
		{
			do
			{
				(*mark)->visframe = r_framecount;
				mark++;
			} while (--c);
		}

	// deal with model fragments in this leaf
		if (pleaf->efrags)
			R_StoreEfrags (&pleaf->efrags);

		return;
	}

// node is just a decision point, so go down the apropriate sides

// find which side of the node we are on
	plane = node->plane;
	dot = PlaneDiff (modelorg, plane);

	if (dot >= 0)
		side = 0;
	else
		side = 1;

// recurse down the children, front side first
	R_RecursiveWorldNode (node->children[side]);

// draw stuff
	c = node->numsurfaces;

	if (c)
	{
		surf = cl.worldmodel->surfaces + node->firstsurface;

		if (dot < 0 -BACKFACE_EPSILON)
			side = SURF_PLANEBACK;
		else if (dot > BACKFACE_EPSILON)
			side = 0;
		{
			for ( ; c ; c--, surf++)
			{
				if (surf->visframe != r_framecount)
					continue;

				// don't backface underwater surfaces, because they warp
				if (!(surf->flags & SURF_UNDERWATER) && ( (dot < 0) ^ !!(surf->flags & SURF_PLANEBACK)) )
					continue;		// wrong side

				if (wireframe)
				{
					R_DrawLinePolys (surf);
					continue;
				}

				if (gl_wireonly.value)
				{
					R_DrawLinePolys (surf);
					continue;
				}

				if (surf->flags & SURF_DRAWSKY) 
				{
					surf->texturechain = skychain;
					skychain = surf;
				} 
					
				else if (surf->flags & SURF_DRAWTURB) 
				{
					surf->texturechain = waterchain;
					waterchain = surf;
				}
				// Tei WaterMap
				//else if (surf->flags & SURF_DRAWTURB || watermap.value) 
				//{
				//	surf->texturechain = waterchain;
				//	waterchain = surf;
				//}
				//Tei WaterMap

// MIRRORS!!
				else if (r_mirroralpha.value < 1.0 && !mirror_render && surf->flags & SURF_MIRROR)
				{
					mirror = true;
					surf->texturechain = mirrorchain;
					mirrorchain = surf;
					continue;
				}
// END
				else if (gl_mtexable)
				{
					R_DrawBrushMTex (surf);
				}

				else
				{
					R_DrawBrushNoMTex (surf);
				}

				if (gl_showpolys.value)
				{
					R_DrawLinePolys (surf);
				}

				R_RenderDynamicLightmaps(surf);
			}
		}
	}

// recurse down the back side
	R_RecursiveWorldNode (node->children[!side]);
}

extern char skyname[];

/*
=============
R_DrawWorld
=============
*/

void R_DrawWorld (void)
{
	entity_t	ent;

	memset (&ent, 0, sizeof(ent));
	ent.model = cl.worldmodel;

	VectorCopy (r_refdef.vieworg, modelorg);

	currententity = &ent;

	memset (lightmap_polys, 0, sizeof(lightmap_polys));

	R_RecursiveWorldNode (cl.worldmodel->nodes);

	if (gl_wireframe.value)
	{
		wireframe = true;
		glDisable (GL_DEPTH_TEST);
		R_RecursiveWorldNode (cl.worldmodel->nodes);
		glEnable (GL_DEPTH_TEST);
		wireframe = false;
	}

	if (skychain) 
	{
		if (skyname[0])
		{
			R_DrawSkyBox ();
			if (r_dosky.value)
				R_DrawSky(skychain);
		}
		else
		{
			if (r_dosky.value)
				R_DrawSky(skychain);
		}
		skychain = NULL;
	}
}	


/*
===============
R_MarkLeaves
===============
*/
void R_MarkLeaves (void)
{
	byte	*vis;
	mnode_t	*node;
	int		i;
	byte	solid[4096];

	if (r_oldviewleaf == r_viewleaf && !r_novis.value)
		return;
	
	if (mirror)
	{
		return;
	}

	r_visframecount++;
	r_oldviewleaf = r_viewleaf;

	if (r_novis.value)
	{
		vis = solid;
		memset (solid, 0xff, (cl.worldmodel->numleafs+7)>>3);
	}
	else
		vis = Mod_LeafPVS (r_viewleaf, cl.worldmodel);
		
	for (i=0 ; i<cl.worldmodel->numleafs ; i++)
	{
		if (vis[i>>3] & (1<<(i&7)))
		{
			node = (mnode_t *)&cl.worldmodel->leafs[i+1];
			do
			{
				if (node->visframe == r_visframecount)
					break;
				node->visframe = r_visframecount;
				node = node->parent;
			} while (node);
		}
	}
}



/*
=============================================================================

  LIGHTMAP ALLOCATION

=============================================================================
*/

// returns a texture number and the position inside it
int AllocBlock (int w, int h, int *x, int *y)
{
	int		i, j;
	int		best, best2;
	int		texnum;

	for (texnum=0 ; texnum<MAX_LIGHTMAPS ; texnum++)
	{
		best = BLOCK_HEIGHT;

		for (i=0 ; i<BLOCK_WIDTH-w ; i++)
		{
			best2 = 0;

			for (j=0 ; j<w ; j++)
			{
				if (allocated[texnum][i+j] >= best)
					break;
				if (allocated[texnum][i+j] > best2)
					best2 = allocated[texnum][i+j];
			}
			if (j == w)
			{	// this is a valid spot
				*x = i;
				*y = best = best2;
			}
		}

		if (best + h > BLOCK_HEIGHT)
			continue;

		for (i=0 ; i<w ; i++)
			allocated[texnum][*x + i] = best + h;

		return texnum;
	}

	Host_Error ("AllocBlock: full, %i unable to find room for %i by %i lightmap",texnum, w, h);
	return 0;
}


mvertex_t	*r_pcurrentvertbase;
model_t		*currentmodel;

int	nColinElim;

/*
================
BuildSurfaceDisplayList
================
*/
void BuildSurfaceDisplayList (msurface_t *fa)
{
	int			i, lindex, lnumverts;
	medge_t		*pedges, *r_pedge;
	int			vertpage;
	float		*vec;
	float		s, t;
	glpoly_t	*poly;

// reconstruct the polygon
	fa->visframe = 0;
	pedges = currentmodel->edges;
	lnumverts = fa->numedges;
	vertpage = 0;

	//
	// draw texture
	//
	poly = Hunk_Alloc (sizeof(glpoly_t) + (lnumverts-4) * VERTEXSIZE*sizeof(float));
	poly->next = fa->polys;
	poly->flags = fa->flags;
	fa->polys = poly;
	poly->numverts = lnumverts;

	for (i=0 ; i<lnumverts ; i++)
	{
		lindex = currentmodel->surfedges[fa->firstedge + i];

		if (lindex > 0)
		{
			r_pedge = &pedges[lindex];
			vec = r_pcurrentvertbase[r_pedge->v[0]].position;
		}
		else
		{
			r_pedge = &pedges[-lindex];
			vec = r_pcurrentvertbase[r_pedge->v[1]].position;
		}
		s = DotProduct (vec, fa->texinfo->vecs[0]) + fa->texinfo->vecs[0][3];
		s /= fa->texinfo->texture->width;

		t = DotProduct (vec, fa->texinfo->vecs[1]) + fa->texinfo->vecs[1][3];
		t /= fa->texinfo->texture->height;

		VectorCopy (vec, poly->verts[i]);
		poly->verts[i][3] = s;
		poly->verts[i][4] = t;

		//
		// lightmap texture coordinates
		//
		s = DotProduct (vec, fa->texinfo->vecs[0]) + fa->texinfo->vecs[0][3];
		s -= fa->texturemins[0];
		s += fa->light_s*16;
		s += 8;
		s /= BLOCK_WIDTH*16; //fa->texinfo->texture->width;

		t = DotProduct (vec, fa->texinfo->vecs[1]) + fa->texinfo->vecs[1][3];
		t -= fa->texturemins[1];
		t += fa->light_t*16;
		t += 8;
		t /= BLOCK_HEIGHT*16; //fa->texinfo->texture->height;

		poly->verts[i][5] = s;
		poly->verts[i][6] = t;
	}

	//
	// remove co-linear points - Ed
	//
	if (!gl_keeptjunctions.value && !(fa->flags & SURF_UNDERWATER) )
	{
		for (i = 0 ; i < lnumverts ; ++i)
		{
			vec3_t v1, v2;
			float *prev, *this, *next;

			prev = poly->verts[(i + lnumverts - 1) % lnumverts];
			this = poly->verts[i];
			next = poly->verts[(i + 1) % lnumverts];

			VectorSubtract( this, prev, v1 );
			VectorNormalize( v1 );
			VectorSubtract( next, prev, v2 );
			VectorNormalize( v2 );

			// skip co-linear points
			#define COLINEAR_EPSILON 0.001
			if ((fabs( v1[0] - v2[0] ) <= COLINEAR_EPSILON) &&
				(fabs( v1[1] - v2[1] ) <= COLINEAR_EPSILON) && 
				(fabs( v1[2] - v2[2] ) <= COLINEAR_EPSILON))
			{
				int j;
				for (j = i + 1; j < lnumverts; ++j)
				{
					int k;
					for (k = 0; k < VERTEXSIZE; ++k)
						poly->verts[j - 1][k] = poly->verts[j][k];
				}
				--lnumverts;
				++nColinElim;
				// retry next vertex next time, which is now current vertex
				--i;
			}
		}
	}
	poly->numverts = lnumverts;

}

/*
========================
GL_CreateSurfaceLightmap
========================
*/
void GL_CreateSurfaceLightmap (msurface_t *surf)
{
	int		smax, tmax;
	byte	*base;

	if (surf->flags & (SURF_DRAWSKY|SURF_DRAWTURB))
		return;

	smax = (surf->extents[0]>>4)+1;
	tmax = (surf->extents[1]>>4)+1;

	surf->lightmaptexturenum = AllocBlock (smax, tmax, &surf->light_s, &surf->light_t);
	base = lightmaps + surf->lightmaptexturenum*lightmap_bytes*BLOCK_WIDTH*BLOCK_HEIGHT;
	base += (surf->light_t * BLOCK_WIDTH + surf->light_s) * lightmap_bytes;
	R_BuildLightMap (surf, base, BLOCK_WIDTH*lightmap_bytes);
}


/*
==================
GL_BuildLightmaps

Builds the lightmap texture
with all the surfaces from all brush models
==================
*/
void GL_BuildLightmaps (void)
{
	int		i, j;
	model_t	*m;

	memset (allocated, 0, sizeof(allocated));

	r_framecount = 1;		// no dlightcache

	if (!lightmap_textures)
	{
		lightmap_textures = texture_extension_number;
		texture_extension_number += MAX_LIGHTMAPS;
	}

	for (j=1 ; j<MAX_MODELS ; j++)
	{
		m = cl.model_precache[j];
		if (!m)
			break;
		if (m->name[0] == '*')
			continue;
		r_pcurrentvertbase = m->vertexes;
		currentmodel = m;
		for (i=0 ; i<m->numsurfaces ; i++)
		{
			GL_CreateSurfaceLightmap (m->surfaces + i);
			if ( m->surfaces[i].flags & SURF_DRAWTURB )
				continue;
			if ( m->surfaces[i].flags & SURF_DRAWSKY )
				continue;
			BuildSurfaceDisplayList (m->surfaces + i);
		}
	}
 	if (gl_mtexable)
		qglSelectTextureSGIS_ARB(TEXTURE1_SGIS_ARB);

	//
	// upload all lightmaps that were filled
	//
	for (i=0 ; i<MAX_LIGHTMAPS ; i++)
	{
		if (!allocated[i][0])
			break;		// no more used
		lightmap_modified[i] = false;
		lightmap_rectchange[i].l = BLOCK_WIDTH;
		lightmap_rectchange[i].t = BLOCK_HEIGHT;
		lightmap_rectchange[i].w = 0;
		lightmap_rectchange[i].h = 0;
		glBindTexture (GL_TEXTURE_2D, lightmap_textures + i);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D (GL_TEXTURE_2D, 0, lightmap_bytes
			, BLOCK_WIDTH, BLOCK_HEIGHT, 0, lightmap_format, GL_UNSIGNED_BYTE, lightmaps+i*BLOCK_WIDTH*BLOCK_HEIGHT*lightmap_bytes);
	}

 	if (gl_mtexable)
		qglSelectTextureSGIS_ARB(TEXTURE0_SGIS_ARB);
}
