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
// console.c

#ifdef NeXT
#include <libc.h>
#endif
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <fcntl.h>
#include "quakedef.h"

int 		con_linewidth;

float		con_cursorspeed = 4;

#define		CON_TEXTSIZE	65535 //16384 extra expace needed for Q2 cvarlist 

qboolean 	con_forcedup;		// because no entities to refresh

int			con_totallines;		// total lines in console scrollback
int			con_backscroll;		// lines up from bottom to display
int			con_current;		// where next message will be printed
int			con_x;				// offset in current line for next print
char		*con_text=0;

cvar_t		con_notifytime = {"con_notifytime","3"};		//seconds

#define	NUM_CON_TIMES 8//Tei was 4
float		con_times[NUM_CON_TIMES];	// realtime time the line was generated
								// for transparent notify lines

int			con_vislines;

qboolean	con_debuglog;

#define		MAXCMDLINE	256 
extern	char	key_lines[32][MAXCMDLINE];
extern	int		edit_line;
extern	int		key_linepos;
extern	int		key_insert; // Tomaz - Enhanced Console Editing	

qboolean	con_initialized;

int			con_notifylines;		// scan lines to clear for notify lines

extern void M_Menu_Main_f (void);

extern qboolean console_enabled; // Tomaz - Console Toggle

/*
================
Con_ToggleConsole_f
================
*/
void Con_ToggleConsole_f (void)
{
	// Tomaz - Console Toggle Begin
	if (console_enabled)
	{
		if (key_dest == key_console)
		{
			if (cls.state == ca_connected)
			{
				key_dest = key_game;
				key_lines[edit_line][1] = 0;	// clear any typing
				key_linepos = 1;
			}
			else
			{
				M_Menu_Main_f ();
			}	
		}
		else
			key_dest = key_console;
	
		SCR_EndLoadingPlaque ();
		memset (con_times, 0, sizeof(con_times));
	}
	// Tomaz - Console Toggle End
}

/*
================
Con_Clear_f
================
*/
void Con_Clear_f (void)
{
	if (con_text)
		Q_memset (con_text, ' ', CON_TEXTSIZE);
}

						
/*
================
Con_ClearNotify
================
*/
void Con_ClearNotify (void)
{
	int		i;
	
	for (i=0 ; i<NUM_CON_TIMES ; i++)
		con_times[i] = 0;
}

						
/*
================
Con_MessageMode_f
================
*/
extern qboolean team_message;

void Con_MessageMode_f (void)
{
	key_dest = key_message;
	team_message = false;
}

						
/*
================
Con_MessageMode2_f
================
*/
void Con_MessageMode2_f (void)
{
	key_dest = key_message;
	team_message = true;
}

//XFX
void Con_MessageMode3_f (void)
{
	key_dest = key_cmd;
	team_message = true;
}
//XFX
						
/*
================
Con_CheckResize

If the line width has changed, reformat the buffer.
================
*/
void Con_CheckResize (void)
{
	int		i, j, width, oldwidth, oldtotallines, numlines, numchars;
	char	tbuf[CON_TEXTSIZE];

	width = (vid.width >> 3) - 2;

	if (width == con_linewidth)
		return;

	if (width < 1)			// video hasn't been initialized yet
	{
		//width = 38;
		width = 78; // LordHavoc: changed from 38 to 78 (320 -> 640 conversion)

		con_linewidth = width;
		con_totallines = CON_TEXTSIZE / con_linewidth;
		Q_memset (con_text, ' ', CON_TEXTSIZE);
	}
	else
	{
		oldwidth = con_linewidth;
		con_linewidth = width;
		oldtotallines = con_totallines;
		con_totallines = CON_TEXTSIZE / con_linewidth;
		numlines = oldtotallines;

		if (con_totallines < numlines)
			numlines = con_totallines;

		numchars = oldwidth;
	
		if (con_linewidth < numchars)
			numchars = con_linewidth;

		Q_memcpy (tbuf, con_text, CON_TEXTSIZE);
		Q_memset (con_text, ' ', CON_TEXTSIZE);

		for (i=0 ; i<numlines ; i++)
		{
			for (j=0 ; j<numchars ; j++)
			{
				con_text[(con_totallines - 1 - i) * con_linewidth + j] =
						tbuf[((con_current - i + oldtotallines) %
							  oldtotallines) * oldwidth + j];
			}
		}

		Con_ClearNotify ();
	}

	con_backscroll = 0;
	con_current = con_totallines - 1;
}

void Con_OpenDebugLog(void);

			
/*
================
Con_Dump_f -- johnfitz -- adapted from quake2 source
================
*/
void Con_Dump_f (void)
{
	int		l, x;
	char	*line;
	FILE	*f;
	char	buffer[1024];
	char	name[MAX_OSPATH];

#if 1 
	//johnfitz -- there is a security risk in writing files with an arbitrary filename. so, 
	//until stuffcmd is crippled to alleviate this risk, just force the default filename.
	sprintf (name, "%s/condump.txt", com_gamedir);
#else
	if (Cmd_Argc() > 2)
	{
		Con_Printf ("usage: condump <filename>\n");
		return;
	}

	if (Cmd_Argc() > 1)
	{
		if (strstr(Cmd_Argv(1), ".."))
		{
			Con_Printf ("Relative pathnames are not allowed.\n");
			return;
		}
		sprintf (name, "%s/%s", com_gamedir, Cmd_Argv(1));
		COM_DefaultExtension (name, ".txt");
	}
	else
		sprintf (name, "%s/condump.txt", com_gamedir);
#endif

	COM_CreatePath (name);
	f = fopen (name, "w");
	if (!f)
	{
		Con_Printf ("ERROR: couldn't open file.\n", name);
		return;
	}

	// skip initial empty lines
	for (l = con_current - con_totallines + 1 ; l <= con_current ; l++)
	{
		line = con_text + (l%con_totallines)*con_linewidth;
		for (x=0 ; x<con_linewidth ; x++)
			if (line[x] != ' ')
				break;
		if (x != con_linewidth)
			break;
	}

	// write the remaining lines
	buffer[con_linewidth] = 0;
	for ( ; l <= con_current ; l++)
	{
		line = con_text + (l%con_totallines)*con_linewidth;
		strncpy (buffer, line, con_linewidth);
		for (x=con_linewidth-1 ; x>=0 ; x--)
		{
			if (buffer[x] == ' ')
				buffer[x] = 0;
			else
				break;
		}
		for (x=0; buffer[x]; x++)
			buffer[x] &= 0x7f;

		fprintf (f, "%s\n", buffer);
	}

	fclose (f);
	Con_Printf ("Dumped console text to %s.\n", name);
}
				


/*
================
Con_Init
================
*/
void Con_Init (void)
{
	con_debuglog = COM_CheckParm("-condebug");

	if (con_debuglog)
	{
		Con_OpenDebugLog ();
	}

	con_text = Hunk_AllocName (CON_TEXTSIZE, "context");
	Q_memset (con_text, ' ', CON_TEXTSIZE);
	con_linewidth = -1;
	Con_CheckResize ();
	
	Con_Printf ("Console initialized.\n");

//
// register our commands
//
	Cvar_RegisterVariable (&con_notifytime);

	Cmd_AddCommand ("toggleconsole", Con_ToggleConsole_f);
	Cmd_AddCommand ("messagemode", Con_MessageMode_f);
	Cmd_AddCommand ("messagemode2", Con_MessageMode2_f);
	Cmd_AddCommand ("messagemode3", Con_MessageMode3_f);
	Cmd_AddCommand ("clear", Con_Clear_f);
	Cmd_AddCommand ("condump", Con_Dump_f); //johnfitz
	con_initialized = true;
}


/*
===============
Con_Linefeed
===============
*/
void Con_Linefeed (void)
{
	// Tomaz - Scroll Enhance Begin
	if (con_backscroll)
		con_backscroll++;
	// Tomaz - Scroll Enhance End

	con_x = 0;
	con_current++;
	Q_memset (&con_text[(con_current%con_totallines)*con_linewidth], ' ', con_linewidth);
}

/*
================
Con_Print

Handles cursor positioning, line wrapping, etc
All console printing must go through this in order to be logged to disk
If no console is visible, the notify window will pop up.
================
*/
void Con_Print (char *txt)
{
	int		y;
	int		c, l;
	static int	cr;
	int		mask;
	
//	con_backscroll = 0;	Tomaz - Scroll Enhance

	if (txt[0] == 1)
	{
		mask = 128;		// go to colored text
		S_LocalSound ("misc/talk.wav");
	// play talk wav
		txt++;
	}
	else if (txt[0] == 2)
	{
		mask = 128;		// go to colored text
		txt++;
	}
	else
		mask = 0;


	while ( (c = *txt) )
	{
	// count word length
		for (l=0 ; l< con_linewidth ; l++)
			if ( txt[l] <= ' ')
				break;

	// word wrap
		if (l != con_linewidth && (con_x + l > con_linewidth) )
			con_x = 0;

		txt++;

		if (cr)
		{
			con_current--;
			cr = false;
		}

		
		if (!con_x)
		{
			Con_Linefeed ();
		// mark time for transparent overlay
			if (con_current >= 0)
				con_times[con_current % NUM_CON_TIMES] = realtime;
		}

		switch (c)
		{
		case '\n':
			con_x = 0;
			break;

		case '\r':
			con_x = 0;
			cr = 1;
			break;

		default:	// display character and advance
			y = con_current % con_totallines;
			con_text[y*con_linewidth+con_x] = c | mask;
			con_x++;
			if (con_x >= con_linewidth)
				con_x = 0;
			break;
		}
		
	}
}

/*
================
Con_DebugLog
================
*/

FILE *logfile;

void Con_OpenDebugLog(void)
{
	logfile = fopen(va("%s/telejano.debug.log", com_gamedir),"w");//tei renamed log
}

void Con_DebugLog(const char *fmt, ...)
{
	va_list		args;

	va_start(args, fmt);

	vfprintf(logfile, fmt, args);

	va_end(args);
}

void Con_CloseDebugLog(void)
{
	fclose (logfile);
}

/*
================
Con_Printf

Handles cursor positioning, line wrapping, etc
================
*/

//Lh! improved log file, closed every time (if crash, something survive!)
void Con_DebugLogFile(char *file, char *fmt, ...)
{
    va_list argptr;
    static char data[1024];
    int fd;

    va_start(argptr, fmt);
    vsprintf(data, fmt, argptr);
    va_end(argptr);
    fd = open(file, O_WRONLY | O_CREAT | O_APPEND, 0666);
    write(fd, data, strlen(data));
    close(fd);
}
// hehehe...

#define	MAXPRINTMSG	16384 //Tei - far vprintf limit , 4096 old value

// FIXME: make a buffer size safe vsprintf?
void Con_Printf (char *fmt, ...)
{
	va_list		argptr;
	char		msg[MAXPRINTMSG];
	static qboolean	inupdate;
	
	va_start (argptr,fmt);
	vsprintf (msg,fmt,argptr);
	va_end (argptr);
	
// also echo to debugging console
	Sys_Printf ("%s", msg);	// also echo to debugging console


// log all messages to file
	if (con_debuglog)
	{
		// can't use va() here because it might overwrite other important things
		char logname[MAX_OSPATH];
		sprintf(logname, "%s/qconsole.log", com_gamedir);
		Con_DebugLogFile(logname, "%s", msg);

		//Con_DebugLog("%s", msg);
	}

	if (!con_initialized)
		return;
		
	if (cls.state == ca_dedicated)
		return;		// no graphics mode

// write it to the scrollable buffer
	Con_Print (msg);

}

/*
================
Con_DPrintf

A Con_Printf that only shows up if the "developer" cvar is set
================
*/
void Con_DPrintf (char *fmt, ...)
{
	va_list		argptr;
	char		msg[MAXPRINTMSG];


	if (!developer.value)
		return;			// don't confuse non-developers with techie stuff...

	va_start (argptr,fmt);
	vsprintf (msg,fmt,argptr);
	va_end (argptr);
	
	Con_Printf ("%s", msg);
}


/*
==================
Con_SafePrintf

Okay to call even when the screen can't be updated
==================
*/
void Con_SafePrintf (char *fmt, ...)
{
	va_list		argptr;
	char		msg[1024];
	int			temp;
		
	va_start (argptr,fmt);
	vsprintf (msg,fmt,argptr);
	va_end (argptr);

	temp = scr_disabled_for_loading;
	scr_disabled_for_loading = true;
	Con_Printf ("%s", msg);
	scr_disabled_for_loading = temp;
}


void Com_sprintf (char *dest, int size, char *fmt, ...)
{
	int		len;
	va_list		argptr;
	char	bigbuffer[0x10000];

	va_start (argptr,fmt);
	len = vsprintf (bigbuffer,fmt,argptr);
	va_end (argptr);
	if (len >= size)
		Con_Printf ("Com_sprintf: overflow of %i in %i\n", len, size);
	strncpy (dest, bigbuffer, size-1);
}



/*
==============================================================================

DRAWING

==============================================================================
*/


/*
================
Con_DrawInput

The input line scrolls horizontally if typing goes beyond the right edge
================
*/
void Con_DrawInput (void)
{
	int		y;
	int		i;
	char	editlinecopy[256], *text;	// Tomaz - Enhanced Console Editing

	if (key_dest != key_console && !con_forcedup)
		return;		// don't draw anything

	// Tomaz - Enhanced Console Editing Begin
	text = strcpy(editlinecopy, key_lines[edit_line]);

	y = strlen(text);

// fill out remainder with spaces
	for (i = y; i < 256; i++)
		text[i] = ' ';
		
// add the cursor frame
	if ((int)(realtime * con_cursorspeed) & 1)	// cursor is visible
		text[key_linepos] = 11 + 130 * key_insert;	// either solid block or triagle facing right
	// Tomaz - Enhanced Console Editing End
		
//	prestep if horizontally scrolling
	if (key_linepos >= con_linewidth)
		text += 1 + key_linepos - con_linewidth;
		
// draw it
	y = con_vislines-16;

	Draw_String (8, y, text, con_linewidth);
//	for (i=0 ; i<con_linewidth ; i++)
//		Draw_Character ( (i+1)<<3, con_vislines - 16, text[i]);

// Tomaz - Enhanced Console Editing
}


/*
================
Con_DrawNotify

Draws the last few lines of output transparently over the game top
================
*/
void Con_DrawNotify (void)
{
	int		x, v;
	char	*text, *say_prompt;
	int		i;
	float	time;
	extern char chat_buffer[];

	int j;	// Tomaz

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);	// Tomaz - Colored Characters

	v = 0;
	for (i= con_current-NUM_CON_TIMES+1 ; i<=con_current ; i++)
	{
		if (i < 0)
			continue;
		time = con_times[i % NUM_CON_TIMES];
		if (time == 0)
			continue;
		time = realtime - time;
		if (time > con_notifytime.value)
			continue;
		text = con_text + (i % con_totallines)*con_linewidth;
		
		clearnotify = 0;
		scr_copytop = 1;

		// Tomaz - Colored Characters Begin
		j = 0;

		
		for (x=0 ; x<con_linewidth ; x++)
		{
			if (text[x] == '&')
			{
				if (text[x + 1] == 'f')
				{
					x += 2;
					glColor3f((float)  (text[x]     - '0') * 0.11111111,	// Tomaz - Speed
							  (float)  (text[x + 1] - '0') * 0.11111111,	// Tomaz - Speed
							  (float)  (text[x + 2] - '0') * 0.11111111);	// Tomaz - Speed
					x += 5;
				}
				else if (text[x + 1] == 'r')
				{
					glColor3f(1,1,1);
					x += 3;
				}
			}
			Draw_Character ( (j+1)<<3, v, text[x]);
			j++;
		}
		
//		Draw_String (8, v, text, con_linewidth);
		v += 8;
		// Tomaz - Colored Characters End
	} 

	if (key_dest == key_message || key_dest == key_cmd)
	{
		clearnotify = 0;
		scr_copytop = 1;
	
		x = 0;			
#if 0		
		Draw_String (8, v, "say:", 0);
		Draw_String (40, v, chat_buffer, 0);
		Draw_Character ( (x+5)<<3, v, 10+((int)(realtime*con_cursorspeed)&1));
		v += 8;
#else
		//johnfitz -- distinguish say and say_team
		if (key_dest== key_message)
		{
			if (team_message)
				say_prompt = "say_team:";
			else
				say_prompt = "say:";
		}
		//johnfitz
		else
		{
				say_prompt = "] ";
		}

		Draw_String (8, v, say_prompt,0); //johnfitz

		while(chat_buffer[x])
		{
			Draw_Character ( (x+strlen(say_prompt)+2)<<3, v, chat_buffer[x]); //johnfitz
			x++;
		}
		Draw_Character ( (x+strlen(say_prompt)+2)<<3, v, 10+((int)(realtime*con_cursorspeed)&1)); //johnfitz
		v += 8;
#endif
	}

	glColor3f(1,1,1);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	if (v > con_notifylines)
		con_notifylines = v;
}

/*
================
Con_DrawConsole

Draws the console with the solid background
The typing input line at the bottom should only be drawn if typing is allowed
================
*/
void Con_DrawConsole (int lines, qboolean drawinput)
{
	int				i, x, y;
	int				rows;
	char			*text;
	int				j;
	int	sb;			// Tomaz - Scroll Enhance
	float			r, g, b;

	if (lines <= 0)
		return;

// draw the background
	Draw_ConsoleBackground (lines);

// draw the text
	con_vislines = lines;

	rows = (lines-16)>>3;		// rows of text to draw
	y = lines - 16 - (rows<<3);	// may start slightly negative

	// Tomaz - Scroll Enhance Begin
	if (con_backscroll)
		sb=2;
	else
		sb=0;
	// Tomaz - Scroll Enhance End

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // Tomaz - Colored Characters

//	for (i= con_current - rows + 1 ; i<=con_current ; i++, y+=8 )
	for (i= con_current - rows + 1 ; i<=con_current - sb ; i++, y+=8) // Tomaz - Scroll Enhance End
	{
		j = i - con_backscroll;
		if (j<0)
			j = 0;
		text = con_text + (j % con_totallines)*con_linewidth;

		// Tomaz - Colored Characters Begin
		j = 0;

		for (x=0 ; x<con_linewidth ; x++)
		{
			if (text[x] == '&')
			{
				if (text[x + 1] == 'f')
				{
					x += 2;
					r = (float)(text[x]     - '0') * 0.11111111;	// Tomaz - Speed
					g = (float)(text[x + 1] - '0') * 0.11111111;	// Tomaz - Speed
					b = (float)(text[x + 2] - '0') * 0.11111111;	// Tomaz - Speed
					glColor3f(r,g,b);
					x += 5;
				}
				else if (text[x + 1] == 'r')
				{
					glColor3f(1,1,1);
					x += 3;
				}
			}
			Draw_Character ( (j+1)<<3, y, text[x]);
			j++;
		}
		

//		Draw_String (8, y, text, con_linewidth);
	}

	// Tomaz - Scroll Enhance Begin
	if (sb)	// are we scrolled back?
	{
		y+=8; // skip a line
			
		// draw arrows to show the buffer is backscrolled
		for (x=0 ; x<con_linewidth ; x+=4)
			Draw_Character ((x+1)<<3, y, '^');
	}
	// Tomaz - Scroll Enhance End


	// draw the input prompt, user text, and cursor if desired
	if (drawinput)
	{
		Con_DrawInput ();
	}
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); 
	glColor3f(1,1,1);
	// Tomaz - Colored Characters End
}


/*
==================
Con_NotifyBox
==================
*/
void Con_NotifyBox (char *text)
{
	double		t1, t2;

// during startup for sound / cd warnings
	Con_Printf("\n\n\35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\37\n");

	Con_Printf (text);

	Con_Printf ("Press a key.\n");
	Con_Printf("\35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\37\n");

	key_count = -2;		// wait for a key down and up
	key_dest = key_console;

	do
	{
		t1 = Sys_FloatTime ();
		SCR_UpdateScreen ();
		Sys_SendKeyEvents ();
		t2 = Sys_FloatTime ();
		realtime += t2-t1;		// make the cursor blink
	} while (key_count < 0);

	Con_Printf ("\n");
	key_dest = key_game;
	realtime = 0;				// put the cursor back to invisible
}

