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

/*
=========
Main Menu
=========
*/

void setmousexy(MOUSEXY *inm, int imx1, int offx, int imy1, int offy);
int IsMouseOver(MOUSEXY inm, POINT ptm);
extern POINT current_pos;
MOUSEXY mmenuxy[5];

int m_save_demonum;

int	main_cursor;
#define	MAIN_ITEMS	5

void M_Menu_Main_f (void)
{
	if (key_dest != key_menu)
	{
		m_save_demonum = cls.demonum;
		cls.demonum = -1;
	}
	key_dest = key_menu;
	m_state = m_main;
	m_entersound = true;

	setmousexy(&mmenuxy[0],  98,  130,  60, 19);
	setmousexy(&mmenuxy[1],  228, 117, 60, 19);
	setmousexy(&mmenuxy[2],  345, 66,  60, 19);
	setmousexy(&mmenuxy[3],  411, 83,  60, 19);
	setmousexy(&mmenuxy[4],  494, 55,  60,19);
}

void M_Main_Draw (void)
{
	int		f;
	qpic_t	*p;

	M_DrawPic (16, 4, Draw_CachePic ("gfx/qplaque.lmp") );
	p = Draw_CachePic ("gfx/ttl_main.lmp");
	M_DrawPic ( (320-p->width)/2, 4, p);
	M_DrawPic (72, 32, Draw_CachePic ("gfx/mainmenu.lmp") );

	Draw_Pic (current_pos.x, current_pos.y, Draw_CachePic ("gfx/menudot1.lmp") );

	f = (int)(host_time * 10)%6;

	M_DrawPic (54, 32 + main_cursor * 20,Draw_CachePic( va("gfx/menudot%i.lmp", f+1 ) ) );
}


void M_Main_Key (int key)
{
	switch (key)
	{
	case K_ESCAPE:
		key_dest = key_game;
		m_state = m_none;
		cls.demonum = m_save_demonum;
		if (cls.demonum != -1 && !cls.demoplayback && cls.state != ca_connected)
			CL_NextDemo ();
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menu1.wav");
		if (++main_cursor >= MAIN_ITEMS)
			main_cursor = 0;
		break;

	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		if (--main_cursor < 0)
			main_cursor = MAIN_ITEMS - 1;
		break;

	case K_ENTER:
		m_entersound = true;

		switch (main_cursor)
		{
		case 0:
			M_Menu_SinglePlayer_f ();
			break;

		case 1:
			M_Menu_MultiPlayer_f ();
			break;

		case 2:
			M_Menu_Options_f ();
			break;

		case 3:
			M_Menu_Help_f ();
			break;

		case 4:
			M_Menu_Quit_f ();
			break;
		}
	}
}