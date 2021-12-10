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
// r_misc.c

#include "quakedef.h"
#include "gl_mirror.h"

void R_InitParticles (void);
void R_ClearParticles ();
void GL_BuildLightmaps (void);
void SHOWLMP_clear();

byte	dottexture[8][8] =
{
	{0,1,1,0,0,0,0,0},
	{1,1,1,1,0,0,0,0},
	{1,1,1,1,0,0,0,0},
	{0,1,1,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
};

/*
==================
R_InitTextures
==================
*/
void	R_InitTextures (void)
{
	int		x,y, m;
	byte	*dest;



// create a simple checkerboard texture for the default
	r_notexture_mip = Hunk_AllocName (sizeof(texture_t) + 16*16+8*8+4*4+2*2, "notexture");
	


	r_notexture_mip->width = r_notexture_mip->height = 16;
	r_notexture_mip->offsets[0] = sizeof(texture_t);
	r_notexture_mip->offsets[1] = r_notexture_mip->offsets[0] + 16*16;
	r_notexture_mip->offsets[2] = r_notexture_mip->offsets[1] + 8*8;
	r_notexture_mip->offsets[3] = r_notexture_mip->offsets[2] + 4*4;
	
	for (m=0 ; m<4 ; m++)
	{
		dest = (byte *)r_notexture_mip + r_notexture_mip->offsets[m];
		for (y=0 ; y< (16>>m) ; y++)
			for (x=0 ; x< (16>>m) ; x++)
			{
				if (  (y< (8>>m) ) ^ (x< (8>>m) ) )
					*dest++ = 0;
				else
					*dest++ = 0xff;
			}
	}	
}

/*
===============
R_Init
===============
*/


#if 1
	//TELEJANO

extern cvar_t r_staticfx;//Tei staticfx


//Tei sun
extern cvar_t gl_sun_x;
extern cvar_t gl_sun_y;
extern cvar_t gl_sun_z;
extern cvar_t gl_sun;
//Tei sun

extern cvar_t gl_xrayblast; //Tei textures theme
extern cvar_t gl_smallparticles; //Tei hehehe... nearest hack (partiCulicàas!)
extern cvar_t gl_themefolder;//Tei textures theme
extern cvar_t gl_decalfolder;//Tei decal folder

extern cvar_t gl_detail;//CHP detail textures
extern cvar_t gl_fontalt;//Tei alternate font

extern cvar_t gl_detailscale;//Tei detail scale 

extern cvar_t gl_dither;//Tei force dither


//Tei autozone
cvar_t  zone_foggreen			= {"zone_foggreen","0", true}; 
cvar_t  zone_fogblue			= {"zone_fogblue","0", true};
cvar_t  zone_fogred			= {"zone_fogred","0", true};
cvar_t  zone_speed				= {"zone_speed","0", true};
cvar_t  zone_enable				= {"zone_enable","0", true};
cvar_t  zone_start				= {"zone_start","1", true};
cvar_t  zone_end				= {"zone_end","2048", true};
//Tei autozone

cvar_t  r_ambient				= {"r_ambient","2", true};
cvar_t  r_ambient2				= {"r_ambient2","3", true};

cvar_t  extra_fx				= {"extra_fx","1", true};


#endif


#if 1 //Telejano RC2
//Tei autozone
void Cmd_LoadZone_f (void)
{
	char	*f;
	int		mark;
	char	zonefile[512];

	
	if (Cmd_Argc () != 2)
	{
		Con_Printf ("loadzone <zone> : load a zone definition file\n");		
		return;
	}

	sprintf(zonefile,"zones/%s.cfg",Cmd_Argv(1));

	mark = Hunk_LowMark ();
	f = (char *)COM_LoadHunkFile (zonefile);
	if (!f)
	{
		Con_Printf ("couldn load zonefile %s\n",zonefile);
		return;
	}

	// Tei Mute scripting
	Con_DPrintf ("execing %s\n",Cmd_Argv(1));
	// Tei Mute scripting

	Cbuf_InsertText (f);
	Hunk_FreeToLowMark (mark);
}


void Cmd_SaveZone_f(void)
{
	FILE	*f;
	char	filename[128];
	///char	zonefile[512];
	
	if (Cmd_Argc () != 2)
	{
		Con_Printf ("savezone <zone> : save a zone definition file\n");		
		return;
	}

	sprintf(filename,"%s/zones/%s.cfg",com_gamedir, Cmd_Argv(1));

	// only if a map is running
	if (sv.active)
	{
		//f = fopen (va("%s/maps/%s.cfg",com_gamedir, sv.name), "w");
		//sprintf(filename, "%s/zones/%s.cfg", com_gamedir, zonefile);
		Con_Printf("%s--\n", filename);
		f = fopen (filename, "w+");
		if (!f)
		{
			Con_Printf ("\nCouldn't save zonefile settings.\n");
			return;
		}
		Con_Printf ("\nSaved zonefile settings.\n");

		fprintf (f, "// Zone Def File\n");

		fprintf (f, "zone_fogred		%s\n",	zone_fogred.value	);
	    fprintf (f, "zone_foggreen		%s\n",	zone_foggreen.value	);
		fprintf (f, "zone_fogblue		%s\n",	zone_fogblue.value	);
		fprintf (f, "zone_speed		%s\n",	zone_speed.value	);
		fprintf (f, "zone_enable		1\n");
		fprintf (f, "zone_start		%s\n",	zone_start.value	);
		fprintf (f, "zone_end		%s\n",	zone_end.value);


		fclose (f);
	}
	else      
		Con_Printf("No map loaded.\n");
}



void Cmd_Zone_f(void)
{
		Con_Printf ("// Zone Def \n");

		Con_Printf ( "zone_fogred		%f\n",	zone_fogred.value	);
	    Con_Printf ( "zone_foggreen		%f\n",	zone_foggreen.value	);
		Con_Printf ( "zone_fogblue		%f\n",	zone_fogblue.value);
		Con_Printf ( "zone_speed		%f\n",	zone_speed.value	);
		Con_Printf ( "zone_enable		%f\n", zone_enable.value);
		Con_Printf ( "zone_start		%f\n",	zone_start.value	);
		Con_Printf ( "zone_end			%f\n",	zone_end.value	);

}

void Cmd_Auto2zone_f(void)
{
	zone_fogred.value	= gl_fogred.value;
	zone_foggreen.value = gl_foggreen.value;
	zone_fogblue.value	= gl_fogblue.value;
	zone_start.value	= gl_fogstart.value;
	zone_end.value		= gl_fogend.value;
	
	Cmd_Zone_f();
}


// Tei autozone
#endif

extern cvar_t sbar_weaponsalpha;
void XR_InitParticles (void);

void R_Init (void)
{	
	extern byte *hunk_base;
	extern cvar_t gl_finish;
	extern cvar_t gl_emulatesoftwaremode;//Tei emulatesoftwaremode
	extern cvar_t mod_zwater;//Tei zwater offset	
	extern cvar_t r_ambient2;//Tei r_ambien2
	extern cvar_t gl_particledumb;//Tei

	Cmd_AddCommand ("timerefresh", R_TimeRefresh_f);	
	Cmd_AddCommand ("pointfile", R_ReadPointFile_f);




	Cvar_RegisterVariable (&r_norefresh);
	Cvar_RegisterVariable (&r_drawviewmodel);
	Cvar_RegisterVariable (&r_shadows);
	Cvar_RegisterVariable (&r_wateralpha);
	Cvar_RegisterVariable (&r_dynamic);
	Cvar_RegisterVariable (&r_mirroralpha);
	Cvar_RegisterVariable (&r_novis);
	Cvar_RegisterVariable (&r_speeds);
	Cvar_RegisterVariable (&gl_finish);
	Cvar_RegisterVariable (&gl_clear);

	Cvar_RegisterVariable (&gl_nocolors);
	Cvar_RegisterVariable (&gl_keeptjunctions);
	Cvar_RegisterVariable (&gl_doubleeyes);
	Cvar_RegisterVariable (&gl_fogenable); 
	Cvar_RegisterVariable (&gl_fogstart); 
	Cvar_RegisterVariable (&gl_fogend); 
	Cvar_RegisterVariable (&gl_fogred); 
	Cvar_RegisterVariable (&gl_fogblue); 
	Cvar_RegisterVariable (&gl_foggreen);
	Cvar_RegisterVariable (&centerfade);	// Tomaz - Fading CenterPrints
	Cvar_RegisterVariable (&sbar_alpha);	// Tomaz - Sbar Alpha
	Cvar_RegisterVariable (&sbar_weaponsalpha);	// Tei weapon show
	Cvar_RegisterVariable (&con_alpha);		// Tomaz - Console Alpha
	Cvar_RegisterVariable (&r_wave);		// Tomaz - Water Wave
	Cvar_RegisterVariable (&gl_glows);		// Tomaz - Glow
	Cvar_RegisterVariable (&r_bobbing);		// Tomaz - Bobbing Items
	Cvar_RegisterVariable (&gl_envmap);		// Tomaz - Enviroment Mapping
	Cvar_RegisterVariable (&gl_caustics);	// Tomaz - Underwater Caustics

	

	Cvar_RegisterVariable (&gl_fbr);		// Tomaz - Fullbrights

	Cvar_RegisterVariable (&skybox_spin);	// Tomaz - Spinning Skyboxes
	Cvar_RegisterVariable (&mapshots);		// Tomaz - MapShots
	Cvar_RegisterVariable (&gl_showpolys);	// Tomaz - Show BSP Polygons
	Cvar_RegisterVariable (&gl_wireframe);	// Tomaz - Draw World as Wireframe and Textures
	Cvar_RegisterVariable (&gl_wireonly);	// Tomaz - Draw World as Wireframe Only
	Cvar_RegisterVariable (&gl_particles);	// Tomaz - Particles
	Cvar_RegisterVariable (&print_center_to_console);	// Tomaz - Prints CenterString's to the console
	Cvar_RegisterVariable (&gl_particle_fire);	// Tomaz - Fire Particles
	Cvar_RegisterVariable (&gl_particledumb);	// Tei make particles dumb, fastes fps for slow comps

#if 1
	//TELEJANO
	Cvar_RegisterVariable (&r_staticfx);	// Tei -  player quality control
	Cvar_RegisterVariable (&r_dosky);	// Tei - watermap
	Cvar_RegisterVariable (&watermap);	// Tei - watermap
	Cvar_RegisterVariable (&fpscroak);	// Tei - fpscontrol

	Cvar_RegisterVariable (&gl_cull);	// Q2!
	
	Cvar_RegisterVariable (&gl_sun_x);	// Tei sun
	Cvar_RegisterVariable (&gl_sun_y);	// Tei sun
	Cvar_RegisterVariable (&gl_sun_z);	// Tei sun
	Cvar_RegisterVariable (&gl_sun);	// Tei sun

	Cvar_RegisterVariable (&r_ambient);	// Tei r_ambient
	Cvar_RegisterVariable (&r_ambient2);	// Tei r_ambient


	Cvar_RegisterVariable (&gl_xrayblast);				// Tei xrayblast
	Cvar_RegisterVariable (&gl_emulatesoftwaremode);	// Tei emulatesoftwaremode
	Cvar_RegisterVariable (&gl_smallparticles);			// Tei emulatesoftwaremode

	Cvar_RegisterVariable (&gl_themefolder);	// Tei texture folder
	Cvar_RegisterVariable (&gl_decalfolder);	// Tei decal folder

	Cvar_RegisterVariable (&gl_detail);	// CHP, detail particles
	Cvar_RegisterVariable (&gl_fontalt); //Tei alternate font

	Cvar_RegisterVariable (&gl_dither); //Tei alternate font

	Cvar_RegisterVariable (&gl_detailscale);	// Tei detail scale

	//Tei autozone
	Cvar_RegisterVariable (&zone_foggreen);
	Cvar_RegisterVariable (&zone_fogblue);
	Cvar_RegisterVariable (&zone_fogred);
	Cvar_RegisterVariable (&zone_speed);
	Cvar_RegisterVariable (&zone_enable);

	Cvar_RegisterVariable (&zone_start);
	Cvar_RegisterVariable (&zone_end);


	Cmd_AddCommand ("loadzone", Cmd_LoadZone_f);
	Cmd_AddCommand ("savezone", Cmd_SaveZone_f);	

	Cmd_AddCommand ("zone", Cmd_Zone_f);	
	Cmd_AddCommand ("auto2zone", Cmd_Auto2zone_f);		
	//Tei autozone

	Cvar_RegisterVariable (&extra_fx);//Tei prj for extra fx

	Cvar_RegisterVariable (&mod_zwater);	// Tei sun

	Cvar_RegisterVariable (&gl_geocaustics);	//Tei geocaustics
	
#endif



	XR_InitParticles(); //Tei QMB particles
	R_InitParticles (); // initiate particle engine

	playertextures = texture_extension_number;
	texture_extension_number += 16;
}

/*
===============
R_TranslatePlayerSkin

Translates a skin texture by the per-player color lookup
===============
*/
void R_TranslatePlayerSkin (int playernum)
{
	int				top, bottom;
	byte			translate[256];
	unsigned		translate32[256];
	unsigned int	i, j, s;
	model_t			*model;
	aliashdr_t		*paliashdr;
	byte			*original;
	unsigned		pixels[512*256], *out;
	unsigned		scaled_width, scaled_height;
	int				inwidth, inheight;
	byte			*inrow;
	unsigned		frac, fracstep;

	currententity = &cl_entities[1+playernum];
	model = currententity->model;
	if (!model)
		return;		// player doesn't have a model yet

	if (model->type != mod_alias)
		return; // only translate skins on alias models

	if (model->aliastype != ALIASTYPE_MDL) 
		return; // only translate skins on .mdl models 

	top = cl.scores[playernum].colors & 0xf0;
	bottom = (cl.scores[playernum].colors &15)<<4;

	for (i=0 ; i<256 ; i++)
		translate[i] = i;

	for (i=0 ; i<16 ; i++)
	{
		if (top < 128)	// the artists made some backwards ranges.  sigh.
			translate[TOP_RANGE+i] = top+i;
		else
			translate[TOP_RANGE+i] = top+15-i;
				
		if (bottom < 128)
			translate[BOTTOM_RANGE+i] = bottom+i;
		else
			translate[BOTTOM_RANGE+i] = bottom+15-i;
	}

	//
	// locate the original skin pixels
	//

	paliashdr = (aliashdr_t *)Mod_Extradata (model);
	s = paliashdr->skinwidth * paliashdr->skinheight;
	if (currententity->skinnum < 0 || currententity->skinnum >= paliashdr->numskins) {
		Con_Printf("(%d): Invalid player skin #%d\n", playernum, currententity->skinnum);
		original = (byte *)paliashdr + paliashdr->texels[0];
	} else
		original = (byte *)paliashdr + paliashdr->texels[currententity->skinnum];
	if (s & 3)
		Sys_Error ("R_TranslateSkin: s&3");

	inwidth = paliashdr->skinwidth;
	inheight = paliashdr->skinheight;

	// because this happens during gameplay, do it fast
	// instead of sending it through gl_upload 8
	glBindTexture (GL_TEXTURE_2D, playertextures + playernum);

	scaled_width = gl_max_size.value < 512 ? gl_max_size.value : 512;
	scaled_height = gl_max_size.value < 256 ? gl_max_size.value : 256;

	for (i=0 ; i<256 ; i++)
		translate32[i] = d_8to24table[translate[i]];

	out = pixels;
	fracstep = inwidth*0x10000/scaled_width;
	for (i=0 ; i<scaled_height ; i++, out += scaled_width)
	{
		inrow = original + inwidth*(i*inheight/scaled_height);
		frac = fracstep >> 1;
		for (j=0 ; j<scaled_width ; j+=4)
		{
			out[j] = translate32[inrow[frac>>16]];
			frac += fracstep;
			out[j+1] = translate32[inrow[frac>>16]];
			frac += fracstep;
			out[j+2] = translate32[inrow[frac>>16]];
			frac += fracstep;
			out[j+3] = translate32[inrow[frac>>16]];
			frac += fracstep;
		}
	}
	glTexImage2D (GL_TEXTURE_2D, 0, gl_solid_format, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

/*
===============
R_NewMap
===============
*/
void XR_ClearParticles (void);

void R_NewMap (void)
{
	int		i;
	extern int r_dlightframecount;

	// Tei - permap cfg
	Cbuf_AddText(va("exec maps/cfgs/%s.cfg\n", sv.name));
	Cbuf_Execute();
	// Tei - permap cfg

	for (i=0 ; i<256 ; i++)
		d_lightstylevalue[i] = 264;		// normal light value

	memset (&r_worldentity, 0, sizeof(r_worldentity));
	r_worldentity.model = cl.worldmodel;

// clear out efrags in case the level hasn't been reloaded
// FIXME: is this one short?
	for (i=0 ; i<cl.worldmodel->numleafs ; i++)
		cl.worldmodel->leafs[i].efrags = NULL;
		 	
	r_viewleaf = NULL;
	R_ClearParticles ();
	XR_ClearParticles ();//Tei QMB!

	r_dlightframecount = 0;

	GL_BuildLightmaps ();

	SHOWLMP_clear();	// Tomaz - Show Hide LMP
}

/*
====================
R_TimeRefresh_f

For program optimization
====================
*/
void R_TimeRefresh_f (void)
{
	int			i;
	float		start, stop, time;

	glDrawBuffer  (GL_FRONT);
	glFinish ();

	start = Sys_FloatTime ();
	for (i=0 ; i<128 ; i++)
	{
		r_refdef.viewangles[1] = i/128.0*360.0;
		R_RenderView ();
	}

	glFinish ();
	stop = Sys_FloatTime ();
	time = stop-start;
	Con_Printf ("%f seconds (%f fps)\n", time, 128/time);

	glDrawBuffer  (GL_BACK);
	GL_EndRendering ();
}