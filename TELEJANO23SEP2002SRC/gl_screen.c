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

// screen.c -- master for refresh, status bar, console, chat, notify, etc

#include "quakedef.h"
#include "png.h"  //fuh : pngscreenshots

/*

background clear
rendering
turtle/net/ram icons
sbar
centerprint / slow centerprint
notify lines
intermission / finale overlay
loading plaque
console
menu

required background clears
required update regions


syncronous draw mode or async
One off screen buffer, with updates either copied or xblited
Need to double buffer?


async draw will require the refresh area to be cleared, because it will be
xblited, but sync draw can just ignore it.

sync
draw

CenterPrint ()
SlowPrint ()
Screen_Update ();
Con_Printf ();

net 
turn off messages option

the refresh is allways rendered, unless the console is full screen


console is:
	notify lines
	half
	full
	

*/


int			glx, gly, glwidth, glheight;

// only the refresh window will be updated unless these variables are flagged 
int			scr_copytop;
int			scr_copyeverything;

float		scr_con_current;
float		scr_conlines;		// lines of console to display

float		oldscreensize, oldfov;
cvar_t		scr_viewsize	= {"viewsize",			"100",	true};
cvar_t		scr_fov			= {"fov",				"90"};
cvar_t		scr_conspeed	= {"scr_conspeed",		"900"};//Tei 15 to 900
cvar_t		scr_showram		= {"showram",			"1"};
cvar_t		scr_showturtle	= {"showturtle",		"0"};
cvar_t		scr_showpause	= {"showpause",			"1"};
cvar_t		gl_triplebuffer = {"gl_triplebuffer",	"1",	true };


extern	cvar_t	crosshair;

qboolean	scr_initialized;		// ready to draw

qpic_t		*scr_ram;
qpic_t		*scr_net;
qpic_t		*scr_turtle;

int			scr_fullupdate;

int			clearconsole;
int			clearnotify;

int			sb_lines;

viddef_t	vid;				// global video state

vrect_t		scr_vrect;

qboolean	scr_disabled_for_loading;
qboolean	scr_drawloading;
float		scr_disabled_time;

extern qboolean console_enabled; // Tomaz - Console Toggle
extern int scr_drawdialog; // NOTE: only used in software mode??

cvar_t brightness = {"brightness", "1.0", true};
cvar_t dbrightness = {"dbrightness", "0", false};//Tei dynamic brigthness (managed by engine)


void SCR_ScreenShot_f (void);
void SCR_CheckDrawCenterString (void);
void SCR_DrawNotifyString (void);
//=============================================================================

/*
====================
CalcFov
====================
*/
float CalcFov (float fov_x, float width, float height)
{
        float   a;
        float   x;

        if (fov_x < 1 || fov_x > 179)
                Sys_Error ("Bad fov: %f", fov_x);

        x = width/tan(fov_x/360*M_PI);

        a = atan (height/x);

        a = a*360/M_PI;

        return a;
}

/*
=================
SCR_CalcRefdef

Must be called whenever vid changes
Internal use only
=================
*/
static void SCR_CalcRefdef (void)
{
	float		size;
	int		h;
	qboolean		full = false;


	scr_fullupdate = 0;		// force a background redraw
	vid.recalc_refdef = 0;

//========================================
	
// bound viewsize
	if (scr_viewsize.value < 30)
		Cvar_Set ("viewsize","30");
	if (scr_viewsize.value > 120)
		Cvar_Set ("viewsize","120");

// bound field of view
	if (scr_fov.value < 10)
		Cvar_Set ("fov","10");
	if (scr_fov.value > 170)
		Cvar_Set ("fov","170");

// intermission is always full screen	
	if (cl.intermission)
		size = 120;
	else
		size = scr_viewsize.value;

	if (size >= 120)
		sb_lines = 0;		// no status bar at all
	else if (size >= 110)
		sb_lines = 24;		// no inventory
	else
		sb_lines = 24+16+8;

	if (scr_viewsize.value >= 100.0) 
	{
		full = true;
		size = 100.0;
	} 
	else
		size = scr_viewsize.value;

	if (cl.intermission)
	{
		full = true;
		size = 100;
		sb_lines = 0;
	}
	size /= 100.0;

	h = vid.height;

	r_refdef.vrect.width = vid.width * size;
	if (r_refdef.vrect.width < 96)
	{
		size = 96.0 / r_refdef.vrect.width;
		r_refdef.vrect.width = 96;	// min for icons
	}

	r_refdef.vrect.height = vid.height * size;

	if (r_refdef.vrect.height > (signed)vid.height)
			r_refdef.vrect.height = vid.height;

	r_refdef.vrect.x = (vid.width - r_refdef.vrect.width)/2;

	if (full)
		r_refdef.vrect.y = 0;
	else 
		r_refdef.vrect.y = (h - r_refdef.vrect.height)/2;

	r_refdef.fov_x = scr_fov.value;//XFX
	r_refdef.fov_y = CalcFov (r_refdef.fov_x, r_refdef.vrect.width, r_refdef.vrect.height) ;

	//r_refdef.fov_x = scr_fov.value * 0.01;//XFX makes stuff disappear


	scr_vrect = r_refdef.vrect;
}


/*
=================
SCR_SizeUp_f

Keybinding command
=================
*/
void SCR_SizeUp_f (void)
{
	Cvar_SetValue ("viewsize",scr_viewsize.value+10);
	vid.recalc_refdef = 1;
}


/*
=================
SCR_SizeDown_f

Keybinding command
=================
*/
void SCR_SizeDown_f (void)
{
	Cvar_SetValue ("viewsize",scr_viewsize.value-10);
	vid.recalc_refdef = 1;
}

//============================================================================
/*
==================
SCR_Init
==================
*/

void SCR_ScreenShotPNG_f(void);
void RegisterScreenshots();


void SCR_Init (void)
{
	Cvar_RegisterVariable (&scr_fov);
	Cvar_RegisterVariable (&scr_viewsize);
	Cvar_RegisterVariable (&scr_conspeed);
	Cvar_RegisterVariable (&scr_showram);
	Cvar_RegisterVariable (&scr_showturtle);
	Cvar_RegisterVariable (&scr_showpause);
	Cvar_RegisterVariable (&gl_triplebuffer);
	Cvar_RegisterVariable (&brightness);
	Cvar_RegisterVariable (&dbrightness);//Tei managed by engine brighness

//
// register our commands
//
	//Cmd_AddCommand ("screenshot",SCR_ScreenShot_f);//OLD! Tga

#if 1 //PNG!

	RegisterScreenshots();
#endif 

	Cmd_AddCommand ("sizeup",SCR_SizeUp_f);
	Cmd_AddCommand ("sizedown",SCR_SizeDown_f);

	scr_ram = Draw_PicFromWad ("ram");
	scr_net = Draw_PicFromWad ("net");
	scr_turtle = Draw_PicFromWad ("turtle");

	scr_initialized = true;
}

/*
==============
SCR_DrawTurtle
==============
*/
void SCR_DrawTurtle (void)
{
	static int	count;
	
	if (!scr_showturtle.value)
		return;

	if (host_frametime < 0.1)
	{
		count = 0;
		return;
	}

	count++;
	if (count < 3)
		return;

	Draw_Pic (scr_vrect.x, scr_vrect.y, scr_turtle);
}

/*
==============
SCR_DrawNet
==============
*/
void SCR_DrawNet (void)
{
	if (realtime - cl.last_received_message < 0.3)
		return;
	if (cls.demoplayback)
		return;

	Draw_Pic (scr_vrect.x+64, scr_vrect.y, scr_net);
}

// Tomaz - FPS Counter Begin
/*
==============
SCR_DrawFPS
==============
*/

float	fpscounter;
char	temp[32];

void SCR_DrawFPS (void)
{
	static double	currtime;
	double			newtime;
	int				calc;

	if ((cl.time + 1) < fpscounter)
		fpscounter = cl.time + 0.1;

	newtime		= Sys_FloatTime();
	calc		= (int) ((1.0 / (newtime - currtime)) + 0.5);
	currtime	= newtime;

	if (cl.time > fpscounter)
	{
		sprintf(temp, "%3i FPS", calc);
		fpscounter = cl.time + 0.1;
		// Tei - FPS control
		if (calc < fpscroak.value )
			Con_Printf (" %d FPS, %.0f Croak\n", calc,fpscroak.value); 
		// Tei - FPS control
	}

	// Tei - FPS control
	if (show_fps.value)
	// Tei - FPS control
		Draw_String( vid.width - 64, 0, temp, 0);
}

/*
==============
GL_BrightenScreen
By Lordhavoc
==============
*/
void GL_BrightenScreen (void)
{
	float f;

	// a lot of people might want to leave brightness at 1, so why slow them down?
	if (((brightness.value + brightness.value ) == 1.0) ) return;

	if (brightness.value < 0.0)
		Cvar_SetValue ("brightness", 0.0);
	else if (brightness.value > 2.0)
		Cvar_SetValue ("brightness", 2.0);

	f = brightness.value + dbrightness.value;//Tei dynamic brightness

	if (f >= 1.01f)
	{
		glDisable (GL_TEXTURE_2D);
		glBlendFunc (GL_DST_COLOR, GL_ONE);

		glBegin (GL_TRIANGLES);

		glColor3f (f-1, f-1, f-1);

		glVertex2f (-5000, -5000);
		glVertex2f (10000, -5000);
		glVertex2f (-5000, 10000);

		glEnd ();

		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_TEXTURE_2D);
	}
}

/*
==============
DrawPause
==============
*/
void SCR_DrawPause (void)
{
	qpic_t	*pic;

	if (!scr_showpause.value)		// turn off for screenshots
		return;

	if (!cl.paused)
		return;

	pic = Draw_CachePic ("gfx/pause.lmp");
	Draw_Pic ( (vid.width - pic->width)/2, 
		(vid.height - 48 - pic->height)/2, pic);
}



/*
==============
SCR_DrawLoading
==============
*/
void SCR_DrawLoading (void)
{
//	qpic_t	*pic;

//	if (!scr_drawloading)
//		return;
		
//	pic = Draw_CachePic ("gfx/loading.lmp");
//	Draw_Pic ( (vid.width - pic->width)/2,(vid.height - 48 - pic->height)/2, pic);
}



//=============================================================================

extern qboolean started_loading;

/*
==================
SCR_SetUpToDrawConsole
==================
*/
void SCR_SetUpToDrawConsole (void)
{
	Con_CheckResize ();
	
	if (scr_drawloading)
		return;		// never a console with loading plaque
		
// decide on the height of the console
	if (con_forcedup)
	{
		if (cl.worldmodel && cls.signon == SIGNONS)
		{
			started_loading = false;
		}
	}

	con_forcedup = !cl.worldmodel || cls.signon != SIGNONS;

	if (con_forcedup)
	{
		scr_conlines = vid.height;		// full screen
		scr_con_current = scr_conlines;
	}
	else if (key_dest == key_console)
		scr_conlines = vid.height/2;	// half screen
	else
		scr_conlines = 0;				// none visible
	
	if (scr_conlines < scr_con_current)
	{
		scr_con_current -= scr_conspeed.value;
		if (scr_conlines > scr_con_current)
			scr_con_current = scr_conlines;

	}
	else if (scr_conlines > scr_con_current)
	{
		scr_con_current += scr_conspeed.value;
		if (scr_conlines < scr_con_current)
			scr_con_current = scr_conlines;
	}

	else
		con_notifylines = 0;
}
	
/*
==================
SCR_DrawConsole
==================
*/
void SCR_DrawConsole (void)
{
	if (scr_con_current)
	{
		// Tomaz - Console Toggle Begin
		if (console_enabled)
		{
			scr_copyeverything = 1;
			Con_DrawConsole (scr_con_current, true);
			clearconsole = 0;
		}
		// Tomaz - Console Toggle End
	}
	else
	{
		if (key_dest == key_game || key_dest == key_message|| key_dest == key_cmd)
			Con_DrawNotify ();	// only draw notify in game
	}
}


/* 
============================================================================== 
 
						SCREEN SHOTS 
 
============================================================================== 
*/ 

typedef struct _TargaHeader {
	unsigned char 	id_length, colormap_type, image_type;
	unsigned short	colormap_index, colormap_length;
	unsigned char	colormap_size;
	unsigned short	x_origin, y_origin, width, height;
	unsigned char	pixel_size, attributes;
} TargaHeader;

// Tomaz - Redone ScreenShot Begin
// Tei - redone screenshots for more shots support
/* 
================== 
SCR_ScreenShot_f
================== 
*/  

void SCR_ScreenShotTGA_f (void) 
{
	byte		*buffer;
	char		tganame[255]; 
	char		checkname[MAX_OSPATH];
	int			i, c, temp;

// 
// find a file name to save it to 
// 
	sprintf (checkname, "%s/shots", com_gamedir);
	Sys_mkdir (checkname);	


	sprintf( tganame,"telejano_000.tga");	

	

	for (i=0 ; i<=999 ; i++) 
	{
		tganame[11-2] = (i/100)%10 + '0';
		tganame[11-1] = (i/10 )%10 + '0';
		tganame[11-0] = (i    )%10 + '0'; 			

		sprintf (checkname, "%s/shots/%s", com_gamedir, tganame);
		if (Sys_FileTime(checkname) == -1)
			break;	// file doesn't exist
	} 
	if (i==999) 
	{
		Con_Printf ("SCR_ScreenShot_f: Couldn't create a TGA file\n"); 
		return;
 	}

	buffer = malloc(glwidth*glheight*3 + 18);
	memset (buffer, 0, 18);
	buffer[2] = 2;		// uncompressed type
	buffer[12] = glwidth&255;
	buffer[13] = glwidth>>8;
	buffer[14] = glheight&255;
	buffer[15] = glheight>>8;
	buffer[16] = 24;	// pixel size

	glReadPixels (glx, gly, glwidth, glheight, GL_RGB, GL_UNSIGNED_BYTE, buffer+18 ); 

	// swap rgb to bgr
	c = 18+glwidth*glheight*3;
	for (i=18 ; i<c ; i+=3)
	{
		temp = buffer[i];
		buffer[i] = buffer[i+2];
		buffer[i+2] = temp;
	}

	COM_WriteFile (va ("shots/%s", tganame), buffer, glwidth*glheight*3 + 18 );

	free (buffer);
	Con_Printf ("Wrote %s\n", tganame);
} 
// Tei - redone screenshots for more shots support


//fuh : png screenshots
void SCR_ScreenShotPNG_f() {
    char    name[MAX_OSPATH];
    byte *data;
    int i;
    FILE *fp;
    png_structp png_ptr;
    png_infop info_ptr;
    png_byte **row_pointers;
	char		checkname[MAX_OSPATH];


//fuh : open file, creating subdirs if needed

	sprintf (checkname, "%s/shots", com_gamedir);
	Sys_mkdir (checkname);	


	sprintf( name,"telejano_000.png");

	for (i=0 ; i<=999 ; i++) 
	{
		name[11-2] = (i/100)%10 + '0';
		name[11-1] = (i/10 )%10 + '0';
		name[11-0] = (i    )%10 + '0'; 			

		sprintf (checkname, "%s/shots/%s", com_gamedir, name);
		if (Sys_FileTime(checkname) == -1)
			break;	// file doesn't exist
	} 
	if (i==999) 
	{
		Con_Printf ("SCR_ScreenShotPNG_f: Couldn't create a PNG file\n"); 
		return;
 	}

    //Q_snprintfz (name, sizeof(name), "%s/%s", com_basedir, filename);
    
    if (!(fp = fopen (checkname, "wb"))) {
        //COM_CreatePath (name);
        if (!(fp = fopen (checkname, "wb")))
            Sys_Error ("Error opening %s", checkname);
    }


    if (!(png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL))) {
        fclose(fp);
        return;
    }

    if (!(info_ptr = png_create_info_struct(png_ptr))) {
        png_destroy_write_struct(&png_ptr, (png_infopp) NULL);
        fclose(fp);
        return;
    }

    if (setjmp(png_ptr->jmpbuf)) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);
        return;
    }

    png_init_io(png_ptr, fp);
    png_set_IHDR(png_ptr, info_ptr, glwidth, glheight, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);    
    png_write_info(png_ptr, info_ptr);

//fuh : glwidth/glheight are screen height/width.  glx/gly is top left coordinates of screen, 
//ually 0,0 unless in windowed mode or something.
  
	data = Z_Malloc (glwidth * glheight * 3);
    row_pointers = Z_Malloc (4 * glheight);
    glReadPixels (glx, gly, glwidth, glheight, GL_RGB, GL_UNSIGNED_BYTE, data );

     //fuh : this is the place to apply gamma/contrast
    //applyGamma(data, glwidth * glheight * 3);

    for (i = 0; i < glheight; i++)
        row_pointers[glheight - i - 1] = data + i * glwidth * 3;
    png_write_image(png_ptr, row_pointers);
    png_write_end(png_ptr, info_ptr);
    Z_Free(data);
    Z_Free(row_pointers);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);
	Con_Printf ("Wrote %s\n", name);

} 

void RegisterScreenshots ()
{
	Cmd_AddCommand ("screenshot_png",SCR_ScreenShotPNG_f);
	Cmd_AddCommand ("screenshot_tga",SCR_ScreenShotTGA_f);
	Cmd_AddCommand ("screenshot",SCR_ScreenShotTGA_f);//Tei, default is tga
}

// Tomaz - Redone ScreenShot End

//=============================================================================
float		scr_centertime_off;

/*
===============
SCR_BeginLoadingPlaque

================
*/
void SCR_BeginLoadingPlaque (void)
{
	S_StopAllSounds (true);

	if (cls.state != ca_connected)
		return;
	if (cls.signon != SIGNONS)
		return;
	
// redraw with no console and the loading plaque
	Con_ClearNotify ();
	scr_centertime_off = 0;
	scr_con_current = 0;

	scr_drawloading = true;
	scr_fullupdate = 0;
	SCR_UpdateScreen ();
	scr_drawloading = false;

	scr_disabled_for_loading = true;
	scr_disabled_time = realtime;
	scr_fullupdate = 0;
}

/*
====================
SCR_EndLoadingPlaque
====================
*/
void SCR_EndLoadingPlaque (void)
{
	scr_disabled_for_loading = false;
	scr_fullupdate = 0;
	Con_ClearNotify ();
}

//=============================================================================

/*
===============
SCR_BringDownConsole

Brings the console down and fades the palettes back to normal
================
*/
void SCR_BringDownConsole (void)
{
	int		i;
	
	scr_centertime_off = 0;
	
	for (i=0 ; i<20 && scr_conlines != scr_con_current ; i++)
		SCR_UpdateScreen ();

	cl.cshifts[0].percent = 0;		// no area contents palette on next frame
	VID_SetPalette (host_basepal);
}

void SCR_TileClear (void)
{
	if (r_refdef.vrect.x > 0) 
	{
		// left
		Draw_TileClear (0, 0, r_refdef.vrect.x, vid.height);
		// right
		Draw_TileClear (r_refdef.vrect.x + r_refdef.vrect.width, 0,
			vid.width - r_refdef.vrect.x + r_refdef.vrect.width, 
			vid.height);
	}
	if (r_refdef.vrect.y > 0) 
	{
		// top
		Draw_TileClear (r_refdef.vrect.x, 0, 
			r_refdef.vrect.x + r_refdef.vrect.width, 
			r_refdef.vrect.y);
		// bottom
		Draw_TileClear (r_refdef.vrect.x,
			r_refdef.vrect.y + r_refdef.vrect.height, 
			r_refdef.vrect.width, 
			vid.height - 
			(r_refdef.vrect.height + r_refdef.vrect.y));
	}
}

/*
==================
SCR_UpdateScreen

This is called every frame, and can also be called explicitly to flush
text to the screen.

WARNING: be very careful calling this from elsewhere, because the refresh
needs almost the entire 256k of stack space!
==================
*/
extern void SHOWLMP_drawall();	// Tomaz - Show Hide LMP

void SCR_UpdateScreen (void)
{
	int				cross;	// Tomaz - Crosshair

	vid.numpages = 2 + gl_triplebuffer.value;

	scr_copytop = 0;
	scr_copyeverything = 0;

	//Con_DPrintf("...scr 1\n");//Tei debug

	if (scr_disabled_for_loading)
	{
		if (realtime - scr_disabled_time > 60)
		{
			scr_disabled_for_loading = false;
			Con_Printf ("load failed.\n");
		}
		else
			return;
	}

	//Con_DPrintf("...scr 2\n");//Tei debug

	if (!scr_initialized || !con_initialized)
		return;				// not initialized yet

	//Con_DPrintf("...scr 3\n");//Tei debug

	GL_BeginRendering (&glx, &gly, &glwidth, &glheight);
	
	//
	// determine size of refresh window
	//

	//Con_DPrintf("...scr 4\n");//Tei debug

	if (oldfov != scr_fov.value)
	{
		oldfov = scr_fov.value;
		vid.recalc_refdef = true;
	}

	if (oldscreensize != scr_viewsize.value)
	{
		oldscreensize = scr_viewsize.value;
		vid.recalc_refdef = true;
	}

	if (vid.recalc_refdef)
		SCR_CalcRefdef ();


	//Con_DPrintf("...scr 5\n");//Tei debug
//
// do 3D refresh drawing, and then update the screen
//
	SCR_SetUpToDrawConsole ();

	//Con_DPrintf("...scr 5 - RenderView\n");//Tei debug


	V_RenderView ();//bug here
		

	//Con_DPrintf("...scr 5 - RenderView - Ok\n");//Tei debug


	GL_Set2D ();

	//Con_DPrintf("...scr 5 - 2d\n");//Tei debug


	//
	// draw any areas not covered by the refresh
	//
	SCR_TileClear ();


	//Con_DPrintf("...scr 6\n");//Tei debug

	//muff - draw map shot until map is loaded and ready
	if (con_forcedup && sv.active)
	{ 
		Draw_MapShots ();
		SCR_DrawLoading ();
	}

	else if (scr_drawdialog)
	{
		Sbar_Draw ();
		Draw_FadeScreen ();
		SCR_DrawNotifyString ();
		scr_copyeverything = true;
	}
	else if (scr_drawloading)
	{
		Draw_MapShots ();
		SCR_DrawLoading ();
	//	Sbar_Draw ();  don't draw the sbar on loading
	}
	else if (cl.intermission == 1 && key_dest == key_game)
	{
		Sbar_IntermissionOverlay ();
	}
	else if (cl.intermission == 2 && key_dest == key_game)
	{
		Sbar_FinaleOverlay ();
		SCR_CheckDrawCenterString ();
	}
	else
	{
		if (crosshair.value)
		{	
			cross = crosshair.value;

			if(cross>10)
			{	
				crosshair.value = 0; 
				Cvar_SetValue ("crosshair", crosshair.value);
				return;
			}
			cross --;
			Draw_Crosshair(cross);
		}	
		SCR_DrawNet ();
		SCR_DrawTurtle ();
		SCR_DrawPause ();
		SCR_CheckDrawCenterString ();
		Sbar_Draw ();
		SHOWLMP_drawall();	
		SCR_DrawConsole ();	
		M_Draw ();
	}

	//Con_DPrintf("...scr 7\n");//Tei debug

	if (show_fps.value || fpscroak.value)
		SCR_DrawFPS ();

	V_UpdatePalette ();

//	GL_BrightenScreen ();
	GL_EndRendering ();

	//Con_DPrintf("...SCR_UpdateScreen - Ok\n");//Tei debug
}

