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
	float	alpha;
	BOOL	blendfunc;
	BOOL	envmap;
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
	char				scriptname[56];
	char				texname[56];
	int					texnum;
	float				texscale;
	BOOL				texexist;
	BOOL				useturb;
	BOOL				usescroll;
	BOOL				usevturb;
	BOOL				useanim;
//Tei more shaders
	BOOL				usegrass;
	BOOL				usegrass2;
	char				grassname[255];
	char				grassname2[255];
	float				grassdensity;
	float				grassdensity2;
	model_t	*			grassmodel;
	model_t	*			grassmodel2;
//Tei more shaders

//Tei detail from rscript
	BOOL				usedetail;
	char				detailname[255];
	int					mydetail;
	float				mydetailscale;
//Tei detail from rscript

//Tei smoke rscript
	BOOL				usesmoke;
	int					smokeheight;
	float				smokescale;
//Tei smoke rscript

//Tei
	BOOL				usewater;
	BOOL				usesnow;
	BOOL				uselight;
	BOOL				userain;
	BOOL				usezing;

			BOOL flava;
			BOOL flight;
			BOOL fnodraw;
			BOOL frain;
			BOOL fsnow;
			BOOL ftele;
			BOOL fullbrights;
			BOOL fwater;
			BOOL islava;
			BOOL issky;
			BOOL istele;
			BOOL iswater;
			BOOL wfx_red;
			BOOL wfx_green;
			BOOL wfx_blue;
			BOOL nodraw;

//Tei

	rturb_t				turb;
	rturb_t				vturb;
	rscroll_t			scroll;
	rflags_t			flags;
	ranim_t				anim;
	int					nextstage;
	char				nextname[56];
} rscript_t;


//Tei mscript

typedef struct
{
	char				scriptname[56];
	char				modelname[56];

	model_t	*			model;

//corona
	BOOL				usecorona;

//mfx
			BOOL use_FIRE;
			BOOL use_FIRE2;
			BOOL use_MISSILE;
			BOOL use_FIRELAMP;
			BOOL use_BLUEFIRE2;
			BOOL use_BLUEFIRE;
			BOOL use_DOWFIRE;
			BOOL use_ENGINEFIRE2;
			BOOL use_BIGFIRE;
			BOOL use_FOGMAKER;
			BOOL use_FOGMAKERLITE;
			BOOL use_WATERFALL;
			BOOL use_SPARKSHOWER;
			BOOL use_ALIENBLOOD;
			BOOL use_BOLTFX;
			BOOL use_SUN;
			BOOL use_LASER;		
			BOOL use_SPIKE;
			BOOL use_GLOW;
			BOOL use_LUX;
			BOOL use_GIB;
			BOOL use_GRASS;
			BOOL use_GRASS2;
			BOOL use_LUX2;		
			BOOL use_LASERBEAM;
			BOOL use_FOOTSOUND;

			BOOL	flags;
			BOOL	fullbright;
			vec3_t	glow_color;
			int	glow_radius;
			BOOL	noshadow;



	float				coronaalpha;
	int					coronasize;

	ranim_t				anim;
	int					nextstage;
	char				nextname[56];
} mscript_t;

//Tei mscript

#define MAX_RS	512
#define MAX_MS	512

rscript_t	rscripts[MAX_RS];
mscript_t	mscripts[MAX_MS];

int GetRSForName(char name[56]);
int GetMSForName(char *name, model_t * model);
