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
// mathlib.c -- math primitives
// Tomaz - Whole File Redone
#include <math.h>
#include "quakedef.h"

void Sys_Error (char *error, ...);

vec3_t vec3_origin = {0,0,0};
int nanmask = 255<<23;

/*-----------------------------------------------------------------*/

double			angleModScalar1;
double			angleModScalar2;
float			degRadScalar;


#if 0 //Tei: Lh math
float ixtable[4096];

/*-----------------------------------------------------------------*/

float m_bytenormals[NUMVERTEXNORMALS][3] =
{
{-0.525731, 0.000000, 0.850651}, {-0.442863, 0.238856, 0.864188}, 
{-0.295242, 0.000000, 0.955423}, {-0.309017, 0.500000, 0.809017}, 
{-0.162460, 0.262866, 0.951056}, {0.000000, 0.000000, 1.000000}, 
{0.000000, 0.850651, 0.525731}, {-0.147621, 0.716567, 0.681718}, 
{0.147621, 0.716567, 0.681718}, {0.000000, 0.525731, 0.850651}, 
{0.309017, 0.500000, 0.809017}, {0.525731, 0.000000, 0.850651}, 
{0.295242, 0.000000, 0.955423}, {0.442863, 0.238856, 0.864188}, 
{0.162460, 0.262866, 0.951056}, {-0.681718, 0.147621, 0.716567}, 
{-0.809017, 0.309017, 0.500000}, {-0.587785, 0.425325, 0.688191}, 
{-0.850651, 0.525731, 0.000000}, {-0.864188, 0.442863, 0.238856},
{-0.716567, 0.681718, 0.147621}, {-0.688191, 0.587785, 0.425325}, 
{-0.500000, 0.809017, 0.309017}, {-0.238856, 0.864188, 0.442863}, 
{-0.425325, 0.688191, 0.587785}, {-0.716567, 0.681718, -0.147621}, 
{-0.500000, 0.809017, -0.309017}, {-0.525731, 0.850651, 0.000000}, 
{0.000000, 0.850651, -0.525731}, {-0.238856, 0.864188, -0.442863},
{0.000000, 0.955423, -0.295242}, {-0.262866, 0.951056, -0.162460}, 
{0.000000, 1.000000, 0.000000}, {0.000000, 0.955423, 0.295242},
{-0.262866, 0.951056, 0.162460}, {0.238856, 0.864188, 0.442863}, 
{0.262866, 0.951056, 0.162460}, {0.500000, 0.809017, 0.309017}, 
{0.238856, 0.864188, -0.442863}, {0.262866, 0.951056, -0.162460}, 
{0.500000, 0.809017, -0.309017}, {0.850651, 0.525731, 0.000000}, 
{0.716567, 0.681718, 0.147621}, {0.716567, 0.681718, -0.147621}, 
{0.525731, 0.850651, 0.000000}, {0.425325, 0.688191, 0.587785}, 
{0.864188, 0.442863, 0.238856}, {0.688191, 0.587785, 0.425325}, 
{0.809017, 0.309017, 0.500000}, {0.681718, 0.147621, 0.716567},
{0.587785, 0.425325, 0.688191}, {0.955423, 0.295242, 0.000000}, 
{1.000000, 0.000000, 0.000000}, {0.951056, 0.162460, 0.262866},
{0.850651, -0.525731, 0.000000}, {0.955423, -0.295242, 0.000000}, 
{0.864188, -0.442863, 0.238856}, {0.951056, -0.162460, 0.262866}, 
{0.809017, -0.309017, 0.500000}, {0.681718, -0.147621, 0.716567},
{0.850651, 0.000000, 0.525731}, {0.864188, 0.442863, -0.238856},
{0.809017, 0.309017, -0.500000}, {0.951056, 0.162460, -0.262866}, 
{0.525731, 0.000000, -0.850651}, {0.681718, 0.147621, -0.716567}, 
{0.681718, -0.147621, -0.716567}, {0.850651, 0.000000, -0.525731},
{0.809017, -0.309017, -0.500000}, {0.864188, -0.442863, -0.238856}, 
{0.951056, -0.162460, -0.262866}, {0.147621, 0.716567, -0.681718}, 
{0.309017, 0.500000, -0.809017}, {0.425325, 0.688191, -0.587785}, 
{0.442863, 0.238856, -0.864188}, {0.587785, 0.425325, -0.688191}, 
{0.688191, 0.587785, -0.425325}, {-0.147621, 0.716567, -0.681718}, 
{-0.309017, 0.500000, -0.809017}, {0.000000, 0.525731, -0.850651},
{-0.525731, 0.000000, -0.850651}, {-0.442863, 0.238856, -0.864188},
{-0.295242, 0.000000, -0.955423}, {-0.162460, 0.262866, -0.951056}, 
{0.000000, 0.000000, -1.000000}, {0.295242, 0.000000, -0.955423}, 
{0.162460, 0.262866, -0.951056}, {-0.442863, -0.238856, -0.864188}, 
{-0.309017, -0.500000, -0.809017}, {-0.162460, -0.262866, -0.951056}, 
{0.000000, -0.850651, -0.525731}, {-0.147621, -0.716567, -0.681718}, 
{0.147621, -0.716567, -0.681718}, {0.000000, -0.525731, -0.850651}, 
{0.309017, -0.500000, -0.809017}, {0.442863, -0.238856, -0.864188}, 
{0.162460, -0.262866, -0.951056}, {0.238856, -0.864188, -0.442863}, 
{0.500000, -0.809017, -0.309017}, {0.425325, -0.688191, -0.587785}, 
{0.716567, -0.681718, -0.147621}, {0.688191, -0.587785, -0.425325}, 
{0.587785, -0.425325, -0.688191}, {0.000000, -0.955423, -0.295242},
{0.000000, -1.000000, 0.000000}, {0.262866, -0.951056, -0.162460}, 
{0.000000, -0.850651, 0.525731}, {0.000000, -0.955423, 0.295242}, 
{0.238856, -0.864188, 0.442863}, {0.262866, -0.951056, 0.162460}, 
{0.500000, -0.809017, 0.309017}, {0.716567, -0.681718, 0.147621}, 
{0.525731, -0.850651, 0.000000}, {-0.238856, -0.864188, -0.442863}, 
{-0.500000, -0.809017, -0.309017}, {-0.262866, -0.951056, -0.162460}, 
{-0.850651, -0.525731, 0.000000}, {-0.716567, -0.681718, -0.147621},
{-0.716567, -0.681718, 0.147621}, {-0.525731, -0.850651, 0.000000}, 
{-0.500000, -0.809017, 0.309017}, {-0.238856, -0.864188, 0.442863},
{-0.262866, -0.951056, 0.162460}, {-0.864188, -0.442863, 0.238856},
{-0.809017, -0.309017, 0.500000}, {-0.688191, -0.587785, 0.425325},
{-0.681718, -0.147621, 0.716567}, {-0.442863, -0.238856, 0.864188},
{-0.587785, -0.425325, 0.688191}, {-0.309017, -0.500000, 0.809017},
{-0.147621, -0.716567, 0.681718}, {-0.425325, -0.688191, 0.587785},
{-0.162460, -0.262866, 0.951056}, {0.442863, -0.238856, 0.864188},
{0.162460, -0.262866, 0.951056}, {0.309017, -0.500000, 0.809017},
{0.147621, -0.716567, 0.681718}, {0.000000, -0.525731, 0.850651},
{0.425325, -0.688191, 0.587785}, {0.587785, -0.425325, 0.688191},
{0.688191, -0.587785, 0.425325}, {-0.955423, 0.295242, 0.000000},
{-0.951056, 0.162460, 0.262866}, {-1.000000, 0.000000, 0.000000},
{-0.850651, 0.000000, 0.525731}, {-0.955423, -0.295242, 0.000000},
{-0.951056, -0.162460, 0.262866}, {-0.864188, 0.442863, -0.238856},
{-0.951056, 0.162460, -0.262866}, {-0.809017, 0.309017, -0.500000},
{-0.864188, -0.442863, -0.238856}, {-0.951056, -0.162460, -0.262866},
{-0.809017, -0.309017, -0.500000}, {-0.681718, 0.147621, -0.716567},
{-0.681718, -0.147621, -0.716567}, {-0.850651, 0.000000, -0.525731},
{-0.688191, 0.587785, -0.425325}, {-0.587785, 0.425325, -0.688191},
{-0.425325, 0.688191, -0.587785}, {-0.425325, -0.688191, -0.587785},
{-0.587785, -0.425325, -0.688191}, {-0.688191, -0.587785, -0.425325},
};
#endif

void Math_Init()
{
#if 0 //Tei lh math speedups
	int a;

	// LordHavoc: setup 1.0f / N table for quick recipricols of integers
	ixtable[0] = 0;
	for (a = 1;a < 4096;a++)
		ixtable[a] = 1.0f / a;
#endif

	degRadScalar	= (float)(M_PI / 180.0);
	angleModScalar1	= (360.0 / 65536.0);
	angleModScalar2	= (65536.0 / 360.0);

}

void ProjectPointOnPlane( vec3_t dst, const vec3_t p, const vec3_t normal )
{
	float d;
	vec3_t n;
	float inv_denom;

	inv_denom = 1.0F / DotProduct( normal, normal );

	d = DotProduct( normal, p ) * inv_denom;

	n[0] = normal[0] * inv_denom;
	n[1] = normal[1] * inv_denom;
	n[2] = normal[2] * inv_denom;

	dst[0] = p[0] - d * n[0];
	dst[1] = p[1] - d * n[1];
	dst[2] = p[2] - d * n[2];
}

/*
** assumes "src" is normalized
*/


void PerpendicularVector( vec3_t dst, const vec3_t src )
{
	// LordHavoc: optimized to death and beyond
	int	pos;
	float minelem;

	if (src[0])
	{
		dst[0] = 0;
		if (src[1])
		{
			dst[1] = 0;
			if (src[2])
			{
				dst[2] = 0;
				pos = 0;
				minelem = fabs(src[0]);
				if (fabs(src[1]) < minelem)
				{
					pos = 1;
					minelem = fabs(src[1]);
				}
				if (fabs(src[2]) < minelem)
					pos = 2;

				dst[pos] = 1;
				dst[0] -= src[pos] * src[0];
				dst[1] -= src[pos] * src[1];
				dst[2] -= src[pos] * src[2];

				// normalize the result
				VectorNormalize(dst);
			}
			else
				dst[2] = 1;
		}
		else
		{
			dst[1] = 1;
			dst[2] = 0;
		}
	}
	else
	{
		dst[0] = 1;
		dst[1] = 0;
		dst[2] = 0;
	}
}


#ifdef _WIN32
#pragma optimize( "", off )
#endif

void VectorVectors(const vec3_t forward, vec3_t right, vec3_t up)
{
	float d;

	right[0] = forward[2];
	right[1] = -forward[0];
	right[2] = forward[1];

	d = -DotProduct(forward, right);
	VectorMA (right, d, forward, right);
	VectorNormalize(right);
	CrossProduct(right, forward, up);
}

void OldRotatePointAroundVector( vec3_t dst, const vec3_t dir, const vec3_t point, float degrees )
{
	float	t0, t1;
	float	angle, c, s;
	vec3_t	vr, vu, vf;

	angle = DEG2RAD(degrees);

	c = cos(angle);
	s = sin(angle);

	VectorCopy (dir, vf);
	VectorVectors(vf, vr, vu);

	t0 = vr[0] *  c + vu[0] * -s;
	t1 = vr[0] *  s + vu[0] *  c;
	dst[0] = (t0 * vr[0] + t1 * vu[0] + vf[0] * vf[0]) * point[0]
	       + (t0 * vr[1] + t1 * vu[1] + vf[0] * vf[1]) * point[1]
	       + (t0 * vr[2] + t1 * vu[2] + vf[0] * vf[2]) * point[2];

	t0 = vr[1] *  c + vu[1] * -s;
	t1 = vr[1] *  s + vu[1] *  c;
	dst[1] = (t0 * vr[0] + t1 * vu[0] + vf[1] * vf[0]) * point[0]
	       + (t0 * vr[1] + t1 * vu[1] + vf[1] * vf[1]) * point[1]
	       + (t0 * vr[2] + t1 * vu[2] + vf[1] * vf[2]) * point[2];

	t0 = vr[2] *  c + vu[2] * -s;
	t1 = vr[2] *  s + vu[2] *  c;
	dst[2] = (t0 * vr[0] + t1 * vu[0] + vf[2] * vf[0]) * point[0]
	       + (t0 * vr[1] + t1 * vu[1] + vf[2] * vf[1]) * point[1]
	       + (t0 * vr[2] + t1 * vu[2] + vf[2] * vf[2]) * point[2];
}


// Lordhavoc optimize
void RotatePointAroundVector( vec3_t dst, const vec3_t dir, const vec3_t point, float degrees )
{
	// LordHavoc: optimized to death and beyond, cryptic in an entirely new way
	float	t0, t1;
	float	angle, c, s;
	vec3_t	vr, vu, vf;

	angle = DEG2RAD(degrees);

	c = cos(angle);
	s = sin(angle);

	vf[0] = dir[0];
	vf[1] = dir[1];
	vf[2] = dir[2];

//	PerpendicularVector(vr, dir);
//	CrossProduct(vr, vf, vu);
	VectorVectors(vf, vr, vu);

	t0 = vr[0] *  c + vu[0] * -s;
	t1 = vr[0] *  s + vu[0] *  c;
	dst[0] = (t0 * vr[0] + t1 * vu[0] + vf[0] * vf[0]) * point[0]
	       + (t0 * vr[1] + t1 * vu[1] + vf[0] * vf[1]) * point[1]
	       + (t0 * vr[2] + t1 * vu[2] + vf[0] * vf[2]) * point[2];

	t0 = vr[1] *  c + vu[1] * -s;
	t1 = vr[1] *  s + vu[1] *  c;
	dst[1] = (t0 * vr[0] + t1 * vu[0] + vf[1] * vf[0]) * point[0]
	       + (t0 * vr[1] + t1 * vu[1] + vf[1] * vf[1]) * point[1]
	       + (t0 * vr[2] + t1 * vu[2] + vf[1] * vf[2]) * point[2];

	t0 = vr[2] *  c + vu[2] * -s;
	t1 = vr[2] *  s + vu[2] *  c;
	dst[2] = (t0 * vr[0] + t1 * vu[0] + vf[2] * vf[0]) * point[0]
	       + (t0 * vr[1] + t1 * vu[1] + vf[2] * vf[1]) * point[1]
	       + (t0 * vr[2] + t1 * vu[2] + vf[2] * vf[2]) * point[2];
}
// Lordhavoc optimize


#ifdef _WIN32
#pragma optimize( "", on )
#endif

/*-----------------------------------------------------------------*/

float	anglemod(float a)
{
	a = angleModScalar1 * ((int)(a * angleModScalar2) & 65535);	// Tomaz Speed
	return a;
}

/*
==================
BoxOnPlaneSide

Returns 1, 2, or 1 + 2
==================
*/
int BoxOnPlaneSide (vec3_t emins, vec3_t emaxs, mplane_t *p)
{
	float	dist1, dist2;
	int		sides;

// general case
	switch (p->signbits)
	{
	case 0:
dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
dist2 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
		break;
	case 1:
dist1 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
		break;
	case 2:
dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
dist2 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
		break;
	case 3:
dist1 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
		break;
	case 4:
dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
dist2 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
		break;
	case 5:
dist1 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
		break;
	case 6:
dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
dist2 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
		break;
	case 7:
dist1 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
		break;
	default:
		dist1 = dist2 = 0;		// shut up compiler
		Sys_Error ("BoxOnPlaneSide:  Bad signbits");
		break;
	}

	sides = 0;
	if (dist1 >= p->dist)
		sides = 1;
	if (dist2 < p->dist)
		sides |= 2;

	return sides;
}

void AngleVectors (vec3_t angles, vec3_t forward, vec3_t right, vec3_t up)
{
	float		angle;
	float		sr, sp, sy, cr, cp, cy;
	
	angle	= DEG2RAD(angles[YAW]);	// Tomaz Speed
	sy		= sin(angle);
	cy		= cos(angle);
	angle	= DEG2RAD(angles[PITCH]);	// Tomaz Speed
	sp		= sin(angle);
	cp		= cos(angle);

	if (right || up)
	{
		angle	= DEG2RAD(angles[ROLL]);	// Tomaz Speed
		sr		= sin(angle);
		cr		= cos(angle);
	}

	forward[0]	= cp*cy;
	forward[1]	= cp*sy;
	forward[2]	= -sp;

	if (right)
	{
		right[0]	= (-1*sr*sp*cy+-1*cr*-sy);
		right[1]	= (-1*sr*sp*sy+-1*cr*cy);
		right[2]	= -1*sr*cp;
	}

	if (up)
	{
		up[0]		= (cr*sp*cy+-sr*-sy);
		up[1]		= (cr*sp*sy+-sr*cy);
		up[2]		= cr*cp;
	}
}
float VectorNormalize (vec3_t v)
{
	float	length = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];

	if (length)
	{
		float ilength;
		length = sqrt (length);		// FIXME
		ilength = 1/length;
		v[0] *= ilength;
		v[1] *= ilength;
		v[2] *= ilength;
	}
		
	return length;
}
#if 0 //FRitz

//johnfitz -- courtesy of lordhavoc
#define VectorNormalize(_v)\
{\
	float _y, _number;\
	_number = DotProduct(_v, _v);\
	if (_number != 0.0)\
	{\
		*((long *)&_y) = 0x5f3759df - ((* (long *) &_number) >> 1);\
		_y = _y * (1.5f - (_number * 0.5f * _y * _y));\
		VectorScale(_v, _y, _v);\
	}\
}

#endif


//Tei lh trace stuff
void AngleVectorsFLU (const vec3_t angles, vec3_t forward, vec3_t left, vec3_t up)
{
	double angle, sr, sp, sy, cr, cp, cy;

	angle = angles[YAW] * (M_PI*2 / 360);
	sy = sin(angle);
	cy = cos(angle);
	angle = angles[PITCH] * (M_PI*2 / 360);
	sp = sin(angle);
	cp = cos(angle);
	if (forward)
	{
		forward[0] = cp*cy;
		forward[1] = cp*sy;
		forward[2] = -sp;
	}
	if (left || up)
	{
		angle = angles[ROLL] * (M_PI*2 / 360);
		sr = sin(angle);
		cr = cos(angle);
		if (left)
		{
			left[0] = sr*sp*cy+cr*-sy;
			left[1] = sr*sp*sy+cr*cy;
			left[2] = sr*cp;
		}
		if (up)
		{
			up[0] = cr*sp*cy+-sr*-sy;
			up[1] = cr*sp*sy+-sr*cy;
			up[2] = cr*cp;
		}
	}
}