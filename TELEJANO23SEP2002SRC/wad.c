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
// wad.c

#include "quakedef.h"

int			wad_numlumps;
lumpinfo_t	*wad_lumps;
byte		*wad_base;

void SwapPic (qpic_t *pic);

/*
==================
W_CleanupName

Lowercases name and pads with spaces and a terminating 0 to the length of
lumpinfo_t->name.
Used so lumpname lookups can proceed rapidly by comparing 4 chars at a time
Space padding is so names can be printed nicely in tables.
Can safely be performed in place.
==================
*/
void W_CleanupName (char *in, char *out)
{
	int		i;
	int		c;
	
	for (i=0 ; i<16 ; i++ )
	{
		c = in[i];
		if (!c)
			break;
			
		if (c >= 'A' && c <= 'Z')
			c += ('a' - 'A');
		out[i] = c;
	}
	
	for ( ; i< 16 ; i++ )
		out[i] = 0;
}



/*
====================
W_LoadWadFile
====================
*/
void W_LoadWadFile (char *filename)
{
	lumpinfo_t		*lump_p;
	wadinfo_t		*header;
	unsigned		i;
	int				infotableofs;

#if 0
	//johnfitz -- modified to use malloc
	//TODO: use cache_alloc
	//TODO: allow for multiple wads
	int		h, len;

	//Draw_BeginDisc ();
	free (wad_base);
	len = COM_OpenFile (filename, &h);
	if (h == -1)
		Sys_Error ("W_LoadWadFile: couldn't load %s", filename);
	wad_base = (unsigned char *)malloc (len);
	Sys_FileRead (h, wad_base, len);                     
	COM_CloseFile (h);
	//Draw_EndDisc ();
	//johnfitz
#else
	wad_base = COM_LoadHunkFile (filename);
	if (!wad_base)
		Sys_Error ("W_LoadWadFile: couldn't load %s", filename);
#endif
	header = (wadinfo_t *)wad_base;
	
	if (header->identification[0] != 'W'
	|| header->identification[1] != 'A'
	|| header->identification[2] != 'D'
	|| header->identification[3] != '2')
		Sys_Error ("Wad file %s doesn't have WAD2 id\n",filename);
		
	wad_numlumps = LittleLong(header->numlumps);
	infotableofs = LittleLong(header->infotableofs);
	wad_lumps = (lumpinfo_t *)(wad_base + infotableofs);
	
	for (i=0, lump_p = wad_lumps ; i<(unsigned)wad_numlumps ; i++,lump_p++)
	{
		lump_p->filepos = LittleLong(lump_p->filepos);
		lump_p->size = LittleLong(lump_p->size);
		W_CleanupName (lump_p->name, lump_p->name);
		if (lump_p->type == TYP_QPIC)
			SwapPic ( (qpic_t *)(wad_base + lump_p->filepos));
	}
}


/*
=============
W_GetLumpinfo
=============
*/
lumpinfo_t	*W_GetLumpinfo (char *name)
{
	int		i;
	lumpinfo_t	*lump_p;
	char	clean[16];
	
	W_CleanupName (name, clean);
	
	for (lump_p=wad_lumps, i=0 ; i<wad_numlumps ; i++,lump_p++)
	{
		if (!strcmp(clean, lump_p->name))
			return lump_p;
	}
	
	Sys_Error ("W_GetLumpinfo: %s not found", name);
	return NULL;
}

void *W_GetLumpName (char *name)
{
	lumpinfo_t	*lump;
	
	lump = W_GetLumpinfo (name);
	
	return (void *)(wad_base + lump->filepos);
}

void *W_GetLumpNum (int num)
{
	lumpinfo_t	*lump;
	
	if (num < 0 || num > wad_numlumps)
		Sys_Error ("W_GetLumpNum: bad number: %i", num);
		
	lump = wad_lumps + num;
	
	return (void *)(wad_base + lump->filepos);
}

/*
=============================================================================

automatic byte swapping

=============================================================================
*/

void SwapPic (qpic_t *pic)
{
	pic->width = LittleLong(pic->width);
	pic->height = LittleLong(pic->height);	
}

#define TEXWAD_MAXIMAGES 16384
typedef struct
{
	char name[16];
	FILE *file;
	int position;
	int size;
} texwadlump_t;

texwadlump_t	texwadlump[TEXWAD_MAXIMAGES];

/*
====================
W_LoadTextureWadFile
====================
*/
void W_LoadTextureWadFile (char *filename, int complain)
{
	lumpinfo_t		*lumps, *lump_p;
	wadinfo_t		header;
	unsigned		i, j;
	int				infotableofs;
	FILE			*file;
	int				numlumps;

	COM_FOpenFile (filename, &file);
	if (!file)
	{
		if (complain)
			Con_Printf ("W_LoadTextureWadFile: couldn't find %s\n", filename);
		return;
	}

	if (fread(&header, sizeof(wadinfo_t), 1, file) != 1)
	{Con_Printf ("W_LoadTextureWadFile: unable to read wad header");return;}
	
	if(header.identification[0] != 'W'
	|| header.identification[1] != 'A'
	|| header.identification[2] != 'D'
	|| header.identification[3] != '3')
	{
		Con_Printf ("W_LoadTextureWadFile: Wad file %s doesn't have WAD3 id\n",filename);
		//return;
	}

	numlumps = LittleLong(header.numlumps);
	if (numlumps < 1 || numlumps > TEXWAD_MAXIMAGES)
	{Con_Printf ("W_LoadTextureWadFile: invalid number of lumps (%i)\n", numlumps);return;}
	infotableofs = LittleLong(header.infotableofs);
	if (fseek(file, infotableofs, SEEK_SET))
	{Con_Printf ("W_LoadTextureWadFile: unable to seek to lump table");return;}
	if (!(lumps = malloc(sizeof(lumpinfo_t)*numlumps)))
	{Con_Printf ("W_LoadTextureWadFile: unable to allocate temporary memory for lump table");return;}

	if (fread(lumps, sizeof(lumpinfo_t), numlumps, file) != (unsigned)numlumps)
	{Con_Printf ("W_LoadTextureWadFile: unable to read lump table");return;}
	
	for (i=0, lump_p = lumps ; i<(unsigned)numlumps ; i++,lump_p++)
	{
		W_CleanupName (lump_p->name, lump_p->name);
		for (j = 0;j < TEXWAD_MAXIMAGES;j++)
		{
			if (texwadlump[j].name[0]) // occupied slot, check the name
			{
				if (!strcmp(lump_p->name, texwadlump[j].name)) // name match, replace old one
					break;
			}
			else // empty slot
				break;
		}
		if (j >= TEXWAD_MAXIMAGES)
			break; // abort loading

		W_CleanupName (lump_p->name, texwadlump[j].name);
		texwadlump[j].file = file;
		texwadlump[j].position = LittleLong(lump_p->filepos);
		texwadlump[j].size = LittleLong(lump_p->disksize);
	}
	free(lumps);
	// leaves the file open
}

byte *W_ConvertWAD3Texture(miptex_t *tex)
{
	byte	*in, *data, *out, *pal;
	int		d, p;

	in		= (byte *)((int) tex + tex->offsets[0]);
	data	= out = malloc(tex->width * tex->height * 4);

	if (!data)
		return NULL;

	image_width		= tex->width;
	image_height	= tex->height;
	pal				= in + (((image_width * image_height) * 85) >> 6);
	pal				+= 2;

	for (d = 0;d < image_width * image_height;d++)
	{
		p = *in++;
		if (tex->name[0] == '{' && p == 255)
			out[0] = out[1] = out[2] = out[3] = 0;
		else
		{
			p *= 3;
			out[0] = pal[p];
			out[1] = pal[p+1];
			out[2] = pal[p+2];
			out[3] = 255;
		}
		out += 4;
	}
	return data;
}

byte *W_GetTexture(char *name)
{
	char		texname[17];
	int			i, j;
	FILE		*file;
	miptex_t	*tex;
	byte		*data;

	texname[16] = 0;

	W_CleanupName (name, texname);

	for (i = 0;i < TEXWAD_MAXIMAGES;i++)
	{
		if (texwadlump[i].name[0])
		{
			if (!strcmp(texname, texwadlump[i].name)) // found it
			{
				file = texwadlump[i].file;
				if (fseek(file, texwadlump[i].position, SEEK_SET))
				{
					Con_Printf("W_GetTexture: corrupt WAD3 file");
					return NULL;
				}

				tex = malloc(texwadlump[i].size);

				if (!tex)
					return NULL;

				if (fread(tex, 1, texwadlump[i].size, file) < (unsigned)texwadlump[i].size)
				{
					Con_Printf("W_GetTexture: corrupt WAD3 file");
					return NULL;
				}

				tex->width = LittleLong(tex->width);
				tex->height = LittleLong(tex->height);
				for (j = 0;j < MIPLEVELS;j++)
					tex->offsets[j] = LittleLong(tex->offsets[j]);

				data = W_ConvertWAD3Texture(tex);
		
				free(tex);

				return data;
			}
		}
		else
			break;
	}
	image_width = image_height = 0;
	return NULL;
}