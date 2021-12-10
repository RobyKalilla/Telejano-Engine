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
/*
RScript loading, parsing, and rendering.

Code syntax is similar to that of shaders
No reference to any shader material was used whilst
making this code, hence the weak and buggy state of it :)
*/

#include "quakedef.h"
#include "gl_rscript.h"
#include "bsp_render.h"

int GetMSForName(char *name, model_t * model);

typedef struct
{
	int		texnum;
	float	sl, tl, sh, th;
} glpic_t;

extern	int			lightmap_textures;
extern	glpoly_t	*lightmap_polys[MAX_LIGHTMAPS];
extern	qboolean	lightmap_modified[MAX_LIGHTMAPS];
extern	glRect_t	lightmap_rectchange[MAX_LIGHTMAPS];
extern	byte		lightmaps[4*MAX_LIGHTMAPS*BLOCK_WIDTH*BLOCK_HEIGHT];

int RS_AnimTexture(int rs)
{
	double rt;

	if (host_time < rscripts[rs].anim.lasttime)
		rscripts[rs].anim.lasttime = 0;

	rt = host_time - rscripts[rs].anim.lasttime;

	if (rt < rscripts[rs].flags.animtime)
		return rscripts[rs].anim.texnum[rscripts[rs].anim.current];

	if (rt > rscripts[rs].flags.animtime)
	{
		rscripts[rs].anim.current	+= (rt / rscripts[rs].flags.animtime);

		while (rscripts[rs].anim.current >= rscripts[rs].anim.num)
			rscripts[rs].anim.current = rscripts[rs].anim.current - rscripts[rs].anim.num;

		rscripts[rs].anim.lasttime	+= rscripts[rs].flags.animtime;
	}
	return rscripts[rs].anim.texnum[rscripts[rs].anim.current];
}

float MakeMapXCoord(float x, int rs)
{
	float txm = 0;

	if (rs > MAX_RS) 
		rs = 0;

	if (!rs) 
		return x;

	if (rscripts[rs].usescroll) 
	{
		txm=realtime*rscripts[rs].scroll.xspeed;
		while (txm > 1 && (1-txm) > 0) txm=1-txm;
		while (txm < 0 && (1+txm) > 1) txm=1+txm;
	}

	if (rscripts[rs].useturb) 
	{
		float power, movediv;

		power		= rscripts[rs].turb.power * 0.05;
		movediv		= rscripts[rs].turb.movediv;
		x			+= sin((x*0.1+realtime) * power) * sin((x*0.1+realtime))/movediv;
	}
	x	+= txm; 

	return x*rscripts[rs].texscale;
}

float MakeMapYCoord(float y, int rs)
{
	float tym = 0;

	if (rs > MAX_RS) 
		rs=0;

	if (!rs) 
		return y;

	if (rscripts[rs].usescroll) 
	{
		tym=realtime*rscripts[rs].scroll.yspeed;
		while (tym > 1 && (1-tym) > 0) tym=1-tym;
		while (tym < 0 && (1+tym) > 1) tym=1+tym;
	}

	if (rscripts[rs].useturb) 
	{
		float power, movediv;

		movediv		= rscripts[rs].turb.movediv;
		power		= rscripts[rs].turb.power *0.05;
		y			+= sin((y*0.1+realtime) * power) * sin((y*0.1+realtime))/movediv;
	}
	y	+= tym; 
	
	return y*rscripts[rs].texscale;
}

void RS_DrawPic (int x, int y, qpic_t *pic)
{
	int				rs;
	glpic_t			*gl;
	qboolean		stage;
	float			tx,ty;

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	gl = (glpic_t *)pic->data;

	stage = true;
	
	rs = pic->rs;

	while (stage)
	{
		if (rscripts[rs].flags.blendfunc)
			glColor4f(1,1,1,rscripts[rs].flags.alpha);
		if (rscripts[rs].useanim)
			glBindTexture (GL_TEXTURE_2D, RS_AnimTexture(rs));

		else if (rscripts[rs].texnum)
			glBindTexture (GL_TEXTURE_2D, rscripts[rs].texnum);
		else
			glBindTexture (GL_TEXTURE_2D, gl->texnum);
			
		glBegin (GL_QUADS);

		tx	= MakeMapXCoord(gl->sl,rs);
		ty	= MakeMapYCoord(gl->tl,rs);
		glTexCoord2f (tx, ty);
		glVertex2f (x, y);

		tx	= MakeMapXCoord(gl->sh,rs);
		ty	= MakeMapYCoord(gl->tl,rs);
		glTexCoord2f (tx, ty);
		glVertex2f (x+pic->width, y);

		tx	= MakeMapXCoord(gl->sh,rs);
		ty	= MakeMapYCoord(gl->th,rs);
		glTexCoord2f (tx, ty);
		glVertex2f (x+pic->width, y+pic->height);

		tx	= MakeMapXCoord(gl->sl,rs);
		ty	= MakeMapYCoord(gl->th,rs);
		glTexCoord2f (tx, ty);
		glVertex2f (x, y+pic->height);

		glEnd ();

		if (rscripts[rs].flags.blendfunc)
			glColor4f(1,1,1,1); 

		if (rscripts[rs].nextstage)
			rs = rscripts[rs].nextstage;
		else
			stage = false;
	}

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}

void FinishScripts(int i)
{
	int c;

	if (!strcmp(rscripts[i].scriptname,""))
		return;

	if (rscripts[i].nextname)
		rscripts[i].nextstage = GetRSForName(rscripts[i].nextname);

	if (!strcmp(rscripts[i].texname,""))
		strcpy(rscripts[i].texname,rscripts[i].scriptname);

	if (rscripts[i].anim.num) 
	{
		for (c=0;c<rscripts[i].anim.num;c++)
			rscripts[i].anim.texnum[c] = loadtextureimage(rscripts[i].anim.name[c].name, false, true);
	}

	rscripts[i].texnum = loadtextureimage(rscripts[i].texname, false, false);
//Tei grass
	if (rscripts[i].usegrass)
	{
		rscripts[i].grassmodel = Mod_ForName(rscripts[i].grassname,false);
		if(!rscripts[i].grassmodel)
			rscripts[i].usegrass = false;//if !grassmodel -> usegrass=0
	}

	if (rscripts[i].usegrass2)
	{
		rscripts[i].grassmodel2 = Mod_ForName(rscripts[i].grassname2,false);
		if(!rscripts[i].grassmodel2)
			rscripts[i].usegrass2 = false;//if !grassmodel -> usegrass=0
	}



	if (rscripts[i].usedetail)
	{
		rscripts[i].mydetail =loadtextureimage(rscripts[i].detailname, false, false);
		//Con_Printf("scale is %f, for %s \n", rscripts[i].mydetailscale,rscripts[i].detailname);
		if(!rscripts[i].mydetail)
			rscripts[i].usedetail = false;//if !grassmodel -> usegrass=0
	}
	
//Tei grass, 
}

int GetRSForName(char *name)
{
	int i;

	for (i=0;i<MAX_RS;i++)
	{
		if (!_stricmp(name,rscripts[i].scriptname))
		{
			FinishScripts(i);
			return i;
		}
	}
	return 0;
}

void FinishMScripts(int i,model_t * model)
{
//	int c;
//	model_t * m;

	if (!strcmp(mscripts[i].scriptname,""))
		return;

	if (mscripts[i].nextname)
		mscripts[i].nextstage = GetRSForName(mscripts[i].nextname);

	if (!strcmp(mscripts[i].modelname,""))
		strcpy(mscripts[i].modelname,mscripts[i].scriptname);
	
	if (!model)
	{
		Con_Printf("model not loaded while triing to setup!\n");
		return;
	}

	mscripts[i].model = model;

	if (mscripts[i].usecorona)
		model->dpxflare = mscripts[i].coronasize;

	//model->fullbright = 1;
	//model->noshadow =1;
	//model->effect = MFX_LASERBEAM;

 model->use_FIRE = mscripts[ i ].use_FIRE;
 model->use_FIRE2 = mscripts[ i ].use_FIRE2; 
 model->use_MISSILE = mscripts[ i ].use_MISSILE; 
 model->use_FIRELAMP = mscripts[ i ].use_FIRELAMP; 
 model->use_BLUEFIRE2 = mscripts[ i ].use_BLUEFIRE2; 
 model->use_BLUEFIRE = mscripts[ i ].use_BLUEFIRE; 
 model->use_DOWFIRE = mscripts[ i ].use_DOWFIRE; 
 model->use_ENGINEFIRE2 = mscripts[ i ].use_ENGINEFIRE2; 
 model->use_BIGFIRE = mscripts[ i ].use_BIGFIRE; 
 model->use_FOGMAKER = mscripts[ i ].use_FOGMAKER; 
 model->use_FOGMAKERLITE = mscripts[ i ].use_FOGMAKERLITE; 
 model->use_WATERFALL = mscripts[ i ].use_WATERFALL; 
 model->use_SPARKSHOWER = mscripts[ i ].use_SPARKSHOWER; 
 model->use_ALIENBLOOD = mscripts[ i ].use_ALIENBLOOD; 
 model->use_BOLTFX = mscripts[ i ].use_BOLTFX; 
 model->use_SUN = mscripts[ i ].use_SUN; 
 model->use_LASER		 = mscripts[ i ].use_LASER	; 
 model->use_SPIKE = mscripts[ i ].use_SPIKE; 
 model->use_GLOW = mscripts[ i ].use_GLOW; 
 model->use_LUX = mscripts[ i ].use_LUX; 
 model->use_GIB = mscripts[ i ].use_GIB; 
 model->use_GRASS = mscripts[ i ].use_GRASS; 
 model->use_GRASS2 = mscripts[ i ].use_GRASS2; 
 model->use_LUX2	 = mscripts[ i ].use_LUX2		; 
 model->use_LASERBEAM = mscripts[ i ].use_LASERBEAM; 

 model->fullbright = mscripts[ i ].fullbright; 
 model->noshadow = mscripts[ i ].noshadow; 
 
 if (mscripts[ i ].flags)
	model->flags = mscripts[ i ].flags; 
	

}


int GetMSForName(char *name, model_t * model)
{
	int i;

	for (i=0;i<MAX_MS;i++)
	{
		if (!_stricmp(name,mscripts[i].scriptname))
		{

			Con_Printf("Finishing mscript for %s\n",mscripts[i].scriptname);
			FinishMScripts(i,model);
			return i;
		}
	}

	//Con_Printf("Failed mscript for %s\n",mscripts[i].scriptname);
	return 0;
}

void InitRenderScripts()
{
	FILE		*f;
	char		ch[1024], sp1[1024], sp2[1024];
	float		fp1, fp2, fp3, fp4;
	int			inscript = 0, num = 1, i;

	COM_FOpenFile("scripts/textures.txt", &f);

	if (!f || feof(f)) 
	{
		Con_Printf(" &f9000 *Failed to load Rscript &r\n");
		return;
	}
	
	ch[0] = 0;
	sp1[0] = 0;
	sp2[0] = 0;

	Con_Printf(" *Loaded Rscript\n");

	do 
	{
		fscanf(f,"%s",ch);
		if (inscript) 
		{
			if (!_stricmp(ch,"turb")) 
			{ // turb effect
				fscanf(f,"%f",&fp1); fscanf(f,"%f",&fp2);
				rscripts[num].turb.power	= fp1;
				rscripts[num].turb.movediv	= fp2;
				rscripts[num].useturb		= true;
			}
			else if (!_stricmp(ch,"turbvert")) 
			{ // vertex turb effect
				fscanf(f,"%f",&fp1); fscanf(f,"%f",&fp2); fscanf(f,"%f",&fp3); fscanf(f,"%f",&fp4);
				rscripts[num].vturb.power	= fp1;
				rscripts[num].usevturb		= true;
			} 
//Tei grass
			else if (!_stricmp(ch,"grass")) 
			{ // grass emulation
				
				fscanf(f,"%s",sp1);fscanf(f,"%f",&fp1);				

				strcpy(rscripts[num].grassname, sp1);				
				rscripts[num].grassdensity = fp1;
				rscripts[num].usegrass		= true;
			} 
			else if (!_stricmp(ch,"grass2")) 
			{ // grass emulation
				
				fscanf(f,"%s",sp1);fscanf(f,"%f",&fp1);				

				strcpy(rscripts[num].grassname2, sp1);				
				rscripts[num].grassdensity2 = fp1;
				rscripts[num].usegrass2		= true;
			} 

//Tei grass
//Tei water
			else if (!_stricmp(ch,"water")) 
			{ 
				
				rscripts[num].usewater		= true;
			} 
//Tei water


//Tei fx
			else if (!_stricmp(ch,"nodraw")) 
			{ 
				rscripts[num].nodraw	= true;
			} 
			else if (!_stricmp(ch,"fullbrights")) 
			{ 
				rscripts[num].fullbrights	= true;
			} 
			else if (!_stricmp(ch,"istele")) 
			{ 
				rscripts[num].istele	= true;
			} 
			else if (!_stricmp(ch,"issky")) 
			{ 
				rscripts[num].issky		= true;
			} 
			else if (!_stricmp(ch,"islava")) 
			{ 
				rscripts[num].islava		= true;
			} 
			else if (!_stricmp(ch,"iswater")) 
			{ 
				rscripts[num].islava		= true;
			} 
			else if (!_stricmp(ch,"fsnow")) 
			{ 
				rscripts[num].fsnow		= true;
			} 
			else if (!_stricmp(ch,"frain")) 
			{ 
				rscripts[num].frain		= true;
			} 
			else if (!_stricmp(ch,"flight")) 
			{ 
				rscripts[num].flight	= true;
			} 
			else if (!_stricmp(ch,"ftele")) 
			{ 
				rscripts[num].ftele		= true;
			} 

//Tei fx

//Tei detail
			else if (!_stricmp(ch,"detail")) 
			{ // detail mapping
				rscripts[num].usedetail		= true;
				
				fscanf(f,"%s",sp1);
				
				strcpy(rscripts[num].detailname, sp1);
				
				//rscripts[i].mydetail =loadtextureimage(sp1, false, false);
				//

				fscanf(f,"%f",&fp1);
				rscripts[num].mydetailscale = fp1;

				//if(!rscripts[num].mydetail)
				//	rscripts[num].usedetail = 0;
			} 
//Tei detail

//Tei smoke
			else if (!_stricmp(ch,"smokefire")) 
			{ // vertex turb effect
				rscripts[num].usesmoke		= true;
				
				fscanf(f,"%f",&fp1);fscanf(f,"%f",&fp2);
				rscripts[num].smokeheight = fp1;				
				rscripts[num].smokescale = fp2;
			} 
//Tei smoke
			else if (!_stricmp(ch,"scroll")) 
			{ // scrolling texture
				fscanf(f,"%f",&fp1); fscanf(f,"%f",&fp2);
				rscripts[num].scroll.xspeed = fp1;
				rscripts[num].scroll.yspeed = fp2;
				rscripts[num].usescroll		= true;
			} 
			else if (!_stricmp(ch,"stage")) 
			{ // next stage
				fscanf(f,"%s",sp1);
				strcpy(rscripts[num].nextname, sp1);
			} 
			else if (!_stricmp(ch,"map")) 
			{ // texture
				fscanf(f,"%s",sp1);
				strcpy(rscripts[num].texname, sp1);
				rscripts[num].texexist = true;
			} 
			else if (!_stricmp(ch,"anim")) 
			{ // anim map
				fscanf(f,"%f",&fp1);
				rscripts[num].anim.num = fp1;
				for (i=0; i<fp1; i++)
				{
					if (i > MAX_ANIM_FRAMES-1)
						continue;
					fscanf(f,"%s",sp1);
					strcpy(rscripts[num].anim.name[i].name,sp1);
				}
				rscripts[num].useanim=true;
			} 
			else if (!_stricmp(ch,"set")) 
			{ // set texture flags
				fscanf(f,"%s",sp1); fscanf(f,"%f",&fp1);
				if (!_stricmp(sp1, "alpha")) // alpha amount
					rscripts[num].flags.alpha		= fp1;

				else if (!_stricmp(sp1, "blendfunc")) // use blendfunc?
					rscripts[num].flags.blendfunc	= fp1;

				else if (!_stricmp(sp1, "texscale")) // texture scaling
					rscripts[num].texscale			= fp1;

				else if (!_stricmp(sp1, "envmap")) // environmental mapping?
					rscripts[num].flags.envmap		= fp1;
				else if (!_stricmp(sp1, "animtime")) // animation timing (ms)
					rscripts[num].flags.animtime	= fp1;
			} 
			else if (!_stricmp(ch,"}")) 
			{
				inscript=0;
				num++;
			}	
		} 
		else 
		{
			if (_stricmp(ch,"{"))
			{
				strcpy(rscripts[num].scriptname,ch);
			} 
			else 
			{
				rscripts[num].flags.alpha		= 1;
				rscripts[num].texscale			= 1;
				rscripts[num].flags.animtime	= 1;
				inscript						= 1;
			}
		}
	} while (!feof(f));

	fclose(f);
}



void InitModelScripts()
{
	FILE		*f;
	char		ch[1024], sp1[1024], sp2[1024];
	float		fp1, fp2, fp3;//, fp4;
	int			inscript = 0, num = 1;//, i;

	COM_FOpenFile("scripts/models.txt", &f);

	if (!f || feof(f)) 
	{
		Con_Printf(" &f9000 *Failed to load mscript &r\n");
		return;
	}
	
	ch[0] = 0;
	sp1[0] = 0;
	sp2[0] = 0;

	Con_Printf(" *Loaded mscript\n");

	do 
	{
		fscanf(f,"%s",ch);
		if (inscript) 
		{
			if (!_stricmp(ch,"corona")) 
			{ // turb effect
				fscanf(f,"%f",&fp1);// fscanf(f,"%f",&fp2);
				//mscripts[num].coronaalpha	= fp1;
				mscripts[num].coronasize	= fp1;
				mscripts[num].usecorona		= true;
			}
			else if (!_stricmp(ch,"use_FIRE2")) 
			{ // turb effect
				mscripts[num].use_MISSILE		= true;
			}

			else if (!_stricmp(ch,"use_MISSILE")) 
			{ // turb effect
				mscripts[num].use_MISSILE		= true;
			}

			else if (!_stricmp(ch,"use_FIRELAMP")) 
			{ // turb effect
				mscripts[num].use_FIRELAMP		= true;
			}

			else if (!_stricmp(ch,"use_BLUEFIRE2")) 
			{ // turb effect
				mscripts[num].use_BLUEFIRE2		= true;
			}

			else if (!_stricmp(ch,"use_BLUEFIRE")) 
			{ // turb effect
				mscripts[num].use_BLUEFIRE		= true;
			}

			else if (!_stricmp(ch,"use_DOWFIRE")) 
			{ // turb effect
				mscripts[num].use_DOWFIRE		= true;
			}

			else if (!_stricmp(ch,"use_ENGINEFIRE2")) 
			{ // turb effect
				mscripts[num].use_ENGINEFIRE2		= true;
			}

			else if (!_stricmp(ch,"use_BIGFIRE")) 
			{ // turb effect
				mscripts[num].use_BIGFIRE		= true;
			}

			else if (!_stricmp(ch,"use_FOGMAKER")) 
			{ // turb effect
				mscripts[num].use_FOGMAKER		= true;
			}

			else if (!_stricmp(ch,"use_FOGMAKERLITE")) 
			{ // turb effect
				mscripts[num].use_FOGMAKERLITE		= true;
			}

			else if (!_stricmp(ch,"use_WATERFALL")) 
			{ // turb effect
				mscripts[num].use_WATERFALL		= true;
			}
			else if (!_stricmp(ch,"use_SPARKSHOWER")) 
			{ // turb effect
				mscripts[num].use_SPARKSHOWER		= true;
			}

			else if (!_stricmp(ch,"use_ALIENBLOOD")) 
			{ // turb effect
				mscripts[num].use_ALIENBLOOD		= true;
			}
			else if (!_stricmp(ch,"use_BOLTFX")) 
			{ // turb effect
				mscripts[num].use_BOLTFX		= true;
			}
			else if (!_stricmp(ch,"use_SUN")) 
			{ // turb effect
				mscripts[num].use_SUN		= true;
			}
			else if (!_stricmp(ch,"use_LASER")) 
			{ // turb effect
				mscripts[num].use_LASER		= true;
			}

			else if (!_stricmp(ch,"use_SPIKE")) 
			{ // turb effect
				mscripts[num].use_SPIKE		= true;
			}
			else if (!_stricmp(ch,"use_LUX")) 
			{ // turb effect
				mscripts[num].use_LUX		= true;
			}

			else if (!_stricmp(ch,"use_LASERBEAM")) 
			{ // turb effect
				mscripts[num].use_LASERBEAM		= true;
			}
			else if (!_stricmp(ch,"use_LUX2")) 
			{ // turb effect
				mscripts[num].use_LUX2		= true;
			}
			else if (!_stricmp(ch,"use_FOOTSOUND")) 
			{ // turb effect
				mscripts[num].use_FOOTSOUND		= true;
			}
			else if (!_stricmp(ch,"use_GRASS")) 
			{ // turb effect
				mscripts[num].use_GRASS		= true;
			}
			else if (!_stricmp(ch,"use_GRASS2")) 
			{ // turb effect
				mscripts[num].use_GRASS2		= true;
			}

			else if (!_stricmp(ch,"use_GIB")) 
			{ // turb effect
				mscripts[num].use_GIB		= true;
			}

			else if (!_stricmp(ch,"fullbright")) 
			{ // turb effect
				mscripts[num].fullbright		= true;
			}
			
			else if (!_stricmp(ch,"noshadown")) 
			{ // turb effect
				mscripts[num].noshadow		= true;
			}
			
			if (!_stricmp(ch,"glow")) 
			{ // turb effect
				fscanf(f,"%f",&fp1);fscanf(f,"%f",&fp2);fscanf(f,"%f",&fp3);
				//mscripts[num].coronaalpha	= fp1;
				mscripts[num].glow_color[0]	= fp1;
				mscripts[num].glow_color[1]	= fp2;
				mscripts[num].glow_color[2]	= fp3;
				
				fscanf(f,"%f",&fp1);
				mscripts[num].glow_radius	= fp1;

				mscripts[num].use_GLOW		= true;
			}

			else if (!_stricmp(ch,"}")) 
			{
				inscript=0;
				num++;
			}	
		} 
		else 
		{
			if (_stricmp(ch,"{"))
			{
				strcpy(mscripts[num].scriptname,ch);
			} 
			else 
			{
				//mscripts[num].flags.alpha		= 1;
				//mscripts[num].texscale			= 1;
				//mscripts[num].flags.animtime	= 1;
				inscript						= 1;
			}
		}
	} while (!feof(f));

	fclose(f);
}

// - - - - - - - - - 

// CrapMenus

#define MAX_CM 3

struct crapmenu
{
	char label[200];
	char control[200];
	char command[200];
};

struct crapmenu cm[MAX_CM];














// - - - - - - - - - 




