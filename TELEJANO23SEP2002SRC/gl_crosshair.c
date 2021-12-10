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
// this file is specific to the crosshair, since we use several crosshairs
//
#include "quakedef.h"

#define MAX_CROSSHAIR 10

//
// Integers
//
int		crosshair_texture[MAX_CROSSHAIR];	
int		crosshair_initialised = true;

//
// External pictures used for loading (see below)
//
byte	crosshair1[32][32];


//
// Structures
//
typedef struct
{
	float	xsize; // x value
	float	ysize; // y value
	float	alpha; // alpha value
	float	red; // rgb values
	float	green;
	float	blue;
} cflags_t;

typedef struct 
{
	// a slim structure, but it's all about looks, isn't it?
	cflags_t			flags; // additional flags
} crosshair_t;


crosshair_t	crosshairs[MAX_CROSSHAIR]; //max 9 crosshairs (FIXME: 10 overwrites 1)

//=============================================================================
/* Main function */

/*
===============
Crosshair_Init

main crosshair function, where all crosshairs are loaded
===============
*/
void Crosshair_Init (void)
{
	int		x, y;
	byte	data[32][32][4];
	FILE	*f;
	int		instructure, texture_num;
	char		ch[1024], file[1024];
	char		tmp[1024];
	char		flag[1024];
	float		value;

	ch[0] = 0;
	tmp[0] = 0; // 2?
	file[0] = 0;
	flag[0] = 0;

	//
	// Load default crosshair
	//
	for (x=0 ; x<32 ; x++)
	{
		for (y=0 ; y<32 ; y++)
		{
			data[x][y][0]	= 255;
			data[x][y][1]	= 255;
			data[x][y][2]	= 255;
			data[x][y][3]	= crosshair1[x][y];
		}
	}
	crosshair_texture[1] = GL_LoadTexture ("crosshair1", 32, 32, &data[0][0][0], true, true, 4);

	//
	// Open script
	//
	COM_FOpenFile("scripts/crosshair.txt",&f);

	if (!f || feof(f)) 
	{
		Con_Printf(" &f9000 *Failed to load crosshair script &r\n");
		crosshair_initialised = false; // disable crosshair engine, and revert all crosshairs back to the default crosshair
	}

	instructure = 0;

	if (crosshair_initialised)
	{
		Con_Printf(" *Loaded crosshair script\n");

		while (!feof(f)) // loop trough this until we find an eof (end of file)
		{
			fscanf(f,"%s",ch);
			if (instructure) // decide if where are in the script
			{	
				if (!_stricmp(ch,"file"))
				{
					fscanf(f,"%s",file);
					crosshair_texture[texture_num] = loadtextureimage (file, false, true);
					
					//Con_Printf("Crosshair: %s\nTexture number %f\n", file, crosshair_texture[texture_num]);
				}
				if (!_stricmp(ch,"set"))
				{
						fscanf(f,"%s",flag);
						fscanf(f,"%f",&value);
						if (!_stricmp(flag, "alpha"))
						{
							crosshairs[texture_num].flags.alpha		= value;
							//							Con_Printf("Alpha is %f\n", value);
						}
						else if (!_stricmp(flag, "xsize"))
						{
							crosshairs[texture_num].flags.xsize		= value;
						//							Con_Printf("Xsize is %f\n", value);
						}
						else if (!_stricmp(flag, "ysize"))
						{
							crosshairs[texture_num].flags.ysize		= value;
						//							Con_Printf("Ysize is %f\n", value);
						}
						else if (!_stricmp(flag, "red"))
						{
							crosshairs[texture_num].flags.red		= value;
						//							Con_Printf("Red is %f\n", value);
						}
						else if (!_stricmp(flag, "green"))
						{
							crosshairs[texture_num].flags.green		= value;
						//							Con_Printf("Green is %f\n", value);
						}
						else if (!_stricmp(flag, "blue"))
						{
							crosshairs[texture_num].flags.blue		= value;
						//		Con_Printf("Blue is %f\n", value);
						}
				}
				else if (!_stricmp(ch,"}"))
				{
					instructure = 0;
				}
			}	
			else 
			{
				if (_stricmp(ch,"{"))
				{
					strcpy(tmp,ch);
					texture_num = (int)(*tmp - '0'); // nasty conversion		
				}
				else
				{
					instructure = 1;
				}
			}
			
		}	
		fclose(f); // close file
	}

}

/*
===============
Draw_Crosshair

function that draws the crosshair to the center of the screen
===============
*/
void Draw_Crosshair (int num)
{
	int		x	= 0;
	int 	y	= 0;
	float	xsize,ysize,alpha,red,green,blue;

	//
	// Default for if it isn't set...
	//
	xsize = 32;
	ysize = 32;
	alpha = 1; 
	red	  = 1; 
	green = 1;
	blue  = 1;

	//
	// Default to internal crosshair if crosshair engine isn't runnning
	//
	if (!crosshair_initialised)
	{
		// default values
		num = 1; // set to default pic
	}
	else
	{
		// assign script values to floats
		xsize = crosshairs[num].flags.xsize;
		ysize = crosshairs[num].flags.ysize;
		alpha = crosshairs[num].flags.alpha;
		red	  = crosshairs[num].flags.red;
		blue  = crosshairs[num].flags.blue;
		green = crosshairs[num].flags.green;
	}

	//
	// Crosshair offset
	//
	x = (vid.width /2) - 16; // was 14
	y = (vid.height/2) - 8;  // was 14

	//
	// Start drawing
	//
	glColor4f(red,green,blue,alpha);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glBindTexture (GL_TEXTURE_2D, crosshair_texture[num]);

	glBegin (GL_QUADS);
	glTexCoord2f (0, 0);
	glVertex2f (x, y);
	glTexCoord2f (1, 0);
	glVertex2f (x+xsize, y);
	glTexCoord2f (1,1);
	glVertex2f (x+xsize, y+ysize);
	glTexCoord2f (0,1);
	glVertex2f (x, y+ysize);
	glEnd ();
	
	// restore display settings
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glColor4f(1,1,1,1); 
}

//=============================================================================
/* Crosshair pics */

/*
===============
This is the default crosshair, which is loaded when our crosshair.txt file isnt available
===============
*/
byte	crosshair1[32][32] =
{
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,214,214,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,214,214,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,214,214,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,214,214,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,214,214,214,214,214,214,214,214,214,214,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,214,214,214,214,214,214,214,214,214,214,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,214,214,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,214,214,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,214,214,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,214,214,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
};