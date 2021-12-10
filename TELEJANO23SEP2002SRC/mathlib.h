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
// mathlib.h
// Tomaz - Whole File Redone
typedef float vec3_t[3];

#ifndef M_PI
#define M_PI		3.141592653589793	// matches value in gcc v2 math.h
#endif

struct	mplane_s;

extern	vec3_t vec3_origin;
extern	int nanmask;

#define	IS_NAN(x) (((*(int *)&x)&nanmask)==nanmask)
#define DEG2RAD( a ) ( a * degRadScalar )


#define lhrandom(MIN,MAX) ((rand() & 32767) * (((MAX)-(MIN)) * (1.0f / 32767.0f)) + (MIN))
#define teirand(MAX) ((rand() & 32767) * ((MAX) * (1.0f / 32767.0f)))

#define DotProduct(x,y) (x[0]*y[0]+x[1]*y[1]+x[2]*y[2])
#define Length(v) (sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]))
#define VectorCompare(a,b) ((a[0] == b[0]) & (a[1] == b[1]) & (a[2] == b[2]))
#define VectorSet(v,a,b,c) (v[0]=a,v[1]=b,v[2]=c)

#define VectorSubtract(a,b,c)								\
(															\
	c[0]	= a[0] - b[0],									\
	c[1]	= a[1] - b[1],									\
	c[2]	= a[2] - b[2]									\
)

#define VectorAdd(a,b,c)									\
(															\
	c[0]	= a[0] + b[0],									\
	c[1]	= a[1] + b[1],									\
	c[2]	= a[2] + b[2]									\
)

#define VectorScale(a,b,c)									\
(															\
	c[0]	= a[0] * b,										\
	c[1]	= a[1] * b,										\
	c[2]	= a[2] * b										\
)

#define VectorCopy(in,out)									\
(															\
	out[0]	= in[0],										\
	out[1]	= in[1],										\
	out[2]	= in[2]											\
)

#define VectorInverse(v)									\
(															\
	v[0]	= -v[0],										\
	v[1]	= -v[1],										\
	v[2]	= -v[2]											\
)

#define CrossProduct(v1,v2,cross)							\
(															\
	cross[0] = v1[1] * v2[2] - v1[2] * v2[1],				\
	cross[1] = v1[2] * v2[0] - v1[0] * v2[2],				\
	cross[2] = v1[0] * v2[1] - v1[1] * v2[0]				\
)

#define VectorMA(a,s,b,c)									\
(															\
	c[0]	= a[0] + (s * b[0]),							\
	c[1]	= a[1] + (s * b[1]),							\
	c[2]	= a[2] + (s * b[2])								\
)

#define VectorClear(a)										\
(															\
	a[0] = 0,												\
	a[1] = 0,												\
	a[2] = 0												\
)

#define VectorNegate(a,b)									\
(															\
	b[0] = -(a[0]),											\
	b[1] = -(a[1]),											\
	b[2] = -(a[2])											\
)

float	VectorNormalize(vec3_t v);


void RotatePointAroundVector( vec3_t dst, const vec3_t dir, const vec3_t point, float degrees );

void	VectorVectors(const vec3_t forward, vec3_t right, vec3_t up);
void	AngleVectors (vec3_t angles, vec3_t forward, vec3_t right, vec3_t up);
int		BoxOnPlaneSide (vec3_t emins, vec3_t emaxs, struct mplane_s *plane);
float	anglemod(float a);

#define BOX_ON_PLANE_SIDE(emins, emaxs, p)	\
	(((p)->type < 3)?						\
	(										\
		((p)->dist <= (emins)[(p)->type])?	\
			1								\
		:									\
		(									\
			((p)->dist >= (emaxs)[(p)->type])?\
				2							\
			:								\
				3							\
		)									\
	)										\
	:										\
		BoxOnPlaneSide(emins, emaxs, p))

extern	float			degRadScalar;
extern	double			angleModScalar1;
extern	double			angleModScalar2;

#define PlaneDiff(point,plane) (((plane)->type < 3 ? (point)[(plane)->type] : DotProduct((point), (plane)->normal)) - (plane)->dist)

void	Math_Init();

//LordHavoc bound
#define bound(min,num,max) (num >= min ? (num < max ? num : max) : min)
