#include "quakedef.h"

#define MAX_DECALS	32


enum
{
	DECAL_NONE,
	DECAL_BULLTMRK,
	DECAL_BLOODMRK1,
	DECAL_BLOODMRK2,
	DECAL_BLOODMRK3,
	DECAL_EXPLOMRK
};


// decals
typedef struct
{
	unsigned char type;
	unsigned int gl_texture;
	double	time;
	float	alpha;
	vec3_t	normal, org;
	vec3_t	v[4];
} decal_t;

decal_t			cl_decals[MAX_DECALS];
extern			client_state_t cl;

static int bulletmarktex = 0;
static int bloodmarktex[3] = {0, 0, 0};
static int explosionmarktex = 0;
int num_decals = 0;

cvar_t gl_polyoffset = {"gl_polyoffset", "-0.1", 0};

/*
===============
CL_ClearDecals

===============
*/

void CL_ClearDecals(void)
{
	memset (cl_decals, 0, sizeof(decal_t)*MAX_DECALS);
	num_decals = 0;
}

/*
===============
CL_AllocDecal

Find the oldest decal
===============
*/
decal_t *CL_AllocDecal(int type)
{
	int i, d = -1;
	double besttime = 9999999.0;

	if (type)
	{
		for (i = 0; i < MAX_DECALS; i++)
		{
			// try finding exact match
			if (cl_decals[i].time < besttime)
			{
				if (!cl_decals[i].type || (cl_decals[i].type == type))
				{
					d = i;
					besttime = cl_decals[i].time;
				}
			}
		}

		if (d > -1)
			return &cl_decals[d];
	}

	d = -1;
	besttime = 9999999.0;

	for (i = 0; i < MAX_DECALS; i++)
	{
		if (cl_decals[i].time < besttime) {
			d = i;
			besttime = cl_decals[i].time;
		}
	}

	if (d > -1)
		return &cl_decals[d];

	return &cl_decals[0];
}

/*
==============
CL_AddDecal

==============
*/
static vec3_t axis[3] = 
{
	{ 1.0f, 0.0f, 0.0f },
	{ 0.0f, 1.0f, 0.0f },
	{ 0.0f, 0.0f, 1.0f }
};

void CL_AddDecal (vec3_t n, vec3_t pt, int type)
{
	decal_t *decal;
	vec3_t fn, up, right, t;
	int major = 0;
	int size = 3;

	if ((type < DECAL_NONE) || (type > DECAL_EXPLOMRK))
		return;

	decal = CL_AllocDecal (type);
	num_decals++;

	VectorCopy (n, decal->normal);
	VectorCopy (pt, decal->org);

	fn[0] = fabs(n[0]);
	fn[1] = fabs(n[1]);
	fn[2] = fabs(n[2]);

	// find the major axis
	if (fn[1] > fn[major])
		major = 1;
	if (fn[2] > fn[major])
		major = 2;

	// build right vector by hand
	if (fn[0] == 1 || fn[1] == 1 || fn[2] == 1)
	{
		if ((major == 0 && n[0] > 0) || major == 1)
			VectorSet(right, 0, 0, -1);
		else if (major == 0)
			VectorSet(right, 0, 0, 1);
		else
			VectorSet(right, n[2], 0, 0);
	}
	else {
		CrossProduct (axis[major], n, right);
	}

	CrossProduct (n, right, up);
	VectorNormalize (up);
	VectorNormalize (right);

	VectorSubtract (right, up, t);
	VectorScale (t, size, t);
	VectorAdd (pt, t, decal->v[1]);
	VectorAdd (pt, -t, decal->v[3]);

	VectorAdd (right, up, t);
	VectorScale (t, size, t);
	VectorAdd (pt, t, decal->v[2]);
	VectorAdd (pt, -t, decal->v[0]);

	decal->time = cl.time + 120.0;
	decal->alpha = 1.0f;

	// damn... BUGFIX
	decal->type = type;

	if (type == DECAL_BULLTMRK)
		decal->gl_texture = bulletmarktex;
	else if ((type >= DECAL_BLOODMRK1) && (type <= DECAL_BLOODMRK3))
		decal->gl_texture = bloodmarktex[type - DECAL_BLOODMRK1];
	else if (decal->gl_texture == DECAL_EXPLOMRK)
		decal->gl_texture = explosionmarktex;
}

static int CompareDecals(const void *a, const void *b)
{
    return ((decal_t*)a)->gl_texture - ((decal_t*)b)->gl_texture;
}

static void R_SortDecals(void)
{
    qsort((void *)cl_decals, MAX_DECALS, sizeof(decal_t), CompareDecals);
}

/*
===============
R_RenderDecals

Minimizing GL-calls by sorting decals
===============
*/
void R_RenderDecals (void)
{
	int i, texnum = 0;
	vec3_t v;

	// don't render if there are no decals
	if (!num_decals)
		return;

	// sort decals by texture
	R_SortDecals();

	glPushMatrix();

	glDepthMask (GL_FALSE);

	glEnable (GL_POLYGON_OFFSET_FILL);
	glPolygonOffset (gl_polyoffset.value, gl_polyoffset.value);

	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	for (i = 0; i < MAX_DECALS; i++)
	{
		if (cl_decals[i].time > cl.time)
		{
			VectorSubtract(cl_decals[i].org, r_refdef.vieworg, v);
			if (DotProduct(cl_decals[i].normal, v) < 0)
				continue;

			glColor4f (1.0f, 1.0f, 1.0f, cl_decals[i].alpha);

			if ((cl_decals[i].gl_texture > 0) &&
				(cl_decals[i].gl_texture != texnum)) {
				texnum = cl_decals[i].gl_texture;
				//GL_Bind (texnum);
				glBindTexture (GL_TEXTURE_2D, texnum);
			}

			glBegin(GL_QUADS);

			glTexCoord2f(0.0f, 0.0f);
			glVertex3fv(cl_decals[i].v[0]);

			glTexCoord2f(1.0f, 0.0f);
			glVertex3fv(cl_decals[i].v[1]);

			glTexCoord2f(1.0f, 1.0f);
			glVertex3fv(cl_decals[i].v[2]);

			glTexCoord2f(0.0f, 1.0f);
			glVertex3fv(cl_decals[i].v[3]);

			glEnd();
		}
		else {
			if (cl_decals[i].alpha > 0)
			{
				cl_decals[i].alpha = 0;
				num_decals--;
			}
		}
	}

	glDisable (GL_POLYGON_OFFSET_FILL);
	glColor4f (1, 1, 1, 1);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glDepthMask (GL_TRUE);
	glPopMatrix();
}

/*
===============
R_InitDecalsTextures

===============
*/
void R_InitDecalsTextures (void)
{
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	bulletmarktex = loadtextureimage ("gfx/hole.tga", true, true);
	explosionmarktex = 0;
}

