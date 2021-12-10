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
// draw.c -- this is the only file outside the refresh that touches the
// vid buffer

#include "quakedef.h"

//
// Structures
//
typedef struct
{
	int		texnum;
	float	sl, tl, sh, th;
} glpic_t;

typedef struct cachepic_s
{
	char		name[MAX_QPATH];
	qpic_t		pic;
	byte		padding[32];	// for appended glpic
} cachepic_t;

//
// Bytes
//
byte		conback_buffer[sizeof(qpic_t) + sizeof(glpic_t)];
byte		menuplyr_pixels[4096];
byte		*draw_chars;

//
// qpic_t
//
qpic_t		*conback = (qpic_t *)&conback_buffer;
qpic_t		*draw_disc;
qpic_t		*draw_backtile;

//
// Defines
//
#define	MAX_CACHED_PICS		128

//
// Cvar's used
//
cvar_t		pr_checkextension = {"pr_checkextension", "1"};
cvar_t		gl_max_size = {"gl_max_size", "0"};


cvar_t		gl_particle_limiter = {"gl_particle_limiter", "5"};//Tei limit particles in face


//
// Integers
//
int			translate_texture;
int			char_texture;
int			map_snapshot;			// MapShots
int			map_snapname;			// MapShots
int			gl_solid_format = 3;
int			gl_alpha_format = 4;
int			GetRSForName(char name[56]);
int			menu_numcachepics;
int			pic_texels;
int			pic_count;

//
// cachepic_t
//
cachepic_t	menu_cachepics[MAX_CACHED_PICS];

//
// Externals
//
extern void LoadSky_f(void);		// Skybox
extern int	shinytexture;			// Enviroment Mapping
extern int	causticstexture[32];		// Underwater Caustics
extern unsigned char d_15to8table[65536];

//
// Functions used from other files
//
void Draw_TextureMode_f (void);
void Crosshair_Init (void);
void Font_Init (void);

//=============================================================================

/* Support Routines */

/*
================
Draw_CachePicFromWad
================
*/
qpic_t *Draw_PicFromWad (char *name)
{
	FILE	*f;
	qpic_t	*p;
	glpic_t	*gl;
	char	newname[32];

	p = W_GetLumpName (name);
	gl = (glpic_t *)p->data;

	sprintf (newname,"gfx/wad/%s.tga", name);

	COM_FOpenFile (newname, &f);

	if (!f)
	{
		sprintf (newname,"gfx/wad/%s.pcx", name);

		COM_FOpenFile (newname, &f);
	}

	if (f)
	{
		fclose (f);

		sprintf (newname,"gfx/wad/%s",name);

		gl->texnum = loadtextureimage(newname, false, false);

		gl->sl = 0;
		gl->sh = 1;
		gl->tl = 0;
		gl->th = 1;

		return p;
	}

	gl->texnum = GL_LoadTexture (name, p->width, p->height, p->data, false, true, 1);

	gl->sl = 0;
	gl->sh = 1;
	gl->tl = 0;
	gl->th = 1;

	return p;
}
	
/*
================
Draw_CachePic
================
*/
qpic_t	*Draw_CachePic (char *path)
{
	cachepic_t	*pic;
	int			i;
	qpic_t		*dat;
	glpic_t		*gl;

	for (pic=menu_cachepics, i=0 ; i<menu_numcachepics ; pic++, i++)
		if (!strcmp (path, pic->name))
			return &pic->pic;

	if (menu_numcachepics == MAX_CACHED_PICS)
		Sys_Error ("menu_numcachepics == MAX_CACHED_PICS");
	menu_numcachepics++;
	strcpy (pic->name, path);

//
// load the pic from disk
//
	

	dat = (qpic_t *)COM_LoadTempFile (path);	
	if (!dat)
		Sys_Error ("Draw_CachePic: failed to load '%s'", path);


	

	SwapPic (dat);

	// HACK HACK HACK --- we need to keep the bytes for
	// the translatable player picture just for the menu
	// configuration dialog
	if (!strcmp (path, "gfx/menuplyr.lmp"))
		memcpy (menuplyr_pixels, dat->data, dat->width*dat->height);

	pic->pic.width = dat->width;
	pic->pic.height = dat->height;
		
	gl = (glpic_t *)pic->pic.data;

	gl->texnum = loadtextureimage (path, false, false);

	//Tei hud
	if (!strncmp(path,"pics/",5))
	{
		pic->pic.width =	image_width;
		pic->pic.height =	image_height;		
	}
	//Tei hud

	if (gl->texnum == 0)
		gl->texnum = GL_LoadTexture (path, dat->width, dat->height, dat->data, false, true, 1);
	
	gl->sl = 0;
	gl->sh = 1;
	gl->tl = 0;
	gl->th = 1;

	return &pic->pic;
}

//=============================================================================

/* Main function */
/*
===============
Draw_Init
===============
*/

GL_ResetTextures_f( void );

extern int detailtexture, grasstexture;


void Draw_Init (void)
{
	int		i;
	qpic_t	*cb;
	glpic_t	*gl;
	int		start;
	int		maxsize;
	byte	*ncdata;
	char	caustics[MAX_QPATH];

	//
	// Register cvar's
	//
	Cvar_RegisterVariable (&gl_max_size);
	Cvar_RegisterVariable (&pr_checkextension);

	Cvar_RegisterVariable (&gl_particle_limiter);//Tei no particles in face

	Con_Printf("Loading Scripts\n");

	//
	// Initialize rscript
	//
	InitRenderScripts(); 
	InitModelScripts();//Tei model scripts

	glGetIntegerv( GL_MAX_TEXTURE_SIZE, &maxsize );
		Cvar_SetValue ("gl_max_size", (int)maxsize);

	//
	// Register commands
	//
	Cmd_AddCommand ( "gl_texturemode",	&Draw_TextureMode_f);
	Cmd_AddCommand ( "loadsky",			&LoadSky_f);
	Cmd_AddCommand ( "resetTextures",	&GL_ResetTextures_f );

	//
	// Initialize font engine
	//
	Font_Init(); // gl_font.c

	//
	// Initialize crosshair engine
	// 
	Crosshair_Init(); // gl_crosshair.c

	//
	// Allocate memory
	//
	start = Hunk_LowMark();

	//
	// Load basic console (LMP)
	//
	cb = (qpic_t *)COM_LoadTempFile ("gfx/conback.lmp");	
	if (!cb)
	{
		Sys_Error ("Couldn't load gfx/conback.lmp");
	}
	SwapPic (cb);

	//
	// Initialize Console
	//
	conback->width = cb->width;
	conback->height = cb->height;
	ncdata = cb->data;
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	//Changed
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	//Changed
	gl = (glpic_t *)conback->data;
	gl->texnum = loadtextureimage ("gfx/conback", false, false); // try to find a console tga/pcx image
	if (gl->texnum == 0)
	{
		gl->texnum = GL_LoadTexture ("conback", conback->width, conback->height, ncdata, false, false, 1);
	}
	gl->sl = 0;
	gl->sh = 1;
	gl->tl = 0;
	gl->th = 1;
	conback->rs = GetRSForName("conback");
	conback->width = vid.width;
	conback->height = vid.height;

	//
	// Free loaded console
	//
	Hunk_FreeToLowMark(start);

	//
	// Save a texture slot for translated picture
	//
	translate_texture = texture_extension_number++;

	//
	// Get the other pics we need
	//
	draw_disc = Draw_PicFromWad ("disc");
	draw_backtile = Draw_PicFromWad ("backtile");
	map_snapshot = loadtextureimage("gfx/mapshots/blankpic", false, false);	// Tomaz - MapShots
	map_snapname = loadtextureimage("gfx/mapshots/blankname", false, false);	// Tomaz - MapShots
	shinytexture = loadtextureimage("textures/shiny", false, true);	// FIXME: Enviroment Mapping 

//	grasstexture = loadtextureimage("textures/grass", false, true);	// FIXME: Enviroment Mapping 
        
	detailtexture = 0;//loadtextureimage("textures/detail", false, true); // CHP - load the Detail Texture

	//
	// Initialize misc functions
	//

	// Load the 32 caustic image files
	for(i=0; i<32; i++)
	{
		sprintf(caustics, "textures/caustics/caust%i", i);
		causticstexture[i] = loadtextureimage(caustics, false, true);
	}
}

//=============================================================================

/* Misc functions */

/*
=============
Draw_MapShots

Draws the mapshots seen when a new map is loaded.
=============
*/
void Draw_MapShots(void)
{
	int		x, y;

	if(!mapshots.value)
		return;

	x = vid.width	* 0.5; 
	y = vid.height	* 0.5;

	glClear(GL_COLOR_BUFFER_BIT);

	glBindTexture (GL_TEXTURE_2D, map_snapname);
	glBegin (GL_QUADS);
	glTexCoord2f (0,0);
	glVertex2f (x-128, y-192);
	glTexCoord2f (1,0);
	glVertex2f (x+128, y-192);
	glTexCoord2f (1,1);	
	glVertex2f (x+128, y-64);
	glTexCoord2f (0,1);
	glVertex2f (x-128, y-64);
	glEnd ();

	glBindTexture (GL_TEXTURE_2D, map_snapshot);
	glBegin (GL_QUADS);
	glTexCoord2f (0,0);
	glVertex2f (0,0);
	glTexCoord2f (1,0);
	glVertex2f (vid.width,0);
	glTexCoord2f (1,1);	
	glVertex2f (vid.width, vid.height);
	glTexCoord2f (0,1);
	glVertex2f (0, vid.height);
	glEnd ();
}

/*
=============
Draw_AlphaPic

Draws a picture with an alpha value.
=============
*/
void Draw_AlphaPic (int x, int y, qpic_t *pic, float alpha)
{
	glpic_t			*gl;

	gl = (glpic_t *)pic->data;

	glColor4f (1,1,1,alpha);
	glBindTexture (GL_TEXTURE_2D, gl->texnum);

	glBegin (GL_QUADS);

	glTexCoord2f	(gl->sl,		gl->tl);
	glVertex2f		(x,				y);
	glTexCoord2f	(gl->sh,		gl->tl);
	glVertex2f		(x+pic->width,	y);
	glTexCoord2f	(gl->sh,		gl->th);
	glVertex2f		(x+pic->width,	y+pic->height);
	glTexCoord2f	(gl->sl,		gl->th);
	glVertex2f		(x,				y+pic->height);

	glEnd ();

	glColor4f (1,1,1,1);
}

/*
=============
Draw_Pic

Draws a normal picture.
=============
*/
void Draw_Pic (int x, int y, qpic_t *pic)
{
	glpic_t			*gl;

	gl = (glpic_t *)pic->data;

	glBindTexture (GL_TEXTURE_2D, gl->texnum);
	glBegin (GL_QUADS);
	glTexCoord2f (gl->sl,       gl->tl);
	glVertex2f   (x,            y);
	glTexCoord2f (gl->sh,       gl->tl);
	glVertex2f   (x+pic->width, y);
	glTexCoord2f (gl->sh,       gl->th);
	glVertex2f   (x+pic->width, y+pic->height);
	glTexCoord2f (gl->sl,       gl->th);
	glVertex2f   (x,            y+pic->height);
	glEnd ();
}

/*
=============
Draw_MenuPlayer

Only used for the player color selection menu
=============
*/
void Draw_MenuPlayer (int x, int y, qpic_t *pic, byte *translation)
{
	int				v, u, c;
	unsigned		trans[64*64], *dest;
	byte			*src;
	int				p;

	glBindTexture (GL_TEXTURE_2D, translate_texture);

	c = pic->width * pic->height;

	dest = trans;
	for (v=0 ; v<64 ; v++, dest += 64)
	{
		src = &menuplyr_pixels[ ((v*pic->height)>>6) *pic->width];
		for (u=0 ; u<64 ; u++)
		{
			p = src[(u*pic->width)>>6];
			if (p == 255)
				dest[u] = p;
			else
				dest[u] =  d_8to24table[translation[p]];
		}
	}

	glTexImage2D (GL_TEXTURE_2D, 0, gl_alpha_format, 64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, trans);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBegin (GL_QUADS);
	glTexCoord2f (0, 0);
	glVertex2f (x, y);
	glTexCoord2f (1, 0);
	glVertex2f (x+pic->width, y);
	glTexCoord2f (1, 1);
	glVertex2f (x+pic->width, y+pic->height);
	glTexCoord2f (0, 1);
	glVertex2f (x, y+pic->height);
	glEnd ();
}

/*
================
Draw_ConsoleBackground
================
*/


// Tei versionen (from DP code)
char *TEIVERSION = __TIME__ " " __DATE__;

void Draw_ConsoleBackground (int lines)
{
	char version[80];

	RS_DrawPic(0,lines - vid.height, conback);

	sprintf (version, "TeiQ %s", TEIVERSION);

	
	Draw_String(vid.width - strlen(version)*8 - 8, lines - 16, version,0);
	//Draw_String(vid.width - 136, lines - 32, version);
}
// Tei versionen


/*
=============
Draw_TileClear

This repeats a 64*64 tile graphic to fill the screen around a sized down
refresh window.
=============
*/
void Draw_TileClear (int x, int y, int w, int h)
{
	glBindTexture (GL_TEXTURE_2D, *(int *)draw_backtile->data);
	glBegin (GL_QUADS);
	glTexCoord2f (x/64.0, y/64.0);
	glVertex2f (x, y);
	glTexCoord2f ( (x+w)/64.0, y/64.0);
	glVertex2f (x+w, y);
	glTexCoord2f ( (x+w)/64.0, (y+h)/64.0);
	glVertex2f (x+w, y+h);
	glTexCoord2f ( x/64.0, (y+h)/64.0 );
	glVertex2f (x, y+h);
	glEnd ();
}


/*
=============
Draw_Fill

Fills a box of pixels with a single color
=============
*/
void Draw_Fill (int x, int y, int w, int h, int c)
{
	glDisable (GL_TEXTURE_2D);
	glColor3f (host_basepal[c*3  ]/255.0,
			   host_basepal[c*3+1]/255.0,
			   host_basepal[c*3+2]/255.0);

	glBegin (GL_QUADS);

	glVertex2f (x,y);
	glVertex2f (x+w, y);
	glVertex2f (x+w, y+h);
	glVertex2f (x, y+h);

	glEnd ();
	glColor3f (1,1,1);
	glEnable (GL_TEXTURE_2D);
}

/*
================
Draw_FadeScreen

================
*/
void Draw_FadeScreen (void)
{
	glDisable (GL_TEXTURE_2D);
	glColor4f (0,0,0,0.75f);	// Tomaz - Menu Transparency
	glBegin (GL_QUADS);

	glVertex2f (0,0);
	glVertex2f (vid.width, 0);
	glVertex2f (vid.width, vid.height);
	glVertex2f (0, vid.height);

	glEnd ();
	glColor4f (1,1,1,1);
	glEnable (GL_TEXTURE_2D);
}

/*
================
Draw_BeginDisc

Draws the little blue disc in the corner of the screen.
Call before beginning any disc IO.
================
*/
void Draw_BeginDisc (void)
{
	if (!draw_disc)
		return;
	glDrawBuffer  (GL_FRONT);
	Draw_Pic (vid.width - 24, 0, draw_disc);
	glDrawBuffer  (GL_BACK);
}

/*
================
GL_Set2D

Setup as if the screen was 320*200
================
*/
void GL_Set2D (void)
{
	glViewport (glx, gly, glwidth, glheight);

	glMatrixMode(GL_PROJECTION);
    glLoadIdentity ();
	glOrtho  (0, vid.width, vid.height, 0, -99999, 99999);

	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity ();
	glDisable (GL_DEPTH_TEST);

	glColor4f (1,1,1,1);
}

/////////////////////////////////
//  FH!
// Fiend Hunter code here

/*
int TA_LoadTGA(const char* name);
int TA_LoadJPG(const char* name);
int TA_LoadPCX(const char* name);
*/

/*

// Outcommented because TQ can do the work by the self.

int TA_LoadTGA(const char* name)
{
	int   tex_num;
	FILE*  f;
	char  path[64];
	char  namo[64];

	strcpy(namo, name);

	// special texture names.  Strip leading character.
	if (!Q_strncmp(namo,"$",1) ||
		!Q_strncmp(namo,"#",1) ||
		!Q_strncmp(namo,"*",1) ||
		!Q_strncmp(namo,"%",1)  )
		strcpy(namo, namo + 1);

	strcat((char *)namo, ".tga");
	sprintf (path, namo);
	COM_FOpenFile (path, &f);


	if (!f) 
		return -1;//draws LMP or JPEG when cannot find matching TGA
	else
	{
		tex_num = LoadTGA (f, name);
		//tex_num = GL_LoadTexture(path, targa_header.width, targa_header.height, 32, targa_rgba, 1, 1);
		//free (targa_rgba);
		return tex_num;
	}
}

byte	*jpeg_rgba;

int TA_LoadJPG(const char* name)
{
	int   tex_num;
	FILE*  f;
	char  path[64];
	char  namo[64];
	int		width, height;

	strcpy(namo, name);

	strcat((char *)namo, ".jpg");
	sprintf (path, namo);
	COM_FOpenFile (path, &f);

	if (!f) 
		return -1;
	else
	{
		LoadJPG(f, &width, &height);
		tex_num = GL_LoadTexture(path, width, height, 32, jpeg_rgba, 1, 1);
		free (jpeg_rgba);
		return tex_num;
	}
}

int TA_LoadPCX(const char* name)
{
	int   tex_num;
	FILE*  f;
	char  path[64];
	char  namo[64];
	int		width, height;

	strcpy(namo, name);

	// special texture names.  Strip leading character.
	if (!Q_strncmp(namo,"$",1) ||
		!Q_strncmp(namo,"#",1) ||
		!Q_strncmp(namo,"*",1) ||
		!Q_strncmp(namo,"%",1)  )
		strcpy(namo, namo + 1);

	strcat((char *)namo, ".pcx");
	sprintf (path, namo);
	COM_FOpenFile (path, &f);

	if (!f) 
		return -1;
	else
	{
		LoadPCX(f);
		tex_num = GL_LoadTexture(path, targa_header.width, targa_header.height, 32, targa_rgba, 1, 1);
		free (jpeg_rgba);
		return tex_num;
	}
}

*/

//
//
///////////////////////////////////


