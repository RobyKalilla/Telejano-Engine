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
// menus
//

enum 
{
	m_none, 
	m_main, 
	m_singleplayer, 
	m_load, 
	m_save, 
	m_multiplayer, 
	m_setup, 
	m_net, 
	m_options, 
	m_controls, 
	m_keys, 
	m_sound, 
	m_video, 
	m_misc, 
	// Tei class select
	m_misc2,
	// Tei class select
	//Tei localmenu editor
	m_LocalMenus,
	//Tei localmenu editor
	m_help, 
	m_quit, 
	m_serialconfig, 
	m_modemconfig, 
	m_lanconfig, 
	m_gameoptions, 
	m_search, 
	m_slist
} m_state;

typedef struct
{
	int mx1;
	int my1;
	int mx2;
	int my2;
}
MOUSEXY;

qboolean	m_entersound;
qboolean	m_recursiveDraw;

void M_DrawCharacter (int cx, int line, int num);
void M_Print (int cx, int cy, char *str);
void M_PrintWhite (int cx, int cy, char *str);
void M_DrawPic (int x, int y, qpic_t *pic);
void M_BuildMenuPlayerTable(int top, int bottom);
void M_DrawMenuPlayer(int x, int y, qpic_t *pic);
void M_DrawTextBox (int x, int y, int width, int lines);
void M_DrawSlider (int x, int y, float range);
void M_DrawCheckbox (int x, int y, int on);

void M_Menu_Main_f (void);

void M_Menu_SinglePlayer_f (void);
void M_Menu_Load_f (void);
void M_Menu_Save_f (void);

void M_Menu_MultiPlayer_f (void);
void M_Menu_Setup_f (void);
void M_Menu_Net_f (void);

void M_Menu_Options_f (void);
void M_Menu_Controls_f (void);
void M_Menu_Keys_f (void);
void M_Menu_Sound_f (void);
void M_Menu_Video_f (void);
void M_Menu_Misc_f (void);

void M_Menu_Help_f (void);

void M_Menu_Quit_f (void);

void M_Menu_SerialConfig_f (void);
void M_Menu_ModemConfig_f (void);
void M_Menu_LanConfig_f (void);
void M_Menu_GameOptions_f (void);
void M_Menu_Search_f (void);
void M_Menu_ServerList_f (void);

void M_Main_Draw (void);

void M_SinglePlayer_Draw (void);
void M_Load_Draw (void);
void M_Save_Draw (void);

void M_MultiPlayer_Draw (void);
void M_Setup_Draw (void);
void M_Net_Draw (void);

void M_Options_Draw (void);
void M_Controls_Draw (void);
void M_Keys_Draw (void);
void M_Sound_Draw (void);
void M_Video_Draw (void);
void M_Misc_Draw (void);

void M_Help_Draw (void);

void M_Quit_Draw (void);

void M_SerialConfig_Draw (void);
void M_ModemConfig_Draw (void);
void M_LanConfig_Draw (void);
void M_GameOptions_Draw (void);
void M_Search_Draw (void);
void M_ServerList_Draw (void);

void M_Main_Key (int key);

void M_SinglePlayer_Key (int key);
void M_Load_Key (int key);
void M_Save_Key (int key);

void M_MultiPlayer_Key (int key);
void M_Setup_Key (int key);
void M_Net_Key (int key);

void M_Options_Key (int key);
void M_Controls_Key (int key);
void M_Keys_Key (int key);
void M_Sound_Key (int key);
void M_Video_Key (int key);
void M_Misc_Key (int key);

void M_Help_Key (int key);

void M_Quit_Key (int key);

void M_SerialConfig_Key (int key);
void M_ModemConfig_Key (int key);
void M_LanConfig_Key (int key);
void M_GameOptions_Key (int key);
void M_Search_Key (int key);
void M_ServerList_Key (int key);

// Tei class menu
void M_Menu_Misc2_f (void);
// Tei class menu

// Tei class menu
void M_Menu_LocalMenus_f (void);
// Tei class menu

// Tei class menu
void M_Misc2_Draw (void);
// Tei class menu

// Tei class menu
void M_Misc2_Key (int key);
// Tei class menu

// Tei 
void M_LocalMenus_Draw (void);
// Tei 


// Tei 
void M_LocalMenus_Key (int key);
// Tei
