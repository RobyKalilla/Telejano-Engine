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
// models.c -- bsp model loading and caching

// models are the only shared resource between a client and server running
// on the same machine.

// DEFINE MODEL, DAMNIT!!Hunk_Alloc

#include "quakedef.h"
#include "gl_mirror.h"

model_t	*loadmodel;
char	loadname[32];	// for hunk tags

void Mod_LoadQ2AliasModel (model_t *mod, void *buffer); // Tomaz - Quake2 Models
void Mod_LoadSpriteModel (model_t *mod, void *buffer);
void Mod_LoadBrushModel (model_t *mod, void *buffer);
void Mod_LoadAliasModel (model_t *mod, void *buffer);
model_t *Mod_LoadModel (model_t *mod, qboolean crash);

byte	mod_novis[MAX_MAP_LEAFS/8];


// LordHavoc: increased from 512 to 2048
#define	MAX_MOD_KNOWN	2048

//#define	MAX_MOD_KNOWN	512//Standard value
model_t	mod_known[MAX_MOD_KNOWN];
int		mod_numknown;

cvar_t gl_subdivide_size = {"gl_subdivide_size", "128", true};

/*
===============
Mod_Init
===============
*/
void Mod_Init (void)
{
	Cvar_RegisterVariable (&gl_subdivide_size);
	memset (mod_novis, 0xff, sizeof(mod_novis));
}

/*
===============
Mod_Extradata

Caches the data if needed
===============
*/
void *Mod_Extradata (model_t *mod)
{
	void	*r;
	
	r = Cache_Check (&mod->cache);
	
#if 1 //Default
	if (r )
		return r;
#else
	if (r && mod->name[0]!='*') 
		return r;
#endif

	Mod_LoadModel (mod, true);
	
	if (!mod->cache.data)
		Sys_Error ("Mod_Extradata: caching failed");
	return mod->cache.data;
}

/*
===============
Mod_PointInLeaf
===============
*/
#if 0 //standard
mleaf_t *Mod_PointInLeaf (vec3_t p, model_t *model)
{
	mnode_t		*node;
	float		d;
	mplane_t	*plane;
	
	if (!model || !model->nodes)
		Sys_Error ("Mod_PointInLeaf: bad model");

	node = model->nodes;
	while (1)
	{
		if (node->contents < 0)
			return (mleaf_t *)node;
		plane = node->plane;
		d = PlaneDiff (p,plane);
		if (d > 0)
			node = node->children[0];
		else
			node = node->children[1];
	}
	
	return NULL;	// never reached
}

#else //LordHavoc version


mleaf_t *Mod_PointInLeaf (vec3_t p, model_t *model)
{
	mnode_t		*node;

	//Mod_CheckLoaded(model);
	if (!model || !model->nodes)
		Sys_Error ("Mod_PointInLeaf: bad model");

	// LordHavoc: modified to start at first clip node,
	// in other words: first node of the (sub)model
	node = model->nodes + model->hulls[0].firstclipnode;
	while (node->contents == 0)
		node = node->children[(node->plane->type < 3 ? p[node->plane->type] : DotProduct (p,node->plane->normal)) < node->plane->dist];

	return (mleaf_t *)node;
}

void Mod_FindNonSolidLocation(vec3_t pos, model_t *mod)
{
	if (Mod_PointInLeaf(pos, mod)->contents != CONTENTS_SOLID) return;
	pos[0]-=1;if (Mod_PointInLeaf(pos, mod)->contents != CONTENTS_SOLID) return;
	pos[0]+=2;if (Mod_PointInLeaf(pos, mod)->contents != CONTENTS_SOLID) return;
	pos[0]-=1;
	pos[1]-=1;if (Mod_PointInLeaf(pos, mod)->contents != CONTENTS_SOLID) return;
	pos[1]+=2;if (Mod_PointInLeaf(pos, mod)->contents != CONTENTS_SOLID) return;
	pos[1]-=1;
	pos[2]-=1;if (Mod_PointInLeaf(pos, mod)->contents != CONTENTS_SOLID) return;
	pos[2]+=2;if (Mod_PointInLeaf(pos, mod)->contents != CONTENTS_SOLID) return;
	pos[2]-=1;
}
#endif

/*
===================
Mod_DecompressVis
===================
*/
byte *Mod_DecompressVis (byte *in, model_t *model)
{
	static byte	decompressed[MAX_MAP_LEAFS/8];
	int		c;
	byte	*out;
	int		row;

	row = (model->numleafs+7)>>3;	
	out = decompressed;

	if (!in)
	{	// no vis info, so make all visible
		while (row)
		{
			*out++ = 0xff;
			row--;
		}
		return decompressed;		
	}

	do
	{
		if (*in)
		{
			*out++ = *in++;
			continue;
		}
	
		c = in[1];
		in += 2;
		while (c)
		{
			*out++ = 0;
			c--;
		}
	} while (out - decompressed < row);
	
	return decompressed;
}

/*
===================
Mod_LeadPVS
===================
*/

byte *Mod_LeafPVS (mleaf_t *leaf, model_t *model)
{
	if (leaf == model->leafs)
		return mod_novis;
	return Mod_DecompressVis (leaf->compressed_vis, model);
}

/*
===================
Mod_ClearAll
===================
*/
void Mod_ClearAll (void)
{
	int		i;
	model_t	*mod;
	
	for (i=0 , mod=mod_known ; i<mod_numknown ; i++, mod++)
		if (mod->type != mod_alias)
			mod->needload = true;
}

/*
==================
Mod_FindName

==================
*/
model_t *Mod_FindName (char *name)
{
	int		i;
	model_t	*mod;
	

	if (!name[0])
	{

#if 1 //Tei hack to load hl maps		
		//Search a loaded model
		for (i=0 , mod=mod_known ; i<mod_numknown ; i++, mod++)
			if (!strcmp (mod->name, name) )
				break;
	
		return mod;
#else
		Host_Error ("Mod_ForName: NULL name");
#endif 
	}
		
//
// search the currently loaded models
//
	for (i=0 , mod=mod_known ; i<mod_numknown ; i++, mod++)
		if (!strcmp (mod->name, name) )
			break;
			
	if (i == mod_numknown)
	{
		if (mod_numknown == MAX_MOD_KNOWN)
			Sys_Error ("mod_numknown == MAX_MOD_KNOWN");
		strcpy (mod->name, name);
		mod->needload = true;
		mod_numknown++;
	}

	return mod;
}

/*
==================
Mod_TouchModel

==================
*/
void Mod_TouchModel (char *name)
{
	model_t	*mod;
	
	mod = Mod_FindName (name);
	
	if (!mod->needload)
	{
		if (mod->type == mod_alias)
			Cache_Check (&mod->cache);
	}
}

qboolean Has_Fullbrights (byte *data, int size)
{
	int j;

	for (j = 0;j < size;j++)
	{
		if (data[j] > 223)
			return true;
	}

	return false;
}

qboolean Has_Fullbrights2 (byte *data, int size)
{
	int j;

	for (j = 0;j < size;j+=4)
	{
		if (data[j] > 223)
			return true;
		if (data[j+1] > 223)
			return true;
		if (data[j+2] > 223)
			return true;
	}

	return false;
}

/*
==================
Mod_LoadModel

Loads a model into the cache
==================
*/
extern cvar_t	gl_decalfolder;//Tei xfx decal folders
extern byte nullmodel[245];

void Mod_LoadHudModel (model_t *mod, void *buffer);


model_t *Mod_LoadModel (model_t *mod, qboolean crash)
{
	void	*d;
	unsigned *buf;
	byte	stackbuf[1024];		// avoid dirtying the cache heap
	char  name[MAX_QPATH], hackname[MAX_QPATH];

	mod->isfx_ = false;//Tei isfx_

	if (!mod->needload)
	{
		if (mod->type == mod_alias)
		{
			d = Cache_Check (&mod->cache);
			if (d)
				return mod;
		}
		else
			return mod;		// not cached at all
	}

//
// because the world is so huge, load it one piece at a time
//
	if (!crash) // they're on crack, mh
	{
	
	}
	

#if 1 //Tei decalfolder loading

	if (mod->name[0]=='*')
	{
		// Expect name "*tree.mdl"

		
		// Load from decal folder  
		sprintf(hackname,"%s",mod->name);		
		hackname[0] = '/';//hacking the path
		sprintf(name, "%s%s",gl_decalfolder.string , hackname);
	}
	else 
	{
		// Normal beavior
		sprintf(name,"%s",mod->name);		
	}
#endif //Tei decalfolder loading

//
// load the file
//


	//Tei null model loading
	if (!strncmp(mod->name,"progs/fx_",9)) 
	{
		//Con_Printf("Nullmodel loading detected!\n");
		buf			= (unsigned *)nullmodel;
		mod->isfx_	= true; //Tei isfx_
	}
	else
		buf = (unsigned *)COM_LoadStackFile (name, stackbuf, sizeof(stackbuf));//XFX
	//Tei null model loading



	if (!buf)
	{
		if (crash)
			Sys_Error ("Mod_NumForName: %s not found", mod->name);
		return NULL;
	}
	
//
// allocate a new model
//
	COM_FileBase (mod->name, loadname);
	
	loadmodel = mod;

//
// fill it in
//

// call the apropriate loader
	mod->needload = false;
	

	if (!strncmp(mod->name, "pics/",5))
			Mod_LoadHudModel (mod, buf);
	else
	{
		switch (LittleLong(*(unsigned *)buf))
		{
		case IDPOLYHEADER:
			Mod_LoadAliasModel (mod, buf);
			break;

		// Tomaz - Quake2 Models Begin
		case MD2IDALIASHEADER:
			Mod_LoadQ2AliasModel (mod, buf);
			break;		
		// Tomaz - Quake2 Models End

		case IDSPRITEHEADER:
			Mod_LoadSpriteModel (mod, buf);
			break;

		/*
		case PCXHEADER:
			Mod_LoadHudModel (mod, buf);
			break;	
		*/	
		default:
			Mod_LoadBrushModel (mod, buf);
			break;
		}
	}

	return mod;
}

/*
==================
Mod_ForName

Loads in a model for the given name
==================
*/
model_t *Mod_ForName (char *name, qboolean crash)
{
	model_t	*mod;
	
	mod = Mod_FindName (name);
	
	return Mod_LoadModel (mod, crash);
}


/*
===============================================================================

					BRUSHMODEL LOADING

===============================================================================
*/
int GetRSForName(char name[56]);
byte	*mod_base;

extern cvar_t debug_textures;//Tei debug 
extern cvar_t gl_themefolder;//Tei texture themes

/*
=================
Mod_LoadQ1Textures
=================
*/
void Mod_LoadQ1Textures (lump_t *l)
{
	int				i, j, num, max, altmax, size;
	miptex_t		*mt;
	texture_t		*tx, *tx2;
	texture_t		*anims[10];
	texture_t		*altanims[10];
	dmiptexlump_t	*m;
	byte			*data, *data2;
	int				*dofs;
	char			texname[64];
	qboolean		fullbrights = false;
	int				offset;//Tei wfx support
	

	if (!l->filelen)
	{
		loadmodel->textures = NULL;
		return;
	}

	m = (dmiptexlump_t *)(mod_base + l->fileofs);
	m->nummiptex = LittleLong (m->nummiptex);
	loadmodel->numtextures = m->nummiptex;
	loadmodel->textures = Hunk_AllocName (m->nummiptex * sizeof(*loadmodel->textures) , loadname);

	dofs = m->dataofs;
	for (i=0 ; i<m->nummiptex ; i++)
	{
		dofs[i] = LittleLong(dofs[i]);
		if (dofs[i] == -1)
			continue;
		mt = (miptex_t *)((byte *)m + dofs[i]);
		mt->width = LittleLong (mt->width);
		mt->height = LittleLong (mt->height);
		for (j=0 ; j<MIPLEVELS ; j++)
			mt->offsets[j] = LittleLong (mt->offsets[j]);
		
		if ( (mt->width & 15) || (mt->height & 15) )
			Host_Error ("Texture %s is not 16 aligned", mt->name);

		tx = Hunk_AllocName (sizeof(texture_t), loadname );
		loadmodel->textures[i] = tx;

		for (j = 0;mt->name[j] && j < 15;j++)
		{
			if (mt->name[j] >= 'A' && mt->name[j] <= 'Z')
				tx->name[j] = mt->name[j] + ('a' - 'A');
			else
				tx->name[j] = mt->name[j];
		}
		for (;j < 16;j++)
			tx->name[j] = 0;

		tx->width = mt->width;
		tx->height = mt->height;
		for (j=0 ; j<MIPLEVELS ; j++)
			tx->offsets[j] = 0;

		tx->rs			= GetRSForName(mt->name);


		sprintf(texname,"%s/%s",gl_themefolder.string,tx->name);//Tei texture theme

		//sprintf (texname, "textures/%s", tx->name);

	// NOTE: uncommenting this will let quake print all textures used in a map to the console
	
		// Tei debug text
		if (debug_textures.value)
			Con_Printf("Loading %s for Q1\n", texname);
		// Tei debug text

		tx->transparent	= false;
		tx->fullbrights	= -1;
		fullbrights		= false;
		// Tei wheater particles fx
		tx->frain		= false;
		tx->flight		= false;
		tx->fsnow		= false;
		tx->flava		= false; 
		tx->ftele		= false; 
		tx->fwater		= false;
		tx->fnodraw		= false;
		// Tei wheater particles fx

		data = loadimagepixels(tx->name, false);

		if (data)
		{


			/*
			// Tei wpfx
			if (!strncmp(tx->name,"light",5))
					tx->flight = true;
			if (!strncmp(tx->name,"win",3))
					tx->flight = true;
			// Tei wpfx
			*/
		
			if (!strncmp(tx->name,"sky",3) && image_width == 256 && image_height == 128)
			{
				tx->width			= image_width;
				tx->height			= image_height;

				R_InitSky (data ,4);

				// Tei wheater particles fx
				tx->issky = true;

				if (!strncmp(tx->name,"skyrain",7))
					tx->frain = true;
				else
				if (!strncmp(tx->name,"skylight",8))
					tx->flight = true;
				else
				if (!strncmp(tx->name,"skysnow",7))
					tx->fsnow = true;
				// Tei wheater particles fx
			}
			else
			{
				tx->width	= mt->width;
				tx->height	= mt->height;

				for (j = 0;j < image_width * image_height;j++)
				{
					if (data[j*4+3] < 255)
					{
						tx->transparent = true;
						break;
					}
				}
				tx->gl_texturenum = GL_LoadTexture (tx->name, image_width, image_height, data, true, tx->transparent, 4);
			}
			free(data);
		}
		else
		{
			if (!strncmp(tx->name,"sky",3) && mt->width == 256 && mt->height == 128)
			{
				tx->width	= mt->width;
				tx->height	= mt->height;
				R_InitSky ((byte *)((int) mt + mt->offsets[0]), 1);

				// Tei wheater particles fx
					tx->issky = true;
	
					if (!strncmp(tx->name,"skyrain",7))
						tx->frain = true;
					else
					if (!strncmp(tx->name,"skylight",8))
						tx->flight = true;
					else
					if (!strncmp(tx->name,"skysnow",7))
						tx->fsnow = true;

					// Tei wheater particles fx
			}
			else
			{
				if (mt->offsets[0])
				{
					// Tei wheater particles fx
					tx->issky = true;

					if (!strncmp(tx->name,"*lava_fx",8))
						tx->flava = true;
					else
					if (!strncmp(tx->name,"*tele_fx",8))
						tx->ftele = true;
					else
					if (!strncmp(tx->name,"*waterfx",8))
						tx->fwater = true;
					else
					if (!strncmp(tx->name,"*no_draw",8))
						tx->fnodraw = true;

					// Tei wheater particles fx

					data				= (byte *)((int) mt + mt->offsets[0]);
					if (data)
					{
						tx->width			= mt->width;
						tx->height			= mt->height;
						
						size = tx->width * tx->height;

						// HACK HACK HACK
						if (!strcmp(mt->name, "shot1sid") && mt->width==32 && mt->height==32
							&& CRC_Block((byte *)(mt+1), size) == 65393)
						{	// This texture in b_shell1.bsp has some of the first 32 pixels painted white.
							// They are invisible in software, but look really ugly in GL. So we just copy
							// 32 pixels from the bottom to make it look nice.
							memcpy (data, data + 32*31, 32);
						}

						// Tei text without fbr
						if (strncmp(tx->name,"nofbr_",4))
							fullbrights = Has_Fullbrights (data, size); // If not snow, check fbr
						// Tei text without fbr

						tx->gl_texturenum	= GL_LoadTexture (tx->name, tx->width, tx->height, data, true, false, 1);

						if (fullbrights)
						{
							data2 = malloc (tx->width * tx->height);
							sprintf (texname, "fbrm_%s", tx->name);

							for (j = 0;j < size;j++)
							{
								if(data[j] > 223)
									data2[j] = data[j];
								else
									data2[j] = 255;
							}

							tx->fullbrights		= GL_LoadTexture (texname, tx->width, tx->height, data2, true, true, 1);
							free(data2);
						}
					}
				}
				else
				{
					tx->width			= r_notexture_mip->width;
					tx->height			= r_notexture_mip->height;
					tx->gl_texturenum	= GL_LoadTexture ("notexture", tx->width, tx->height, (byte *)((int) r_notexture_mip + r_notexture_mip->offsets[0]), true, false, 1);
				}
			}
		}
	
		//Tei wfx
		if (tx->flava || tx->ftele || tx->fwater || tx->fnodraw )
		{							 
			offset = sizeof("*waterfx") - 1;//Ecual size others..
			if ( tx->name[ offset + 0 ] >='0' && tx->name[ offset + 0] <='9' &&
				 tx->name[ offset + 1 ] &&
				 tx->name[ offset + 2 ]
				) {

				tx->wfx_red   = (tx->name[ offset + 0] - '0')*25;
				tx->wfx_green = (tx->name[ offset + 1] - '0')*25;
				tx->wfx_blue  = (tx->name[ offset + 2] - '0')*25;
				if (debug_textures.value)
					Con_Printf ("debug: wfx Color: %d , %d, %d\n",tx->wfx_red,tx->wfx_green,tx->wfx_blue );
			}
			else
				if (debug_textures.value)
					Con_Printf ("debug: wfx mp color found without colorized\n", tx->name, offset);
		}
		//Tei wfx

	}

//
// sequence the animations
//

	// Tei snowtextures fix
	if (!strncmp(tx->name,"snow",4))
		tx->fullbrights = -1;	
	// Tei snowtextures fix

	
	
	for (i=0 ; i<m->nummiptex ; i++)
	{
		tx = loadmodel->textures[i];
		if (!tx || tx->name[0] != '+')
			continue;
		if (tx->anim_next)
			continue;	// allready sequenced

	// find the number of frames in the animation
		memset (anims, 0, sizeof(anims));
		memset (altanims, 0, sizeof(altanims));

		max = tx->name[1];
		altmax = 0;
		if (max >= 'a' && max <= 'z')
			max -= 'a' - 'A';
		if (max >= '0' && max <= '9')
		{
			max -= '0';
			altmax = 0;
			anims[max] = tx;
			max++;
		}
		else if (max >= 'A' && max <= 'J')
		{
			altmax = max - 'A';
			max = 0;
			altanims[altmax] = tx;
			altmax++;
		}
		else
			Sys_Error ("Bad animating texture %s", tx->name);

		for (j=i+1 ; j<m->nummiptex ; j++)
		{
			tx2 = loadmodel->textures[j];
			if (!tx2 || tx2->name[0] != '+')
				continue;
			if (strcmp (tx2->name+2, tx->name+2))
				continue;

			num = tx2->name[1];
			if (num >= 'a' && num <= 'z')
				num -= 'a' - 'A';
			if (num >= '0' && num <= '9')
			{
				num -= '0';
				anims[num] = tx2;
				if (num+1 > max)
					max = num + 1;
			}
			else if (num >= 'A' && num <= 'J')
			{
				num = num - 'A';
				altanims[num] = tx2;
				if (num+1 > altmax)
					altmax = num+1;
			}
			else
				Sys_Error ("Bad animating texture %s", tx->name);
		}
		
#define	ANIM_CYCLE	2
	// link them all together
		for (j=0 ; j<max ; j++)
		{
			tx2 = anims[j];
			if (!tx2)
				Sys_Error ("Missing frame %i of %s",j, tx->name);
			tx2->anim_total = max * ANIM_CYCLE;
			tx2->anim_min = j * ANIM_CYCLE;
			tx2->anim_max = (j+1) * ANIM_CYCLE;
			tx2->anim_next = anims[ (j+1)%max ];
			if (altmax)
				tx2->alternate_anims = altanims[0];
		}
		for (j=0 ; j<altmax ; j++)
		{
			tx2 = altanims[j];
			if (!tx2)
				Sys_Error ("Missing frame %i of %s",j, tx->name);
			tx2->anim_total = altmax * ANIM_CYCLE;
			tx2->anim_min = j * ANIM_CYCLE;
			tx2->anim_max = (j+1) * ANIM_CYCLE;
			tx2->anim_next = altanims[ (j+1)%altmax ];
			if (max)
				tx2->alternate_anims = anims[0];
		}
	}
}

/*
=================
Mod_LoadHLTextures
=================
*/

extern cvar_t debug_textures;

void Mod_LoadHLTextures (lump_t *l)
{
	int				i, j, num, max, altmax;
	miptex_t		*mt;
	texture_t		*tx, *tx2;
	texture_t		*anims[10];
	texture_t		*altanims[10];
	dmiptexlump_t	*m;
	byte			*data;
	int				*dofs;
	char			texname[64];
	qboolean		fullbrights = false;

	if (!l->filelen)
	{
		loadmodel->textures = NULL;
		return;
	}

	m = (dmiptexlump_t *)(mod_base + l->fileofs);
	m->nummiptex = LittleLong (m->nummiptex);
	loadmodel->numtextures = m->nummiptex;
	loadmodel->textures = Hunk_AllocName (m->nummiptex * sizeof(*loadmodel->textures) , loadname);

	dofs = m->dataofs;
	for (i=0 ; i<m->nummiptex ; i++)
	{
		dofs[i] = LittleLong(dofs[i]);
		if (dofs[i] == -1)
			continue;
		mt = (miptex_t *)((byte *)m + dofs[i]);
		mt->width = LittleLong (mt->width);
		mt->height = LittleLong (mt->height);
		for (j=0 ; j<MIPLEVELS ; j++)
			mt->offsets[j] = LittleLong (mt->offsets[j]);
		
		if ( (mt->width & 15) || (mt->height & 15) )
			Host_Error ("Texture %s is not 16 aligned", mt->name);

		tx = Hunk_AllocName (sizeof(texture_t), loadname );
		loadmodel->textures[i] = tx;

		for (j = 0;mt->name[j] && j < 15;j++)
		{
			if (mt->name[j] >= 'A' && mt->name[j] <= 'Z')
				tx->name[j] = mt->name[j] + ('a' - 'A');
			else
				tx->name[j] = mt->name[j];
		}
		for (;j < 16;j++)
			tx->name[j] = 0;

		tx->width = mt->width;
		tx->height = mt->height;
		for (j=0 ; j<MIPLEVELS ; j++)
			tx->offsets[j] = 0;

		tx->rs			= GetRSForName(mt->name);

		//sprintf (texname, "textures/%s", tx->name);
		sprintf(texname,"%s/%s",gl_themefolder.string,tx->name);//Tei texture theme



	// NOTE: uncommenting this will let quake print all textures used in a map to the console
		if (debug_textures.value) //Tei debug text
			Con_Printf("%s\n", texname);

		tx->transparent	= false;
		tx->fullbrights = -1;

		data = loadimagepixels(tx->name, false);

		if (data)
		{
			tx->width	= mt->width;
			tx->height	= mt->height;

			for (j = 0;j < image_width*image_height;j++)
			{
				if (data[j*4+3] < 255)
				{
					tx->transparent = true;
					break;
				}
			}
			tx->gl_texturenum = GL_LoadTexture (tx->name, image_width, image_height, data, true, tx->transparent, 4);

			free(data);
		}
		else
		{
			if (tx->name[0] == '{')
				tx->transparent = true;

			if (mt->offsets[0])
			{
				data				= W_ConvertWAD3Texture(mt);
				if (data)
				{
					tx->width			= mt->width;
					tx->height			= mt->height;
					tx->gl_texturenum	= GL_LoadTexture (tx->name, mt->width, mt->height, data, true, tx->transparent, 4);
					free(data);
				}
			}
			if (!data)
			{
				data				= W_GetTexture(mt->name);
				if (data)
				{
					tx->width			= image_width;
					tx->height			= image_height;
					tx->gl_texturenum	= GL_LoadTexture (tx->name, image_width, image_height, data, true, tx->transparent, 4);
					free(data);
				}
			}
			if (!data)
			{
				tx->width			= r_notexture_mip->width;
				tx->height			= r_notexture_mip->height;
				tx->gl_texturenum	= GL_LoadTexture ("notexture", tx->width, tx->height, (byte *)((int) r_notexture_mip + r_notexture_mip->offsets[0]), true, false, 1);
			}
		}
	}

//
// sequence the animations
//
	for (i=0 ; i<m->nummiptex ; i++)
	{
		tx = loadmodel->textures[i];
		if (!tx || tx->name[0] != '+')
			continue;
		if (tx->anim_next)
			continue;	// allready sequenced

	// find the number of frames in the animation
		memset (anims, 0, sizeof(anims));
		memset (altanims, 0, sizeof(altanims));

		max = tx->name[1];
		altmax = 0;
		if (max >= 'a' && max <= 'z')
			max -= 'a' - 'A';
		if (max >= '0' && max <= '9')
		{
			max -= '0';
			altmax = 0;
			anims[max] = tx;
			max++;
		}
		else if (max >= 'A' && max <= 'J')
		{
			altmax = max - 'A';
			max = 0;
			altanims[altmax] = tx;
			altmax++;
		}
		else
			Sys_Error ("Bad animating texture %s", tx->name);

		for (j=i+1 ; j<m->nummiptex ; j++)
		{
			tx2 = loadmodel->textures[j];
			if (!tx2 || tx2->name[0] != '+')
				continue;
			if (strcmp (tx2->name+2, tx->name+2))
				continue;

			num = tx2->name[1];
			if (num >= 'a' && num <= 'z')
				num -= 'a' - 'A';
			if (num >= '0' && num <= '9')
			{
				num -= '0';
				anims[num] = tx2;
				if (num+1 > max)
					max = num + 1;
			}
			else if (num >= 'A' && num <= 'J')
			{
				num = num - 'A';
				altanims[num] = tx2;
				if (num+1 > altmax)
					altmax = num+1;
			}
			else
				Sys_Error ("Bad animating texture %s", tx->name);
		}
		
#define	ANIM_CYCLE	2
	// link them all together
		for (j=0 ; j<max ; j++)
		{
			tx2 = anims[j];
			if (!tx2)
				Sys_Error ("Missing frame %i of %s",j, tx->name);
			tx2->anim_total = max * ANIM_CYCLE;
			tx2->anim_min = j * ANIM_CYCLE;
			tx2->anim_max = (j+1) * ANIM_CYCLE;
			tx2->anim_next = anims[ (j+1)%max ];
			if (altmax)
				tx2->alternate_anims = altanims[0];
		}
		for (j=0 ; j<altmax ; j++)
		{
			tx2 = altanims[j];
			if (!tx2)
				Sys_Error ("Missing frame %i of %s",j, tx->name);
			tx2->anim_total = altmax * ANIM_CYCLE;
			tx2->anim_min = j * ANIM_CYCLE;
			tx2->anim_max = (j+1) * ANIM_CYCLE;
			tx2->anim_next = altanims[ (j+1)%altmax ];
			if (max)
				tx2->alternate_anims = anims[0];
		}
	}
}

/*
==================
Mod_LoadQ1Lighting
==================
*/
void Mod_LoadQ1Lighting (lump_t *l)
{
	// Tomaz - Lit Support Begin
	int i;
	byte *in, *out, *data;
	byte d;
	char litfilename[1024];

	loadmodel->lightdata = NULL;

	strcpy(litfilename, loadmodel->name);
	COM_StripExtension(litfilename, litfilename);
	strcat(litfilename, ".lit");
	data = (byte*) COM_LoadHunkFile (litfilename);
	if (data)
	{
		if (data[0] == 'Q' && data[1] == 'L' && data[2] == 'I' && data[3] == 'T')
		{
			i = LittleLong(((int *)data)[1]);
			if (i == 1)
			{
				Con_DPrintf("%s loaded", litfilename);
				loadmodel->lightdata = data + 8;
				return;
			}
			else
				Con_Printf("Unknown .lit file version (%d)\n", i);
		}
		else
			Con_Printf("Corrupt .lit file (old version?), ignoring\n");
	}

	if (!l->filelen)
 		return;

	loadmodel->lightdata = Hunk_AllocName ( l->filelen*3, litfilename);
	in = loadmodel->lightdata + l->filelen*2; // place the file at the end, so it will not be overwritten until the very last write
	out = loadmodel->lightdata;
	memcpy (in, mod_base + l->fileofs, l->filelen);
	for (i = 0;i < l->filelen;i++)
	{
		d = *in++; 
		out[0]	= d;
		out[1]	= d;
		out[2]	= d;
		out		+= 3;
	}
	// Tomaz - Lit Support End
}

/*
==================
Mod_LoadHLLighting
==================
*/
void Mod_LoadHLLighting (lump_t *l)
{
	loadmodel->lightdata = NULL;

	loadmodel->lightdata = Hunk_AllocName ( l->filelen, va("%s lightmaps", loadname));
	memcpy (loadmodel->lightdata, mod_base + l->fileofs, l->filelen);
}

/*
=================
Mod_LoadVisibility
=================
*/
void Mod_LoadVisibility (lump_t *l)
{
	if (!l->filelen)
	{
		loadmodel->visdata = NULL;
		return;
	}
	loadmodel->visdata = Hunk_AllocName ( l->filelen, loadname);	
	memcpy (loadmodel->visdata, mod_base + l->fileofs, l->filelen);
}

void CL_ParseEntityLump(char *entdata);	// Tomaz - Qc Parsing

/*
=================
Mod_LoadEntities
=================
*/
void Mod_LoadEntities (lump_t *l)
{
	if (!l->filelen)
	{
		loadmodel->entities = NULL;
		return;
	}
	loadmodel->entities = Hunk_AllocName ( l->filelen, loadname);	
	memcpy (loadmodel->entities, mod_base + l->fileofs, l->filelen);

	CL_ParseEntityLump(loadmodel->entities);	// Tomaz - Parse Data
}


/*
=================
Mod_LoadVertexes
=================
*/
void Mod_LoadVertexes (lump_t *l)
{
	dvertex_t	*in;
	mvertex_t	*out;
	int			i, count;

	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Sys_Error ("MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = Hunk_AllocName ( count*sizeof(*out), loadname);	

	loadmodel->vertexes = out;
	loadmodel->numvertexes = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		out->position[0] = LittleFloat (in->point[0]);
		out->position[1] = LittleFloat (in->point[1]);
		out->position[2] = LittleFloat (in->point[2]);
	}
}

/*
=================
Mod_LoadSubmodels
=================
*/
void Mod_LoadSubmodels (lump_t *l)
{
	dmodel_t	*in;
	dmodel_t	*out;
	int			i, j, count;

	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Sys_Error ("MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = Hunk_AllocName ( count*sizeof(*out), loadname);	

	loadmodel->submodels = out;
	loadmodel->numsubmodels = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		for (j=0 ; j<3 ; j++)
		{	// spread the mins / maxs by a pixel
			out->mins[j] = LittleFloat (in->mins[j]) - 1;
			out->maxs[j] = LittleFloat (in->maxs[j]) + 1;
			out->origin[j] = LittleFloat (in->origin[j]);
		}
		for (j=0 ; j<MAX_MAP_HULLS ; j++)
			out->headnode[j] = LittleLong (in->headnode[j]);
		out->visleafs = LittleLong (in->visleafs);
		out->firstface = LittleLong (in->firstface);
		out->numfaces = LittleLong (in->numfaces);
	}
}

/*
=================
Mod_LoadEdges
=================
*/
void Mod_LoadEdges (lump_t *l)
{
	dedge_t *in;
	medge_t *out;
	int 	i, count;

	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Sys_Error ("MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = Hunk_AllocName ( (count + 1) * sizeof(*out), loadname);	

	loadmodel->edges = out;
	loadmodel->numedges = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		out->v[0] = (unsigned short)LittleShort(in->v[0]);
		out->v[1] = (unsigned short)LittleShort(in->v[1]);
	}
}

/*
=================
Mod_LoadTexinfo
=================
*/
void Mod_LoadTexinfo (lump_t *l)
{
	texinfo_t *in;
	mtexinfo_t *out;
	int 	i, j, count;
	int		miptex;
	float	len1, len2;

	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Sys_Error ("MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = Hunk_AllocName ( count*sizeof(*out), loadname);	

	loadmodel->texinfo = out;
	loadmodel->numtexinfo = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		for (j=0 ; j<8 ; j++)
			out->vecs[0][j] = LittleFloat (in->vecs[0][j]);
		len1 = Length (out->vecs[0]);
		len2 = Length (out->vecs[1]);
		len1 = (len1 + len2)/2;
		if (len1 < 0.32)
			out->mipadjust = 4;
		else if (len1 < 0.49)
			out->mipadjust = 3;
		else if (len1 < 0.99)
			out->mipadjust = 2;
		else
			out->mipadjust = 1;

		miptex = LittleLong (in->miptex);
		out->flags = LittleLong (in->flags);
	
		if (!loadmodel->textures)
		{
			out->texture = r_notexture_mip;	// checkerboard texture
			out->flags = 0;
		}
		else
		{
			if (miptex >= loadmodel->numtextures)
				Sys_Error ("miptex >= loadmodel->numtextures");
			out->texture = loadmodel->textures[miptex];
			if (!out->texture)
			{
				out->texture = r_notexture_mip; // texture not found
				out->flags = 0;
			}
		}
	}
}

/*
================
CalcSurfaceExtents

Fills in s->texturemins[] and s->extents[]
================
*/
void CalcSurfaceExtents (msurface_t *s)
{
	float	mins[2], maxs[2], val;
	int		i,j, e;
	mvertex_t	*v;
	mtexinfo_t	*tex;
	int		bmins[2], bmaxs[2];

	mins[0] = mins[1] =  99999;
	maxs[0] = maxs[1] = -99999;

	tex = s->texinfo;
	
	for (i=0 ; i<s->numedges ; i++)
	{
		e = loadmodel->surfedges[s->firstedge+i];
		if (e >= 0)
			v = &loadmodel->vertexes[loadmodel->edges[e].v[0]];
		else
			v = &loadmodel->vertexes[loadmodel->edges[-e].v[1]];
		
		for (j=0 ; j<2 ; j++)
		{
			val = v->position[0] * tex->vecs[j][0] + 
				v->position[1] * tex->vecs[j][1] +
				v->position[2] * tex->vecs[j][2] +
				tex->vecs[j][3];
			if (val < mins[j])
				mins[j] = val;
			if (val > maxs[j])
				maxs[j] = val;
		}
	}

	for (i=0 ; i<2 ; i++)
	{	
		bmins[i] = floor(mins[i] * 0.0625);	// Tomaz - Speed
		bmaxs[i] = ceil (maxs[i] * 0.0625);	// Tomaz - Speed

		s->texturemins[i] = bmins[i] * 16;
		s->extents[i] = (bmaxs[i] - bmins[i]) * 16;
		if ((tex->flags & TEX_SPECIAL) == 0 && (s->extents[i]+1) > (256*16))
			Host_Error ("Bad surface extents");
	}
}

void GL_SubdivideSurface (msurface_t *s);
qboolean hl_map;

/*
=================
Mod_LoadFaces
=================
*/
void Mod_LoadFaces (lump_t *l)
{
	dface_t		*in;
	msurface_t 	*out;
	int			i, count, surfnum;
	int			planenum, side;

	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Sys_Error ("MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = Hunk_AllocName ( count*sizeof(*out), loadname);	

	loadmodel->surfaces = out;
	loadmodel->numsurfaces = count;

	for ( surfnum=0 ; surfnum<count ; surfnum++, in++, out++)
	{
		out->firstedge = LittleLong(in->firstedge);
		out->numedges = LittleShort(in->numedges);		
		out->flags = 0;

		planenum = LittleShort(in->planenum);
		side = LittleShort(in->side);
		if (side)
			out->flags |= SURF_PLANEBACK;			

		out->plane = loadmodel->planes + planenum;

		out->texinfo = loadmodel->texinfo + LittleShort (in->texinfo);

		CalcSurfaceExtents (out);
				
	// lighting info

		for (i=0 ; i<MAXLIGHTMAPS ; i++)
			out->styles[i] = in->styles[i];
		i = LittleLong(in->lightofs);
		if (i == -1)
			out->samples = NULL;
		else if	(hl_map)
			out->samples = loadmodel->lightdata + i;		
		else
			out->samples = loadmodel->lightdata + (i * 3);	// Tomaz - Lit Support

	// set the drawing flags flag

		if (!Q_strncmp(out->texinfo->texture->name,"sky",3))	// sky
		{
			out->flags |= (SURF_DRAWSKY | SURF_DRAWTILED);
			GL_SubdivideSurface (out);	// cut up polygon for warps
			continue;
		}

// FIXME: support for iD mirrors, remove this and do this correct with rscript
		if (!strncmp (out->texinfo->texture->name, "window02_1", 10))
		{
			out->flags |= SURF_MIRROR;
			continue;
		}
// END
// FIXME: support for iD mirrors, remove this and do this correct with rscript
		/*
		if (!strncmp (out->texinfo->texture->name, "tech", 4))
		{
			out->flags |= SURF_MIRROR;
			continue;
		}
	*/
		// END
#if 0		
		//JHL:ADD; Make thee shine like a glass!
		if (!Q_strncmp(out->texinfo->texture->name,"window",6) ||
			!Q_strncmp(out->texinfo->texture->name,"afloor3_1",9)||
			!Q_strncmp(out->texinfo->texture->name,"wizwin",6) )
			out->flags |= SURF_SHINY_GLASS;

		//JHL:ADD; Make thee shine like the might steel!
		if ( (!Q_strncmp(out->texinfo->texture->name,"metal", 5)
		      && Q_strncmp(out->texinfo->texture->name,"metal4", 6)
		      && Q_strncmp(out->texinfo->texture->name,"metal5", 6)
		      && Q_strncmp(out->texinfo->texture->name,"metal1_6", 8)
		      && Q_strncmp(out->texinfo->texture->name,"metal2_1", 8))
		      || !Q_strncmp(out->texinfo->texture->name,"cop",3)
		      || !Q_strncmp(out->texinfo->texture->name,"floor",5)
			  || !strncmp (out->texinfo->texture->name, "sfloor", 6)
			  )
			out->flags |= SURF_SHINY_METAL;
#endif

		//Tei grass
		if (!Q_strncmp(out->texinfo->texture->name,"grx_",4))
			out->flags |= SURF_GRASS;
		//Tei grass

		if (out->texinfo->texture->name[0] == '*'	||	// turbulent
			out->texinfo->texture->name[0] == '!')
		{
			out->flags |= (SURF_DRAWTURB | SURF_DRAWTILED);
			for (i=0 ; i<2 ; i++)
			{
				out->extents[i] = 16384;
				out->texturemins[i] = -8192;
			}
			GL_SubdivideSurface (out);	// cut up polygon for warps
			continue;
		}
	}
}

/*
=================
Mod_SetParent
=================
*/
void Mod_SetParent (mnode_t *node, mnode_t *parent)
{
	node->parent = parent;
	if (node->contents < 0)
		return;
	Mod_SetParent (node->children[0], node);
	Mod_SetParent (node->children[1], node);
}

/*
=================
Mod_LoadNodes
=================
*/
void Mod_LoadNodes (lump_t *l)
{
	int			i, j, count, p;
	dnode_t		*in;
	mnode_t 	*out;

	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Sys_Error ("MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = Hunk_AllocName ( count*sizeof(*out), loadname);	

	loadmodel->nodes = out;
	loadmodel->numnodes = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		for (j=0 ; j<3 ; j++)
		{
			out->mins[j] = LittleShort (in->mins[j]);
			out->maxs[j] = LittleShort (in->maxs[j]);
		}
	
		p = LittleLong(in->planenum);
		out->plane = loadmodel->planes + p;

		out->firstsurface = LittleShort (in->firstface);
		out->numsurfaces = LittleShort (in->numfaces);
		
		for (j=0 ; j<2 ; j++)
		{
			p = LittleShort (in->children[j]);
			if (p >= 0)
				out->children[j] = loadmodel->nodes + p;
			else
				out->children[j] = (mnode_t *)(loadmodel->leafs + (-1 - p));
		}
	}
	
	Mod_SetParent (loadmodel->nodes, NULL);	// sets nodes and leafs
}

/*
=================
Mod_LoadLeafs
=================
*/
void Mod_LoadLeafs (lump_t *l)
{
	dleaf_t 	*in;
	mleaf_t 	*out;
	int			i, j, count, p;

	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Sys_Error ("MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = Hunk_AllocName ( count*sizeof(*out), loadname);	

	loadmodel->leafs = out;
	loadmodel->numleafs = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		for (j=0 ; j<3 ; j++)
		{
			out->mins[j] = LittleShort (in->mins[j]);
			out->maxs[j] = LittleShort (in->maxs[j]);
		}

		p = LittleLong(in->contents);
		out->contents = p;

		out->firstmarksurface = loadmodel->marksurfaces +
			LittleShort(in->firstmarksurface);
		out->nummarksurfaces = LittleShort(in->nummarksurfaces);
		
		p = LittleLong(in->visofs);
		if (p == -1)
			out->compressed_vis = NULL;
		else
			out->compressed_vis = loadmodel->visdata + p;
		out->efrags = NULL;
		
		for (j=0 ; j<4 ; j++)
			out->ambient_sound_level[j] = in->ambient_level[j];

		// gl underwater warp
		if (out->contents != CONTENTS_EMPTY) 
		{
			for (j=0 ; j<out->nummarksurfaces ; j++)
				out->firstmarksurface[j]->flags |= SURF_UNDERWATER;
		}
	}	
}

/*
=================
Mod_LoadQ1Clipnodes
=================
*/
void Mod_LoadQ1Clipnodes (lump_t *l)
{
	dclipnode_t *in, *out;
	int			i, count;
	hull_t		*hull;

	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Sys_Error ("MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = Hunk_AllocName ( count*sizeof(*out), loadname);	

	loadmodel->clipnodes = out;
	loadmodel->numclipnodes = count;

	hull = &loadmodel->hulls[1];
	hull->clipnodes = out;
	hull->firstclipnode = 0;
	hull->lastclipnode = count-1;
	hull->planes = loadmodel->planes;
	hull->clip_mins[0] = -16;
	hull->clip_mins[1] = -16;
	hull->clip_mins[2] = -24;
	hull->clip_maxs[0] = 16;
	hull->clip_maxs[1] = 16;
	hull->clip_maxs[2] = 32;

	hull = &loadmodel->hulls[2];
	hull->clipnodes = out;
	hull->firstclipnode = 0;
	hull->lastclipnode = count-1;
	hull->planes = loadmodel->planes;
	hull->clip_mins[0] = -32;
	hull->clip_mins[1] = -32;
	hull->clip_mins[2] = -24;
	hull->clip_maxs[0] = 32;
	hull->clip_maxs[1] = 32;
	hull->clip_maxs[2] = 64;

	// Tomaz - Crouch Code Begin
	hull = &loadmodel->hulls[3];
	hull->clipnodes = out;
	hull->firstclipnode = 0;
	hull->lastclipnode = count-1;
	hull->planes = loadmodel->planes;
	hull->clip_mins[0] = -16;
	hull->clip_mins[1] = -16;
	hull->clip_mins[2] = -24;
	hull->clip_maxs[0] = 16;
	hull->clip_maxs[1] = 16;
	hull->clip_maxs[2] = 32;
	// Tomaz - Crouch Code End

	for (i=0 ; i<count ; i++, out++, in++)
	{
		out->planenum = LittleLong(in->planenum);
		out->children[0] = LittleShort(in->children[0]);
		out->children[1] = LittleShort(in->children[1]);
	}
}

/*
=================
Mod_LoadHLClipnodes
=================
*/
void Mod_LoadHLClipnodes (lump_t *l)
{
	dclipnode_t *in, *out;
	int			i, count;
	hull_t		*hull;

	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Sys_Error ("MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count	= l->filelen / sizeof(*in);
	out		= Hunk_AllocName ( count*sizeof(*out), loadname);	

	loadmodel->clipnodes	= out;
	loadmodel->numclipnodes = count;

	hull				= &loadmodel->hulls[1];
	hull->clipnodes		= out;
	hull->firstclipnode = 0;
	hull->lastclipnode	= count-1;
	hull->planes		= loadmodel->planes;
	hull->clip_mins[0]	= -16;
	hull->clip_mins[1]	= -16;
	hull->clip_mins[2]	= -36;
	hull->clip_maxs[0]	= 16;
	hull->clip_maxs[1]	= 16;
	hull->clip_maxs[2]	= 36;
	VectorSubtract(hull->clip_maxs, hull->clip_mins, hull->clip_size);//Tei trace lh

	hull				= &loadmodel->hulls[2];
	hull->clipnodes		= out;
	hull->firstclipnode = 0;
	hull->lastclipnode	= count-1;
	hull->planes		= loadmodel->planes;
	hull->clip_mins[0]	= -32;
	hull->clip_mins[1]	= -32;
	hull->clip_mins[2]	= -32;
	hull->clip_maxs[0]	= 32;
	hull->clip_maxs[1]	= 32;
	hull->clip_maxs[2]	= 32;
	VectorSubtract(hull->clip_maxs, hull->clip_mins, hull->clip_size);//Tei trace lh

	hull				= &loadmodel->hulls[3];
	hull->clipnodes		= out;
	hull->firstclipnode = 0;
	hull->lastclipnode	= count-1;
	hull->planes		= loadmodel->planes;
	hull->clip_mins[0]	= -16;
	hull->clip_mins[1]	= -16;
	hull->clip_mins[2]	= -18;
	hull->clip_maxs[0]	= 16;
	hull->clip_maxs[1]	= 16;
	hull->clip_maxs[2]	= 18;
	VectorSubtract(hull->clip_maxs, hull->clip_mins, hull->clip_size);//Tei trace lh

	for (i=0 ; i<count ; i++, out++, in++)
	{
		out->planenum = LittleLong(in->planenum);
		out->children[0] = LittleShort(in->children[0]);
		out->children[1] = LittleShort(in->children[1]);
	}
}

/*
=================
Mod_MakeHull0

Duplicate the drawing hull structure as a clipping hull
thanks go to MH for fixing this. (http://mhquake.quakesrc.org)
MH - includes dirty hack to fix sky nodes not being set to CONTENTS_SKY in ID maps
=================
*/
void Mod_MakeHull0 (void)
{
	mnode_t	 *in, *child;
	dclipnode_t *out;
	int i, j, count;
	hull_t *hull;

	hull = &loadmodel->hulls[0];	

	in = loadmodel->nodes;
	count = loadmodel->numnodes;
	out = Hunk_AllocName ( count*sizeof(*out), loadname);	

	hull->clipnodes = out;
	hull->firstclipnode = 0;
	hull->lastclipnode = count-1;
	hull->planes = loadmodel->planes;

	//Con_Printf("XFX LETSRUINEVERYTHING\n");//Tei XFX debug

	for (i = 0; i < count; i++, out++, in++)
	{
		out->planenum = in->plane - loadmodel->planes;

//LETS_RUIN_EVERYTHING (Mh dead code, delete say Mh. Needed by sun fx)
#if 0
		for (j = 0; j < 2; j++)
		{
			child = in->children[j];

			if (child->contents < 0)
			{
				// check the node for a sky texture - this way is much faster than using strncmp
				// we need only check the first surface (if that's sky, the rest of it will be
				// too), and since everything else has been loaded at this stage, it's safe to
				// check it this way.
				if (loadmodel->surfaces[in->firstsurface].texinfo->texture->name[0] == 's' &&
					loadmodel->surfaces[in->firstsurface].texinfo->texture->name[1] == 'k' &&
					loadmodel->surfaces[in->firstsurface].texinfo->texture->name[2] == 'y')
					out->children[j] = CONTENTS_SKY;
				else out->children[j] = child->contents;
			}
			else out->children[j] = child - loadmodel->nodes;
		}
//LETS_RUIN_EVERYTHING (Dead code (bug))
	
#else	
		for (j = 0; j < 2; j++)
		{
			child = in->children[j];

			if (child->contents < 0)
				out->children[j] = child->contents;
			else 
				out->children[j] = child - loadmodel->nodes;
		}
#endif	
	}
}


/*
=================
Mod_LoadMarksurfaces
=================
*/
void Mod_LoadMarksurfaces (lump_t *l)
{	
	int		i, j, count;
	short		*in;
	msurface_t **out;
	
	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Sys_Error ("MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = Hunk_AllocName ( count*sizeof(*out), loadname);	

	loadmodel->marksurfaces = out;
	loadmodel->nummarksurfaces = count;

	for ( i=0 ; i<count ; i++)
	{
		j = LittleShort(in[i]);
		if (j >= loadmodel->numsurfaces)
			Sys_Error ("Mod_ParseMarksurfaces: bad surface number");
		out[i] = loadmodel->surfaces + j;
	}
}

/*
=================
Mod_LoadSurfedges
=================
*/
void Mod_LoadSurfedges (lump_t *l)
{	
	int		i, count;
	int		*in, *out;
	
	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Sys_Error ("MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = Hunk_AllocName ( count*sizeof(*out), loadname);	

	loadmodel->surfedges = out;
	loadmodel->numsurfedges = count;

	for ( i=0 ; i<count ; i++)
		out[i] = LittleLong (in[i]);
}


/*
=================
Mod_LoadPlanes
=================
*/
void Mod_LoadPlanes (lump_t *l)
{
	int			i, j;
	mplane_t	*out;
	dplane_t 	*in;
	int			count;
	int			bits;
	
	in = (void *)(mod_base + l->fileofs);
	if (l->filelen % sizeof(*in))
		Sys_Error ("MOD_LoadBmodel: funny lump size in %s",loadmodel->name);
	count = l->filelen / sizeof(*in);
	out = Hunk_AllocName ( count*2*sizeof(*out), loadname);	
	
	loadmodel->planes = out;
	loadmodel->numplanes = count;

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		bits = 0;
		for (j=0 ; j<3 ; j++)
		{
			out->normal[j] = LittleFloat (in->normal[j]);
			if (out->normal[j] < 0)
				bits |= 1<<j;
		}

		out->dist = LittleFloat (in->dist);
		out->type = LittleLong (in->type);
		out->signbits = bits;
	}
}

/*
=================
RadiusFromBounds
=================
*/
float RadiusFromBounds (vec3_t mins, vec3_t maxs)
{
	int		i;
	vec3_t	corner;

	for (i=0 ; i<3 ; i++)
	{
		corner[i] = fabs(mins[i]) > fabs(maxs[i]) ? fabs(mins[i]) : fabs(maxs[i]);
	}

	return Length (corner);
}

/*
=================
Mod_LoadBrushModel
=================
*/

void Mod_LoadBrushModel (model_t *mod, void *buffer)
{
	int			i, j;
	dheader_t	*header;
	dmodel_t 	*bm;
	
	loadmodel->type = mod_brush;
	
	header = (dheader_t *)buffer;

	i = LittleLong (header->version);

	// Tomaz - Fixing Wrong BSP Version Error Begin
	if ((i != Q1BSP) && (i != HLBSP))	// Tomaz - HL Maps
	{
		Con_Printf("Mod_LoadBrushModel: %s has wrong version number (%i should be %i (Q1) or %i (HL))", mod->name, i, Q1BSP, HLBSP);
		mod->numvertexes=-1;	// HACK - incorrect BSP version is no longer fatal
		return;
	}
	// Tomaz - Fixing Wrong BSP Version Error End

	if (i == HLBSP)
		hl_map = true;
	else
		hl_map = false;

	mod->ishlbsp = hl_map;//Tei lh trace support 

// swap all the lumps
	mod_base = (byte *)header;

	for (i=0 ; i<sizeof(dheader_t)/4 ; i++)
		((int *)header)[i] = LittleLong ( ((int *)header)[i]);

// load into heap

	Mod_LoadEntities	(&header->lumps[LUMP_ENTITIES]);	
	Mod_LoadVertexes	(&header->lumps[LUMP_VERTEXES]);
	Mod_LoadEdges		(&header->lumps[LUMP_EDGES]);
	Mod_LoadSurfedges	(&header->lumps[LUMP_SURFEDGES]);

	if (!hl_map)
		Mod_LoadQ1Textures	(&header->lumps[LUMP_TEXTURES]);
	else
		Mod_LoadHLTextures	(&header->lumps[LUMP_TEXTURES]);

	if (!hl_map)
		Mod_LoadQ1Lighting	(&header->lumps[LUMP_LIGHTING]);
	else
		Mod_LoadHLLighting	(&header->lumps[LUMP_LIGHTING]);

	Mod_LoadPlanes		(&header->lumps[LUMP_PLANES]);
	Mod_LoadTexinfo		(&header->lumps[LUMP_TEXINFO]);
	Mod_LoadFaces		(&header->lumps[LUMP_FACES]);
	Mod_LoadMarksurfaces(&header->lumps[LUMP_MARKSURFACES]);
	Mod_LoadVisibility	(&header->lumps[LUMP_VISIBILITY]);
	Mod_LoadLeafs		(&header->lumps[LUMP_LEAFS]);
	Mod_LoadNodes		(&header->lumps[LUMP_NODES]);

	if (!hl_map)
		Mod_LoadQ1Clipnodes	(&header->lumps[LUMP_CLIPNODES]);
	else
		Mod_LoadHLClipnodes	(&header->lumps[LUMP_CLIPNODES]);

	Mod_LoadSubmodels	(&header->lumps[LUMP_MODELS]);

	Mod_MakeHull0 ();
	
	mod->numframes = 2;		// regular and alternate animation

	//


//
// set up the submodels (FIXME: this is confusing)
//
	for (i=0 ; i<mod->numsubmodels ; i++)
	{
		bm = &mod->submodels[i];

		mod->hulls[0].firstclipnode = bm->headnode[0];
		for (j=1 ; j<MAX_MAP_HULLS ; j++)
		{
			mod->hulls[j].firstclipnode = bm->headnode[j];
			mod->hulls[j].lastclipnode = mod->numclipnodes-1;
		}
		
		mod->firstmodelsurface = bm->firstface;
		mod->nummodelsurfaces = bm->numfaces;
		
		VectorCopy (bm->maxs, mod->maxs);
		VectorCopy (bm->mins, mod->mins);

		mod->radius = RadiusFromBounds (mod->mins, mod->maxs);

		mod->numleafs = bm->visleafs;

		if (i < mod->numsubmodels-1)
		{	// duplicate the basic information
			char	name[10];

			sprintf (name, "*%i", i+1);
			loadmodel = Mod_FindName (name);
			*loadmodel = *mod;
			strcpy (loadmodel->name, name);
			mod = loadmodel;
		}
	}
}

/*
================
Mod_Print
================
*/
void Mod_Print (void)
{
	int		i;
	model_t	*mod;

	Con_Printf ("Cached models:\n");
	for (i=0, mod=mod_known ; i < mod_numknown ; i++, mod++)
	{
		Con_Printf ("%8p : %s\n",mod->cache.data, mod->name);
	}
}


