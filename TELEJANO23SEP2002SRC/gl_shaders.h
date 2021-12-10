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

#define MAX_ANIM_FRAMES 20
#define MAX_RS	1024

typedef struct
{
	float	power;
	float	movediv;
} rturb_t;

typedef struct
{
	float	xspeed;
	float	yspeed;
} rscroll_t;

typedef struct
{
	float		srcmode;
	float		dstmode;
} rblend_t;

typedef struct
{
	float	alpha;
	BOOL	alphafunc;
	BOOL	blendfunc;
	BOOL	nolightmap;
	BOOL	envmap;
	BOOL	drawsky;
	BOOL	isportal;
	float	animtime;
} rflags_t;

typedef struct
{
	char	name[56];
} rfname_t;

typedef struct
{
	int			num;
	rfname_t	name[MAX_ANIM_FRAMES];
	int			texnum[MAX_ANIM_FRAMES];
	int			current;
	double		lasttime;
} ranim_t;

typedef struct
{
	vec3_t		origin;
	vec3_t		angle;
} rportal_t;

typedef struct
{
	char				scriptname[56];
	char				texname[56];
	int					texnum;
	float				texscale;
	BOOL				useturb;
	BOOL				usescroll;
	BOOL				useblendmode;
	BOOL				usevturb;
	BOOL				useanim;
	rturb_t				turb;
	rturb_t				vturb;
	rscroll_t			scroll;
	rblend_t			blendmode;
	rflags_t			flags;
	ranim_t				anim;
	rportal_t			portal;
	int					nextstage;
	char				nextname[56];
} rscript_t;

rscript_t	rscripts[MAX_RS];