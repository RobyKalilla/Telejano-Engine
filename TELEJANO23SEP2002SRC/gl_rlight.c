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
// r_light.c

#include "quakedef.h"

int	r_dlightframecount = 0;

mplane_t		*lightplane;

vec3_t			lightspot;
vec3_t			lightcolor;

//LordHavoc Coronas


#define MAX_TEXTUREUNITS 8
#define MAX_TEXTUREUNITS 8 //unknom 



typedef float vec_t;
//extern int c_meshtris, c_meshs, c_transtris, c_transmeshs;


typedef struct
{
	int transparent;
	int depthwrite; // force depth writing enabled even if polygon is not opaque
	int depthdisable; // disable depth read/write entirely
	int blendfunc1;
	int blendfunc2;
	int numtriangles;
	int *index;
	int numverts;
	float *vertex;
	int vertexstep;
	float *color;
	int colorstep;
	float cr, cg, cb, ca; // if color is NULL, these are used for all vertices
	int tex[MAX_TEXTUREUNITS];
	float *texcoords[MAX_TEXTUREUNITS];
	int texcoordstep[MAX_TEXTUREUNITS];
	float texrgbscale[MAX_TEXTUREUNITS]; // used only if COMBINE is present
}
rmeshinfo_t;

typedef struct
{
	vec3_t origin;
	vec_t cullradius2; // only for culling comparisons, squared version
	vec3_t light; // the brightness of the light
	vec_t cullradius; // only for culling comparisons
	vec_t lightsubtract; // to avoid sudden brightness change at cullradius, subtract this
	entity_t *ent; // owner of this light
}
rdlight_t;



static int lightcorona;
static int coronapolyindex[6] = {0, 1, 2, 0, 2, 3};

#if 0
void r_light_start(void)
{
	float dx, dy;
	int x, y, a;
	byte pixels[32][32][4];
	//lighttexturepool = R_AllocTexturePool();
	for (y = 0;y < 32;y++)
	{
		dy = (y - 15.5f) * (1.0f / 16.0f);
		for (x = 0;x < 32;x++)
		{
			dx = (x - 15.5f) * (1.0f / 16.0f);
			a = ((1.0f / (dx * dx + dy * dy + 0.2f)) - (1.0f / (1.0f + 0.2))) * 64.0f / (1.0f / (1.0f + 0.2));
			a = bound(0, a, 255);
			pixels[y][x][0] = 255;
			pixels[y][x][1] = 255;
			pixels[y][x][2] = 255;
			pixels[y][x][3] = a;
		}
	}
	lightcorona = GL_LoadTexture("lightcorona", 32, 32, &pixels[0][0][0], true, true, 4);
}
#endif
/*
float TraceLine (vec3_t start, vec3_t end, vec3_t impact, vec3_t normal, int contents, int hitbmodels)
{
	double maxfrac, startd[3], endd[3];
	trace_t trace;

// FIXME: broken, fix it
//	if (impact == NULL && normal == NULL && contents == 0)
//		return SV_TestLine (cl.worldmodel->hulls, 0, start, end);

	VectorCopy(start, startd);
	VectorCopy(end, endd);

	//Mod_CheckLoaded(cl.worldmodel);
	memset (&trace, 0, sizeof(trace));
	VectorCopy (endd, trace.endpos);
	trace.fraction = 1;
	trace.startcontents = contents;
	VectorCopy(startd, RecursiveHullCheckInfo.start);
	VectorSubtract(endd, startd, RecursiveHullCheckInfo.dist);
	RecursiveHullCheckInfo.hull = cl.worldmodel->hulls;
	RecursiveHullCheckInfo.trace = &trace;
	SV_RecursiveHullCheck (0, 0, 1, startd, endd);
	if (impact)
		VectorCopy (trace.endpos, impact);
	if (normal)
		VectorCopy (trace.plane.normal, normal);
	traceline_endcontents = trace.endcontents;
	maxfrac = trace.fraction;

	if (hitbmodels && traceline_entities)
	{
		int n;
		entity_render_t *ent;
		double start2[3], end2[3], tracemins[3], tracemaxs[3];
		tracemins[0] = min(start[0], end[0]);
		tracemaxs[0] = max(start[0], end[0]);
		tracemins[1] = min(start[1], end[1]);
		tracemaxs[1] = max(start[1], end[1]);
		tracemins[2] = min(start[2], end[2]);
		tracemaxs[2] = max(start[2], end[2]);

		// look for embedded bmodels
		for (n = 0;n < traceline_entities;n++)
		{
			ent = traceline_entity[n];
			if (ent->mins[0] > tracemaxs[0] || ent->maxs[0] < tracemins[0]
			 || ent->mins[1] > tracemaxs[1] || ent->maxs[1] < tracemins[1]
			 || ent->mins[2] > tracemaxs[2] || ent->maxs[2] < tracemins[2])
			 	continue;

			softwaretransformforentity(ent);
			softwareuntransform(start, start2);
			softwareuntransform(end, end2);

			memset (&trace, 0, sizeof(trace));
			VectorCopy (end2, trace.endpos);
			trace.fraction = 1;
			trace.startcontents = contents;
			VectorCopy(start2, RecursiveHullCheckInfo.start);
			VectorSubtract(end2, start2, RecursiveHullCheckInfo.dist);
			RecursiveHullCheckInfo.hull = ent->model->hulls;
			RecursiveHullCheckInfo.trace = &trace;
			SV_RecursiveHullCheck (ent->model->hulls->firstclipnode, 0, 1, start2, end2);

			if (trace.allsolid || trace.startsolid || trace.fraction < maxfrac)
			{
				maxfrac = trace.fraction;
				if (impact)
				{
					softwaretransform(trace.endpos, impact);
				}
				if (normal)
				{
					softwaretransformdirection(trace.plane.normal, normal);
				}
				traceline_endcontents = trace.endcontents;
			}
		}
	}
	return maxfrac;
}
*/

extern int particle_tex;
extern int flare_tex;
extern int circle_tex;
extern int flareglow_tex;
//Tei XFX code

void R_DrawCorona(vec3_t pos, float radius , int visible, int alfa)
{
	float viewdist;
	int	texnum;
	
	vec3_t		up				= { vup[0] * 0.75f, vup[1] * 0.75f, vup[2] * 0.75f };
	vec3_t		right			= { vright[0] * 0.75f, vright[1] * 0.75f, vright[2] * 0.75f };
	vec3_t		coord[4];


	viewdist = DotProduct(r_origin, vpn);
	
	VectorAdd( up, right, coord[0] );
	VectorSubtract( right, up, coord[1] );
	VectorNegate( coord[0], coord[2] );
	VectorNegate( coord[1], coord[3] );

	texnum		= flareglow_tex;//particle_tex;//lightcorona;//circle_tex;//flare_tex;//
	//texnum		= circle_tex;//lightcorona;//circle_tex;//flare_tex;//

	if (!visible)
		glDisable(GL_DEPTH_TEST);
	glBindTexture( GL_TEXTURE_2D, texnum );
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
		
	//glDepthMask(false);

	glBindTexture( GL_TEXTURE_2D, texnum );
		
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	
	//glColor4ub( 255, 255, 255, (GLubyte)alfa);
	glColor4ub( 255, 255, 255, alfa);
	//glColor4ub( 0, 0, 0, alfa);
			
	glPushMatrix();

	glTranslatef( pos[0] , pos[1], pos[2]   );	

	//radius = radius * 0.1 * viewdist * 0.01f ;

	glScalef( radius , radius, radius);//Tei , scaled flares

	glBegin( GL_QUADS );
	{				
		glTexCoord2f(	0,		1	);
		glVertex3fv(	coord[0]	);
			
		glTexCoord2f(	0,		0	);
		glVertex3fv(	coord[1]	);
						
		glTexCoord2f(	1,		0	); 
		glVertex3fv(	coord[2]	);
						
		glTexCoord2f(	1,		1	);
		glVertex3fv(	coord[3]	);
						
		glEnd();
	}
	glPopMatrix();
			
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	
	//glDepthMask(true);
	
	glColor4f(1,1,1,1);

	if( !visible )
		glEnable(GL_DEPTH_TEST);
}



/*
void R_DrawCorona2(vec3_t pos, float radius , int visible, int alfa, int texnum)
{
	float viewdist;
	//int	texnum;
	
	vec3_t		up				= { vup[0] * 0.75f, vup[1] * 0.75f, vup[2] * 0.75f };
	vec3_t		right			= { vright[0] * 0.75f, vright[1] * 0.75f, vright[2] * 0.75f };
	vec3_t		coord[4];


	viewdist = DotProduct(r_origin, vpn);
	
	VectorAdd( up, right, coord[0] );
	VectorSubtract( right, up, coord[1] );
	VectorNegate( coord[0], coord[2] );
	VectorNegate( coord[1], coord[3] );

	//texnum		= circle_tex;//particle_tex;//lightcorona;//circle_tex;//flare_tex;//

	if (!visible)
		glDisable(GL_DEPTH_TEST);
	glBindTexture( GL_TEXTURE_2D, texnum );
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
		
	//glDepthMask(false);

	glBindTexture( GL_TEXTURE_2D, texnum );
		
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	
	glColor4ub( 255, 255, 255, alfa);
	//glColor4ub( 0, 0, 0, alfa);
			
	glPushMatrix();

	glTranslatef( pos[0] , pos[1], pos[2]   );	

	//radius = radius * 0.1 * viewdist * 0.01f ;

	glScalef( radius , radius, radius);

	glBegin( GL_QUADS );
	{				
		glTexCoord2f(	0,		1	);
		glVertex3fv(	coord[0]	);
			
		glTexCoord2f(	0,		0	);
		glVertex3fv(	coord[1]	);
						
		glTexCoord2f(	1,		0	); 
		glVertex3fv(	coord[2]	);
						
		glTexCoord2f(	1,		1	);
		glVertex3fv(	coord[3]	);
						
		glEnd();
	}
	glPopMatrix();
			
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	
	//glDepthMask(true);
	
	glColor4f(1,1,1,1);

	if( !visible )
		glEnable(GL_DEPTH_TEST);
}
*/


/*
void R_DrawCoronas(void)
{
	int i;
	//rmeshinfo_t m;
	//float tvxyz[4][4], tvst[4][2], diff[3];
	dlight_t *dl;
	float radius,texnum, alpha, scale, dist, viewdist;
	vec3_t		up				= { vup[0] * 0.75f, vup[1] * 0.75f, vup[2] * 0.75f };
	vec3_t		right			= { vright[0] * 0.75f, vright[1] * 0.75f, vright[2] * 0.75f };
	
	vec3_t		coord[4];


	viewdist = DotProduct(r_origin, vpn);
	
	dl =  cl_dlights;

	for (i = 0;i < MAX_DLIGHTS ;i++)
	{
		dl = &cl_dlights[i];
		if (dl->die < cl.time)
		{
			dist = (DotProduct(dl->origin, vpn) - viewdist);
			if (dist >= 24.0f)
			{
	
				VectorAdd( up, right, coord[0] );
				VectorSubtract( right, up, coord[1] );
				VectorNegate( coord[0], coord[2] );
				VectorNegate( coord[1], coord[3] );

				scale = dl->minlight * 0.25f;
				radius = dl->radius * scale;
				scale = scale * viewdist;

				texnum		= particle_tex;
				glDisable(GL_DEPTH_TEST);
				glBindTexture( GL_TEXTURE_2D, texnum );
		
				glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
		
				//glDepthMask(false);
		
				glBindTexture( GL_TEXTURE_2D, texnum );
		
				glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			
				//alpha	= 50;
		
				glColor4ub( 255, 255, 255, 200);
			
				glPushMatrix();

				glTranslatef( dl->origin[0], dl->origin[1], dl->origin[2] );
				
				glScalef( dl->radius , dl->radius, dl->radius);//Tei

				glBegin( GL_QUADS );
					
				glTexCoord2f(	0,		1	);
				glVertex3fv(	coord[0]	);
						
				glTexCoord2f(	0,		0	);
				glVertex3fv(	coord[1]	);
						
				glTexCoord2f(	1,		0	); 
				glVertex3fv(	coord[2]	);
					
				glTexCoord2f(	1,		1	);
				glVertex3fv(	coord[3]	);
					
				glEnd();
				glPopMatrix();
			
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
				//glDepthMask(true);
				glColor4f(1,1,1,1);
				glEnable(GL_DEPTH_TEST);

			}
		}
	}
}
*/

//Tei XFX code


//LordHavoc Coronas


/*
==================
R_AnimateLight
==================
*/
void R_AnimateLight (void)
{
	int			i,j,k;


	
//
// light animations
// 'm' is normal light, 'a' is no light, 'z' is double bright
	i = (int)(cl.time*10);
	for (j=0 ; j<MAX_LIGHTSTYLES ; j++)
	{
		if (!cl_lightstyle[j].length)
		{
			d_lightstylevalue[j] = 256;
			continue;
		}
		k = i % cl_lightstyle[j].length;
		k = cl_lightstyle[j].map[k] - 'a';
#if 1 //Telejano RC2
		k = k*25;//Tei more light 24
#else
		k = k*22;
#endif
		d_lightstylevalue[j] = k;
	}	
}

/*
=============================================================================

DYNAMIC LIGHTS

=============================================================================
*/

/*
=============
R_MarkLights
=============
*/
void R_MarkLightsNoVis (dlight_t *light, int bit, mnode_t *node)
{
	mplane_t	*splitplane;
	float		dist;
	msurface_t	*surf;
	int			i;
	float		l, maxdist;
	int			j, s, t;
	vec3_t		impact;

loc0:
	if (node->contents < 0)
		return;

	splitplane = node->plane;
	dist = PlaneDiff (light->origin, splitplane);

	if (dist > light->radius)
	{
		node = node->children[0];
		goto loc0;
	}
	if (dist < -light->radius)
	{
		node = node->children[1];
		goto loc0;
	}
		
// mark the polygons
	maxdist = light->radius*light->radius;

	surf = cl.worldmodel->surfaces + node->firstsurface;
	for (i = 0; i < node->numsurfaces; i++, surf++)
	{
		for (j=0 ; j<3 ; j++)
			impact[j] = light->origin[j] - surf->plane->normal[j]*dist;

		// clamp center of light to corner and check brightness
		l = DotProduct (impact, surf->texinfo->vecs[0]) + surf->texinfo->vecs[0][3] - surf->texturemins[0];
		s = l+0.5;
		if (s < 0) 
			s = 0;
		else if (s > surf->extents[0]) 
			s = surf->extents[0];
		s = l - s;
		l = DotProduct (impact, surf->texinfo->vecs[1]) + surf->texinfo->vecs[1][3] - surf->texturemins[1];
		t = l+0.5;
		if (t < 0) 
			t = 0;
		else if (t > surf->extents[1]) 
			t = surf->extents[1];
		t = l - t;
		// compare to minimum light
		if ((s*s+t*t+dist*dist) < maxdist)
		{
			if (surf->dlightframe != r_framecount) // not dynamic until now
			{
				surf->dlightbits = bit;
				surf->dlightframe = r_framecount;
			}
			else // already dynamic
				surf->dlightbits |= bit;
		}
	}

	if (node->children[0]->contents >= 0)
	{
		if (node->children[1]->contents >= 0)
		{
			R_MarkLightsNoVis (light, bit, node->children[0]);
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
}

void R_MarkLights (dlight_t *light, int bit, model_t *model)
{
	mleaf_t *pvsleaf = Mod_PointInLeaf (light->origin, model);
	
	if (!pvsleaf->compressed_vis)
	{
		// no vis info, so make all visible
		R_MarkLightsNoVis(light, bit, model->nodes + model->hulls[0].firstclipnode);
		return;
	}
	else
	{
		int		i, k, l, m, c;
		msurface_t *surf, **mark;
		mleaf_t *leaf;
		byte	*in = pvsleaf->compressed_vis;
		int		row = (model->numleafs+7)>>3;
		float	low[3], high[3], radius, dist, maxdist;

		r_dlightframecount++;

		radius = light->radius * 2;

		low[0] = light->origin[0] - radius;
		low[1] = light->origin[1] - radius;
		low[2] = light->origin[2] - radius;
		high[0] = light->origin[0] + radius;
		high[1] = light->origin[1] + radius;
		high[2] = light->origin[2] + radius;

		// for comparisons to minimum acceptable light
		maxdist = radius*radius;
		//maxdist = radius*radius * 0.5f;//Tei

		k = 0;
		while (k < row)
		{
			c = *in++;
			if (c)
			{
				l = model->numleafs - (k << 3);
				if (l > 8)
					l = 8;
				for (i=0 ; i<l ; i++)
				{
					if (c & (1<<i))
					{
						leaf = &model->leafs[(k << 3)+i+1];
						if (leaf->visframe != r_visframecount)
							continue;
						if (leaf->contents == CONTENTS_SOLID)
							continue;
						// if out of the light radius, skip
						if (leaf->mins[0] > high[0] || leaf->maxs[0] < low[0]
						 || leaf->mins[1] > high[1] || leaf->maxs[1] < low[1]
						 || leaf->mins[2] > high[2] || leaf->maxs[2] < low[2])
							continue; 
						if ((m = leaf->nummarksurfaces))
						{
							mark = leaf->firstmarksurface;
							do
							{
								surf = *mark++;

								if (surf->lightframe == r_dlightframecount)
									continue;

								surf->lightframe = r_dlightframecount;
								dist = PlaneDiff(light->origin, surf->plane);
								if (surf->flags & SURF_PLANEBACK)
									dist = -dist;
								// LordHavoc: make sure it is infront of the surface and not too far away
								if (dist >= -0.25f && (dist < radius))
								{
									int d;
									float dist2, impact[3];

									dist2 = dist * dist;

									impact[0] = light->origin[0] - surf->plane->normal[0] * dist;
									impact[1] = light->origin[1] - surf->plane->normal[1] * dist;
									impact[2] = light->origin[2] - surf->plane->normal[2] * dist;

									d = DotProduct (impact, surf->texinfo->vecs[0]) + surf->texinfo->vecs[0][3] - surf->texturemins[0];

									if (d < 0)
									{
										dist2 += d * d;
										if (dist2 >= maxdist)
											continue;
									}
									else
									{
										d -= surf->extents[0] + 16;
										if (d > 0)
										{
											dist2 += d * d;
											if (dist2 >= maxdist)
												continue;
										}
									}

									d = DotProduct (impact, surf->texinfo->vecs[1]) + surf->texinfo->vecs[1][3] - surf->texturemins[1];

									if (d < 0)
									{
										dist2 += d * d;
										if (dist2 >= maxdist)
											continue;
									}
									else
									{
										d -= surf->extents[1] + 16;
										if (d > 0)
										{
											dist2 += d * d;
											if (dist2 >= maxdist)
												continue;
										}
									}

									if (surf->dlightframe != r_framecount) // not dynamic until now
									{
										surf->dlightbits = 0;
										surf->dlightframe = r_framecount;
									}
									surf->dlightbits |= bit;
								}
							}
							while (--m);
						}
					}
				}
				k++;
				continue;
			}
		
			k += *in++;
		}
	}
}


/*
=============
R_PushDlights
=============
*/
void
R_PushDlights (void)
{
	int         i;
	dlight_t   *l;

	l = cl_dlights;

	for (i = 0; i < MAX_DLIGHTS; i++, l++) {
		if (l->die < cl.time || !l->radius)
			continue;
		R_MarkLights (l, 1 << i, cl.worldmodel);
	}
}

/*
=============================================================================

LIGHT SAMPLING

=============================================================================
*/

// Tomaz - Lit Support Begin
int RecursiveLightPoint (vec3_t color, mnode_t *node, vec3_t start, vec3_t end)
{
	
	int		side;
	float		front, back, frac;
	vec3_t		mid;

loc0:
	if (node->contents < 0)
		return false;		// didn't hit anything
	
// calculate mid point
	front = PlaneDiff(start, node->plane);
	back = PlaneDiff(end, node->plane);

	if ((back < 0) == (front < 0))
	{
		node = node->children[front < 0];
		goto loc0;
	}
	
	frac = front / (front-back);
	mid[0] = start[0] + (end[0] - start[0])*frac;
	mid[1] = start[1] + (end[1] - start[1])*frac;
	mid[2] = start[2] + (end[2] - start[2])*frac;



// go down front side
	if (RecursiveLightPoint (color, node->children[front < 0], start, mid))
		return true;	// hit something
	else
	{
		int i, ds, dt;
		msurface_t *surf;
	// check for impact on this node
		VectorCopy (mid, lightspot);
		lightplane = node->plane;

		surf = cl.worldmodel->surfaces + node->firstsurface;
		for (i = 0;i < node->numsurfaces;i++, surf++)
		{
			if (surf->flags & SURF_DRAWTILED)
				continue;	// no lightmaps

			ds = (int) ((float) DotProduct (mid, surf->texinfo->vecs[0]) + surf->texinfo->vecs[0][3]);
			dt = (int) ((float) DotProduct (mid, surf->texinfo->vecs[1]) + surf->texinfo->vecs[1][3]);

			if (ds < surf->texturemins[0] || dt < surf->texturemins[1])
				continue;
			
			ds -= surf->texturemins[0];
			dt -= surf->texturemins[1];
			
			if (ds > surf->extents[0] || dt > surf->extents[1])
				continue;

			if (surf->samples)
			{
				byte *lightmap;
				int maps, size3, line3, dsfrac = ds & 15, dtfrac = dt & 15, r00 = 0, g00 = 0, b00 = 0, r01 = 0, g01 = 0, b01 = 0, r10 = 0, g10 = 0, b10 = 0, r11 = 0, g11 = 0, b11 = 0;
				float scale;//tei from lh
				line3 = ((surf->extents[0]>>4)+1)*3;

				lightmap = surf->samples + ((dt>>4) * ((surf->extents[0]>>4)+1) + (ds>>4))*3;

				size3 = ((surf->extents[0]>>4)+1) * ((surf->extents[1]>>4)+1)*3; // LordHavoc: *3 for colored lighting

				for (maps = 0;maps < MAXLIGHTMAPS && surf->styles[maps] != 255;maps++)
				{
					scale = (float) d_lightstylevalue[surf->styles[maps]] * 1.0 / 256.0;	// Tomaz - Speed
					// NOTE: r=red, g=green, b=blue
					// numbers are tex-coords
					r00 += (float) lightmap[      0] * scale; // red 00
					g00 += (float) lightmap[      1] * scale; // green 00
					b00 += (float) lightmap[	  2] * scale; // blue 00
					r01 += (float) lightmap[      3] * scale; // red 01
					g01 += (float) lightmap[      4] * scale; // green 01
					b01 += (float) lightmap[      5] * scale; // blue 01	
					r10 += (float) lightmap[line3+0] * scale; // red 10
					g10 += (float) lightmap[line3+1] * scale; // green 10
					b10 += (float) lightmap[line3+2] * scale; // blue 10
					r11 += (float) lightmap[line3+3] * scale; // red 11
					g11 += (float) lightmap[line3+4] * scale; // green 11
					b11 += (float) lightmap[line3+5] * scale; // blue 11
#if 1 //Telejano RC2 speed
					lightmap += size3;//((surf->extents[0]>>4)+1) * ((surf->extents[1]>>4)+1)*3;//Tei optizimacion
#else
					lightmap += ((surf->extents[0]>>4)+1) * ((surf->extents[1]>>4)+1)*3;
#endif
				}


				/*
				//Original tomaz
				color[0] += (float) ((((((((r11-r10) * dsfrac) >> 4) + r10)-((((r01-r00) * dsfrac) >> 4) + r00)) * dtfrac) >> 4) + ((((r01-r00) * dsfrac) >> 4) + r00)) * (1.0f / 32768.0f);
				color[1] += (float) ((((((((g11-g10) * dsfrac) >> 4) + g10)-((((g01-g00) * dsfrac) >> 4) + g00)) * dtfrac) >> 4) + ((((g01-g00) * dsfrac) >> 4) + g00)) * (1.0f / 32768.0f);
				color[2] += (float) ((((((((b11-b10) * dsfrac) >> 4) + b10)-((((b01-b00) * dsfrac) >> 4) + b00)) * dtfrac) >> 4) + ((((b01-b00) * dsfrac) >> 4) + b00)) * (1.0f / 32768.0f);
				*/

				//lh version
				color[0] += (float) ((int) ((((((((r11-r10) * dsfrac) >> 4) + r10)-((((r01-r00) * dsfrac) >> 4) + r00)) * dtfrac) >> 4) + ((((r01-r00) * dsfrac) >> 4) + r00)));
				color[1] += (float) ((int) ((((((((g11-g10) * dsfrac) >> 4) + g10)-((((g01-g00) * dsfrac) >> 4) + g00)) * dtfrac) >> 4) + ((((g01-g00) * dsfrac) >> 4) + g00)));
				color[2] += (float) ((int) ((((((((b11-b10) * dsfrac) >> 4) + b10)-((((b01-b00) * dsfrac) >> 4) + b00)) * dtfrac) >> 4) + ((((b01-b00) * dsfrac) >> 4) + b00)));
			}
			return true; // success
		}

	// go down back side
		
		// go down back side
#if 1 //Telejano RC2 speed
		side = front < node->plane->dist;
		node = node->children[side ^ 1];
		//startz = mid;
		//distz = endz - startz;
		goto loc0;
#else
		return RecursiveLightPoint (color, node->children[front >= 0], mid, end);
#endif
	}
}

#if 1
extern cvar_t r_ambient;//Tei ambient
#endif

void R_LightPoint (vec3_t p)
{
	vec3_t		end;
	
	if (!cl.worldmodel->lightdata)
	{
		lightcolor[0] = lightcolor[1] = lightcolor[2] = 255;
		return;
	}
	
	end[0] = p[0];
	end[1] = p[1];
	end[2] = p[2] - 2048;

#if 1 //Telejano RC2 feature r_ambient
	lightcolor[0] = lightcolor[1] = lightcolor[2] = r_ambient.value;//Tei
#else
	lightcolor[0] = lightcolor[1] = lightcolor[2] = 0;//Tei
#endif
	RecursiveLightPoint (lightcolor, cl.worldmodel->nodes, p, end);
}
// Tomaz - Lit Support End