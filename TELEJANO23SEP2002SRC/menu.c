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

#define	SLIDER_RANGE	10

/*
===============
M_DrawCharacter
===============
*/
void M_DrawCharacter (int cx, int line, int num)
{
	// BramBo: edit, added fourth param
	Draw_Character ( cx + ((vid.width - 320)>>1), line, num);
}

void M_Print (int cx, int cy, char *str)
{
	Draw_Alt_String (cx + ((vid.width - 320)>>1), cy, str, 0);
}

void M_PrintWhite (int cx, int cy, char *str)
{
	Draw_String (cx + ((vid.width - 320)>>1), cy, str, 0);
}

void M_DrawPic (int x, int y, qpic_t *pic)
{
	Draw_Pic (x + ((vid.width - 320)>>1), y, pic);
}

byte identityTable[256];
byte translationTable[256];

void M_BuildMenuPlayerTable(int top, int bottom)
{
	int		j;
	byte	*dest, *source;

	for (j = 0; j < 256; j++)
		identityTable[j] = j;
	dest = translationTable;
	source = identityTable;
	memcpy (dest, source, 256);

	if (top < 128)	// the artists made some backwards ranges.  sigh.
		memcpy (dest + TOP_RANGE, source + top, 16);
	else
		for (j=0 ; j<16 ; j++)
			dest[TOP_RANGE+j] = source[top+15-j];

	if (bottom < 128)
		memcpy (dest + BOTTOM_RANGE, source + bottom, 16);
	else
		for (j=0 ; j<16 ; j++)
			dest[BOTTOM_RANGE+j] = source[bottom+15-j];
}


void M_DrawMenuPlayer(int x, int y, qpic_t *pic)
{
	Draw_MenuPlayer (x + ((vid.width - 320)>>1), y, pic, translationTable);
}


void M_DrawTextBox (int x, int y, int width, int lines)
{
	qpic_t	*p;
	int		cx, cy;
	int		n;

	// draw left side
	cx = x;
	cy = y;
	p = Draw_CachePic ("gfx/box_tl.lmp");
	M_DrawPic (cx, cy, p);
	p = Draw_CachePic ("gfx/box_ml.lmp");
	for (n = 0; n < lines; n++)
	{
		cy += 8;
		M_DrawPic (cx, cy, p);
	}
	p = Draw_CachePic ("gfx/box_bl.lmp");
	M_DrawPic (cx, cy+8, p);

	// draw middle
	cx += 8;
	while (width > 0)
	{
		cy = y;
		p = Draw_CachePic ("gfx/box_tm.lmp");
		M_DrawPic (cx, cy, p);
		p = Draw_CachePic ("gfx/box_mm.lmp");
		for (n = 0; n < lines; n++)
		{
			cy += 8;
			if (n == 1)
				p = Draw_CachePic ("gfx/box_mm2.lmp");
			M_DrawPic (cx, cy, p);
		}
		p = Draw_CachePic ("gfx/box_bm.lmp");
		M_DrawPic (cx, cy+8, p);
		width -= 2;
		cx += 16;
	}

	// draw right side
	cy = y;
	p = Draw_CachePic ("gfx/box_tr.lmp");
	M_DrawPic (cx, cy, p);
	p = Draw_CachePic ("gfx/box_mr.lmp");
	for (n = 0; n < lines; n++)
	{
		cy += 8;
		M_DrawPic (cx, cy, p);
	}
	p = Draw_CachePic ("gfx/box_br.lmp");
	M_DrawPic (cx, cy+8, p);
}

void M_DrawSlider (int x, int y, float range)
{
	int	i;

	if (range < 0)
		range = 0;
	if (range > 1)
		range = 1;
	M_DrawCharacter (x-8, y, 128);
	for (i=0 ; i<SLIDER_RANGE ; i++)
		M_DrawCharacter (x + i*8, y, 129);
	M_DrawCharacter (x+i*8, y, 130);
	M_DrawCharacter (x + (SLIDER_RANGE-1)*8 * range, y, 131);
}

//Tei slider
char slider[11];

char * MakeSlider(float max, float dx)
{

	int t,i;
	float fx;

	fx = max/10;

	for (i=0;i<10;i++)
		slider[i] = (char)129;

	slider[0] = (char)128;
	slider[9] = (char)130;	
	
	t = dx / fx;

	if(t<0)
		t=0;
	else
	if (t>9)
		t=9;

	slider[t] = (char)131;

	slider[10] = 0;

	return slider;
}
//Tei slider

void M_DrawCheckbox (int x, int y, int on)
{
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	if (on)
	{
		glColor3f(0,1,0);
		M_Print (x, y, "on");
	}
	else
	{
		glColor3f(1,0,0);
		M_Print (x, y, "off");
	}

	glColor3f(1,1,1);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}

/*
================
M_ToggleMenu_f
================
*/

void M_ToggleMenu_f (void)
{
	m_entersound = true;

	if (key_dest == key_menu)
	{
		if (m_state != m_main)
		{
			M_Menu_Main_f ();
			return;
		}
		key_dest = key_game;
		m_state = m_none;
		return;
	}
	if (key_dest == key_console)
	{
		Con_ToggleConsole_f ();
	}
	else
	{
		M_Menu_Main_f ();
	}
}

cvar_t samplerate	= {"samplerate", "0", true};			// Tomaz - Samplerate

void M_Init (void)
{
	Cmd_AddCommand ("togglemenu",		M_ToggleMenu_f);

	Cmd_AddCommand ("menu_main",		M_Menu_Main_f);
	Cmd_AddCommand ("menu_singleplayer",M_Menu_SinglePlayer_f);
	Cmd_AddCommand ("menu_load",		M_Menu_Load_f);
	Cmd_AddCommand ("menu_save",		M_Menu_Save_f);
	Cmd_AddCommand ("menu_multiplayer",	M_Menu_MultiPlayer_f);
	Cmd_AddCommand ("menu_setup",		M_Menu_Setup_f);
	Cmd_AddCommand ("menu_options",		M_Menu_Options_f);
	Cmd_AddCommand ("menu_controls",	M_Menu_Controls_f);
	Cmd_AddCommand ("menu_keys",		M_Menu_Keys_f);
	Cmd_AddCommand ("menu_sound",		M_Menu_Sound_f);
	Cmd_AddCommand ("menu_video",		M_Menu_Video_f);
	Cmd_AddCommand ("menu_misc",		M_Menu_Misc_f);
	Cmd_AddCommand ("help",				M_Menu_Help_f);
	Cmd_AddCommand ("menu_quit",		M_Menu_Quit_f);
	
	// Tei class select
	Cmd_AddCommand ("menu_misc2",		M_Menu_Misc2_f);
	// Tei class select

	// Tei class select
	Cmd_AddCommand ("emenus",		M_Menu_LocalMenus_f);
	// Tei class select



	Cvar_RegisterVariable (&samplerate);	// Tomaz - Samplerate
}

void M_Draw (void)
{
	if (m_state == m_none || key_dest != key_menu)
		return;

	if (!m_recursiveDraw)
	{
		scr_copyeverything = 1;

		if (scr_con_current)
		{
			Draw_ConsoleBackground (vid.height);
			S_ExtraUpdate ();
		}
		else
			Draw_FadeScreen ();

		scr_fullupdate = 0;
	}
	else
	{
		m_recursiveDraw = false;
	}

	switch (m_state)
	{
	case m_none:
		break;

	case m_main:
		M_Main_Draw ();
		break;

	case m_singleplayer:
		M_SinglePlayer_Draw ();
		break;

	case m_load:
		M_Load_Draw ();
		break;

	case m_save:
		M_Save_Draw ();
		break;

	case m_multiplayer:
		M_MultiPlayer_Draw ();
		break;

	case m_setup:
		M_Setup_Draw ();
		break;

	case m_net:
		M_Net_Draw ();
		break;

	case m_options:
		M_Options_Draw ();
		break;

	case m_controls:
		M_Controls_Draw ();
		break;

	case m_keys:
		M_Keys_Draw ();
		break;

	case m_sound:
		M_Sound_Draw ();
		break;

	case m_video:
		M_Video_Draw ();
		break;

	case m_misc:
		M_Misc_Draw ();
		break;

	// Tei class menu
	case m_misc2:
		M_Misc2_Draw ();
		break;
	// Tei class menu


	//Tei localmenus
	case m_LocalMenus:
		M_LocalMenus_Draw ();
		break;
	//Tei localmenus


	case m_help:
		M_Help_Draw ();
		break;

	case m_quit:
		M_Quit_Draw ();
		break;

	case m_serialconfig:
		M_SerialConfig_Draw ();
		break;

	case m_modemconfig:
		M_ModemConfig_Draw ();
		break;

	case m_lanconfig:
		M_LanConfig_Draw ();
		break;

	case m_gameoptions:
		M_GameOptions_Draw ();
		break;

	case m_search:
		M_Search_Draw ();
		break;

	case m_slist:
		M_ServerList_Draw ();
		break;
	}

	if (m_entersound)
	{
		S_LocalSound ("misc/menu2.wav");
		m_entersound = false;
	}

	S_ExtraUpdate ();
}


void M_Keydown (int key)
{
	switch (m_state)
	{
	case m_none:
		return;

	case m_main:
		M_Main_Key (key);
		return;

	case m_singleplayer:
		M_SinglePlayer_Key (key);
		return;

	case m_load:
		M_Load_Key (key);
		return;

	case m_save:
		M_Save_Key (key);
		return;

	case m_multiplayer:
		M_MultiPlayer_Key (key);
		return;

	case m_setup:
		M_Setup_Key (key);
		return;

	case m_net:
		M_Net_Key (key);
		return;

	case m_options:
		M_Options_Key (key);
		return;

	case m_controls:
		M_Controls_Key (key);
		return;

	case m_keys:
		M_Keys_Key (key);
		return;

	case m_sound:
		M_Sound_Key (key);
		return;

	case m_video:
		M_Video_Key (key);
		return;

	case m_misc:
		M_Misc_Key (key);
		return;

	// Tei class menu 		
	case m_misc2:
		M_Misc2_Key (key);
		return;
	// Tei class menu
	

	//Tei 
	case m_LocalMenus:
		M_LocalMenus_Key (key);
		return;
	//Tei

	case m_help:
		M_Help_Key (key);
		return;

	case m_quit:
		M_Quit_Key (key);
		return;

	case m_serialconfig:
		M_SerialConfig_Key (key);
		return;

	case m_modemconfig:
		M_ModemConfig_Key (key);
		return;

	case m_lanconfig:
		M_LanConfig_Key (key);
		return;

	case m_gameoptions:
		M_GameOptions_Key (key);
		return;

	case m_search:
		M_Search_Key (key);
		break;

	case m_slist:
		M_ServerList_Key (key);
		return;
	}
}

int IsMouseOver(MOUSEXY inm, POINT ptm)
{
	if (ptm.x >= inm.mx1 && ptm.x <= inm.mx2)
		if (ptm.y >= inm.my1 && ptm.y <= inm.my2)
			return 1;
	return 0;
}

void setmousexy(MOUSEXY *inm, int imx1, int offx, int imy1, int offy)
{
	inm->mx1 = imx1;
	inm->mx2 = inm->mx1 + offx;
	inm->my1 = imy1;
	inm->my2 = inm->my1 + offy;
}