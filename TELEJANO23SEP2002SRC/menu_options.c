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
============
Options Menu
============
*/

extern cvar_t brightness;

int		options_cursor;
#define	OPTIONS_ITEMS	4

void M_Menu_Options_f (void)
{
	key_dest = key_menu;
	m_state = m_options;
	m_entersound = true;
}

void M_Options_Draw (void)
{
	qpic_t	*p;

	M_DrawPic (16, 4, Draw_CachePic ("gfx/qplaque.lmp") );
	p = Draw_CachePic ("gfx/p_option.lmp");
	M_DrawPic ( (320-p->width)/2, 4, p);

	M_Print (16, 32, "              Controls");
	M_Print (16, 40, "                 Sound");
	M_Print (16, 48, "                 Video");
	M_Print (16, 56, "                  Misc");
// cursor
	M_DrawCharacter (200, 32 + options_cursor*8, 12+((int)(realtime*4)&1));
}


void M_Options_Key (int k)
{
	switch (k)
	{
	case K_ESCAPE:
		M_Menu_Main_f ();
		break;

	case K_ENTER:
		m_entersound = true;
		switch (options_cursor)
		{
		case 0:
			M_Menu_Controls_f ();
			break;
		case 1:
			M_Menu_Sound_f ();
			break;
		case 2:
			M_Menu_Video_f ();
			break;
		case 3:
			M_Menu_Misc_f ();
			break;
		}
		return;

	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		options_cursor--;
		if (options_cursor < 0)
			options_cursor = OPTIONS_ITEMS-1;
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menu1.wav");
		options_cursor++;
		if (options_cursor >= OPTIONS_ITEMS)
			options_cursor = 0;
		break;
	}
}

/*
==========
Controls Menu
==========
*/

int		controls_cursor;
#define	CONTROLS_ITEMS	6

void M_Menu_Controls_f () 
{
	key_dest = key_menu;
	m_state = m_controls;
	m_entersound = true;
}

void M_ControlsAdjustSliders (int dir)
{
	S_LocalSound ("misc/menu3.wav");

	switch (controls_cursor)
	{
	case 0:
		M_Menu_Keys_f ();
		break;

	case 1:	// mouse speed
		sensitivity.value += dir * 0.5;
		if (sensitivity.value < 1)
			sensitivity.value = 1;
		if (sensitivity.value > 11)
			sensitivity.value = 11;
		Cvar_SetValue ("sensitivity", sensitivity.value);
		break;

	case 2:	// allways run
		if (cl_forwardspeed.value > 200)
		{
			Cvar_SetValue ("cl_forwardspeed", 200);
			Cvar_SetValue ("cl_backspeed", 200);
		}
		else
		{
			Cvar_SetValue ("cl_forwardspeed", 400);
			Cvar_SetValue ("cl_backspeed", 400);
		}
		break;

	case 3:	// invert mouse
		Cvar_SetValue ("m_pitch", -m_pitch.value);
		break;

	case 4:	// lookspring
		Cvar_SetValue ("lookspring", !lookspring.value);
		break;

	case 5:	// lookstrafe
		Cvar_SetValue ("lookstrafe", !lookstrafe.value);
		break;
	}
}

void M_Controls_Draw () 
{
	float	r;

	M_Print (16, 32, "             Bind Keys");

	M_Print (16, 40, "           Mouse Speed");
	r = (sensitivity.value - 1) * 0.1;	// Tomaz - Speed
	M_DrawSlider (220, 40, r);

	M_Print (16, 48, "            Always Run");
	M_DrawCheckbox (220, 48, cl_forwardspeed.value > 200);

	M_Print (16, 56, "          Invert Mouse");
	M_DrawCheckbox (220, 56, m_pitch.value < 0);

	M_Print (16, 64, "            Lookspring");
	M_DrawCheckbox (220, 64, lookspring.value);

	M_Print (16, 72, "            Lookstrafe");
	M_DrawCheckbox (220, 72, lookstrafe.value);

	M_DrawCharacter (200, 32 + controls_cursor*8, 12+((int)(realtime*4)&1));
}

void M_Controls_Key (int key) 
{
	switch (key)
	{
	case K_ESCAPE:
		M_Menu_Options_f ();
		break;

	case K_ENTER:
		m_entersound = true;
		M_ControlsAdjustSliders (1);
		return;

	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		if (--controls_cursor < 0)
			controls_cursor = CONTROLS_ITEMS-1;
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menu1.wav");
		if (++controls_cursor >= CONTROLS_ITEMS)
			controls_cursor = 0;
		break;

	case K_LEFTARROW:
		M_ControlsAdjustSliders (-1);
		break;

	case K_RIGHTARROW:
		M_ControlsAdjustSliders (1);
		break;
	}
}

/*
=========
Keys Menu
=========
*/

char *bindnames[][2] =
{
{"+attack", 		"attack"},
{"impulse 10", 		"change weapon"},
{"+jump", 			"jump / swim up"},
{"+forward", 		"walk forward"},
{"+back", 			"backpedal"},
{"+left", 			"turn left"},
{"+right", 			"turn right"},
{"+speed", 			"run"},
{"+moveleft", 		"step left"},
{"+moveright", 		"step right"},
{"+strafe", 		"sidestep"},
{"+lookup", 		"look up"},
{"+lookdown", 		"look down"},
{"centerview", 		"center view"},
{"+mlook", 			"mouse look"},
{"+klook", 			"keyboard look"},
{"+moveup",			"swim up"},
{"+movedown",		"swim down"},
// Tei extra key
{"mod_focus",   "flashlight"},
{"messagemode3",	"fast command"},
{"toggleviewmode",	"toggleviewmodes"},
//{"flymode",		"fly on/off"},
//{"+flyon",		"fly mode"},
{"addbot",		"add bot"},
{"delbot",		"del bot"},
//{"botcam",		"bot cam"},
//{"chasecam",		"chase cam"},
//{"menu_misc2",		"class"},
//{"autozoom",		"autozoom"},
//{"autocross",		"autocross"},
//{"expertmove",		"expertmove"},
{"screenshot",		"screenshot"}
// Tei extra key

};

#define	NUMCOMMANDS	(sizeof(bindnames)/sizeof(bindnames[0]))

int		keys_cursor;
int		bind_grab;

void M_Menu_Keys_f (void)
{
	key_dest = key_menu;
	m_state = m_keys;
	m_entersound = true;
}


//#define NUMKEYS 5

#if 1
void M_FindKeysForCommand (char *command, int *twokeys)
{
	int		count;
	int		j;
	int		l;
	char	*b;


	twokeys[0] = twokeys[1] = -1;
	l = strlen(command);
	count = 0;

	for (j=0 ; j<256 ; j++)
	{
		b = keybindings[j];
		if (!b)
			continue;
		if (!strncmp (b, command, l) )
		{
			twokeys[count] = j;
			count++;
			if (count == 2)
				break;
		}
	}
}
#else //Tei new version

void M_FindKeysForCommand (char *command, int *keys)
{
	int		count;
	int		j;
	char	*b;

	for (j = 0;j < NUMKEYS;j++)
		keys[j] = -1;

	count = 0;

	for (j=0 ; j<256 ; j++)
	{
		b = keybindings[j];
		if (!b)
			continue;
		if (!strcmp (b, command) )
		{
			keys[count++] = j;
			if (count == NUMKEYS)
				break;
		}
	}
}

#endif

void M_UnbindCommand (char *command)
{
	int		j;
	int		l;
	char	*b;

	l = strlen(command);

	for (j=0 ; j<256 ; j++)
	{
		b = keybindings[j];
		if (!b)
			continue;
		if (!strncmp (b, command, l) )
			Key_SetBinding (j, "");
	}
}

#define NUMKEYS 5

void M_Keys_Draw (void)
{
	int		i, l;
	int		keys[NUMKEYS];//Tei more binds
	//int		keys[2];
	char	*name;
	int		x, y;
	qpic_t	*p;


	p = Draw_CachePic ("gfx/ttl_cstm.lmp");
	M_DrawPic ( (320-p->width)/2, 4, p);

	if (bind_grab)
		M_Print (12, 32, "Press a key or button for this action");
	else
		M_Print (18, 32, "Enter to change, backspace to clear");

// search for known bindings
	for (i=0 ; i<NUMCOMMANDS ; i++)
	{
		y = 48 + 8*i;

		M_Print (16, y, bindnames[i][1]);

		l = strlen (bindnames[i][0]);

		M_FindKeysForCommand (bindnames[i][0], keys);

		if (keys[0] == -1)
			M_Print (140, y, "???");
#if 1 //Old version
		else
		{
			name = Key_KeynumToString (keys[0]);//Very strange bug [9] --> [0]
			M_Print (140, y, name);
			x = strlen(name) * 8;
			if (keys[1] != -1)
			{
				M_Print (140 + x + 8, y, "or");
				M_Print (140 + x + 32, y, Key_KeynumToString (keys[1]));
			}
		}
#else
		else //Tei from Tomaz tomazrace
		{
			keystring[0] = 0;
			for (j = 0;j < NUMKEYS;j++)
			{
				if (keys[j] != -1)
				{
					if (j > 0)
						strcat(keystring, " or ");
					strcat(keystring, Key_KeynumToString (keys[j]));
				}
			}
		}
		M_Print (150, y, keystring);
#endif
	}

	if (bind_grab)
		M_DrawCharacter (130, 48 + keys_cursor*8, '=');
	else
		M_DrawCharacter (130, 48 + keys_cursor*8, 12+((int)(realtime*4)&1));
}


void M_Keys_Key (int k)
{
	char	cmd[80];
	int		keys[2];

	if (bind_grab)
	{	// defining a key
		S_LocalSound ("misc/menu1.wav");
		if (k == K_ESCAPE)
		{
			bind_grab = false;
		}
		else if (k != '`')
		{
			sprintf (cmd, "bind \"%s\" \"%s\"\n", Key_KeynumToString (k), bindnames[keys_cursor][0]);
			Cbuf_InsertText (cmd);
		}

		bind_grab = false;
		return;
	}

	switch (k)
	{
	case K_ESCAPE:
		M_Menu_Options_f ();
		break;

	case K_LEFTARROW:
	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		keys_cursor--;
		if (keys_cursor < 0)
			keys_cursor = NUMCOMMANDS-1;
		break;

	case K_DOWNARROW:
	case K_RIGHTARROW:
		S_LocalSound ("misc/menu1.wav");
		keys_cursor++;
		if (keys_cursor >= NUMCOMMANDS)
			keys_cursor = 0;
		break;

	case K_ENTER:		// go into bind mode
		M_FindKeysForCommand (bindnames[keys_cursor][0], keys);
		S_LocalSound ("misc/menu2.wav");
		if (keys[1] != -1)
			M_UnbindCommand (bindnames[keys_cursor][0]);
		bind_grab = true;
		break;

	case K_BACKSPACE:		// delete bindings
	case K_DEL:				// delete bindings
		S_LocalSound ("misc/menu2.wav");
		M_UnbindCommand (bindnames[keys_cursor][0]);
		break;
	}
}

/*
==========
Sound Menu
==========
*/

int	sound_cursor;
#define	SOUND_ITEMS	2

void M_Menu_Sound_f (void)
{
	key_dest = key_menu;
	m_state = m_sound;
	m_entersound = true;
}

#if 0 //Cheap fix
void M_SoundAdjustSliders (int dir)
{
	float v;

	S_LocalSound ("misc/menu3.wav");

	switch (sound_cursor)
	{
        case 0: // music volume CheapAlert - I fixed it
                if (bgmvolume.value < 0.0f)
                        bgmvolume.value = 0.0f; // totally muted
                if (bgmvolume.value > 1.0f)
                        bgmvolume.value = 1.0f;
                Cvar_SetValue ("bgmvolume", bgmvolume.value);
		break;

        case 1: // sfx volume CheapAlert - I fixed it
//                volume.value += dir * 0.05f;
                if (volume.value < 0.0f)
                        volume.value = 0.0f; // totally muted
                if (volume.value > 1.0f)
                        volume.value = 1.0f;
                Cvar_SetValue ("volume", volume.value);
		break;
		default:
			break;
	}
}


#else
void M_SoundAdjustSliders (int dir)
{
	S_LocalSound ("misc/menu3.wav");

	switch (sound_cursor)
	{
	case 0:	// music volume
		bgmvolume.value += dir * 0.05f;

		if (bgmvolume.value < 0)
			bgmvolume.value = 0;
		if (bgmvolume.value > 1)
			bgmvolume.value = 1;

		 Cvar_SetValue ("bgmvolume", bgmvolume.value);
		break;

	case 1:	// sfx volume
                volume.value += dir * 0.05f;
                if (volume.value < 0.0f)
                        volume.value = 0.0f; // totally muted
                if (volume.value > 1.0f)
                        volume.value = 1.0f;
                Cvar_SetValue ("volume", volume.value);
		break;

	case 2:	// mp3 or cd
		break;

	case 3: // samplerate
		break;

	case 4: // restart
		break;
	}
}
#endif

void M_Sound_Draw () 
{
	float	r;

	M_Print (16, 32, "       CD Music Volume");
	r = bgmvolume.value;
	M_DrawSlider (220, 32, r);

	M_Print (16, 40, "          Sound Volume");
	r = volume.value;
	M_DrawSlider (220, 40, r);

	//M_Print (16, 48, "      Sound Samplerate");

	//if (samplerate.value == 0)
	//	M_Print (220, 48, "11025");
	//else if (samplerate.value == 1)
	//	M_Print (220, 48, "22050");
	//else if (samplerate.value == 2)
	//	M_Print (220, 48, "custom");

	//M_Print (16, 56, "         Restart Sound");

	M_DrawCharacter (200, 32 + sound_cursor*8, 12+((int)(realtime*4)&1));
}

void M_Sound_Key (int key)
{
	switch (key)
	{
	case K_ESCAPE:
		M_Menu_Options_f ();
		break;

	case K_ENTER:
		m_entersound = true;
		M_SoundAdjustSliders (1);
		return;

	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		if (--sound_cursor < 0)
			sound_cursor = SOUND_ITEMS-1;
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menu1.wav");
		if (++sound_cursor >= SOUND_ITEMS)
			sound_cursor = 0;
		break;

	case K_LEFTARROW:
		M_SoundAdjustSliders (-1);
		break;

	case K_RIGHTARROW:
		M_SoundAdjustSliders (1);
		break;
	}
}

/*
==========
Video Menu
==========
*/

extern cvar_t gl_geocaustics;
extern cvar_t r_autolava;
extern cvar_t r_autotele;
extern cvar_t r_autofogwater;
extern cvar_t extra_fx;
extern cvar_t gl_particle_limiter;
extern cvar_t cl_bobq2;
extern cvar_t sv_skyflattern;
extern cvar_t sv_skyspeedscale;
extern cvar_t sv_skydim;
extern cvar_t sv_skyvalue;


int		video_cursor;
#define	VIDEO_ITEMS	29

void M_Menu_Video_f () 
{
	key_dest = key_menu;
	m_state = m_video;
	m_entersound = true;
}

void M_VideoAdjustSliders (int dir)
{
	S_LocalSound ("misc/menu3.wav");

	switch (video_cursor)
	{
	case 0:	// screen size
		scr_viewsize.value += dir * 10;
		if (scr_viewsize.value < 30)
			scr_viewsize.value = 30;
		if (scr_viewsize.value > 120)
			scr_viewsize.value = 120;
		Cvar_SetValue ("viewsize", scr_viewsize.value);
		break;

	case 1:	// brightness
		brightness.value -= dir * 0.05;
		if (brightness.value < 0.5)
			brightness.value = 0.5;
		if (brightness.value > 1)
			brightness.value = 1;

		Cvar_SetValue ("brightness", brightness.value);
		break;

	case 2:		// shadows
		Cvar_SetValue ("r_shadows", !r_shadows.value);
		break;

	case 3:		// water alpha
		r_wateralpha.value += dir * 0.05; 
		if (r_wateralpha.value < 0)
			r_wateralpha.value = 0;
		if (r_wateralpha.value > 1)
			r_wateralpha.value = 1;
		Cvar_SetValue ("r_wateralpha", r_wateralpha.value);
		break;

	case 4:		// console alpha
		con_alpha.value += dir * 0.05; 
		if (con_alpha.value < 0)
			con_alpha.value = 0;
		if (con_alpha.value > 1)
			con_alpha.value = 1;
		Cvar_SetValue ("con_alpha", con_alpha.value);
		break;

	case 5:		// sbar alpha
		sbar_alpha.value += dir * 0.05; 
		if (sbar_alpha.value < 0)
			sbar_alpha.value = 0;
		if (sbar_alpha.value > 1)
			sbar_alpha.value = 1;
		Cvar_SetValue ("sbar_alpha", sbar_alpha.value);
		break;

	case 6:	// Glows
		Cvar_SetValue ("gl_glows", !gl_glows.value);
		break;

	case 7:	// Fullbrights
		Cvar_SetValue ("gl_fbr", !gl_fbr.value);
		break;

	case 8:	// Enviroment Mapping
		Cvar_SetValue ("gl_envmap", !gl_envmap.value);
		break;

	case 9:	// Underwater Caustics
		Cvar_SetValue ("gl_caustics", !gl_caustics.value);
		break;

	case 10:	// Fading CenterPrints
		Cvar_SetValue ("centerfade", !centerfade.value);
		break;

	case 11:		// fog
		Cvar_SetValue ("gl_fogenable", !gl_fogenable.value);
		break;

	case 12:	// fog start
		gl_fogstart.value += dir * 50.0f;
		if (gl_fogstart.value < 0.0f)
			gl_fogstart.value = 0.0f;
		if (gl_fogstart.value > 4096.0f)
			gl_fogstart.value = 4096.0f;
		if ((gl_fogend.value - gl_fogstart.value) < 500)
			 gl_fogstart.value = gl_fogend.value - 500;
		Cvar_SetValue ("gl_fogstart", gl_fogstart.value);
		break;

	case 13:	// fog end
		gl_fogend.value += dir * 50.0f;
		if (gl_fogend.value < 500.0f)
			gl_fogend.value = 500.0f;
		if (gl_fogend.value > 4096.0f)
			gl_fogend.value = 4096.0f;
		if ((gl_fogend.value - gl_fogstart.value) < 500)
			 gl_fogend.value = gl_fogstart.value + 500;
		Cvar_SetValue ("gl_fogend", gl_fogend.value);
		break;

	case 14:	// fog red
		gl_fogred.value += dir * 0.05f;
		if (gl_fogred.value < 0.0f)
			gl_fogred.value = 0.0f;
		if (gl_fogred.value > 1.0f)
			gl_fogred.value = 1.0f;
		Cvar_SetValue ("gl_fogred", gl_fogred.value);
		break;

	case 15:	// fog green 
		gl_foggreen.value += dir * 0.05f;
		if (gl_foggreen.value < 0.0f)
			gl_foggreen.value = 0.0f;
		if (gl_foggreen.value > 1.0f)
			gl_foggreen.value = 1.0f;
		Cvar_SetValue ("gl_foggreen", gl_foggreen.value);
		break;

	case 16:	// fog blue
		gl_fogblue.value += dir * 0.05f;
		if (gl_fogblue.value < 0.0f)
			gl_fogblue.value = 0.0f;
		if (gl_fogblue.value > 1.0f)
			gl_fogblue.value = 1.0f;
		Cvar_SetValue ("gl_fogblue", gl_fogblue.value);
		break;

	case 17:	// Crosshair graphic
		crosshair.value += dir * 1.0f;
		if (crosshair.value < 0.0f)
			crosshair.value = 0.0f;
		if (crosshair.value > 10.0f)
			crosshair.value = 10.0f;
		Cvar_SetValue ("crosshair", crosshair.value);
		break;
	case 18:		
		Cvar_SetValue ("gl_geocaustics", !gl_geocaustics.value);
		break;
	case 19:		
		Cvar_SetValue ("r_autolava", !r_autolava.value);
		break;
	case 20:		
		Cvar_SetValue ("r_autotele", !r_autotele.value);
		break;
	case 21:		
		Cvar_SetValue ("r_autofogwater", !r_autofogwater.value);
		break;

	case 22:	
		extra_fx.value += dir * 1.0f;
		if (extra_fx.value < 0.0f)
			extra_fx.value = 0.0f;
		if (extra_fx.value > 10.0f)
			extra_fx.value = 10.0f;
		Cvar_SetValue ("extra_fx", extra_fx.value);
		break;

	case 23:	
		gl_particle_limiter.value += dir * 0.5f;
		if (gl_particle_limiter.value < 0.0f)
			gl_particle_limiter.value = 0.0f;
		if (gl_particle_limiter.value > 20.0f)
			gl_particle_limiter.value = 20.0f;
		Cvar_SetValue ("gl_particle_limiter", gl_particle_limiter.value);
		break;
	case 24:	
		cl_bobq2.value += dir * 0.5f;
		if (cl_bobq2.value < 0.0f)
			cl_bobq2.value = 0.0f;
		if (cl_bobq2.value > 4.0f)
			cl_bobq2.value = 4.0f;
		Cvar_SetValue ("cl_bobq2", cl_bobq2.value);
		break;
	
	case 25:	
		sv_skyflattern.value += dir * 0.5f;
		if (sv_skyflattern.value < 0.0f)
			sv_skyflattern.value = 0.0f;
		if (sv_skyflattern.value > 64.0f)
			sv_skyflattern.value = 64.0f;
		Cvar_SetValue ("sv_skyflattern", sv_skyflattern.value);
		break;		

	case 26:	
		sv_skyspeedscale.value += dir * 0.5f;
		if (sv_skyspeedscale.value < 0.0f)
			sv_skyspeedscale.value = 0.0f;
		if (sv_skyspeedscale.value > 64.0f)
			sv_skyspeedscale.value = 64.0f;
		Cvar_SetValue ("sv_skyspeedscale", sv_skyspeedscale.value);
		break;	

	case 27:	
		sv_skydim.value += dir * 0.5f;
		if (sv_skydim.value < 0.0f)
			sv_skydim.value = 0.0f;
		if (sv_skydim.value > 64.0f)
			sv_skydim.value = 64.0f;
		Cvar_SetValue ("sv_skydim", sv_skydim.value);
		break;		
		
	case 28:	
		sv_skyvalue.value += dir * 0.5f;
		if (sv_skyvalue.value < 0.0f)
			sv_skyvalue.value = 0.0f;
		if (sv_skyvalue.value > 64.0f)
			sv_skyvalue.value = 64.0f;
		Cvar_SetValue ("sv_skyvalue", sv_skyvalue.value);
		break;
	}
}


extern cvar_t temp1;

void M_Video_Draw () 
{
	float	r;
	int t;

	M_Print (16, 32, "           Screen size");
	r = (scr_viewsize.value - 30) * 0.0111111111;
	M_DrawSlider (220, 32, r);

	M_Print (16, 40, "            Brightness");
	r = (1.0 - brightness.value) * 2;
	M_DrawSlider (220, 40, r);

	M_Print (16, 48,"               Shadows");
	M_DrawCheckbox (220, 48, r_shadows.value);

	M_Print (16, 56, "           Water Alpha");
	r = r_wateralpha.value;
	M_DrawSlider (220, 56, r);

	M_Print (16, 64, "         Console Alpha");
	r = con_alpha.value;
	M_DrawSlider (220, 64, r);

	M_Print (16, 72, "      Status Bar Alpha");
	r = sbar_alpha.value;
	M_DrawSlider (220, 72, r);

	M_Print (16, 80, "                 Glows");
	M_DrawCheckbox (220, 80, gl_glows.value);

	M_Print (16, 88, "           Fullbrights");
	M_DrawCheckbox (220, 88, gl_fbr.value);

	M_Print (16, 96, "    Enviroment Mapping");
	M_DrawCheckbox (220, 96, gl_envmap.value);

	M_Print (16, 104,"   Underwater Caustics");
	M_DrawCheckbox (220, 104, gl_caustics.value);

	M_Print (16, 112,"   Fading CenterPrints");
	M_DrawCheckbox (220, 112, centerfade.value);

	M_Print (16, 120,"                   Fog");
	M_DrawCheckbox (220, 120, gl_fogenable.value);

	M_Print (16, 128,"             Fog Start");
	r = gl_fogstart.value * 0.000244140625;	// Tomaz - Speed
	M_DrawSlider (220, 128, r);	

	M_Print (16, 136,"               Fog End");
	r = gl_fogend.value * 0.000244140625;	// Tomaz - Speed
	M_DrawSlider (220, 136, r);

	M_Print (16, 144,"               Red Fog");

	M_Print (16, 152,"             Green Fog");

	M_Print (16, 160,"              Blue Fog");

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);	

	r = gl_fogred.value;
	glColor3f(1,0,0);
	M_DrawSlider (220, 144, r);

	r = gl_foggreen.value;
	glColor3f(0,1,0);
	M_DrawSlider (220, 152, r);

	r = gl_fogblue.value;
	glColor3f(0,0,1);
	M_DrawSlider (220, 160, r);
	glColor3f(1,1,1);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	M_Print (16, 168,"             Crosshair");
	r = crosshair.value * 0.1;	// Tomaz - Speed
	M_DrawSlider (220, 168, r);

	M_DrawCharacter (200, 32 + video_cursor*8, 12+((int)(realtime*4)&1));

	M_Print (16, 176,"            GeoCaustic");
	M_DrawCheckbox (220, 176, gl_geocaustics.value);

	t= 176;

	t+=8;
	M_Print (16, t,"          AutoLavaFire");
	M_DrawCheckbox (220, t, r_autolava.value);

	t+=8;
	M_Print (16, t,"          AutoTeleport");
	M_DrawCheckbox (220, t, r_autotele.value);

	t+=8;
	M_Print (16, t,"          AutoFogwater");
	M_DrawCheckbox (220, t, r_autofogwater.value);

	t+=8;
	M_Print (16, t,"         ExtraFX Level");
	r = extra_fx.value * 0.1;
	M_DrawSlider (220, t, r);

	t+=8;
	M_Print (16, t,"   Particle Near Limit");
	r = gl_particle_limiter.value * 0.1*0.5;
	M_DrawSlider (220, t, r);

	t+=8;
	M_Print (16, t,"   Weapon Bobing Style");
	r = cl_bobq2.value * 0.01;
	M_DrawSlider (220, t, cl_bobq2.value);

	t+=8;
	M_Print (16, t,"              Sky Flat");
	r = sv_skyflattern.value * 0.01;
	M_DrawSlider (220, t, sv_skyflattern.value);

	t+=8;
	M_Print (16, t,"             Sky Speed");
	r = sv_skyspeedscale.value * 0.01;
	M_DrawSlider (220, t, sv_skyspeedscale.value);

	t+=8;
	M_Print (16, t,"              Sky Size");
	r = sv_skydim.value * 0.01;
	M_DrawSlider (220, t, sv_skydim.value);

	t+=8;
	M_Print (16, t,"              Sky Slow");
	r = sv_skyvalue.value * 0.01;
	M_DrawSlider (220, t, sv_skyvalue.value);

}

void M_Video_Key (int key) 
{
	switch (key)
	{
	case K_ESCAPE:
		M_Menu_Options_f ();
		break;

	case K_ENTER:
		m_entersound = true;
		M_VideoAdjustSliders (1);
		return;

	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		if (--video_cursor < 0)
			video_cursor = VIDEO_ITEMS-1;
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menu1.wav");
		if (++video_cursor >= VIDEO_ITEMS)
			video_cursor = 0;
		break;

	case K_LEFTARROW:
		M_VideoAdjustSliders (-1);
		break;

	case K_RIGHTARROW:
		M_VideoAdjustSliders (1);
		break;
	}
}

/*
=========
Misc Menu
=========
*/

int		misc_cursor;
#define	MISC_ITEMS	9

void M_Menu_Misc_f ()
{
	key_dest = key_menu;
	m_state = m_misc;
	m_entersound = true;
}

void M_MiscAdjustSliders (int dir)
{
	S_LocalSound ("misc/menu3.wav");

	switch (misc_cursor)
	{
	case 0:	// improved aiming
		Cvar_SetValue ("impaim", !impaim.value);
		break;

	case 1:		// chase mode
		Cvar_SetValue ("chase_active", !chase_active.value);
		break;
	case 2:		// clear our of bounds
		Cvar_SetValue ("gl_clear", !gl_clear.value);
		break;
	case 3:		// show weapon
		Cvar_SetValue ("r_drawviewmodel", !r_drawviewmodel.value);
		break;
	
	case 4:	// Mouse Look
		Cvar_SetValue ("in_mlook", !in_mlook.value);
		break;

	case 5:	// Fps Counter
		Cvar_SetValue ("show_fps", !show_fps.value);
		break;

	case 6:	// Bobbing Items
		Cvar_SetValue ("r_bobbing", !r_bobbing.value);
		break;

	case 7:	// Wavesize
		r_wave.value += dir * 1.0f;
		if (r_wave.value < 0.0f)
			r_wave.value = 0.0f;
		if (r_wave.value > 20.0f)
			r_wave.value = 20.0f;
		Cvar_SetValue ("r_wave", r_wave.value);
		break;

	case 8:// MapShots
		Cvar_SetValue ("mapshots", !mapshots.value);
		break;
	}
}

void M_Misc_Draw () 
{
	float	r;

	M_Print (16, 32, "       Improved Aiming");
	M_DrawCheckbox (220, 32, impaim.value);

	M_Print (16, 40, "          Chase Camera");
	M_DrawCheckbox (220, 40, chase_active.value);

	M_Print (16, 48, "       Clear Clip Area");
	M_DrawCheckbox (220, 48, gl_clear.value);

	M_Print (16, 56, "           Show Weapon");
	M_DrawCheckbox (220, 56, r_drawviewmodel.value);

	M_Print (16, 64, "            Mouse Look");
	M_DrawCheckbox (220, 64, in_mlook.value);

	M_Print (16, 72, "           Fps Counter");
	M_DrawCheckbox (220, 72, show_fps.value);

	M_Print (16, 80, "         Boobing Items");//not a FrikaC spell error 
	M_DrawCheckbox (220, 80, r_bobbing.value);

	M_Print (16, 88, "             Wave Size");
	r = r_wave.value * 0.05;	// Tomaz - Speed
	M_DrawSlider (220, 88, r);

	M_Print (16, 96, "              MapShots");
	M_DrawCheckbox (220, 96, mapshots.value);

	M_DrawCharacter (200, 32 + misc_cursor*8, 12+((int)(realtime*4)&1));
}
void M_Misc_Key (int key)
{
	switch (key)
	{
	case K_ESCAPE:
		M_Menu_Options_f ();
		break;

	case K_ENTER:
		m_entersound = true;
		M_MiscAdjustSliders (1);
		return;

	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		if (--misc_cursor < 0)
			misc_cursor = MISC_ITEMS-1;
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menu1.wav");
		if (++misc_cursor >= MISC_ITEMS)
			misc_cursor = 0;
		break;

	case K_LEFTARROW:
		M_MiscAdjustSliders (-1);
		break;

	case K_RIGHTARROW:
		M_MiscAdjustSliders (1);
		break;
	}
}

// Tei class select

/*
=========
Misc2 Menu
=========
*/

int		misc2_cursor;
#define	MISC2_ITEMS	8

void M_Menu_Misc2_f ()
{
	key_dest = key_menu;//keys get the menu
	m_state = m_misc2;
	m_entersound = true;
}

void M_Misc2AdjustSliders (int dir)
{
	S_LocalSound ("misc/menu3.wav");
}

void M_Misc2_Draw () 
{

	qpic_t	*p;

	if (misc2_cursor == 0)
		p = Draw_CachePic ("gfx/mech/mech3.lmp");
	else
	if (misc2_cursor == 1)
		p = Draw_CachePic ("gfx/mech/spider3.lmp");
	else
	if (misc2_cursor == 2)
		p = Draw_CachePic ("gfx/mech/fallen3.lmp");
	else
	if (misc2_cursor == 3)
		p = Draw_CachePic ("gfx/mech/harv3.lmp");
	else
	if (misc2_cursor == 4)
		p = Draw_CachePic ("gfx/mech/zepe3.lmp");
	else
	if (misc2_cursor == 5)
		p = Draw_CachePic ("gfx/mech/herc3.lmp");
	else
	if (misc2_cursor == 6)
		p = Draw_CachePic ("gfx/mech/mamu3.lmp");
	else
	if (misc2_cursor == 7)
		p = Draw_CachePic ("gfx/mech/heav3.lmp");
	else {
		p = Draw_CachePic ("gfx/mech/spider3.lmp");
		misc2_cursor = 0;
	}

	M_Print (16, 22, ".:: Select your Class ::.");
	M_DrawPic ( (320-p->width)/2, 60, p);

}

void M_Misc2_Key (int key)
{
	switch (key)
	{
	case K_ESCAPE:
		key_dest = key_game;
		m_state = m_none;
		break;

	case K_ENTER:
		m_entersound = true;
		key_dest = key_game;
		m_state = m_none;

		if (misc2_cursor==0) 
			Cbuf_AddText ("impulse 33");//Exec changeclass
		else
		if (misc2_cursor==1) 
			Cbuf_AddText ("impulse 34");//Exec changeclass
		else
		if (misc2_cursor==2) 
			Cbuf_AddText ("impulse 35");//Exec changeclass
		else
		if (misc2_cursor==3) 
			Cbuf_AddText ("impulse 36");//Exec changeclass
		else
		if (misc2_cursor==4) 
			Cbuf_AddText ("impulse 37");//Exec changeclass
		else
		if (misc2_cursor==5) 
			Cbuf_AddText ("impulse 38");//Exec changeclass
		else
		if (misc2_cursor==6) 
			Cbuf_AddText ("impulse 39");//Exec changeclass
		else
			Cbuf_AddText ("impulse 40");//Exec changeclass
		return;

	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		if (--misc2_cursor < 0)
			misc2_cursor = MISC2_ITEMS-1;
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menu1.wav");
		if (++misc2_cursor >= MISC2_ITEMS)
			misc2_cursor = 0;
		break;

	case K_LEFTARROW:
		M_Misc2AdjustSliders (-1);
		break;

	case K_RIGHTARROW:
		M_Misc2AdjustSliders (1);
		break;
	}
}

// Tei class select


//-----------------------------------------------------------------------
//
// LocalMenus editor
//  


/*
============
LocalMenus Menu
============
*/

extern cvar_t menu_0;
extern cvar_t menu_1;
extern cvar_t menu_2;
extern cvar_t menu_3;
extern cvar_t menu_4;
extern cvar_t menu_5;
extern cvar_t menu_6;
extern cvar_t menu_7;
extern cvar_t menu_8;
extern cvar_t menu_9;

extern cvar_t xmenu_0;
extern cvar_t xmenu_1;
extern cvar_t xmenu_2;
extern cvar_t xmenu_3;
extern cvar_t xmenu_4;
extern cvar_t xmenu_5;
extern cvar_t xmenu_6;
extern cvar_t xmenu_7;
extern cvar_t xmenu_8;
extern cvar_t xmenu_9;

extern cvar_t menu_items;
extern cvar_t menu_image;
extern cvar_t menu_select;


static int		LocalMenus_cursor;

static char tmpString[8192];
static qboolean modedit;
static cvar_t *editing = 0;

#define	LocalMenus_ITEMS	22

void M_Menu_LocalMenus_f (void)
{
	key_dest = key_menu;
	m_state = m_LocalMenus;
	m_entersound = true;
	//editing = NULL;
	Cvar_Set("menu_mode","1");
}


void M_DrawTextBoxColor (float x, float y, int maxlen, int lines, float red, float green, float blue)
{

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);	

	glColor3f(red,green,blue);
	M_DrawTextBox (x, y, maxlen, lines);
	glColor3f(1,1,1);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
};


#define YMENUS 0
#define BASEX 0
#define SIZEBOX 40
#define CURSOROFFSET (-8)
#define STROFFSET (8*8)
#define YOFFSET 32

#define MPRINT(stringie)	M_Print (BASEX,y+=YOFFSET,stringie);M_DrawTextBoxColor (BASEX+6*8, y-8, SIZEBOX, 1,0.6f,0.6f,0.6f);

#define XMPRINT(stringie)	M_Print (BASEX, y+=YOFFSET,stringie);M_DrawTextBoxColor (BASEX+6*8, y-8, SIZEBOX, 1,0.2f,0.2f,0.2f);

#define ZMPRINT(stringie)	M_Print (BASEX+STROFFSET, y+=YOFFSET,stringie.string);//M_DrawTextBox (BASEY+6*8, y-8, SIZEBOX, 1);


void M_LocalMenus_Draw (void)
{
	qpic_t	*p;
	int y = YMENUS;

	//M_DrawPic (16, 4, Draw_CachePic ("gfx/qplaque.lmp") );
	p = Draw_CachePic ("gfx/ttl_cstm.lmp");
	M_DrawPic ( (320-p->width)/2, 4, p);

	y = YMENUS;

	MPRINT("Title");
	MPRINT("Menu1");
	MPRINT("Menu2");
	MPRINT("Menu3");
	MPRINT("Menu4");
	MPRINT("Menu5");
	MPRINT("Menu6");
	MPRINT("Menu7");
	MPRINT("Menu8");
	MPRINT("Menu9");

	y = YMENUS + YOFFSET/2;
	
	XMPRINT("img");
	XMPRINT(" x1");
	XMPRINT(" x2");
	XMPRINT(" x3");
	XMPRINT(" x4");
	XMPRINT(" x5");
	XMPRINT(" x6");
	XMPRINT(" x7");
	XMPRINT(" x8");
	XMPRINT(" x9");

	y = YMENUS;

	ZMPRINT(menu_0);
	ZMPRINT(menu_1);
	ZMPRINT(menu_2);
	ZMPRINT(menu_3);
	ZMPRINT(menu_4);
	ZMPRINT(menu_5);
	ZMPRINT(menu_6);
	ZMPRINT(menu_7);
	ZMPRINT(menu_8);
	ZMPRINT(menu_9);

	y = YMENUS + YOFFSET/2;

	ZMPRINT(menu_image);
	ZMPRINT(xmenu_1);
	ZMPRINT(xmenu_2);
	ZMPRINT(xmenu_3);
	ZMPRINT(xmenu_4);
	ZMPRINT(xmenu_5);
	ZMPRINT(xmenu_6);
	ZMPRINT(xmenu_7);
	ZMPRINT(xmenu_8);
	ZMPRINT(xmenu_9);
	
	M_Print (BASEX,y+=YOFFSET/2,"Continue Game");
	M_Print (BASEX,y+=YOFFSET/2,"Save as 'new.menu.txt' and continue.");

	M_DrawCharacter (BASEX + CURSOROFFSET, YMENUS+ (LocalMenus_cursor+2)*(YOFFSET/2), 12+((int)(realtime*4)&1));
	
	if (modedit)
	{
		M_DrawCharacter (BASEX +STROFFSET+ 8*strlen(tmpString), YMENUS+ (LocalMenus_cursor+2)*(YOFFSET/2), 10+((int)(realtime*4)&1));
		M_Print (BASEX+STROFFSET,YMENUS+ (LocalMenus_cursor+2)*(YOFFSET/2),tmpString);	
	}

}

void SaveLocalMenuConfig (void)
{
	FILE	*f;
	char	filename[128];

	//f = fopen (va("%s/maps/%s.cfg",com_gamedir, sv.name), "w");
		sprintf(filename, "%s/%s.menu.txt", com_gamedir, "new");
		Con_Printf("%s--\n", filename);
		f = fopen (filename, "w");
		if (!f)
		{
			Con_Printf ("\nCouldn't save edit settings.\n");
			return;
		}
		Con_Printf ("\nSaved edit settings.\n");


		fprintf (f, "// Autogenerated Menu\n\nmenu mode 1;menu_clear\n");

		fprintf (f, "menu_0		\"%s\"\n" ,	menu_0.string	);
		fprintf (f, "menu_image		\"%s\"\n" ,	menu_image.string	);


		fprintf (f, "menu_1		\"%s\"\n" ,	menu_1.string	);
		fprintf (f, "xmenu_1		\"%s\"\n" ,	xmenu_1.string	);

		fprintf (f, "menu_2		\"%s\"\n" ,	menu_2.string	);
		fprintf (f, "xmenu_2		\"%s\"\n" ,	xmenu_2.string	);
		fprintf (f, "menu_3		\"%s\"\n" ,	menu_3.string	);
		fprintf (f, "xmenu_3		\"%s\"\n" ,	xmenu_3.string	);
		fprintf (f, "menu_4		\"%s\"\n" ,	menu_4.string	);
		fprintf (f, "xmenu_4		\"%s\"\n" ,	xmenu_4.string	);
		fprintf (f, "menu_5		\"%s\"\n" ,	menu_5.string	);
		fprintf (f, "xmenu_5		\"%s\"\n" ,	xmenu_5.string	);
		fprintf (f, "menu_6		\"%s\"\n" ,	menu_6.string	);
		fprintf (f, "xmenu_6		\"%s\"\n" ,	xmenu_6.string	);
		fprintf (f, "menu_7		\"%s\"\n" ,	menu_7.string	);
		fprintf (f, "xmenu_7		\"%s\"\n" ,	xmenu_7.string	);
		fprintf (f, "menu_8		\"%s\"\n" ,	menu_8.string	);
		fprintf (f, "xmenu_8		\"%s\"\n" ,	xmenu_8.string	);
		fprintf (f, "menu_9		\"%s\"\n" ,	menu_9.string	);
		fprintf (f, "xmenu_9		\"%s\"\n" ,	xmenu_9.string	);

		fprintf (f, "menu_items		\"%s\"\n" ,	menu_items.string	);

		fprintf (f, "bindefault menu; menu_select 0 \n");

		fclose (f);
}


void M_LocalMenus_Key (int k)
{
	int l;
	qboolean docopy = false;

	switch (k)
	{
	case K_ESCAPE:
		M_Menu_Main_f ();
		break;

	case K_ENTER:
		m_entersound = true;
		if (!modedit)
		{
			modedit = true;
			docopy = true;
		}
		else
		{
			modedit = false;
		}

		if(menu_items.value < LocalMenus_cursor /2)
		{
			Cvar_Set("menu_items",va("%d",LocalMenus_cursor/2));
			if (menu_select.value == 0)
			{
				Cvar_Set("menu_select",va("%d",LocalMenus_cursor/2));
			}
		}

		switch (LocalMenus_cursor)
		{
		case 0:
#define UPDATEC(menu_is)	editing = &menu_is;if (docopy) strcpy(tmpString,editing->string);else Cvar_Set(editing->name,tmpString);				

			UPDATEC(menu_0);
			break;
		case 1:	UPDATEC(menu_image);	break;
		case 2:	UPDATEC(menu_1);	break;
		case 3:	UPDATEC(xmenu_1);	break;
		case 4:	UPDATEC(menu_2);	break;
		case 5:	UPDATEC(xmenu_2);	break;
		case 6:	UPDATEC(menu_3);	break;
		case 7:	UPDATEC(xmenu_3);	break;
		case 8:	UPDATEC(menu_4);	break;
		case 9:	UPDATEC(xmenu_4);	break;
		case 10:	UPDATEC(menu_5);	break;
		case 11:	UPDATEC(xmenu_5);	break;
		case 12:	UPDATEC(menu_6);	break;
		case 13:	UPDATEC(xmenu_6);	break;
		case 14:	UPDATEC(menu_7);	break;
		case 15:	UPDATEC(xmenu_7);	break;
		case 16:	UPDATEC(menu_8);	break;
		case 17:	UPDATEC(xmenu_8);	break;
		case 18:	UPDATEC(menu_9);	break;
		case 19:	UPDATEC(xmenu_9);	break;
		case 21:
			SaveLocalMenuConfig();
		case 20:
			key_dest = key_game;
			m_state = m_none;
			//LocalMenus_cursor = 0;
			tmpString[0]=0;
			modedit = false;			
		default:
			break;
		}
		return;

	case K_UPARROW:
		S_LocalSound ("misc/menu1.wav");
		LocalMenus_cursor--;
		if (LocalMenus_cursor < 0)
			LocalMenus_cursor = LocalMenus_ITEMS-1;
		editing = NULL;
		break;

	case K_DOWNARROW:
		S_LocalSound ("misc/menu1.wav");
		LocalMenus_cursor++;
		if (LocalMenus_cursor >= LocalMenus_ITEMS)
			LocalMenus_cursor = 0;
		editing = NULL;
		break;
	case K_BACKSPACE:
		if (strlen(tmpString))
				tmpString[strlen(tmpString)-1] = 0;
		if (editing)
		{
			Cvar_Set(editing->name,tmpString);
		}
		break;
	default:
		if (k<32 || k>=132)
			break;
		//Con_Printf("car: %d\n",k);
		l = strlen(tmpString);
		if (l < 21)
		{
				tmpString[l+1] = 0;
				tmpString[l] = k;
		}
	}
}

