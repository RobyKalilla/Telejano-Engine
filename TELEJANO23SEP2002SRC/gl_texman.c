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
// gl_texman - this file holds all texture management related functions.
//
#include "quakedef.h"
#include "jpeglib.h"

// Mh! jpeg loader
//#include "jpeglib.h"
// Mh! jpeg loader

typedef struct
{
	int				width;
	int				height;
	int				texnum;
	int				bytesperpixel;
	char			identifier[64];
	qboolean		mipmap;
	unsigned short	crc;
} gltexture_t;

typedef struct
{
	char	*name;
	int		minimize;
	int		maximize;
} glmode_t;

typedef struct showlmp_s
{
	qboolean	isactive;
	int			x;
	int			y;
	char		label[32];
	char		pic[128];
	int			type;		// Tei
	int			typeparam1;	// Tei
	int			typeparam2;	// Tei
	int			typeparam3;	// Tei
	float		alpha;
	qpic_t	*	qpic;
} showlmp_t;


#define SHOWLMP_MAXLABELS 256
#define	MAX_GLTEXTURES	2048

showlmp_t showlmp[SHOWLMP_MAXLABELS];
gltexture_t	gltextures[MAX_GLTEXTURES];

int		numgltextures;
int		texels;
int		gl_filter_min = GL_LINEAR_MIPMAP_LINEAR;
int		gl_filter_max = GL_LINEAR;
int		lhcsumtable[256];
int		image_width;
int		image_height;

glmode_t modes[] = {
	{"GL_NEAREST", GL_NEAREST, GL_NEAREST},
	{"GL_LINEAR", GL_LINEAR, GL_LINEAR},
	{"GL_NEAREST_MIPMAP_NEAREST", GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST},
	{"GL_LINEAR_MIPMAP_NEAREST", GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR},
	{"GL_NEAREST_MIPMAP_LINEAR", GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST},
	{"GL_LINEAR_MIPMAP_LINEAR", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR}
};

/*
===============
Draw_TextureMode_f
Change texture mode, see glmode_t above
===============
*/
void Draw_TextureMode_f (void)
{
	int		i;
	gltexture_t	*glt;

	if (Cmd_Argc() == 1)
	{
		for (i=0 ; i< 6 ; i++)
			if (gl_filter_min == modes[i].minimize)
			{
				Con_Printf ("%s\n", modes[i].name);
				return;
			}
		Con_Printf ("current filter is unknown???\n");
		return;
	}

	for (i=0 ; i< 6 ; i++)
	{
		if (!Q_strcasecmp (modes[i].name, Cmd_Argv(1) ) )
			break;
	}
	if (i == 6)
	{
		Con_Printf ("bad filter name\n");
		return;
	}

	gl_filter_min = modes[i].minimize;
	gl_filter_max = modes[i].maximize;

	// change all the existing mipmap texture objects
	for (i=0, glt=gltextures ; i<numgltextures ; i++, glt++)
	{
		if (glt->mipmap)
		{
			glBindTexture (GL_TEXTURE_2D, glt->texnum);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);
		}
	}
}

void GL_ResetTextures_f( void )
{
	int				i;
	gltexture_t*	t;
	
	for( i = 0, t = gltextures ; i < numgltextures ; i++, t++ )
	{
		glBindTexture( GL_TEXTURE_2D, t->texnum );
		
		if( t->mipmap )
		{
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max );
		}
	}
}

/*
===========
GL_Mipmap
===========
*/
void GL_Mipmap (byte *in, int width, int height)
{
	int		i, j;
	byte	*out;

	width <<=2;
	height >>= 1;
	out = in;
	for (i=0 ; i<height ; i++, in+=width)
	{
		for (j=0 ; j<width ; j+=8, out+=4, in+=8)
		{
			out[0] = (in[0] + in[4] + in[width+0] + in[width+4])>>2;
			out[1] = (in[1] + in[5] + in[width+1] + in[width+5])>>2;
			out[2] = (in[2] + in[6] + in[width+2] + in[width+6])>>2;
			out[3] = (in[3] + in[7] + in[width+3] + in[width+7])>>2;
		}
	}
}

void R_ResampleTextureLerpLine (byte *in, byte *out, int inwidth, int outwidth)
{
	int		j, xi, oldx, f, fstep, endx;

	oldx	= 0;
	fstep	= (int) (inwidth * 65536.0f / outwidth);
	endx	= (inwidth - 1);

	for (j = 0,f = 0;j < outwidth;j++, f += fstep)
	{
		xi = (int) f >> 16;
		if (xi != oldx)
		{
			in += (xi - oldx) * 4;
			oldx = xi;
		}
		if (xi < endx)
		{
			int lerp = f & 0xFFFF;
			*out++ = (byte) ((((in[4] - in[0]) * lerp) >> 16) + in[0]);
			*out++ = (byte) ((((in[5] - in[1]) * lerp) >> 16) + in[1]);
			*out++ = (byte) ((((in[6] - in[2]) * lerp) >> 16) + in[2]);
			*out++ = (byte) ((((in[7] - in[3]) * lerp) >> 16) + in[3]);
		}
		else
		{
			*out++ = in[0];
			*out++ = in[1];
			*out++ = in[2];
			*out++ = in[3];
		}
	}
}

/*
================
R_ResampleTexture
================
*/
void R_ResampleTexture (void *indata, int inwidth, int inheight, void *outdata,  int outwidth, int outheight)
{
	int		i, j, yi, oldy, f, fstep, endy = (inheight-1);
	byte	*inrow, *out, *row1, *row2;

	out		= outdata;
	fstep	= (int)(inheight * 65536.0f / outheight);

	row1	= malloc(outwidth * 4);
	row2	= malloc(outwidth * 4);

	inrow	= indata;
	oldy	= 0;

	R_ResampleTextureLerpLine (inrow,				row1, inwidth, outwidth);
	R_ResampleTextureLerpLine (inrow + inwidth * 4, row2, inwidth, outwidth);

	for (i=0, f=0; i<outheight; i++, f+=fstep)
	{
		yi = f >> 16;
		if (yi < endy)
		{
			int lerp = f & 0xFFFF;

			if (yi != oldy)
			{
				inrow = (byte *)indata + inwidth * 4 * yi;
				if (yi == oldy+1)
				{
					memcpy(row1, row2, outwidth * 4);
				}
				else
				{
					R_ResampleTextureLerpLine (inrow, row1, inwidth, outwidth);
				}

				R_ResampleTextureLerpLine (inrow + inwidth*4, row2, inwidth, outwidth);
				oldy = yi;
			}

			j = outwidth - 4;

			while(j >= 0)
			{
				out[ 0] = (byte) ((((row2[ 0] - row1[ 0]) * lerp) >> 16) + row1[ 0]);
				out[ 1] = (byte) ((((row2[ 1] - row1[ 1]) * lerp) >> 16) + row1[ 1]);
				out[ 2] = (byte) ((((row2[ 2] - row1[ 2]) * lerp) >> 16) + row1[ 2]);
				out[ 3] = (byte) ((((row2[ 3] - row1[ 3]) * lerp) >> 16) + row1[ 3]);
				out[ 4] = (byte) ((((row2[ 4] - row1[ 4]) * lerp) >> 16) + row1[ 4]);
				out[ 5] = (byte) ((((row2[ 5] - row1[ 5]) * lerp) >> 16) + row1[ 5]);
				out[ 6] = (byte) ((((row2[ 6] - row1[ 6]) * lerp) >> 16) + row1[ 6]);
				out[ 7] = (byte) ((((row2[ 7] - row1[ 7]) * lerp) >> 16) + row1[ 7]);
				out[ 8] = (byte) ((((row2[ 8] - row1[ 8]) * lerp) >> 16) + row1[ 8]);
				out[ 9] = (byte) ((((row2[ 9] - row1[ 9]) * lerp) >> 16) + row1[ 9]);
				out[10] = (byte) ((((row2[10] - row1[10]) * lerp) >> 16) + row1[10]);
				out[11] = (byte) ((((row2[11] - row1[11]) * lerp) >> 16) + row1[11]);
				out[12] = (byte) ((((row2[12] - row1[12]) * lerp) >> 16) + row1[12]);
				out[13] = (byte) ((((row2[13] - row1[13]) * lerp) >> 16) + row1[13]);
				out[14] = (byte) ((((row2[14] - row1[14]) * lerp) >> 16) + row1[14]);
				out[15] = (byte) ((((row2[15] - row1[15]) * lerp) >> 16) + row1[15]);
				out		+= 16;
				row1	+= 16;
				row2	+= 16;
				j		-= 4;
			}
			if (j & 2)
			{
				out[ 0] = (byte) ((((row2[ 0] - row1[ 0]) * lerp) >> 16) + row1[ 0]);
				out[ 1] = (byte) ((((row2[ 1] - row1[ 1]) * lerp) >> 16) + row1[ 1]);
				out[ 2] = (byte) ((((row2[ 2] - row1[ 2]) * lerp) >> 16) + row1[ 2]);
				out[ 3] = (byte) ((((row2[ 3] - row1[ 3]) * lerp) >> 16) + row1[ 3]);
				out[ 4] = (byte) ((((row2[ 4] - row1[ 4]) * lerp) >> 16) + row1[ 4]);
				out[ 5] = (byte) ((((row2[ 5] - row1[ 5]) * lerp) >> 16) + row1[ 5]);
				out[ 6] = (byte) ((((row2[ 6] - row1[ 6]) * lerp) >> 16) + row1[ 6]);
				out[ 7] = (byte) ((((row2[ 7] - row1[ 7]) * lerp) >> 16) + row1[ 7]);
				out		+= 8;
				row1	+= 8;
				row2	+= 8;
			}
			if (j & 1)
			{
				out[ 0] = (byte) ((((row2[ 0] - row1[ 0]) * lerp) >> 16) + row1[ 0]);
				out[ 1] = (byte) ((((row2[ 1] - row1[ 1]) * lerp) >> 16) + row1[ 1]);
				out[ 2] = (byte) ((((row2[ 2] - row1[ 2]) * lerp) >> 16) + row1[ 2]);
				out[ 3] = (byte) ((((row2[ 3] - row1[ 3]) * lerp) >> 16) + row1[ 3]);
				out		+= 4;
				row1	+= 4;
				row2	+= 4;
			}
			row1 -= outwidth * 4;
			row2 -= outwidth * 4;
		}
		else
		{
			if (yi != oldy)
			{
				inrow = (byte *)indata + inwidth * 4 * yi;
				
				if (yi == oldy+1)
				{
					memcpy(row1, row2, outwidth * 4);
				}
				else
				{
					R_ResampleTextureLerpLine (inrow, row1, inwidth, outwidth);
				}
				oldy = yi;
			}
			memcpy(out, row1, outwidth * 4);
		}
	}
	free(row1);
	free(row2);
}

/*
================
GL_UploadMipmaps
================
*/
void GL_UploadMipmaps (unsigned *data, int width, int height, qboolean alpha)
{
	static unsigned	scaled[1024 * 1024 * 4];
	int				scaled_width, scaled_height, type, miplevel;

	type		= alpha ? gl_alpha_format : gl_solid_format;
	miplevel	= 0;

	for (scaled_width  = 2; scaled_width  < width;  scaled_width  <<= 1);
	for (scaled_height = 2; scaled_height < height; scaled_height <<= 1);

	if (scaled_width  > gl_max_size.value)
		scaled_width  = gl_max_size.value;
	if (scaled_height > gl_max_size.value)
		scaled_height = gl_max_size.value;

	if (scaled_width  > 1024)
		scaled_width  = 1024;
	if (scaled_height > 1024)
		scaled_height = 1024;

	if (scaled_width != width || scaled_height != height)
	{
		R_ResampleTexture (data, width, height, scaled, scaled_width, scaled_height);
	}
	else
	{
		memcpy (scaled, data, width * height * 4);
	}

	glTexImage2D (GL_TEXTURE_2D, 0, type, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, scaled);

	while (scaled_width > 1 || scaled_height > 1)
	{
		GL_Mipmap ((byte *)scaled, scaled_width, scaled_height);

		scaled_width  >>= 1;
		scaled_height >>= 1;

		if (scaled_width  < 1)
			scaled_width  = 1;
		if (scaled_height < 1)
			scaled_height = 1;

		miplevel++;

		glTexImage2D (GL_TEXTURE_2D, miplevel, type, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, scaled);
	}
	
	glTexParameterf	(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min);
	glTexParameterf	(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);
}

/*
=========
GL_Upload
=========
*/
void GL_Upload (unsigned *data, int width, int height, qboolean alpha)
{
	static unsigned	scaled[1024 * 1024 * 4];
	int				scaled_width, scaled_height, type;

	type = alpha ? gl_alpha_format : gl_solid_format;

	for (scaled_width  = 2; scaled_width  < width;  scaled_width  <<= 1);
	for (scaled_height = 2; scaled_height < height; scaled_height <<= 1);

	if (scaled_width  > gl_max_size.value)
		scaled_width  = gl_max_size.value;
	if (scaled_height > gl_max_size.value)
		scaled_height = gl_max_size.value;

	if (scaled_width  > 1024)
		scaled_width  = 1024;
	if (scaled_height > 1024)
		scaled_height = 1024;

	if (scaled_width != width || scaled_height != height)
	{
		R_ResampleTexture (data, width, height, scaled, scaled_width, scaled_height);

		glTexImage2D	(GL_TEXTURE_2D, 0, type, scaled_width, scaled_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, scaled);
		
		glTexParameterf	(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_max);
		glTexParameterf	(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);

		return;
	}
	
	glTexImage2D	(GL_TEXTURE_2D, 0, type, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	
	glTexParameterf	(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_max);
	glTexParameterf	(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);
}

/*
==========
GL_Upload8
==========
*/
void GL_Upload8 (byte *data, int width, int height, qboolean mipmap, qboolean alpha)
{
	static unsigned int	trans[0x40000];
	int					s = width * height;
	int					i;
	qboolean			noalpha = true;

	for( i = 0 ; i < s && i < 0x40000 ; ++i )
	{
		trans[i]	= d_8to24table[data[i]];

		if( data[i] == 255 )
		{
			noalpha	= false;
		}
	}

	if(!mipmap)
	{
		GL_Upload (trans, width, height, (alpha && !noalpha));
		return;
	}

	GL_UploadMipmaps (trans, width, height, (alpha && !noalpha));
}

int argh, argh2;

/*
==============
GL_LoadTexture
==============
*/
int GL_LoadTexture (char *identifier, int width, int height, byte *data, qboolean mipmap, qboolean alpha, int bytesperpixel)
{
	int				i;
	gltexture_t		*glt;
	unsigned short	crc;

	if (!identifier[0])
	{
		Con_Printf("GL_LoadTexture: no identifier\n");
		sprintf (identifier, "%s_%i", "argh", argh);
		argh++;
	}

	crc = CRC_Block(data, width*height*bytesperpixel);

	for (i=0, glt=gltextures ; i < numgltextures ; i++, glt++)
	{
		if (!strcmp (identifier, glt->identifier))
		{
			if (crc != glt->crc || width != glt->width || height != glt->height)
			{
				Con_DPrintf("GL_LoadTexture: cache mismatch\n");
				sprintf (identifier, "%s_%i", identifier, argh2);
				argh2++;
				goto GL_LoadTexture_setup;
			}
			return glt->texnum;
		}
	}

GL_LoadTexture_setup:

	glt = &gltextures[numgltextures];
	numgltextures++;

	strcpy (glt->identifier, identifier);
	glt->texnum = texture_extension_number;
	texture_extension_number++;

	glt->crc			= crc;
	glt->width			= width;
	glt->height			= height;
	glt->mipmap			= mipmap;
	glt->bytesperpixel	= bytesperpixel;

	if (!isDedicated)
	{
		glBindTexture (GL_TEXTURE_2D, glt->texnum);
		
		if (bytesperpixel == 1)
		{
			GL_Upload8 (data, width, height, mipmap, alpha);
		}
		else if (bytesperpixel == 4)
		{
			if (mipmap)
			{
				GL_UploadMipmaps ((void *)data, width, height, alpha);
			}
			else
			{
				GL_Upload ((void *)data, width, height, alpha);
			}
		}
		else
		{
			Sys_Error("GL_LoadTexture: unknown bytesperpixel\n");
		}
	}

	return glt->texnum;
}

/****************************************/


// MH! Jpg loader

/*
============
LoadJPG
============
*/
/*
byte *LoadJPG (FILE *f, int *width, int *height)
{
    struct jpeg_decompress_struct cinfo;
    JDIMENSION num_scanlines;
    JSAMPARRAY in;
    struct jpeg_error_mgr jerr;
    int numPixels;
    int row_stride;
    byte *out;
    int count;
    int i;

    // set up the decompression.
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress (&cinfo);

    // inititalize the source
    jpeg_stdio_src (&cinfo, f);

    // initialize decompression
    (void) jpeg_read_header (&cinfo, TRUE);
    (void) jpeg_start_decompress (&cinfo);

    // set up the width and height for return
    *width = cinfo.image_width;
    *height = cinfo.image_height;

    // initialize the return data
    numPixels = (*width) * (*height);
    jpeg_rgba = malloc ((numPixels * 4));

    // initialize the input buffer - we'll use the in-built memory management routines in the
    // JPEG library because it will automatically free the used memory for us when we destroy
    // the decompression structure. cool.
    row_stride = cinfo.output_width * cinfo.output_components;
    in = (*cinfo.mem->alloc_sarray) ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

    // bit of error checking
    if (cinfo.output_components != 3)
        goto error;

    if ((numPixels * 4) != ((row_stride * cinfo.output_height) + numPixels))
        goto error;

    // read the jpeg
    count = 0;

    while (cinfo.output_scanline < cinfo.output_height) 
    {
        num_scanlines = jpeg_read_scanlines(&cinfo, in, 1);
        out = in[0];

        for (i = 0; i < row_stride;)
        {
            jpeg_rgba[count++] = out[i++];
            jpeg_rgba[count++] = out[i++];
            jpeg_rgba[count++] = out[i++];
            jpeg_rgba[count++] = 255;
        }
    }

    // finish decompression and destroy the jpeg
    (void) jpeg_finish_decompress (&cinfo);
    jpeg_destroy_decompress (&cinfo);
    return jpeg_rgba;

error:
    Con_DPrintf ("Invalid JPEG Format\n");
    jpeg_destroy_decompress (&cinfo);
    return NULL;
}

*/
/*
int LoadJPG (int *width, int *height, qboolean complain, qboolean blah)
{
	struct jpeg_decompress_struct cinfo;
	JDIMENSION num_scanlines;
	JSAMPARRAY in;
	struct jpeg_error_mgr jerr;
	int numPixels;
	int row_stride;
	byte *out;
	int count;
	int i;
	int r, g, b;

	// set up the decompression.
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress (&cinfo);

	// inititalize the source
	jpeg_stdio_src (&cinfo, file32bit);

	// initialize decompression
	(void) jpeg_read_header (&cinfo, TRUE);
	(void) jpeg_start_decompress (&cinfo);

	// set up the width and height for return
	if (complain)
	{
		if (*width != cinfo.image_width) return 0;
		if (*height != cinfo.image_height) return 0;
	}

	*width = cinfo.image_width;
	*height = cinfo.image_height;
	numPixels = (*width) * (*height);

	// initialize the input buffer - we'll use the in-built memory management routines in the
	// JPEG library because it will automatically free the used memory for us when we destroy
	// the decompression structure.  cool.
	row_stride = cinfo.output_width * cinfo.output_components;
	in = (*cinfo.mem->alloc_sarray) ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

	// bit of error checking
	if (cinfo.output_components != 3)
		goto error;

	if ((numPixels * 4) != ((row_stride * cinfo.output_height) + numPixels))
		goto error;

	// read the jpeg
	count = 0;

	// initialize the return data
	image_rgba = malloc ((numPixels * 4));

	while (cinfo.output_scanline < cinfo.output_height) 
	{
		num_scanlines = jpeg_read_scanlines(&cinfo, in, 1);
		out = in[0];

		for (i = 0; i < row_stride;)
		{
			r = image_rgba[count++] = out[i++];
			g = image_rgba[count++] = out[i++];
			b = image_rgba[count++] = out[i++];
			image_rgba[count++] = 255;
		}
	}

	// finish decompression and destroy the jpeg
	(void) jpeg_finish_decompress (&cinfo);
	jpeg_destroy_decompress (&cinfo);

	return 1;

error:
	// this should rarely (if ever) happen, but just in case...
	Con_DPrintf ("Invalid JPEG Format\n");
	jpeg_destroy_decompress (&cinfo);

	return 0;
}
*/
// MH! Jpg loader


// Tomaz - TGA Begin

/*
===========
PCX Loading
===========
*/

typedef struct
{
	char		manufacturer;
	char		version;
	char		encoding;
	char		bits_per_pixel;
	unsigned short	xmin,ymin,xmax,ymax;
	unsigned short	hres,vres;
	unsigned char	palette[48];
	char		reserved;
	char		color_planes;
	unsigned short	bytes_per_line;
	unsigned short	palette_type;
	char		filler[58];
	unsigned 	data;			// unbounded
} pcx_t;



/*
=======
LoadPCX
=======
*/
byte* LoadPCX (FILE *f, char *name)
{
	pcx_t	*pcx, pcxbuf;
	byte	palette[768];
	byte	*pix, *image_rgba;
	int		x, y;
	int		dataByte, runLength;
	int		count;




//
// parse the PCX file
//
	fread (&pcxbuf, 1, sizeof(pcxbuf), f);

	pcx = &pcxbuf;

	if (pcx->manufacturer	!= 0x0a	|| 
		pcx->version		!= 5	|| 
		pcx->encoding		!= 1	|| 
		pcx->bits_per_pixel != 8	||
		// Allow up to 1024X1024 pcx textures, i don't know what negative side effects
		// this will, or can have, but it works. 3dfx users will have to go without it
		// though.
		pcx->xmax			> 1024	||	// was 320 
		pcx->ymax			> 1024)	// was 256
	{
		Con_Printf ("%s Bad pcx file\n", name);
		return NULL;
	}

	// seek to palette
	fseek (f, -768, SEEK_END);
	fread (palette, 1, 768, f);

	fseek (f, sizeof(pcxbuf) - 4, SEEK_SET);

	count = (pcx->xmax+1) * (pcx->ymax+1);
	image_rgba = malloc( count * 4);

	for (y=0 ; y<=pcx->ymax ; y++)
	{
		pix = image_rgba + 4*y*(pcx->xmax+1);
		for (x=0 ; x<=pcx->xmax ; )
		{
			dataByte = fgetc(f);

			if((dataByte & 0xC0) == 0xC0)
			{
				runLength = dataByte & 0x3F;
				dataByte = fgetc(f);
			}
			else
				runLength = 1;

			while(runLength-- > 0)
			{
				pix[0] = palette[dataByte*3];
				pix[1] = palette[dataByte*3+1];
				pix[2] = palette[dataByte*3+2];
				pix[3] = 255;
				pix += 4;
				x++;
			}
		}
	}
	fclose(f);
	image_width = pcx->xmax+1;
	image_height = pcx->ymax+1;
	return image_rgba;
}

/*
=============
TARGA LOADING
=============
*/

typedef struct _TargaHeader 
{
	unsigned char 	id_length, colormap_type, image_type;
	unsigned short	colormap_index, colormap_length;
	unsigned char	colormap_size;
	unsigned short	x_origin, y_origin, width, height;
	unsigned char	pixel_size, attributes;
} TargaHeader;


TargaHeader		targa_header;

int fgetLittleShort (FILE *f)
{
	byte	b1, b2;

	b1 = fgetc(f);
	b2 = fgetc(f);

	return (short)(b1 + b2*256);
}

int fgetLittleLong (FILE *f)
{
	byte	b1, b2, b3, b4;

	b1 = fgetc(f);
	b2 = fgetc(f);
	b3 = fgetc(f);
	b4 = fgetc(f);

	return b1 + (b2<<8) + (b3<<16) + (b4<<24);
}


//RandomMan jpg
byte *LoadJPG (FILE *f, char *name)
{
    struct jpeg_decompress_struct cinfo;
    JDIMENSION num_scanlines;
    JSAMPARRAY in;
    struct jpeg_error_mgr jerr;
    int numPixels;
    int row_stride;
    byte *out;
    int count;
    int i;
    int r, g, b;
    byte *image_rgba;

    // set up the decompression.
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress (&cinfo);

    // inititalize the source
    jpeg_stdio_src (&cinfo, f);

    // initialize decompression
    (void) jpeg_read_header (&cinfo, TRUE);
    (void) jpeg_start_decompress (&cinfo);

    numPixels = cinfo.image_width * cinfo.image_height;

    // initialize the input buffer - we'll use the in-built memory management routines in the
    // JPEG library because it will automatically free the used memory for us when we destroy
    // the decompression structure.  cool.
    row_stride = cinfo.output_width * cinfo.output_components;
    in = (*cinfo.mem->alloc_sarray) ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

    // bit of error checking
    if (cinfo.output_components != 3)
        goto error;


    // initialize the return data
    image_rgba = malloc ((numPixels * 4));

    // read the jpeg
    count = 0;

    while (cinfo.output_scanline < cinfo.output_height) 
    {
        num_scanlines = jpeg_read_scanlines(&cinfo, in, 1);
        out = in[0];

        for (i = 0; i < row_stride;)
        {
            r = image_rgba[count++] = out[i++];
            g = image_rgba[count++] = out[i++];
            b = image_rgba[count++] = out[i++];
            image_rgba[count++] = 255;
        }
    }

    // finish decompression and destroy the jpeg
    image_width = cinfo.image_width;
    image_height = cinfo.image_height;

    (void) jpeg_finish_decompress (&cinfo);
    jpeg_destroy_decompress (&cinfo);

    fclose (f);
    return image_rgba;

error:
    // this should rarely (if ever) happen, but just in case...
    Con_DPrintf ("Invalid JPEG Format file %s\n", name);
    jpeg_destroy_decompress (&cinfo);

    fclose (f);
    return NULL;
} 

//RandomMan jpg

/*
=======
LoadTGA
=======
*/
byte* LoadTGA (FILE *fin, char *name)
{
	int		columns, rows, numPixels;
	byte	*pixbuf;
	int		row, column;
	byte	*image_rgba;

	targa_header.id_length			= fgetc(fin);
	targa_header.colormap_type		= fgetc(fin);
	targa_header.image_type			= fgetc(fin);
	
	targa_header.colormap_index		= fgetLittleShort(fin);
	targa_header.colormap_length	= fgetLittleShort(fin);
	targa_header.colormap_size		= fgetc(fin);
	targa_header.x_origin			= fgetLittleShort(fin);
	targa_header.y_origin			= fgetLittleShort(fin);
	targa_header.width				= fgetLittleShort(fin);
	targa_header.height				= fgetLittleShort(fin);

	targa_header.pixel_size			= fgetc(fin);
	targa_header.attributes			= fgetc(fin);

	if (targa_header.image_type!=2 && targa_header.image_type!=10) 
		Host_Error ("LoadTGA: %s Only type 2 and 10 targa RGB images supported\n", name);

	if (targa_header.colormap_type !=0 || (targa_header.pixel_size!=32 && targa_header.pixel_size!=24))
		Host_Error ("LoadTGA: %s Only 32 or 24 bit images supported (no colormaps)\n", name);

	columns = targa_header.width;
	rows = targa_header.height;
	numPixels = columns * rows;

	image_rgba = malloc (numPixels*4);
	
	if (targa_header.id_length != 0)
		fseek(fin, targa_header.id_length, SEEK_CUR);  // skip TARGA image comment
	
	if (targa_header.image_type==2) 
	{  // Uncompressed, RGB images
		for(row=rows-1; row>=0; row--) 
		{
			pixbuf = image_rgba + row*columns*4;
			for(column=0; column<columns; column++) 
			{
				unsigned char red = 0,green = 0,blue = 0,alphabyte = 0;
				switch (targa_header.pixel_size) 
				{
					case 24:
							
							blue		= getc(fin);
							green		= getc(fin);
							red			= getc(fin);
							pixbuf[0]	= red;
							pixbuf[1]	= green;
							pixbuf[2]	= blue;
							pixbuf[3]	= 255;
							pixbuf		+= 4;
							break;
					case 32:
							blue		= getc(fin);
							green		= getc(fin);
							red			= getc(fin);
							alphabyte	= getc(fin);
							pixbuf[0]	= red;
							pixbuf[1]	= green;
							pixbuf[2]	= blue;
							pixbuf[3]	= alphabyte;
							pixbuf		+= 4;
							break;
				}
			}
		}
	}
	else if (targa_header.image_type==10) 
	{   // Runlength encoded RGB images
		unsigned char red = 0,green = 0,blue = 0,alphabyte = 0,packetHeader,packetSize,j;
		for(row=rows-1; row>=0; row--) 
		{
			pixbuf = image_rgba + row*columns*4;
			for(column=0; column<columns; ) 
			{
				packetHeader=getc(fin);
				packetSize = 1 + (packetHeader & 0x7f);
				if (packetHeader & 0x80) 
				{        // run-length packet
					switch (targa_header.pixel_size) 
					{
						case 24:
								blue		= getc(fin);
								green		= getc(fin);
								red			= getc(fin);
								alphabyte	= 255;
								break;
						case 32:
								blue		= getc(fin);
								green		= getc(fin);
								red			= getc(fin);
								alphabyte	= getc(fin);
								break;
					}
					for(j=0;j<packetSize;j++) 
					{
						pixbuf[0]	= red;
						pixbuf[1]	= green;
						pixbuf[2]	= blue;
						pixbuf[3]	= alphabyte;
						pixbuf		+= 4;
						column++;
						if (column==columns) 
						{ // run spans across rows
							column=0;
							if (row>0)
								row--;
							else
								goto breakOut;
							pixbuf = image_rgba + row*columns*4;
						}
					}
				}
				else 
				{	// non run-length packet
					for(j=0;j<packetSize;j++) 
					{
						switch (targa_header.pixel_size)
						{
							case 24:
									blue		= getc(fin);
									green		= getc(fin);
									red			= getc(fin);
									pixbuf[0]	= red;
									pixbuf[1]	= green;
									pixbuf[2]	= blue;
									pixbuf[3]	= 255;
									pixbuf		+= 4;
									break;
							case 32:
									blue		= getc(fin);
									green		= getc(fin);
									red			= getc(fin);
									alphabyte	= getc(fin);
									pixbuf[0]	= red;
									pixbuf[1]	= green;
									pixbuf[2]	= blue;
									pixbuf[3]	= alphabyte;
									pixbuf		+= 4;
									break;
						}
						column++;
						if (column==columns) 
						{ // pixel packet run spans across rows
							column=0;
							if (row>0)
								row--;
							else
								goto breakOut;
							pixbuf = image_rgba + row*columns*4;
						}
					}
				}
			}
			breakOut:;
		}
	}
	
	fclose(fin);
	image_width = columns;
	image_height = rows;
	return image_rgba;
}

cvar_t debug_textures =  {"debug_textures","0"};

extern cvar_t gl_themefolder;//XFX


//def for shiny.pcx
//#include "tex_shiny.c"
// shiny here
#if 0

//Dont work!
byte* LoadBUILTIN (byte * data)
{
	pcx_t	*pcx, pcxbuf;
	byte	palette[768];
	byte	*pix, *image_rgba;
	int		x, y;
	int		dataByte, runLength;
	int		count;
	int		dx=0;

	Con_Printf("hello from BUILTIN function working here\n");

	pcx = (struct pcx_t *)data;

	if (pcx->manufacturer	!= 0x0a	|| 
		pcx->version		!= 5	|| 
		pcx->encoding		!= 1	|| 
		pcx->bits_per_pixel != 8	||
		// Allow up to 1024X1024 pcx textures, i don't know what negative side effects
		// this will, or can have, but it works. 3dfx users will have to go without it
		// though.
		pcx->xmax			> 1024	||	// was 320 
		pcx->ymax			> 1024)	// was 256
	{
		Con_Printf ("Bad pcx builtin\n");
		return NULL;
	}


	Con_Printf("PCX data loaded: xmx %d ymx %d", pcx->xmax, pcx->xmin);

	// seek to palette

	//palette = (byte *)data; 
	memcpy(palette,data,sizeof(palette)-1);

	//fseek (f, sizeof(pcxbuf) - 4, SEEK_SET);

	dx = sizeof(pcxbuf) - 4;

	count = (pcx->xmax+1) * (pcx->ymax+1);
	image_rgba = malloc( count * 4);

	for (y=0 ; y<=pcx->ymax ; y++)
	{
		pix = image_rgba + 4*y*(pcx->xmax+1);
		for (x=0 ; x<=pcx->xmax ; )
		{
			dataByte = data[dx++];//fgetc(f);

			if((dataByte & 0xC0) == 0xC0)
			{
				runLength = dataByte & 0x3F;
				dataByte = data[dx++];;
			}
			else
				runLength = 1;

			while(runLength-- > 0)
			{
				pix[0] = palette[dataByte*3];
				pix[1] = palette[dataByte*3+1];
				pix[2] = palette[dataByte*3+2];
				pix[3] = 255;
				pix += 4;
				x++;
			}
		}
	}
	image_width = pcx->xmax+1;
	image_height = pcx->ymax+1;
	return image_rgba;
}
#endif 

byte * LoadPNG (FILE *fin,char * name) ;

byte* loadimagepixels (char* filename, qboolean complain)
{
	FILE	*f;
	char	basename[128], name[128];
	byte	*c;
	//int		bighack = 855;//hack hack hack

	COM_StripExtension(filename, basename); // strip the extension to allow TGA and PCX

	for (c = basename;*c;c++)
		if (*c == '*')
			*c = '#';

	//sprintf (name, "textures/%s.tga", basename);
	sprintf (name, "%s/%s.tga", gl_themefolder.string,basename);//XFX
	COM_FOpenFile (name, &f);
	if (f)
		return LoadTGA (f, name);

//RandomMan
	sprintf (name, "%s/%s.jpg", gl_themefolder.string,basename);//XFX
	COM_FOpenFile (name, &f);
	if (f)
		return LoadJPG (f, name);
//RandomMan

	//sprintf (name, "textures/%s.pcx", basename);

#if 0 //Builtin detail texture, dont work!
	if (!strcmp(filename, "textures/shiny"))
	{
		Con_Printf("Built-in shiny loaded.\n");
		return LoadBUILTIN(shiny);
	}
#endif

	
	sprintf (name, "%s/%s.pcx", gl_themefolder.string,basename);//XFX

	COM_FOpenFile (name, &f);
	if (f)
		return LoadPCX (f, name);

//Tei png support
	sprintf (name, "%s/%s.png", gl_themefolder.string,basename);//XFX
	COM_FOpenFile (name, &f);
	if (f)
		return LoadPNG (f, name);
//Tei png support

	sprintf (name, "%s.tga", basename);
	COM_FOpenFile (name, &f);
	if (f)
		return LoadTGA (f, name);

	sprintf (name, "%s.pcx", basename);
	COM_FOpenFile (name, &f);
	if (f)
		return LoadPCX (f, name);

//Tei png support
	sprintf (name, "%s.png", basename);
	COM_FOpenFile (name, &f);
	if (f)
		return LoadPNG (f, name);
//Tei png support


//RandomMan
  sprintf (name, "%s.jpg", basename);
    COM_FOpenFile (name, &f);
    if (f)
        return LoadJPG (f, name); 
//RandomMan


	if (complain || debug_textures.value) // Tei debugtextures
		Con_Printf ("Couldn't load '%s' as tga,pcx,png or jpg file\n", filename);
	return NULL;
}

int loadtextureimage (char* filename, qboolean complain, qboolean mipmap)
{
	int			j, texnum;
	byte		*data;
	qboolean	transparent;

	if (isDedicated)
		return 0;

	transparent	= false;

	data = loadimagepixels (filename, complain);

	if(!data)
		return 0;

	for (j = 0;j < image_width*image_height;j++)
	{
		if (data[j*4+3] < 255)
		{
			transparent = true;
			break;
		}
	}

	texnum = GL_LoadTexture (filename, image_width, image_height, data, mipmap, transparent, 4);

	free(data);

	return texnum;
}

int loadtextureimage2 (char* filename, qboolean complain, qboolean mipmap)
{
	int			j, texnum, i;
	byte		*data;
	qboolean	transparent;

	transparent	= false;

	data = loadimagepixels (filename, complain);

	if(!data)
		return 0;

	for (j = 0;j < image_width*image_height;j++)
	{
		if (data[j*4+3] < 255)
		{
			transparent = true;
			break;
		}
	}

	for (j = 0;j < 32;j++)
	{
		for (i = 0;i < 32;i++)
		{
			if (i == 0 && j == 0)
				Con_Printf("%s\n", filename);

			if (i == 0)
				Con_Printf("{");

			if (i == 31)
				Con_Printf("%3i},\n", data[(j*32+i)*4+3]);
			else
				Con_Printf("%3i,", data[(j*32+i)*4+3]);
		}
	}

	texnum = GL_LoadTexture (filename, image_width, image_height, data, mipmap, transparent, 4);

	free(data);

	return texnum;
}

int loadtextureimage3 (char* filename, qboolean complain, qboolean mipmap, byte *data)
{
	int			j, texnum;
	qboolean	transparent;

	if (isDedicated)
		return 0;

	transparent	= false;

	data = loadimagepixels (filename, complain);

	if(!data)
		return 0;

	for (j = 0;j < image_width*image_height;j++)
	{
		if (data[j*4+3] < 255)
		{
			transparent = true;
			break;
		}
	}

	texnum = GL_LoadTexture (filename, image_width, image_height, data, mipmap, transparent, 4);

	return texnum;
}

// Tomaz - TGA End

// Tomaz - ShowLMP Begin
void HIDELMP()
{
	int		i;
	byte	*lmplabel;

	lmplabel = MSG_ReadString();
	for (i = 0;i < SHOWLMP_MAXLABELS;i++)
	{
		if (showlmp[i].isactive && strcmp(showlmp[i].label, lmplabel) == 0)
		{
			showlmp[i].isactive = false;
			return;
		}
	}
}

// Tei new lmp 
#define ORG_NW 0
#define ORG_NE 1
#define ORG_SW 2
#define ORG_SE 3
#define ORG_CC 4
#define ORG_CN 5
#define ORG_CS 6
#define ORG_CW 7
#define ORG_CE 8

#define CNT_ARRAY		1
#define CNT_STRING		2
#define CNT_ALPHA		3
#define CNT_DEFAULT		0
#define CNT_HUDTMP		4
// Tei new lmp 




void RecalcXY ( float *xx, float *yy, int origin )
{
	int midx, midy;
	float x,y;

	x = xx[0];
	y = yy[0];

	midy = vid.height * 0.5;// >>1
	midx = vid.width * 0.5;// >>1

	// Tei - new showlmp
	switch ( origin ) 
	{
		case ORG_NW:
			break;
		case ORG_NE:
			x = vid.width - x;//Inv
			break;
		case ORG_SW:
			y = vid.height - y;//Inv
			break;
		case ORG_SE:
			y = vid.height - y;//inv
			x = vid.width - x;//Inv
			break;
		case ORG_CC:
			y = midy + (y - 8000);//NegCoded
			x = midx + (x - 8000);//NegCoded
			break;
		case ORG_CN:
			x = midx + (x - 8000);//NegCoded
			break;
		case ORG_CS:
			x = midx + (x - 8000);//NegCoded
			y = vid.height - y;//Inverse
			break;
		case ORG_CW:
			y = midy + (y - 8000);//NegCoded
			break;
		case ORG_CE:
			y = midy + (y - 8000);//NegCoded
			x = vid.height - x; //Inverse
			break;
		default:
			break;
	}

	xx[0] = x;
	yy[0] = y;
}

//Tomaz showlmp
void SHOWLMP( int origin )
{
	int i, k;
	byte lmplabel[256], picname[256];
	float x, y;

	strcpy(lmplabel,MSG_ReadString());
	strcpy(picname, MSG_ReadString());
	x = MSG_ReadShort();
	y = MSG_ReadShort();
	
	RecalcXY( &x,&y,origin);//Tei origins

	k = -1;
	for (i = 0;i < SHOWLMP_MAXLABELS;i++)
	{
		if (showlmp[i].isactive)
		{
			if (strcmp(showlmp[i].label, lmplabel) == 0)
			{
				k = i;
				break; // drop out to replace it
			}
		}
		else if (k < 0) // find first empty one to replace
			k = i;
	}

	if (k < 0)
		return;

	// change existing one
	showlmp[k].isactive = true;
	strcpy(showlmp[k].label, lmplabel);
	strcpy(showlmp[k].pic, picname);
	showlmp[k].x = x;
	showlmp[k].y = y;
	showlmp[k].type = CNT_DEFAULT;//Tei

}
//Tomaz showlmp



void UpdateLmp(void)
{
	int		i;
	byte	*lmplabel;

	lmplabel = MSG_ReadString();

	for (i = 0;i < SHOWLMP_MAXLABELS;i++)
	{
		if (showlmp[i].isactive && strcmp(showlmp[i].label, lmplabel) == 0)
		{
			//showlmp[i].isactive = true;
			strcpy(showlmp[i].pic,MSG_ReadString());
			return;
		}
	}
}

void MoveLmp(void)
{
	int		i;
	byte	*lmplabel;
	float x, y;
	int origin;

	lmplabel	= MSG_ReadString();
	origin		= MSG_ReadByte();
	x			= MSG_ReadShort();
	y			= MSG_ReadShort();

	RecalcXY( &x,&y,origin);

	for (i = 0;i < SHOWLMP_MAXLABELS;i++)
	{
		if (showlmp[i].isactive && strcmp(showlmp[i].label, lmplabel) == 0)
		{
			showlmp[i].x = x;
			showlmp[i].x = y;
			return;
		}
	}
}






// Tei lmp counters
void SHOWCNT( int origin )
{
	int i, k;
	byte lmplabel[256], picname[256];
	float x, y, type, typeparm1, typeparm2, typeparm3;
	strcpy(lmplabel,MSG_ReadString());
	strcpy(picname, MSG_ReadString());
	x =			MSG_ReadShort();
	y =			MSG_ReadShort();
	type =		MSG_ReadByte();
	typeparm1 =	MSG_ReadShort();
	typeparm2 =	MSG_ReadShort();
	typeparm3 =	MSG_ReadShort();

	RecalcXY( &x,&y,origin);

	Con_Printf(" x: %d, y: %d \n", x,y);

	k = -1;
	for (i = 0;i < SHOWLMP_MAXLABELS;i++)
	{
		if (showlmp[i].isactive)
		{
			if (strcmp(showlmp[i].label, lmplabel) == 0)
			{
				k = i;
				break; // drop out to replace it
			}
		}
		else if (k < 0) // find first empty one to replace
			k = i;
	}

	if (k < 0)
		return;

	// change existing one
	showlmp[k].isactive = true;
	strcpy(showlmp[k].label, lmplabel);
	strcpy(showlmp[k].pic, picname);
	showlmp[k].x = x;
	showlmp[k].y = y;
	showlmp[k].type = type;
	showlmp[k].typeparam1 = typeparm1;
	showlmp[k].typeparam2 = typeparm2;
	showlmp[k].typeparam3 = typeparm3;

}

// Tei lmp counter

void Draw_AlphaPic (int x, int y, qpic_t *pic, float alpha);
void UpdateLocalMenus ();


qpic_t	*Draw_CachePic (char *path);

void SHOWLMP_drawall()
{
	// Tei showlmp redone
	int i,t;
//	qpic_t * pic;
	
	for (i = 0;i < SHOWLMP_MAXLABELS;i++)
		if (showlmp[i].isactive)
		switch(showlmp[i].type)
		{
			case CNT_ARRAY:
				for (t=0;t<showlmp[i].typeparam1;t++)
				Draw_Pic(showlmp[i].x + t * showlmp[i].typeparam2 , showlmp[i].y + t  * showlmp[i].typeparam3, Draw_CachePic(showlmp[i].pic));
				break;
			case CNT_STRING:
				Draw_String (showlmp[i].x,showlmp[i].y, showlmp[i].pic, 0);
				break;
			case CNT_ALPHA:
				Draw_AlphaPic(showlmp[i].x ,showlmp[i].y , Draw_CachePic(showlmp[i].pic), showlmp[i].typeparam1 * 0.01 );
				break;
			case CNT_HUDTMP:
				//Only for HUD entitys
				Draw_AlphaPic(showlmp[i].x ,showlmp[i].y , Draw_CachePic(showlmp[i].pic), showlmp[i].alpha  );
				showlmp[i].isactive = false;
				break;
			case CNT_DEFAULT:
			default:
				Draw_Pic(showlmp[i].x, showlmp[i].y, Draw_CachePic(showlmp[i].pic));
				break;
		}
	// Tei showlmp redone


	UpdateLocalMenus ();// Run localmenus stuff

}

void SHOWLMP_clear()
{
	int i;
	for (i = 0;i < SHOWLMP_MAXLABELS;i++)
		showlmp[i].isactive = false;
}
// Tomaz - ShowLMP End


#define ADJUST 2000

// Global HUD values
int hud_x, hud_y, hud_alpha;

void HUD_RecalcXY ( vec3_t location, qpic_t * pic )
{
	int midx, midy;
	float x,y;
	int origin;
	float sx, sy;
	x = (float)location[0];
	y = (float)location[1];
	origin = location[2];

	if (pic)
	{
		sx = pic->width;
		sy = pic->height;
	}
	else
	{
		sx = sy = 0;
	}


	midy = vid.height * 0.5;// >>1
	midx = vid.width * 0.5;// >>1

	switch ( origin ) 
	{
		case ORG_NW:
			break;
		case ORG_NE:
			x = vid.width - (x+sx);//Inv
			break;
		case ORG_SW:
			y = vid.height - (y+sy);//Inv
			break;
		case ORG_SE:
			y = vid.height - (y+sy);//inv
			x = vid.width - (x+sx);//Inv
			break;
		case ORG_CC:
			y = midy + (y - sy*0.5 - ADJUST);//NegCoded
			x = midx + (x - sx*0.5 - ADJUST);//NegCoded
			break;
		case ORG_CN:
			x = midx + (x - sx * 0.5- ADJUST);//NegCoded
			//Con_Printf("y : %d\n",y);
			break;
		case ORG_CS:
			x = midx + (x - sx*0.5  - ADJUST);//NegCoded
			y = vid.height - (y+sy);//Inverse
			break;
		case ORG_CW:
			y = midy + (y - sy*0.5- ADJUST);//NegCoded
			break;
		case ORG_CE:
			y = midy + (y - sy*0.5- ADJUST);//NegCoded
			x = vid.height - (x+sx); //Inverse
			break;
		default:
			break;
	}

	hud_y = y;
	hud_x = x;	
}



void DefHUD( vec3_t origin, float alpha, char * name)
{
	int i;
	showlmp_t * hud;

	//Find for a free SHOWLMP slot

	for (i = 0;i < SHOWLMP_MAXLABELS;i++)
		if (!showlmp[i].isactive) break;

	if ( i>= SHOWLMP_MAXLABELS)
		return;

	//Alternate Origin offsets support


	HUD_RecalcXY(origin, Draw_CachePic(name) );

	// Define stuff with appropiate data
	hud = &showlmp[i];
	hud->isactive = true;
	hud->x = hud_x;
	hud->y = hud_y;
	strcpy(hud->pic, name );
	hud->type  = CNT_HUDTMP;
	hud->alpha = alpha;
}

void R_DrawHudModel(entity_t * self)
{
	
	DefHUD( self->origin, self->alpha, self->model->name);
}




// - - - MenuSystem

cvar_t	menu_0 = { "menu_0",""};//option 0 Gfx/String ..
cvar_t	menu_1 = { "menu_1",""};//option 1
cvar_t	menu_2 = { "menu_2",""};//option 2
cvar_t	menu_3 = { "menu_3",""};//option 3
cvar_t	menu_4 = { "menu_4",""};//option 4
cvar_t	menu_5 = { "menu_5",""};
cvar_t	menu_6 = { "menu_6",""};
cvar_t	menu_7 = { "menu_7",""};
cvar_t	menu_8 = { "menu_8",""};
cvar_t	menu_9 = { "menu_9",""};
cvar_t	menu_select = { "menu_select","0"};//option selected (highlight)
cvar_t  menu_mode = { "menu_mode","0"};//0=nomenu, 2=gfx, 1=string
cvar_t  menu_items = { "menu_items","0"};//n# items

cvar_t  menu_scale = { "menu_scale","2"};//scale

cvar_t	xmenu_0 = { "xmenu_0",""};//Title - command
cvar_t	xmenu_1 = { "xmenu_1",""};//option 1
cvar_t	xmenu_2 = { "xmenu_2",""};//option 2
cvar_t	xmenu_3 = { "xmenu_3",""};//option 3
cvar_t	xmenu_4 = { "xmenu_4",""};//option 4
cvar_t	xmenu_5 = { "xmenu_5",""};
cvar_t	xmenu_6 = { "xmenu_6",""};
cvar_t	xmenu_7 = { "xmenu_7",""};
cvar_t	xmenu_8 = { "xmenu_8",""};
cvar_t	xmenu_9 = { "xmenu_9",""};

cvar_t	menu_image = { "menu_image","gfx/menu.tga"};

cvar_t	menu_actual = { "menu_actual","menu_0"};


void Menu_Down_f(void)
{
	static char menusel[100];

	Cvar_SetValue("menu_select", (float)menu_select.value + 1);//down a element
	
	if( menu_select.value > menu_items.value)
	{
		Cvar_SetValue("menu_select", 1);//Init of menu
	}
	
	//To link-cvars
	//sprintf(menusel,"menu_%d",menu_select.value);
	//Cvar_Set("menu_actual", menusel);//Init of menu

};

void Menu_Up_f(void)
{
	static char menusel[100];

	Cvar_SetValue("menu_select", (float)menu_select.value - 1);

	if( menu_select.value < 1)
	{
		Cvar_SetValue("menu_select", (float)menu_items.value );//Top of menu
	}

	////To link-cvars
	//sprintf(menusel,"menu_%d",menu_select.value);
	//Cvar_Set("menu_actual", menusel);//Init of menu
};


void Menu_Big_f(void)
{
	Cvar_SetValue("menu_scale", (float)menu_scale.value + 1);
}

void Menu_Small_f(void)
{
	if (menu_scale.value>1)
		Cvar_SetValue("menu_scale", (float)menu_scale.value - 1);
}


void Menu_Exec_f(void)
{
	static char menusel[100];
	int i = menu_select.value;

	if(!menu_mode.value)
		return;
	if (menu_select.value > menu_items.value)
		return;

	////To link-cvars
	//sprintf(menusel,"menu_%d",menu_select.value);
	//Cvar_Set("menu_actual", menusel);//Init of menu


#define EXEC( themenu )   Cbuf_AddText (themenu.string);Cbuf_AddText ("\n");
	switch( i)
	{
		case 1:
			EXEC( xmenu_1 );
			break;
		case 2:
			EXEC( xmenu_2 );
			break;
		case 3:
			EXEC( xmenu_3 );
			break;
		case 4:
			EXEC( xmenu_4 );
			break;
		case 5:
			EXEC( xmenu_5 );
			break;
		case 6:
			EXEC( xmenu_6 );
			break;
		case 7:
			EXEC( xmenu_7 );
			break;
		case 8:
			EXEC( xmenu_8 );
			break;
		case 9:
			EXEC( xmenu_9 );
			break;
		default:
			break;
	}
}

void Menu_Clear_f (void)
{
		Cvar_Set("menu_0","" );
		Cvar_Set("menu_1","" );
		Cvar_Set("menu_2","" );
		Cvar_Set("menu_3","" );
		Cvar_Set("menu_4","" );
		Cvar_Set("menu_5","" );
		Cvar_Set("menu_6","" );
		Cvar_Set("menu_7","" );
		Cvar_Set("menu_8","" );
		Cvar_Set("menu_9","" );
		Cvar_Set("xmenu_0","" );
		Cvar_Set("xmenu_1","" );
		Cvar_Set("xmenu_2","" );
		Cvar_Set("xmenu_3","" );
		Cvar_Set("xmenu_4","" );
		Cvar_Set("xmenu_5","" );
		Cvar_Set("xmenu_6","" );
		Cvar_Set("xmenu_7","" );
		Cvar_Set("xmenu_8","" );
		Cvar_Set("xmenu_9","" );
}



void CL_LocalMenus_Init(void)
{
	//Labels / gfx files

	Cvar_RegisterVariable (&menu_0);
	Cvar_RegisterVariable (&menu_1);
	Cvar_RegisterVariable (&menu_2);
	Cvar_RegisterVariable (&menu_3);
	Cvar_RegisterVariable (&menu_4);
	Cvar_RegisterVariable (&menu_5);
	Cvar_RegisterVariable (&menu_6);
	Cvar_RegisterVariable (&menu_7);
	Cvar_RegisterVariable (&menu_8);
	Cvar_RegisterVariable (&menu_9);

	//Config

	Cvar_RegisterVariable (&menu_select);
	Cvar_RegisterVariable (&menu_mode);
	Cvar_RegisterVariable (&menu_items);

	Cvar_RegisterVariable (&menu_scale);

	Cvar_RegisterVariable (&menu_image);


	//Commands
	Cvar_RegisterVariable (&xmenu_0);
	Cvar_RegisterVariable (&xmenu_1);
	Cvar_RegisterVariable (&xmenu_2);
	Cvar_RegisterVariable (&xmenu_3);
	Cvar_RegisterVariable (&xmenu_4);
	Cvar_RegisterVariable (&xmenu_5);
	Cvar_RegisterVariable (&xmenu_6);
	Cvar_RegisterVariable (&xmenu_7);
	Cvar_RegisterVariable (&xmenu_8);
	Cvar_RegisterVariable (&xmenu_9);

	Cmd_AddCommand ("menu_up", Menu_Up_f);
	Cmd_AddCommand ("menu_down", Menu_Down_f);

	Cmd_AddCommand ("menu_big", Menu_Big_f);
	Cmd_AddCommand ("menu_small", Menu_Small_f);

	Cmd_AddCommand ("menu_exec", Menu_Exec_f);

	Cmd_AddCommand ("menu_clear", Menu_Clear_f);


}

#define MENUBASE 20
void Draw_StringMult (int x, int y, char *str, int maxlen, int mult);

void DrawQ_String (float x, float y, char *string, int maxlen, float scalex, float scaley, float red, float green, float blue, float alpha, int flags)
{

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);	

	glColor3f(red,green,blue);
	Draw_StringMult(x,y,string,maxlen,scalex);
	glColor3f(1,1,1);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
};

typedef struct
{
	int		texnum;
	float	sl, tl, sh, th;
} glpic_t;

void DrawQ_Pic (float x, float y, char *picname, float width, float height, float red, float green, float blue, float alpha, int flags)
{
	
	glpic_t			*gl;
	qpic_t			*pic = Draw_CachePic(picname);

	gl = (glpic_t *)pic->data;

	glColor4f (red,green,blue,alpha);
	glBindTexture (GL_TEXTURE_2D, gl->texnum);

	glBegin (GL_QUADS);

	glTexCoord2f	(gl->sl,		gl->tl);
	glVertex2f		(x,				y);
	glTexCoord2f	(gl->sh,		gl->tl);
	glVertex2f		(x+width,	y);
	glTexCoord2f	(gl->sh,		gl->th);
	glVertex2f		(x+width,	y+height);
	glTexCoord2f	(gl->sl,		gl->th);
	glVertex2f		(x,				y+height);

	glEnd ();

	glColor4f (1,1,1,1);//Draw_AlphaPic()
}

int COM_FileExists(char *filename);

void UpdateLocalMenus ()
{
	
	int	i;
	char * menusel;
	char nullmenu[] = "";
	int y= MENUBASE;
	int x= 65*0.5*menu_scale.value;

	if (!menu_mode.value )
		return;
//DrawQ_String (x, y, string,maxlen,scalex,scaley, float red, float green, float blue, float alpha, int flags) 

	//320x205
	if (!strcmp(menu_image.string,"") && COM_FileExists(menu_image.string))
	{
		DrawQ_Pic(0,0, menu_image.string, 320*0.5*menu_scale.value, 205*0.5*menu_scale.value, 1, 1, 1, 0.9f, 0);
	}

#define SayScreen(stringie) DrawQ_String(x,y,stringie.string,100,4*menu_scale.value,4*menu_scale.value,0.4f,1,0.4f,1,0);
#define SayNext  y += 6 * menu_scale.value; 

	//SayScreen(menu_0);
	
	//Title Highlithed
	DrawQ_String(x,y,menu_0.string,100,4*menu_scale.value,4*menu_scale.value,0.5f,1,0.5f,1,0);
	//Draw_String(x,y,menu_0.string,100);
	SayNext;
	SayNext;
	SayScreen(menu_1);
	SayNext;
	SayScreen(menu_2);
	SayNext;
	SayScreen(menu_3);
	SayNext;
	SayScreen(menu_4);
	SayNext;
	SayScreen(menu_5);
	SayNext;
	SayScreen(menu_6);
	SayNext;
	SayScreen(menu_7);
	SayNext;
	SayScreen(menu_8);
	SayNext;
	SayScreen(menu_9);


	if(menu_select.value)
	{
		y= MENUBASE + 6*menu_scale.value * (menu_select.value+1);

		switch (i=menu_select.value)//a ugly way to pass a integer 
		{
			case 0:
				menusel = nullmenu;
				break;
			case 1:
				menusel = menu_1.string;
				break;
			case 2:
				menusel = menu_2.string;
				break;
			case 3:
				menusel = menu_3.string;
				break;
			case 4:
				menusel = menu_4.string;
				break;
			case 5:
				menusel = menu_5.string;
				break;
			case 6:
				menusel = menu_6.string;
				break;
			case 7:
				menusel = menu_7.string;
				break;
			case 8:
				menusel = menu_8.string;
				break;
			case 9:
				menusel = menu_9.string;
				break;
			default:
				menusel = nullmenu;
			break;
		}


		DrawQ_String(x,y,menusel,100,4*menu_scale.value,4*menu_scale.value,1,1,1,1,0);
		DrawQ_String(x-4*menu_scale.value,y,"\x8d",100,4*menu_scale.value,4*menu_scale.value,1,1,1,1,0);
	}

}


