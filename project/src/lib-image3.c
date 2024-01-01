
/***************************************************************************
 *                         _______ _______ _______                         *
 *                        |  ___  |____   |  ___  |                        *
 *                        | |   |_|    / /| |   |_|                        *
 *                        | |_____    / / | |_____                         *
 *                        |_____  |  / /  |_____  |                        *
 *                         _    | | / /    _    | |                        *
 *                        | |___| |/ /____| |___| |                        *
 *                        |_______|_______|_______|                        *
 *                                                                         *
 *                            Wiimms SZS Tools                             *
 *                          https://szs.wiimm.de/                          *
 *                                                                         *
 ***************************************************************************
 *                                                                         *
 *   This file is part of the SZS project.                                 *
 *   Visit https://szs.wiimm.de/ for project details and sources.          *
 *                                                                         *
 *   Copyright (c) 2011-2024 by Dirk Clemens <wiimm@wiimm.de>              *
 *                                                                         *
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   See file gpl-2.0.txt or http://www.gnu.org/licenses/gpl-2.0.txt       *
 *                                                                         *
 ***************************************************************************/

#include "lib-std.h"
#include "lib-image.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			data structures			///////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct PatchRange1_t
{
    int			dest;	// destination coordinate
    int			src;	// source coordinate
    int			len;	// length of copy operation
    int			size;	// total number of pixels

} PatchRange1_t;

///////////////////////////////////////////////////////////////////////////////

typedef struct PatchRange2_t
{
    PatchRange1_t	x;	// horizontal values
    PatchRange1_t	y;	// vertical values

} PatchRange2_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////		    image creation + drawing		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError CreateIMG
(
    Image_t		* img,		// image object
    bool		init_img,	// true: initialize 'img' first
    uint		width,		// width a new image
    uint		height,		// height of new image
    Color_t		col		// fill color
)
{
    DASSERT(img);

    if (init_img)
	InitializeIMG(img);

    if (!width)
	width = 1;
    const uint xwidth = EXPAND8(width);

    if (!height)
	height = 1;
    const uint xheight = EXPAND8(height);

    uint size = xwidth * xheight;
    Color_t *data = MALLOC(size*sizeof(col));

    img->width = width;
    img->height = height;
    AssignDataRGB(img,0,(u8*)data);
    while ( size-- > 0 )
	*data++ = col;

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError CopyRectIMG
(
    Image_t		* dest,		// dest image
    int			dest_x,		// left coordinate of dest
    int			dest_y,		// top coordinate of dest

    const Image_t	* src,		// source image
    int			src_x,		// left coordinate of src
    int			src_y,		// top coordinate of src

    int			width,		// width of rect to copy
    int			height,		// height of rect to copy

    PatchImage_t	patch_mode	// one of COPY, MIX, FOREGROUND, BACKGROUND
)
{
    DASSERT(dest);
    DASSERT(src);

    if ( dest->iform != IMG_X_RGB )
	ConvertIMG(dest,false,0,IMG_X_RGB,PAL_AUTO);
    DASSERT(dest->iform == IMG_X_RGB);

    if ( src->iform != IMG_X_RGB )
	return ERROR0(ERR_INTERNAL,0);


    //--- normalize horizontal values

    if ( dest_x < 0 )
    {
	width  += dest_x;
	src_x  -= dest_x;
	dest_x  = 0;
    }

    if ( src_x < 0 )
    {
	width  += src_x;
	dest_x -= src_x;
	src_x   = 0;
    }

    int max = dest->width - dest_x;
    if ( width > max )
	 width = max;

    max = src->width - src_x;
    if ( width > max )
	 width = max;


    //--- normalize vertical values

    if ( dest_y < 0 )
    {
	height += dest_y;
	src_y  -= dest_y;
	dest_y  = 0;
    }

    if ( src_y < 0 )
    {
	height += src_y;
	dest_y -= src_y;
	src_y   = 0;
    }

    max = dest->height - dest_y;
    if ( height > max )
	 height = max;

    max = src->height - src_y;
    if ( height > max )
	 height = max;


    //--- copy rect

    if ( width > 0 && height > 0 )
    {
	DASSERT( dest_x >= 0 && dest_x + width  <= dest->width );
	DASSERT( dest_y >= 0 && dest_y + height <= dest->height );
	DASSERT( src_x >= 0 && src_x + width  <= src->width );
	DASSERT( src_y >= 0 && src_y + height <= src->height );

	Color_t *dcol = (Color_t*)dest->data + dest_y * dest->xwidth + dest_x;
	const Color_t *scol = (Color_t*)src->data + src_y * src->xwidth + src_x;

	const uint dest_skip = dest->xwidth - width;
	const uint src_skip = src->xwidth - width;

	switch ( patch_mode & PIM_M_COPY )
	{
	  case PIM_BACKGROUND:
	    while ( height-- > 0 )
	    {
	      uint n = width;
	      while ( n-- > 0 )
	      {
		const uint dalpha = dcol->a;
		const uint salpha = 0xff - dalpha;
		dcol->r = ( dalpha * dcol->r + salpha * scol->r + 0x80 ) / 0xff;
		dcol->g = ( dalpha * dcol->g + salpha * scol->g + 0x80 ) / 0xff;
		dcol->b = ( dalpha * dcol->b + salpha * scol->b + 0x80 ) / 0xff;
		dcol->a = ( 0xfe01 - ( 0xff - dcol->a ) * ( 0xff - scol->a )) / 0xff;
		dcol++;
		scol++;
	      }
	      DASSERT( (u8*)dcol <= dest->data + dest->data_size );
	      DASSERT( (u8*)scol <= src->data + src->data_size );
	      dcol += dest_skip;
	      scol += src_skip;
	    }
	    break;

	  case PIM_FOREGROUND:
	    while ( height-- > 0 )
	    {
	      uint n = width;
	      while ( n-- > 0 )
	      {
		const uint salpha = scol->a;
		const uint dalpha = 0xff - salpha;
		dcol->r = ( dalpha * dcol->r + salpha * scol->r + 0x80 ) / 0xff;
		dcol->g = ( dalpha * dcol->g + salpha * scol->g + 0x80 ) / 0xff;
		dcol->b = ( dalpha * dcol->b + salpha * scol->b + 0x80 ) / 0xff;
		dcol->a = ( 0xfe01 - ( 0xff - dcol->a ) * ( 0xff - scol->a )) / 0xff;
		dcol++;
		scol++;
	      }
	      DASSERT( (u8*)dcol <= dest->data + dest->data_size );
	      DASSERT( (u8*)scol <= src->data + src->data_size );
	      dcol += dest_skip;
	      scol += src_skip;
	    }
	    break;

	  case PIM_MIX:
	    while ( height-- > 0 )
	    {
	      uint n = width;
	      while ( n-- > 0 )
	      {
		const uint alpha = dcol->a + scol->a;
		if (alpha)
		{
		    const uint alpha2 = alpha/2;
		    dcol->r = ( dcol->a * dcol->r + scol->a * scol->r + alpha2 ) / alpha;
		    dcol->g = ( dcol->a * dcol->g + scol->a * scol->g + alpha2 ) / alpha;
		    dcol->b = ( dcol->a * dcol->b + scol->a * scol->b + alpha2 ) / alpha;
		    dcol->a = ( 0xfe01 - ( 0xff - dcol->a ) * ( 0xff - scol->a )) / 0xff;
		}
		dcol++;
		scol++;
	      }
	      DASSERT( (u8*)dcol <= dest->data + dest->data_size );
	      DASSERT( (u8*)scol <= src->data + src->data_size );
	      dcol += dest_skip;
	      scol += src_skip;
	    }
	    break;

	    default: // PIM_COPY:
	    while ( height-- > 0 )
	    {
		uint n = width;
		while ( n-- > 0 )
		    *dcol++ = *scol++;
		DASSERT( (u8*)dcol <= dest->data + dest->data_size );
		DASSERT( (u8*)scol <= src->data + src->data_size );
		dcol += dest_skip;
		scol += src_skip;
	    }
	    break;
	}
    }

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError DrawPointIMG
(
    Image_t		* img,		// image
    int			x,		// x position (robust)
    int			y,		// y position (robust)
    Color_t		col,		// draw color
    bool		combine		// true: combine 'col' with current one
)
{
    DASSERT(img);
    if ( img->iform != IMG_X_RGB )
	ConvertIMG(img,false,0,IMG_X_RGB,PAL_AUTO);

    if ( x >= 0 && x < img->width && y >= 0 && y < img->height && col.a )
    {
	Color_t *dest = (Color_t*)img->data + y * img->xwidth + x;

	if ( combine && col.a != 0xff )
	{
	    const uint da = ( 0xff - col.a ) * dest->a;
	    dest->r = ( da * dest->r + 0xff * col.a * col.r ) / 0xfe01;
	    dest->g = ( da * dest->g + 0xff * col.a * col.g ) / 0xfe01;
	    dest->b = ( da * dest->b + 0xff * col.a * col.b ) / 0xfe01;
	    dest->a = 0xff - (0xff-dest->a)*(0xff-col.a)/0xff;
	}
	else
	    *dest = col;
    }

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError DrawHLineIMG
(
    Image_t		* img,		// image
    int			x1,		// start point (robust)
    int			x2,		// end point (not drawn, robust)
    int			y,		// common y coordinate (robust)
    Color_t		col		// draw color
)
{
    DASSERT(img);
    if ( img->iform != IMG_X_RGB )
	ConvertIMG(img,false,0,IMG_X_RGB,PAL_AUTO);

    if ( x1 > x2 )
    {
	const uint temp = x1;
	x1 = x2 - 1;
	x2 = temp + 1;
    }

    if ( x1 < 0 )
	 x1 = 0;
    if ( x2 > img->width )
	 x2 = img->width;

    if ( x1 < x2 && y >= 0 && y < img->height )
    {
	Color_t *dest = (Color_t*)img->data + y * img->xwidth + x1;
	uint n = x2 - x1;
	while ( n-- > 0 )
	    *dest++ = col;
	DASSERT( (u8*)dest <= img->data + img->data_size );
    }

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError DrawVLineIMG
(
    Image_t		* img,		// image
    int			x,		// common x coordinate (robust)
    int			y1,		// start point (robust)
    int			y2,		// end point (not drawn, robust)
    Color_t		col		// draw color
)
{
    DASSERT(img);
    if ( img->iform != IMG_X_RGB )
	ConvertIMG(img,false,0,IMG_X_RGB,PAL_AUTO);

    if ( y1 > y2 )
    {
	const uint temp = y1;
	y1 = y2 - 1;
	y2 = temp + 1;
    }

    if ( y1 < 0 )
	 y1 = 0;
    if ( y2 > img->height )
	 y2 = img->height;

    if ( y1 < y2 && x >= 0 && x < img->width )
    {
	Color_t *dest = (Color_t*)img->data + y1 * img->xwidth + x;
	uint n = y2 - y1;
	while ( n-- > 0 )
	{
	    *dest = col;
	    dest += img->xwidth;
	}
	DASSERT( (u8*)dest <= img->data + img->data_size );
    }

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError DrawFrameIMG
(
    Image_t		* img,		// image
    int			x1,		// left coordinate (robust)
    int			y1,		// top coordinate (robust)
    int			x2,		// right coordinate (robust)
    int			y2,		// bottom coordinate (robust)
    Color_t		col		// draw color
)
{
    DASSERT(img);
    DrawHLineIMG(img,x1,x2,y1,col);
    DrawVLineIMG(img,x2,y1,y2,col);
    DrawHLineIMG(img,x2,x1,y2,col);
    DrawVLineIMG(img,x1,y2,y1,col);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError FillRectIMG
(
    Image_t		* img,		// image
    int			x1,		// left coordinate (robust)
    int			y1,		// top coordinate (robust)
    int			x2,		// right coordinate (robust)
    int			y2,		// bottom coordinate (robust)
    Color_t		col,		// draw color
    bool		combine		// true: combine 'col' with current one
)
{
    DASSERT(img);
    if ( img->iform != IMG_X_RGB )
	ConvertIMG(img,false,0,IMG_X_RGB,PAL_AUTO);

    if ( x1 > x2 )
    {
	const uint temp = x1;
	x1 = x2 - 1;
	x2 = temp + 1;
    }

    if ( x1 < 0 )
	 x1 = 0;
    if ( x2 > img->width )
	 x2 = img->width;

    if ( y1 > y2 )
    {
	const uint temp = y1;
	y1 = y2 - 1;
	y2 = temp + 1;
    }

    if ( y1 < 0 )
	 y1 = 0;
    if ( y2 > img->height )
	 y2 = img->height;

    if ( x1 < x2 && y1 < y2 )
    {
	Color_t *dest = (Color_t*)img->data + y1 * img->xwidth + x1;
	const uint skip = img->xwidth - (x2-x1);
	uint yn = y2 - y1;
	if ( combine && col.a != 0xff )
	{
	    const uint r = 0xff * col.a * col.r;
	    const uint g = 0xff * col.a * col.g;
	    const uint b = 0xff * col.a * col.b;

	    while ( yn-- > 0 )
	    {
		uint xn = x2 - x1;
		while ( xn-- > 0 )
		{
		    const uint da = ( 0xff - col.a ) * dest->a;
		    dest->r = ( da * dest->r + r ) / 0xfe01;
		    dest->g = ( da * dest->g + g ) / 0xfe01;
		    dest->b = ( da * dest->b + b ) / 0xfe01;
		    dest->a = 0xff - (0xff-dest->a)*(0xff-col.a)/0xff;
		    dest++;
		}
		DASSERT( (u8*)dest <= img->data + img->data_size );
		dest += skip;
	    }
	}
	else
	{
	    while ( yn-- > 0 )
	    {
		uint xn = x2 - x1;
		while ( xn-- > 0 )
		    *dest++ = col;
		DASSERT( (u8*)dest <= img->data + img->data_size );
		dest += skip;
	    }
	}
    }
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError FillHGradientIMG
(
    Image_t		* img,		// image
    int			x1,		// left coordinate (robust)
    int			y1,		// top coordinate (robust)
    int			x2,		// right coordinate (robust)
    int			y2,		// bottom coordinate (robust)
    Color_t		col1,		// draw color left
    Color_t		col2		// draw color right
)
{
    DASSERT(img);
    if ( img->iform != IMG_X_RGB )
	ConvertIMG(img,false,0,IMG_X_RGB,PAL_AUTO);

    if ( x1 > x2 )
    {
	const uint temp = x1;
	x1 = x2 - 1;
	x2 = temp + 1;

	const Color_t col = col1;
	col1 = col2;
	col2 = col;
    }

    if ( x1 < 0 )
	 x1 = 0;
    if ( x2 > img->width )
	 x2 = img->width;

    if ( y1 > y2 )
    {
	const uint temp = y1;
	y1 = y2 - 1;
	y2 = temp + 1;
    }

    if ( y1 < 0 )
	 y1 = 0;
    if ( y2 > img->height )
	 y2 = img->height;

    if ( x1 < x2 && y1 < y2 )
    {
	Color_t *dest1 = (Color_t*)img->data + y1 * img->xwidth + x1;

	const uint xn = x2 - x1, xn2 = xn/2;
	uint xi;
	for ( xi = 0; xi < xn; xi++, dest1++ )
	{
	    Color_t col;
	    col.r = ( xi * col2.r + (xn-xi) * col1.r + xn2 ) / xn;
	    col.g = ( xi * col2.g + (xn-xi) * col1.g + xn2 ) / xn;
	    col.b = ( xi * col2.b + (xn-xi) * col1.b + xn2 ) / xn;
	    col.a = ( xi * col2.a + (xn-xi) * col1.a + xn2 ) / xn;

	    Color_t *dest2 = dest1;
	    uint yn = y2 - y1;
	    while ( yn-- > 0 )
	    {
		*dest2 = col;
		dest2 += img->xwidth;
	    }
	}
	DASSERT( (u8*)dest1 <= img->data + img->data_size );
    }

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError FillVGradientIMG
(
    Image_t		* img,		// image
    int			x1,		// left coordinate (robust)
    int			y1,		// top coordinate (robust)
    int			x2,		// right coordinate (robust)
    int			y2,		// bottom coordinate (robust)
    Color_t		col1,		// draw color top
    Color_t		col2		// draw color bottom
)
{
    DASSERT(img);
    if ( img->iform != IMG_X_RGB )
	ConvertIMG(img,false,0,IMG_X_RGB,PAL_AUTO);

    if ( x1 > x2 )
    {
	const uint temp = x1;
	x1 = x2 - 1;
	x2 = temp + 1;
    }

    if ( x1 < 0 )
	 x1 = 0;
    if ( x2 > img->width )
	 x2 = img->width;

    if ( y1 > y2 )
    {
	const uint temp = y1;
	y1 = y2 - 1;
	y2 = temp + 1;

	const Color_t col = col1;
	col1 = col2;
	col2 = col;
    }

    if ( y1 < 0 )
	 y1 = 0;
    if ( y2 > img->height )
	 y2 = img->height;

    if ( x1 < x2 && y1 < y2 )
    {
	Color_t *dest = (Color_t*)img->data + y1 * img->xwidth + x1;
	const uint skip = img->xwidth - (x2-x1);

	const uint yn = y2 - y1, yn2 = yn/2;
	uint yi;
	for ( yi = 0; yi < yn; yi++ )
	{
	    Color_t col;
	    col.r = ( yi * col2.r + (yn-yi) * col1.r + yn2 ) / yn;
	    col.g = ( yi * col2.g + (yn-yi) * col1.g + yn2 ) / yn;
	    col.b = ( yi * col2.b + (yn-yi) * col1.b + yn2 ) / yn;
	    col.a = ( yi * col2.a + (yn-yi) * col1.a + yn2 ) / yn;

	    uint xn = x2 - x1;
	    while ( xn-- > 0 )
		*dest++ = col;
	    dest += skip;
	}
	DASSERT( (u8*)dest <= img->data + img->data_size );
    }

    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			image resizing			///////////////
///////////////////////////////////////////////////////////////////////////////

bool fast_resize_enabled = false;

///////////////////////////////////////////////////////////////////////////////

enumError ResizeIMG
(
    Image_t		* dest,		// dest image (may be same as src)
    bool		init_dest,	// true: initialize 'dest' first
    const Image_t	* src,		// source image (if NULL: use dest)
    uint		width,		// new width of image
    uint		height		// new height of image
					// if one of 'width' or 'height' is NULL
					//	-> keep aspect ratio
)
{
    return fast_resize_enabled
	? FastResizeIMG(dest,init_dest,src,width,height)
	: SmartResizeIMG(dest,init_dest,src,width,height);
}

///////////////////////////////////////////////////////////////////////////////

enumError FastResizeIMG
(
    Image_t		* dest,		// dest image (may be same as src)
    bool		init_dest,	// true: initialize 'dest' first
    const Image_t	* src,		// source image (if NULL: use dest)
    uint		width,		// new width of image
    uint		height		// new height of image
					// if one of 'width' or 'height' is NULL
					//	-> keep aspect ratio
)
{
    DASSERT(dest);
    if (init_dest)
	InitializeIMG(dest);
    if (!src)
	src = dest;


    //--- analyze width and height

    if ( !src->width || !src->height )
    {
	CopyIMG(dest,false,src,true);
	return ERR_OK;
    }

    if (!width)
    {
	if (!height)
	{
	    CopyIMG(dest,false,src,true);
	    NormalizeFrameIMG(dest);
	    return ERR_OK;
	}
	width = ( src->width * height + src->height/2 ) / src->height;
    }
    else if (!height)
	height = ( src->height * width + src->width/2 ) / src->width;

    if ( width == src->width && height == src->height )
    {
	//--- no resize needed

	CopyIMG(dest,false,src,true);
	NormalizeFrameIMG(dest);
	return ERR_OK;
    }

    if ( 2*width == src->width && 2*height == src->height )
    {
	//--- use fast HalfIMG()

	if ( src != dest )
	    CopyIMG(dest,false,src,true);
	return HalfIMG(dest);
    }


    //--- convert to IMG_X_RGB

    PRINT("RESIZE: %u*%u -> %u*%u\n",
		src->width, src->height, width, height );

    if ( src->iform != IMG_X_RGB )
    {
	noPRINT("FastResizeIMG: src= %s [%x], dest= %s [%x]\n",
		PrintFormatIMG(src), src->iform,
		PrintFormatIMG(dest), dest->iform );

	if (IsGrayIMG(src))
	{
	    //--- convert temporary to IMG_X_RGB

	    enumError err = ConvertIMG(dest,false,src,IMG_X_RGB,PAL_AUTO);
	    noPRINT("FastResizeIMG: gray= %s [%x]\n",PrintFormatIMG(dest),dest->iform);
	    if (!err)
	    {
		err = FastResizeIMG(dest,false,dest,width,height);
		if (!err)
		    err = ConvertToGRAY(dest,dest,PAL_AUTO);
	    }
	    return err;
	}

	enumError err = ConvertIMG(dest,false,src,IMG_X_RGB,PAL_AUTO);
	if (err)
	    return err;
	src = dest;
	noPRINT("FastResizeIMG: conv= %s [%x]\n",
		PrintFormatIMG(dest), dest->iform );

	if ( src->iform != IMG_X_RGB )
	    return ERROR0(ERR_INTERNAL,
			"Image format 'X-RGB' expected, but '%s' found\n",
			PrintFormatIMG(src));
    }


    //--- setup new image data

    DASSERT( src->iform == IMG_X_RGB );

    const uint bytes_per_pixel	= 4;
    const uint xwidth		= EXPAND8(width);
    const uint xheight		= EXPAND8(height);
    const uint data_size	= xwidth * xheight * bytes_per_pixel;
    u8 *data			= MALLOC(data_size);
    memset(data,0xff,data_size);
    PRINT(" - new image: %u*%u -> %u*%u -> %u bytes\n",
		width, height, xwidth, xheight, data_size );

    u8 * dest1 = data;
    const u8 * src1 = src->data;
    uint src_line_size  = bytes_per_pixel * src->xwidth;
    uint dest_line_size = bytes_per_pixel * xwidth;


    //--- main loop
    //    naming scheme: abx: a=Height|Width, b=Src|Dest, x=1|2|N|MAX

    const uint wmax = width  < src->width  ? width  : src->width;
    const uint hmax = height < src->height ? height : src->height;
    uint ht, hs2 = 0, hd2 = 0;
    for ( ht = 1; ht <= hmax; ht++ )
    {
	const uint hs1 = hs2;
	hs2 = ( ht * src->height + hmax/2 ) / hmax;
	const uint hsn = hs2 - hs1;

	const uint hd1 = hd2;
	hd2 = ( ht * height + hmax/2 ) / hmax;
	const uint hdn = hd2 - hd1;

	noPRINT("  LINE: %5u .. %5u -> %5u .. %5u [ %3u -> %3u ]\n",
	hs1, hs2, hd1, hd2, hsn, hdn );
	DASSERT( hsn >= 1 && hdn >= 1 );
	DASSERT( hsn == 1 || hdn == 1 );

	if ( hsn == 1 && width == src->width )
	{
	    //--- copy complete line without any calculations
	    memcpy(dest1,src1,width*bytes_per_pixel);
	}
	else
	{
	    //--- mix each cell

	    u8 * dest2 = dest1;
	    const u8 * src2 = src1;

	    uint wd, ws2 = 0, wd2 = 0;
	    for ( wd = 1; wd <= wmax; wd++ )
	    {
		const uint ws1 = ws2;
		ws2 = ( wd * src->width + wmax/2 ) / wmax;
		const uint wsn = ws2 - ws1;

		const uint wd1 = wd2;
		wd2 = ( wd * width + wmax/2 ) / wmax;
		const uint wdn = wd2 - wd1;

		noPRINT("COPY: %4u..%4u*%4u..%4u -> %4u..%4u*%4u..%4u [ %2u*%2u -> %2u*%2u ]\n",
				ws1, ws2, hs1, hs2,
				wd1, wd2, hd1, hd2,
				wsn, wdn, hsn, hdn );
		DASSERT( wsn >= 1 && wdn >= 1 );
		DASSERT( wsn == 1 || wdn == 1 );

		DASSERT(bytes_per_pixel==4);
		uint sum[4];
		memset(sum,0,sizeof(sum));

		if ( hsn == 1 )
		{
		    uint nw = wsn;
		    while ( nw-- > 0 )
		    {
			sum[0] += *src2++;
			sum[1] += *src2++;
			sum[2] += *src2++;
			sum[3] += *src2++;
		    }
		}
		else if ( hsn == 2 )
		{
		    uint nw = wsn;
		    while ( nw-- > 0 )
		    {
			sum[0] += (uint)*src2 + src2[src_line_size]; src2++;
			sum[1] += (uint)*src2 + src2[src_line_size]; src2++;
			sum[2] += (uint)*src2 + src2[src_line_size]; src2++;
			sum[3] += (uint)*src2 + src2[src_line_size]; src2++;
		    }
		}
		else
		{
		    uint nh = hsn;
		    while ( nh-- > 0 )
		    {
			const u8 * src3 = src2 + nh * src_line_size;
			uint nw = wsn;
			while ( nw-- > 0 )
			{
			    sum[0] += *src3++;
			    sum[1] += *src3++;
			    sum[2] += *src3++;
			    sum[3] += *src3++;
			}
		    }
		    src2 += wsn * bytes_per_pixel;
		}

		uint n = wdn, div = hsn * wsn;
		while ( n-- > 0 )
		{
		    *dest2++ = sum[0] / div;
		    *dest2++ = sum[1] / div;
		    *dest2++ = sum[2] / div;
		    *dest2++ = sum[3] / div;
		}
	    }
	}

	//--- duplicate lines

	uint i;
	for ( i = 1; i < hdn; i++ )
	{
	    memcpy(dest1+dest_line_size,dest1,dest_line_size);
	    dest1 += dest_line_size;
	}

	//--- adjust first level pointers

	dest1 += dest_line_size;
	src1  += src_line_size * hsn;
    }


    //--- assign data

    FreeIMG(dest,src);

    dest->data		= data;
    dest->data_alloced	= true;
    dest->width		= width;
    dest->height	= height;
    dest->xwidth	= xwidth;
    dest->xheight	= xheight;
    dest->data_size	= dest->xwidth * dest->xheight * 4;
    dest->iform		= IMG_X_RGB;
    NormalizeFrameIMG(dest);

    PRINT(" - new image: %u*%u -> %u*%u -> %u bytes\n",
		dest->width, dest->height, dest->xwidth, dest->xheight, dest->data_size );

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError SmartResizeIMG
(
    Image_t		* dest,		// dest image (may be same as src)
    bool		init_dest,	// true: initialize 'dest' first
    const Image_t	* src,		// source image (if NULL: use dest)
    uint		width,		// new width of image
    uint		height		// new height of image
					// if one of 'width' or 'height' is NULL
					//	-> keep aspect ratio
)
{
    DASSERT(dest);
    if (init_dest)
	InitializeIMG(dest);
    if (!src)
	src = dest;


    //--- analyze width and height

    if ( !src->width || !src->height )
    {
	CopyIMG(dest,false,src,true);
	return ERR_OK;
    }

    if (!width)
    {
	if (!height)
	{
	    CopyIMG(dest,false,src,true);
	    NormalizeFrameIMG(dest);
	    return ERR_OK;
	}
	width = ( src->width * height + src->height/2 ) / src->height;
    }
    else if (!height)
	height = ( src->height * width + src->width/2 ) / src->width;

    if ( width == src->width && height == src->height )
    {
	//--- no resize needed

	CopyIMG(dest,false,src,true);
	NormalizeFrameIMG(dest);
	return ERR_OK;
    }

    if ( 2*width == src->width && 2*height == src->height )
    {
	//--- use fast HalfIMG()

	if ( src != dest )
	    CopyIMG(dest,false,src,true);
	return HalfIMG(dest);
    }


    //--- convert to IMG_X_RGB

    PRINT("RESIZE: %u*%u -> %u*%u\n",
		src->width, src->height, width, height );

    if ( src->iform != IMG_X_RGB )
    {
	noPRINT("SmartResizeIMG: src= %s [%x], dest= %s [%x]\n",
		PrintFormatIMG(src), src->iform,
		PrintFormatIMG(dest), dest->iform );

	if (IsGrayIMG(src))
	{
	    //--- convert temporary to IMG_X_RGB

	    enumError err = ConvertIMG(dest,false,src,IMG_X_RGB,PAL_AUTO);
	    noPRINT("SmartResizeIMG: gray= %s [%x]\n",PrintFormatIMG(dest),dest->iform);
	    if (!err)
	    {
		err = SmartResizeIMG(dest,false,dest,width,height);
		if (!err)
		    err = ConvertToGRAY(dest,dest,PAL_AUTO);
	    }
	    return err;
	}

	enumError err = ConvertIMG(dest,false,src,IMG_X_RGB,PAL_AUTO);
	if (err)
	    return err;
	src = dest;
	noPRINT("SmartResizeIMG: conv= %s [%x]\n",
		PrintFormatIMG(dest), dest->iform );

	if ( src->iform != IMG_X_RGB )
	    return ERROR0(ERR_INTERNAL,
			"Image format 'X-RGB' expected, but '%s' found\n",
			PrintFormatIMG(src));
    }


    //--- setup new image data

    DASSERT( src->iform == IMG_X_RGB );

    const uint bytes_per_pixel	= 4;
    const uint xwidth		= EXPAND8(width);
    const uint xheight		= EXPAND8(height);
    const uint data_size	= xwidth * xheight * bytes_per_pixel;
    u8 *data			= MALLOC(data_size);
    memset(data,0xff,data_size);
    PRINT(" - new image: %u*%u -> %u*%u -> %u bytes\n",
		width, height, xwidth, xheight, data_size );

    u8 * dest1 = data;
    const uint dest_line_skip = bytes_per_pixel * ( xwidth - width );

    //const u8 * src1 = src->data;
    const uint src_line_size  = bytes_per_pixel * src->xwidth;
    u8 *temp_src = MALLOC(src_line_size);
    //const uint dest_line_size = bytes_per_pixel * xwidth;

    ResizeHelper_t rh, rv;
    InitializeResize(&rh,src->width,width);
    InitializeResize(&rv,src->height,height);

    bool v_ok;
    for ( v_ok = FirstResize(&rv); v_ok; v_ok = NextResize(&rv) )
    {
	//--- proccess vertical points

	const u8 *src2 = src->data + src_line_size * rv.src_idx;
	u8 *temp = temp_src;
	uint n = src->width;
	while ( n-- > 0 )
	{
	    uint sum0 = rv.half_factor;
	    uint sum1 = rv.half_factor;
	    uint sum2 = rv.half_factor;
	    uint sum3 = rv.half_factor;

	    const u8 *src3 = src2;
	    const ResizeElement_t *re;
	    for ( re = rv.elem; re->n_elem; re++ )
	    {
		uint ne;
		for ( ne = re->n_elem; ne > 0; ne-- )
		{
		    noPRINT("COPY %u, %u: %zu / %zu / %u\n",
				rv.src_idx, ne,
				( src2 - src->data ) / src_line_size,
				( src3 - src->data ) / src_line_size,
				src->height );
		    DASSERT( src3 < src->data + src->data_size );
		    sum0 += src3[0] * re->factor;
		    sum1 += src3[1] * re->factor;
		    sum2 += src3[2] * re->factor;
		    sum3 += src3[3] * re->factor;
		    src3 += src_line_size;
		}
	    }
	    *temp++ = sum0 / rv.sum_factor;
	    *temp++ = sum1 / rv.sum_factor;
	    *temp++ = sum2 / rv.sum_factor;
	    *temp++ = sum3 / rv.sum_factor;
	    DASSERT( temp <= temp_src + src_line_size );
	    src2 += bytes_per_pixel;
	    DASSERT( src2 <= src->data + src->data_size );
	}
	DASSERT( temp == temp_src + bytes_per_pixel * src->width );

	#if HAVE_PRINT0
	{
	    static bool done = false;
	    if (!done)
	    {
		done = true;
		HEXDUMP16(4,0,temp_src,temp-temp_src);
	    }
	}
	#endif

	bool h_ok;
	for ( h_ok = FirstResize(&rh); h_ok; h_ok = NextResize(&rh) )
	{
	    uint sum0 = rh.half_factor;
	    uint sum1 = rh.half_factor;
	    uint sum2 = rh.half_factor;
	    uint sum3 = rh.half_factor;

	    u8 *temp = temp_src + rh.src_idx * bytes_per_pixel;

	    const ResizeElement_t *re;
	    for ( re = rh.elem; re->n_elem; re++ )
	    {
		uint ne;
		for ( ne = re->n_elem; ne > 0; ne-- )
		{
		    sum0 += *temp++ * re->factor;
		    sum1 += *temp++ * re->factor;
		    sum2 += *temp++ * re->factor;
		    sum3 += *temp++ * re->factor;
		}
	    }

	    *dest1++ = sum0 / rh.sum_factor;
	    *dest1++ = sum1 / rh.sum_factor;
	    *dest1++ = sum2 / rh.sum_factor;
	    *dest1++ = sum3 / rh.sum_factor;
	}
	dest1 += dest_line_skip;
	noPRINT("DEST1: %u/4 = %u, line %u.%u/%u\n",
		(int)(dest1-data),
		(int)(dest1-data)/bytes_per_pixel,
		(int)(dest1-data)/(bytes_per_pixel*xwidth),
		(int)(dest1-data)%(bytes_per_pixel*xwidth), height ) ;
    }
    DASSERT_MSG( dest1 == data + (bytes_per_pixel*xwidth) * height,
	"%p - %p %zd, dest= %lu, col = %lu.%lu/%u line\n",
	dest1, data + data_size, dest1 - data - data_size,
	(dest1-data)/bytes_per_pixel,
	(dest1-data)/(bytes_per_pixel*xwidth),
	(dest1-data)%(bytes_per_pixel*xwidth), height ) ;
    FREE(temp_src);


    //--- assign data

    FreeIMG(dest,src);

    dest->data		= data;
    dest->data_alloced	= true;
    dest->width		= width;
    dest->height	= height;
    dest->xwidth	= xwidth;
    dest->xheight	= xheight;
    dest->data_size	= dest->xwidth * dest->xheight * 4;
    dest->iform		= IMG_X_RGB;
    NormalizeFrameIMG(dest);

    PRINT(" - new image: %u*%u -> %u*%u -> %u bytes\n",
		dest->width, dest->height, dest->xwidth, dest->xheight, dest->data_size );

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError HalfIMG
(
    Image_t		* img		// source and dest
)
{
    DASSERT(img);
    Transform2XIMG(img);
    enumError err = ExecTransformIMG(img);
    const uint newwd  = img->width/2;
    const uint newht  = img->height/2;
    if ( err || !newwd || !newht )
	return err;

    NormalizeFrameIMG(img);
    const uint newxwd = EXPAND8(newwd);
    const uint newxht = EXPAND8(newht);
    PRINT("HalfIMG() %u*%u -> %u*%u -> %u*%u\n",
		img->width, img->height, newwd, newht, newxwd, newxht );

    if ( img->iform == IMG_X_GRAY )
    {
	const uint pixel_bytes = 2;
	const uint src_add   = pixel_bytes * img->xwidth;
	const uint dest_add  = pixel_bytes * newxwd;
	const uint data_size = pixel_bytes * newxwd * newxht;

	u8 *data = MALLOC(data_size);
	u8 *dest1 = data;
	const u8 *src1 = img->data;
	uint ih = newht;
	while ( ih-- > 0 )
	{
	    u8 *dest2 = dest1;
	    const u8 *src2 = src1;
	    uint iw = newwd;
	    while ( iw-- > 0 )
	    {
		*dest2++ = ( *src2
				+ src2[pixel_bytes]
				+ src2[src_add]
				+ src2[src_add+pixel_bytes]
				+ 2 ) / 4;
		src2++;
		*dest2++ = ( *src2
				+ src2[pixel_bytes]
				+ src2[src_add]
				+ src2[src_add+pixel_bytes]
				+ 2 ) / 4;
		src2 += pixel_bytes + 1;
		DASSERT( dest2 <= data + data_size );
		DASSERT( src2 <= img->data + img->data_size );
	    }
	    noPRINT("%3u: %u*%u %5zx..%5zx/%x -> %u*%u %5zx..%5zx/%x\n",
		newht - ih - 1,
		img->width, img->height, src1-img->data, src2-img->data, img->data_size,
		newwd, newht, dest1-data, dest2-data, data_size );

	    dest1 += dest_add;
	    src1  += 2 * src_add;
	    DASSERT( dest1 <= data + data_size );
	    DASSERT( src1 <= img->data + img->data_size );
	}

	img->width = newwd;
	img->height= newht;
	AssignDataGRAY(img,0,data);
	return ERR_OK;
    }

    if ( img->iform >= IMG_X_PAL__MIN && img->iform <= IMG_X_PAL__MAX )
    {
	enumError err = ConvertToRGB(img,img,PAL_AUTO);
	if (err)
	    return err;
    }

    if ( img->iform == IMG_X_RGB )
    {
	const uint pixel_bytes = 4;
	const uint src_add   = pixel_bytes * img->xwidth;
	const uint dest_add  = pixel_bytes * newxwd;
	const uint data_size = pixel_bytes * newxwd * newxht;

	u8 *data = MALLOC(data_size);
	u8 *dest1 = data;
	const u8 *src1 = img->data;
	uint ih = newht;
	while ( ih-- > 0 )
	{
	    u8 *dest2 = dest1;
	    const u8 *src2 = src1;
	    uint iw = newwd;
	    while ( iw-- > 0 )
	    {
		*dest2++ = ( *src2
				+ src2[pixel_bytes]
				+ src2[src_add]
				+ src2[src_add+pixel_bytes]
				+ 2 ) / 4;
		src2++;
		*dest2++ = ( *src2
				+ src2[pixel_bytes]
				+ src2[src_add]
				+ src2[src_add+pixel_bytes]
				+ 2 ) / 4;
		src2++;
		*dest2++ = ( *src2
				+ src2[pixel_bytes]
				+ src2[src_add]
				+ src2[src_add+pixel_bytes]
				+ 2 ) / 4;
		src2++;
		*dest2++ = ( *src2
				+ src2[pixel_bytes]
				+ src2[src_add]
				+ src2[src_add+pixel_bytes]
				+ 2 ) / 4;
		src2 += pixel_bytes + 1;
		DASSERT( dest2 <= data + data_size );
		DASSERT( src2 <= img->data + img->data_size );
	    }
	    noPRINT("%3u: %u*%u %5zx..%5zx/%x -> %u*%u %5zx..%5zx/%x\n",
		newht - ih - 1,
		img->width, img->height, src1-img->data, src2-img->data, img->data_size,
		newwd, newht, dest1-data, dest2-data, data_size );

	    dest1 += dest_add;
	    src1  += 2 * src_add;
	    DASSERT( dest1 <= data + data_size );
	    DASSERT( src1 <= img->data + img->data_size );
	}

	img->width  = newwd;
	img->height = newht;
	AssignDataRGB(img,0,data);
    }
    else
	return ERROR0(ERR_INTERNAL,0);

    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			patching helpers		///////////////
///////////////////////////////////////////////////////////////////////////////

static PatchImage_t calc_copy_range
(
    PatchRange1_t	*dest,
    PatchRange1_t	*src1,
    PatchRange1_t	*src2,
    PatchImage_t	patch_mode,
    PatchImage_t	align_mode
)
{
    DASSERT(dest);
    DASSERT(src1);
    DASSERT(src2);

    switch ( patch_mode & PIM_M_SIZE )
    {
	case PIM_GROW:
	    dest->size = src1->size > src2->size ? src1->size : src2->size;
	    break;

	case PIM_SHRINK:
	    dest->size = src1->size < src2->size ? src1->size : src2->size;
	    break;

	default: // PIM_LEAVE
	    dest->size = src1->size;
	    break;
    };

    switch ( align_mode & PIM_M_V )
    {
	case PIM_TOP:
	    src1->dest = 0;
	    src1->src  = 0;
	    src1->len  = dest->size < src1->size ? dest->size : src1->size;

	    src2->dest = 0;
	    src2->src  = 0;
	    src2->len  = dest->size < src2->size ? dest->size : src2->size;
	    break;

	case PIM_BOTTOM:
	    if ( dest->size >= src1->size )
	    {
		src1->dest = dest->size - src1->size;
		src1->src  = 0;
		src1->len  = src1->size;
	    }
	    else
	    {
		src1->dest = 0;
		src1->src  = src1->size - dest->size;
		src1->len  = dest->size;
	    }

	    if ( dest->size >= src2->size )
	    {
		src2->dest = dest->size - src2->size;
		src2->src  = 0;
		src2->len  = src2->size;
	    }
	    else
	    {
		src2->dest = 0;
		src2->src  = src2->size - dest->size;
		src2->len  = dest->size;
	    }
	    break;

	case PIM_INS_TOP:
	    // override the local size and global copy mode!
	    patch_mode = patch_mode & ~PIM_M_COPY | PIM_COPY;
	    dest->size = src1->size + src2->size;

	    src1->dest = src2->size;
	    src1->src  = 0;
	    src1->len  = src1->size;

	    src2->dest = 0;
	    src2->src  = 0;
	    src2->len  = src2->size;
	    break;

	case PIM_INS_BOTTOM:
	    // override the local size and global copy mode!
	    patch_mode = patch_mode & ~PIM_M_COPY | PIM_COPY;
	    dest->size = src1->size + src2->size;

	    src1->dest = 0;
	    src1->src  = 0;
	    src1->len  = src1->size;

	    src2->dest = src1->size;
	    src2->src  = 0;
	    src2->len  = src2->size;
	    break;

	default: // PIM_VCENTER:
	    if ( dest->size >= src1->size )
	    {
		src1->dest = ( dest->size - src1->size ) / 2;
		src1->src  = 0;
		src1->len  = src1->size;
	    }
	    else
	    {
		src1->dest = 0;
		src1->src  = ( src1->size - dest->size ) / 2;
		src1->len  = dest->size;
	    }

	    if ( dest->size >= src2->size )
	    {
		src2->dest = ( dest->size - src2->size ) / 2;
		src2->src  = 0;
		src2->len  = src2->size;
	    }
	    else
	    {
		src2->dest = 0;
		src2->src  = ( src2->size - dest->size ) / 2;
		src2->len  = dest->size;
	    }
	    break;
    }

    return patch_mode;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			PatchIMG()			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError PatchIMG
(
    Image_t		* dest_img,	// destination image, can be one of the sources
    Image_t		* src_img1,	// base source
    Image_t		* src_img2,	// patch source
    PatchImage_t	patch_mode	// patch mode
)
{
    DASSERT(dest_img);
    DASSERT(src_img1);
    DASSERT(src_img2);

    if ( src_img1->iform != IMG_X_RGB )
	ConvertIMG(src_img1,false,0,IMG_X_RGB,PAL_AUTO);
    if ( src_img2->iform != IMG_X_RGB )
	ConvertIMG(src_img2,false,0,IMG_X_RGB,PAL_AUTO);


    //--- calculate copy range

    PatchRange2_t dest, src1, src2;
    src1.x.size = src_img1->width;
    src1.y.size = src_img1->height;
    src2.x.size = src_img2->width;
    src2.y.size = src_img2->height;

    patch_mode = calc_copy_range( &dest.x, &src1.x, &src2.x,
					patch_mode, patch_mode >> PIM_SHIFT_V_H );
    patch_mode = calc_copy_range( &dest.y, &src1.y, &src2.y,
					patch_mode, patch_mode );

    noPRINT(" - COPY-X: %3d..%-3d -> %3d..%-3d  |  %3d..%-3d -> %3d..%-3d  | %3d %3d -> %3d\n",
	    src1.x.src, src1.x.src+src1.x.len, src1.x.dest, src1.x.dest+src1.x.len,
	    src2.x.src, src2.x.src+src2.x.len, src2.x.dest, src2.x.dest+src2.x.len,
	    src1.x.size, src2.x.size, dest.x.size );
    noPRINT(" - COPY-Y: %3d..%-3d -> %3d..%-3d  |  %3d..%-3d -> %3d..%-3d  | %3d %3d -> %3d\n",
	    src1.y.src, src1.y.src+src1.y.len, src1.y.dest, src1.y.dest+src1.y.len,
	    src2.y.src, src2.y.src+src2.y.len, src2.y.dest, src2.y.dest+src2.y.len,
	    src1.y.size, src2.y.size, dest.y.size );


    //--- create the new image

    if ( dest_img == src_img1
	&& dest.x.size == src1.x.size
	&& dest.y.size == src1.y.size )
    {
	//--- one direct operation

	PRINT("DIRECT IMAGE OPERATION\n");
	CopyRectIMG( dest_img,	src2.x.dest, src2.y.dest,
		     src_img2,	src2.x.src,  src2.y.src,
				src2.x.len,  src2.y.len,
		     patch_mode );
    }
    else
    {
	//--- we need a temporary image

	Color_t transparent;
	SETCOLOR(transparent,0xff,0xff,0xff,0x00);
	Image_t temp_img;
	CreateIMG(&temp_img,true,dest.x.size,dest.y.size,transparent);

	CopyRectIMG( &temp_img,	src1.x.dest, src1.y.dest,
		     src_img1,	src1.x.src,  src1.y.src,
				src1.x.len,  src1.y.len,
		     PIM_COPY );

	CopyRectIMG( &temp_img,	src2.x.dest, src2.y.dest,
		     src_img2,	src2.x.src,  src2.y.src,
				src2.x.len,  src2.y.len,
		     patch_mode );

	MoveDataIMG(dest_img,&temp_img);
    }
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			patch interface			///////////////
///////////////////////////////////////////////////////////////////////////////

static bool	pimg_valid	= false;
static uint	n_pimg		= 0;
static Image_t	*pimg		= 0;

///////////////////////////////////////////////////////////////////////////////

bool SetupPatchListIMG()
{
    static bool active = false;
    if ( active || !patch_image_list.used )
	return false;

    if (pimg_valid)
	return n_pimg > 0;
    pimg_valid = active = true;

    TRACE("-\n");
    TRACE_SIZEOF(PatchRange1_t);
    TRACE_SIZEOF(PatchRange2_t);
    TRACE("-\n");

    DASSERT(!pimg);
    pimg = CALLOC(patch_image_list.used,sizeof(*pimg));
    n_pimg = 0;

    uint ip;
    for ( ip = 0; ip < patch_image_list.used; ip++ )
    {
	const FormatFieldItem_t * ffi = patch_image_list.list + ip;
	Image_t *img = pimg + n_pimg;
	if (LoadIMG(img,true,ffi->key,0,false,true,false))
	    ResetIMG(img);
	else
	{
	    n_pimg++;
	    img->patch_mode = ffi->patch_mode;
	    ConvertIMG(img,false,0,IMG_X_RGB,PAL_AUTO);
	    PRINT("PATCH-IMG LOADED: %04x = %s\n",img->patch_mode,img->path);
	}
    }

    active = false;
    return n_pimg > 0;
}

///////////////////////////////////////////////////////////////////////////////

void ResetPatchListIMG()
{
    uint ip;
    for ( ip = 0; ip < n_pimg; ip++ )
	ResetIMG( pimg + ip );
    FREE(pimg);

    pimg_valid	= false;
    n_pimg	= 0;
    pimg	= 0;
}

///////////////////////////////////////////////////////////////////////////////

enumError PatchListIMG
(
    Image_t		* img		// image to transform
)
{
    DASSERT(img);
    if ( img->patch_done || !SetupPatchListIMG() )
	return ERR_OK;

    FreeMipmapsIMG(img);
    Transform2XIMG(img);
    enumError err = ExecTransformIMG(img);
    if ( err || img->iform != IMG_X_RGB )
	return err;

    PRINT("PATCH %s\n",img->path);

    uint ip;
    for ( ip = 0; ip < n_pimg && err == ERR_OK; ip++ )
    {
	Image_t *pat = pimg + ip;
	if ( verbose > 1 )
	    fprintf(stdlog,"  - PATCH %04x = %s\n",pat->patch_mode,pat->path);
	err = PatchIMG(img,img,pat,pat->patch_mode);
    }

    img->patch_done = true;
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			image lists			///////////////
///////////////////////////////////////////////////////////////////////////////

void PrintImageHead
(
    int			line_indent,	// indention of complete line
    uint		long_count	// set the verbosity
)
{
    if ( long_count > 1 )
	printf( "\n"
		"%*sfile   image     mip  width     image  attributes(Gray,Alpha,Palette)\n"
		"%*stype   type      map  height     size  ^  block file path\n"
		"%*s%.79s\n",
		line_indent,"", line_indent,"", line_indent,"", Minus300 );
    else if ( long_count )
	printf( "\n"
		"%*sfile   image     mip  width     image\n"
		"%*stype   type      map  height     size  file path\n"
		"%*s%.79s\n",
		line_indent,"", line_indent,"", line_indent,"", Minus300 );
    else
	printf( "\n"
		"%*sfile   image     mip\n"
		"%*stype   type      map file path\n"
		"%*s%.79s\n",
		line_indent,"", line_indent,"", line_indent,"", Minus300 );
}

///////////////////////////////////////////////////////////////////////////////

static ccp SizeColor ( u32 size )
{
    if (!size)
	return colset->warn;

    u32 m = 1;
    while ( !( m & size ) )
	m <<= 1;

    return m != size
	? colset->warn
	: m > 1024
	    ? colset->hint
	    : "";
}

///////////////////////////////////////////////////////////////////////////////

int PrintImage
(
    const void		* data,		// valid data pointer
					// if NULL: force data line
    uint		data_size,	// size of 'data'
    ccp			fname,		// file name for listing
    int			line_indent,	// indention of complete line
    int			fname_indent,	// indention of 'fname'
    uint		long_count,	// set the verbosity
    bool		ignore		// true: ignore non images
)
{
    DASSERT(fname);

    Image_t img;
    InitializeIMG(&img);
    const ImageGeometry_t *geo = 0;

    if ( data && data_size )
    {
	ScanDataIMG(&img,true,data,data_size);
	geo = GetImageGeometry(img.iform);

	if ( img.info_fform == FF_PNG && long_count )
	    LoadPNG(&img,false,0,fname,0);
    }
    else
	ignore = false;

    if (geo)
    {
	printf("%-6s %02x %-6s",
	    GetNameFF(0,img.info_fform),
	    img.iform, GetImageFormatName(img.iform,"?") );

	if ( img.info_n_image < 1 || img.info_n_image == 1 && ignore )
	    fputs("  -",stdout);
	else
	    printf(" %2u",img.info_n_image-1);

	if (long_count)
	{
	    ccp wcol, hcol;
	    if (colorize_stdout)
	    {
		wcol = SizeColor(img.width);
		hcol = SizeColor(img.height);
	    }
	    else
		wcol = hcol = "";

	    printf(" %s%4u%s*%s%-4u%s %7u",
		wcol, img.width, *wcol ? TermTextModeReset : "",
		hcol, img.height, *hcol ? TermTextModeReset : "",
		img.xwidth * img.xheight * geo->bits_per_pixel / 8 );
	}

	if (long_count>1)
	    printf("  %c%c%c %u*%u",
		geo->is_gray     ? 'G' :'-',
		geo->has_alpha   ? 'A' :'-',
		geo->max_palette ? 'P' :'-',
		geo->block_width, geo->block_height );

	printf("  %*s%s\n",fname_indent,"",fname);
    }
    else if ( !ignore || img.width && img.height )
    {
	printf("%-6s  - -       -", GetNameFF(0,img.info_fform) );
	if (long_count)
	{
	    if ( img.width && img.height )
		printf(" %4u*%-4u       -",img.width,img.height);
	    else
		fputs("     -           -",stdout);
	}
	if (long_count>1)
	    fputs("  ---  - ",stdout);
	printf("  %*s%s\n", fname_indent,"", fname );
    }

    ResetIMG(&img);
    return 0;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		Median Cut: data structures		///////////////
///////////////////////////////////////////////////////////////////////////////

#undef CHAN
#define CHAN 4

#if 0
  #define mcPRINT PRINT
#else
  #define mcPRINT noPRINT
#endif

//-----------------------------------------------------------------------------

typedef struct mc_elem_t
{
    u8			val[CHAN];	// channel tuple
    u32			index;		// index of element

} mc_elem_t;

//-----------------------------------------------------------------------------

typedef struct mc_block_t
{
    u8			min[CHAN];	// minimum value of each channel
    u8			max[CHAN];	// maximum value of each channel
    uint		max_len;	// maximum length of all channels

    mc_elem_t		* elem_beg;	// pointer to first element
    mc_elem_t		* elem_end;	// pointer to end of element list

    struct mc_block_t	* next;		// pointer to next block

} mc_block_t;

//-----------------------------------------------------------------------------

typedef struct mc_t
{
    uint		n_elem;		// number of element
    mc_elem_t		* elem;		// list with all elements, alloced

    uint		n_block;	// number of used blocks
    uint		max_block;	// max number of blocks
    mc_block_t		* block_head;	// head of sorted list of blocks
    mc_block_t		* block_data;	// alloced blocked data
    mc_block_t		last_block;	// special last block

} mc_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////		    Median Cut: Debugging		///////////////
///////////////////////////////////////////////////////////////////////////////
#if HAVE_PRINT
///////////////////////////////////////////////////////////////////////////////

static void DumpBlock ( const mc_t *mc, const mc_block_t *blk, ccp title )
{
    DASSERT(mc);
    DASSERT(blk);
    DASSERT(CHAN==4);

    mcPRINT("BLOCK %u/%u/%u [%s]\n",
	(int)( blk - mc->block_data ), mc->n_block, mc->max_block,
	title ? title : "" );

    mcPRINT("  ** range: %02x..%02x %02x..%02x %02x..%02x %02x..%02x"
	  " ** len: %3u=0x%03x ** elem: %u..%u %3u/%u **\n",
	blk->min[0], blk->max[0],
	blk->min[1], blk->max[1],
	blk->min[2], blk->max[2],
	blk->min[3], blk->max[3],
	blk->max_len, blk->max_len,
	(int)( blk->elem_beg - mc->elem ),
	(int)( blk->elem_end - mc->elem ),
	(int)( blk->elem_end - blk->elem_beg ),
	mc->n_elem );
}

///////////////////////////////////////////////////////////////////////////////

static void DumpBlockList ( const mc_t *mc, ccp title )
{
    DASSERT(mc);

    mcPRINT("BLOCK-LIST %u/%u [%s]\n",
	mc->n_block, mc->max_block, title ? title : "" );

    mc_block_t * blk = mc->block_head;
    uint cnt;
    for ( cnt = 0; blk; cnt++, blk = blk->next )
    {
	mcPRINT("%4u %s: len: %3u=0x%03x ** elem: %u..%u %3u/%u\n",
		cnt, info, blk->max_len, blk->max_len,
		(int)( blk->elem_beg - mc->elem ),
		(int)( blk->elem_end - mc->elem ),
		(int)( blk->elem_end - blk->elem_beg ),
		mc->n_elem );
    }
}

///////////////////////////////////////////////////////////////////////////////

#define DUMPBLOCK(m,b,t) DumpBlock(m,b,t)
#define DUMPBLOCKLIST(m,t) DumpBlockList(m,t)

#else // !HAVE_PRINT
    #define DUMPBLOCK(m,b,t)
    #define DUMPBLOCKLIST(m,t)
#endif

#define noDUMPBLOCK(m,b,t)
#define noDUMPBLOCKLIST(m,t)

//
///////////////////////////////////////////////////////////////////////////////
///////////////		    Median Cut: Helpers			///////////////
///////////////////////////////////////////////////////////////////////////////

static uint CalcMinMax ( mc_block_t *blk )
{
    DASSERT(blk);
    memset(blk->min,~0,sizeof(blk->min));
    memset(blk->max, 0,sizeof(blk->max));

    const mc_elem_t *elem;
    for ( elem = blk->elem_beg; elem < blk->elem_end; elem++ )
    {
	uint c;
	for ( c = 0; c < CHAN; c++ )
	{
	    const u8 val = elem->val[c];
	    noPRINT("C: %02x -> %02x..%02x\n", val, blk->min[c], blk->max[c] );
	    if ( blk->min[c] > val )
		 blk->min[c] = val;
	    if ( blk->max[c] < val )
		 blk->max[c] = val;
	}
    }

    uint max_len = 0, c;
    for ( c = 0; c < CHAN; c++ )
    {
	const uint ml = blk->max[c] - blk->min[c];
	if ( max_len < ml )
	     max_len = ml;
    }

    return blk->max_len = max_len;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		    Median Cut: Interface		///////////////
///////////////////////////////////////////////////////////////////////////////

uint MedianCut
(
    const u32		* data,		// input: 4-tuple aka RGBA
					// data is read at the very beginning
					// and may overlay with 'index' and/or 'pal'
    u16			* index,	// not NULL: store index into palette,
					//           can be the space as 'data'

    // The follwing 3 parameters are used for 'data' and 'index'.
    // They allow to skip unused pixel at the end or each row.
    uint		width,		// used pixels per row
    uint		xwidth,		// >width: pixels per row
    uint		height,		// number or rows

    // pallette data
    u32			* pal,		// not NULL: store calculated palette
    uint		pal_elem	// number of wanted palette entries

    // return value = number of calculated palette values
)
{
    DASSERT(data);
    DASSERT(width);
    DASSERT(height);
    DASSERT(pal);
    DASSERT(pal_elem);

    PRINT("MedianCut(), %d,%dx%d, pal=%d, n_pal=%u\n",
		width, xwidth, height, pal!=0, pal_elem );

 #if HAVE_PRINT || DEBUG
    u64 usec = GetTimerUSec();
 #endif

    static bool done = false;
    if (!done)
    {
	done = true;
	TRACE("- median cut\n");
	TRACE_SIZEOF(mc_t);
	TRACE_SIZEOF(mc_block_t);
	TRACE_SIZEOF(mc_elem_t);
	TRACE("-\n");
    }


    //--- setup data structure

    const uint n_elem = width * height;
    if ( xwidth <= width )
    {
	// normalization and a little optimization
	width = xwidth = n_elem;
	height = 1;
    }

    if ( pal_elem > 0x10000 )
	 pal_elem = 0x10000;

    mc_t mc;
    memset(&mc,0,sizeof(mc));
    mc.n_elem		= n_elem;
    mc.elem		= CALLOC(sizeof(*mc.elem),n_elem);
    mc.n_block		= 1;
    mc.max_block	= pal_elem;
    mc.block_data	= CALLOC(sizeof(*mc.block_data),pal_elem);
    mc.block_head	= mc.block_data;
    mc.last_block.elem_beg = mc.elem;
    mc.last_block.elem_end = mc.elem;

    //--- setup first block

    {
	mc_block_t * block	= mc.block_head;
	block->next		= &mc.last_block;
	block->elem_beg	= mc.elem;
	block->elem_end	= mc.elem + n_elem;

	const uint delta = xwidth - width;
	mc_elem_t * elem = mc.elem;
	uint h = height, i = 0;
	while ( h-- > 0 )
	{
	    uint w = width;
	    while ( w-- > 0 )
	    {
		elem->index = i++;
		memcpy(elem->val,data,sizeof(elem->val));
		elem++;
		data++;
	    }
	    data += delta;
	    i += delta;
	}
	CalcMinMax(block);
	DUMPBLOCK(&mc,block,"startup");
    }


    //--- median cut

    while ( mc.n_block < mc.max_block )
    {
	//--- pick first block

	mc_block_t * b1 = mc.block_head;
	DASSERT( b1 );
	DASSERT( b1->next );
	DASSERT( b1->max_len >= b1->next->max_len );

	if (!b1->max_len)
	    break;

	mc.block_head = b1->next;


	//--- find channel with max length

	uint c;
	for ( c = 0; b1->max[c] - b1->min[c] != b1->max_len; c++ )
	    ;
	DASSERT( c < CHAN );


	//--- split block

	DASSERT( mc.n_block < mc.max_block );
	mc_block_t *b2 = mc.block_data + mc.n_block++;
	memcpy(b2,b1,sizeof(*b2));

	const u8 split_val = ( b1->min[c] + b1->max[c] + 1 ) / 2;
	mcPRINT("\n----- SPLIT b=%zu,%zu, c#%u at %02x, len=%02x -----\n",
		b1 - mc.block_data, b2 - mc.block_data,
		c, split_val, b1->max_len );
	//b1->max[c] = b1->min[c];
	//b2->min[c] = b2->max[c];

	mc_elem_t * e1 = b1->elem_beg;
	mc_elem_t * e2 = b1->elem_end - 1;
	for(;;)
	{
	    //--- find elements to swap

	    while ( e1 < e2 && e1->val[c] <  split_val )
		e1++;

	    while ( e1 < e2 && e2->val[c] >= split_val )
		e2--;

	    if ( e1 >= e2 )
		break;

	    //--- swap elements

	    noPRINT("SWAP: %zu,%zu : #%u %02x >= %02x > %02x\n",
			e1-mc.elem,e2-mc.elem,
			c, e1->val[c], split_val, e2->val[c] );
	    mc_elem_t temp;
	    memcpy(&temp,e1,sizeof(temp));
	    memcpy(e1,e2,sizeof(temp));
	    memcpy(e2,&temp,sizeof(temp));
	}
	mcPRINT(" %zu,%zu,%zu,%zu : #%u %02x < %02x <= %02x\n",
		b1->elem_beg - mc.elem, e1 - mc.elem,
		e2 - mc.elem, b1->elem_end - mc.elem,
		c, e1[-1].val[c], split_val, e1[0].val[c] );
	DASSERT( e1 > b1->elem_beg );
	DASSERT( e1 < b1->elem_end );
	DASSERT ( e1[-1].val[c] <  split_val );
	DASSERT ( e1[ 0].val[c] >= split_val );

	b1->elem_end = b2->elem_beg = e1;
	CalcMinMax(b1);
	CalcMinMax(b2);

	DUMPBLOCK(&mc,b1,"split 1");
	DUMPBLOCK(&mc,b2,"split 2");


	//--- insert both blocks into list

	noDUMPBLOCKLIST(&mc,"0");

	if ( b1->max_len < b2->max_len )
	{
	    mc_block_t *temp = b1;
	    b1 = b2;
	    b2 = temp;
	}

	mc_block_t **ptr = &mc.block_head;
	while ( (*ptr)->max_len > b1->max_len )
	    ptr = &(*ptr)->next;
	b1->next = *ptr;
	*ptr = b1;
	noDUMPBLOCKLIST(&mc,"b1");

	while ( (*ptr)->max_len > b2->max_len )
	    ptr = &(*ptr)->next;
	b2->next = *ptr;
	*ptr = b2;
	noDUMPBLOCKLIST(&mc,"b2");
    }
    DUMPBLOCKLIST(&mc,"END");


    //--- fill index table

    if (index)
    {
	const uint max_index = xwidth * height;
	memset(index,0,max_index*sizeof(*index));
	uint blk_idx = 0;
	const mc_block_t * blk;
	for ( blk = mc.block_head; blk != &mc.last_block; blk = blk->next, blk_idx++ )
	{
	    const mc_elem_t * elem;
	    for ( elem = blk->elem_beg; elem < blk->elem_end; elem++ )
	    {
		DASSERT( elem->index < max_index );
		index[elem->index] = blk_idx;
	    }
	}
    }


    //--- fill palette table

    if (pal)
    {
	u8 *dest = (u8*)pal;
	const mc_block_t * blk;
	for ( blk = mc.block_head; blk != &mc.last_block; blk = blk->next )
	{
	    uint val[CHAN];
	    memset(val,0,sizeof(val));

	    const mc_elem_t * elem;
	    for ( elem = blk->elem_beg; elem < blk->elem_end; elem++ )
	    {
		uint c;
		for ( c = 0; c < CHAN; c++ )
		    val[c] += elem->val[c];
	    }

	    const uint n = blk->elem_end - blk->elem_beg;
	    DASSERT(n);
	    uint c;
	    for ( c = 0; c < CHAN; c++ )
		*dest++ = ( val[c] + n/2 ) / n;
	}
    }


    //--- clean & return

    FREE(mc.elem);
    FREE(mc.block_data);


 #if HAVE_PRINT
    usec = GetTimerUSec() - usec;
    PRINT("MEDIAN-CUT: elem=%u, pal=%u/%u, %llu µsec\n",
		n_elem, mc.n_block, pal_elem, usec );
 #elif DEBUG
    usec = GetTimerUSec() - usec;
    TRACE("MEDIAN-CUT: elem=%u, pal=%u/%u, %llu µsec\n",
		n_elem, mc.n_block, pal_elem, usec );
 #else
    PRINT("MEDIAN-CUT: elem=%u, pal=%u/%u\n", n_elem, mc.n_block, pal_elem );
 #endif

    return mc.n_block;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////

