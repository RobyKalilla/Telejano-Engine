
#if 0 //OLD menu
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
#include "quakedef.h"
#include "winquake.h"
#include "menu.h"

// Tei creds

static const char *credits[] =
{
	"              - - - ",
	"",
  //"       T H U N D E R C A T S!"
    "       T E L E J A N O - Q 1!"
	"", 
  //"      The Manga Mech Quake Mod",
    "      The Tei Quake Engine",
	"     (GNU)Copyright 2002 by Tei",
	"            ",
	"",
	"       Thanks to:",
	"  John Carmack    Tomaz\n",
	"  John Cash       Rafael\n",
	"  Brian Hook      Zoid\n",
	"  FrikaC          CocoT\n",
	"  Jeff            Seth G.\n",
	"  Makula          Xage\n",
	"  Merc101		   psycopathz\n",
    "",
	"     Special Thanks to Tomaz &\n",
	"     Brambo for is Engine...\n",
	"",
	"              * * *",
	"",
	"         TQ 1.48 By Tomaz",
	"         Special Thanks To",
	"  LordHavoc     Heffo     Fett    \n",
	"  Phoenix       Radix     MrG     \n",
	"  FrikaC        Muff      Vic     \n",
	"        Additional Thanks To      \n",
	"  scar3crow     CocoT     BramBo  \n",
	"  ArchAngel     Koolio    Quest   \n",
	"  MrPeanut      P5YCHO    Ender   \n",
	"  Atomizer      Deetee    Topaz   \n",
	"  illusion      Midgar    jAGO    \n",
	"  KrimZon       Akuma     Horn    \n",
	"  dakilla       Krust     Harb    \n",
	"",
	"",
	"",
	"",
	"",
	0
};

#define CREDLINES 33

// Tei creds

/*
=========
Quit Menu
=========
*/

int			msgNumber;
int			m_quit_prevstate;
qboolean	wasInMenus;

void M_Menu_Quit_f (void)
{
	if (m_state == m_quit)
		return;
	wasInMenus = (key_dest == key_menu);
	key_dest = key_menu;
	m_quit_prevstate = m_state;
	m_state = m_quit;
	m_entersound = true;
	msgNumber = rand()&7;
}

void M_Quit_Key (int key)
{
	switch (key)
	{
	case K_ESCAPE:
	case 'n':
	case 'N':
		if (wasInMenus)
		{
			m_state = m_quit_prevstate;
			m_entersound = true;
		}
		else
		{
			key_dest = key_game;
			m_state = m_none;
		}
		break;

	case 'Y':
	case 'y':
		key_dest = key_console;
		Host_Quit_f ();
		break;

	default:
		break;
	}
}

int line;

void M_Quit_Draw (void)
{
	int pline,i;

	int base = 12;
	int offsetline = 0;
	int line = 0;

	if (wasInMenus)
	{
		m_state = m_quit_prevstate;
		m_recursiveDraw = true;
		M_Draw ();
		m_state = m_quit;
	}

	M_DrawTextBox (0, 0, 38, 23);

	pline = cl.time;
	offsetline = pline % CREDLINES;
	
	for ( i= 0; i< 20; i++)
	{
			M_Print( 16, (base +i*8), credits[(i + offsetline) % CREDLINES]);
	}
	M_PrintWhite( 16,base + 23 * 8, "       Press 'y' to Quit");
}

#else

//New menu from CheapHack :D

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
                                      ( don't they have a website!???? )
*/
#include "quakedef.h"
#include "winquake.h"
#include "menu.h"

/*
=========
Quit Menu
=========
*/

int			msgNumber;
int			m_quit_prevstate;
qboolean	wasInMenus;

char *quitMessage [] = 
{
/* .........1.........2.... */
  "  I pity the fool who   ",
  "   tries to quit this   ",
  "         game.          ",
  "                        ",
 
  "                        ",
  "    There is no lame    ",
  "     excuses allowed    ",
  "                        ",

  "                        ",
  "    Don't quit......    ",
  "    I don't suck.       ",
  "                        ",

  "                        ",
  "      If you leave,     ",
  "   you'll be branded a  ",
  "       QUITTER!         ",
 
  "                        ",
  "      if (y)            ",
  "     self.netname =     ",
  "       'loser'          ",
 
  "                        ",
  "       Not enough!      ",
  "   Go kill some more!   ",
  "                        ",
 
  "                        ",
  " I am not through with  ",
  "        you yet.        ",
  "                        ",
 
  "      Poll results:     ",
  "      0% say yes        ",
  "      100% say no       ",
  "                        "
};

void M_Menu_Quit_f (void)
{
	if (m_state == m_quit)
		return;
	wasInMenus = (key_dest == key_menu);
	key_dest = key_menu;
	m_quit_prevstate = m_state;
	m_state = m_quit;
	m_entersound = true;
	msgNumber = rand()&7;
}

void M_Quit_Key (int key)
{
	switch (key)
	{
	case K_ESCAPE:
	case 'n':
	case 'N':
		if (wasInMenus)
		{
			m_state = m_quit_prevstate;
			m_entersound = true;
		}
		else
		{
			key_dest = key_game;
			m_state = m_none;
		}
		break;

	case 'Y':
	case 'y':
		key_dest = key_console;
		Host_Quit_f ();
		break;

	default:
		break;
	}
}

void M_Quit_Draw (void)
{
	if (wasInMenus)
	{
		m_state = m_quit_prevstate;
		m_recursiveDraw = true;
		M_Draw ();
		m_state = m_quit;
	}

	M_DrawTextBox (56, 76, 24, 4);
	M_Print (64, 84,  quitMessage[msgNumber*4+0]);
	M_Print (64, 92,  quitMessage[msgNumber*4+1]);
	M_Print (64, 100, quitMessage[msgNumber*4+2]);
	M_Print (64, 108, quitMessage[msgNumber*4+3]);
}


#endif


