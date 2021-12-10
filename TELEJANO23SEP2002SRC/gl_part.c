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
//  This code is out-of-control. Need a total rewrite.
//
//



//
// particle engine
//

#include "quakedef.h"

void DefineFlareColor(vec3_t origin, int radius, int mode, int alfa, float red, float green, float blue) ;
void R_ParticleExplosionSmoke (vec3_t origin) ;

#define COLOR(qred, qgreen, qblue)   p->colorred=(qred);p->colorgreen= (qgreen);p->colorblue=(qblue);
#define NEWPARTICLE()    		p	= addParticle(); if(!p)	{return;}


#define FAR_LERP 10

typedef enum	ptype_s		ptype_t;
typedef struct	particle_s	particle_t;

void R_Blood (vec3_t origin, int alfa);
void R_AlienBlood (vec3_t origin);
void R_AlienBlood2 (vec3_t origin);
void R_ParticlePuff (vec3_t origin, int alfa);
void Draw_HazeParticle(float size, particle_t *p);	//Tei xray
//void Draw_HazeParticle2(float size, particle_t *p);	//Tei xfx
void Draw_HazeParticleOndraw(particle_t *p);		//Tei haze
void R_ParticleStelar (vec3_t origin, int alfa);
void R_ParticleFire (vec3_t origin, int dx, int scale);
void R_SuperZing (vec3_t start, vec3_t end);//Tei


enum ptype_s
{
	pt_static		= 0 << 0, 
		pt_smoke		= 1 << 0, 
		pt_bubble		= 1 << 1,
		pt_explode		= 1 << 2, 
		pt_blood		= 1 << 3,
		pt_blood2		= 1 << 4,
		pt_snow			= 1 << 5,
		pt_rain			= 1 << 6,
		pt_bulletpuff	= 1 << 7, 
		pt_fade			= 1 << 8,
		pt_grav			= 1 << 9,
		pt_rail			= 1 << 10,
		pt_blob			= 1 << 11,
		pt_blob2		= 1 << 12,
		pt_smokeexp		= 1 << 13,
		pt_fire			= 1 << 14,
		pt_fire2		= 1 << 15,
		// Tei extra beahv
		pt_slowmoke		= 1 << 16,
		pt_expandfade	= 1 << 17,
		pt_expand		= 1 << 18,
		pt_slowmoke2	= 1 << 19,
		pt_snowfade		= 1 << 20,
		pt_fadeout		= 1 << 21,
		pt_fog			= 1 << 22,
		pt_fog2			= 1 << 23,
		pt_gorigin		= 1 << 24
		// Tei extra beahv
};


#define FXC_NOFX		0
#define FXC_AUTOGRAY	1
#define FXC_AUTOGRAY2	2
#define FXC_AUTOALFA	3
#define FXC_AUTOSMOKE	4
#define FXC_STELAR		5
#define FXC_BLOOD		6
#define FXC_ALIENBLOOD	7
#define FXC_ALIENBLOOD2	8
#define FXC_AUTOGRAY3	9
#define FXC_DeeTeeLAZER 10  //Suggestion from DeeTee
#define FXC_AUTOSMOKE2	11
#define FXC_HAZE		12 //Q2MAX Haze!
#define FXC_VERTIHAZE	13
#define FXC_REMOVENEXT	14
#define FXC_BIGBLOOD	15
#define FXC_FIRE		16
#define FXC_ZING		17



#define HZ_NOHAZE		0
#define HZ_NORMAL		1
#define HZ_LIGHTSTORM	2
#define HZ_LIGHTCAOS	3



struct particle_s
{
	int					texnum;
	int					contents;
	
	float				die;
	float				time;
	float				alpha;
	float				scale;
	
	float				scalex,scaley,scalez; // Tei particled scaled
	
	float				bounce;
	
	byte				colorred;
	byte				colorgreen;
	byte				colorblue;
	
	//Tei velcolor
	byte				fxcolor;
	//Tei velcolor
	
	vec3_t				origin;
	vec3_t				velocity;
	
	
	// Tei implosion
	vec3_t				gorigin;
	// Tei implosion

	vec3_t				gvelocity;// for hazes..
	

	ptype_t				type;
	
	qboolean			glow;
	qboolean			wfx; // Tei wheater fx particle
	//qboolean			volumetric; //Tei 
	
	particle_t			*next;
	particle_t			*prev;
	
	//int					simple;//Tei rendering option
	
	// Tei nearest
	qboolean			nearest;
	// Tei nearest
	
	// Tei haze 
	int					hazetype; //For hazes
	int					storm; //haze storm size!
	// Tei haze

};

int		particle_tex;
int		smoke1_tex;
int		smoke2_tex;
int		smoke3_tex;
int		smoke4_tex;
int		blood_tex;
int		bubble_tex;
int		snow_tex;
int		snow1_tex;//Tei venomus snow
int		snow2_tex;//Tei venomus snow

//Tei builtinsmoke
int     b_smoke1_tex;
int     b_smoke2_tex;
int     b_smoke3_tex;
int     b_smoke4_tex;
//Tei builtinsmoke


int		rain_tex;

// Tei new particles hardcoded

int		flareglow_tex;
int		fire_tex;
int		firecycle_tex[6];
int		flare_tex;
int		money_tex;
int		flama_tex;
int		circle_tex;
int		focus_tex;

int		bolt_tex;
// Tei new particles hardcoded

//Tei externalized tex
int zing1_tex;
int zing2_tex;
int zing3_tex;
int zing4_tex;
//Tei externalized tex

//Tei force internal particle

int builtin_particle_tex;

//Tei force internal particle


particle_t	*active_particles, *free_particles, *particles;

int		r_numparticles;

vec3_t	r_pright, r_pup, r_ppn;

extern byte	particle[32][32];
extern byte	smoke1[32][32];
extern byte	smoke2[32][32];
extern byte	smoke3[32][32];
extern byte	smoke4[32][32];
extern byte	blood[32][32];
extern byte	bubble[32][32];
extern byte	snow[32][32];
extern byte	rain[32][32];

// Tei new particles hardcoded
extern byte	fire[32][32];
extern byte	flare[32][32];
extern byte	money[32][32];
extern byte	flama[32][32];
extern byte	circle[32][32];
extern byte focus[32][32];
// Tei new particles hardcoded

//Tei
void R_ParticleSmoke (vec3_t origin, int alfa);
void R_ParticleExplosionAlien (vec3_t origin) ;

extern cvar_t gl_smallparticles;//Tei smallparticles


void LuxHerePart (particle_t *p) 
{
	dlight_t	*dl;
	dl = CL_AllocDlight (0);
	if (!dl)
		return;
	VectorCopy (p->origin, dl->origin);
	dl->color[0]	= p->colorred;
	dl->color[1]	= p->colorgreen;
	dl->color[2]	= p->colorblue;
	dl->radius	= p->scale * 10;
	dl->die	  	= cl.time + 7;
	dl->decay	= p->scale ;
};
//Tei

//Tei procedural textures
int dxpoint(int x, int y, int posx, int posy)
{
	return sqrt(pow(fabs(x-posx),2)+pow(fabs(y-posy),2));
}
//Tei procedural textures

//Tei fast long vector
int dxvector (vec3_t *vec1, vec3_t * vec2)
{
	vec3_t dxv;

	VectorSubtract(vec1, vec2, dxv) ;
	return fabs(dxv[0])+fabs(dxv[1])+fabs(dxv[2]);
}
//Tei long fast vector


//Tei external load textures
int LoadParticleFile(char *part)
{
	char filename[8192];
	byte * dataimg;
	int result;

	sprintf(filename,"particle/%s",part );

	dataimg = 0;
	dataimg = loadimagepixels(filename, true);//false
	if (dataimg) {
		//Con_Printf("try: loading from .tga");
		result = GL_LoadTexture (part, image_width, image_height, dataimg, true,true, 4);		
	}
	else
		result = 0;

	return result;
};
//Tei external load textures

//Tei add externarlize particles
extern int detailtexture;
extern int detailtexture2;//XFX

model_t * grassmodel;

#include "gl_rscript.h"

void R_InitParticleTexture (void)
{
	int		x, y, f, t;
	byte	data[32][32][4];
	char	name[256];
//	byte	*dataimg;

//Tei grass
	grassmodel = 0;
	grassmodel = Mod_ForName("textures/grass.mdl",false);

	for (t=0; t<512;t++)
	{
		if (rscripts[t].grassname)
				rscripts[t].grassmodel = Mod_ForName(rscripts[t].grassname,false); 

		if (rscripts[t].grassmodel)
			rscripts[t].grassmodel->effect = MFX_GRASS;

		if (rscripts[t].grassname2)
				rscripts[t].grassmodel2 = Mod_ForName(rscripts[t].grassname2,false); 

		if (rscripts[t].grassmodel2)
			rscripts[t].grassmodel2->effect = MFX_GRASS;

		if(rscripts[t].usedetail)	
			rscripts[t].mydetail =loadtextureimage(rscripts[t].detailname, false, false);
	}

	if (grassmodel)
		grassmodel->effect = MFX_GRASS;

	
//Tei grass

	//Moved here because Chp suggest it.
	detailtexture = 0;
	detailtexture = loadtextureimage("textures/detail", false, true); // CHP - load the Detail Texture
	
	detailtexture2 = loadtextureimage("textures/geodetail", false, true); // xfx
	
	for (x=0 ; x<32 ; x++)
	{
		for (y=0 ; y<32 ; y++)
		{
			data[x][y][0]	= 255;
			data[x][y][1]	= 255;
			data[x][y][2]	= 255;
			data[x][y][3]	= particle[x][y];
		}
	}

	/* Small stuff */

	particle_tex = 0;
	particle_tex = LoadParticleFile("particle");//Tei externalize particles	
	if (!particle_tex)
		particle_tex = GL_LoadTexture ("particle", 32, 32, &data[0][0][0], true, true, 4);

	builtin_particle_tex = GL_LoadTexture ("bparticle", 32, 32, &data[0][0][0], true, true, 4);


	/* Flare Glows.. */

	flareglow_tex = 0;
	flareglow_tex = LoadParticleFile("fglow");//Tei externalize particles. (short 8+3 name!	)
	if (!flareglow_tex)
		flareglow_tex = GL_LoadTexture ("flareglow", 32, 32, &data[0][0][0], true, true, 4);


	for (x=0 ; x<32 ; x++)
	{
		for (y=0 ; y<32 ; y++)
		{
			data[x][y][0]	= 255;
			data[x][y][1]	= 255;
			data[x][y][2]	= 255;
			data[x][y][3]	= smoke1[x][y];
		}
	}

	/* Smoke textures */

	smoke1_tex = 0;
	smoke1_tex = LoadParticleFile("smoke1");//Tei externalize particles	
	if (!smoke1_tex)
		smoke1_tex = GL_LoadTexture ("smoke1", 32, 32, &data[0][0][0], true, true, 4);

	b_smoke1_tex = GL_LoadTexture ("b_smoke1", 32, 32, &data[0][0][0], true, true, 4);
	
	for (x=0 ; x<32 ; x++)
	{
		for (y=0 ; y<32 ; y++)
		{
			data[x][y][0]	= 255;
			data[x][y][1]	= 255;
			data[x][y][2]	= 255;
			data[x][y][3]	= smoke2[x][y];
		}
	}
	
	smoke2_tex = 0;
	smoke2_tex = LoadParticleFile("smoke2");//Tei externalize particles	
	if (!smoke2_tex)
		smoke2_tex = GL_LoadTexture ("smoke2", 32, 32, &data[0][0][0], true, true, 4);
	
	b_smoke2_tex = GL_LoadTexture ("b_smoke2", 32, 32, &data[0][0][0], true, true, 4);

	for (x=0 ; x<32 ; x++)
	{
		for (y=0 ; y<32 ; y++)
		{
			data[x][y][0]	= 255;
			data[x][y][1]	= 255;
			data[x][y][2]	= 255;
			data[x][y][3]	= smoke3[x][y];
		}
	}

	smoke3_tex = 0;
	smoke3_tex = LoadParticleFile("smoke3");//Tei externalize particles	
	if (!smoke3_tex)
		smoke3_tex = GL_LoadTexture ("smoke3", 32, 32, &data[0][0][0], true, true, 4);
	
	b_smoke3_tex = GL_LoadTexture ("b_smoke3", 32, 32, &data[0][0][0], true, true, 4);
	
	for (x=0 ; x<32 ; x++)
	{
		for (y=0 ; y<32 ; y++)
		{
			data[x][y][0]	= 255;
			data[x][y][1]	= 255;
			data[x][y][2]	= 255;
			data[x][y][3]	= smoke4[x][y];
		}
	}

	
	smoke4_tex = 0;
	smoke4_tex = LoadParticleFile("smoke4");//Tei externalize particles	
	if (!smoke4_tex)
		smoke4_tex = GL_LoadTexture ("smoke4", 32, 32, &data[0][0][0], true, true, 4);
	
	b_smoke4_tex = GL_LoadTexture ("b_smoke4", 32, 32, &data[0][0][0], true, true, 4);

	for (x=0 ; x<32 ; x++)
	{
		for (y=0 ; y<32 ; y++)
		{
			data[x][y][0]	= 255;
			data[x][y][1]	= 255;
			data[x][y][2]	= 255;
			data[x][y][3]	= blood[x][y];
		}
	}

	/* Blood (runeffect blood) */
	
	blood_tex = LoadParticleFile("blood");//Tei externalize particles	
	if(!blood_tex)
		blood_tex = GL_LoadTexture ("blood", 32, 32, &data[0][0][0], true, true, 4);
	
	for (x=0 ; x<32 ; x++)
	{
		for (y=0 ; y<32 ; y++)
		{
			data[x][y][0]	= 255;
			data[x][y][1]	= 255;
			data[x][y][2]	= 255;
			data[x][y][3]	= bubble[x][y];
		}
	}

	/* OnWater */
	
	bubble_tex = LoadParticleFile("bubble");//Tei externalize particles	
	if (!bubble_tex)
		bubble_tex = GL_LoadTexture ("bubble", 32, 32, &data[0][0][0], true, true, 4);
	
	for (x=0 ; x<32 ; x++)
	{
		for (y=0 ; y<32 ; y++)
		{
			data[x][y][0]	= 255;
			data[x][y][1]	= 255;
			data[x][y][2]	= 255;
			data[x][y][3]	= snow[x][y];
		}
	}
	
	/* Snow */

	snow_tex = LoadParticleFile("snow");//Tei externalize particles	
	if(!snow_tex)
		snow_tex = GL_LoadTexture ("snow", 32, 32, &data[0][0][0], true, true, 4);

	snow1_tex = LoadParticleFile("snow1");//Tei externalize particles	
	if(!snow1_tex)
		snow1_tex = GL_LoadTexture ("snow1", 32, 32, &data[0][0][0], true, true, 4);

	snow2_tex = LoadParticleFile("snow2");//Tei externalize particles	
	if(!snow2_tex)
		snow2_tex = GL_LoadTexture ("snow2", 32, 32, &data[0][0][0], true, true, 4);

	

	for (x=0 ; x<32 ; x++)
	{
		for (y=0 ; y<32 ; y++)
		{
			data[x][y][0]	= 255;
			data[x][y][1]	= 255;
			data[x][y][2]	= 255;
			data[x][y][3]	= rain[x][y];
		}
	}

	/* rain? */

	rain_tex = LoadParticleFile("rain_tex");//Tei externalize particles	
	if(!rain_tex)
		rain_tex = GL_LoadTexture ("rain", 32, 32, &data[0][0][0], true, true, 4);
	
	// Tei new tex
	
	/*  Ugly fire for smallsize/numerous */

	for (x=0 ; x<32 ; x++)
	{
		for (y=0 ; y<32 ; y++)
		{
			data[x][y][0]	= 255;
			data[x][y][1]	= 255;
			data[x][y][2]	= 255;
			data[x][y][3]	= fire[x][y];
		}
	}

	fire_tex = LoadParticleFile("fire");//Tei externalize particles	
	if(!fire_tex){
		fire_tex = LoadParticleFile("flama");//Tei externalize particles | suggestion by merc101	
				fire_tex = GL_LoadTexture ("fire", 32, 32, &data[0][0][0], true, true, 4);
	}



	/* Fire pack */

	for(t=0;t<6;t++) 
	{
		sprintf(name,"fcycle%d",t);
		firecycle_tex[t] = LoadParticleFile(name);
		if (!firecycle_tex[t])
			firecycle_tex[t] = GL_LoadTexture ("fire", 32, 32, &data[0][0][0], true, true, 4);
	}

	/* Flare.  where? */

	for (x=0 ; x<32 ; x++)
	{
		for (y=0 ; y<32 ; y++)
		{
			data[x][y][0]	= 255;
			data[x][y][1]	= 255;
			data[x][y][2]	= 255;
			data[x][y][3]	= flare[x][y];
		}
	}

	
	flare_tex = LoadParticleFile("flare");//Tei externalize particles	
	if (!flare_tex)
		flare_tex = GL_LoadTexture ("flare", 32, 32, &data[0][0][0], true, true, 4);
	
	bolt_tex = LoadParticleFile("bolt");//Tei externalize particles	
	if (!bolt_tex)
		bolt_tex = GL_LoadTexture ("bolt", 32, 32, &data[0][0][0], true, true, 4);


	/* money fx */

	for (x=0 ; x<32 ; x++)
	{
		for (y=0 ; y<32 ; y++)
		{
			data[x][y][0]	= 255;
			data[x][y][1]	= 255;
			data[x][y][2]	= 255;
			data[x][y][3]	= money[x][y];
		}
	}

	money_tex = LoadParticleFile("money");	//Tei externalize particles
	if (!money_tex)
		money_tex = GL_LoadTexture ("money", 32, 32, &data[0][0][0], true, true, 4);
	
	for (x=0 ; x<32 ; x++)
	{
		for (y=0 ; y<32 ; y++)
		{
			data[x][y][0]	= 255;
			data[x][y][1]	= 255;
			data[x][y][2]	= 255;
			data[x][y][3]	= flama[x][y];
		}
	}

	
	flama_tex = LoadParticleFile("flama");//Tei externalize particles	
	if (!flama_tex)
		flama_tex = GL_LoadTexture ("flama", 32, 32, &data[0][0][0], true, true, 4);
	
	for (x=0 ; x<32 ; x++)
	{
		for (y=0 ; y<32 ; y++)
		{
			data[x][y][0]	= 255;
			data[x][y][1]	= 255;
			data[x][y][2]	= 255;
			data[x][y][3]	= circle[x][y];
		}
	}

	
	circle_tex = LoadParticleFile("circle");//Tei externalize particles
	if (!circle)
		circle_tex = GL_LoadTexture ("circle", 32, 32, &data[0][0][0], true, true, 4);

	for (x=0 ; x<32 ; x++)
	{
		for (y=0 ; y<32 ; y++)
		{
			data[x][y][0]	= 255;
			data[x][y][1]	= 255;
			data[x][y][2]	= 255;
			
			//data[x][y][3]	=
			f =  focus[x][y];

			f = f *0.7 - 41;

			if (f>=0)
				data[x][y][3]	= f%255;
			else
				data[x][y][3]	= 0;

		}
	}
	

	focus_tex = LoadParticleFile("focus");//Tei externalize particles
	if (!focus_tex)
		focus_tex = GL_LoadTexture ("focus", 32, 32, &data[0][0][0], true, true, 4);

	zing1_tex = LoadParticleFile("zing1");//Tei externalize particles
	if (!zing1_tex)
		zing1_tex = GL_LoadTexture ("zing1", 32, 32, &data[0][0][0], true, true, 4);
	zing2_tex = LoadParticleFile("zing2");//Tei externalize particles
	if (!zing2_tex)
		zing2_tex = GL_LoadTexture ("zing2", 32, 32, &data[0][0][0], true, true, 4);
	zing3_tex = LoadParticleFile("zing3");//Tei externalize particles
	if (!zing3_tex)
		zing3_tex = GL_LoadTexture ("zing3", 32, 32, &data[0][0][0], true, true, 4);
	zing4_tex = LoadParticleFile("zing4");//Tei externalize particles
	if (!zing4_tex)
		zing4_tex = GL_LoadTexture ("zing4", 32, 32, &data[0][0][0], true, true, 4);




}
//Tei add externarlize particles

//Return a realistic particle from similar sets 


//Get External Textures by Random Nature
#define GEX_NORMAL  0
#define GEX_FIRE	1
#define GEX_LAVA	2
#define GEX_SNOW	3
#define GEX_ZING	4
#define GEX_SMOKE	5
#define GEX_B_SMOKE	6


//Tei gex
int GetTex(int type )
{
	int t;
	
	switch(type)
	{
	case GEX_FIRE:
		t = teirand(8);
		if (t<5)
			return firecycle_tex[t];
		else
		if (t==6)
			return smoke1_tex + ( rand()& 3);
		else
		if (t==7)
			return flama_tex;//fire_tex;
		else
		return particle_tex;

		break;
	case GEX_LAVA:
		if (rand()&1)
			return particle_tex;
		else
			if (rand()&1)
				return smoke1_tex;
			else
				return smoke2_tex;
		break;
	case GEX_SNOW:
		return snow_tex + teirand(2);
		break;
	case GEX_ZING:
		return zing1_tex + teirand(3);
		break;

	case GEX_SMOKE:
		return smoke1_tex + (rand() & 3);
		break;

	case GEX_B_SMOKE:
		t=lhrandom(1,4);
		switch(t){
		case 1:
			return b_smoke1_tex;
		case 2:
			return b_smoke2_tex;
		case 3:
			return b_smoke3_tex;
		default:
			return b_smoke4_tex;
		}
		break;
	default:
		return	particle_tex;
	}

};

#define COL_SPARK	1

void GetColor(int type, particle_t *p)
{
	switch (type)
	{
	case COL_SPARK:
		if(rand()&1)
		{
			COLOR(99,99,99);
		}
		else
		{
			COLOR(255,200,100);
		}
		break;
	default:
			COLOR(99,99,99);
		break;
	}
}


//Tei gex

/*
===============
R_InitParticles
===============
*/
void R_InitParticles (void)
{
	int		i = COM_CheckParm ("-particles");
	
	r_numparticles = 4096;
	
	if (i)
	{
		r_numparticles = (int)(atoi(com_argv[i+1]));
	}


	particles = (particle_t *) Hunk_AllocName (r_numparticles * sizeof(particle_t), "particles");
	
	memset(particles, 0, r_numparticles * sizeof(particle_t));
	
	for( i = 0 ; i < r_numparticles ; ++i )
	{
		particles[i].prev	= &particles[i - 1];
		particles[i].next	= &particles[i + 1];
	}
	
	particles[0].prev		= NULL;
	particles[r_numparticles - 1].next	= NULL;
	
	free_particles			= &particles[0];
	active_particles		= NULL;
	
	R_InitParticleTexture ();
}

/*
===============
R_ClearParticles
===============
*/
void R_ClearParticles (void)
{
	int		i;
	
	free_particles			= &particles[0];
	active_particles		= NULL;
	
	memset(particles, 0, r_numparticles * sizeof(particle_t));
	
	for( i = 0 ; i < r_numparticles ; ++i )
	{
		particles[i].prev	= &particles[i - 1];
		particles[i].next	= &particles[i + 1];
	}
	
	particles[0].prev		= NULL;
	particles[r_numparticles - 1].next	= NULL;
}

particle_t* addParticle()
{
	particle_t*	p	= free_particles;
	
	if(!p)
	{
		return NULL;
	}
	
	free_particles	= p->next;
	
	if(free_particles)
	{
		free_particles->prev	= NULL;
	}
	
	p->next			= active_particles;
	
	if(active_particles)
	{
		active_particles->prev	= p;
	}
	
	active_particles			= p;
	
	// Tei scale part
	p->scalex	= 1;
	p->scaley	= 1;
	p->scalez	= 1; 
	p->fxcolor	= 0;
	// Tei scale part
	
	p->hazetype = HZ_NOHAZE;// Tei haze
	p->nearest	= 0;// Tei nearest
	
	return p;
}

particle_t* remParticle(particle_t*	p)
{
	particle_t*	next;
	
	if(!p)
	{
		return NULL;
	}
	
	// Tei static fix
	if( p->type == pt_static)
	{
		return p->next;
	}
	// Tei static fix
	
	//Tei snowfade
	if (p->type == pt_snowfade )
	{
		p->type = pt_expandfade;
		return p->next;
	}
	//Tei snowfade
	
	p->nearest = 0;//Tei
	
	if(p == active_particles)
	{
		active_particles	= p->next;
	}
	
	if(p->next)
	{
		p->next->prev		= p->prev;
	}
	
	if(p->prev)
	{
		p->prev->next		= p->next;
	}
	
	if(free_particles)
	{
		free_particles->prev	= p;
	}
	
	next					= p->next;
	
	p->next					= free_particles;
	
	free_particles			= p;
	
	p->prev	= NULL;
	
	return next;
}

/*
===============
R_ReadPointFile
===============
*/
void R_ReadPointFile_f (void)
{
	FILE	*f;
	vec3_t	origin;
	int		r;
	int		c;
	particle_t	*p;
	char	name[MAX_OSPATH];
	byte	*color;
	
	sprintf (name,"maps/%s.pts", sv.name);
	
	COM_FOpenFile (name, &f);
	if (!f)
	{
		Con_Printf ("couldn't open %s\n", name);
		return;
	}
	
	Con_Printf ("Reading %s...\n", name);
	c = 0;
	for ( ;; )
	{
		
		r = fscanf (f,"%f %f %f\n", &origin[0], &origin[1], &origin[2]);
		if (r != 3)
			break;
		c++;
		
		NEWPARTICLE();
		
		color = (byte *) &d_8to24table[(int)(-c)&15];		
		
		p->texnum			= particle_tex;
		p->contents			= 0;
		
		p->die				= 99;
		p->alpha			= 200;
		p->scale			= 2;
		p->bounce			= 0;
		
		COLOR(color[0],color[1],color[2]);
		
		p->origin[0]		= origin[0];
		p->origin[1]		= origin[1];
		p->origin[2]		= origin[2];
		
		p->velocity[0]		= vec3_origin[0];
		p->velocity[1]		= vec3_origin[1];
		p->velocity[2]		= vec3_origin[2];
		
		p->type				= pt_static;
		p->glow				= true;
	}
	
	fclose (f);
	Con_Printf ("%i points read\n", c);
}

/*
===============
R_EntityParticles
===============
*/

float	r_avertexnormals[162][3] = {
#include "anorms.h"
};

vec3_t			avelocities[128];



// Tei id old code

void R_DarkFieldParticles (entity_t *ent)
{
	int			i, j, k;
	particle_t	*p;
	float		vel;
	vec3_t		dir;
	vec3_t		org;
	
	org[0] = ent->origin[0];
	org[1] = ent->origin[1];
	org[2] = ent->origin[2];
	for (i=-16 ; i<16 ; i+=8)
		for (j=-16 ; j<16 ; j+=8)
			for (k=0 ; k<32 ; k+=8)
			{

				NEWPARTICLE();
				
				p->die = cl.time + 0.2 + (rand()&7) * 0.02;
				p->colorred			= 150 + rand()%6;
				p->colorgreen		= 150 + rand()%6;
				p->colorblue		= 150 + rand()%6;
				
				p->scale			= 2;
				p->alpha			= 200;
				p->texnum			= bubble_tex;//particle_tex;
				p->bounce			= 1;
				p->glow				= false;
				
				
				p->type = pt_grav;
				
				dir[0] = j*8;
				dir[1] = i*8;
				dir[2] = k*8;
				
				p->origin[0] = org[0] + i + (rand()&3);
				p->origin[1] = org[1] + j + (rand()&3);
				p->origin[2] = org[2] + k + (rand()&3);
				
				VectorNormalize (dir);						
				vel = 50 + (rand()&63);
				VectorScale (dir, vel, p->velocity);
			}
}
// Tei id old code


void R_BeamZing (vec3_t origin, vec3_t end);

void R_RayFieldParticles (vec3_t org)
{
	int			t;

	particle_t	*p;
//	float		vel;
//	vec3_t		dir;
	

	for (t=0 ; t<4 ; t++)
	{
	
		NEWPARTICLE();
		
				p->die = cl.time + 0.01;

				COLOR(200,200,255);
				
				p->scale			= 2;
				p->alpha			= 155;
				p->texnum			= GetTex(GEX_ZING);//particle_tex;
				p->bounce			= 1;
				p->glow				= true;
				
				
				p->type = pt_static;
				
				
				p->origin[0] = org[0] + lhrandom(-15,15);
				p->origin[1] = org[1] + lhrandom(-15,15);
				p->origin[2] = org[2] + lhrandom(-15,15);				

				VectorCopy(org,p->gorigin);

				p->hazetype = HZ_NORMAL;

				//R_BeamZing(p->origin,ent->origin);
				//VectorCopy(p->origin, oldorg);
	}
}
// Tei id old code



void R_EntityParticles (entity_t *ent)
{
	int			i;
	float		sp, sy, cp, cy, angle;
	vec3_t		forward;
	particle_t	*p;
	
	if (!avelocities[0][0])
	{
		for (i=0 ; i<384 ; i++)
		{
			avelocities[0][i] = (rand() & 255) * 0.01;
		}
	}
	
	for (i=0 ; i<128 ; i++)
	{
		angle				= cl.time * avelocities[i][0];
		sy					= sin(angle);
		cy					= cos(angle);
		
		angle				= cl.time * avelocities[i][1];
		sp					= sin(angle);
		cp					= cos(angle);
		
		forward[0]			= cp*cy;
		forward[1]			= cp*sy;
		forward[2]			= -sp;
		
		NEWPARTICLE();
		
		p->scale			= 2;
		p->alpha			= 200;
		p->die				= cl.time + 0.01;
		
		p->texnum			= particle_tex;
		p->bounce			= 0;

		COLOR(255,243,27);

		p->type				= pt_static;
		p->glow				= true;
		
		p->origin[0]		= ent->origin[0] + r_avertexnormals[i][0]*64 + forward[0]*16;			
		p->origin[1]		= ent->origin[1] + r_avertexnormals[i][1]*64 + forward[1]*16;			
		p->origin[2]		= ent->origin[2] + r_avertexnormals[i][2]*64 + forward[2]*16;			
		//R_BeamZing(p->origin,ent->origin);

	}
}

// Tei entpar2
void R_ShadowParticles (entity_t *ent)
{
	int			i;
	float		sp, sy, cp, cy, angle;
	vec3_t		forward;
	particle_t	*p;
	
	if (!avelocities[0][0])
	{
		for (i=0 ; i<384 ; i++)
		{
			avelocities[0][i] = (rand() & 255) * 0.01;
		}
	}
	
	for (i=0 ; i<128 ; i++)
	{
		angle				= cl.time * avelocities[i][0];
		sy					= sin(angle);
		cy					= cos(angle);
		
		angle				= cl.time * avelocities[i][1];
		sp					= sin(angle);
		cp					= cos(angle);
		
		forward[0]			= cp*cy;
		forward[1]			= cp*sy;
		forward[2]			= -sp;
		
		NEWPARTICLE();
		
		p->scale			= 12;
		p->alpha			= 200;
		p->die				= cl.time + 0.1;
		
		p->texnum			= smoke1_tex;
		if (rand()&1)
			p->texnum			= smoke2_tex;
		if (rand()&1)
			p->texnum			= particle_tex;
		
		p->bounce			= 0;

		COLOR(0,0,0);

		p->type				= pt_static;
		p->glow				= false;
		
		p->origin[0]		= ent->origin[0] + r_avertexnormals[i][0]*64 + forward[0]*16;			
		p->origin[1]		= ent->origin[1] + r_avertexnormals[i][1]*64 + forward[1]*16;			
		p->origin[2]		= ent->origin[2] + r_avertexnormals[i][2]*64 + forward[2]*16;			
	}
}
// Tei entpar2

/*
===============
R_ParseParticleEffect

  Parse an effect out of the server message
  ===============
*/
void R_ParseParticleEffect (void)
{
	vec3_t		origin, direction;
	int			count, color;
	
	origin[0]		= MSG_ReadCoord ();
	origin[1]		= MSG_ReadCoord ();
	origin[2]		= MSG_ReadCoord ();
	
	direction[0]	= MSG_ReadChar () * 0.0625;
	direction[1]	= MSG_ReadChar () * 0.0625;
	direction[2]	= MSG_ReadChar () * 0.0625;
	
	count			= MSG_ReadByte ();
	color			= MSG_ReadByte ();
	
	R_RunParticleEffect (origin, direction, color, count);
}

/*
===============
R_ParticleExplosion

  ===============
*/
void R_ParticleExplosion (vec3_t origin)
{
	int			i, contents;
	particle_t	*p;
	byte		*color;

	contents			= Mod_PointInLeaf(origin, cl.worldmodel)->contents;
	

	for (i=0 ; i<64 ; i++)
	{
		NEWPARTICLE();
		
		p->scale			= 1;//2;
		p->alpha			= 240;		
		p->texnum			= smoke1_tex + (rand() & 3);
		
		p->bounce			= 0;
		p->type				= pt_smokeexp;
		
		p->velocity[0]		= (rand() & 3) - 2;
		p->velocity[1]		= (rand() & 3) - 2;
		p->velocity[2]		= (rand() & 3) - 2;
		
		color				= (byte *) &d_8to24table[(int)(rand() & 7) + 105];

		COLOR(color[0],color[1],color[2]);

		p->die				= cl.time + 5;
		p->glow				= true;
		
		p->origin[0]		= origin[0] + (rand() &31) - 16;
		p->origin[1]		= origin[1] + (rand() &31) - 16;
		p->origin[2]		= origin[2] + (rand() &31) - 16;
		p->fxcolor				= 0;
		
	}
	
	for (i=0 ; i<256 ; i++)
		//for (i=0 ; i<8 ; i++) //Tei explo tech
	{

		NEWPARTICLE();	


		
		p->scale			= 2;//(rand() & 3) +1;
		p->alpha			= 250;
		p->die				= cl.time + 2;//5;
		
		p->fxcolor				= 0;
		
		
		if ((contents		== CONTENTS_EMPTY) ||
			(contents		== CONTENTS_SOLID))
		{
			if (rand()&1)
				p->texnum		= particle_tex;
			else
				p->texnum		= flare_tex;
			
			p->bounce		= 1.5;
			
			// Tei sparkles color
			COLOR( 255, 200, 100);
			
			p->type			= pt_explode;
			p->glow			= true;
		}
		else
		{
			p->texnum		= bubble_tex;
			p->bounce		= 0;
			
			COLOR( 127, 127, 255);
			
			p->type			= pt_bubble;
			p->glow			= false;
		}
		
		p->origin[0]		= origin[0] + ((rand() & 31) - 16);
		p->origin[1]		= origin[1] + ((rand() & 31) - 16);
		p->origin[2]		= origin[2] + ((rand() & 31) - 16);
		
		p->velocity[0]		= (rand() & 511) - 256;
		p->velocity[1]		= (rand() & 511) - 256;
		p->velocity[2]		= (rand() & 511) - 256;
	}
}


void R_ParticleExplosionX (vec3_t origin)
{
	int			i, contents;
	particle_t	*p;
    
	contents			= Mod_PointInLeaf(origin, cl.worldmodel)->contents;
	
	for (i=0 ; i<232 ; i++)
	{

		NEWPARTICLE();
		
		p->scale			= 2;//(rand() & 3) +1;
		p->alpha			= 150;
		p->die				= cl.time + 3;
		
		p->fxcolor				= FXC_AUTOGRAY3;
		
		//if (rand()&1)
		if ((contents		== CONTENTS_EMPTY) ||
			(contents		== CONTENTS_SOLID))
		{
			p->texnum		= particle_tex;
		}
		else
			p->texnum		= bubble_tex;
		
		//else
		//	p->texnum		= flare_tex;
		
		p->bounce		= 0;
		
		// Tei sparkles color
		COLOR( 255, 200, 100);
		
		p->type			= pt_grav;//pt_explode;
		p->glow			= true;
		
		p->origin[0]		= origin[0] + lhrandom(-8,8);
		p->origin[1]		= origin[1] + lhrandom(-8,8);
		p->origin[2]		= origin[2] + lhrandom(-8,8);
		
		p->velocity[0]		= lhrandom(-255,255);
		p->velocity[1]		= lhrandom(-255,255);
		p->velocity[2]		= lhrandom(-255,255);
	}
}


void R_ParticleTeExplosion (vec3_t origin)
{
	int			i, contents;
	particle_t	*p;
	byte		*color;

	contents			= Mod_PointInLeaf(origin, cl.worldmodel)->contents;

	
	//Con_Printf("Hello\n");
	for (i=0 ; i<64 ; i++)
	{

		NEWPARTICLE();
		
		p->scale			= 1;//2;
		
		p->alpha			= 240;
		
		p->texnum			= smoke1_tex + (rand() & 3);
		
		p->bounce			= 0;
		p->type				= pt_smokeexp;
		
		p->velocity[0]		= lhrandom(-32,32);//(rand() & 3) - 2;
		p->velocity[1]		= lhrandom(-32,32);//(rand() & 3) - 2;
		p->velocity[2]		= lhrandom(-32,32);//(rand() & 3) - 2;
		
		color				= (byte *) &d_8to24table[(int)(rand() & 7) + 105];
		
		COLOR(color[0],color[1],color[2]);

		p->die				= cl.time + 5;
		p->glow				= true;
		
		p->origin[0]		= origin[0] + lhrandom(-16,16);//(rand() &31) - 16;
		p->origin[1]		= origin[1] + lhrandom(-16,16);//(rand() &31) - 16;
		p->origin[2]		= origin[2] + lhrandom(-16,16);//(rand() &31) - 16;
	}
	
	//for (i=0 ; i<256 ; i++)
	for (i=0 ; i<16 ; i++) //Tei explo tech
	{

		NEWPARTICLE();
		
		p->scale			= 2;//(rand() & 3) +1;
		p->alpha			= 250;
		p->die				= cl.time + 2;//5;
		
		p->fxcolor			= FXC_STELAR;
		//p->fxcolor			= FXC_FIRE;
		
		if ((contents		== CONTENTS_EMPTY) ||
			(contents		== CONTENTS_SOLID))
		{
			if (rand()&1)
				p->texnum		= particle_tex;
			else
				p->texnum		= flare_tex;
			
			p->bounce		= 1.5;
			
			// Tei sparkles color
			COLOR(255,200,200);
			
			p->type			= pt_explode;
			p->glow			= true;
		}
		else
		{
			p->texnum		= bubble_tex;
			p->bounce		= 0;
			
			COLOR( 127,127,255);
			
			p->type			= pt_bubble;
			p->glow			= false;
		}
		
		p->origin[0]		= origin[0] + ((rand() & 31) - 16);
		p->origin[1]		= origin[1] + ((rand() & 31) - 16);
		p->origin[2]		= origin[2] + ((rand() & 31) - 16);
		
		p->velocity[0]		= lhrandom(-256,256);//(rand() & 511) - 256;
		p->velocity[1]		= lhrandom(-256,256);//(rand() & 511) - 256;
		p->velocity[2]		= lhrandom(-256,256);//(rand() & 511) - 256;
	}
}



// Tei implosion

void R_ParticleImplosion (vec3_t origin)
{
	int			i, k;
	particle_t	*p;
	
	
	for (i=0 ; i<16 ; i++)//16
	{

		NEWPARTICLE();

		p->scale			= 1;//10;//10;//10;//2;
		p->alpha			= 200;
		
		p->texnum			= flareglow_tex;// particle_tex;	//focus_tex;//GetTex(GEX_SNOW);//p
		//p->texnum			= focus_tex;//

		p->bounce			= 0;
		p->type				= pt_gorigin;
		
		p->gorigin[0]		= origin[0];
		p->gorigin[1]		= origin[1];
		p->gorigin[2]		= origin[2];
		
		COLOR( 255,255,255);

		//p->fxcolor = FXC_ALIENBLOOD2;
		
		p->glow				= true;
		
		p->die				= cl.time + 5;
		
		k = i  * 0.1;
		p->origin[0]		= origin[0] + (rand() &255*k) - 127*k;
		p->origin[1]		= origin[1] + (rand() &255*k) - 127*k;
		p->origin[2]		= origin[2] + (rand() &255*k) - 127*k;
		
		p->velocity[0] = (p->gorigin[0] - p->origin[0]) * 0.6;
		p->velocity[1] = (p->gorigin[1] - p->origin[1]) * 0.6;
		p->velocity[2] = (p->gorigin[2] - p->origin[2]) * 0.6;
		
	}
	
}

void R_ParticleImplosionHaze (vec3_t origin)
{
	int			i, k;
	particle_t	*p;
	
	
	for (i=0 ; i<16 ; i++)
	{

		NEWPARTICLE();
		
		p->scale			= 1;//2;
		p->alpha			= 100;
		p->texnum			= focus_tex;//particle_tex;
		p->bounce			= 0;


		p->type				= pt_explode;//expandfade;//gorigin;
		
		p->gorigin[0]		= origin[0];
		p->gorigin[1]		= origin[1];
		p->gorigin[2]		= origin[2];
		
		COLOR( 255,255,255);
		
		p->fxcolor = FXC_HAZE;
		p->hazetype = HZ_LIGHTCAOS;
		p->storm = 200;
		
		p->glow				= true;
		
		p->die				= cl.time + 5;
		
		k = i  * 0.1;
		p->origin[0]		= origin[0] + (rand() &255*k) - 127*k;
		p->origin[1]		= origin[1] + (rand() &255*k) - 127*k;
		p->origin[2]		= origin[2] + (rand() &255*k) - 127*k;
		
		p->velocity[0] = (p->gorigin[0] - p->origin[0]) * 0.6;
		p->velocity[1] = (p->gorigin[1] - p->origin[1]) * 0.6;
		p->velocity[2] = (p->gorigin[2] - p->origin[2]) * 0.6;
		
	}
	
}


void R_ParticleSuperHazes (vec3_t origin)
{
	int			i, k;
	particle_t	*p;
	
	
	for (i=0 ; i<32 ; i++)
	{

		NEWPARTICLE();
		
		p->scale			= 1;//2;
		p->alpha			= 100;
		p->texnum			= focus_tex;//particle_tex;
		p->bounce			= 0;


		p->type				= pt_explode;//expandfade;//gorigin;
		
		p->gorigin[0]		= origin[0];
		p->gorigin[1]		= origin[1];
		p->gorigin[2]		= origin[2];
		
		COLOR( 255,255,255);
		
		p->fxcolor = FXC_HAZE;
		p->hazetype = HZ_NORMAL;
		p->storm = 0;
		
		p->glow				= true;
		
		p->die				= cl.time + 5;
		
		k = i  * 0.1;
		p->origin[0]		= origin[0] + (rand() &255*k) - 127*k;
		p->origin[1]		= origin[1] + (rand() &255*k) - 127*k;
		p->origin[2]		= origin[2] + (rand() &255*k) - 127*k;
		
		p->velocity[0] = (p->gorigin[0] - p->origin[0]) * 0.6;
		p->velocity[1] = (p->gorigin[1] - p->origin[1]) * 0.6;
		p->velocity[2] = (p->gorigin[2] - p->origin[2]) * 0.6;
		
	}
	
}



void R_ParticleImplosionXray (vec3_t origin)
{
	int			i, k;
	particle_t	*p;
	
	
	for (i=0 ; i<16 ; i++)
	{
	
		NEWPARTICLE();
		
		p->scale			= 1;//2;
		p->alpha			= 100;
		p->texnum			= focus_tex;//particle_tex;
		p->bounce			= 0;

		p->type				= pt_expandfade;//pt_gorigin;
		
		p->gorigin[0]		= origin[0];
		p->gorigin[1]		= origin[1];
		p->gorigin[2]		= origin[2];
		
		COLOR( 255,255,255);
		
		//p->fxcolor = FXC_HAZE;
		if (teirand(25*i)<2) {
			p->hazetype = HZ_LIGHTSTORM;
			k = 4;
			p->storm = 50;
		}
		else {
			p->hazetype = HZ_NORMAL;
			k = teirand(64);
		}
		
		p->glow				= true;
		
		p->die				= cl.time + 0.5;//xfx less impresive

		p->origin[0]		= origin[0] + lhrandom(-k,k);
		p->origin[1]		= origin[1] + lhrandom(-k,k);
		p->origin[2]		= origin[2] + lhrandom(-k,k);

		k = 32;
		p->velocity[0] = lhrandom(-k,k);//(p->gorigin[0] - p->origin[0]) * k;
		p->velocity[1] = lhrandom(-k,k);//(p->gorigin[1] - p->origin[1]) * k;
		p->velocity[2] = lhrandom(-k,k);//(p->gorigin[2] - p->origin[2]) * k;
		
	}
	
}



void R_ParticleImpSuperSpike (vec3_t origin)
{
	int			i, k;
	particle_t	*p;
	
	
	for (i=0 ; i<16 ; i++)
	{

		NEWPARTICLE();
		
		p->scale			= 1;//2;
		p->alpha			= 100;
		p->texnum			= focus_tex;//particle_tex;
		p->bounce			= 0;

		p->type				= pt_expandfade;//pt_gorigin;
		
		p->gorigin[0]		= origin[0];
		p->gorigin[1]		= origin[1];
		p->gorigin[2]		= origin[2];
		
		COLOR( 255,255,255);
		
		p->hazetype = HZ_NORMAL;
		k = teirand(16);
		
		p->glow				= true;
		
		p->die				= cl.time + 0.5;//xfx less impresive

		p->origin[0]		= origin[0] + lhrandom(-k,k);
		p->origin[1]		= origin[1] + lhrandom(-k,k);
		p->origin[2]		= origin[2] + lhrandom(-k,k);

		k = 32;
		p->velocity[0] = lhrandom(-k,k);//(p->gorigin[0] - p->origin[0]) * k;
		p->velocity[1] = lhrandom(-k,k);//(p->gorigin[1] - p->origin[1]) * k;
		p->velocity[2] = lhrandom(-k,k);//(p->gorigin[2] - p->origin[2]) * k;
		
	}
	
}


void R_ParticleIstar (vec3_t origin)
{
	int			i, k;
	particle_t	*p;
	
	
	for (i=0 ; i<32 ; i++)
	{
		NEWPARTICLE();

		
		p->scale			= 1;//2;
		p->alpha			= 100;
		p->texnum			= focus_tex;//particle_tex;
		p->bounce			= 0;

		p->type				= pt_expandfade;//pt_gorigin;
		
		p->gorigin[0]		= origin[0];
		p->gorigin[1]		= origin[1];
		p->gorigin[2]		= origin[2];
		
		COLOR( 255,255,255);
		
		p->hazetype = HZ_NORMAL;
		k = teirand(16);
		
		p->glow				= true;
		
		p->die				= cl.time + 0.005;//xfx less impresive

		p->origin[0]		= origin[0] + lhrandom(-k,k);
		p->origin[1]		= origin[1] + lhrandom(-k,k);
		p->origin[2]		= origin[2] + lhrandom(-k,k);

		k = 32;
		p->velocity[0] = lhrandom(-k,k);//(p->gorigin[0] - p->origin[0]) * k;
		p->velocity[1] = lhrandom(-k,k);//(p->gorigin[1] - p->origin[1]) * k;
		p->velocity[2] = lhrandom(-k,k);//(p->gorigin[2] - p->origin[2]) * k;
		
	}
	
}



void R_ParticleBeam(vec3_t origin, vec3_t end)
{
	//nt			i, k;
	particle_t	*p;
	

	NEWPARTICLE();
	
	p->scale			= 1;//2;
	p->alpha			= 200;
	p->texnum			= particle_tex;
	p->bounce			= 0;
	p->type				= pt_fire;
	
	p->gorigin[0]		= origin[0];
	p->gorigin[1]		= origin[1];
	p->gorigin[2]		= origin[2];
	
	COLOR( 155,155,255);
	
	p->fxcolor = FXC_HAZE;
	
	p->glow				= true;
	
	p->die				= cl.time + 5;
	
	//k = i  * 0.1;
	p->origin[0]		= end[0];
	p->origin[1]		= end[1];
	p->origin[2]		= end[2];
	
	p->velocity[0] = (p->gorigin[0] - p->origin[0]) * 0.6;
	p->velocity[1] = (p->gorigin[1] - p->origin[1]) * 0.6;
	p->velocity[2] = (p->gorigin[2] - p->origin[2]) * 0.6;
}


// Tei particle fog
extern cvar_t sv_stepsize;
void R_ParticleFog (vec3_t origin)
{
	particle_t	*p;
	
	NEWPARTICLE();
	
	p->scale			= rand()&7;
	p->scalex			= 1+ rand()&15;
	p->scaley			= 1+ rand()&15;
	p->alpha			= lhrandom(0.1,3);//rand()&7;
	p->texnum			= smoke1_tex + (rand() & 3);

	p->bounce			= 0;
	
	p->type				= pt_fog;
	
	COLOR( 64, 64, 64 );
	
	p->glow				= false;//true;
	
	
	p->die				= cl.time + (rand()&15);//12
	
	p->origin[0]		= origin[0] + (rand() &255) - 127;
	p->origin[1]		= origin[1] + (rand() &255) - 127;
	p->origin[2]		= origin[2] + 1;
	
	p->velocity[0] = 0;
	p->velocity[1] = 0;
	p->velocity[2] = 0;
}

void R_ParticleFogX (vec3_t origin, float density)
{
	particle_t	*p;
	
	NEWPARTICLE();
	
	p->scale			= rand()&7;
	p->scalex			= 1+ rand()&15;
	p->scaley			= 1+ rand()&15;
	p->alpha			= lhrandom(0.1,density);//rand()&7;
	p->texnum			= smoke1_tex + (rand() & 3);

	p->bounce			= 0;
	
	p->type				= pt_fog;
	
	COLOR( 64, 64, 64 );
	
	p->glow				= false;//true;
	
	
	p->die				= cl.time + (rand()&15);//12
	
	p->origin[0]		= origin[0] + (rand() &255) - 127;
	p->origin[1]		= origin[1] + (rand() &255) - 127;
	p->origin[2]		= origin[2] + 1;
	
	p->velocity[0] = 0;
	p->velocity[1] = 0;
	p->velocity[2] = 0;
}

void R_ParticleFogColor (vec3_t origin, int red, int green, int blue, int alphabase)
{
	particle_t	*p;
	
	NEWPARTICLE();
	
	p->scale			= lhrandom(1,3);
	p->scalex			= lhrandom(8,260);
	p->scaley			= p->scalex;
	p->alpha			= lhrandom(10,alphabase);//rand()&7;
	p->texnum			= GetTex(GEX_B_SMOKE);//smoke1_tex + (rand() & 3);
	p->bounce			= 0;
	//p->type				= pt_snowfadein;
	
	p->type				= pt_fog2;
	
	COLOR(red,green,blue);
	
	p->glow				= false;//true;
	
	p->die				= cl.time + teirand(100);//(rand()&15);//12
	
	p->origin[0]		= origin[0] + (rand() &255) - 127;
	p->origin[1]		= origin[1] + (rand() &255) - 127;
	p->origin[2]		= origin[2] + 1;
	
	p->velocity[0] = 0;
	p->velocity[1] = 0;
	p->velocity[2] = 0;
}

void R_ParticleFogLava (vec3_t origin) //Dark version
{
	particle_t	*p;
	
	NEWPARTICLE();
	
	p->scale			= rand()&7;
	p->scalex			= 1+ rand()&15;
	p->scaley			= 1+ rand()&15;
	p->alpha			= rand()&7;
	p->texnum			= smoke1_tex + (rand() & 3);
	p->bounce			= 0;
	//p->type				= pt_snowfadein;
	
	p->type				= pt_fog;
	
	COLOR( 250, 100, 100);
	
	p->glow				= true;//false;//true;
	
	p->die				= cl.time + (rand()&15);//12
	
	p->origin[0]		= origin[0] + (rand() &255) - 127;
	p->origin[1]		= origin[1] + (rand() &255) - 127;
	p->origin[2]		= origin[2] + 1;
	
	p->velocity[0] = 0;
	p->velocity[1] = 0;
	p->velocity[2] = 0;//rand()&7;
}

// Tei particle fog

// Tei day light emu
void R_ParticleDay (vec3_t origin)
{
	particle_t	*p;
	
	NEWPARTICLE();
	
	p->scale			= rand()&3;
	p->scalez			= 1+ rand()&127;
	p->alpha			= rand()&15;//7
	p->texnum			= smoke1_tex + (rand() & 3);
	p->bounce			= 0;
	
	p->type				= pt_fog;
	
	COLOR( 255, 255, 255);
	
	p->glow				= false;//true;
	
	p->die				= cl.time + (rand()&7);//12
	
	if (rand()&1) {
		p->origin[0]		= origin[0] + (rand() &127) - 63;
		p->origin[1]		= origin[1] + (rand() &127) - 63;
	}
	else {
		p->origin[0]		= origin[0] + (rand() &63) - 15;
		p->origin[1]		= origin[1] + (rand() &63) - 15;
	}
	
	
	
	p->origin[2]		= origin[2] - 1;
	
	p->velocity[0] = 0;
	p->velocity[1] = 0;
	p->velocity[2] = 0;
}


void R_ParticleDay2 (vec3_t origin, int lux)
{
	particle_t	*p;
	
	NEWPARTICLE();
	
	p->scale			= rand()&3;
	p->scalez			= 1+ rand()&127;
	p->alpha			= rand()&3;
	p->texnum			= smoke1_tex + (rand() & 3);
	p->bounce			= 0;
	
	p->type				= pt_fog;
	
	COLOR( 255, 255, 255);
	
	p->glow				= false;//true;
	
	p->die				= cl.time + (rand()&7);//12
	
	switch (lux)
	{
	case 0:
		p->origin[0]		= origin[0] + (rand() &1) - 0.5;
		p->origin[1]		= origin[1] + (rand() &1) - 0.5;
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	default:
		p->origin[0]		= origin[0] + (rand() &lux) - lux*0.5;
		p->origin[1]		= origin[1] + (rand() &lux) - lux*0.5;
		break;
	}
	
	
	
	p->origin[2]		= origin[2] - 1;
	
	p->velocity[0] = 0;
	p->velocity[1] = 0;
	p->velocity[2] = 0;
}
// Tei day light emu


// Tei implosion sutil
void R_ParticleImplosionSutil (vec3_t origin)
{
	int			i;
	particle_t	*p;
	
	for (i=0 ; i<32 ; i++)
	{
		NEWPARTICLE();
		
		p->scale			= 1;//2;
		p->alpha			= 200;
		p->texnum			= particle_tex;
		p->bounce			= 0;
		p->type				= pt_gorigin;
		
		p->gorigin[0]		= origin[0];
		p->gorigin[1]		= origin[1];
		p->gorigin[2]		= origin[2];
		
		COLOR( 255, 255, 255);
		
		p->die				= cl.time + 15;
		p->glow				= true;
		
		p->origin[0]		= origin[0] + (rand() &127) - 63;
		p->origin[1]		= origin[1] + (rand() &127) - 63;
		p->origin[2]		= origin[2] + (rand() &127) - 63;
		
		p->velocity[0] = (p->origin[0] - p->gorigin[0]) * 0.1;
		p->velocity[1] = (p->origin[1] - p->gorigin[1]) * 0.1;
		p->velocity[2] = (p->origin[2] - p->gorigin[2]) * 0.1;
		
	}
	
}
// Tei implosion






// Tei par 2
void R_ParticleExplosion3 (vec3_t origin)
{
	int			i, contents;
	particle_t	*p;
	//	byte		*color;
	contents			= Mod_PointInLeaf(origin, cl.worldmodel)->contents;

		
	for (i=0 ; i<64 ; i++)
	{
		NEWPARTICLE();		
		
		p->scale			= 2 + (rand()&6);		
		p->alpha			= 200+(rand()&32);		
		p->texnum			= smoke1_tex + (rand() & 3);		
		p->bounce			= 0;
		
		p->type				= pt_fire;//pt_snow;//pt_smokeexp;
		
		p->velocity[0]		= (rand() & 3) - 2;
		p->velocity[1]		= (rand() & 3) - 2;
		p->velocity[2]		= (rand() & 3) - 2;
		
		COLOR( 0, 0, 0);
		
		p->die				= cl.time + 3;
		p->glow				= false;//true;
		
		p->origin[0]		= origin[0] + (rand() &16) - 7;
		p->origin[1]		= origin[1] + (rand() &16) - 7;
		p->origin[2]		= origin[2] + (rand() &16) - 7;
	}
	
	for (i=0 ; i<256 ; i++)
	{
		NEWPARTICLE();
		
		
		p->scale			= (rand() & 2) +1;
		p->alpha			= 100;
		p->die				= cl.time + 3;
		
		if ((contents		== CONTENTS_EMPTY) ||
			(contents		== CONTENTS_SOLID))
		{
			if (rand()&1)
				p->texnum		= particle_tex;
			else
				p->texnum		= flare_tex;
			
			p->bounce		= 1.5;
			
			COLOR( 255, 243, 255);
			
			
			p->type			= pt_explode;
			p->glow			= true;
		}
		else
		{
			p->texnum		= bubble_tex;
			p->bounce		= 0;
			
			COLOR( 127, 127, 255); 
			
			p->type			= pt_bubble;
			p->glow			= false;
		}
		
		p->origin[0]		= origin[0] + ((rand() & 31) - 16);
		p->origin[1]		= origin[1] + ((rand() & 31) - 16);
		p->origin[2]		= origin[2] + ((rand() & 31) - 16);
		
		p->velocity[0]		= (rand() & 128) - 64;
		p->velocity[1]		= (rand() & 128) - 64;
		p->velocity[2]		= (rand() & 128) - 64;
	}
}

#if 0
//Old version
void R_ParticleExplosion4 (vec3_t origin) //Dark
{
	int			i, contents;
	particle_t	*p;
	//	byte		*color;

	contents			= Mod_PointInLeaf(origin, cl.worldmodel)->contents;
	
	for (i=0 ; i<16 ; i++)
	{

		NEWPARTICLE();
		
		p->scale			= 12 + (rand()&7);
		
		p->alpha			= 200+(rand()&32);
		
		p->texnum			= smoke1_tex + (rand() & 3);
		
		p->bounce			= 0;
		
		p->type				= pt_fire;//pt_snow;//pt_smokeexp;
		
		p->velocity[0]		= (rand() & 7) - 4;
		p->velocity[1]		= (rand() & 7) - 4;
		p->velocity[2]		= (rand() & 7) - 4;
		
		COLOR( 0, 0, 0);
		
		p->die				= cl.time + 6;
		p->glow				= false;//true;
		
		p->origin[0]		= origin[0] + (rand() &31) - 16;
		p->origin[1]		= origin[1] + (rand() &31) - 16;
		p->origin[2]		= origin[2] + (rand() &31) - 16;
	}
	
	for (i=0 ; i<256 ; i++)
	{
		NEWPARTICLE();
		
		
		p->scale			= (rand() & 8) +5;
		p->alpha			= 200;
		p->die				= cl.time + 3;
		
		if ((contents		== CONTENTS_EMPTY) ||
			(contents		== CONTENTS_SOLID))
		{
			if (rand()&1)
				p->texnum		= particle_tex;
			else
				p->texnum		= flare_tex;
			
			p->bounce		= 1.5;
			
			COLOR( 255, 243, 255); 
			
			p->type			= pt_fire;//pt_explode;
			p->glow			= false;//true;
		}
		else
		{
			p->texnum		= bubble_tex;
			p->bounce		= 0;
			
			p->colorred		= 127;
			p->colorgreen	= 127;
			p->colorblue	= 255;
			
			p->type			= pt_bubble;
			p->glow			= false;
		}
		
		p->origin[0]		= origin[0] + ((rand() & 31) - 16);
		p->origin[1]		= origin[1] + ((rand() & 31) - 16);
		p->origin[2]		= origin[2] + ((rand() & 31) - 16);
		
		p->velocity[0]		= (rand() & 128) - 64;
		p->velocity[1]		= (rand() & 128) - 64;
		p->velocity[2]		= (rand() & 128) - 64;
	}
}
#else
//New version
void R_ParticleExplosion4 (vec3_t origin) //Dark
{
	int			i, contents;
	particle_t	*p;
	//	byte		*color;
	contents			= Mod_PointInLeaf(origin, cl.worldmodel)->contents;
	
	// TRAILS
	for (i=0 ; i<14 ; i++)
	{
		NEWPARTICLE();
		
		
		p->scale			= 1;
		
		p->alpha			= 200+(rand()&32);
		
		p->texnum			= smoke1_tex + (rand() & 3);
		
		p->bounce			= 1;
		
		p->type				= pt_explode;//smokeexp;//fire;//pt_snow;//pt_smokeexp;
		
		p->velocity[0]		= lhrandom(-105,105);
		p->velocity[1]		= lhrandom(-105,105);
		p->velocity[2]		= lhrandom(-105,195);
		
		//color				= (byte *) &d_8to24table[(int)(rand() & 7) + 105];
		COLOR( 0, 0, 0);
		
		p->fxcolor =		FXC_AUTOSMOKE2;
		p->die				= cl.time + 16;
		p->glow				= false;//true;
		
		p->origin[0]		= origin[0] + lhrandom(-6,6);
		p->origin[1]		= origin[1] + lhrandom(-6,6);
		p->origin[2]		= origin[2] + lhrandom(-6,6);
	}
	
	//BROWN SMOKE
	for (i=0 ; i<8 ; i++)
	{
		NEWPARTICLE();
		
		
		p->scale			= 23 + (rand()&31);
		p->alpha			= 210+(rand()&32);
		p->texnum			= smoke1_tex + (rand() & 3);
		p->bounce			= 1;
		p->type				= pt_slowmoke;//pt_snow;//pt_snow;//pt_smokeexp;
		
		p->velocity[0]		= (rand() & 7) - 4;
		p->velocity[1]		= (rand() & 7) - 4;
		p->velocity[2]		= (rand() & 15) - 4;
		
		//color				= (byte *) &d_8to24table[(int)(rand() & 7) + 105];
		COLOR( 40, 20, 10);
		
		p->die				= cl.time + 16;
		p->glow				= false;//true;
		
		p->origin[0]		= origin[0] + (rand() &31) - 16;
		p->origin[1]		= origin[1] + (rand() &31) - 16;
		p->origin[2]		= origin[2] + (rand() &31) - 8;
	}
	
}
#endif

//Tei decal
void R_Decal (vec3_t origin) 
{
	int		 contents;
	particle_t	*p;
	//	byte		*color;
	contents			= Mod_PointInLeaf(origin, cl.worldmodel)->contents;
	
	if (contents!=CONTENTS_EMPTY)
		return;

	NEWPARTICLE();
		
		
	p->scale			= 1;		
	p->alpha			= 200+(rand()&32);		
	p->texnum			= GetTex(GEX_SMOKE);		
	p->bounce			= 1;
		
	p->type				= pt_explode;//smokeexp;//fire;//pt_snow;//pt_smokeexp;
		
	p->velocity[0]		= lhrandom(-105,105);
	p->velocity[1]		= lhrandom(-105,105);
	p->velocity[2]		= lhrandom(-105,195);
		
		//color				= (byte *) &d_8to24table[(int)(rand() & 7) + 105];
	COLOR( 0, 0, 0);
		
	p->fxcolor =		FXC_AUTOSMOKE2;
	p->die				= cl.time + 16;
	p->glow				= false;//true;
		
	VectorCopy(origin,p->origin );
}



void R_ParticleExplosion5 (vec3_t origin) //Dark
{
	int			i, contents;
	particle_t	*p;
	//	byte		*color;
	contents			= Mod_PointInLeaf(origin, cl.worldmodel)->contents;
	
	for (i=0 ; i<8 ; i++)
	{
		p	= addParticle();
		if(!p)
		{
			return;
		}
		
		
		p->scale			= 23 + (rand()&31);
		p->alpha			= 210+(rand()&32);
		p->texnum			= smoke1_tex + (rand() & 3);
		p->bounce			= 1;
		p->type				= pt_slowmoke;//pt_snow;//pt_snow;//pt_smokeexp;
		
		p->velocity[0]		= (rand() & 7) - 4;
		p->velocity[1]		= (rand() & 7) - 4;
		p->velocity[2]		= (rand() & 15) - 4;
		
		//color				= (byte *) &d_8to24table[(int)(rand() & 7) + 105];
		COLOR( 40, 20, 10);
		
		p->die				= cl.time + 16;
		p->glow				= false;//true;
		
		p->origin[0]		= origin[0] + (rand() &31) - 16;
		p->origin[1]		= origin[1] + (rand() &31) - 16;
		p->origin[2]		= origin[2] + (rand() &31) - 8;
	}
	
	for (i=0 ; i<64 ; i++)
	{
		p	= addParticle();
		if(!p)
		{
			return;
		}
		
		
		p->scale			= 1;
		p->alpha			= 200;
		p->die				= cl.time + 13;
		
		if ((contents		== CONTENTS_EMPTY) ||
			(contents		== CONTENTS_SOLID))
		{
			if (rand()&1)
				p->texnum		= particle_tex;
			else
				p->texnum		= flare_tex;
			
			p->bounce		= 1.5;
			
			COLOR( 33, 33, 33);
			
			p->type			= pt_smoke;//explode;
			p->glow			= false;//true;
		}
		else
		{
			p->texnum		= bubble_tex;
			p->bounce		= 0;
			
			COLOR( 127, 127, 255);
			
			p->type			= pt_bubble;
			p->glow			= false;
		}
		
		p->origin[0]		= origin[0] + ((rand() & 31) - 16);
		p->origin[1]		= origin[1] + ((rand() & 31) - 16);
		p->origin[2]		= origin[2] + ((rand() & 31) - 8);
		
		p->velocity[0]		= (rand() & 255) - 128;
		p->velocity[1]		= (rand() & 255) - 128;
		p->velocity[2]		= (rand() & 255) - 16;
	}
}


// Tei par 2

/*
===============
R_ParticleExplosion2

  ===============
*/
void R_ParticleExplosion2 (vec3_t origin, int colorStart, int colorLength)
{
	int			i,contents;
	particle_t	*p;
	int			colorMod = 0;
	byte		*color;
	
	for (i=0; i<384; i++)
	{
		p	= addParticle();
		if(!p)
		{
			return;
		}
		
		contents			= Mod_PointInLeaf(p->origin, cl.worldmodel)->contents;
		
		p->scale			= (rand() & 3) +1;
		p->alpha			= 200;
		p->die				= cl.time + 5;
		
		if ((contents		== CONTENTS_EMPTY) ||
			(contents		== CONTENTS_SOLID))
		{
			p->texnum		= particle_tex;
			p->bounce		= 0;
			color			= (byte *) &d_8to24table[(int)colorStart + (colorMod % colorLength)];
			
			p->colorred		= color[0];
			p->colorgreen	= color[1];
			p->colorblue	= color[2];
			
			
			colorMod++;
			p->type			= pt_explode;
			p->glow			= true;
		}
		else
		{
			p->texnum		= bubble_tex;
			p->bounce		= 0;
			COLOR( 127, 127, 255);
			p->type			= pt_bubble;
			p->glow			= false;
		}
		
		p->origin[0]		= origin[0] + ((rand() & 31) - 16);
		p->origin[1]		= origin[1] + ((rand() & 31) - 16);
		p->origin[2]		= origin[2] + ((rand() & 31) - 16);
		
		p->velocity[0]		= (rand() & 511) - 256;
		p->velocity[1]		= (rand() & 511) - 256;
		p->velocity[2]		= (rand() & 511) - 256;
	}
}

/*
===============
R_BlobExplosion

  ===============
*/
void R_BlobExplosion (vec3_t origin)
{
	int			i;
	particle_t	*p;
	byte		*color;
	
	for (i=0 ; i<384 ; i++)
	{
		p	= addParticle();
		if(!p)
		{
			return;
		}
		
		p->die				= cl.time + 1;
		
		p->type				= pt_blob;
		p->glow				= true;
		
		color				= (byte *) &d_8to24table[(int)(rand() & 7) + 66];
		
		p->colorred			= color[0];
		p->colorgreen		= color[1];
		p->colorblue		= color[2];
		
		p->origin[0]		= origin[0] + (rand() & 31) - 16;
		p->origin[1]		= origin[1] + (rand() & 31) - 16;
		p->origin[2]		= origin[2] + (rand() & 31) - 16;
		
		p->velocity[0]		= (rand() & 511) - 256;
		p->velocity[1]		= (rand() & 511) - 256;
		p->velocity[2]		= (rand() & 511) - 256;
		
		if (i & 1)
		{
			p->type			= pt_blob2;
			
			color			= (byte *) &d_8to24table[(int)(rand() & 7) + 150];
			
			p->colorred		= color[0];
			p->colorgreen	= color[1];
			p->colorblue	= color[2];
		}
	}
}

/*
===============
R_RunParticleEffect

  ===============
*/
extern cvar_t mod_extendedparticle;//Tei extended particles
void R_FireExplo (vec3_t  origin, float scale);

void R_RunParticleEffect (vec3_t origin, vec3_t direction, int color, int count)
{
	int			i, contents;
	particle_t	*p;
	byte		*color24;
	
	if (count == 128)
	{
		for (i=0 ; i<count ; i++)
		{	// rocket explosion
			p	= addParticle();
			if(!p)
			{
				return;
			}
			
			contents			= Mod_PointInLeaf(p->origin, cl.worldmodel)->contents;
			
			p->nearest			= true; //best to see!
			
			p->scale			= 2;
			p->alpha			= 200;
			p->die				= cl.time + 5;
			
			if ((contents		== CONTENTS_EMPTY) ||
				(contents		== CONTENTS_SOLID))
			{
				p->texnum		= particle_tex;
				p->bounce		= 0;
				
				COLOR( 255, 243, 147);
				
				p->type			= pt_explode;
				p->glow			= true;
			}
			else
			{
				p->texnum		= bubble_tex;
				p->bounce		= 0;
				
				COLOR( 127, 127, 255);
				
				p->type			= pt_bubble;
				p->glow			= false;
			}

			//Tei improved particle effect
			p->origin[0]		= origin[0] + lhrandom(-i,i);//((rand() & 31) - 16);
			p->origin[1]		= origin[1] + lhrandom(-i,i);//((rand() & 31) - 16);
			p->origin[2]		= origin[2] + lhrandom(-i,i);//((rand() & 31) - 16);
			//Tei improved particle effect
			
			p->velocity[0]		= (rand() & 511) - 256;
			p->velocity[1]		= (rand() & 511) - 256;
			p->velocity[2]		= (rand() & 511) - 256;
		}
		return;
	}
	
//	Con_Printf("Particle, with color %d\n",color);

	for (i=0 ; i<count ; i++)
	{
		p	= addParticle();
		if(!p)
		{
			return;
		}
		
		p->texnum			= blood_tex;
		p->bounce			= 0;
		p->scale			= 2;
		p->alpha			= 200;
		p->die				= cl.time + 5;
		
		color24				= (byte *) &d_8to24table[(int)(rand() & 3) + color];
		
		p->colorred			= color24[0];
		p->colorgreen		= color24[1];
		p->colorblue		= color24[2];
		
		p->type				= pt_blood2;
		p->glow				= false;

		//Tei with improved particles fx
		p->origin[0]		= origin[0] +lhrandom(-i/2,i/2);// ((rand() & 15) - 8);
		p->origin[1]		= origin[1] +lhrandom(-i/2,i/2);// ((rand() & 15) - 8);
		p->origin[2]		= origin[2] +lhrandom(-i/2,i/2);// ((rand() & 15) - 8);
		//Tei with improved particles fx
		
		p->velocity[0]		= direction[0] * 15;
		p->velocity[1]		= direction[1] * 15;
		p->velocity[2]		= direction[2] * 15;

		if (mod_extendedparticle.value)
		{
			if (color == 73 /* || color == 225*/)//BLOOD
			{
				p->fxcolor	= FXC_BLOOD;
				p->bounce	= 1;
			}
			else
			if (color == 20) //alienblood
			{
				//Wizz spike hit
				p->fxcolor = FXC_ALIENBLOOD;
			}
			else {
				p->texnum = particle_tex;
				p->glow = true;
			}
		}
	}

	if (color==20 && mod_extendedparticle.value)
		R_ParticleExplosionAlien (origin);
	
	if (color==226 && mod_extendedparticle.value)
		R_FireExplo (origin,3);



}

/*
===============
R_SparkShower

  ===============
*/
void R_SparkShower (vec3_t origin, vec3_t direction)
{
	int			i, contents;
	particle_t	*p;
	
	p	= addParticle();
	if (!free_particles)
	{
		return;
	}
	
	contents			= Mod_PointInLeaf(p->origin, cl.worldmodel)->contents;
	
	if ((contents		== CONTENTS_EMPTY) ||
		(contents		== CONTENTS_SOLID))
	{
		p->scale			= 1;
		p->alpha			= lhrandom(100,150);
		p->texnum			= GetTex(GEX_SMOKE);//smoke1_tex + (rand() & 3);
		//p->texnum			= smoke_texFH + (rand() & 1)+(rand() & 1);
		
		p->bounce			= 0;
		p->type				= pt_bulletpuff;
		p->glow				= false;
		
		COLOR( 187, 187, 187);
		
		p->nearest			= false; //best to see!
		
		
		p->die				= cl.time + 5;
		
		VectorCopy(origin,p->origin);
		/*
		p->origin[0]		= origin[0];
		p->origin[1]		= origin[1];
		p->origin[2]		= origin[2];
		*/
		
		p->velocity[0]		= (rand() & 3) - 1;

		p->velocity[1]		= (rand() & 3) - 1;
		p->velocity[2]		= lhrandom(6,12);//Tei smoke up
	}


	for (i=0 ; i<10 ; i++)
	{
		p	= addParticle();
		if(!p)
		{
			return;
		}
		
		
		p->scale			= 1;//(rand() & 3) +1; //Tei black sparkles
		p->nearest			= true; //best to see!
		
		p->alpha			= 200;
		p->die				= cl.time + 5;
		
		if ((contents		== CONTENTS_EMPTY) ||
			(contents		== CONTENTS_SOLID))
		{
			p->texnum		= particle_tex;
			p->bounce		= 0;//1.5;
			
			
			p->die				= cl.time + 15;
			p->scale = 1;
			p->alpha			= 254;

			GetColor(COL_SPARK,p);
			
			p->type			= pt_explode;
			p->glow			= true;//false;//true; //Tei black sparkles
		}
		else
		{
			p->texnum		= bubble_tex;
			p->bounce		= 0;
			
			p->colorred		= 127;
			p->colorgreen	= 127;
			p->colorblue	= 255;
			
			p->type			= pt_bubble;
			p->glow			= false;
		}
		
		p->origin[0]		= origin[0] + ((rand() & 7) - 4);
		p->origin[1]		= origin[1] + ((rand() & 7) - 4);
		p->origin[2]		= origin[2] + ((rand() & 7) - 4);
		
		p->velocity[0]		= direction[0] + (rand() & 127) - 64;
		p->velocity[1]		= direction[1] + (rand() & 127) - 64;
		p->velocity[2]		= direction[2] + (rand() & 127) - 64;
	}
}

//Tei
void R_SparkShowerQ2 (vec3_t origin, vec3_t direction)
{
	int			i, contents;
	particle_t	*p;
	vec3_t		upa;
	
	VectorCopy( direction, upa);
	VectorNormalize(upa);
	
	p	= addParticle();
	if (!free_particles)
	{
		return;
	}
	
	contents			= Mod_PointInLeaf(p->origin, cl.worldmodel)->contents;
	
	if ((contents		== CONTENTS_EMPTY) ||
		(contents		== CONTENTS_SOLID))
	{
		p->scale			= 1;
		p->alpha			= 200;
		
		//p->scale			= 12;
		//p->alpha			= 90;
		
		
		p->texnum			= smoke1_tex + (rand() & 3);
		//p->texnum			= smoke_texFH + (rand() & 1)+(rand() & 1);
		
		p->bounce			= 0;
		p->type				= pt_bulletpuff;
		p->glow				= true;//false;
		
		COLOR( 187, 187, 187);
		
		p->nearest			= false; //best to see!
		
		
		p->die				= cl.time + 5;
		
		p->origin[0]		= origin[0] + upa[0] ;
		p->origin[1]		= origin[1] + upa[1] ;
		p->origin[2]		= origin[2] + upa[2] ;
		
		p->velocity[0]		= (rand() & 3) - 1;
		p->velocity[1]		= (rand() & 3) - 1;
		p->velocity[2]		= (rand() & 3) - 1;
	}
	
	for (i=0 ; i<123 ; i++)//23
	{
		p	= addParticle();
		if(!p)
		{
			return;
		}
		
		
		p->scale			= lhrandom(1,6);//(rand() & 3) +1; //Tei black sparkles
		p->nearest			= true; //best to see!
		
		p->alpha			= 200;
		p->die				= cl.time + 5;
		
		if ((contents		== CONTENTS_EMPTY) ||
			(contents		== CONTENTS_SOLID))
		{
			p->texnum		= particle_tex;
			p->bounce		= 0;//1.5;
			
			
			p->die				= cl.time + 15;
			p->scale = 1;
			p->alpha			= 254;

			GetColor(COL_SPARK,p);
			
			p->type			= pt_explode;
			p->glow			= true;//false;//true; //Tei black sparkles
		}
		else
		{
			p->texnum		= bubble_tex;
			p->bounce		= 0;
			
			p->colorred		= 127;
			p->colorgreen	= 127;
			p->colorblue	= 255;
			
			p->type			= pt_bubble;
			p->glow			= false;
		}
		
		p->origin[0]		= origin[0] ;
		p->origin[1]		= origin[1] ;
		p->origin[2]		= origin[2] ;
		
		p->velocity[0]		= direction[0] * lhrandom(1,10)+ lhrandom(-32,32);
		p->velocity[1]		= direction[1] * lhrandom(1,10)+ lhrandom(-32,32);
		p->velocity[2]		= direction[2] * lhrandom(1,10)+ lhrandom(-32,32);
	}
}

void R_SparkShowerQ3 (vec3_t origin, vec3_t direction)
{
	int			i, contents;
	particle_t	*p;
	
	p	= addParticle();
	if (!free_particles)
	{
		return;
	}
	
	contents			= CONTENTS_EMPTY;//Mod_PointInLeaf(p->origin, cl.worldmodel)->contents;
	
	if ((contents		== CONTENTS_EMPTY) ||
		(contents		== CONTENTS_SOLID))
	{
		p->scale			= 1;
		p->alpha			= 200;
		p->texnum			= smoke1_tex + (rand() & 3);
		//p->texnum			= smoke_texFH + (rand() & 1)+(rand() & 1);
		
		p->bounce			= 0;
		p->type				= pt_bulletpuff;
		p->glow				= false;
		
		COLOR( 0, 0, 0);
		
		p->nearest			= false; //best to see!
		
		
		p->die				= cl.time + 5;
		
		p->origin[0]		= origin[0];// + direction[0] * 0.5;
		p->origin[1]		= origin[1];// + direction[1] * 0.5;
		p->origin[2]		= origin[2];// + direction[2] * 0.5;
		
		p->velocity[0]		= (rand() & 3) - 1;
		p->velocity[1]		= (rand() & 3) - 1;
		p->velocity[2]		= (rand() & 3) - 1;
	}
	
	for (i=0 ; i<23 ; i++)
	{
		p	= addParticle();
		if(!p)
		{
			return;
		}
		
		
		p->scale			= lhrandom(1,6);//(rand() & 3) +1; //Tei black sparkles
		p->nearest			= true; //best to see!
		
		p->alpha			= 200;
		p->die				= cl.time + 5;
		
		if ((contents		== CONTENTS_EMPTY) ||
			(contents		== CONTENTS_SOLID))
		{
			p->texnum		= particle_tex;
			p->bounce		= 0;//1.5;
			
			
			p->die				= cl.time + 15;
			p->scale = 1;
			p->alpha			= 254;

			GetColor(COL_SPARK,p);
			
			p->type			= pt_fire;//explode;
			p->glow			= true;//false;//true; //Tei black sparkles

			//p->fxcolor = FXC_ALIENBLOOD;
		}
		else
		{
			p->texnum		= bubble_tex;
			p->bounce		= 0;
			
			p->colorred		= 127;
			p->colorgreen	= 127;
			p->colorblue	= 255;
			
			p->type			= pt_bubble;
			p->glow			= false;
		}
		
		p->origin[0]		= origin[0] ;
		p->origin[1]		= origin[1] ;
		p->origin[2]		= origin[2] ;
		
		p->velocity[0]		= direction[0] * lhrandom(1,10)+ lhrandom(-32,32);
		p->velocity[1]		= direction[1] * lhrandom(1,10)+ lhrandom(-32,32);
		p->velocity[2]		= direction[2] * lhrandom(1,10)+ lhrandom(-32,64);
	}
}


void R_SparkShowerQ3Fire (vec3_t origin, vec3_t direction)
{
	int			i, contents;
	particle_t	*p;
	
	p	= addParticle();
	if (!free_particles)
	{
		return;
	}
	
	contents			= CONTENTS_EMPTY;//Mod_PointInLeaf(p->origin, cl.worldmodel)->contents;
	
	if ((contents		== CONTENTS_EMPTY) ||
		(contents		== CONTENTS_SOLID))
	{
		p->scale			= 1;
		p->alpha			= 200;
		p->texnum			= smoke1_tex + (rand() & 3);
		//p->texnum			= smoke_texFH + (rand() & 1)+(rand() & 1);
		
		p->bounce			= 0;
		p->type				= pt_bulletpuff;
		p->glow				= false;
		
		COLOR( 0, 0, 0);
		
		p->nearest			= false; //best to see!
		
		
		p->die				= cl.time + 5;
		
		p->origin[0]		= origin[0];// + direction[0] * 0.5;
		p->origin[1]		= origin[1];// + direction[1] * 0.5;
		p->origin[2]		= origin[2];// + direction[2] * 0.5;
		
		p->velocity[0]		= (rand() & 3) - 1;
		p->velocity[1]		= (rand() & 3) - 1;
		p->velocity[2]		= (rand() & 3) - 1;
	}
	
	for (i=0 ; i<32 ; i++)
	{
		p	= addParticle();
		if(!p)
		{
			return;
		}
		
		
		p->scale			= lhrandom(1,6);//(rand() & 3) +1; //Tei black sparkles
		p->nearest			= true; //best to see!
		
		p->alpha			= 200;
		p->die				= cl.time + 5;
		
		if ((contents		== CONTENTS_EMPTY) ||
			(contents		== CONTENTS_SOLID))
		{
			p->texnum		= particle_tex;
			p->bounce		= 0;//1.5;
			
			
			//Tei black sparkles
			p->die				= cl.time + 15;
			p->scale = 1;
			p->alpha			= 254;
			COLOR( 255, 200, 100);
			//Tei black sparkles
			
			p->type			= pt_fire;//explode;
			p->glow			= true;//false;//true; //Tei black sparkles

			p->fxcolor = FXC_FIRE;
		}
		else
		{
			p->texnum		= bubble_tex;
			p->bounce		= 0;
			
			p->colorred		= 127;
			p->colorgreen	= 127;
			p->colorblue	= 255;
			
			p->type			= pt_bubble;
			p->glow			= false;
		}
		
		p->origin[0]		= origin[0] ;
		p->origin[1]		= origin[1] ;
		p->origin[2]		= origin[2] ;
		
		p->velocity[0]		= direction[0] * lhrandom(1,10)+ lhrandom(-32,32);
		p->velocity[1]		= direction[1] * lhrandom(1,10)+ lhrandom(-32,32);
		p->velocity[2]		= direction[2] * lhrandom(1,10)+ lhrandom(-32,64);
	}
}

//Tei


/*
===============
R_Snow
===============
*/
void R_Snow (vec3_t min, vec3_t max, int flakes)
{
	int			i;
	vec3_t		difference;
	particle_t	*p;
	
	for (i=0 ; i<flakes ; i++)
	{
		NEWPARTICLE();
		
		VectorSubtract(max, min, difference);
		
		p->die				= cl.time + 10;
		
		p->scale			= (rand() & 7) +3;
		p->alpha			= 100;//200;
		p->texnum			= GetTex(GEX_SNOW);//snow_tex;
		
		//if (rand()&1)
		//	p->texnum			= rain_tex;
		
		p->bounce			= 0;
		p->type				= pt_snow;
		p->glow				= true;//false;
		
		COLOR( 255, 255, 255);
		
		p->origin[0]		= difference[0] * (rand () & 2047) * 0.00048828125 + min[0];	// Tomaz - Speed
		p->origin[1]		= difference[1] * (rand () & 2047) * 0.00048828125 + min[1];	// Tomaz - Speed
		p->origin[2]		= max[2] - 10;
		
		p->velocity[0]		= 0;
		p->velocity[1]		= 0;
		p->velocity[2]		= -50;
	}
}

// Tei printemps

void R_TempusVivendi (vec3_t min, vec3_t max, int flakes)
{
	int			i;
	vec3_t		difference;
	particle_t	*p;
	
	for (i=0 ; i<flakes ; i++)
	{
		p	= addParticle();
		if(!p)
		{
			return;
		}
		
		VectorSubtract(max, min, difference);
		
		p->die				= cl.time + 10 * (rand() & 3);
		
		p->scale			= (rand() & 13) +1;
		p->alpha			= 200 + (rand() & 8) +1;
		p->texnum			= GetTex(GEX_SNOW);//snow_tex;
		p->bounce			= 0;
		p->type				= pt_snow;
		p->glow				= true;//false;
		
		
		p->colorred			= 215 + rand()&3 * 10 ;
		p->colorgreen		= 215 + rand()&3 * 10;
		p->colorblue		= 215 + rand()&3 * 10;		
		
		p->origin[0]		= difference[0] * (rand () & 2047) * 0.00048828125 + min[0];	// Tomaz - Speed
		p->origin[1]		= difference[1] * (rand () & 2047) * 0.00048828125 + min[1];	// Tomaz - Speed
		p->origin[2]		= max[2] - 10;
		
		/*
		p->velocity[0]		= 0;
		p->velocity[1]		= 0;
		p->velocity[2]		= -50;
		*/
		p->velocity[0]		= (50 - rand()&50) * 90;
		p->velocity[1]		= (50 - rand()&50) * 90;
		p->velocity[2]		= 25 - rand()&75;
	}
}

// Tei printemps


// Tei firetemps
/*
===============
R_Snow
===============
*/
void R_TempusFire (vec3_t min, vec3_t max, int flakes)
{
	int			i;
	vec3_t		difference;
	particle_t	*p;
	
	for (i=0 ; i<flakes ; i++)
	{
		p	= addParticle();
		if(!p)
		{
			return;
		}
		
		VectorSubtract(max, min, difference);
		
		p->die				= cl.time + 10 * (rand() & 3);
		
		p->scale			= 10 + (rand() & 13) +1;
		p->alpha			= 200 + (rand() & 8) +1;
		
		p->texnum			= smoke1_tex;
		if (rand()&1)
			p->texnum			= smoke2_tex;
		p->bounce			= 0;
		p->type				= pt_snow;
		p->glow				= true;//false;
		
		
		p->colorred			= 215 + rand()&3 * 10 ;
		p->colorgreen		= 215 + rand()&3 * 10;
		p->colorblue		= 215 + rand()&3 * 10;		
		
		
		p->colorred		= 227;
		p->colorgreen	= 151;
		p->colorblue	= 79;
		
		p->origin[0]		= difference[0] * (rand () & 2047) * 0.00048828125 + min[0];	// Tomaz - Speed
		p->origin[1]		= difference[1] * (rand () & 2047) * 0.00048828125 + min[1];	// Tomaz - Speed
		p->origin[2]		= max[2] - 10;
		
		/*
		p->velocity[0]		= 0;
		p->velocity[1]		= 0;
		p->velocity[2]		= -50;
		*/
		p->velocity[0]		= (50 - rand()&50) * 90 - 1140;
		p->velocity[1]		= (50 - rand()&50) * 90 - 1140;
		p->velocity[2]		= 25 - rand()&75;
	}
}

// Tei firetemps



// Tei smoketemps
/*
===============
R_Snow
===============
*/
void R_TempusSmoke (vec3_t min, vec3_t max, int flakes)
{
	int			i;
	vec3_t		difference;
	particle_t	*p;
	
	for (i=0 ; i<flakes ; i++)
	{
		p	= addParticle();
		if(!p)
		{
			return;
		}
		
		VectorSubtract(max, min, difference);
		
		p->die				= cl.time + 10 * (rand() & 3);
		
		p->scale			= 10 + (rand() & 13) +1;
		p->alpha			= 200 + (rand() & 8) +1;
		
		p->texnum			= smoke1_tex;
		if (rand()&1)
			p->texnum			= smoke2_tex;
		if (rand()&1)
			p->texnum			= smoke3_tex;
		if (rand()&1)
			p->texnum			= particle_tex;
		
		p->bounce			= 0;
		p->type				= pt_snow;
		p->glow				= true;//false;
		
		
		COLOR( 250, 250, 250);
		
		p->origin[0]		= difference[0] * (rand () & 2047) * 0.00048828125 + min[0];	// Tomaz - Speed
		p->origin[1]		= difference[1] * (rand () & 2047) * 0.00048828125 + min[1];	// Tomaz - Speed
		p->origin[2]		= max[2] - 10;
		
		/*
		p->velocity[0]		= 0;
		p->velocity[1]		= 0;
		p->velocity[2]		= -50;
		*/
		p->velocity[0]		= (50 - rand()&50) * 90;
		p->velocity[1]		= (50 - rand()&50) * 90;
		p->velocity[2]		= 25 - rand()&75;
	}
}

// Tei smoketemps


void R_TempusDarkSmoke (vec3_t min, vec3_t max, int flakes)
{
	int			i;
	vec3_t		difference;
	particle_t	*p;
	
	for (i=0 ; i<flakes ; i++)
	{
		p	= addParticle();
		if(!p)
		{
			return;
		}
		
		VectorSubtract(max, min, difference);
		
		p->die				= cl.time + 10 * (rand() & 3);
		
		p->scale			= 40 + (rand() & 43) +1;
		p->alpha			= 200 + (rand() & 8) +1;
		
		p->texnum			= smoke1_tex;
		if (rand()&1)
			p->texnum			= smoke2_tex;
		if (rand()&1)
			p->texnum			= smoke3_tex;
		if (rand()&1)
			p->texnum			= particle_tex;
		
		p->bounce			= 0;
		p->type				= pt_snow;
		p->glow				= false;
		
		COLOR( 0, 0 , 0);
		
		p->origin[0]		= difference[0] * (rand () & 2047) * 0.00048828125 + min[0];	// Tomaz - Speed
		p->origin[1]		= difference[1] * (rand () & 2047) * 0.00048828125 + min[1];	// Tomaz - Speed
		p->origin[2]		= max[2] - 10;
		
		p->velocity[0]		= (50 - rand()&50) * 90 - 1000;
		p->velocity[1]		= (50 - rand()&50) * 90 - 1000;
		p->velocity[2]		= 25 - rand()&75;
	}
}

// Tei dark smoketemps


/*
===============
R_Rain
===============
*/
void R_Rain (vec3_t min, vec3_t max, int drops)
{
	int			i;
	vec3_t		difference;
	particle_t	*p;
	

	for (i=0 ; i<drops ; i++)
	{
		p	= addParticle();
		if(!p)
		{
			return;
		}
		
		VectorSubtract(max, min, difference);
		
		p->die				= cl.time + 10;
		
		p->scale			= (rand() & 3) +1;
		p->scalez			= 100;//Tei rain best
		p->alpha			= 200;
		
		p->texnum			= particle_tex;//Tei rain best
		p->bounce			= 0;
		p->type				= pt_rain;
		p->glow				= false;
		
		COLOR( 255, 255, 255);
		
		p->origin[0]		= difference[0] * (rand () & 2047) * 0.00048828125 + min[0];	// Tomaz - Speed
		p->origin[1]		= difference[1] * (rand () & 2047) * 0.00048828125 + min[1];	// Tomaz - Speed
		p->origin[2]		= max[2] - 10;
		
		p->velocity[0]		= 0;
		p->velocity[1]		= 0;
		p->velocity[2]		= -400;
	}
}

//////////////////////////////////////////////////////////////////
// Tei fog plash

void R_FogSplashLite (vec3_t origin)
{
	int			i, j;
	particle_t	*p;
	
	for (i=-2 ; i<2 ; i++)
	{	
		for (j=-2 ; j<2 ; j++)
		{
			
			p	= addParticle();
			if(!p)
			{
				return;
			}
			
			if (rand()&1)
				return;
			
			p->texnum			= smoke3_tex;
			p->bounce			= 1;
			p->scale			= 6;
			p->alpha			= 221;
			p->die				= cl.time + 35;
			
			COLOR( 255, 222, 222);
			
			if (rand()&1)
				p->type				= pt_smoke;
			else
				p->type				= pt_fade;
			
			
			p->glow				= true;
			
			p->velocity[0]		= (rand() & 63 + 50) * (j*8 + (rand()&7)) * 0.003;
			p->velocity[1]		= (rand() & 63 + 50) * (i*8 + (rand()&7)) * 0.003;
			p->velocity[2]		= (rand() & 63 + 50) * ((i+j)*8 + (rand()&7)) * 0.02 + 120 + (120 * rand()&1);//((rand() & 63 + 50) * 256 + 50) * 0.01;
			
			p->origin[0]		= origin[0] + i * 10 + (rand() & 17);
			p->origin[1]		= origin[1] + j * 10 + (rand() & 17);
			p->origin[2]		= origin[2] + (i+j)*10 + (rand() & 7);
		}
	}
}

// Tei fog plash

//////////////////////////////////////////////////////////////////
// Tei fog plash

void R_FogSplash (vec3_t origin)
{
	int			i, j;
	particle_t	*p;
	
	for (i=-2 ; i<2 ; i++)
	{	
		for (j=-2 ; j<2 ; j++)
		{
			
			p	= addParticle();
			if(!p)
			{
				return;
			}
			
			p->texnum			= smoke3_tex;
			p->bounce			= 1;
			p->scale			= 45;
			p->alpha			= 200;
			p->die				= cl.time + 1345;
			
			if (rand()&1) {
				COLOR( 222, 222, 222);
			} else {
				COLOR(79, 151, 227);
			}
			
			if (rand()&1)
				p->type				= pt_smoke;
			else
				p->type				= pt_fade;
			
			
			
			p->glow				= true;
			
			p->velocity[0]		= (rand() & 63 + 50) * (j*8 + (rand()&7)) * 0.05;
			p->velocity[1]		= (rand() & 63 + 50) * (i*8 + (rand()&7)) * 0.05;
			p->velocity[2]		= (rand() & 63 + 50) * ((i+j)*8 + (rand()&7)) * 0.02 + 220 + (220 * rand()&1);//((rand() & 63 + 50) * 256 + 50) * 0.01;
			
			p->origin[0]		= origin[0] + i * 10 + (rand() & 27);
			p->origin[1]		= origin[1] + j * 10 + (rand() & 27);
			p->origin[2]		= origin[2] + (i+j)*10 + (rand() & 7);
		}
	}
}

// Tei fog plash

// Tei chof
void R_FogChof (vec3_t origin)
{
	int			i, j;
	particle_t	*p;
	
	for (i=-16 ; i<16 ; i++)
	{	
		for (j=-16 ; j<16 ; j++)
		{
			p	= addParticle();
			if(!p)
			{
				return;
			}
			
			p->texnum			= smoke3_tex;
			p->bounce			= 1;
			p->scale			= 28;
			p->alpha			= 200;
			p->die				= cl.time + 45;
			
			COLOR( 222, 222, 222);
			
			p->type				= pt_smoke;
			p->glow				= true;
			
			p->velocity[0]		= (rand() & 63 + 50) * (j*8 + (rand()&7));
			p->velocity[1]		= (rand() & 63 + 50) * (i*8 + (rand()&7));
			p->velocity[2]		= (rand() & 63 + 50) * 256 + 50;
			
			p->origin[0]		= origin[0] + (rand() & 7) + (j * 8);
			p->origin[1]		= origin[1] + (rand() & 7) + (i * 8);
			p->origin[2]		= origin[2] + (rand() & 63);
		}
	}
}

// Tei chof



/*
===============
R_LavaSplash
===============
*/
void R_LavaSplash (vec3_t origin)
{
	int			i, j;
	particle_t	*p;
	
	for (i=-16 ; i<16 ; i++)
	{	
		for (j=-16 ; j<16 ; j++)
		{
			p	= addParticle();
			if(!p)
			{
				return;
			}
			
			p->texnum			= particle_tex;
			p->bounce			= 0;
			p->scale			= 2;
			p->alpha			= 200;
			p->die				= cl.time + 5;
			
			COLOR( 163, 39, 11);
			
			p->type				= pt_grav;
			p->glow				= true;
			
			p->velocity[0]		= (rand() & 63 + 50) * (j*8 + (rand()&7));
			p->velocity[1]		= (rand() & 63 + 50) * (i*8 + (rand()&7));
			p->velocity[2]		= (rand() & 63 + 50) * 256;
			
			p->origin[0]		= origin[0] + (rand() & 7) + (j * 8);
			p->origin[1]		= origin[1] + (rand() & 7) + (i * 8);
			p->origin[2]		= origin[2] + (rand() & 63);
			
		}
	}
}

/*
===============
R_TeleportSplash

  ===============
*/
void R_TeleportSplash (vec3_t origin)
{
	int			i, j, k;
	particle_t	*p;
	
	for (i=-16 ; i<16 ; i+=4)
	{	
		for (j=-16 ; j<16 ; j+=4)
		{
			for (k=-16 ; k<16 ; k+=4)
			{
				p	= addParticle();
				if(!p)
				{
					return;
				}
				
				p->texnum			= bolt_tex;//particle_tex;
				p->bounce			= 0;
				p->scale			= lhrandom(1,10);
				p->alpha			= lhrandom(100,200);
				p->die				= cl.time + 1;
				
				COLOR( 255, 255, 255);
				
				p->type				= pt_fade;
				p->glow				= true;
				
				p->origin[0]		= origin[0] + i + (rand() & 7);
				p->origin[1]		= origin[1] + j + (rand() & 7);
				p->origin[2]		= origin[2] + k + (rand() & 7);
				
				p->velocity[0]		= i*2 + (rand() & 31) - 16;
				p->velocity[1]		= j*2 + (rand() & 31) - 16;
				p->velocity[2]		= k*2 + (rand() & 31) + 24;
			}
		}
	}
}

// Tei custom fx
void R_CustomFX (vec3_t origin,vec3_t color,vec3_t veloc , int num, int particle, int bounce, int scale, int die, int type )
{
	int			i;
	particle_t	*p;
	
	for (i= 0  ; i<num ; i++){	
		p	= addParticle();
		if(!p)
		{
			return;
		}
		
		p->texnum			= particle;
		p->bounce			= bounce;
		p->scale			= scale;
		p->alpha			= 200;
		p->die				= cl.time + die;
		
		p->colorred			= color[0];
		p->colorgreen		= color[1];
		p->colorblue		= color[2];				
		
		p->type				= type;
		p->glow				= true;
		
		p->origin[0]		= origin[0];
		p->origin[1]		= origin[1];
		p->origin[2]		= origin[2];
		
		p->velocity[0]		= veloc[0];
		p->velocity[1]		= veloc[1];
		p->velocity[2]		= veloc[2];
	}
}
// Tei custom fx
void AddTrailTrail2(vec3_t start, vec3_t end, float time, float size, vec3_t color);
void R_RailTrail (vec3_t start, vec3_t end, vec3_t angle)
{
	vec3_t		vec, oldpoint,norigin;
	float		len;
	vec3_t		forward, right, up,kcolor;
//	particle_t	*p;
//	byte		*color;

	VectorCopy(vec3_origin,oldpoint);
	
	VectorSubtract (end, start, vec);
	
	len = VectorNormalize(vec);
	//AddTrailTrail2(end,start,0.5,3,&d_8to24table[(int)(rand() & 7) + 208]);			
	
	kcolor[0]=kcolor[1]=kcolor[2]=1;

	VectorMA(vec3_origin,8,vec,vec);

	while (len > 0)
	{
		/*
		{  
			NEWPARTICLE();
			
			p->alpha			= 200;
			p->scale			= 3;
			p->die				= cl.time + 1;
			p->glow				= true;
			
			COLOR( 255, 255, 255);
			
			p->type				= pt_rail;
			//Tei new rail
			p->texnum			= particle_tex;
			//Tei new rail
			p->bounce			= 0;
			
			p->origin[0]		= start[0];
			p->origin[1]		= start[1];
			p->origin[2]		= start[2];
			
			p->velocity[0]		= 0;
			p->velocity[1]		= 0;
			p->velocity[2]		= 0;
		}
		*/
		AngleVectors (angle, forward, right, up);
		
		norigin[0]		= start[0] + right[0] * 6;
		norigin[1]		= start[1] + right[1] * 6;
		norigin[2]		= start[2] + right[2] * 6;
		
		angle[2] += 6;
	
		if (oldpoint[0]) {
			R_SuperZing(oldpoint, norigin);//XFX
			//AddTrailTrail2(oldpoint,norigin,0.5,3,kcolor);			
		}

		VectorCopy(norigin, oldpoint);

		len-=8;
		VectorAdd (start, vec, start);
	}                                     
}

// Tei custom fx

// Tei hipertrail

void R_ZTrail (vec3_t start, vec3_t end, vec3_t angle)
{
	vec3_t		vec;
	float		len;
	vec3_t		forward, right, up;
	particle_t	*p;
	byte		*color;
	
	VectorSubtract (end, start, vec);
	
	len = VectorNormalize(vec);
	
	while (len > 0)
	{
		p	= addParticle();
		if(!p)
		{
			return;
		}
		
		p->alpha			= 250;
		p->scale			= 4;
		p->die				= cl.time + 55;
		p->glow				= true;
		
		color				= (byte *) &d_8to24table[(int)(rand() & 7) + 208];
		
		p->colorred			= color[0];
		p->colorgreen		= color[1];
		p->colorblue		= color[2];
		
		p->type				= pt_rail;
		p->texnum			= flama_tex;
		p->bounce			= 0;
		
		AngleVectors (angle, forward, right, up);
		
		p->origin[0]		= start[0] + right[0];// * 3;
		p->origin[1]		= start[1] + right[1];// * 3;
		p->origin[2]		= start[2] + right[2];// * 3;
		
		p->velocity[0]		= right[0] * 80;
		p->velocity[1]		= right[1] * 80;
		p->velocity[2]		= right[2] * 80;
		
		angle[2] += 5;//5
		
		len--;
		VectorAdd (start, vec, start);
	}                                     
}

// Tei hipertrail
void R_FireExploLava (vec3_t  origin, float scale);
void R_FireFlama (vec3_t  origin, float scale);
void R_DowFire (entity_t *ent, qboolean fire2);
void DefineFlare(vec3_t origin, int radius, int mode, int alfa);
// Tei best RT



void R_RocketTrailX (vec3_t start)
{
	int			contents;
//	vec3_t		dir;
	particle_t	*p;
	
	
	contents = Mod_PointInLeaf(start, cl.worldmodel)->contents;
	
	if (contents == CONTENTS_SKY || contents == CONTENTS_LAVA)
		return;
	
	
//	Con_Printf("Trail-X!\n");
	
		p	= addParticle();
		if(!p)
		{
			return;
		}
		
		VectorCopy(  start , p->origin );
		
		if (contents == CONTENTS_EMPTY || contents == CONTENTS_SOLID)
		{
			p->die			= cl.time + 10;
			
			COLOR( 187, 187, 187);
			
			p->alpha		= 250;//ent->debris_smoke * 28;
			p->scale		= 3;//2;
			p->texnum		= smoke1_tex + (rand() & 3);//GetTex(GEX_FIRE);//
			p->type			= pt_smoke;//;//pt_smoke;
			p->bounce		= 0;
			//p->simple		= 1;//Tei
					
			p->velocity[0]	= (rand() & 15) - 8;
			p->velocity[1]	= (rand() & 15) - 8;
			p->velocity[2]	= teirand(20);//(rand() & 31) - 7;//+ 15;//+
			
		}
		else
		{
			p->die			= cl.time + 10;
			
			COLOR( 127, 127, 255);
			
			p->alpha		= 200;
			p->scale		= (rand() & 3) +1;
			p->texnum		= bubble_tex;
			p->bounce		= 0;
			p->type			= pt_bubble;
			
			p->velocity[0]	= 0;
			p->velocity[1]	= 0;
			p->velocity[2]	= 20;
		}
		
		p->glow				= false;

		p	= addParticle();
		if(!p)
		{
			return;
		}
		
		VectorCopy(  start , p->origin );
		
		if (contents == CONTENTS_EMPTY || contents == CONTENTS_SOLID)
		{
			p->die			= cl.time + 0.1;
			
			COLOR( 187, 187, 187);
			
			p->alpha		= 100;//ent->debris_smoke * 28;
			p->scale		= 11;//2;
			p->texnum		= GetTex(GEX_FIRE);//
			p->type			= pt_fire;//pt_smoke;//;//pt_smoke;
			p->bounce		= 0;
			//p->simple		= 1;//Tei
					
			p->velocity[0]	= (rand() & 15) - 8;
			p->velocity[1]	= (rand() & 15) - 8;
			p->velocity[2]	= teirand(30);//(rand() & 31) - 7;//+ 15;//+
			p->glow				= true;	
		}


}

/* Rocket Trail, ...eeehh.. yes, strange name, forget it */

void R_FireRocketTrailX (entity_t *ent, qboolean fire2)
{
	vec3_t med, med1, med2;//, dxv;

	int dx; //vlen distance 


	/* avoid far joins */
	//VectorSubtract(ent->origin, ent->oldorg, dxv) ;
	dx = dxvector(ent->origin, ent->oldorg);//dxv[0]+dxv[1]+dxv[2];


	if (ent->oldorg[0]==0 && dx>FAR_LERP)
	{
		VectorCopy(ent->origin,ent->oldorg);
	}

	DefineFlare(ent->origin, 100, 0,20);//Tei dp flares 

	if (ent->oldorg[0]!=ent->origin[0] && ent->oldorg[1]!=ent->origin[1] && ent->oldorg[2]!=ent->origin[2])
	{
	
		R_RocketTrailX(ent->origin);

//		Con_Printf("4x!\n");
	
		med[0] = (ent->origin[0]+ent->oldorg[0] )* 0.5;
		med[1] = (ent->origin[1]+ent->oldorg[1] )* 0.5;
		med[2] = (ent->origin[2]+ent->oldorg[2] )* 0.5;

		med1[0] = (ent->origin[0]+med[0] )* 0.5;
		med1[1] = (ent->origin[1]+med[1] )* 0.5;
		med1[2] = (ent->origin[2]+med[2] )* 0.5;

		med2[0] = (ent->oldorg[0]+med[0] )* 0.5;
		med2[1] = (ent->oldorg[1]+med[1] )* 0.5;
		med2[2] = (ent->oldorg[2]+med[2] )* 0.5;
		
		R_RocketTrailX(med);
		R_RocketTrailX(med1);
		R_RocketTrailX(med2);
	}
	else
	{
		
//	Con_Printf("1x!\n");
			R_RocketTrailX(ent->origin);
	}
}



void R_RocketTrail (vec3_t start, vec3_t end, entity_t *ent)
{
	int			contents, t;
	vec3_t		dir;
	particle_t	*p;
	
	if (ent->debris_smoke <= 0)
		return;
	
	contents = Mod_PointInLeaf(start, cl.worldmodel)->contents;
	
	if (contents == CONTENTS_SKY || contents == CONTENTS_LAVA)
		return;
	
	VectorSubtract(start, end, dir);
	VectorNormalize( dir );
	//
	


	//if (cl.time > ent->time_left)
	//{
	
	DefineFlare(start, 100, 0,20);//Tei dp flares 
	
	for (t=0;t<3;t++)
	{
		p	= addParticle();
		if(!p)
		{
			return;
		}
		
		
		VectorMA(start,lhrandom(1,3),dir, start);//Tei smooth smoke
		VectorCopy(  start , p->origin );
		
		if (contents == CONTENTS_EMPTY || contents == CONTENTS_SOLID)
		{
			p->die			= cl.time + 10;
			
			COLOR( 187, 187, 187);
			
			p->alpha		= 100;//ent->debris_smoke * 28;
			p->scale		= 3;//2;
			p->texnum		= smoke1_tex + (rand() & 3);
			p->type			= pt_smoke;
			p->bounce		= 0;
			//p->simple		= 1;//Tei
			
			if (ent->effects3 == EF3_CHEAPLENZ)
			{
				
				if (rand()&1) {
					p->scaley = 60;
					p->scalex = 1;
					p->scalez = 1;
				}
				else
					if (rand()&1)
						p->scalex = 60;
					else
						p->scalez = 60;
					
					
					p->type = pt_fadeout;
			}
			
			p->velocity[0]	= (rand() & 15) - 8;
			p->velocity[1]	= (rand() & 15) - 8;
			p->velocity[2]	= (rand() & 31) - 7;//+ 15;//+
			
		}
		else
		{
			p->die			= cl.time + 10;
			
			COLOR( 127, 127, 255);
			
			p->alpha		= 200;
			p->scale		= (rand() & 3) +1;
			p->texnum		= bubble_tex;
			p->bounce		= 0;
			p->type			= pt_bubble;
			
			p->velocity[0]	= 0;
			p->velocity[1]	= 0;
			p->velocity[2]	= 20;
		}
		p->glow				= false;
	}
}

void R_GrenadeTrail (vec3_t start, vec3_t end, entity_t *ent)
{
	int			contents, t;
	vec3_t		dir;
	particle_t	*p;
	
#if 0
	return; 
#else
	//TELEJANO
#endif
	
	
	if (ent->debris_smoke <= 0)
		return;
	
	contents = Mod_PointInLeaf(start, cl.worldmodel)->contents;
	
	if (contents == CONTENTS_SKY || contents == CONTENTS_LAVA)
		return;
	
	VectorSubtract(start, end, dir);
	VectorNormalize( dir );
	//
	
	//if (cl.time > ent->time_left)
	//{
	
	//DefineFlare(start, 100, 0,20);//Tei dp flares 
	
	for (t=0;t<3;t++)
	{
		NEWPARTICLE();
		
		VectorMA(start,lhrandom(1,3),dir, start);//Tei smooth smoke
		VectorCopy(  start , p->origin );
		
		if (contents == CONTENTS_EMPTY || contents == CONTENTS_SOLID)
		{
			p->die			= cl.time + 10;
			
			//

			COLOR( cl_grenclassic.value, cl_grenclassic.value, cl_grenclassic.value);
			
			p->alpha		= 170;//ent->debris_smoke * 28;
			p->scale		= 8;//2;
			p->texnum		= GetTex(GEX_SMOKE);//
			p->type			= pt_smoke;
			p->bounce		= 1;
			//p->simple		= 1;//Tei
			
			if (ent->effects3 == EF3_CHEAPLENZ)
			{
				
				if (rand()&1) {
					p->scaley = 60;
					p->scalex = 1;
					p->scalez = 1;
				}
				else
					if (rand()&1)
						p->scalex = 60;
					else
						p->scalez = 60;
					
					
					p->type = pt_fadeout;
			}
			
			p->velocity[0]	= (rand() & 15) - 8;
			p->velocity[1]	= (rand() & 15) - 8;
			p->velocity[2]	= (rand() & 31) - 7;//+ 15;//+
			
		}
		else
		{
			p->die			= cl.time + 10;
			
			COLOR( 127, 127, 255);
			
			p->alpha		= 200;
			p->scale		= (rand() & 3) +1;
			p->texnum		= bubble_tex;
			p->bounce		= 0;
			p->type			= pt_bubble;
			
			p->velocity[0]	= 0;
			p->velocity[1]	= 0;
			p->velocity[2]	= 20;
		}
		p->glow				= false;
	}
}
// Tei best RT

// Tei fx
void R_TrailGray (entity_t * e)
{
	particle_t	*p;
	
	p	= addParticle();
	if(!p)
	{
		return;
	}
	
	VectorCopy(e->origin, p->origin);
	
	p->die			= cl.time + 10;//e->frame;
	
	COLOR( 127, 127, 127);
	
	//p->alpha		= rand()&127;
	p->alpha		= rand()&63;
	
	//p->scale		= 32 + rand()&31;
    p->scale		= 16 + rand()&31;
	
	p->texnum		= smoke1_tex + (rand() & 3);
	p->type			= pt_slowmoke2;
	p->bounce		= 0;
	p->glow			= 0;
	p->velocity[0]	= ((rand() & 15) - 8)*2;
	p->velocity[1]	= ((rand() & 15) - 8)*2;
	p->velocity[2]	= (rand() & 15);
}

void R_TrailGray3 (entity_t * e)
{
	particle_t	*p;
	
	p	= addParticle();
	if(!p)
	{
		return;
	}
	
	VectorCopy(e->origin, p->origin);
	
	p->die			= cl.time + 10;//e->frame;
	
	COLOR( 127, 127, 127);
	
	p->alpha		= rand()&63;
	
    p->scale		= 1 + rand()&7;
	p->scalex		= 12 + rand()&3;
	p->scaley		= 12 + rand()&3;
	p->scalez		= 1;
	
	
	p->texnum		= smoke1_tex + (rand() & 3);
	p->type			= pt_slowmoke2;
	p->bounce		= 0;
	p->glow			= 0;
	p->velocity[0]	= ((rand() & 15) - 8)*2;
	p->velocity[1]	= ((rand() & 15) - 8)*2;
	p->velocity[2]	= (rand() & 15);
}


void R_TrailGray2 (entity_t * e)
{
	particle_t	*p;
	
	p	= addParticle();
	if(!p)
	{
		return;
	}
	
	VectorCopy(e->origin, p->origin);
	
	p->die			= cl.time + 10;//e->frame;
	
	COLOR( 127, 127, 127);
	
	p->alpha		= rand()&127;
	
	p->scale		= 32 + rand()&31;
	
	p->texnum		= smoke1_tex + (rand() & 3);
	p->type			= pt_slowmoke2;
	p->bounce		= 0;
	p->glow			= 0;
	p->velocity[0]	= ((rand() & 15) - 8)*2;
	p->velocity[1]	= ((rand() & 15) - 8)*2;
	p->velocity[2]	= (rand() & 15);
}

// Tei fx


// Tei circles trail
void R_CircleRocketTrail (vec3_t start, vec3_t end, entity_t *ent)
{
	int			contents;
	particle_t	*p;
	
	if (ent->debris_smoke <= 0)
		return;
	
	contents = Mod_PointInLeaf(start, cl.worldmodel)->contents;
	
	if (contents == CONTENTS_SKY || contents == CONTENTS_LAVA)
		return;
	
	if (cl.time > ent->time_left)
	{
		p	= addParticle();
		if(!p)
		{
			return;
		}
		
		VectorCopy(start, p->origin);
		
		if (contents == CONTENTS_EMPTY || contents == CONTENTS_SOLID)
		{
			p->die			= cl.time + 10;
			
			COLOR( 187, 187, 187);
			
			p->alpha		= ent->debris_smoke * 28;
			p->scale		= 2;
			p->texnum		= circle_tex;
			p->type			= 	pt_expandfade;//pt_slowmoke;//pt_fade	;//pt_static;
			p->bounce		= 0;
			p->glow				= false;
			p->velocity[0]	= 0;
			p->velocity[1]	= 0;
			p->velocity[2]	= 0;
			
		}
		else
		{
			p->die			= cl.time + 10;
			
			p->colorred		= 127;
			p->colorgreen	= 127;
			p->colorblue	= 255;
			COLOR( 127, 127, 255);
			
			p->alpha		= 200;
			p->scale		= (rand() & 3) +1;
			p->texnum		= bubble_tex;
			p->bounce		= 0;
			p->type			= pt_bubble;
			
			p->velocity[0]	= 0;
			p->velocity[1]	= 0;
			p->velocity[2]	= 20;
		}
		p->glow				= false;
		ent->time_left		= cl.time + 0.01;
		ent->debris_smoke	-= 0.05f;
	}
}


// Tei circles trail


// Tei Black estela begin


void R_DarkRocket (vec3_t start, int subtype);
void R_DarkFire (entity_t *ent, qboolean fire2)
{
	vec3_t med, med1, med2;

//	Con_Printf("ehlo\n");

	if (ent->oldorg[0]==0)
	{
//		Con_Printf("zulo\n");
		VectorCopy(ent->origin,ent->oldorg);
	}

	if (ent->oldorg[0]!=ent->origin[0] || ent->oldorg[1]!=ent->origin[1] || ent->oldorg[2]!=ent->origin[2])
	{
//		Con_Printf("111\n");

		R_DarkRocket(ent->origin,fire2);

		med[0] = (ent->origin[0]+ent->oldorg[0] )* 0.5;
		med[1] = (ent->origin[1]+ent->oldorg[1] )* 0.5;
		med[2] = (ent->origin[2]+ent->oldorg[2] )* 0.5;

		med1[0] = (ent->origin[0]+med[0] )* 0.5;
		med1[1] = (ent->origin[1]+med[1] )* 0.5;
		med1[2] = (ent->origin[2]+med[2] )* 0.5;

		med2[0] = (ent->oldorg[0]+med[0] )* 0.5;
		med2[1] = (ent->oldorg[1]+med[1] )* 0.5;
		med2[2] = (ent->oldorg[2]+med[2] )* 0.5;
		
		R_DarkRocket(med,fire2);
		R_DarkRocket(med1,fire2);
		R_DarkRocket(med2,fire2);
	}
	else
	{
			R_DarkRocket(ent->origin,fire2);
	}
}




void R_DarkRocket (vec3_t start, int subtype)
{
	int			contents;
	particle_t	*p;
	
	
	contents = Mod_PointInLeaf(start, cl.worldmodel)->contents;
	
	if (contents == CONTENTS_SKY || contents == CONTENTS_LAVA)
		return;
	
	p	= addParticle();
	if(!p)
	{
		return;
	}
	
	VectorCopy(start, p->origin);
	
	if (contents == CONTENTS_EMPTY || contents == CONTENTS_SOLID)
	{
		p->die			= cl.time + 10;
		
		COLOR( 0, 0, 0);
		
		p->alpha		= 230;
		p->scale		= 12;
		p->texnum		= smoke1_tex + (rand() & 3);
		
		// Smoke that not die
		p->type			= pt_snow			;//pt_snow;//pt_smoke;
		p->fxcolor = FXC_AUTOSMOKE;
		
		p->bounce		= 0;
		
		p->velocity[0]	= (rand() & 15) - 8;
		p->velocity[1]	= (rand() & 15) - 8;
		p->velocity[2]	= (rand() & 31) + 15;
		
	}
	else
	{
		p->die			= cl.time + 10;
		
		COLOR( 127, 127, 255);
		
		p->alpha		= 200;
		p->scale		= (rand() & 3) +1;
		p->texnum		= bubble_tex;
		p->bounce		= 0;
		p->type			= pt_bubble;
		
		p->velocity[0]	= 0;
		p->velocity[1]	= 0;
		p->velocity[2]	= 20;
	}
	p->glow				= false;
}

void R_DarkRocketTrail (vec3_t start, vec3_t end, entity_t *ent, qboolean subtype)
{
	int			contents;
	particle_t	*p;
	
	if (ent->debris_smoke <= 0)
		return;
	
	contents = Mod_PointInLeaf(start, cl.worldmodel)->contents;
	
	if (contents == CONTENTS_SKY || contents == CONTENTS_LAVA)
		return;
	
	if (cl.time > ent->time_left)
	{
		p	= addParticle();
		if(!p)
		{
			return;
		}
		
		VectorCopy(start, p->origin);
		
		if (contents == CONTENTS_EMPTY || contents == CONTENTS_SOLID)
		{
			p->die			= cl.time + 10;
			
			COLOR( 0, 0 , 0);
			
			p->alpha		= ent->debris_smoke * 28;
			p->scale		= 12;
			p->texnum		= smoke1_tex + (rand() & 3);
			if (subtype == true)
				p->type			= pt_fade			;//pt_snow;//pt_smoke;
			else
				// Smoke that not die
				p->type			= pt_snow			;//pt_snow;//pt_smoke;
			
			p->fxcolor = FXC_AUTOALFA;
			p->bounce		= 0;
			
			p->velocity[0]	= (rand() & 15) - 8;
			p->velocity[1]	= (rand() & 15) - 8;
			p->velocity[2]	= (rand() & 31) + 15;
			
		}
		else
		{
			p->die			= cl.time + 10;
			
			COLOR( 127, 127, 255);
			
			p->alpha		= 200;
			p->scale		= (rand() & 3) +1;
			p->texnum		= bubble_tex;
			p->bounce		= 0;
			p->type			= pt_bubble;
			
			p->velocity[0]	= 0;
			p->velocity[1]	= 0;
			p->velocity[2]	= 20;
		}
		p->glow				= false;
		ent->time_left		= cl.time + 0.01;
		ent->debris_smoke	-= 0.05f;
	}
}

// Tei Black estela end


void R_BloodTrail (vec3_t start, vec3_t end, entity_t *ent)
{
	particle_t	*p;
	byte		*color;
	
	if (cl.time > ent->time_left)
	{
		p	= addParticle();
		if(!p)
		{
			return;
		}
		
		p->die				= cl.time + 10;
		
		color				= (byte *) &d_8to24table[(int)(rand() & 3) + 68];
		
		p->colorred			= color[0];
		p->colorgreen		= color[1];
		p->colorblue		= color[2];
		
		p->alpha			= 200;
		p->scale			= 4;
		p->texnum			= blood_tex;
		p->bounce			= 0;
		p->type				= pt_blood2;
		p->glow				= false;
		
		p->velocity[0]		= (rand() & 15) - 8;
		p->velocity[1]		= (rand() & 15) - 8;
		p->velocity[2]		= (rand() & 15) - 8;
		
		p->origin[0]		= start[0] + ((rand() & 3) - 2);
		p->origin[1]		= start[1] + ((rand() & 3) - 2);
		p->origin[2]		= start[2] + ((rand() & 3) - 2);
		
		ent->time_left		= cl.time + 0.01;
		p->fxcolor = FXC_BLOOD;
	}
}

void R_TracerTrail (vec3_t start, vec3_t end, entity_t *ent, byte color)
{
	vec3_t		vec;
	static int	tracercount;
	particle_t	*p;
	byte		*color24;
	
	VectorSubtract (end, start, vec);
	
	if (cl.time > ent->time_left)
	{
		{
			p	= addParticle();
			if(!p)
			{
				return;
			}
			
			VectorCopy (start, p->origin);
			
			p->die				= cl.time + 0.3;
			
			color24				= (byte *) &d_8to24table[(int)color];
			
			p->colorred			= color24[0];
			p->colorgreen		= color24[1];
			p->colorblue		= color24[2];
			
			p->alpha			= 200;
			p->scale			= 3;
			p->texnum			= particle_tex;
			p->bounce			= 0;
			p->type				= pt_static;
			p->glow				= true;
			
			//if (lhrandom(0,100)<4)
			//	p->fxcolor = FXC_ALIENBLOOD;//Tei xfx// This is cool?
			
			p->velocity[0]		= 5 * -vec[1];
			p->velocity[1]		= 5 *  vec[0];
			p->velocity[2]		= 0;
		}
		
		{
			p	= addParticle();
			if(!p)
			{
				return;
			}
			
			VectorCopy (start, p->origin);
			
			p->die				= cl.time + 0.3;
			
			color24				= (byte *) &d_8to24table[(int)color];
			
			p->colorred			= color24[0];
			p->colorgreen		= color24[1];
			p->colorblue		= color24[2];
			
			p->alpha			= 200;
			p->scale			= 3;
			p->texnum			= particle_tex;
			p->bounce			= 0;
			p->type				= pt_static;
			p->glow				= true;
			
			p->velocity[0]		= 5 *  vec[1];
			p->velocity[1]		= 5 * -vec[0];
			p->velocity[2]		= 0;
		}
		ent->time_left			= cl.time + 0.01;
	}
}


void R_TracerKTrail (vec3_t start, vec3_t end, entity_t *ent, byte color)
{
	vec3_t		vec;
	static int	tracercount;
	particle_t	*p;
	byte		*color24;
	
	VectorSubtract (end, start, vec);
	
	if (cl.time > ent->time_left)
	{
		{
			p	= addParticle();
			if(!p)
			{
				return;
			}
			
			VectorCopy (start, p->origin);
			
			p->die				= cl.time + 0.3;
			
			color24				= (byte *) &d_8to24table[(int)color];
			
			p->colorred			= color24[0];
			p->colorgreen		= color24[1];
			p->colorblue		= color24[2];
			
			p->alpha			= 200;
			p->scale			= 8 + (rand()&3);
			p->texnum			= GetTex(GEX_LAVA);//_tex;//particle_tex;
			p->bounce			= 0;
			p->type				= pt_static;
			p->glow				= true;
			
			//if (lhrandom(0,100)<4)
			//	p->fxcolor = FXC_ALIENBLOOD;//Tei xfx// This is cool?
			
			p->fxcolor = FXC_AUTOGRAY3;
			
			if (rand()&1)
			{
				p->velocity[0]		= 5 * -vec[1];
				p->velocity[1]		= 5 *  vec[0];
			}
			else
			{
				p->velocity[0]		= 5 * vec[1];
				p->velocity[1]		= 5 * -vec[0];
			}
			p->velocity[2]		= vec[2];//0;
		}
		
		ent->time_left			= cl.time + 0.01;
	}
}


void R_TracerWizard (vec3_t start, vec3_t end, entity_t *ent, byte color)
{
	vec3_t		vec;
	static int	tracercount;
	particle_t	*p;
	byte		*color24;
	
	VectorSubtract (end, start, vec);
	
	if  ( vec[0]==0 && vec[1]==0 && vec[2]==0)
		return;
	
	
	if (cl.time > ent->time_left)
	{
		{
			p	= addParticle();
			if(!p)
			{
				return;
			}
			
			VectorCopy (start, p->origin);
			
			p->die				= cl.time + 0.3;
			
			color24				= (byte *) &d_8to24table[(int)color];
			
			p->colorred			= color24[0];
			p->colorgreen		= color24[1];
			p->colorblue		= color24[2];
			
			p->alpha			= 180;
			p->scale			= rand()&13;
			p->texnum			= particle_tex;
			p->bounce			= 0;
			p->type				= pt_static;//expand;//pt_;// pt_fire;
			p->glow				= true;
			
			p->velocity[0]		= vec[0] * 10;
			p->velocity[1]		= vec[1] * 10;
			p->velocity[2]		= vec[2] * 10;
			
			if (teirand(100)<50)
				p->fxcolor = FXC_ALIENBLOOD;//Tei xfx// This is cool?
		}
		ent->time_left			= cl.time + 0.01;
	}
}


void R_VoorTrail (vec3_t start, vec3_t end, entity_t *ent)
{
	particle_t	*p;
	
	if (cl.time > ent->time_left)
	{
		p	= addParticle();
		if(!p)
		{
			return;
		}
		
		p->die				= cl.time + 2;
		
		p->colorred			= 187;
		p->colorgreen		= 115;
		p->colorblue		= 159;		
		
		COLOR(187,115,159);
		
		p->alpha			= 200;
		p->scale			= 3;
		p->texnum			= particle_tex;
		p->bounce			= 0;
		p->type				= pt_fade;
		p->glow				= true;
		
		p->velocity[0]		= (rand() & 15) - 8;
		p->velocity[1]		= (rand() & 15) - 8;
		p->velocity[2]		= (rand() & 15) - 8;
		
		p->origin[0]		= start[0] + ((rand() & 3) - 2);
		p->origin[1]		= start[1] + ((rand() & 3) - 2);
		p->origin[2]		= start[2] + ((rand() & 3) - 2);
		
		ent->time_left		= cl.time + 0.01;
	}
}

/*
==========
R_Fire
==========
*/
void R_FireX (vec3_t org, qboolean fire2);


void R_Fire (entity_t *ent, qboolean fire2)
{
	vec3_t med, med1, med2;//, dxv;
	int dx;

	/* avoid far joins */
	//VectorSubtract(ent->origin, ent->oldorg, dxv) ;
	dx = dxvector(ent->origin, ent->oldorg);//dxv[0]+dxv[1]+dxv[2];

	if (ent->oldorg[0]==0 || dx > FAR_LERP)
	{
		VectorCopy(ent->origin,ent->oldorg);
	}

	if (ent->oldorg[0]!=ent->origin[0] && ent->oldorg[1]!=ent->origin[1] && ent->oldorg[2]!=ent->origin[2])
	{
	
		R_FireX(ent->origin,fire2);

		med[0] = (ent->origin[0]+ent->oldorg[0] )* 0.5;
		med[1] = (ent->origin[1]+ent->oldorg[1] )* 0.5;
		med[2] = (ent->origin[2]+ent->oldorg[2] )* 0.5;

		med1[0] = (ent->origin[0]+med[0] )* 0.5;
		med1[1] = (ent->origin[1]+med[1] )* 0.5;
		med1[2] = (ent->origin[2]+med[2] )* 0.5;

		med2[0] = (ent->oldorg[0]+med[0] )* 0.5;
		med2[1] = (ent->oldorg[1]+med[1] )* 0.5;
		med2[2] = (ent->oldorg[2]+med[2] )* 0.5;
		
		R_FireX(med,fire2);
		R_FireX(med1,fire2);
		R_FireX(med2,fire2);
	}
	else
	{
			R_FireX(ent->origin,fire2);
	}
}



void R_FireX (vec3_t org, qboolean fire2)
{

	particle_t	*p;
//	vec3_t		org;

	
	//VectorCopy(ent->origin, org);


		p	= addParticle();
		if(!p)
		{
			return;
		}
		
		p->die			= cl.time + 5;
		
		COLOR(227,151,79);
		
		p->alpha		= 128;
		p->scale		= 10;
		p->texnum		= particle_tex;//GetTex(GEX_FIRE);// firecycle_tex[t];//fire_tex;//particle_tex;
		p->bounce		= 0;
		p->type			= pt_fire;
		p->glow			= true;
		
		
		p->velocity[0]	= (rand() & 3) - 2;// + ent->origin[0] - ent->oldorg[0];
		p->velocity[1]	= (rand() & 3) - 2;// + ent->origin[0] - ent->oldorg[0];
		p->velocity[2]	= 0;//+ ent->origin[0] - ent->oldorg[0];
		
		p->origin[0]	= org[0];
		p->origin[1]	= org[1];
		p->origin[2]	= org[2] + 4;
		
}


void R_Fire2 (entity_t *ent, qboolean fire2)
{
	particle_t	*p;

			//Con_Printf("Zero\m");


	p	= addParticle();
	if(!p)
	{
		return;
	}
	
	p->die			= cl.time + 5;
	
	COLOR(227,151,79);
	
	p->alpha		= 128;
	p->scale		= (rand() & 7)+1;

	p->texnum		= GetTex(GEX_FIRE);
	//p->texnum		= firecycle_tex[t];//fire_tex;//particle_tex;

	p->bounce		= 0;
	p->type			= pt_fire;
	p->glow			= true;
	
	p->velocity[0]	= (rand() & 3) - 2;
	p->velocity[1]	= (rand() & 3) - 2;
	p->velocity[2]	= (rand() & 3) - 2;
	
	p->origin[0]	= ent->origin[0]+(rand() & 3) - 2;
	p->origin[1]	= ent->origin[1]+(rand() & 3) - 2;
	p->origin[2]	= ent->origin[2]+(rand() & 3) - 2;
	
	if (fire2)
	{
		p->origin[2]	= ent->origin[2] - 2;
		if (ent->frame)
		{
			p->scale		= 30;
			p->velocity[0]	= (rand() & 7) - 4;
			p->velocity[1]	= (rand() & 7) - 4;
			p->velocity[2]	= 0;
			p->type			= pt_fire2;
		}
	}
}

// Tei big fire

void R_BigFire (entity_t *ent, qboolean fire2)
{
	particle_t	*p;
	
	if( cl.time + 1 < ent->time_left )
	{
		ent->time_left = cl.time + 0.01;
	}
	
	if (cl.time > ent->time_left)
	{
		p	= addParticle();
		if(!p)
		{
			return;
		}
		
		p->die			= cl.time + 10;
		
		COLOR(227,151,79);
		
		p->alpha		= 128;
		p->scale		= 90;
		p->texnum		= particle_tex;
		p->bounce		= 0;
		p->type			= pt_fire;
		p->glow			= true;
		
		p->velocity[0]	= (rand() & 3) - 2;
		p->velocity[1]	= (rand() & 3) - 2;
		p->velocity[2]	= 0;//(rand() & 3) - 2;
		
		p->origin[0]	= ent->origin[0];
		p->origin[1]	= ent->origin[1];
		p->origin[2]	= ent->origin[2] + 4;
		
		if (fire2)
		{
			p->origin[2]	= ent->origin[2] - 2;
			if (ent->frame)
			{
				p->scale		= 30;
				p->velocity[0]	= (rand() & 7) - 4;
				p->velocity[1]	= (rand() & 7) - 4;
				p->velocity[2]	= 0;
				p->type			= pt_fire2;
			}
		}
		ent->time_left	= cl.time + 0.01;
	}
}
// Tei big fire


// Tei big fire down
/*
==========
R_DowFire
==========
*/
void R_DowFire (entity_t *ent, qboolean fire2)
{
	particle_t	*p;
	//vec3_t * vfor;
	float		sr, sp, sy, cr, cp, cy;
	float		angle;
	
	if( cl.time + 1 < ent->time_left )
	{
		ent->time_left = cl.time + 0.01;
	}
	
	if (cl.time > ent->time_left)
	{
		NEWPARTICLE();
		
		p->die			= cl.time + 20;
		
		if (rand()&1) {
			// fire1
			COLOR(227,151,79);
			
		}
		else{
			if (rand()&1) {
				if (rand()&1) {
					// gas
					COLOR(79,151,227);
				}
				else {
					// smoke
					COLOR(-79,-151,-227);
				}
			} else {
				// fire2
				COLOR(-79,-151,-227);
			}
			
		}
		/*
		// smoke
		p->colorred		= 10;
		p->colorgreen	= 10;
		p->colorblue	= 10;
		*/
		
		
		p->alpha		= 128;
		
		if (rand()&1){
			
			p->scale		= 40;
			if ( rand()&1 ) 
				p->scale		= 60;
		}
		else
			p->scale		= 25;
		
		if (rand()&1)
			p->texnum		= fire_tex;//snow_tex;//particle_tex;rain_tex;
		else 
			p->texnum		= smoke1_tex;//snow_tex;//particle_tex;rain_tex;
		
		//p->texnum = flama_tex;
		
		p->bounce		= 1;
		
		if (rand()&1)
			p->type			= pt_fire;
		else
			p->type			= pt_fire2;
		
		p->glow			= true;
		
		
		//AngleVectors
		
		angle	= DEG2RAD(ent->angles[YAW]);	// Tomaz Speed
		sy		= sin(angle);
		cy		= cos(angle);
		angle	= DEG2RAD(ent->angles[PITCH]);	// Tomaz Speed
		sp		= sin(angle);
		cp		= cos(angle);
		angle	= DEG2RAD(ent->angles[ROLL]);	// Tomaz Speed
		sr		= sin(angle);
		cr		= cos(angle);
		
		p->velocity[0]	= cp*cy * 160;
		p->velocity[1]	= cp*sy * 160;
		p->velocity[2]	=  -sp * 160;
		
		VectorCopy(ent->origin, p->origin);

	
		ent->time_left	= cl.time + 0.01;
	}
}
// Tei big fire down

// Tei - small enginefire begin
void R_SmallDowFire (entity_t *ent, qboolean fire2)
{
	particle_t	*p;
	//vec3_t * vfor;
	float		sr, sp, sy, cr, cp, cy;
	float		angle;
	
	if( cl.time + 1 < ent->time_left )
	{
		ent->time_left = cl.time + 0.01;
	}
	
	if (cl.time > ent->time_left)
	{
		NEWPARTICLE();
		
		p->die			= cl.time + 20;
		
		if (rand()&1) {
			// fire1
			COLOR(227,151,79);
		}
		else{
			if (rand()&1) {
				if (rand()&1) {
					// gas
					COLOR(79,151,227);
				}
				else {
					// smoke
					COLOR(-79,-151,-227);
				}
			} else {
				// fire2
				COLOR(-79,-151,-227);
			}
			
		}
		
		p->alpha		= 128;
		
		if (rand()&1){
			
			p->scale		= 6;
			if ( rand()&1 ) 
				p->scale		= 8;
		}
		else
			p->scale		= 5;
		
		if (rand()&1)
			p->texnum		= fire_tex;//snow_tex;//particle_tex;rain_tex;
		else 
			p->texnum		= smoke1_tex;//snow_tex;//particle_tex;rain_tex;
		
		//p->texnum = flama_tex;
		
		p->bounce		= 1;
		
		if (rand()&1)
			p->type			= pt_fire;
		else
			p->type			= pt_fire2;
		
		p->glow			= true;
		
		
		//AngleVectors
		
		angle	= DEG2RAD(ent->angles[YAW]);	// Tomaz Speed
		sy		= sin(angle);
		cy		= cos(angle);
		angle	= DEG2RAD(ent->angles[PITCH]);	// Tomaz Speed
		sp		= sin(angle);
		cp		= cos(angle);
		angle	= DEG2RAD(ent->angles[ROLL]);	// Tomaz Speed
		sr		= sin(angle);
		cr		= cos(angle);
		
		p->velocity[0]	= cp*cy * 16;
		p->velocity[1]	= cp*sy * 16;
		p->velocity[2]	=  -sp * 16;
			
		VectorCopy(ent->origin, p->origin);
	
		ent->time_left	= cl.time + 0.01;
	}
}


void R_FireBack (entity_t *ent, qboolean fire2)
{
	particle_t	*p;
	//vec3_t * vfor;
	float		sr, sp, sy, cr, cp, cy;
	float		angle, val;
		
	NEWPARTICLE();
	
	p->die			= cl.time + 5;
	
	p->alpha		= 128;
	
	COLOR(79,151,227);
	
	p->scale		= 7 + (rand()&31);
	p->scalex		= (rand()&1) + 1;
	p->scaley		= p->scalex;
	
	p->texnum		= particle_tex;
	
	p->bounce		= 0;
	
	if (rand()&1)
		p->type			= pt_fire;
	else
		p->type			= pt_fire2;
	
	p->glow			= true;
	
	
	angle	= DEG2RAD(ent->angles[YAW]);	// Tomaz Speed
	sy		= sin(angle);
	cy		= cos(angle);
	angle	= DEG2RAD(ent->angles[PITCH]);	// Tomaz Speed
	sp		= sin(angle);
	cp		= cos(angle);
	angle	= DEG2RAD(ent->angles[ROLL]);	// Tomaz Speed
	sr		= sin(angle);
	cr		= cos(angle);
	
	val = (rand()&31)+1;
	
	p->velocity[0]	= (cp*cy * val)* -20;
	p->velocity[1]	= (cp*sy * val)* -20;
	p->velocity[2]	=  (-sp * val ) * -20;
	
	
	VectorCopy(ent->origin, p->origin);

	//t->time_left	= cl.time + 0.01;
}

// Tei small enginefire end


// Tei magic fire down
/*
==========
R_DowFire
==========
*/
void R_MagicFire (entity_t *ent, qboolean fire2)
{
	particle_t	*p;
	//vec3_t * vfor;
	float		sr, sp, sy, cr, cp, cy;
	float		angle;
	
	if( cl.time + 1 < ent->time_left )
	{
		ent->time_left = cl.time + 0.01;
	}
	
	if (cl.time > ent->time_left)
	{
		NEWPARTICLE();
		
		p->die			= cl.time + 20;
		
		if (rand()&1) {
			// fire1
			COLOR(227,151,79);
		}
		else{
			if (rand()&1) {
				if (rand()&1) {
					// gas
					COLOR(79,151,227);
				}
				else {
					// smoke
					COLOR(-79,-151,-227);
				}
			} else {
				// fire2
				COLOR(-79,-151,-227);
			}
			
		}
		// smoke
		if (rand()&1) 
		{
			p->colorred		= 10 + rand()&129;
			p->colorgreen	= 10+ rand()&129;
			p->colorblue	= 10+ rand()&129;
		}
		
		
		p->alpha		= 128;
		
		if (rand()&1){
			
			p->scale		= 40;
			if ( rand()&1 ) 
				p->scale		= 60;
		}
		else
			p->scale		= 25;
		
		p->texnum		= rain_tex;
		
		p->bounce		= 0;
		
		
		//if (rand()&1)
		p->type			= pt_smokeexp;//pt_blob2;//pt_explode;//pt_snow;
		//else
		//	p->type			= pt_fire2;
		
		p->glow			= true;
		
		
		angle	= DEG2RAD(ent->angles[YAW]) + sin( anglemod(100*cl.time)/90*M_PI) * 5;	// Tomaz Speed
		sy		= sin(angle);
		cy		= cos(angle);
		angle	= DEG2RAD(ent->angles[PITCH]);	// Tomaz Speed
		sp		= sin(angle);
		cp		= cos(angle);
		angle	= DEG2RAD(ent->angles[ROLL]);	// Tomaz Speed
		sr		= sin(angle);
		cr		= cos(angle);
		
		p->velocity[0]	= cp*cy * 160  ;
		p->velocity[1]	= cp*sy * 160 ;
		p->velocity[2]	=  -sp * 160 ;
			
		
		VectorCopy(ent->origin, p->origin);
		
		ent->time_left	= cl.time + 0.01;
	}
}
// Tei magic fire down



// Tei big fire down
/*
==========
R_DowFire
==========
*/
void R_FireClassic (entity_t *ent, qboolean fire2)
{
	particle_t	*p;
	//vec3_t * vfor;
	
	if( cl.time + 1 < ent->time_left )
	{
		ent->time_left = cl.time + 0.01;
	}
	
	if (cl.time > ent->time_left)
	{
		NEWPARTICLE();

		
		p->die			= cl.time + 20;
		
		if (rand()&1) {
			// fire1
			COLOR(227,151,79);
			
		}
		else{
			if (rand()&1) {
				if (rand()&1) {
					// gas
					COLOR(79,151,227);
				}
				else {
					// smoke
					COLOR(-79,-151,-227);
				}
			} else {
				// fire2
				COLOR(-79,-151,-227);
			}
			
		}
		
		p->alpha		= 128;
		
		if (rand()&1){
			
			p->scale		= 40;
			if ( rand()&1 ) 
				p->scale		= 60;
		}
		else
			p->scale		= 25;
		
	
		p->texnum  = GetTex(GEX_FIRE);
		/*
		if (rand()&1)
			if (rand()&1)
				p->texnum		= fire_tex;//snow_tex;//particle_tex;rain_tex;
			else
				p->texnum		= smoke2_tex;//snow_tex;//particle_tex;rain_tex;
			else 
				p->texnum		= smoke1_tex;//snow_tex;//particle_tex;rain_tex;
		*/	
			//p->texnum = swdf.value;
			
			p->bounce		= 1;
			
			if (rand()&1)
				p->type			= pt_fire;
			else
				p->type			= pt_fire2;
			
			p->glow			= true;
			
			p->velocity[0]	= (rand() & 3) - 2;
			p->velocity[1]	= (rand() & 3) - 2;
			p->velocity[2]	= -200 - ((rand() & 8) );
				
			VectorCopy(ent->origin, p->origin);
			
			ent->time_left	= cl.time + 0.01;
	}
}
// Tei big fire down


// Tei - Fire explosion











// Tei waterfall
/*
==========
R_WaterFall
==========
*/
void R_WaterFall (entity_t *ent, qboolean fire2)
{
	particle_t	*p;
	//vec3_t * vfor;
	
	if( cl.time + 1 < ent->time_left )
	{
		ent->time_left = cl.time + 0.01;
	}
	
	if (cl.time > ent->time_left)
	{
		NEWPARTICLE();
		

		p->die			= cl.time + 80;
		
		if (rand()&1) {
			// fire1
			COLOR(250,250,250);
		}
		else{
			COLOR(79,151,227);
		}

		if (rand()&1)
			p->alpha		= 128;
		else
			p->alpha		= 250;
		
		
		if (rand()&1){
			
			p->scale		= 40;
			if ( rand()&1 ) 
				p->scale		= 60;
		}
		else
			p->scale		= 32;
		
		p->texnum = GetTex(GEX_SMOKE);
								
			p->bounce		= 1;
			
			if (rand()&1)
				p->type			= pt_fire;
			else
				p->type			= pt_fire2;
			
			p->glow			= true;
			
			p->velocity[0]	= (rand() & 3) - 2;
			p->velocity[1]	= (rand() & 3) - 2;
			p->velocity[2]	= -200 - ((rand() & 8) );
			
			
			p->origin[0]	= ent->origin[0] ;
			p->origin[1]	= ent->origin[1] ;
			p->origin[2]	= ent->origin[2] ;
			
			ent->time_left	= cl.time + 0.01;
	}
}
// Tei waterfall end




// Tei Blue fire

/*
==========
R_FireBlue
==========
*/
void R_FireBlue (entity_t *ent, qboolean fire2)
{
	particle_t	*p;
	
	if( cl.time + 1 < ent->time_left )
	{
		ent->time_left = cl.time + 0.01;
	}
	
	if (cl.time > ent->time_left)
	{
		NEWPARTICLE();
		
		p->die			= cl.time + 5;
		
		COLOR(79,151,227);
		
		p->alpha		= 128;
		p->scale		= 10;
		p->texnum		= particle_tex;
		p->bounce		= 0;
		p->type			= pt_fire;
		p->glow			= true;
		
		p->velocity[0]	= (rand() & 3) - 2;
		p->velocity[1]	= (rand() & 3) - 2;
		p->velocity[2]	= 0;//(rand() & 3) - 2;
		
		p->origin[0]	= ent->origin[0];
		p->origin[1]	= ent->origin[1];
		p->origin[2]	= ent->origin[2] + 4;
		
		if (fire2)
		{
			p->origin[2]	= ent->origin[2] - 2;
			if (ent->frame)
			{
				p->scale		= 30;
				p->velocity[0]	= (rand() & 7) - 4;
				p->velocity[1]	= (rand() & 7) - 4;
				p->velocity[2]	= 0;
				p->type			= pt_fire2;
			}
		}
		ent->time_left	= cl.time + 0.01;
	}
}

// Tei Blue fire


#define	DIST_EPSILON	(0.03125)

int SV_HullPointContents (hull_t *hull, int num, vec3_t p);

qboolean detectCollision( int num, vec3_t start, vec3_t end, vec3_t impact, vec3_t normal )
{
	dclipnode_t	*node, *nodes = cl.worldmodel->hulls->clipnodes;
	mplane_t	*plane, *planes = cl.worldmodel->hulls->planes;
	float		t1, t2;
	float		frac;
	vec3_t		mid;
	int			side;
	
	qboolean	t1neg, t2neg;
	
	while( num >= 0 )
	{
		t1neg = false;
		t2neg = false;
		//
		// find the point distances
		//
		node	= nodes + num;
		plane	= planes + node->planenum;
		
		t1		= start[plane->type] - plane->dist;
		t2		= end[plane->type] - plane->dist;
		
		if( plane->type >= 3 )
		{
			t1	= DotProduct (plane->normal, start) - plane->dist;
			t2	= DotProduct (plane->normal, end) - plane->dist;
		}
		
		if( t1 < 0 )
		{
			t1neg	= true;
		}
		if( t2 < 0 )
		{
			t2neg	= true;
		}
		
		if( !t1neg && !t2neg )
		{
			num	= node->children[0];
			continue;
		}
		
		if( t1neg && t2neg )
		{
			num	= node->children[1];
			continue;
		}
		
		// put the crosspoint DIST_EPSILON pixels on the near side
		if( t1neg )
		{
			frac	= (t1 + DIST_EPSILON)/(t1-t2);
		}
		else
		{
			frac	= (t1 - DIST_EPSILON)/(t1-t2);
		}
		
		//
		//	clamp to [0,1]
		if(frac < 0)
		{
			frac	= 0;
		}
		
		if(frac > 1)
		{
			frac	= 1;
		}
		
		mid[0]	= start[0] + frac*(end[0] - start[0]);
		mid[1]	= start[1] + frac*(end[1] - start[1]);
		mid[2]	= start[2] + frac*(end[2] - start[2]);
		
		side = ( t1 < 0 );
		
		// move up to the node
		if( detectCollision( node->children[side], start, mid, impact, normal ) )
		{
			return true;
		}
		
		if( SV_HullPointContents( cl.worldmodel->hulls, node->children[side^1], mid ) != CONTENTS_SOLID )
		{
			// go past the node
			num		= node->children[ side^1 ];
			
			//	NOTE: this is ok, because we won't need start any more! ONLY because of that!
			//			IE: CHANGE IT if you use it elsewhere, and keep using start
			VectorCopy( mid, start );
			
			continue;
		}
		
		//==================
		// the other side of the node is solid, this is the impact point
		//==================
		if( !side )
		{
			VectorCopy( plane->normal, normal );
		}
		else
		{
			VectorNegate( plane->normal, normal );
		}
		
		VectorCopy( mid, impact );
		
		return true;
	}
	
	//	empty
	return false;
}

extern	qboolean	mirror_render;
extern	cvar_t		sv_gravity;

//Tei volumetric 
float CalcVolLevel ( particle_t * pe , float maxdx)
{
	particle_t	*p				= active_particles;
	float sal = 0;
	
	while( p )
	{
		if ( (fabs( p->origin[0] - pe->origin[0] ) + fabs( p->origin[1] - pe->origin[1] ) +fabs( p->origin[2] - pe->origin[2] )) < maxdx )
			sal++;
		p	= p->next;
	}
	return sal;
};
// Tei volumetric


extern cvar_t sv_stepsize;//Tei
extern cvar_t temp1;
extern cvar_t r_autofogfog;//Tei

void R_ParticleBubble (vec3_t origin);

//
//	R_MoveParticles
//

void R_FireStaticFog (vec3_t  origin);

extern cvar_t r_autowindx;
extern cvar_t r_autowindy;
extern cvar_t r_autowindz;



void R_MoveParticles( void )
{
	particle_t	*p				= active_particles;
	
	
	//
	//	time vars
	//
	
	const float		frametime		= cl.time - cl.oldtime;
	const float		time			= frametime * 10;
	const float		fastgrav		= frametime * sv_gravity.value * 0.5f;
	const float		slowgrav		= frametime * sv_gravity.value * 0.05f;
	const float		evenslowergrav	= frametime * sv_gravity.value * 0.02f;
	
	const float		ft2				= frametime * 2.0f;
	const float		ft4				= frametime * 4.0f;
	const float		ft8				= frametime * 8.0f;
	const float		ft16			= frametime * 16.0f;
	const float		ft32			= frametime * 32.0f;
	const float		ft64			= frametime * 64.0f;
	const float		ft128			= frametime * 128.0f;
	const float		ft256			= frametime * 256.0f;
	
	float dx;	//Tei

	
	//
	//	bounce  vars
	//
	
	vec3_t		impact, normal, oldorigin, rvec;
	float		dist;
	

	//XFX
	if (cl.oldtime == cl.time)
		return;

	//
	//	loop through all the particles, doing physics for each
	//

	//Tei more ambient
	if (r_autofogfog.value)
	{
		rvec[0] = lhrandom(-4000,4000);
		rvec[1] = lhrandom(-4000,4000);

		if(r_autofogfog.value == 2)
			rvec[2] = lhrandom(-1060,-800);
		else
			rvec[2] = lhrandom(-4000,-4000);
		R_FireStaticFog (rvec);
	}
	//Tei more ambient
	

	while( p )
	{
		
		//	if its dead, kill it...
		//if (p->type == pt_static )
		
			if( p->die < cl.time )
			{
				// Tei polite particle deletion
				p->type = pt_fadeout;
				/*
				p	= remParticle( p );
				continue;
				*/
				p->alpha	   -= ft8;
				
				p->die = cl.time + 0.3;
				
				if (p->texnum == smoke1_tex)
					p->texnum = smoke2_tex;
				else
					if (p->texnum == smoke2_tex)
						p->texnum = smoke1_tex;
					// Tei polite particle deletion
			}
			
			
			
			
			//
			//	move the particle
			//
			
			VectorCopy(p->origin, oldorigin);
			
			p->origin[0] += p->velocity[0] * frametime;
			p->origin[1] += p->velocity[1] * frametime;
			p->origin[2] += p->velocity[2] * frametime;
			

			//Tei haze moves!
			p->gorigin[0] += p->gvelocity[0] * frametime;
			p->gorigin[1] += p->gvelocity[1] * frametime;
			p->gorigin[2] += p->gvelocity[2] * frametime;
			//Tei hazes moves!


			//Tei wind
			if (r_autowindx.value || r_autowindy.value || r_autowindz.value)
			{
				p->origin[0] += r_autowindx.value;
				p->origin[1] += r_autowindy.value;
				p->origin[2] += r_autowindz.value;
			}

			//Tei wind
			
			//
			//	handle all the velocity/alpha/scale physics and particle-specific stuff
			//	do this here so we can kill as many alpha and scale out particles as possible before bouncing
			//
			
			// Tei fxcolor
			switch( p->fxcolor)
			{
			case FXC_AUTOGRAY:
				if(p->colorblue)
					p->colorblue--;
				if(p->colorred)
					p->colorred--;
				if(p->colorgreen)
					p->colorgreen--;
				break;
			case FXC_AUTOGRAY2:
				//p->glow = false;
				if (rand()&1)
					break;
				if(p->colorblue)
					p->colorblue  = p->colorblue * 0.999;
				if(p->colorred)
					p->colorred   = p->colorred * 0.999;
				if(p->colorgreen)
					p->colorgreen = p->colorgreen * 0.999;
				break;
			case FXC_AUTOGRAY3:
				if(p->colorblue)
					p->colorblue  = p->colorblue * 0.999;
				if(p->colorred)
					p->colorred   = p->colorred * 0.999;
				if(p->colorgreen)
					p->colorgreen = p->colorgreen * 0.999;
				break;
				
			case FXC_AUTOALFA:
				p->alpha		-= ft32;
				p->scale		+= ft8;
				if(p->colorblue<255)
					p->colorblue++;
				if(p->colorred<255)
					p->colorred++;
				if(p->colorgreen<255)
					p->colorgreen++;
			case FXC_AUTOSMOKE:
				p->alpha		-= ft128;
				p->scale		+= ft2;
				if(p->colorblue<255)
					p->colorblue++;
				if(p->colorred<255)
					p->colorred++;
				if(p->colorgreen<255)
					p->colorgreen++;
				break;
			case FXC_STELAR:
				//R_ParticleImplosion( p->origin );
				p->alpha		-= ft128;
				p->scale		+= ft2;
				
				//R_ParticleSmoke( p->origin , p->alpha * 0.3f);
				R_ParticleStelar( p->origin , p->alpha * 0.3f);
				
				break;
			case FXC_AUTOSMOKE2:
				p->alpha		-= ft8;
				p->scale		+= ft8;

				if (cl.oldtime == cl.time)
					break;

				if (rand()&1)
					R_ParticlePuff( p->origin , 100);
				else
					R_ParticlePuff( p->origin , p->alpha);
				
				break;
			case FXC_BLOOD:
				R_Blood( p->origin , p->alpha * 0.3f);
				break;
			case FXC_ALIENBLOOD:
				R_AlienBlood( p->origin);
				break;
				
			case FXC_ALIENBLOOD2:
				if (cl.oldtime == cl.time)
					break;

				R_AlienBlood2( p->origin);
				break;
				
			case FXC_DeeTeeLAZER:
				if (cl.time > p->time)
				{
					p->alpha -= frametime * 256;
					p->time = cl.time + (rand() & 2) * 0.1;
					p->velocity[0] = (rand()&60)-25+ p->velocity[0];
					p->velocity[1] = (rand()&60)-25+ p->velocity[1];
					p->velocity[2] = (rand()&60)-25+ p->velocity[2];
					if (p->alpha < 1)
						p->die = -1;
				}
				break;
			case FXC_VERTIHAZE:
				p->gorigin[2] = p->origin[2] + 63;
				break;
			case FXC_FIRE:
				p->scale += ft2;//XFX
				p->alpha -= ft32;
				//p->alpha *= 0.5;

				p->texnum = particle_tex;

				if (!p->fxcolor)
					p->fxcolor = FXC_AUTOGRAY3;
				
				//if (teirand(100)<5)
					R_ParticleFire(p->origin, 3, 6);
				DefineFlareColor(p->origin,18,0,90,250,0,0);
				break;
			case FXC_ZING:
				p->texnum = GetTex(GEX_ZING);
				break;
			default:
				break;
		}
		// Tei fxcolor
		
		if( p->type & ( pt_grav | pt_fade | pt_blood ) )
		{
			p->velocity[2]	-= slowgrav;
			p->alpha		-= ft128;//Tei blood spha
		}
		
		if( p->type & ( pt_blood2 | pt_explode ) )
		{
			p->velocity[2]	-= fastgrav;
		}
		
		if( p->type & ( pt_smoke | pt_bulletpuff | pt_smokeexp | pt_fade | pt_rail ) )
		{
			p->alpha		-= ft256;
		}
		
		if( p->type & ( pt_blood2 | pt_explode ) )
		{
			p->alpha		-= ft128;
		}
		
		if( p->type & ( pt_fire | pt_fire2 ) )
		{
			p->alpha		-= ft64;
			

			//Tei natural fire hack			
			if (teirand(100)<5)
			if (p->texnum > firecycle_tex[0] && p->texnum < firecycle_tex[5]) 
			{		
				p->texnum = p->texnum + 1;				
			}
			else
			if (p->texnum == particle_tex)
				p->texnum = GetTex(GEX_FIRE);
			else
			if( p->texnum == flama_tex && teirand(100)<50)
				p->texnum = particle_tex;
			//Tei natural fire hack
		}
		
		if( p->type & ( pt_smoke | pt_bulletpuff | pt_smokeexp ) )
		{
			p->scale		+= ft16;
		}
		
		switch( p->type )
		{
			// Tei static fix
		case pt_static:
			{
				p->time = cl.time + 3;
				break;
			}
			// Tei static fix
			
			
		case pt_rain:
			{
				p->velocity[2] = -fabs(p->velocity[2]) - 10 ;
				//p->type = pt_fadeout;
				p->velocity[0] = 0;
				p->velocity[1] = 0;
				break;
			}
			
			
		case pt_bubble:
			{
				p->velocity[0] = (rand() & 15) - 8;
				p->velocity[1] = (rand() & 15) - 8;
				p->velocity[2] = (rand() & 31) + 64;
				
				if (teirand(50)<2)
					R_ParticleBubble(p->origin);//Tei XFX very much boubles.
				break;
			}
			
		case pt_explode:
			{
				p->scale		-= ft2;
				
				break;
			}
			
		case pt_snow:
			{
				if (cl.time > p->time)
				{
					p->time = cl.time + (rand() & 3) * 0.1;
					p->velocity[0] = (rand() & 31) - 16;
					p->velocity[1] = (rand() & 31) - 16;
				}	
				break;
			}
			
		case pt_snowfade:
			{
				if (cl.time > p->time)
				{
					p->time = cl.time + (rand() & 3) * 0.1;
					p->velocity[0] = (rand() & 31) - 16;
					p->velocity[1] = (rand() & 31) - 16;
					p->scale	   += ft16;
					if (p->alpha > ft32)
						p->alpha	   -= ft32;
					else
						p->alpha = 0;
					p->velocity[1] = p->velocity[1] + (rand() & 7);
				}	
				break;
			}
			
			
		case pt_fire:
			{
				p->velocity[2]	+= evenslowergrav;
				p->scale		-= ft2;
				
				break;
			}
			
		case pt_fire2:
			{
				p->velocity[2]	+= slowgrav;
				p->scale		-= ft4;
				
				break;
			}
			
		case pt_slowmoke:
			{
				if (cl.time > p->time)
				{
					p->time = cl.time + (rand() & 3) * 0.1;
					p->velocity[0] = (rand() & 31) - 16;
					p->velocity[1] = (rand() & 31) - 16;
					p->velocity[2] += slowgrav;
					p->alpha	   -= ft256;
				}
				
				break;
			}
		case pt_slowmoke2:
			{
				if (cl.time > p->time)
				{
					p->time = cl.time + (rand() & 3) * 0.1;
					p->velocity[0] = p->velocity[0] * 0.5 + (rand() & 7) - 3;
					p->velocity[1] = p->velocity[1] * 0.5 + (rand() & 7) - 3;
					p->velocity[2] += slowgrav * 0.1;
					p->alpha	   -= ft128;
					p->scale	   += ft128;
				}
				
				break;
			}		
			
		case pt_expandfade:
			{
				p->alpha	   -= ft128;//ft256;
				p->scale	   += ft4;
				break;
				
			}
			
		case pt_expand:
			{
				p->alpha	   -= ft64;//ft256;
				p->scale	   += ft128;
				break;
			}
			
		case pt_fadeout:
			{
				p->alpha = p->alpha * 0.4;
				break;
			}
			
		case pt_fog2:
			{
				p->alpha +=  ft2;
				p->scale += ft2;
				
				if ( p->type == pt_fog2 && (teirand(512)<2) )
					p->type = pt_expandfade;//more fps
				
				if (p->die < (cl.time + 3)) {
					if (rand()&1)
						p->type = pt_expandfade;				
				}
				break;
			}
		case pt_fog:
			{
				p->alpha +=  ft2;
				p->scale += ft4;
				
				if ( p->type == pt_fog2 && (teirand(64)<2) )
					p->type = pt_expandfade;//more fps
				
				if (p->die < (cl.time + 3)) {
					if (rand()&1)
						p->type = pt_expandfade;
					
				}
				break;
			}
			
		case pt_gorigin:
			{
				//p->alpha	-= ft8;
				//p->scale	+= ft16;
				
				p->alpha	-= ft16;
				//p->scale	+= ft8;
				
				dx = fabs( p->origin[0] - p->gorigin[0] ) + fabs( p->origin[1] - p->gorigin[1] ) +fabs( p->origin[2] - p->gorigin[2] );
				if ( dx < 5)
					p->type = pt_fadeout;
				break;
			}
			
		default:
			break;
			
		}
		
		
		//
		//	if alpha is <= 0, its invisible, loose it; if scale is <= 0, its invisible, loose it
		//
		
		if( p->alpha <= 0.0f || p->scale <= 0.0f )
		{
			p	= remParticle( p );
			continue;
		}		
		
		
		//
		//	this is a big CPU eater... but oh well, its needed. =] we check other break out situations first
		//	then run this check
		//	
		//	also, we're doing this stuff before the bounce is thought of because... well... never seen stuff bounce all that much off the sky. ;]
		//
		
		p->contents = Mod_PointInLeaf(p->origin, cl.worldmodel)->contents;
		
		
		//
		//	check for sky impact - no ammount of bouncing matters here
		//
		
		if( p->contents == CONTENTS_SKY )
		{
			p = remParticle(p);
			continue;
		}
		
		
		//
		//	handle bounce - only do this if its in solid... ;] no need to bust our balls for nothing...
		//
		
		if( p->bounce )
		{
			if( detectCollision( 0, oldorigin, p->origin, impact, normal ) )
			{
				VectorCopy(impact, p->origin);
				
				dist = DotProduct(p->velocity, normal) * -p->bounce;
				
				VectorMA(p->velocity, dist, normal, p->velocity);
				
				if( DotProduct(p->velocity, p->velocity) < 0.03 )
				{
					VectorClear(p->velocity);
					p->bounce = 0;
				}
			}
		}
		
		//
		//	now we check for being inside a solid, and get rid of it if it is - note, not bouncing stuff inside of a solid - it bounces back out
		
		//
		
		if( p->contents == CONTENTS_SOLID && !p->bounce )
		{
			if (p->type == pt_rain)
				R_ParticleExplosionSmoke(p->origin);
			
			p	= remParticle( p );

			continue;
		}
		
		
		//
		//	lastly, we have a bunch more CONTENTS checks, but only on certain particle type combos...
		//
		
		//	check for bubbles outa the water
		if( ( p->type & pt_bubble ) && ( p->contents == CONTENTS_EMPTY ) )
		{
			p	= remParticle( p );

			continue;
		}
		
		//	check for stuff in the water...
		if( ( p->type & ( pt_explode | pt_bulletpuff | pt_smokeexp | pt_slowmoke ) ) && ( p->contents != CONTENTS_EMPTY ) && ( p->contents != CONTENTS_SOLID ) )
		{
			p	= remParticle( p );
			
			continue;
		}
		
		


		//
		//	move pointer up one
		//
		
		p	= p->next;
	}
}


//
//	R_DrawParticles
//

extern cvar_t gl_particle_limiter;//Tei limit particles in face


void R_DrawParticles( qboolean waterpart )
{
	particle_t	*p			 	= active_particles;
	
	int			texnum;
	
	byte		alpha;
	
	vec3_t		up				= { vup[0] * 0.75f, vup[1] * 0.75f, vup[2] * 0.75f };
	vec3_t		right			= { vright[0] * 0.75f, vright[1] * 0.75f, vright[2] * 0.75f };
	
	vec3_t		coord[4];
	
	vec3_t		v;//Tei no particles in face
	float	dist;//Tei no particles in face
//	int		lastnearest;//Tei nearest support
	
	//
	//	if there aren't any particles active, don't try to draw particles!! =]
	//
	
	if( !p )
	{
		return;
	}
	
	//
	//	create the coordinates along the plane parallel to our view
	//
	
	VectorAdd( up, right, coord[0] );
	VectorSubtract( right, up, coord[1] );
	VectorNegate( coord[0], coord[2] );
	VectorNegate( coord[1], coord[3] );
	
	//
	//	setup for the loop
	//
	
	if (gl_fogenable.value)//Tei fix for square particles
	{
		glDisable(GL_FOG);
	}


	//	fix up texnum
	texnum		= p->texnum;
	glBindTexture( GL_TEXTURE_2D, texnum );
	
	//	get blending setup
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	
	//	remove write to depth buffer
	glDepthMask(false);
	
	//
	//	loop
	//
	
	//glDisable( GL_DITHER ); Suggest by user, doesnot work, make particles green.
	
	while( p )
	{
		
		
		//	if the texnums don't match, end primitives, and bind a new texture, then restart primitives
		if( p->texnum != texnum )
		{
			texnum	= p->texnum;
			glBindTexture( GL_TEXTURE_2D, texnum );
		}
		
		if (p->glow){
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		}
		else
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		//
		//	ensure that we draw only those particles on the 'right' side of the water as det. by waterpart
		//
		
		if(!waterpart && p->contents != CONTENTS_EMPTY)
		{
			p = p->next;
			continue;
		}
		
		if(waterpart && p->contents == CONTENTS_EMPTY)
		{
			p = p->next;
			continue;
		}
		
		
		if ( gl_particle_limiter.value )
		{
			VectorSubtract(p->origin, r_origin, v);
			dist= Length(v) / p->scale;

			//Limit big near particles
			if (dist < gl_particle_limiter.value)
			{
				p = p->next;
				continue;
				//Con_Printf("%f - dist\n", dist);
			}

			//Limit far particles 
			if (dist > 600) {
				p = p ->next;
				continue;
			}
		}
		
		


		alpha	= p->alpha;
		
		glColor4ub(p->colorred, p->colorgreen, p->colorblue, alpha);
		
		switch (  p->hazetype )
		{
		case HZ_NOHAZE:
			glPushMatrix();
			{	
				//
				//	translate and scale it
				//
				glTranslatef( p->origin[0], p->origin[1], p->origin[2] );
						
				glScalef( p->scale * p->scalex , p->scale * p->scaley, p->scale * p->scalez);//Tei
				
				glBegin( GL_QUADS );
				{
					//
					//	pass the vertex data to OpenGL
					//
					
					glTexCoord2f(	0,		1	);
					glVertex3fv(	coord[0]	);
					
					glTexCoord2f(	0,		0	);
					glVertex3fv(	coord[1]	);
					
					glTexCoord2f(	1,		0	); 
					glVertex3fv(	coord[2]	);
					
					glTexCoord2f(	1,		1	);
					glVertex3fv(	coord[3]	);
					
				}
				glEnd();			
			}
			glPopMatrix();
			break;
		case HZ_LIGHTSTORM:
		case HZ_LIGHTCAOS:
		case HZ_NORMAL:
			Draw_HazeParticleOndraw(p);
			break;
		default:
			Con_Printf("Unknom haze particle\n");
			break;
		}
		
		p = p->next;
	}

	
#if 0	
	if (gl_smallparticles.value && lastnearest) {
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);		
	}
#endif
	
	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glDepthMask(true);
	glColor4f(1,1,1,1);
	
	if (gl_fogenable.value)//Tei fix for square particles
	{
		glEnable(GL_FOG);
	}
	
	
	//glEnable( GL_DITHER ); Suggest by user
}






void R_FireExplo (vec3_t  origin, float scale)
{
	particle_t	*p;
	int i;//,k;
	int ext = 7 * scale;
	int pos = 32 * scale;
	
	for (i =0; i < 6; i++)
	{
		NEWPARTICLE();
		
		//ext = ext + rand()&1;
		pos = pos + rand()&63  ;
		
		p->die			= cl.time + 20;
		
		p->fxcolor = FXC_AUTOGRAY2;
		//p->volumetric = true;
		
		if (rand()&1) {
			// fire1
			COLOR(227,151,79);
		}
		else{
			if (rand()&1) {
				if (rand()&1) {
					// gas
					COLOR(79,151,227);
					
				}
				else {
					// smoke
					COLOR(-79,-151,-227);
				}
			} else {
				// fire2
				COLOR(-79,-151,-227);
				
			}
			
		}
		
		p->alpha		= 128;
		
		if (rand()&1){
			
			p->scale		= 10 * scale;
			if ( rand()&1 ) 
				p->scale		= 15 * scale;
		}
		else
			p->scale		= 7 * scale;
		
		if (p->scale < 255)
			p->scale = p->scale + 5;
		
	   p->texnum = GetTex(GEX_FIRE);//firecycle_tex[k];
		



			p->bounce		= 1;
			
			if (rand()&1)
				p->type			= pt_fire;
			else
				p->type			= pt_fire2;
			
			p->glow			= true;
			
			p->velocity[0]	= (rand() & (ext *2+1)) - ext;
			p->velocity[1]	= (rand() & (ext *2+1)) - ext;
			p->velocity[2]	= 100 - ((rand() & 8) );
			
			
			p->origin[0]	= origin[0] +(rand() & (pos*2+1)) - pos; 
			p->origin[1]	= origin[1] +(rand() & (pos*2+1)) - pos;
			p->origin[2]	= origin[2] ;
	}
}

void R_FireExploLava (vec3_t  origin, float scale)
{
	particle_t	*p;
	int ext = 7 * scale;
	int pos = 32 * scale;
	
	NEWPARTICLE();
	
	pos = pos + rand()&63  ;
	
	p->die			= cl.time + 20;
	
	if (rand()&1) {
		// fire1
		COLOR(227,151,79);
	}
	else{
		if (rand()&1) {
			if (rand()&1) {
				// gas
				COLOR(70,151,227);
			}
			else {
				// smoke
				COLOR(-70,-151,-227);
			}
		} else {
			// fire2
			COLOR(-70,-151,-227);
		}
		
	}
	
	p->alpha		= rand()&63;
	
	if (rand()&1){
		
		p->scale		= 10 * scale;
		if ( rand()&1 ) 
			p->scale		= 15 * scale;
	}
	else
		p->scale		= 7 * scale;
	
	if (p->scale < 255)
		p->scale = p->scale + 5;
	
   p->texnum = GetTex(GEX_LAVA);//firecycle_tex[k];

		
		if (rand()&1)
			p->type			= pt_fire;
		else
			p->type			= pt_fire2;
	
	
		if ( rand()&7 == 7 )
		{
			p->bounce		= 1;
			p->texnum = bubble_tex;
			p->scale = 3;
			p->alpha = 3;
			p->type =	pt_bubble;
		}
		else
			p->bounce		=  0;
		
	
		p->glow			= true;
		
		p->velocity[0]	= (rand() & (ext *2+1)) - ext;
		p->velocity[1]	= (rand() & (ext *2+1)) - ext;
		p->velocity[2]	= 100 - ((rand() & 8) );
		
		
		p->origin[0]	= origin[0] +(rand() & (pos*2+1)) - pos; 
		p->origin[1]	= origin[1] +(rand() & (pos*2+1)) - pos;
		p->origin[2]	= origin[2] ;
}


void R_FireExploBlack (vec3_t  origin, float scale)
{
	particle_t	*p;
	int ext = 2 * scale;
	int pos = 7 * scale;
	
	NEWPARTICLE();
	
	//ext = ext + rand()&1;
	pos = pos + rand()&63  ;
	
	p->die			= cl.time + 20;
	
	COLOR( 0,0,0);
	
	p->alpha		= rand()&127;
	
	
	if (rand()&1){
		
		p->scale		= 10 * scale;
		if ( rand()&1 ) 
			p->scale		= 15 * scale;
	}
	else
		p->scale		= 7 * scale;
	
	if (p->scale < 255)
		p->scale = p->scale + 5;
	
	p->texnum		= smoke1_tex + (rand()&3);//snow_tex;//particle_tex;rain_tex;
	
	p->bounce		= 0;
	
	if (rand()&1)
		p->type			= pt_snowfade;
	//else
	//	p->type			= pt_slowmoke;
	
	p->glow			= false;
	
	p->velocity[0]	= (rand() & (ext *2+1)) - ext;
	p->velocity[1]	= (rand() & (ext *2+1)) - ext;
	p->velocity[2]	= 30 - ((rand() & 8) );
	
	
	p->origin[0]	= origin[0] +(rand() & (pos*2+1)) - pos; 
	p->origin[1]	= origin[1] +(rand() & (pos*2+1)) - pos;
	p->origin[2]	= origin[2] + pos * 2 + (rand()& pos);
}


void R_FireFlama (vec3_t  origin, float scale)
{
	particle_t	*p;
	int i;
	int ext = 7 * scale;
	int pos = 31 * scale;
	
	for (i =0; i < 6; i++)
	{
		NEWPARTICLE();

		ext = ext + rand()&1;
		pos = pos + rand()&1;
		
		p->die			= cl.time + 20;
		
		if (rand()&1) {
			// fire1
			COLOR( 227, 151, 79);
		}
		else{
			if (rand()&1) {
				if (rand()&1) {
					// gas
					COLOR( 79,151, 227);
				}
				else {
					// smoke
					COLOR( -79,-151,-227 );
				}
			} else {
				// fire2
				COLOR( -79,-151,-227 );
			}
			
		}
		
		//p->alpha		= 128;
		
		p->alpha		= rand()&127;
		
		if (rand()&1){
			
			p->scale		= 20 * scale;
			if ( rand()&1 ) 
				p->scale		= 30 * scale;
		}
		else
			p->scale		= 15 * scale;
		
		if (rand()&1)
			if (rand()&1)
				p->texnum		= fire_tex;//snow_tex;//particle_tex;rain_tex;
			else
				p->texnum		= smoke2_tex;//snow_tex;//particle_tex;rain_tex;
			else 
				p->texnum		= smoke1_tex;//snow_tex;//particle_tex;rain_tex;
			
			if (rand()&1)
				p->texnum		= flama_tex;//snow_tex;//particle_tex;rain_tex;
			
			
			p->bounce		= 1;
			
			if (rand()&1)
				p->type			= pt_fire;
			else
				p->type			= pt_fire2;
			
			p->glow			= true;
			
			p->velocity[0]	= (rand() & (ext *2+1)) - ext;
			p->velocity[1]	= (rand() & (ext *2+1)) - ext;
			p->velocity[2]	= 100 - ((rand() & 8) );
			
			
			p->origin[0]	= origin[0] +(rand() & (pos*2+1)) - pos; 
			p->origin[1]	= origin[1] +(rand() & (pos*2+1)) - pos;
			p->origin[2]	= origin[2] ;
	}
}


void R_ExploRing (vec3_t  origin, float scale)
{
	particle_t	*p;
	int i;
	int ext = 127 * scale;
	int pos = 32 * scale;
	
	
	for (i =0; i < 6; i++)
	{
		NEWPARTICLE();

		ext = ext + rand()&1;
		pos = pos + rand()&1;
		
		p->die			= cl.time + 20 ;
		
		if (rand()&1) {
			COLOR( 227, 151, 79 );
		}
		
		p->alpha		= 60;
		p->scale		= 2 * scale;
		p->texnum		= circle_tex;
		p->bounce		= 0;
		p->type			= pt_expand;
		p->glow			= true;
		
		p->velocity[0]	= ((rand() & (ext *2+1)) - ext) * 50;
		p->velocity[1]	= ((rand() & (ext *2+1)) - ext) * 50;
		p->velocity[2]	= 0;
		
		
		VectorCopy(origin,p->origin);
	}
}

void R_ZRing (vec3_t  origin, float scale)
{
	particle_t	*p;
	int ext = 127 * scale;
	int pos = 32 * scale;
	
	
	NEWPARTICLE();

	ext = ext + rand()&1;
	pos = pos + rand()&1;
	
	p->die			= cl.time + 20 ;
	
	if (rand()&1) {
		COLOR( 227,251, 252);
	}
	
	p->alpha		= 60;
	p->scale		= scale;
	p->texnum		= circle_tex;
	p->bounce		= 0;
	p->type			= pt_expand;
	p->glow			= true;
	
	p->velocity[0]	= 0;
	p->velocity[1]	= 0;
	p->velocity[2]	= 0;
	

	VectorCopy(origin,p->origin);
}


void R_ParticleSnow (vec3_t origin)
{
	particle_t	*p;
	
	NEWPARTICLE();
	
	p->scale			= 2 +rand()&3 ;
	p->alpha			= 80;
	p->texnum			=  GetTex(GEX_SNOW);//snow_tex;
	p->bounce			= 0;
	p->type				= pt_snow;
	
	COLOR( 255, 255, 255);
	
	p->die				= cl.time + 15;
	p->glow				= true;
	
	p->origin[0]		= origin[0] + (rand() &127) - 63;
	p->origin[1]		= origin[1] + (rand() &127) - 63;
	p->origin[2]		= origin[2] + (rand() &127) - 63;
	
	p->velocity[2]		= -( 20 + rand()&63);
}


void R_ZerstorerRain (vec3_t org);

void R_ParticleRain (vec3_t origin)
{
	particle_t	*p;
	

	NEWPARTICLE();

	
	p->scale			= 1 +rand()&1 ;
	p->scalez			= 1 +rand()&63;
	
	p->alpha			= 70;
	p->texnum			= particle_tex;
	p->bounce			= 0;
	p->type				= pt_rain;
	
	COLOR( 255, 255, 255);
	
	p->die				= cl.time + 2;
	p->glow				= true;
	
	p->origin[0]		= origin[0] + (rand() &127) - 63;
	p->origin[1]		= origin[1] + (rand() &127) - 63;
	p->origin[2]		= origin[2] + (rand() &127) - 63;
	
	p->velocity[2]		= -( 1920 + rand()&511 );
}


void R_ParticleDream (vec3_t origin)
{
	particle_t	*p;
	
	NEWPARTICLE();

	
	p->scale			= 12+ rand()&31;
	
	p->alpha			= 200 + rand()&31;
	
	p->texnum			= smoke1_tex + (rand() & 3);
	
	p->bounce			= 1;
	p->type				= pt_fire;
	
	COLOR( 255,100,110 );
	
	p->die				= cl.time + 8;
	p->glow				= false;
	
	p->origin[0]		= origin[0] + (rand() &127) - 63;
	p->origin[1]		= origin[1] + (rand() &127) - 63;
	p->origin[2]		= origin[2] + (rand() &127) - 63;
	
	p->velocity[2]		= 0;
}




void R_FireStaticFog (vec3_t  origin)
{
	particle_t	*p;
	int i;
	int pos = 32 ;
	
	for (i =0; i < 6; i++)
	{

		NEWPARTICLE();
		
		p->die			= cl.time + 20;
		
		p->scale  = rand()&7;
		p->scalex = rand()&127;
		p->scaley = rand()&127;
		
		COLOR( 255,255,255);
		
		p->alpha		= rand()&31;
		p->bounce		= 0;
		p->texnum		= smoke1_tex + (rand() & 3);
		
		p->type			= pt_static;
		p->glow			= true;
		
		p->origin[0]	= origin[0] +(rand() & (pos*2-1)) - pos; 
		p->origin[1]	= origin[1] +(rand() & (pos*2-1)) - pos;
		p->origin[2]	= origin[2] ;
		
		VectorCopy(vec3_origin,p->velocity);

		
	}
}


void R_ParticleBlast (vec3_t origin, vec3_t dir, int num,vec3_t color, byte scale)
{
	int			i;
	particle_t	*p;
	
	for( i=0; i<num;i++)
	{
		p	= addParticle();
		if(!p)
		{
			return;
		}
		
		p->scale			= 1 ;
		p->scalex			= 1+ rand()&31;
		p->scaley			= 1+ rand()&31;
		p->scalez			= 1+ rand()&31;
		
		p->alpha			= 80;
		p->texnum			= particle_tex;
		p->bounce			= 0;
		p->type				= pt_static;
		
		COLOR( color[0], color[1], color[2]);
		
		p->die				= cl.time + 15;
		p->glow				= true;
		
		if( scale)
		{
			p->origin[0]		= origin[0] + ((rand()&(scale*2-1)) - scale) ;
			p->origin[1]		= origin[1] + ((rand()&(scale*2-1)) - scale);
			p->origin[2]		= origin[2] + ((rand()&(scale*2-1)) - scale);
		}
		else
		{
			VectorCopy(origin,p->origin);
		}

		VectorCopy(dir,p->velocity);
	}
}

void R_ParticleBlastTimed (vec3_t origin, vec3_t dir, int num,vec3_t color, byte scale, int time)
{
	int i;
	particle_t	*p;
	
	for( i=0; i<num;i++)
	{
		p	= addParticle();
		if(!p)
		{
			return;
		}
		
		p->scale			= 1 ;
		p->scalex			= 1+ rand()&31;
		p->scaley			= 1+ rand()&31;
		p->scalez			= 1+ rand()&31;
		
		p->alpha			= 80;
		p->texnum			= particle_tex;
		p->bounce			= 0;
		p->type				= pt_static;
		
		COLOR( color[0], color[1], color[2]);
		
		p->die				= cl.time + time;
		p->glow				= true;
		
		if( scale)
		{
			p->origin[0]		= origin[0] + ((rand()&(scale*2-1)) - scale) ;
			p->origin[1]		= origin[1] + ((rand()&(scale*2-1)) - scale);
			p->origin[2]		= origin[2] + ((rand()&(scale*2-1)) - scale);
		}
		else
		{
			VectorCopy(origin,p->origin);
		}
		
		VectorCopy(dir,p->velocity);
	}
}


void R_ParticleSmoke (vec3_t origin, int alfa)
{
	particle_t	*p;
	
	p	= addParticle();
	if(!p)
	{
		return;
	}
	
	p->die			= cl.time + 5;
	
	p->colorred		= lhrandom(50,159);//209;
	p->colorgreen	= lhrandom(10,91);//201;
	p->colorblue	= lhrandom(25,50);
	
	
	
	
	p->alpha		= alfa;
	p->scale		= 5;
	p->texnum		= smoke1_tex + (rand() & 3);//particle_tex;
	p->bounce		= 0;
	p->type			= pt_expand;//pt_fire;pt_slowmoke;
	
	p->glow			= true;
		
	
	if (rand()&1)
		p->fxcolor		= FXC_AUTOALFA;
	
	p->velocity[0]	= (rand() & 3) - 2;
	p->velocity[1]	= (rand() & 3) - 2;
	p->velocity[2]	= (rand() & 3) ;
	
	VectorCopy(origin,p->origin);
}



void R_ParticleStelar (vec3_t origin, int alfa)
{
	particle_t	*p;
	
	NEWPARTICLE();
			
	p->die			= cl.time + 5;
	
	p->colorred		= lhrandom(50,159);//209;
	p->colorgreen	= lhrandom(10,91);//201;
	p->colorblue	= lhrandom(25,50);
	
	
	
	
	p->alpha		= alfa;
	p->scale		= 5;
	p->texnum		= GetTex(GEX_FIRE);//smoke1_tex + (rand() & 3);//particle_tex;
	p->bounce		= 0;
	p->type			= pt_expand;//pt_fire;pt_slowmoke;
	
	p->glow			= true;
	
	//p->colorred		= 247;
	//p->colorgreen	= 191;
	//p->colorblue	= 79;
	
	//p->glow			= false;
	
	
	if (rand()&1)
		p->fxcolor		= FXC_AUTOALFA;
	
	p->velocity[0]	= (rand() & 3) - 2;
	p->velocity[1]	= (rand() & 3) - 2;
	p->velocity[2]	= (rand() & 3) ;
	
	VectorCopy(origin,p->origin);
}

void R_Blood (vec3_t origin, int alfa)
{
	particle_t	*p;
	
	NEWPARTICLE();		
			
	p->die			= cl.time + 5;//215?
	
	p->colorred		= lhrandom(50,255);//209;
	p->colorgreen	= lhrandom(10,21);//201;
	p->colorblue	= lhrandom(10,20);
	
	p->alpha		= alfa;
	p->scale		= lhrandom(1,4);
	p->scalex		= 1;
	p->scaley		= 1;
	
	p->texnum		= smoke1_tex + (rand() & 3);//particle_tex;
	p->bounce		= 1;
	p->type			= pt_grav;//pt_fire;pt_slowmoke;
	p->glow			= false;
	
	p->velocity[0]	= (rand() & 3) - 2;
	p->velocity[1]	= (rand() & 3) - 2;
	p->velocity[2]	= lhrandom(-3,3);
	
	VectorCopy(origin,p->origin);
}



#if 0
void R_BigBlood (vec3_t origin, int alfa)
{
	particle_t	*p;
	
	NEWPARTICLE();		
			
	p->die			= cl.time + 5;//215?
	
	p->colorred		= lhrandom(50,255);//209;
	p->colorgreen	= lhrandom(10,21);//201;
	p->colorblue	= lhrandom(10,20);
	
	p->alpha		= alfa;
	p->scale		= lhrandom(2,3);
	p->scalex		= lhrandom(2,10)+5;
	p->scaley		= lhrandom(2,10)+5;
	p->scalez		= teirand(2)+1;
	
	p->texnum		= smoke1_tex + (rand() & 3);//particle_tex;
	p->bounce		= 1;
	p->type			= pt_grav;//pt_fire;pt_slowmoke;
	p->glow			= true;
	
	p->velocity[0]	= (rand() & 3) - 2;
	p->velocity[1]	= (rand() & 3) - 2;
	p->velocity[2]	= lhrandom(-3,3);
	
	VectorCopy(origin,p->origin);
}
#endif

void R_ParticleFire (vec3_t origin, int dx, int scale)
{
	particle_t	*p;
	
	NEWPARTICLE();
			
	p->die			= cl.time + 5;
	
	COLOR( 227,151, 79 );
	
	p->alpha		= 128;
	p->scale		= scale;
	p->texnum		= particle_tex;
	p->bounce		= 0;
	p->type			= pt_fire;
	p->glow			= true;
	
	p->velocity[0]	= lhrandom(-dx,dx);
	p->velocity[1]	= lhrandom(-dx,dx);
	p->velocity[2]	= lhrandom(-dx,dx);
	
	p->origin[0]	= origin[0];
	p->origin[1]	= origin[1];
	p->origin[2]	= origin[2] + 4;
}


//void R_AlienBlood (vec3_t origin);

void R_AlienBloodExplosion (vec3_t origin)
{
	particle_t	*p;
	int dx = 5;
	
	NEWPARTICLE();		
			
	p->die			= cl.time + 5;
	
	COLOR( 7, 251, 9 );
	
	p->alpha		= 128;
	p->scale		= lhrandom(1,4);
	p->texnum		= particle_tex;
	p->bounce		= 0;
	p->type			= pt_fire;
	p->glow			= true;
	
	p->fxcolor = FXC_ALIENBLOOD; 
	
	p->velocity[0]	= lhrandom(-dx,dx);
	p->velocity[1]	= lhrandom(-dx,dx);
	p->velocity[2]	= lhrandom(-dx,0);
	
	VectorCopy(origin,p->origin);
}


void R_AlienBoltExplosion (vec3_t origin)
{
	particle_t	*p;
	int dx = 5;
	
	NEWPARTICLE()		
			
	p->die			= cl.time + 1;
	
	COLOR( 90, 90 ,250);
	
	p->alpha		= 10;
	p->scale		= lhrandom(1,4);
	p->texnum		= particle_tex;
	p->bounce		= 0;
	p->type			= pt_expandfade;
	p->glow			= true;
	
	p->fxcolor = FXC_ALIENBLOOD2; 
	
	p->velocity[0]	= lhrandom(-dx,dx);
	p->velocity[1]	= lhrandom(-dx,dx);
	p->velocity[2]	= lhrandom(-dx,dx);
	
	VectorCopy(origin,p->origin);
}


void R_AlienBlood (vec3_t origin)
{
	particle_t	*p;
	int dx = 5;
	
	NEWPARTICLE();
			
	p->die			= cl.time + 5;
	
	COLOR( 7, 251, 9);
	
	p->alpha		= 128;
	p->scale		= lhrandom(1,4);
	p->texnum		= particle_tex;
	p->bounce		= 0;
	p->type			= pt_fire;
	p->glow			= true;
	
	p->velocity[0]	= lhrandom(-dx,dx);
	p->velocity[1]	= lhrandom(-dx,dx);
	p->velocity[2]	= lhrandom(-dx,0);
	
	VectorCopy(origin,p->origin);
}

void R_AlienBlood2 (vec3_t origin)
{
	particle_t	*p;
	int dx = 5;
	
	NEWPARTICLE();
			
	p->die			= cl.time + 5;
	
	COLOR( 90,90,250);
	
	p->alpha		= 128;
	p->scale		= lhrandom(1,4);
	p->texnum		= particle_tex;
	p->bounce		= 0;
	p->type			= pt_fire2;
	p->glow			= true;
	
	p->velocity[0]	= lhrandom(-dx,dx);
	p->velocity[1]	= lhrandom(-dx,dx);
	p->velocity[2]	= lhrandom(-dx,dx);
	
	VectorCopy(origin,p->origin);
}


//DeeTee suggested, but uglyfied by tei
/*
===============
R_LightParticles

  ===============
*/

#if 1
void R_LightParticles (vec3_t end)
{
	int                     i;
	byte*					color;
	particle_t      *p;
	
	
	for (i=0 ; i<256 ; i++)
	{
		NEWPARTICLE();
		
		p->alpha                        = 200;
		p->die                          = cl.time + 1;
		p->type                         = pt_fire;//static;
		
		
		p->scale                        = 1;
		
		color		= (byte *) &d_8to24table[206+rand()%6];
		
		COLOR( color[0],color[1],color[2] );
		
		p->fxcolor						= FXC_DeeTeeLAZER;	
		
		p->texnum                       = particle_tex;//flareparticletexture;
		
		VectorCopy(end,p->origin);
		
		p->velocity[0]                       = (rand() & 128) - 64;
		p->velocity[1]                       = (rand() & 128) - 64;
		p->velocity[2]                       = (rand() & 128) - 64;
	}
}
#else 

// Original version

void R_LightParticles (vec3_t end)
{
	int                     i;
	byte*					color;
	particle_t      *p;
	
	for (i=0 ; i<256 ; i++)
	{
		NEWPARTICLE();
		
		p->alpha                        = 255;
		p->die                          = cl.time + 5;
		p->type                         = pt_fire;//static;
		
		
		p->scale                        = 20;
		
		color		= (byte *) &d_8to24table[206+rand()%6];
		
		COLOR( color[0],color[1],color[2] );
		
		p->fxcolor						= FXC_DeeTeeLAZER;	
		
		p->texnum                       = flare_tex;//particle_tex;//flareparticletexture;
		
		VectorCopy(end,p->origin);

		p->velocity[0]                       = (rand() & 128) - 64;
		p->velocity[1]                       = (rand() & 128) - 64;
		p->velocity[2]                       = (rand() & 128) - 64;
	}
}

#endif
//DeeTee suggested, but uglyfied by tei


void R_ParticlePuff (vec3_t origin, int alfa)
{
	particle_t	*p;
	
	NEWPARTICLE();
			
	p->die			= cl.time + 5;
	
	COLOR( 227,251, 251 );
	
	p->alpha		= alfa;
	p->scale		= 3;
	p->texnum		= particle_tex;
	p->bounce		= 0;
	p->type			= pt_smoke;
	p->glow			= true;
	
	p->velocity[0]	= lhrandom(-1,1);
	p->velocity[1]	= lhrandom(-1,1);
	p->velocity[2]	= lhrandom(-1,1);
	
	p->origin[0]	= origin[0];
	p->origin[1]	= origin[1];
	p->origin[2]	= origin[2];
}



void R_ParticleBubble (vec3_t origin)
{
	particle_t	*p;
	int		contents	= Mod_PointInLeaf(origin, cl.worldmodel)->contents;

	if (contents != CONTENTS_WATER)
		return;
	
	NEWPARTICLE();
			
	p->die			= cl.time + 5;
	
	COLOR( 227,251, 251 );
	
	p->alpha		= lhrandom(100,200);
	p->scale		= lhrandom(1,3);
	p->texnum		= bubble_tex;
	p->bounce		= 0;
	p->type			= pt_bubble;
	p->glow			= true;
	
	p->velocity[0]	= lhrandom(-1,1);
	p->velocity[1]	= lhrandom(-1,1);
	p->velocity[2]	= lhrandom(0,4);
	
	p->origin[0]	= origin[0]+ lhrandom(-2,2);
	p->origin[1]	= origin[1]+ lhrandom(-2,2);
	p->origin[2]	= origin[2]+ lhrandom(0,2);
}

void R_ParticleBubbleGib (vec3_t origin)
{
	particle_t	*p;
	int		contents	= Mod_PointInLeaf(origin, cl.worldmodel)->contents;

	if (contents != CONTENTS_WATER)
		return;
	
	NEWPARTICLE();
			
	p->die			= cl.time + 5;
	
	COLOR( 227,251, 251 );
	
	p->alpha		= lhrandom(100,200);
	p->scale		= lhrandom(1,3);
	p->texnum		= blood_tex;
	p->bounce		= 0;
	p->type			= pt_bubble;
	p->glow			= true;
	
	p->velocity[0]	= lhrandom(-1,1);
	p->velocity[1]	= lhrandom(-1,1);
	p->velocity[2]	= lhrandom(0,4);
	
	p->origin[0]	= origin[0]+ lhrandom(-2,2);
	p->origin[1]	= origin[1]+ lhrandom(-2,2);
	p->origin[2]	= origin[2]+ lhrandom(0,2);
}



//How STUPID 2 copys of the same code with different color?
void R_ParticleExplosionAlien (vec3_t origin) 
{
	int			i, contents;
	particle_t	*p;
	//	byte		*color;
	contents			= Mod_PointInLeaf(origin, cl.worldmodel)->contents;
	
	// TRAILS
	for (i=0 ; i<14 ; i++)
	{
		NEWPARTICLE();
		
		p->scale			= 1;
		p->alpha			= 200+(rand()&32);
		p->texnum			= smoke1_tex + (rand() & 3);
		p->bounce			= 1;
		p->type				= pt_explode;
		
		p->velocity[0]		= lhrandom(-105,105);
		p->velocity[1]		= lhrandom(-105,105);
		p->velocity[2]		= lhrandom(-105,195);
		
		//color				= (byte *) &d_8to24table[(int)(rand() & 7) + 105];
		COLOR( 0, 0, 0);
		
		p->fxcolor =		FXC_ALIENBLOOD2;
		p->die				= cl.time + 16;
		p->glow				= false;//true;
		
		p->origin[0]		= origin[0] + lhrandom(-6,6);
		p->origin[1]		= origin[1] + lhrandom(-6,6);
		p->origin[2]		= origin[2] + lhrandom(-6,6);
	}
	
	//BROWN SMOKE (actually GREEN)
	for (i=0 ; i<8 ; i++)
	{
		p	= addParticle();
		if(!p)
		{
			return;
		}
		
		
		p->scale			= 23 + (rand()&31);
		p->alpha			= 210+(rand()&32);
		p->texnum			= smoke1_tex + (rand() & 3);
		p->bounce			= 1;
		p->type				= pt_slowmoke;//pt_snow;//pt_snow;//pt_smokeexp;
		
		p->velocity[0]		= (rand() & 7) - 4;
		p->velocity[1]		= (rand() & 7) - 4;
		p->velocity[2]		= (rand() & 15) - 4;
		
		//color				= (byte *) &d_8to24table[(int)(rand() & 7) + 105];
		//COLOR( 90,90,250);...lovable BLUE
		COLOR( 90,250,90);//lovable GREEN
		
		p->die				= cl.time + 16;
		p->glow				= false;//true;
		
		p->origin[0]		= origin[0] + (rand() &31) - 16;
		p->origin[1]		= origin[1] + (rand() &31) - 16;
		p->origin[2]		= origin[2] + (rand() &31) - 8;
	}
	
}



void R_ParticleExplosionSmoke (vec3_t origin) 
{
	int			i;//, contents;
	particle_t	*p;

	//BROWN SMOKE (actually GREEN)
	for (i=0 ; i<8 ; i++)
	{
		p	= addParticle();
		if(!p)
		{
			return;
		}
		
		
		p->scale			= lhrandom(2,6);
		p->alpha			= 10+(rand()&32);

		p->texnum			= GetTex(GEX_SMOKE);//smoke1_tex + (rand() & 3);
		p->bounce			= 1;
		p->type				= pt_slowmoke;//pt_snow;//pt_snow;//pt_smokeexp;
		
		p->velocity[0]		= (rand() & 7) - 4;
		p->velocity[1]		= (rand() & 7) - 4;
		p->velocity[2]		= (rand() & 15) - 4;
		
		COLOR( 120,150,130);//strange gray
		
		p->die				= cl.time + 16;
		p->glow				= false;//true;
		/*
		p->scalez			= 0.8;
		p->scaley			= 2;
		p->scalex			= 2;
		*/


		
		p->origin[0]		= origin[0] + lhrandom(0,4);
		p->origin[1]		= origin[1] + lhrandom(0,4);
		p->origin[2]		= origin[2] + lhrandom(0,4);
	}
	
}

//Q2MAX! Interesting but REALLY UGLY!

#if 0
void Draw_HazeBeam(float size, particle_t *p)
{
	float	tlen, halflen, len, dec = size, warpfactor, warpadd, oldwarpadd, warpsize, time, i=0,j, factor, alpha;
	vec3_t	move, lastvec, thisvec, tempvec;
	vec3_t	angl_coord[4];
	vec3_t	ang_up, ang_right, vdelta;
	int texnum;
	
	texnum		= p->texnum;
	glBindTexture( GL_TEXTURE_2D, texnum );
	
	//	get blending setup
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	
	//	remove write to depth buffer
	glDepthMask(false);
	
	
	warpsize = dec * 0.1f;
	warpfactor = M_PI*1.0f;
	//time = -r_refdef.time*10.0;
	time = realtime;// - e->translate_start_time; 
	
	VectorSubtract(p->origin, p->gorigin, move);
	//VectorSubtract(p->origin, p->angle, move);
	len = VectorNormalize(move);
	
	VectorCopy(move, ang_up);
	VectorSubtract(r_refdef.vieworg, p->gorigin, vdelta);
	CrossProduct(ang_up, vdelta, ang_right);
	if(!VectorCompare(ang_right, vec3_origin))
		VectorNormalize(ang_right);
	
	VectorScale (ang_right, dec, ang_right);
	VectorScale (ang_up, dec, ang_up);
	
	VectorScale(move, dec, move);
	VectorCopy(p->gorigin, lastvec);
	VectorAdd(lastvec, move, thisvec);
	VectorCopy(thisvec, tempvec);
	
	warpadd = 0.25*sin(time+warpfactor);
	factor = 1;
	halflen = len/2;
	
	thisvec[0]= (thisvec[0]*2 + teirand(5))/2;
	thisvec[1]= (thisvec[1]*2 + teirand(5))/2;
	thisvec[2]= (thisvec[2]*2 + teirand(5))/2;
	alpha	= p->alpha;
	
	glColor4ub(p->colorred, p->colorgreen, p->colorblue, alpha);
	
	while (len>dec)
	{	i+=warpsize;
	
				VectorAdd(thisvec, ang_right, angl_coord[0]);
				VectorAdd(lastvec, ang_right, angl_coord[1]);
				VectorSubtract(lastvec, ang_right, angl_coord[2]);
				VectorSubtract(thisvec, ang_right, angl_coord[3]);
				
				glPushMatrix();
				{
					glBegin (GL_QUADS);
					{
						glTexCoord2f (0+warpadd, 1);
						glVertex3fv  (angl_coord[0]);
						glTexCoord2f (0+oldwarpadd, 0);
						glVertex3fv  (angl_coord[1]);
						glTexCoord2f (1+oldwarpadd, 0); 
						glVertex3fv  (angl_coord[2]);
						glTexCoord2f (1+warpadd, 1);
						glVertex3fv  (angl_coord[3]);
					}
					glEnd ();
					
				}
				glPopMatrix ();
				
				tlen = (len<halflen)? fabs(len-halflen): fabs(len-halflen);
				factor = tlen/(size*size);
				
				if (factor > 1+size/3.0)
					factor = 1+size/3.0;
				else if (factor < 1)
					factor = 1;
				
				oldwarpadd = warpadd;
				warpadd = 0.25*sin(time+i+warpfactor);
				len-=dec;
				VectorCopy(thisvec, lastvec);
				VectorAdd(tempvec, move, tempvec);
				VectorAdd(lastvec, move, thisvec);
				thisvec[0]= ((thisvec[0] + teirand(10)) + tempvec[0]*factor)/(factor+1);
				thisvec[1]= ((thisvec[1] + teirand(10)) + tempvec[1]*factor)/(factor+1);
				thisvec[2]= ((thisvec[2] + teirand(10)) + tempvec[2]*factor)/(factor+1);
	}
	
	i+=warpsize;
	
	//one more time
	if (len>0)
	{
		VectorAdd(p->origin, ang_right, angl_coord[0]);
		VectorAdd(lastvec, ang_right, angl_coord[1]);
		VectorSubtract(lastvec, ang_right, angl_coord[2]);
		VectorSubtract(p->origin, ang_right, angl_coord[3]);
		
		glPushMatrix();
		{
			glBegin (GL_QUADS);
			{
				glTexCoord2f (0+warpadd, 1);
				glVertex3fv  (angl_coord[0]);
				glTexCoord2f (0+oldwarpadd, 0);
				glVertex3fv  (angl_coord[1]);
				glTexCoord2f (1+oldwarpadd, 0); 
				glVertex3fv  (angl_coord[2]);
				glTexCoord2f (1+warpadd, 1);
				glVertex3fv  (angl_coord[3]);
			}
			glEnd ();
			
		}
		glPopMatrix ();
	}
	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glDepthMask(true);
	glColor4f(1,1,1,1);
	
}
#endif
//Q2MAX!

//Q2MAX!
// Whe suspect this is now not-called
#if 0
void Draw_HazeParticle2(float size, particle_t *p)
{
	float	tlen, halflen, len, dec = size, warpadd, oldwarpadd, warpsize, time, i=0,j, factor;//, alpha;
	vec3_t	move, lastvec, thisvec, tempvec;
	vec3_t	angl_coord[4];
	vec3_t	ang_up, ang_right, vdelta;
	int texnum;
	
	texnum		= p->texnum;
	glBindTexture( GL_TEXTURE_2D, texnum );
	
	//	get blending setup
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	
	//	remove write to depth buffer
	glDepthMask(false);
	
	
	if (p->hazetype == HZ_LIGHTSTORM && teirand(100)<4)
		DefineFlare(p->origin, teirand(200),0,teirand(100));		
	
	VectorSubtract(p->origin, p->gorigin, vdelta);
	VectorNormalize(vdelta);
	
	VectorCopy(vdelta, ang_up);
	VectorSubtract(r_refdef.vieworg, p->gorigin, vdelta);
	CrossProduct(ang_up, vdelta, ang_right);
	
	if(!VectorCompare(ang_right, vec3_origin))
		VectorNormalize(ang_right);
	
	VectorScale(ang_right, size, ang_right);
	
	VectorAdd(p->origin, ang_right, angl_coord[0]);
	VectorAdd(p->gorigin, ang_right, angl_coord[1]);
	VectorSubtract(p->gorigin, ang_right, angl_coord[2]);
	VectorSubtract(p->origin, ang_right, angl_coord[3]);
	
	glPushMatrix();
	{
		glBegin (GL_QUADS);
		{
			glTexCoord2f (0, 1);
			glVertex3fv  (angl_coord[0]);
			glTexCoord2f (0, 0);
			glVertex3fv  (angl_coord[1]);
			glTexCoord2f (1, 0); 
			glVertex3fv  (angl_coord[2]);
			glTexCoord2f (1, 1);
			glVertex3fv  (angl_coord[3]);
		}
		glEnd ();
		
	}
	glPopMatrix ();
	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glDepthMask(true);
	glColor4f(1,1,1,1);
	
}
#endif
//Q2MAX!


//Q2MAX! Interesting but REALLY UGLY!

// Version that run inside R_DrawParticles

void Draw_HazeParticleOndraw(particle_t *p)
{
	float	 dec, i, size;
	vec3_t	angl_coord[4];
	vec3_t	ang_up, ang_right, vdelta;
	
    size	= p->scale;
	dec		= size;
	i = 0;
	
	if (p->hazetype == HZ_LIGHTSTORM && teirand(100)<4)
		DefineFlare(p->origin, teirand(p->storm),0,teirand(100));		
	else
	if (p->hazetype == HZ_LIGHTCAOS)
	{
		//Con_Printf("heelo\n");
		if (teirand(100)<4)
			DefineFlare(p->origin, teirand(p->storm),0,teirand(100));		
	}

	
	VectorSubtract(p->origin, p->gorigin, vdelta);
	VectorNormalize(vdelta);
	
	VectorCopy(vdelta, ang_up);
	VectorSubtract(r_refdef.vieworg, p->gorigin, vdelta);
	CrossProduct(ang_up, vdelta, ang_right);
	
	if(!VectorCompare(ang_right, vec3_origin))
		VectorNormalize(ang_right);
	
	VectorScale(ang_right, size, ang_right);
	
	VectorAdd(p->origin, ang_right, angl_coord[0]);
	VectorAdd(p->gorigin, ang_right, angl_coord[1]);
	VectorSubtract(p->gorigin, ang_right, angl_coord[2]);
	VectorSubtract(p->origin, ang_right, angl_coord[3]);
	
	glPushMatrix();
	{
		glBegin (GL_QUADS);
		{
			glTexCoord2f (0, 1);
			glVertex3fv  (angl_coord[0]);
			glTexCoord2f (0, 0);
			glVertex3fv  (angl_coord[1]);
			glTexCoord2f (1, 0); 
			glVertex3fv  (angl_coord[2]);
			glTexCoord2f (1, 1);
			glVertex3fv  (angl_coord[3]);
		}
		glEnd ();
		
	}
	glPopMatrix ();
	
	
}
//Q2MAX!



void R_FireExploHaze (vec3_t  origin, float scale)
{
	particle_t	*p;
	int i;
	int ext = 7 * scale;
	int pos = 32 * scale;
	
	for (i =0; i < 6; i++)
	{
		NEWPARTICLE();		
		
		pos = pos + rand()&63  ;
		
		p->die			= cl.time + 20;
		
		VectorCopy(origin, p->gorigin);
		p->hazetype = HZ_LIGHTCAOS;
		
		p->storm = 100;

		if (rand()&1) {
			// fire1
			COLOR(227,151,79);
		}
		else{
			if (rand()&1) {
				if (rand()&1) {
					// gas
					COLOR(79,151,227);
					
				}
				else {
					// smoke
					COLOR(-79,-151,-227);
				}
			} else {
				// fire2
				COLOR(-79,-151,-227);
				
			}
			
		}
		
		p->alpha		= 128;
		
		p->scale = teirand(5)+0.1;
		
		
		
		if (rand()&1)
			if (rand()&1)
				p->texnum		= fire_tex;//snow_tex;//particle_tex;rain_tex;
			else
				p->texnum		= smoke2_tex;//snow_tex;//particle_tex;rain_tex;
			else 
				p->texnum		= smoke1_tex;//snow_tex;//particle_tex;rain_tex;
			           
            
       p->bounce       = 1;            
       p->type         = pt_static;           
       p->glow         = true;
            
       p->velocity[0]  = lhrandom(-64,64);
       p->velocity[1]  = lhrandom(-64,64);
       p->velocity[2]  = lhrandom(-64,64);
            
	   VectorCopy( p->velocity, p->gvelocity);
   
       p->origin[0]    = origin[0] + lhrandom(-132,132); 
       p->origin[1]    = origin[1] + lhrandom(-132,132);
       p->origin[2]    = origin[2] + lhrandom(-132,132);
	}
}




void R_BeamZing (vec3_t origin, vec3_t end)
{
	int			i;
	particle_t	*p;

	for (i=0 ; i<3 ; i++)
	{
		NEWPARTICLE();		
		
		p->texnum		= GetTex(GEX_ZING);//particle_tex;

		p->bounce		= 1;
			
		p->die				= cl.time + 0.01;

		p->scale			= lhrandom(4,6);
		p->alpha			= 255;
		
		if (rand()&1)
		{
		
			COLOR(79,151,227);
		}
		else
		{
			COLOR(255,255,255);
		}

		p->type			= pt_static;//expandfade;//static;
		p->glow			= true;//false;//true; //Tei black sparkles
		p->hazetype		= HZ_NORMAL;

		p->fxcolor		= FXC_ZING;

		p->origin[0]		= origin[0];
		p->origin[1]		= origin[1];
		p->origin[2]		= origin[2];

		p->gorigin[0]		= end[0];// + lhrandom(-5,-5) ;
		p->gorigin[1]		= end[1];// + lhrandom(-5,-5) ;
		p->gorigin[2]		= end[2];// + lhrandom(-5,-5) ;

		VectorCopy(vec3_origin, p->velocity);

	}
}



void R_BeamSpike (vec3_t origin, vec3_t end)
{
	int			i;
	particle_t	*p;

	for (i=0 ; i<3 ; i++)
	{
		NEWPARTICLE();		
		
		p->texnum		= builtin_particle_tex;

		p->bounce		= 1;
			
		p->die				= cl.time + 0.1;

		p->scale			= 1;
		p->alpha			= 255;
		
		COLOR(255,255,255);


		p->type			= pt_static;//expandfade;//static;
		p->glow			= true;//false;//true; //Tei black sparkles
		p->hazetype		= HZ_NORMAL;

		//p->fxcolor		= FXC_ZING;

		p->origin[0]		= origin[0];
		p->origin[1]		= origin[1];
		p->origin[2]		= origin[2];

		p->gorigin[0]		= end[0];
		p->gorigin[1]		= end[1];
		p->gorigin[2]		= end[2];

		VectorCopy(vec3_origin, p->velocity);

	}
}


void R_ParticleCircle (vec3_t origin)
{
	int			i;
	particle_t	*p;

	for (i=0 ; i<3 ; i++)
	{
		NEWPARTICLE();		
		
		p->texnum		= circle_tex;

		p->bounce		= 1;
			
		p->die				= cl.time + 0.1;

		p->scale			= lhrandom(4,6);
		p->alpha			= 255;
		
		COLOR(255,255,255);


		p->type			= pt_static;//expandfade;//static;
		p->glow			= true;//false;//true; //Tei black sparkles

		p->origin[0]		= origin[0];
		p->origin[1]		= origin[1];
		p->origin[2]		= origin[2];


		VectorCopy(vec3_origin, p->velocity);

	}
}
