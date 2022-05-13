
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
 *   Copyright (c) 2011-2022 by Dirk Clemens <wiimm@wiimm.de>              *
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
#include "lib-breff.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			IMG management			///////////////
///////////////////////////////////////////////////////////////////////////////

uint image_seq_num = 0;
static int convert_depth = 0;

//-----------------------------------------------------------------------------

void InitializeIMG (  Image_t * img )
{
    DASSERT(img);
    noPRINT("InitializeIMG() img=%p\n",img);

    memset(img,0,sizeof(*img));

    img->iform		= IMG_INVALID;
    img->pform		= PAL_INVALID;

    img->info_fform	= FF_INVALID;
    img->info_iform	= IMG_INVALID;
    img->info_pform	= PAL_INVALID;

    img->tform_fform0	= img->tform_fform = FF_INVALID;
    img->tform_iform0	= img->tform_iform = IMG_INVALID;
    img->tform_pform0	= img->tform_pform = PAL_INVALID;

    img->endian		= &be_func;
    img->path		= EmptyString;

    //img->seq_num	= ++image_seq_num;
}

///////////////////////////////////////////////////////////////////////////////

void ResetIMG ( Image_t * img )
{
    DASSERT(img);
    noPRINT("ResetIMG() img=%p, container=%p\n",img,img->container);

    FreeIMG(img,0);
    image_seq_num--;
    InitializeIMG(img);
}

///////////////////////////////////////////////////////////////////////////////

void RemoveContainerIMG ( Image_t * img )
{
    DASSERT(img);
    if (img->container)
    {
	u8 * container = img->container;
	while (img)
	{
	    if ( img->data && !img->data_alloced )
	    {
		u8 *data		= MALLOC(img->data_size);
		memcpy(data,img->data,img->data_size);
		img->data		= data;
		img->data_alloced	= true;
	    }

	    if ( img->pal && !img->pal_alloced )
	    {
		u8 *pal			= MALLOC(img->pal_size);
		memcpy(pal,img->pal,img->pal_size);
		img->pal		= pal;
		img->pal_alloced	= true;
	    }

	    img->container = 0;
	    img = img->mipmap;
	}
	FREE(container);
    }
}

///////////////////////////////////////////////////////////////////////////////

void FreeIMG
(
    Image_t		* img,		// destination image, dynamic data freed
    const Image_t	* src		// not NULL: copy parameters, but not data
)
{
    DASSERT(img);

    //--- free data

    if (img->mipmap)
	FreeMipmapsIMG(img);

    FREE(img->container);

    if (img->data_alloced)
	FREE(img->data);

    if (img->pal_alloced)
	FREE(img->pal);


    //--- copy param & path handling

    if ( !src )
    {
	if (img->path_alloced)
	    FreeString(img->path);
	img->path_alloced = 0;
	img->path = EmptyString;
    }
    else if ( src != img )
    {
	if (img->path_alloced)
	    FreeString(img->path);

	memcpy(img,src,sizeof(*img));

	if (img->path_alloced)
	    img->path = STRDUP(src->path);
    }


    //--- reset dynamic data

    img->container	= 0;

    img->data		= 0;
    img->data_size	= 0;
    img->data_alloced	= false;

    img->pal		= 0;
    img->pal_size	= 0;
    img->pal_alloced	= false;
    img->n_pal		= 0;

    img->seq_num	= ++image_seq_num;
    img->conv_count++;
}

///////////////////////////////////////////////////////////////////////////////

void FreeMipmapsIMG
(
    Image_t		* img		// not NULL: remove mipmaps
)
{
    if ( img && img->mipmap )
    {
	ResetIMG(img->mipmap);
	FREE(img->mipmap);
	img->mipmap = 0;
    }
}

///////////////////////////////////////////////////////////////////////////////

uint CountMipmapsIMG
(
    const Image_t	* img		// count mipmaps
)
{
    DASSERT(img);

    uint count = 0;
    for(;;)
    {
	img = img->mipmap;
	if (!img)
	    return count;
	count++;
    }
}

///////////////////////////////////////////////////////////////////////////////

void CopyIMG
(
    Image_t		* dest,		// destination image
    bool		init_dest,	// true: initialize 'dest' first
    const Image_t	* src,		// valid source image
    bool		copy_mipmap	// true: copy mipmaps too
)
{
    DASSERT(dest);
    noPRINT("%*sCopyIMG() %p[%d] < %p mm=%d\n",
		convert_depth*2, "",
		dest, init_dest, src, src ? CountMipmapsIMG(src) : -1 );

    if (init_dest)
	InitializeIMG(dest);

    if ( src != dest )
    {
	if (!init_dest)
	    ResetIMG(dest);

	if (src)
	{
	    DASSERT(!dest->mipmap);
	    memcpy(dest,src,sizeof(*dest));

	    if ( copy_mipmap && dest->mipmap )
	    {
		dest->mipmap = MALLOC(sizeof(*dest->mipmap));
		CopyIMG(dest->mipmap,true,src->mipmap,true);
	    }
	    else
		dest->mipmap = 0;

	    if ( dest->data_alloced || dest->container )
	    {
		dest->data = MALLOC(dest->data_size);
		memcpy(dest->data,src->data,dest->data_size);
		dest->data_alloced = true;
	    }

	    if ( dest->pal_alloced || dest->container )
	    {
		dest->pal = MALLOC(dest->pal_size);
		memcpy(dest->pal,src->pal,dest->pal_size);
		dest->pal_alloced = true;
	    }

	    dest->container = 0;

	    if (dest->path_alloced)
		dest->path = STRDUP(src->path);
	}
    }
}

///////////////////////////////////////////////////////////////////////////////

void MoveIMG
(
    Image_t		* dest,		// destination image
    bool		init_dest,	// true: initialize 'dest' first
    Image_t		* src		// NULL or source image, isresetted
)
{
    DASSERT(dest);

    if (init_dest)
	InitializeIMG(dest);

    if ( src != dest )
    {
	if (!init_dest)
	    ResetIMG(dest);
	if (src)
	{
	    memcpy(dest,src,sizeof(*dest));
	    InitializeIMG(src);
	}
    }
}

///////////////////////////////////////////////////////////////////////////////

void MoveDataIMG
(
    Image_t		* dest,		// valid destination image
    Image_t		* src		// valid source image, is resetted
)
{
    DASSERT(dest);
    DASSERT(src);

    if ( src != dest )
    {
	if (dest->container)
	    FREE(dest->container);
	dest->container		= src->container;
	src->container		= 0;

	dest->iform		= src->iform;
	if (dest->data_alloced)
	    FREE(dest->data);
	dest->data		= src->data;
	dest->data_size		= src->data_size;
	dest->data_alloced	= src->data_alloced;
	src->data_alloced	= false;

	dest->pform		= src->pform;
	if (dest->pal_alloced)
	    FREE(dest->pal);
	dest->pal		= src->pal;
	dest->pal_size		= src->pal_size;
	dest->n_pal		= src->n_pal;
	dest->pal_alloced	= src->pal_alloced;
	src->pal_alloced	= false;

	dest->mipmap		= src->mipmap;
	src->mipmap		= 0;

	dest->width		= src->width;
	dest->height		= src->height;
	dest->xwidth		= src->xwidth;
	dest->xheight		= src->xheight;

	ResetIMG(src);
    }
}

///////////////////////////////////////////////////////////////////////////////

void ScanDataIMG
(
    Image_t		* img,		// destination image
    bool		init_img,	// true: initialize 'img' first
    const void		* data,		// data to scan
    uint		data_size	// size of 'data'
)
{
    DASSERT(img);

    if (init_img)
	InitializeIMG(img);
    else
	ResetIMG(img);

    if ( !data || !data_size )
	return;

// [[analyse-magic]]
    img->info_fform = GetByMagicFF(data,data_size,0); // [[magic]]
    switch (img->info_fform)
    {
      case FF_TPL:
	{
	    const tpl_header_t * tpl;
	    const tpl_pal_header_t * tp;
	    const tpl_img_header_t * ti;
	    if (SetupPointerTPL( data, data_size, 0,
				&tpl, 0, &tp, &ti, 0, 0, &be_func ))
	    {
		img->iform		= be32(&ti->iform);
		img->info_iform		= img->iform;
		img->width		= be16(&ti->width);
		img->height		= be16(&ti->height);
		img->info_n_image	= be32(&tpl->n_image);
		if (tp)
		    img->pform = img->info_pform = be32(&tp->pform);
	    }
	}
	break;

      case FF_BTI:
	{
	    const bti_header_t *bti = (bti_header_t*)data;
	    img->iform		= bti->iform;
	    img->info_iform	= img->iform;
	    img->width		= be16(&bti->width);
	    img->height		= be16(&bti->height);
	    img->info_n_image	= bti->n_image;
	    img->pform		= be16(&bti->pform);
	    img->info_pform	= img->pform;
	}
	break;

      case FF_TEX:
      case FF_TEX_CT: // ??? [[CTCODE]] add ctcode info
	{
	    const brsub_header_t *bh	= (brsub_header_t*)data;
	    const uint n_grp		= GetSectionNumBRSUB(data,data_size,&be_func);
	    const tex_info_t *ti	= (tex_info_t*)( bh->grp_offset + n_grp );

	    img->iform		= be32(&ti->iform);
	    img->info_iform	= img->iform;
	    img->width		= be16(&ti->width);
	    img->height		= be16(&ti->height);
	    img->info_n_image	= be32(&ti->n_image);
	}
	break;

      case FF_BREFT_IMG:
	{
	    const breft_image_t * bi = (breft_image_t*)data;
	    img->iform		= bi->iform;
	    img->info_iform	= img->iform;
	    img->width		= be16(&bi->width);
	    img->height		= be16(&bi->height);
	    img->info_size	= be32(&bi->img_size);
	    img->info_n_image	= bi->n_image ? bi->n_image : 1;
	}
	break;

      default:
	return;
    }

    const ImageGeometry_t *geo = GetImageGeometry(img->iform);
    if (geo)
    {
	img->xwidth  = ALIGN32(img->width, geo->block_width);
	img->xheight = ALIGN32(img->height,geo->block_height);
    }
    else
    {
	img->xwidth  = EXPAND8(img->width);
	img->xheight = EXPAND8(img->height);
    }
}

///////////////////////////////////////////////////////////////////////////////

bool NormalizeFrameIMG
(
    Image_t		* img		// pointer to valid img
)
{
    bool status = false;

    if ( img->iform >= IMG_X__MIN && img->iform <= IMG_X__MAX )
    {
	const uint cell_size = img->iform == IMG_X_RGB ? 4 : 2;
	// IMG_X_GRAY & IMG_X_PAL* : size is 2

	if ( img->width > 0 && img->xwidth > img->width )
	{
	    const uint fill_size = ( img->xwidth - img->width ) * cell_size;
	    u8 *row = img->data + img->width * cell_size;
	    uint ih = img->height;
	    while ( ih-- > 0 )
	    {
		u8 *src  = row - cell_size;
		u8 *dest = row;
		u8 *end  = row + fill_size;
		while ( dest < end )
		    *dest++ = *src++;
		row += img->xwidth * cell_size;
	    }
	    status = true;
	}

	if ( img->height > 0 && img->xheight > img->height )
	{
	    const uint line_size = img->xwidth * cell_size;
	    u8 *dest = img->data + img->height * line_size;
	    u8 *end  = img->data + img->xheight * line_size;
	    u8 *src  = dest - line_size;
	    while ( dest < end )
		*dest++ = *src++;
	    status = true;
	}
    }

    return status;
}

///////////////////////////////////////////////////////////////////////////////

int CheckAlphaIMG
(
    const Image_t	* img,		// pointer to valid img
    bool		force		// true: don't believe 'img->alpha_status'
)
{
    DASSERT(img);
    if ( !force && img->alpha_status )
	return img->alpha_status;

    uint size = 4;
    switch (img->iform)
    {
	case IMG_X_GRAY:
	    size = 2;
	case IMG_X_RGB:
	    {
		const u8 * data = img->data + size - 1;
		uint ih = img->height;
		while ( ih-- > 0 )
		{
		    uint iw = img->width;
		    while ( iw-- > 0 )
		    {
			if ( *data != 0xff )
			    return ((Image_t*)img)->alpha_status = 1;
			data += size;
		    }
		    data += ( img->xwidth - img->width ) * size;
		}
		return ((Image_t*)img)->alpha_status = -1;
	    }

	case IMG_X_PAL4:
	case IMG_X_PAL8:
	case IMG_X_PAL14:
	    if (img->n_pal)
	    {
		DASSERT(img->pal);
		uint n = img->n_pal;
		const u8 * data = img->pal + 3;
		while ( n-- > 0 )
		{
		    if ( *data != 0xff )
			return ((Image_t*)img)->alpha_status = 1;
		    data += 4;
		}
	    }
	    return ((Image_t*)img)->alpha_status = -1;

	default:
	    {
		const ImageGeometry_t *geo = GetImageGeometry(img->iform);
		return ((Image_t*)img)->alpha_status
			= geo && !geo->has_alpha ? -1 : 0;
	    }
    }
}

///////////////////////////////////////////////////////////////////////////////

bool IsGrayIMG
(
    const Image_t	* img		// pointer to valid img
)
{
    DASSERT(img);
    image_format_t xform;
    NormalizeIF(&xform,img->iform,IMG_X_RGB,img->pform);
    return xform == IMG_X_GRAY;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			conversion helpers		///////////////
///////////////////////////////////////////////////////////////////////////////

static u8 * AllocDataIMG
(
    const Image_t	* src,		// image template (read width and height)
    uint		bytes_per_pix,	// number of bytes per pixel
    uint		* data_size	// not NULL: store data size
)
{
    DASSERT(src);

    const uint size = EXPAND8(src->width) * EXPAND8(src->height) * bytes_per_pix;
    if (data_size)
	*data_size = size;
    return CALLOC(1,size);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void AssignDataGRAY
(
    Image_t		* dest,		// valid destination
    const Image_t	* src,		// NULL or image template
    u8			* data		// with AllocDataIMG() alloced data
)
{
    DASSERT(dest);
    DASSERT(data);

    FreeIMG(dest,src);

    dest->data		= data;
    dest->data_alloced	= true;
    dest->xwidth	= EXPAND8(dest->width);
    dest->xheight	= EXPAND8(dest->height);
    dest->data_size	= dest->xwidth * dest->xheight * 2;
    dest->iform		= IMG_X_GRAY;
    dest->is_grayed	= true;
}

///////////////////////////////////////////////////////////////////////////////

void AssignDataRGB
(
    Image_t		* dest,		// valid destination
    const Image_t	* src,		// NULL or image template
    u8			* data		// with AllocDataIMG() alloced data
)
{
    DASSERT(dest);
    DASSERT(data);

    FreeIMG(dest,src);

    dest->data		= data;
    dest->data_alloced	= true;
    dest->xwidth	= EXPAND8(dest->width);
    dest->xheight	= EXPAND8(dest->height);
    dest->data_size	= dest->xwidth * dest->xheight * 4;
    dest->iform		= IMG_X_RGB;
}

///////////////////////////////////////////////////////////////////////////////

static void AssignDataPAL
(
    Image_t		* dest,		// valid destination
    const Image_t	* src,		// NULL or image template
    u16			* itab,		// with AllocDataIMG() alloced data
    u8			* pal_data,	// palette data
    uint		size_pal,	// NULL or size of 'pal_data'
    uint		n_pal,		// number of used palette entries
    image_format_t	iform,		// palette image format
    int			alpha_status	// alpha status of image
)
{
    DASSERT(dest);
    DASSERT(itab);
    DASSERT(pal_data);
    DASSERT( iform >= IMG_X_PAL__MIN && iform <= IMG_X_PAL__MAX );

    FreeIMG(dest,src);

    dest->data		= (u8*)itab;
    dest->data_alloced	= true;
    dest->xwidth	= EXPAND8(dest->width);
    dest->xheight	= EXPAND8(dest->height);
    dest->data_size	= dest->xwidth * dest->xheight * 2;
    dest->iform		= iform;
    dest->alpha_status	= alpha_status;

    dest->pform		= PAL_X_RGB;
    dest->pal		= pal_data;
    dest->pal_size	= size_pal ? size_pal : n_pal * 4;
    dest->pal_alloced	= true;
    dest->n_pal		= n_pal;
}

///////////////////////////////////////////////////////////////////////////////

static void AssignData
(
    Image_t		* dest,		// valid destination
    const Image_t	* src,		// NULL or image template
    u8			* data,		// alloced data
    uint		data_size,	// size of 'data'
    image_format_t	iform		// image format
)
{
    DASSERT(dest);
    DASSERT(data);

    FreeIMG(dest,src);

    dest->data		= data;
    dest->data_alloced	= true;
    dest->data_size	= data_size;
    dest->xwidth	= EXPAND8(dest->width);
    dest->xheight	= EXPAND8(dest->height);
    dest->iform		= iform;
    dest->pform		= PAL_INVALID;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static enumError CalcImageBlock
(
    const Image_t	* img,		// image template (read width and height)
    uint		bits_per_pixel,	// number of bits per pixel
    uint		block_width,	// width of a single block
    uint		block_height,	// height of a single block

    uint		* h_blocks,	// not NULL: store number of horizontal blocks
    uint		* v_blocks,	// not NULL: store number of vertical blocks
    uint		* img_size,	// not NULL: return image size

    bool		calc_only	// true: do no validation
)
{
    DASSERT(img);

    uint xwidth, xheight;
    const uint size = CalcImageSize( img->width, img->height,
			bits_per_pixel, block_width, block_height,
			&xwidth, &xheight, h_blocks, v_blocks );
    if (img_size)
	*img_size = size;

    if ( calc_only || xwidth > 0 && xheight > 0 && size <= img->data_size )
	return ERR_OK;

    PRINT("h=%u,%u,%u  v=%u,%u,%u, size=%u/%u <= %x/%x\n",
		img->width, block_width, xwidth,
		img->height, block_height, xheight,
		size, img->data_size, size, img->data_size );

    return ERROR0(ERR_INVALID_IFORM,
		"Impossible geometry of image [0x%02x=%s]: %s\n",
		img->iform, GetImageFormatName(img->iform,"?"), img->path );
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			palette helpers			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError TransformPalette
(
    Image_t		* dest_img,	// valid destination
    const Image_t	* src_img,	// valid source
    image_format_t	iform,		// new image format, only valid IMG_X_PAL*
    u16			* itab		// index table (2*xwidth*xheight), alloced
)
{
    DASSERT(dest_img);
    DASSERT(src_img);
    DASSERT(itab);
    DASSERT( iform >= IMG_X_PAL__MIN && iform <= IMG_X_PAL__MAX );


    //--- create palette

    const uint n_idx = GetPaletteCountIF(iform);
    DASSERT( n_idx );
    DASSERT( n_idx * 4 <= sizeof(iobuf) );
    u8 *pal_data = CALLOC(n_idx,4);
    u8 *pdest = pal_data;

    u16 (*rd16) ( const void * data_ptr ) = src_img->endian->rd16;
    //u32 (*rd32) ( const void * data_ptr ) = src_img->endian->rd32;

    const u8 * src = src_img->pal;
    const uint n_pal = src ? src_img->n_pal : 0;
    uint pi;

    int alpha_status = 0; // don't know

    switch (src_img->pform)
    {
 #if 0
      case PAL_I4:
	alpha_status = -1;
	for ( pi = n_pal/2; pi > 0; pi-- )
	{
	    const u8 val = *src++;
	    const u8 gray1 = cc48[ val >> 4 ];
	    *pdest++ = gray1;		// red
	    *pdest++ = gray1;		// green
	    *pdest++ = gray1;		// blue
	    *pdest++ = 0xff;		// alpha
	    const u8 gray2 = cc48[ val & 0x0f ];
	    *pdest++ = gray2;		// red
	    *pdest++ = gray2;		// green
	    *pdest++ = gray2;		// blue
	    *pdest++ = 0xff;		// alpha
	}
	break;
 #endif

 #if 0
      case PAL_I8:
	alpha_status = -1;
	for ( pi = n_pal; pi > 0; pi-- )
	{
	    const u8 gray = *src++;
	    *pdest++ = gray;	// red
	    *pdest++ = gray;	// green
	    *pdest++ = gray;	// blue
	    *pdest++ = 0xff;	// alpha
	}
	break;
 #endif

 #if 0
      case PAL_IA4:
	for ( pi = n_pal; pi > 0; pi-- )
	{
	    const u8 val = *src++;
	    const u8 gray = cc48[ val & 0x0f ];
	    *pdest++ = gray;			// red
	    *pdest++ = gray;			// green
	    *pdest++ = gray;			// blue
	    *pdest++ = cc48[ val >> 4 ];	// alpha
	}
	break;
 #endif

      case PAL_IA8:
	for ( pi = n_pal; pi > 0; pi-- )
	{
	    const u16 val = rd16(src);
	    src += 2;
	    *pdest++ = val;		// red
	    *pdest++ = val;		// green
	    *pdest++ = val;		// blue
	    *pdest++ = val >> 8;	// alpha
	}
	break;

      case PAL_RGB565:
	alpha_status = -1;
	for ( pi = n_pal; pi > 0; pi-- )
	{
	    const u16 val = rd16(src);
	    src += 2;
	    *pdest++ = cc58[ val >> 11 ];	 // red
	    *pdest++ = cc68[ val >>  5 & 0x3f ]; // green
	    *pdest++ = cc58[ val       & 0x1f ]; // blue
	    *pdest++ = 0xff;			 // alpha
	}
	break;

      case PAL_RGB5A3:
	for ( pi = n_pal; pi > 0; pi-- )
	{
	    const u16 val = rd16(src);
	    src += 2;
	    if ( val & 0x8000 )
	    {
		*pdest++ = cc58[ val >> 10 & 0x1f ];	// red
		*pdest++ = cc58[ val >>  5 & 0x1f ];	// green
		*pdest++ = cc58[ val       & 0x1f ];	// blue
		*pdest++ = 0xff;			// alpha
	    }
	    else
	    {
		*pdest++ = cc48[ val >>  8 & 0x0f ];	// red
		*pdest++ = cc48[ val >>  4 & 0x0f ];	// green
		*pdest++ = cc48[ val       & 0x0f ];	// blue
		*pdest++ = cc38[ val >> 12 & 0x07 ];	// alpha
	    }
	}
	break;

#if 0 // [[not-needed]] not needed yet 2011-06
      case PAL_RGBA32:
	//alpha_status = 0;
	for ( pi = n_pal; pi > 0; pi-- )
	{
	 #if 1 // unkown, which variant will be true
	    const u32 val = rd32(src);
	    src += 4;
	    *pdest++ = val >> 16;	// red
	    *pdest++ = val >>  8;	// green
	    *pdest++ = val;		// blue
	    *pdest++ = val >> 24;	// alpha
	 #else
	    const u16 ar = rd16(src);
	    const u16 gb = rd16(src+n_pal*2);	// delta unclear ???
	    src += 2;
	    *pdest++ = ar;	// red
	    *pdest++ = gb >> 8;	// green
	    *pdest++ = gb;	// blue
	    *pdest++ = ar >> 8;	// alpha
	 #endif
	}
	break;
 #endif

      default:
	FREE(pal_data);
	FREE(itab);
	return ERROR0(ERR_INVALID_IFORM,
		"Palette format 0x%02x [%s] not supported: %s\n",
		src_img->pform, GetPaletteFormatName(src_img->pform,"?"), src_img->path );
    }


    //--- create image using the palette

    AssignDataPAL( dest_img, src_img, itab,
			pal_data, 0, n_pal, iform, alpha_status );
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		    convert gray : rgb : palette	///////////////
///////////////////////////////////////////////////////////////////////////////

enumError ConvertToRGB
(
    Image_t		* dest_img,	// valid destination
    const Image_t	* src_img,	// valid source
    palette_format_t	pform		// wanted palette format => ignored
)
{
    DASSERT(dest_img);
    DASSERT(src_img);

    //--- mipmap support

    if (src_img->mipmap) // --> do the conversion for each image standalone!
    {
	if ( src_img != dest_img )
	    CopyIMG(dest_img,false,src_img,true); // this makes it easier to handle
	RemoveContainerIMG(dest_img);
	DASSERT(!dest_img->container);

	while (dest_img)
	{
	    Image_t *next = dest_img->mipmap;
	    dest_img->mipmap = 0;
	    enumError err = ConvertToRGB(dest_img,dest_img,pform);
	    dest_img->mipmap = next;
	    if (err)
		return err;
	    dest_img = next;
	}
	return ERR_OK;
    }


    //--- convert image

    if ( src_img->iform == IMG_X_RGB )
    {
	PRINT("%*sCONVERT: RGB {%u,%u} -> RGB {%u,%u}\n",
		convert_depth*2,"",
		src_img->conv_count, src_img->seq_num,
		dest_img->conv_count, dest_img->seq_num );
	CopyIMG(dest_img,false,src_img,true);
    }
    else if ( src_img->iform == IMG_X_GRAY )
    {
	PRINT("%*sCONVERT: GRAY {%u,%u} -> RGB {%u,%u}\n",
		convert_depth*2,"",
		src_img->conv_count, src_img->seq_num,
		dest_img->conv_count, dest_img->seq_num );

	DASSERT( EXPAND8(src_img->width) == src_img->xwidth );
	DASSERT( EXPAND8(src_img->height) == src_img->xheight );

	const u8 *src = src_img->data;
	const u8 *src_end = src + 2 * src_img->xwidth * src_img->xheight;

	uint data_size;
	u8 * data = AllocDataIMG(src_img,4,&data_size);
	u8 * dest = data;

	while ( src < src_end )
	{
	    const u8 val = *src++;
	    *dest++ = val;
	    *dest++ = val;
	    *dest++ = val;
	    *dest++ = *src++;
	}

	DASSERT( dest == data + data_size );
	AssignDataRGB(dest_img,src_img,data);
    }
    else if ( src_img->iform >= IMG_X_PAL__MIN && src_img->iform <= IMG_X_PAL__MAX )
    {
	PRINT("%*sCONVERT: PAL {%u,%u} -> RGB {%u,%u}\n",
		convert_depth*2,"",
		src_img->conv_count, src_img->seq_num,
		dest_img->conv_count, dest_img->seq_num );

	uint data_size;
	u8 *data = AllocDataIMG(src_img,4,&data_size);

	const uint n_pal = src_img->n_pal;
	if (!n_pal)
	    memset(data,0xff,data_size);
	else
	{
	    DASSERT(src_img->pal);
	    const u32 * pal = (u32*)src_img->pal;

	    const u16 *src = (u16*)src_img->data;
	    const u16 *src_end = src + src_img->xwidth * src_img->xheight;

	    u8 *dest = data;

	    while ( src < src_end )
	    {
		const u16 val = *src++;
		memcpy( dest, pal + ( val < n_pal ? val : 0 ), 4 );
		dest += 4;
	    }

	    DASSERT( dest == data + data_size );
	}

	AssignDataRGB(dest_img,src_img,data);
    }
    else
	ASSERT(0);

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError ConvertToGRAY
(
    Image_t		* dest_img,	// valid destination
    const Image_t	* src_img,	// valid source
    palette_format_t	pform		// wanted palette format => ignored
)
{
    DASSERT(dest_img);
    DASSERT(src_img);

    //--- mipmap support

    if (src_img->mipmap) // --> do the conversion for each image standalone!
    {
	if ( src_img != dest_img )
	    CopyIMG(dest_img,false,src_img,true); // this makes it easier to handle
	RemoveContainerIMG(dest_img);
	DASSERT(!dest_img->container);

	while (dest_img)
	{
	    Image_t *next = dest_img->mipmap;
	    dest_img->mipmap = 0;
	    enumError err = ConvertToGRAY(dest_img,dest_img,pform);
	    dest_img->mipmap = next;
	    if (err)
		return err;
	    dest_img = next;
	}
	return ERR_OK;
    }


    //--- convert image

    if ( src_img->iform == IMG_X_GRAY )
    {
	PRINT("%*sCONVERT: GRAY {%u,%u} -> GRAY {%u,%u}\n",
		convert_depth*2,"",
		src_img->conv_count, src_img->seq_num,
		dest_img->conv_count, dest_img->seq_num );

	CopyIMG(dest_img,false,src_img,true);
	return ERR_OK;
    }

    if ( src_img->iform >= IMG_X_PAL__MIN && src_img->iform <= IMG_X_PAL__MAX )
    {
	convert_depth++;
	enumError err = ConvertToRGB(dest_img,src_img,pform);
	convert_depth--;
	if (err)
	    return err;
	src_img = dest_img;
    }

    if ( src_img->iform == IMG_X_RGB )
    {
	PRINT("%*sCONVERT: RGB {%u,%u} -> GRAY {%u,%u}\n",
		convert_depth*2,"",
		src_img->conv_count, src_img->seq_num,
		dest_img->conv_count, dest_img->seq_num );

	DASSERT( EXPAND8(src_img->width) == src_img->xwidth );
	DASSERT( EXPAND8(src_img->height) == src_img->xheight );

	const u8 *src = src_img->data;
	const u8 *src_end = src + 4 * src_img->xwidth * src_img->xheight;

	uint data_size;
	u8 * data = AllocDataIMG(src_img,2,&data_size);
	u8 * dest = data;

	while ( src < src_end )
	{
	    *dest++ = ( src[0] + src[1] + src[2] + 1 ) / 3;
	    src += 3;
	    *dest++ = *src++;
	}

	DASSERT( dest == data + data_size );
	AssignDataGRAY(dest_img,src_img,data);
    }
    else
	ASSERT(0);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError ConvertToPALETTE
(
    Image_t		* dest_img,	// valid destination
    const Image_t	* src_img,	// valid source
    uint		max_pal,	// maximum palette values, 0 = auto
    image_format_t	iform		// if a valid PAL format:
					//    convert to this and
					//    reduce 'max_pal' if neccessary
)
{
    DASSERT(dest_img);
    DASSERT(src_img);


    //--- mipmap support

    if (src_img->mipmap) // --> do the conversion for each image standalone!
    {
	if ( src_img != dest_img )
	    CopyIMG(dest_img,false,src_img,true); // this makes it easier to handle
	RemoveContainerIMG(dest_img);
	DASSERT(!dest_img->container);

	while (dest_img)
	{
	    Image_t *next = dest_img->mipmap;
	    dest_img->mipmap = 0;
	    enumError err = ConvertToPALETTE(dest_img,dest_img,max_pal,iform);
	    dest_img->mipmap = next;
	    if (err)
		return err;
	    dest_img = next;
	}
	return ERR_OK;
    }


    //--- convert image

    bool auto_mode = false;
    uint pal_limit = 0;
    switch(iform)
    {
	case IMG_X_PAL4:
	    pal_limit = 0x10;
	    break;

	case IMG_X_PAL8:
	    pal_limit = 0x100;
	    break;

	case IMG_X_PAL14:
	    pal_limit = 0x4000;
	    break;

	default:
	    iform = IMG_X_PAL;
	    pal_limit = 0x4000;
	    auto_mode = true;
	    break;
    }

    if ( !max_pal || max_pal > pal_limit )
	max_pal = pal_limit;

    if (   src_img->iform >= IMG_X_PAL__MIN
	&& src_img->iform <= IMG_X_PAL__MAX
	&& src_img->n_pal <= max_pal )
    {
	CopyIMG(dest_img,false,src_img,true);
	if (auto_mode)
	{
	    if ( dest_img->n_pal <= 0x10 )
		iform = IMG_X_PAL4;
	    else if ( dest_img->n_pal <= 0x100 )
		iform = IMG_X_PAL8;
	    else
		iform = IMG_X_PAL14;
	}
	dest_img->iform = iform;
	return ERR_OK;
    }

    if ( src_img->iform != IMG_X_RGB )
    {
	convert_depth++;
	enumError err = ConvertToRGB(dest_img,src_img,PAL_AUTO);
	convert_depth--;
	if (err)
	    return err;
	src_img = dest_img;
    }

    PRINT("%*sCONVERT: RGB {%u,%u} -> PAL {%u,%u}\n",
		convert_depth*2,"",
		src_img->conv_count, src_img->seq_num,
		dest_img->conv_count, dest_img->seq_num );
    if ( src_img->iform != IMG_X_RGB )
	ERROR0(ERR_INTERNAL,0);

    u16 *index = (u16*)AllocDataIMG(src_img,2,0);
    u8  *pal_data = CALLOC(max_pal,4);
    uint n_pal = MedianCut( (u32*)src_img->data, index,
				src_img->width, src_img->xwidth, src_img->height,
				(u32*)pal_data, max_pal );

    if (auto_mode)
    {
	if ( n_pal <= 0x10 )
	    iform = IMG_X_PAL4;
	else if ( n_pal <= 0x100 )
	    iform = IMG_X_PAL8;
	else
	    iform = IMG_X_PAL14;
    }

    AssignDataPAL( dest_img, src_img,
			index, pal_data, max_pal*4, n_pal, iform, 0 );
    return ERR_OK;
}

//-----------------------------------------------------------------------------

static enumError conv_to_palette_auto
(
    Image_t		* dest_img,	// valid destination
    const Image_t	* src_img,	// valid source
    palette_format_t	pform		// wanted palette format => ignored
)
{
    return ConvertToPALETTE(dest_img,src_img,0,IMG_X_PAL);
}

//-----------------------------------------------------------------------------

static enumError conv_to_palette_4
(
    Image_t		* dest_img,	// valid destination
    const Image_t	* src_img,	// valid source
    palette_format_t	pform		// wanted palette format => ignored
)
{
    return ConvertToPALETTE(dest_img,src_img,0,IMG_X_PAL8);
}

//-----------------------------------------------------------------------------

static enumError conv_to_palette_8
(
    Image_t		* dest_img,	// valid destination
    const Image_t	* src_img,	// valid source
    palette_format_t	pform		// wanted palette format => ignored
)
{
    return ConvertToPALETTE(dest_img,src_img,0,IMG_X_PAL8);
}

//-----------------------------------------------------------------------------

static enumError conv_to_palette_14
(
    Image_t		* dest_img,	// valid destination
    const Image_t	* src_img,	// valid source
    palette_format_t	pform		// wanted palette format => ignored
)
{
    return ConvertToPALETTE(dest_img,src_img,0,IMG_X_PAL14);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    conv_from_*()		///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError conv_from_I4
(
    Image_t		* dest_img,	// valid destination
    const Image_t	* src_img,	// valid source
    image_format_t	iform		// new image format, only valid IMG_X_*
)
{
    DASSERT(dest_img);
    DASSERT(src_img);
    DASSERT( src_img->iform == IMG_I4 );
    DASSERT( iform >= IMG_X__MIN && iform <= IMG_X__MAX );

    const uint bits_per_pixel	= 4;
    const uint block_width	= 8;
    const uint block_height	= 8;

    uint h_blocks, v_blocks, img_size;
    enumError err = CalcImageBlock(src_img,
				bits_per_pixel, block_width, block_height,
				&h_blocks, &v_blocks, &img_size, false );
    if (err)
	return err;

    if ( iform == IMG_X_RGB )
    {
	// special RGB conversion

	uint data_size;
	u8 * data = AllocDataIMG(src_img,4,&data_size);
	u8 * dest1 = data;
	const u8 *src = src_img->data;

	const uint block_size = block_width * 4;
	const uint line_size  = EXPAND8(src_img->width) * 4;

	while ( v_blocks-- > 0 )
	{
	    u8 * dest2 = dest1;
	    uint hblk = h_blocks;
	    while ( hblk-- > 0 )
	    {
		u8 * dest3 = dest2;
		uint iv = block_height;
		while ( iv-- > 0 )
		{
		    uint ih = block_width/2;
		    while ( ih-- > 0 )
		    {
			const u8 val = *src++;
			const u8 rgb1 = cc48[ val >> 4 ];
			*dest3++ = rgb1;  // red
			*dest3++ = rgb1;  // green
			*dest3++ = rgb1;  // blue
			*dest3++ = 0xff;  // alpha

			const u8 rgb2 = cc48[ val & 0x0f ];
			*dest3++ = rgb2;  // red
			*dest3++ = rgb2;  // green
			*dest3++ = rgb2;  // blue
			*dest3++ = 0xff;  // alpha
		    }
		    dest3 += line_size - block_size;
		}
		dest2 += block_size;
	    }
	    dest1 += line_size * block_height;
	}

	DASSERT( src == src_img->data + img_size );
	DASSERT( dest1 <= data + data_size );
	AssignDataRGB(dest_img,src_img,data);
    }
    else
    {
	// use GRAY conversion elsewise

	uint data_size;
	u8 * data = AllocDataIMG(src_img,2,&data_size);
	u8 * dest1 = data;
	const u8 *src = src_img->data;

	const uint block_size = block_width * 2;
	const uint line_size  = EXPAND8(src_img->width) * 2;

	while ( v_blocks-- > 0 )
	{
	    u8 * dest2 = dest1;
	    uint hblk = h_blocks;
	    while ( hblk-- > 0 )
	    {
		u8 * dest3 = dest2;
		uint iv = block_height;
		while ( iv-- > 0 )
		{
		    uint ih = block_width/2;
		    while ( ih-- > 0 )
		    {
			const u8 val = *src++;
			*dest3++ = cc48[ val >> 4 ];	// gray
			*dest3++ = 0xff;		// alpha
			*dest3++ = cc48[ val & 0x0f ];	// gray
			*dest3++ = 0xff;		// alpha
		    }
		    dest3 += line_size - block_size;
		}
		dest2 += block_size;
	    }
	    dest1 += line_size * block_height;
	}

	DASSERT( src == src_img->data + img_size );
	DASSERT( dest1 <= data + data_size );
	AssignDataGRAY(dest_img,src_img,data);
    }

    dest_img->alpha_status = -1;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError conv_from_I8
(
    Image_t		* dest_img,	// valid destination
    const Image_t	* src_img,	// valid source
    image_format_t	iform		// new image format, only valid IMG_X_*
)
{
    DASSERT(dest_img);
    DASSERT(src_img);
    DASSERT( src_img->iform == IMG_I8 );
    DASSERT( iform >= IMG_X__MIN && iform <= IMG_X__MAX );

    const uint bits_per_pixel	= 8;
    const uint block_width	= 8;
    const uint block_height	= 4;

    uint h_blocks, v_blocks, img_size;
    enumError err = CalcImageBlock(src_img,
				bits_per_pixel, block_width, block_height,
				&h_blocks, &v_blocks, &img_size, false );
    if (err)
	return err;

    if ( iform == IMG_X_RGB )
    {
	// special RGB conversion

	uint data_size;
	u8 * data = AllocDataIMG(src_img,4,&data_size);
	u8 * dest1 = data;
	const u8 *src = src_img->data;

	const uint block_size = block_width * 4;
	const uint line_size  = EXPAND8(src_img->width) * 4;

	while ( v_blocks-- > 0 )
	{
	    u8 * dest2 = dest1;
	    uint hblk = h_blocks;
	    while ( hblk-- > 0 )
	    {
		u8 * dest3 = dest2;
		uint iv = block_height;
		while ( iv-- > 0 )
		{
		    uint ih = block_width;
		    while ( ih-- > 0 )
		    {
			const u8 val = *src++;
			*dest3++ = val;  // red
			*dest3++ = val;  // green
			*dest3++ = val;  // blue
			*dest3++ = 0xff; // alpha
		    }
		    dest3 += line_size - block_size;
		}
		dest2 += block_size;
	    }
	    dest1 += line_size * block_height;
	}

	DASSERT( src == src_img->data + img_size );
	DASSERT( dest1 <= data + data_size );
	AssignDataRGB(dest_img,src_img,data);
    }
    else
    {
	// use GRAY conversion elsewise

	uint data_size;
	u8 * data = AllocDataIMG(src_img,2,&data_size);
	u8 * dest1 = data;
	const u8 *src = src_img->data;

	const uint block_size = block_width * 2;
	const uint line_size  = EXPAND8(src_img->width) * 2;

	while ( v_blocks-- > 0 )
	{
	    u8 * dest2 = dest1;
	    uint hblk = h_blocks;
	    while ( hblk-- > 0 )
	    {
		u8 * dest3 = dest2;
		uint iv = block_height;
		while ( iv-- > 0 )
		{
		    uint ih = block_width;
		    while ( ih-- > 0 )
		    {
			*dest3++ = *src++;	// gray
			*dest3++ = 0xff;	// alpha
		    }
		    dest3 += line_size - block_size;
		}
		dest2 += block_size;
	    }
	    dest1 += line_size * block_height;
	}

	DASSERT( src == src_img->data + img_size );
	DASSERT( dest1 <= data + data_size );
	AssignDataGRAY(dest_img,src_img,data);
    }

    dest_img->alpha_status = -1;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError conv_from_IA4
(
    Image_t		* dest_img,	// valid destination
    const Image_t	* src_img,	// valid source
    image_format_t	iform		// new image format, only valid IMG_X_*
)
{
    DASSERT(dest_img);
    DASSERT(src_img);
    DASSERT( src_img->iform == IMG_IA4 );
    DASSERT( iform >= IMG_X__MIN && iform <= IMG_X__MAX );

    const uint bits_per_pixel	= 8;
    const uint block_width	= 8;
    const uint block_height	= 4;

    uint h_blocks, v_blocks, img_size;
    enumError err = CalcImageBlock(src_img,
				bits_per_pixel, block_width, block_height,
				&h_blocks, &v_blocks, &img_size, false );
    if (err)
	return err;

    if ( iform == IMG_X_RGB )
    {
	// special RGB conversion

	uint data_size;
	u8 * data = AllocDataIMG(src_img,4,&data_size);
	u8 * dest1 = data;
	const u8 *src = src_img->data;

	const uint block_size = block_width * 4;
	const uint line_size  = EXPAND8(src_img->width) * 4;

	while ( v_blocks-- > 0 )
	{
	    u8 * dest2 = dest1;
	    uint hblk = h_blocks;
	    while ( hblk-- > 0 )
	    {
		u8 * dest3 = dest2;
		uint iv = block_height;
		while ( iv-- > 0 )
		{
		    uint ih = block_width;
		    while ( ih-- > 0 )
		    {
			const u8 val = *src++;
			const u8 rgb = cc48[ val & 0x0f ];
			*dest3++ = rgb;			// red
			*dest3++ = rgb;			// green
			*dest3++ = rgb;			// blue
			*dest3++ = cc48[ val >> 4 ];	// alpha
		    }
		    dest3 += line_size - block_size;
		}
		dest2 += block_size;
	    }
	    dest1 += line_size * block_height;
	}

	DASSERT( src == src_img->data + img_size );
	DASSERT( dest1 <= data + data_size );
	AssignDataRGB(dest_img,src_img,data);
    }
    else
    {
	// use GRAY conversion elsewise

	uint data_size;
	u8 * data = AllocDataIMG(src_img,2,&data_size);
	u8 * dest1 = data;
	const u8 *src = src_img->data;

	const uint block_size = block_width * 2;
	const uint line_size  = EXPAND8(src_img->width) * 2;

	while ( v_blocks-- > 0 )
	{
	    u8 * dest2 = dest1;
	    uint hblk = h_blocks;
	    while ( hblk-- > 0 )
	    {
		u8 * dest3 = dest2;
		uint iv = block_height;
		while ( iv-- > 0 )
		{
		    uint ih = block_width;
		    while ( ih-- > 0 )
		    {
			const u8 val = *src++;
			*dest3++ = cc48[ val & 0x0f ];	// gray
			*dest3++ = cc48[ val >> 4 ];	// alpha
		    }
		    dest3 += line_size - block_size;
		}
		dest2 += block_size;
	    }
	    dest1 += line_size * block_height;
	}

	DASSERT( src == src_img->data + img_size );
	DASSERT( dest1 <= data + data_size );
	AssignDataGRAY(dest_img,src_img,data);
    }

    dest_img->alpha_status = 0;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError conv_from_IA8
(
    Image_t		* dest_img,	// valid destination
    const Image_t	* src_img,	// valid source
    image_format_t	iform		// new image format, only valid IMG_X_*
)
{
    DASSERT(dest_img);
    DASSERT(src_img);
    DASSERT( src_img->iform == IMG_IA8 );
    DASSERT( iform >= IMG_X__MIN && iform <= IMG_X__MAX );

    const uint bits_per_pixel	= 16;
    const uint block_width	=  4;
    const uint block_height	=  4;

    uint h_blocks, v_blocks, img_size;
    enumError err = CalcImageBlock(src_img,
				bits_per_pixel, block_width, block_height,
				&h_blocks, &v_blocks, &img_size, false );
    if (err)
	return err;

    u16 (*rd16) ( const void * data_ptr ) = src_img->endian->rd16;
    noPRINT("from_IA8: wh=%u*%u, blk=%u,%u -> nb=%u,%u\n",
		src_img->width, src_img->height,
		block_width, block_height,
		h_blocks, v_blocks );

    if ( iform == IMG_X_RGB )
    {
	// special RGB conversion

	uint data_size;
	u8 * data = AllocDataIMG(src_img,4,&data_size);
	u8 * dest1 = data;
	const u8 *src = src_img->data;

	const uint block_size = block_width * 4;
	const uint line_size  = EXPAND8(src_img->width) * 4;

	while ( v_blocks-- > 0 )
	{
	    u8 * dest2 = dest1;
	    uint hblk = h_blocks;
	    while ( hblk-- > 0 )
	    {
		u8 * dest3 = dest2;
		uint iv = block_height;
		while ( iv-- > 0 )
		{
		    uint ih = block_width;
		    while ( ih-- > 0 )
		    {
			const u16 val = rd16(src);
			src += 2;
			*dest3++ = val;		// red
			*dest3++ = val;		// green
			*dest3++ = val;		// blue
			*dest3++ = val >> 8;	// alpha
		    }
		    dest3 += line_size - block_size;
		}
		dest2 += block_size;
	    }
	    dest1 += line_size * block_height;
	}

	DASSERT( src == src_img->data + img_size );
	DASSERT( dest1 <= data + data_size );
	AssignDataRGB(dest_img,src_img,data);
    }
    else
    {
	// use GRAY conversion elsewise

	uint data_size;
	u8 * data = AllocDataIMG(src_img,2,&data_size);
	u8 * dest1 = data;
	const u8 *src = src_img->data;

	const uint block_size = block_width * 2;
	const uint line_size  = EXPAND8(src_img->width) * 2;

	while ( v_blocks-- > 0 )
	{
	    u8 * dest2 = dest1;
	    uint hblk = h_blocks;
	    while ( hblk-- > 0 )
	    {
		u8 * dest3 = dest2;
		uint iv = block_height;
		while ( iv-- > 0 )
		{
		    uint ih = block_width;
		    while ( ih-- > 0 )
		    {
			const u16 val = rd16(src);
			src += 2;
			*dest3++ = val;		// gray
			*dest3++ = val >> 8;	// alpha
		    }
		    dest3 += line_size - block_size;
		}
		dest2 += block_size;
	    }
	    dest1 += line_size * block_height;
	}

	DASSERT( src == src_img->data + img_size );
	DASSERT( dest1 <= data + data_size );
	AssignDataGRAY(dest_img,src_img,data);
    }

    dest_img->alpha_status = 0;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError conv_from_RGB565
(
    Image_t		* dest_img,	// valid destination
    const Image_t	* src_img,	// valid source
    image_format_t	iform		// new image format, only valid IMG_X_*
)
{
    DASSERT(dest_img);
    DASSERT(src_img);
    DASSERT( src_img->iform == IMG_RGB565 );
    DASSERT( iform >= IMG_X__MIN && iform <= IMG_X__MAX );

    const uint bits_per_pixel	= 16;
    const uint block_width	=  4;
    const uint block_height	=  4;

    uint h_blocks, v_blocks, img_size;
    enumError err = CalcImageBlock(src_img,
				bits_per_pixel, block_width, block_height,
				&h_blocks, &v_blocks, &img_size, false );
    if (err)
	return err;

    u16 (*rd16) ( const void * data_ptr ) = src_img->endian->rd16;

    uint data_size;
    u8 * data = AllocDataIMG(src_img,4,&data_size);
    u8 * dest1 = data;
    const u8 *src = src_img->data;

    const uint block_size = block_width * 4;
    const uint line_size  = EXPAND8(src_img->width) * 4;

    while ( v_blocks-- > 0 )
    {
	u8 * dest2 = dest1;
	uint hblk = h_blocks;
	while ( hblk-- > 0 )
	{
	    u8 * dest3 = dest2;
	    uint iv = block_height;
	    while ( iv-- > 0 )
	    {
		uint ih = block_width;
		while ( ih-- > 0 )
		{
		    const u16 val = rd16(src);
		    src += 2;
		    *dest3++ = cc58[ val >> 11 ];	 // red
		    *dest3++ = cc68[ val >>  5 & 0x3f ]; // green
		    *dest3++ = cc58[ val       & 0x1f ]; // blue
		    *dest3++ = 0xff;			 // alpha
		}
		dest3 += line_size - block_size;
	    }
	    dest2 += block_size;
	}
	dest1 += line_size * block_height;
    }

    DASSERT( src == src_img->data + img_size );
    DASSERT( dest1 <= data + data_size );
    AssignDataRGB(dest_img,src_img,data);
    dest_img->alpha_status = -1;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError conv_from_RGB5A3
(
    Image_t		* dest_img,	// valid destination
    const Image_t	* src_img,	// valid source
    image_format_t	iform		// new image format, only valid IMG_X_*
)
{
    DASSERT(dest_img);
    DASSERT(src_img);
    DASSERT( src_img->iform == IMG_RGB5A3 );
    DASSERT( iform >= IMG_X__MIN && iform <= IMG_X__MAX );

    const uint bits_per_pixel	= 16;
    const uint block_width	=  4;
    const uint block_height	=  4;

    uint h_blocks, v_blocks, img_size;
    enumError err = CalcImageBlock(src_img,
				bits_per_pixel, block_width, block_height,
				&h_blocks, &v_blocks, &img_size, false );
    if (err)
	return err;

    u16 (*rd16) ( const void * data_ptr ) = src_img->endian->rd16;

    uint data_size;
    u8 * data = AllocDataIMG(src_img,4,&data_size);
    u8 * dest1 = data;
    const u8 *src = src_img->data;

    const uint block_size = block_width * 4;
    const uint line_size  = EXPAND8(src_img->width) * 4;

    while ( v_blocks-- > 0 )
    {
	u8 * dest2 = dest1;
	uint hblk = h_blocks;
	while ( hblk-- > 0 )
	{
	    u8 * dest3 = dest2;
	    uint iv = block_height;
	    while ( iv-- > 0 )
	    {
		uint ih = block_width;
		while ( ih-- > 0 )
		{
		    const u16 val = rd16(src);
		    src += 2;
		    if ( val & 0x8000 )
		    {
			*dest3++ = cc58[ val >> 10 & 0x1f ];	// red
			*dest3++ = cc58[ val >>  5 & 0x1f ];	// green
			*dest3++ = cc58[ val       & 0x1f ];	// blue
			*dest3++ = 0xff;			// alpha
		    }
		    else
		    {
			*dest3++ = cc48[ val >>  8 & 0x0f ];	// red
			*dest3++ = cc48[ val >>  4 & 0x0f ];	// green
			*dest3++ = cc48[ val       & 0x0f ];	// blue
			*dest3++ = cc38[ val >> 12 & 0x07 ];	// alpha
		    }
		}
		dest3 += line_size - block_size;
	    }
	    dest2 += block_size;
	}
	dest1 += line_size * block_height;
    }

    DASSERT( src == src_img->data + img_size );
    DASSERT( dest1 <= data + data_size );
    AssignDataRGB(dest_img,src_img,data);
    dest_img->alpha_status = 0;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError conv_from_RGBA32
(
    Image_t		* dest_img,	// valid destination
    const Image_t	* src_img,	// valid source
    image_format_t	iform		// new image format, only valid IMG_X_*
)
{
    DASSERT(dest_img);
    DASSERT(src_img);
    DASSERT( src_img->iform == IMG_RGBA32 );
    DASSERT( iform >= IMG_X__MIN && iform <= IMG_X__MAX );


    const uint bits_per_pixel	= 32;
    const uint block_width	=  4;
    const uint block_height	=  4;

    uint h_blocks, v_blocks, img_size;
    enumError err = CalcImageBlock(src_img,
				bits_per_pixel, block_width, block_height,
				&h_blocks, &v_blocks, &img_size, false );
    if (err)
	return err;

    u16 (*rd16) ( const void * data_ptr ) = src_img->endian->rd16;

    uint data_size;
    u8 * data = AllocDataIMG(src_img,4,&data_size);
    u8 * dest1 = data;
    const u8 *src = src_img->data;

    const uint block_size = block_width * 4;
    const uint line_size  = EXPAND8(src_img->width) * 4;

    while ( v_blocks-- > 0 )
    {
	u8 * dest2 = dest1;
	uint hblk = h_blocks;
	while ( hblk-- > 0 )
	{
	    u8 * dest3 = dest2;
	    uint iv = block_height;
	    while ( iv-- > 0 )
	    {
		uint ih = block_width;
		while ( ih-- > 0 )
		{
		    const u16 ar = rd16(src);
		    const u16 gb = rd16(src+0x20);
		    src += 2;
		    *dest3++ = ar;	// red
		    *dest3++ = gb >> 8;	// green
		    *dest3++ = gb;	// blue
		    *dest3++ = ar >> 8;	// alpha
		}
		dest3 += line_size - block_size;
	    }
	    src += block_height * block_width * 2;
	    dest2 += block_size;
	}
	dest1 += line_size * block_height;
    }

    DASSERT( src == src_img->data + img_size );
    DASSERT( dest1 <= data + data_size );
    AssignDataRGB(dest_img,src_img,data);
    dest_img->alpha_status = 0;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError conv_from_C4
(
    Image_t		* dest_img,	// valid destination
    const Image_t	* src_img,	// valid source
    image_format_t	iform		// new image format, only valid IMG_X_*
)
{
    DASSERT(dest_img);
    DASSERT(src_img);
    DASSERT( src_img->iform == IMG_C4 );
    DASSERT( iform >= IMG_X__MIN && iform <= IMG_X__MAX );

    const uint bits_per_pixel	=  4;
    const uint block_width	=  8;
    const uint block_height	=  8;

    uint h_blocks, v_blocks, img_size;
    enumError err = CalcImageBlock(src_img,
				bits_per_pixel, block_width, block_height,
				&h_blocks, &v_blocks, &img_size, false );
    if (err)
	return err;

    // here we transform the data to a linear 16-bit palette-index table

    const uint xwidth = EXPAND8(src_img->width);
    const uint xheight = EXPAND8(src_img->height);
    u16 * itab = CALLOC( xwidth * xheight, 2 );
    u16 * dest1 = itab;
    const u8 *src = src_img->data;

    while ( v_blocks-- > 0 )
    {
	u16 * dest2 = dest1;
	uint hblk = h_blocks;
	while ( hblk-- > 0 )
	{
	    u16 * dest3 = dest2;
	    uint iv = block_height;
	    while ( iv-- > 0 )
	    {
		uint ih = block_width/2;
		while ( ih-- > 0 )
		{
		    const u8 val = *src++;
		    *dest3++ = val >> 4;
		    *dest3++ = val & 0x0f;
		}
		dest3 += xwidth - block_width;
	    }
	    dest2 += block_width;
	}
	dest1 += xwidth * block_height;
    }
    noPRINT("src=0x%zx/0x%x = %zu/%u [%u*%u]\n",
		src - src_img->data, img_size,
		src - src_img->data, img_size,
		src_img->width, src_img->height );
    DASSERT( src == src_img->data + img_size );
    DASSERT( dest1 <= itab + xwidth * xheight * 2 );

    //--- now do the palette transformation

    return TransformPalette(dest_img,src_img,IMG_X_PAL4,itab);
}

///////////////////////////////////////////////////////////////////////////////

static enumError conv_from_C8
(
    Image_t		* dest_img,	// valid destination
    const Image_t	* src_img,	// valid source
    image_format_t	iform		// new image format, only valid IMG_X_*
)
{
    DASSERT(dest_img);
    DASSERT(src_img);
    DASSERT( src_img->iform == IMG_C8 );
    DASSERT( iform >= IMG_X__MIN && iform <= IMG_X__MAX );

    const uint bits_per_pixel	=  8;
    const uint block_width	=  8;
    const uint block_height	=  4;

    uint h_blocks, v_blocks, img_size;
    enumError err = CalcImageBlock(src_img,
				bits_per_pixel, block_width, block_height,
				&h_blocks, &v_blocks, &img_size, false );
    if (err)
	return err;

    // here we transform the data to a linear 16-bit palette-index table

    const uint xwidth = EXPAND8(src_img->width);
    const uint xheight = EXPAND8(src_img->height);
    u16 * itab = CALLOC( xwidth * xheight, 2 );
    u16 * dest1 = itab;
    const u8 *src = src_img->data;

    while ( v_blocks-- > 0 )
    {
	u16 * dest2 = dest1;
	uint hblk = h_blocks;
	while ( hblk-- > 0 )
	{
	    u16 * dest3 = dest2;
	    uint iv = block_height;
	    while ( iv-- > 0 )
	    {
		uint ih = block_width;
		while ( ih-- > 0 )
		    *dest3++ = *src++;
		dest3 += xwidth - block_width;
	    }
	    dest2 += block_width;
	}
	dest1 += xwidth * block_height;
    }
    noPRINT("src=0x%zx/0x%x = %zu/%u [%u*%u]\n",
		src - src_img->data, img_size,
		src - src_img->data, img_size,
		src_img->width, src_img->height );
    DASSERT( src == src_img->data + img_size );
    DASSERT( dest1 <= itab + xwidth * xheight * 2 );

    //--- now do the palette transformation

    return TransformPalette(dest_img,src_img,IMG_X_PAL8,itab);
}

///////////////////////////////////////////////////////////////////////////////

static enumError conv_from_C14X2
(
    Image_t		* dest_img,	// valid destination
    const Image_t	* src_img,	// valid source
    image_format_t	iform		// new image format, only valid IMG_X_*
)
{
    DASSERT(dest_img);
    DASSERT(src_img);
    DASSERT( src_img->iform == IMG_C14X2 );
    DASSERT( iform >= IMG_X__MIN && iform <= IMG_X__MAX );

    const uint bits_per_pixel	= 16;
    const uint block_width	=  4;
    const uint block_height	=  4;

    uint h_blocks, v_blocks, img_size;
    enumError err = CalcImageBlock(src_img,
				bits_per_pixel, block_width, block_height,
				&h_blocks, &v_blocks, &img_size, false );
    if (err)
	return err;

    // here we transform the data to a linear 16-bit palette-index table

    u16 (*rd16) ( const void * data_ptr ) = src_img->endian->rd16;

    const uint xwidth = EXPAND8(src_img->width);
    const uint xheight = EXPAND8(src_img->height);
    u16 * itab = CALLOC( xwidth * xheight, 2 );
    u16 * dest1 = itab;
    const u8 *src = src_img->data;

    while ( v_blocks-- > 0 )
    {
	u16 * dest2 = dest1;
	uint hblk = h_blocks;
	while ( hblk-- > 0 )
	{
	    u16 * dest3 = dest2;
	    uint iv = block_height;
	    while ( iv-- > 0 )
	    {
		uint ih = block_width;
		while ( ih-- > 0 )
		{
		    *dest3++ = rd16(src) & 0x3fff;
		    src += 2;
		}
		dest3 += xwidth - block_width;
	    }
	    dest2 += block_width;
	}
	dest1 += xwidth * block_height;
    }
    noPRINT("src=0x%zx/0x%x = %zu/%u [%u*%u]\n",
		src - src_img->data, img_size,
		src - src_img->data, img_size,
		src_img->width, src_img->height );
    DASSERT( src == src_img->data + img_size );
    DASSERT( dest1 <= itab + xwidth * xheight * 2 );

    //--- now do the palette transformation

    return TransformPalette(dest_img,src_img,IMG_X_PAL14,itab);
}

///////////////////////////////////////////////////////////////////////////////

static enumError conv_from_CMPR
(
    Image_t		* dest_img,	// valid destination
    const Image_t	* src_img,	// valid source
    image_format_t	iform		// new image format, only valid IMG_X_*
)
{
    DASSERT(dest_img);
    DASSERT(src_img);
    DASSERT( src_img->iform == IMG_CMPR );
    DASSERT( iform >= IMG_X__MIN && iform <= IMG_X__MAX );

    const uint bits_per_pixel	= 4;
    const uint block_width	= 8;
    const uint block_height	= 8;

    uint h_blocks, v_blocks, img_size;
    enumError err = CalcImageBlock(src_img,
				bits_per_pixel, block_width, block_height,
				&h_blocks, &v_blocks, &img_size, false );
    if (err)
	return err;

    u16 (*rd16) ( const void * data_ptr ) = src_img->endian->rd16;

    uint data_size;
    u8 * data = AllocDataIMG(src_img,4,&data_size);
    u8 * dest1 = data;
    const u8 *src = src_img->data;

    const uint block_size = block_width * 4;
    const uint line_size  = EXPAND8(src_img->width) * 4;
    const uint delta[] = { 0, 16, 4*line_size, 4*line_size+16 };

    while ( v_blocks-- > 0 )
    {
	u8 * dest2 = dest1;
	uint hblk = h_blocks;
	while ( hblk-- > 0 )
	{
	    uint subb;
	    for ( subb = 0; subb < 4; subb++ )
	    {
		u8 palette[4][4], *pal = palette[0];

		const u16 val1 = rd16(src);
		src += 2;
		*pal++ = cc58[ val1 >> 11 & 0x1f ];
		*pal++ = cc68[ val1 >>  5 & 0x3f ];
		*pal++ = cc58[ val1       & 0x1f ];
		*pal++ = 0xff;

		const u16 val2 = rd16(src);
		src += 2;
		*pal++ = cc58[ val2 >> 11 & 0x1f ];
		*pal++ = cc68[ val2 >>  5 & 0x3f ];
		*pal++ = cc58[ val2       & 0x1f ];
		*pal++ = 0xff;

		if ( val1 > val2 )
		{
		    *pal++ = ( 2 * palette[0][0] + palette[1][0] ) / 3;
		    *pal++ = ( 2 * palette[0][1] + palette[1][1] ) / 3;
		    *pal++ = ( 2 * palette[0][2] + palette[1][2] ) / 3;
		    *pal++ = 0xff;

		    *pal++ = ( 2 * palette[1][0] + palette[0][0] ) / 3;
		    *pal++ = ( 2 * palette[1][1] + palette[0][1] ) / 3;
		    *pal++ = ( 2 * palette[1][2] + palette[0][2] ) / 3;
		    *pal++ = 0xff;
		}
		else
		{
		    *pal++ = ( palette[0][0] + palette[1][0] ) / 2;
		    *pal++ = ( palette[0][1] + palette[1][1] ) / 2;
		    *pal++ = ( palette[0][2] + palette[1][2] ) / 2;
		    *pal++ = 0xff;

		    *pal++ = 0;
		    *pal++ = 0;
		    *pal++ = 0;
		    *pal++ = 0;
		}

		u8 *dest3 = dest2 + delta[subb];
		uint i;
		for ( i = 0; i < 4; i++ )
		{
		    u8 val = *src++;
		    memcpy(dest3+12, palette[ val & 3 ], 4); val >>= 2;
		    memcpy(dest3+ 8, palette[ val & 3 ], 4); val >>= 2;
		    memcpy(dest3+ 4, palette[ val & 3 ], 4); val >>= 2;
		    memcpy(dest3   , palette[ val & 3 ], 4);
		    dest3 += line_size;
		}
	    }
	    dest2 += block_size;
	}
	dest1 += line_size * block_height;
    }

    DASSERT( src == src_img->data + img_size );
    DASSERT( dest1 <= data + data_size );
    AssignDataRGB(dest_img,src_img,data);
    dest_img->alpha_status = 0;
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    conv_to_*()			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError conv_to_I4
(
    Image_t		* dest_img,	// valid destination
    const Image_t	* src_img,	// valid source
    palette_format_t	pform		// wanted palette format => ignored
)
{
    DASSERT(dest_img);
    DASSERT(src_img);
    DASSERT( src_img->iform >= IMG_X__MIN && src_img->iform <= IMG_X__MAX );


    //--- first convert to IMG_X_GRAY

    if ( src_img->iform != IMG_X_GRAY )
    {
	enumError err = ConvertIMG(dest_img,false,src_img,IMG_X_GRAY,PAL_INVALID);
	if (err)
	    return err;
	src_img = dest_img;
    }
    DASSERT( src_img->iform == IMG_X_GRAY );


    //--- and now convert IMG_X_GRAY -> I4

    const uint bits_per_pixel	= 4;
    const uint block_width	= 8;
    const uint block_height	= 8;

    uint h_blocks, v_blocks, img_size;
    CalcImageBlock( src_img, bits_per_pixel, block_width, block_height,
			&h_blocks, &v_blocks, &img_size, true );


    // use GRAY conversion elsewise

    u8 *data = CALLOC(1,img_size);
    u8 *dest = data;
    const u8 *src1 = src_img->data;

    const uint block_size = block_width * 2;
    DASSERT( EXPAND8(src_img->width) == src_img->xwidth );
    const uint line_size = src_img->xwidth * 2;

    while ( v_blocks-- > 0 )
    {
	const u8 *src2 = src1;
	uint hblk = h_blocks;
	while ( hblk-- > 0 )
	{
	    const u8 *src3 = src2;
	    uint iv = block_height;
	    while ( iv-- > 0 )
	    {
		uint ih = block_width/2;
		while ( ih-- > 0 )
		{
		    *dest++ = cc84[ src3[0] ] << 4 | cc84[ src3[2] ];
		    src3 += 4;
		}
		src3 += line_size - block_size;
	    }
	    src2 += block_size;
	}
	src1 += line_size * block_height;
    }

    DASSERT( dest == data + img_size );
    DASSERT( src1 <= src_img->data + src_img->data_size );

    AssignData(dest_img,src_img,data,img_size,IMG_I4);
    dest_img->is_grayed = true;
    dest_img->alpha_status = -1;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError conv_to_I8
(
    Image_t		* dest_img,	// valid destination
    const Image_t	* src_img,	// valid source
    palette_format_t	pform		// wanted palette format => ignored
)
{
    DASSERT(dest_img);
    DASSERT(src_img);
    DASSERT( src_img->iform >= IMG_X__MIN && src_img->iform <= IMG_X__MAX );


    //--- first convert to IMG_X_GRAY

    if ( src_img->iform != IMG_X_GRAY )
    {
	enumError err = ConvertIMG(dest_img,false,src_img,IMG_X_GRAY,PAL_INVALID);
	if (err)
	    return err;
	src_img = dest_img;
    }
    DASSERT( src_img->iform == IMG_X_GRAY );


    //--- and now convert IMG_X_GRAY -> I8

    const uint bits_per_pixel	= 8;
    const uint block_width	= 8;
    const uint block_height	= 4;

    uint h_blocks, v_blocks, img_size;
    CalcImageBlock( src_img, bits_per_pixel, block_width, block_height,
			&h_blocks, &v_blocks, &img_size, true );

    u8 *data = CALLOC(1,img_size);
    u8 *dest = data;
    const u8 *src1 = src_img->data;

    const uint block_size = block_width * 2;
    DASSERT( EXPAND8(src_img->width) == src_img->xwidth );
    const uint line_size = src_img->xwidth * 2;

    while ( v_blocks-- > 0 )
    {
	const u8 *src2 = src1;
	uint hblk = h_blocks;
	while ( hblk-- > 0 )
	{
	    const u8 *src3 = src2;
	    uint iv = block_height;
	    while ( iv-- > 0 )
	    {
		uint ih = block_width;
		while ( ih-- > 0 )
		{
		    *dest++ = *src3;	// gray
		    src3 += 2;		// skip alpha
		}
		src3 += line_size - block_size;
	    }
	    src2 += block_size;
	}
	src1 += line_size * block_height;
    }

    DASSERT( dest == data + img_size );
    DASSERT( src1 <= src_img->data + src_img->data_size );

    AssignData(dest_img,src_img,data,img_size,IMG_I8);
    dest_img->is_grayed = true;
    dest_img->alpha_status = -1;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError conv_to_IA4
(
    Image_t		* dest_img,	// valid destination
    const Image_t	* src_img,	// valid source
    palette_format_t	pform		// wanted palette format => ignored
)
{
    DASSERT(dest_img);
    DASSERT(src_img);
    DASSERT( src_img->iform >= IMG_X__MIN && src_img->iform <= IMG_X__MAX );


    //--- first convert to IMG_X_GRAY

    if ( src_img->iform != IMG_X_GRAY )
    {
	enumError err = ConvertIMG(dest_img,false,src_img,IMG_X_GRAY,PAL_INVALID);
	if (err)
	    return err;
	src_img = dest_img;
    }
    DASSERT( src_img->iform == IMG_X_GRAY );


    //--- and now convert IMG_X_GRAY -> IA4

    const uint bits_per_pixel	= 8;
    const uint block_width	= 8;
    const uint block_height	= 4;

    uint h_blocks, v_blocks, img_size;
    CalcImageBlock(src_img, bits_per_pixel, block_width, block_height,
			&h_blocks, &v_blocks, &img_size, true );

    u8 *data = CALLOC(1,img_size);
    u8 *dest = data;
    const u8 *src1 = src_img->data;

    const uint block_size = block_width * 2;
    DASSERT( EXPAND8(src_img->width) == src_img->xwidth );
    const uint line_size = src_img->xwidth * 2;

    while ( v_blocks-- > 0 )
    {
	const u8 *src2 = src1;
	uint hblk = h_blocks;
	while ( hblk-- > 0 )
	{
	    const u8 *src3 = src2;
	    uint iv = block_height;
	    while ( iv-- > 0 )
	    {
		uint ih = block_width;
		while ( ih-- > 0 )
		{
		    *dest++ = cc84[ src3[1] ] << 4 | cc84[ src3[0] ];
		    src3 += 2;
		}
		src3 += line_size - block_size;
	    }
	    src2 += block_size;
	}
	src1 += line_size * block_height;
    }

    DASSERT( dest == data + img_size );
    DASSERT( src1 <= src_img->data + src_img->data_size );

    AssignData(dest_img,src_img,data,img_size,IMG_IA4);
    dest_img->is_grayed = true;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError conv_to_IA8
(
    Image_t		* dest_img,	// valid destination
    const Image_t	* src_img,	// valid source
    palette_format_t	pform		// wanted palette format => ignored
)
{
    DASSERT(dest_img);
    DASSERT(src_img);
    DASSERT( src_img->iform >= IMG_X__MIN && src_img->iform <= IMG_X__MAX );


    //--- first convert to IMG_X_GRAY

    if ( src_img->iform != IMG_X_GRAY )
    {
	enumError err = ConvertIMG(dest_img,false,src_img,IMG_X_GRAY,PAL_INVALID);
	if (err)
	    return err;
	src_img = dest_img;
    }
    DASSERT( src_img->iform == IMG_X_GRAY );


    //--- and now convert IMG_X_GRAY -> IA8

    const uint bits_per_pixel	= 16;
    const uint block_width	=  4;
    const uint block_height	=  4;

    uint h_blocks, v_blocks, img_size;
    CalcImageBlock( src_img, bits_per_pixel, block_width, block_height,
			&h_blocks, &v_blocks, &img_size, true );

    void (*wr16) ( void*, u16 ) = src_img->endian->wr16;

    u8 *data = CALLOC(1,img_size);
    u8 *dest = data;
    const u8 *src1 = src_img->data;

    const uint block_size = block_width * 2;
    DASSERT( EXPAND8(src_img->width) == src_img->xwidth );
    const uint line_size = src_img->xwidth * 2;

    while ( v_blocks-- > 0 )
    {
	const u8 *src2 = src1;
	uint hblk = h_blocks;
	while ( hblk-- > 0 )
	{
	    const u8 *src3 = src2;
	    uint iv = block_height;
	    while ( iv-- > 0 )
	    {
		uint ih = block_width;
		while ( ih-- > 0 )
		{
		    wr16( dest, src3[0] | (u16)src3[1] << 8 );
		    dest += 2;
		    src3 += 2;
		}
		src3 += line_size - block_size;
	    }
	    src2 += block_size;
	}
	src1 += line_size * block_height;
    }

    DASSERT( dest == data + img_size );
    DASSERT( src1 <= src_img->data + src_img->data_size );

    AssignData(dest_img,src_img,data,img_size,IMG_IA8);
    dest_img->is_grayed = true;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError conv_to_RGB565
(
    Image_t		* dest_img,	// valid destination
    const Image_t	* src_img,	// valid source
    palette_format_t	pform		// wanted palette format => ignored
)
{
    DASSERT(dest_img);
    DASSERT(src_img);
    DASSERT( src_img->iform >= IMG_X__MIN && src_img->iform <= IMG_X__MAX );


    //--- first convert to IMG_X_RGB

    if ( src_img->iform != IMG_X_RGB )
    {
	enumError err = ConvertIMG(dest_img,false,src_img,IMG_X_RGB,PAL_INVALID);
	if (err)
	    return err;
	src_img = dest_img;
    }
    DASSERT( src_img->iform == IMG_X_RGB );


    //--- and now convert IMG_X_RGB -> RGB565

    const uint bits_per_pixel	= 16;
    const uint block_width	=  4;
    const uint block_height	=  4;

    uint h_blocks, v_blocks, img_size;
    CalcImageBlock( src_img, bits_per_pixel, block_width, block_height,
			&h_blocks, &v_blocks, &img_size, true );

    void (*wr16) ( void*, u16 ) = src_img->endian->wr16;

    u8 *data = CALLOC(1,img_size);
    u8 *dest = data;
    const u8 *src1 = src_img->data;

    const uint block_size = block_width * 4;
    DASSERT( EXPAND8(src_img->width) == src_img->xwidth );
    const uint line_size = src_img->xwidth * 4;

    while ( v_blocks-- > 0 )
    {
	const u8 *src2 = src1;
	uint hblk = h_blocks;
	while ( hblk-- > 0 )
	{
	    const u8 *src3 = src2;
	    uint iv = block_height;
	    while ( iv-- > 0 )
	    {
		uint ih = block_width;
		while ( ih-- > 0 )
		{
		    wr16( dest,	  cc85[ src3[0] ] << 11
				| cc86[ src3[1] ] <<  5
				| cc85[ src3[2] ]       );
		    dest += 2;
		    src3 += 4;
		}
		src3 += line_size - block_size;
	    }
	    src2 += block_size;
	}
	src1 += line_size * block_height;
    }

    DASSERT( dest == data + img_size );
    DASSERT( src1 <= src_img->data + src_img->data_size );

    AssignData(dest_img,src_img,data,img_size,IMG_RGB565);
    dest_img->alpha_status = -1;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError conv_to_RGB5A3
(
    Image_t		* dest_img,	// valid destination
    const Image_t	* src_img,	// valid source
    palette_format_t	pform		// wanted palette format => ignored
)
{
    DASSERT(dest_img);
    DASSERT(src_img);
    DASSERT( src_img->iform >= IMG_X__MIN && src_img->iform <= IMG_X__MAX );


    //--- first convert to IMG_X_RGB

    if ( src_img->iform != IMG_X_RGB )
    {
	enumError err = ConvertIMG(dest_img,false,src_img,IMG_X_RGB,PAL_INVALID);
	if (err)
	    return err;
	src_img = dest_img;
    }
    DASSERT( src_img->iform == IMG_X_RGB );


    //--- and now convert IMG_X_RGB -> RGB5A3

    const uint bits_per_pixel	= 16;
    const uint block_width	=  4;
    const uint block_height	=  4;

    uint h_blocks, v_blocks, img_size;
    CalcImageBlock( src_img, bits_per_pixel, block_width, block_height,
			&h_blocks, &v_blocks, &img_size, true );

    void (*wr16) ( void*, u16 ) = src_img->endian->wr16;

    u8 *data = CALLOC(1,img_size);
    u8 *dest = data;
    const u8 *src1 = src_img->data;

    const uint block_size = block_width * 4;
    DASSERT( EXPAND8(src_img->width) == src_img->xwidth );
    const uint line_size = src_img->xwidth * 4;

    while ( v_blocks-- > 0 )
    {
	const u8 *src2 = src1;
	uint hblk = h_blocks;
	while ( hblk-- > 0 )
	{
	    const u8 *src3 = src2;
	    uint iv = block_height;
	    while ( iv-- > 0 )
	    {
		uint ih = block_width;
		while ( ih-- > 0 )
		{
		    if ( src3[3] == 0xff )
			wr16( dest,  cc85[ src3[0] ] << 10
				   | cc85[ src3[1] ] <<  5
				   | cc85[ src3[2] ]
				   | 0x8000 );
		    else
			wr16( dest,  cc84[ src3[0] ] <<  8
				   | cc84[ src3[1] ] <<  4
				   | cc84[ src3[2] ]
				   | cc83[ src3[3] ] << 12 );
		    dest += 2;
		    src3 += 4;
		}
		src3 += line_size - block_size;
	    }
	    src2 += block_size;
	}
	src1 += line_size * block_height;
    }

    DASSERT( dest == data + img_size );
    DASSERT( src1 <= src_img->data + src_img->data_size );

    AssignData(dest_img,src_img,data,img_size,IMG_RGB5A3);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError conv_to_RGBA32
(
    Image_t		* dest_img,	// valid destination
    const Image_t	* src_img,	// valid source
    palette_format_t	pform		// wanted palette format => ignored
)
{
    DASSERT(dest_img);
    DASSERT(src_img);
    DASSERT( src_img->iform >= IMG_X__MIN && src_img->iform <= IMG_X__MAX );


    //--- first convert to IMG_X_RGB

    if ( src_img->iform != IMG_X_RGB )
    {
	enumError err = ConvertIMG(dest_img,false,src_img,IMG_X_RGB,PAL_INVALID);
	if (err)
	    return err;
	src_img = dest_img;
    }
    DASSERT( src_img->iform == IMG_X_RGB );


    //--- and now convert IMG_X_RGB -> RGBA32

    const uint bits_per_pixel	= 32;
    const uint block_width	=  4;
    const uint block_height	=  4;

    uint h_blocks, v_blocks, img_size;
    CalcImageBlock( src_img, bits_per_pixel, block_width, block_height,
			&h_blocks, &v_blocks, &img_size, true );

    void (*wr16) ( void*, u16 ) = src_img->endian->wr16;

    u8 *data = CALLOC(1,img_size);
    u8 *dest = data;
    const u8 *src1 = src_img->data;

    const uint block_size = block_width * 4;
    DASSERT( EXPAND8(src_img->width) == src_img->xwidth );
    const uint line_size = src_img->xwidth * 4;

    while ( v_blocks-- > 0 )
    {
	const u8 *src2 = src1;
	uint hblk = h_blocks;
	while ( hblk-- > 0 )
	{
	    const u8 *src3 = src2;
	    uint iv = block_height;
	    while ( iv-- > 0 )
	    {
		uint ih = block_width;
		while ( ih-- > 0 )
		{
		    wr16( dest,      src3[0] | (u16)src3[3] << 8 );
		    wr16( dest+0x20, src3[2] | (u16)src3[1] << 8 );
		    dest += 2;
		    src3 += 4;
		}
		src3 += line_size - block_size;
	    }
	    dest += block_height * block_width * 2;
	    src2 += block_size;
	}
	src1 += line_size * block_height;
    }

    DASSERT( dest == data + img_size );
    DASSERT( src1 <= src_img->data + src_img->data_size );

    AssignData(dest_img,src_img,data,img_size,IMG_RGBA32);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    conv_to_C*()		///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError create_C_palette
(
    Image_t		* img,		// valid destination
    const Image_t	* src_img,	// valid source
    image_format_t	iform,		// wanted image format (one of IMG_X_PAL*)
    palette_format_t	pform		// wanted palette format
)
{
    DASSERT(img);

    //--- convert to IMG_X_RGB

    CopyIMG(img,false,src_img,true);
    Transform2XRGB(img);
    enumError err = ExecTransformIMG(img);
    if (err)
	return err;
    DASSERT( img->iform == IMG_X_RGB );


    //--- find palette format

    switch (pform)
    {
      case PAL_IA8:
      case PAL_RGB565:
      case PAL_RGB5A3:
	// ok for the moment
	break;

      default:
	// calculate the best palette type
	pform = img->is_grayed
		? PAL_IA8
		: CheckAlphaIMG(img,false) < 0
			? PAL_RGB565
			: PAL_RGB5A3;
	break;
    }
    PRINT("%*s  create_C_palette() %s.%s\n",
		2*convert_depth, "",
		GetImageFormatName(iform,"?"),
		GetPaletteFormatName(pform,"?") );


    //--- reduce color depth or normalize image

    u8 *ptr = img->data;
    u8 *end = ptr + img->data_size;

    switch (pform)
    {
      case PAL_IA8:
	while ( ptr < end )
	{
	    const u8 gray = ( ptr[0] + ptr[1] + ptr[2] + 1 ) / 3;
	    *ptr++ = gray;
	    *ptr++ = gray;
	    *ptr++ = gray;
	    ptr++; // dont't touch alhpa
	}
	break;

      case PAL_RGB565:
	while ( ptr < end )
	{
	    *ptr = cc85s1[*ptr]; ptr++;
	    *ptr = cc86  [*ptr]; ptr++;
	    *ptr = cc85s1[*ptr]; ptr++;
	    *ptr++ = 0xff;
	}
	break;

      case PAL_RGB5A3:
	while ( ptr < end )
	{
	    *ptr = cc85[*ptr]; ptr++;
	    *ptr = cc85[*ptr]; ptr++;
	    *ptr = cc85[*ptr]; ptr++;
	    *ptr = cc83[*ptr]; ptr++;
	}
	break;

      default:
	// never reached
	ASSERT(0);
    }


    //--- convert now to palette

    err = ConvertToPALETTE(img,img,0,iform);
    img->pform = pform;
    if (err)
	return err;

    //--- transform the palette

    DASSERT(img->pal);
    const u8 *src = img->pal;
    u8 *dest = img->pal;
    uint n = img->n_pal;
    void (*wr16) ( void*, u16 ) = img->endian->wr16;

    switch (pform)
    {
      case PAL_IA8:
	img->is_grayed = true;
	while ( n-- > 0 )
	{
	    wr16( dest, src[0] | src[3] << 8 );
	    dest += 2;
	    src  += 4;
	}
	break;

      case PAL_RGB565:
	while ( n-- > 0 )
	{
	    wr16( dest,	  ( src[0] & 0xfe ) << 10 // already shifted by 1
			|   src[1]          <<  5
			|   src[2]          >>  1 // already shifted by 1
		);
	    dest += 2;
	    src  += 4;
	}
	break;

      case PAL_RGB5A3:
	while ( n-- > 0 )
	{
	    if ( src[3] == 0x07 )
		wr16( dest,  src[0] << 10
			   | src[1] <<  5
			   | src[2]
			   | 0x8000 );
	    else
		wr16( dest,  ( src[0] & 0xfe ) <<  7
			   | ( src[0] & 0xfe ) <<  3
			   | src[0]            >> 1
			   | src[3] << 12 );
	    dest += 2;
	    src  += 4;
	}
	break;

      default:
	// never reached
	ASSERT(0);
    }

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static enumError conv_to_C4
(
    Image_t		* img,		// valid destination
    const Image_t	* src_img,	// valid source
    palette_format_t	pform		// wanted palette format
)
{
    DASSERT(img);
    DASSERT(src_img);
    PRINT("conv_to_C8()\n");

    enumError err = create_C_palette(img,src_img,IMG_X_PAL4,pform);
    if (err)
	return err;

    const uint bits_per_pixel	=  4;
    const uint block_width	=  8;
    const uint block_height	=  8;

    uint h_blocks, v_blocks, img_size;
    CalcImageBlock(img, bits_per_pixel, block_width, block_height,
			&h_blocks, &v_blocks, &img_size, true );

    const uint xwidth = EXPAND8(img->width);
    //const uint xheight = EXPAND8(img->height);
    u8 *data = CALLOC(1,img_size);
    u8 *dest = data;
    const u16 *src1 = (u16*)img->data;

    while ( v_blocks-- > 0 )
    {
	const u16 * src2 = src1;
	uint hblk = h_blocks;
	while ( hblk-- > 0 )
	{
	    const u16 * src3 = src2;
	    uint iv = block_height;
	    while ( iv-- > 0 )
	    {
		uint ih = block_width / 2;
		while ( ih-- > 0 )
		{
		    *dest++ = src3[0] << 4 | src3[1];
		    src3 += 2;
		}
		src3 += xwidth - block_width;
	    }
	    src2 += block_width;
	}
	src1 += xwidth * block_height;
    }

    DASSERT( dest == data + img_size );
    DASSERT( (u8*)src1 <= img->data + img->data_size );

    img->data		= data;
    img->data_alloced	= true;
    img->data_size	= img_size;
    img->xwidth		= EXPAND8(img->width);
    img->xheight	= EXPAND8(img->height);
    img->iform		= IMG_C4;
    img->alpha_status	= 0;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError conv_to_C8
(
    Image_t		* img,		// valid destination
    const Image_t	* src_img,	// valid source
    palette_format_t	pform		// wanted palette format
)
{
    DASSERT(img);
    DASSERT(src_img);
    PRINT("conv_to_C8()\n");

    enumError err = create_C_palette(img,src_img,IMG_X_PAL8,pform);
    if (err)
	return err;

    const uint bits_per_pixel	=  8;
    const uint block_width	=  8;
    const uint block_height	=  4;

    uint h_blocks, v_blocks, img_size;
    CalcImageBlock(img, bits_per_pixel, block_width, block_height,
			&h_blocks, &v_blocks, &img_size, true );

    const uint xwidth = EXPAND8(img->width);
    //const uint xheight = EXPAND8(img->height);
    u8 *data = CALLOC(1,img_size);
    u8 *dest = data;
    const u16 *src1 = (u16*)img->data;

    while ( v_blocks-- > 0 )
    {
	const u16 * src2 = src1;
	uint hblk = h_blocks;
	while ( hblk-- > 0 )
	{
	    const u16 * src3 = src2;
	    uint iv = block_height;
	    while ( iv-- > 0 )
	    {
		uint ih = block_width;
		while ( ih-- > 0 )
		    *dest++ = *src3++;
		src3 += xwidth - block_width;
	    }
	    src2 += block_width;
	}
	src1 += xwidth * block_height;
    }

    DASSERT( dest == data + img_size );
    DASSERT( (u8*)src1 <= img->data + img->data_size );

    img->data		= data;
    img->data_alloced	= true;
    img->data_size	= img_size;
    img->xwidth		= EXPAND8(img->width);
    img->xheight	= EXPAND8(img->height);
    img->iform		= IMG_C8;
    img->alpha_status	= 0;
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError conv_to_C14X2
(
    Image_t		* img,		// valid destination
    const Image_t	* src_img,	// valid source
    palette_format_t	pform		// wanted palette format
)
{
    DASSERT(img);
    DASSERT(src_img);
    PRINT("conv_to_C14X2()\n");

    enumError err = create_C_palette(img,src_img,IMG_X_PAL14,pform);
    if (err)
	return err;

    const uint bits_per_pixel	= 16;
    const uint block_width	=  4;
    const uint block_height	=  4;

    uint h_blocks, v_blocks, img_size;
    CalcImageBlock(img, bits_per_pixel, block_width, block_height,
			&h_blocks, &v_blocks, &img_size, true );

    const uint xwidth = EXPAND8(img->width);
    //const uint xheight = EXPAND8(img->height);
    u8 *data = CALLOC(1,img_size);
    u8 *dest = data;
    const u16 *src1 = (u16*)img->data;
    void (*wr16) ( void*, u16 ) = img->endian->wr16;

    while ( v_blocks-- > 0 )
    {
	const u16 * src2 = src1;
	uint hblk = h_blocks;
	while ( hblk-- > 0 )
	{
	    const u16 * src3 = src2;
	    uint iv = block_height;
	    while ( iv-- > 0 )
	    {
		uint ih = block_width;
		while ( ih-- > 0 )
		{
		    wr16(dest,*src3++);
		    dest += 2;
		}
		src3 += xwidth - block_width;
	    }
	    src2 += block_width;
	}
	src1 += xwidth * block_height;
    }

    DASSERT( dest == data + img_size );
    DASSERT( (u8*)src1 <= img->data + img->data_size );

    img->data		= data;
    img->data_alloced	= true;
    img->data_size	= img_size;
    img->xwidth		= EXPAND8(img->width);
    img->xheight	= EXPAND8(img->height);
    img->iform		= IMG_C14X2;
    img->alpha_status	= 0;
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			conv_to_CMPR()			///////////////
///////////////////////////////////////////////////////////////////////////////

static uint calc_distance ( const u8 * v1, const u8 * v2 )
{
    const int d0 = (int)*v1++ - (int)*v2++;
    const int d1 = (int)*v1++ - (int)*v2++;
    const int d2 = (int)*v1++ - (int)*v2++;
    return abs(d0) + abs(d1) + abs(d2);
}

//-----------------------------------------------------------------------------

static uint calc_distance_square ( const u8 * v1, const u8 * v2 )
{
    const int d0 = (int)*v1++ - (int)*v2++;
    const int d1 = (int)*v1++ - (int)*v2++;
    const int d2 = (int)*v1++ - (int)*v2++;
    return d0*d0 + d1*d1 + d2*d2;
}

///////////////////////////////////////////////////////////////////////////////

void CMPR_close_info
(
    const u8		*data,		// source data
    cmpr_info_t		*info,		// info data structure
    u8			*dest,		// store destination data here (never 0)
    bool		fill_info	// true: fill info with statistics
)
{
    DASSERT(info);
    DASSERT(dest);

    u8 *pal0 = info->p[0];
    u8 *pal1 = info->p[1];

    if (!info->opaque_count)
    {
	// all pixel are transparent

	memcpy(dest,info->default_vector,sizeof(info->default_vector));
	if (fill_info)
	    memset(info->index,3,sizeof(info->index));
	return;
    }
    else if ( info->opaque_count < CMPR_MAX_COL )
    {
	// we have at least one transparent pixel

	u16 p0	= cc85[ pal0[0] ] << 11
		| cc86[ pal0[1] ] <<  5
		| cc85[ pal0[2] ];
	u16 p1	= cc85[ pal1[0] ] << 11
		| cc86[ pal1[1] ] <<  5
		| cc85[ pal1[2] ];
	if ( p0 == p1 )
	{
	    // make p0 < p1
	    p0 &= ~0x0020; // modify least significant bit of green
	    p1 |=  0x0020;
	}
	else if ( p0 > p1 )
	{
	    u16 ptemp = p0;
	    p0 = p1;
	    p1 = ptemp;
	}
	DASSERT( p0 < p1 );
	write_be16(dest,p0); dest += 2;
	write_be16(dest,p1); dest += 2;

	// re calculate palette colors

	pal0[0] = cc58[ p0 >> 11 ];
	pal0[1] = cc68[ p0 >>  5 & 0x3f ];
	pal0[2] = cc58[ p0       & 0x1f ];
	pal0[3] = 0xff;

	pal1[0] = cc58[ p1 >> 11 ];
	pal1[1] = cc68[ p1 >>  5 & 0x3f ];
	pal1[2] = cc58[ p1       & 0x1f ];
	pal1[3] = 0xff;

	// calculate median palette value

	u8 *pal2 = info->p[2];
	pal2[0] = ( pal0[0] + pal1[0] ) / 2;
	pal2[1] = ( pal0[1] + pal1[1] ) / 2;
	pal2[2] = ( pal0[2] + pal1[2] ) / 2;
	pal2[3] = 0xff;
	memcpy(info->p[3],pal2,4);

	uint i;
	for ( i = 0; i < 4; i++ )
	{
	    u8 val = 0;
	    uint j;
	    for ( j = 0; j < 4; j++, data += 4 )
	    {
		val <<= 2;
		if ( data[3] & 0x80 )
		{
		    const uint d0 = calc_distance(data,pal0);
		    const uint d1 = calc_distance(data,pal1);
		    const uint d2 = calc_distance(data,pal2);
		    if ( d1 <= d2 )
			val |= d1 <= d0;
		    else if ( d2 < d0 )
			val |= 2;
		}
		else
		    val |= 3;
	    }
	    *dest++ = val;
	}
    }
    else
    {
	// we haven't any transparent pixel

	u16 p0	= cc85[ pal0[0] ] << 11
		| cc86[ pal0[1] ] <<  5
		| cc85[ pal0[2] ];
	u16 p1	= cc85[ pal1[0] ] << 11
		| cc86[ pal1[1] ] <<  5
		| cc85[ pal1[2] ];
	if ( p0 == p1 )
	{
	    // make p0 > p1
	    p0 |= 1;
	    p1 &= ~(u16)1;
	}
	else if ( p0 < p1 )
	{
	    u16 ptemp = p0;
	    p0 = p1;
	    p1 = ptemp;
	}
	DASSERT( p0 > p1 );
	write_be16(dest,p0); dest += 2;
	write_be16(dest,p1); dest += 2;

	// re calculate palette colors

	pal0[0] = cc58[ p0 >> 11 ];
	pal0[1] = cc68[ p0 >>  5 & 0x3f ];
	pal0[2] = cc58[ p0       & 0x1f ];
	pal0[3] = 0xff;

	pal1[0] = cc58[ p1 >> 11 ];
	pal1[1] = cc68[ p1 >>  5 & 0x3f ];
	pal1[2] = cc58[ p1       & 0x1f ];
	pal1[3] = 0xff;

	// calculate median palette values

	u8 *pal2 = info->p[2];
	pal2[0] = ( 2 * pal0[0] + pal1[0] ) / 3;
	pal2[1] = ( 2 * pal0[1] + pal1[1] ) / 3;
	pal2[2] = ( 2 * pal0[2] + pal1[2] ) / 3;
	pal2[3] = 0xff;

	u8 *pal3 = info->p[3];
	pal3[0] = ( pal0[0] + 2 * pal1[0] ) / 3;
	pal3[1] = ( pal0[1] + 2 * pal1[1] ) / 3;
	pal3[2] = ( pal0[2] + 2 * pal1[2] ) / 3;
	pal3[3] = 0xff;

	uint i;
	for ( i = 0; i < 4; i++ )
	{
	    u8 val = 0;
	    uint j;
	    for ( j = 0; j < 4; j++, data += 4 )
	    {
		val <<= 2;
		const uint d0 = calc_distance(data,pal0);
		const uint d1 = calc_distance(data,pal1);
		const uint d2 = calc_distance(data,pal2);
		const uint d3 = calc_distance(data,pal3);
		if ( d0 <= d1 )
		{
		    if ( d2 <= d3 )
			val |= d0 <= d2 ? 0 : 2;
		    else
			val |= d0 <= d3 ? 0 : 3;
		}
		else
		{
		    if ( d2 <= d3 )
			val |= d1 <= d2 ? 1 : 2;
		    else
			val |= d1 <= d3 ? 1 : 3;
		}
	    }
	    *dest++ = val;
	}
    }

    if (fill_info)
    {
	uint i;
	for ( i = 0; i < 4; i++ )
	{
	    info->p[i][0] = cc58[cc85[info->p[i][0]]];
	    info->p[i][1] = cc68[cc86[info->p[i][1]]];
	    info->p[i][2] = cc58[cc85[info->p[i][2]]];
	}

	memcpy(info->cmpr,dest-8,8);
	dest -= 4;
	data -= CMPR_DATA_SIZE;

	uint val = 0;
	for ( i = 0; i < CMPR_MAX_COL; i++ )
	{
	    if (!(i&3))
		val = *dest++;
	    uint dist = 0;
	    switch ( val & 0xc0 )
	    {
		case 0x00:
		    info->index[i] = 0;
		    dist = calc_distance(data,info->p[0]);
		    break;

		case 0x40:
		    info->index[i] = 1;
		    dist = calc_distance(data,info->p[1]);
		    break;

		case 0x80:
		    info->index[i] = 2;
		    dist = calc_distance(data,info->p[2]);
		    break;

		case 0xc0:
		    info->index[i] = 3;
		    if ( info->opaque_count == CMPR_MAX_COL )
			dist = calc_distance(data,info->p[3]);
		    break;
	    }
	    info->total_dist += dist;
	    info->dist[i] = dist;
	    data += 4;
	    val <<= 2;
	}
    }
}

//
///////////////////////////////////////////////////////////////////////////////

void CMPR_wiimm
(
    const u8		*data,		// source data
    cmpr_info_t		*info		// info data structure
)
{
    DASSERT(info);
    InitializeCmprInfo(info);
    info->name = "Wiimm";

    typedef struct sum_t
    {
	u8   col[4];
	uint count;
    } sum_t;

    sum_t sum[CMPR_MAX_COL];
    uint n_sum = 0, opaque_count = 0;
    uint col[3] = {0,0,0};

    const u8 *data_end = data + CMPR_DATA_SIZE;
    const u8 *dat;
    for ( dat = data; dat < data_end; dat += 4 )
    {
	col[0] += dat[0];
	col[1] += dat[1];
	col[2] += dat[2];

	if ( dat[3] & 0x80 )
	{
	    opaque_count++;
	    u8 col[4];
	    col[0] = cc58[cc85[dat[0]]];
	    col[1] = cc68[cc86[dat[1]]];
	    col[2] = cc58[cc85[dat[2]]];

	    uint s;
	    for ( s = 0; s < n_sum; s++ )
	    {
		if (!memcmp(sum[s].col,col,3))
		{
		    sum[s].count++;
		    goto abort_s;
		}
	    }
	    col[3] = 0xff;
	    memcpy(sum[n_sum].col,col,4);
	    sum[n_sum].count = 1;
	    n_sum++;
	  abort_s:;
	}
    }

    if (!opt_cmpr_valid)
    {
	const u32 rgb	= col[0] / CMPR_MAX_COL << 16
			| col[1] / CMPR_MAX_COL <<  8
			| col[2] / CMPR_MAX_COL;

	// modify least significant bit of green
	const u16 c1 = RGB_to_RGB565(rgb) & ~0x0020;
	const u16 c2 = c1 | 0x0020;
	write_be16(info->default_vector,c1);
	write_be16(info->default_vector+2,c2);
    }

    info->opaque_count = opaque_count;
    if (!opaque_count)
	return;

    DASSERT(n_sum);
    if ( n_sum < 3 )
    {
	memcpy(info->p[0],sum[0].col,4);
	memcpy(info->p[1],sum[n_sum-1].col,4);
	return;
    }

    DASSERT( opaque_count >= 3 );
    //HEXDUMP16(0,0,sum,sizeof(sum));

    uint best0 = 0, best1 = 0, max_dist = UINT_MAX;
    if ( info->opaque_count < CMPR_MAX_COL )
    {
	// we have transparent points -> 1 middle point

	uint s0;
	for ( s0 = 0; s0 < n_sum; s0++ )
	{
	    u8 *pal0 = sum[s0].col;
	    uint s1;
	    for ( s1 = s0+1; s1 < n_sum; s1++ )
	    {
		u8 *pal1 = sum[s1].col;
		u8 pal2[4];
		pal2[0] = ( pal0[0] + pal1[0] ) / 2;
		pal2[1] = ( pal0[1] + pal1[1] ) / 2;
		pal2[2] = ( pal0[2] + pal1[2] ) / 2;

		uint dist = 0;
		const u8 * dat;
		for ( dat = data; dat < data_end && dist < max_dist; dat += 4 )
		{
		    if ( dat[3] & 0x80 )
		    {
			const uint d0 = calc_distance(dat,pal0);
			const uint d1 = calc_distance(dat,pal1);
			const uint d2 = calc_distance(dat,pal2);
			if ( d0 <= d1 )
			    dist += d0 < d2 ? d0 : d2;
			else
			    dist += d1 < d2 ? d1 : d2;
		    }
		}
		if ( max_dist > dist )
		{
		    max_dist = dist;
		    best0 = s0;
		    best1 = s1;
		}
	    }
	}
    }
    else
    {
	// no transparent points -> 2 middle point

	uint s0;
	for ( s0 = 0; s0 < n_sum; s0++ )
	{
	    u8 *pal0 = sum[s0].col;
	    uint s1;
	    for ( s1 = s0+1; s1 < n_sum; s1++ )
	    {
		u8 *pal1 = sum[s1].col;
		u8 pal2[4];
		pal2[0] = ( 2 * pal0[0] + pal1[0] ) / 3;
		pal2[1] = ( 2 * pal0[1] + pal1[1] ) / 3;
		pal2[2] = ( 2 * pal0[2] + pal1[2] ) / 3;
		u8 pal3[4];
		pal3[0] = ( pal0[0] + 2 * pal1[0] ) / 3;
		pal3[1] = ( pal0[1] + 2 * pal1[1] ) / 3;
		pal3[2] = ( pal0[2] + 2 * pal1[2] ) / 3;

		uint dist = 0;
		const u8 * dat;
		for ( dat = data; dat < data_end && dist < max_dist; dat += 4 )
		{
		    const uint d0 = calc_distance(dat,pal0);
		    const uint d1 = calc_distance(dat,pal1);
		    const uint d2 = calc_distance(dat,pal2);
		    const uint d3 = calc_distance(dat,pal3);
		    if ( d0 <= d1 )
		    {
			if ( d2 <= d3 )
			    dist += d0 < d2 ? d0 : d2;
			else
			    dist += d0 < d3 ? d0 : d3;
		    }
		    else
		    {
			if ( d2 <= d3 )
			    dist += d1 <= d2 ? d1 : d2;
			else
			    dist += d1 <= d3 ? d1 : d3;
		    }
		}
		if ( max_dist > dist )
		{
		    max_dist = dist;
		    best0 = s0;
		    best1 = s1;
		}
	    }
	}
    }

    memcpy(info->p[0],sum[best0].col,4);
    memcpy(info->p[1],sum[best1].col,4);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static enumError conv_to_CMPR
(
    Image_t		* dest_img,	// valid destination
    const Image_t	* src_img,	// valid source
    palette_format_t	pform		// wanted palette format => ignored
)
{
    DASSERT(dest_img);
    DASSERT(src_img);
    DASSERT( src_img->iform >= IMG_X__MIN && src_img->iform <= IMG_X__MAX );


    //--- first convert to IMG_X_RGB

    if ( src_img->iform != IMG_X_RGB )
    {
	enumError err = ConvertIMG(dest_img,false,src_img,IMG_X_RGB,PAL_INVALID);
	if (err)
	    return err;
    }
    else
	CopyIMG(dest_img,false,src_img,true);
    DASSERT( dest_img->iform == IMG_X_RGB );
    NormalizeFrameIMG(dest_img);


    //--- and now convert IMG_X_RGB -> CMPR

    const uint bits_per_pixel	= 4;
    const uint block_width	= 8;
    const uint block_height	= 8;

    uint h_blocks, v_blocks, img_size;
    CalcImageBlock( dest_img, bits_per_pixel, block_width, block_height,
			&h_blocks, &v_blocks, &img_size, true );

    u8 *data = CALLOC(1,img_size);
    u8 *dest = data;
    const u8 *src1 = dest_img->data;

    const uint block_size = block_width * 4;
    DASSERT( EXPAND8(dest_img->width) == dest_img->xwidth );
    const uint line_size = dest_img->xwidth * 4;
    const uint delta[] = { 0, 16, 4*line_size, 4*line_size+16 };

    while ( v_blocks-- > 0 )
    {
      const u8 *src2 = src1;
      uint hblk = h_blocks;
      while ( hblk-- > 0 )
      {
	uint subb;
	for ( subb = 0; subb < 4; subb++ )
	{
	    //---- first collect the data of the 16 pixel

	    u8 vector[16*4], *vect = vector;
	    const u8 *src3 = src2 + delta[subb];
	    uint i;
	    for ( i = 0; i < 4; i++ )
	    {
		memcpy(vect,src3,16);
		vect += 16;
		src3 += line_size;
	    }
	    DASSERT( vect == vector + sizeof(vector) );


	    //--- analyze data

	    cmpr_info_t info;
	    CMPR_wiimm(vector,&info);
	    PRINT("CMPR: no=%u, %08x %08x %08x %08x\n",
		info.opaque_count,
		*(u32*)info.p[0], *(u32*)info.p[1],
		*(u32*)info.p[2], *(u32*)info.p[3] );
	    CMPR_close_info(vector,&info,dest,false);
	    dest += 8;
	}
	src2 += block_size;
      }
      src1 += line_size * block_height;
    }

    DASSERT( dest == data + img_size );
    DASSERT( src1 <= dest_img->data + dest_img->data_size );

    AssignData(dest_img,dest_img,data,img_size,IMG_CMPR);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			IMG conversions			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError ConvertIMG
(
    Image_t		* dest,		// destination of conversion
    bool		init_dest,	// true: initialize 'dest' first
    const Image_t	* src,		// source of conversion.
					// if src==NULL or src==dest
					//   -> inplace conversion of 'dest'
    image_format_t	iform,		// new image format
    palette_format_t	pform		// new palette format
)
{
    DASSERT(dest);
    if (init_dest)
	InitializeIMG(dest);
    if (!src)
	src = dest;


    //--- base setup

    image_format_t temp_iform;
    iform = NormalizeIF(&temp_iform,iform,src->iform,pform);
    pform = NormalizePF(iform,pform,src->pform);
    PRINT("%*sCONVERT-IMG: [ %-6s -> %-6s -> %-6s ] [%s->%s] mm=%u {%u,%u}\n",
		convert_depth*2, "",
		GetImageFormatName(src->iform,"?"),
		GetImageFormatName(temp_iform,"?"),
		GetImageFormatName(iform,"?"),
		GetPaletteFormatName(src->pform,"?"),
		GetPaletteFormatName(pform,"?"),
		CountMipmapsIMG(src),
		src->conv_count, src->seq_num );

    convert_depth++;


    //--- mipmap support

    if (src->mipmap) // --> do the conversion for each image standalone!
    {
	if ( src != dest )
	    CopyIMG(dest,false,src,true); // this makes it easier to handle
	RemoveContainerIMG(dest);
	DASSERT(!dest->container);

	while (dest)
	{
	    Image_t *next = dest->mipmap;
	    dest->mipmap = 0;
	    enumError err = ConvertIMG(dest,false,dest,iform,pform);
	    dest->mipmap = next;
	    if (err)
	    {
		convert_depth--;
		return err;
	    }
	    dest = next;
	}
	convert_depth--;
	return ERR_OK;
    }


    //--- some more tests

    if ( convert_depth > 10
	|| src->iform == IMG_INVALID
	|| iform == src->iform
		&& ( src->pform == PAL_INVALID || pform == src->pform ))
    {
	convert_depth--;
	CopyIMG(dest,false,src,true);
	return ERR_OK;
    }


    //--- select sub conversions I

    enumError (*func1) ( Image_t*, const Image_t*, image_format_t ) = 0;

    switch (src->iform)
    {
      case IMG_I4:	func1 = conv_from_I4; break;
      case IMG_I8:	func1 = conv_from_I8; break;
      case IMG_IA4:	func1 = conv_from_IA4; break;
      case IMG_IA8:	func1 = conv_from_IA8; break;
      case IMG_RGB565:	func1 = conv_from_RGB565; break;
      case IMG_RGB5A3:	func1 = conv_from_RGB5A3; break;
      case IMG_RGBA32:	func1 = conv_from_RGBA32; break;
      case IMG_C4:	func1 = conv_from_C4; break;
      case IMG_C8:	func1 = conv_from_C8; break;
      case IMG_C14X2:	func1 = conv_from_C14X2; break;
      case IMG_CMPR:	func1 = conv_from_CMPR; break;

      case IMG_X_GRAY:	break;
      case IMG_X_RGB:	break;
      case IMG_X_PAL4:	break;
      case IMG_X_PAL8:	break;
      case IMG_X_PAL14:	break;

      default:
	convert_depth--;
	return ERROR0(ERR_INVALID_IFORM,
		"Image format 0x%02x [%s] not supported: %s\n",
		src->iform, GetImageFormatName(src->iform,"?"), src->path );
    }

    if (func1)
    {
	const enumError err = func1(dest,src,temp_iform);
	if (err)
	{
	    convert_depth--;
	    return err;
	}
	src = dest;
    }


    //--- select sub conversions II

    if ( temp_iform != src->iform )
    {
	enumError (*func2) ( Image_t*, const Image_t*, palette_format_t ) = 0;
	switch(temp_iform)
	{
	    case IMG_X_GRAY:	func2 = ConvertToGRAY; break;
	    case IMG_X_RGB:	func2 = ConvertToRGB; break;
	    case IMG_X_PAL:	func2 = conv_to_palette_auto; break;
	    case IMG_X_PAL4:	func2 = conv_to_palette_4; break;
	    case IMG_X_PAL8:	func2 = conv_to_palette_8; break;
	    case IMG_X_PAL14:	func2 = conv_to_palette_14; break;

	    default:		return ERROR0(ERR_INTERNAL,0);
	}

	if (func2)
	{
	    const enumError err = func2(dest,src,pform);
	    if (err)
	    {
		convert_depth--;
		return err;
	    }
	    src = dest;
	}
    }


    //--- select sub conversions III

    enumError (*func2) ( Image_t*, const Image_t*, palette_format_t ) = 0;
    switch(iform)
    {
      case IMG_I4:	func2 = conv_to_I4; break;
      case IMG_I8:	func2 = conv_to_I8; break;
      case IMG_IA4:	func2 = conv_to_IA4; break;
      case IMG_IA8:	func2 = conv_to_IA8; break;
      case IMG_RGB565:	func2 = conv_to_RGB565; break;
      case IMG_RGB5A3:	func2 = conv_to_RGB5A3; break;
      case IMG_RGBA32:	func2 = conv_to_RGBA32; break;
      case IMG_CMPR:	func2 = conv_to_CMPR; break;
      case IMG_X_GRAY:	func2 = ConvertToGRAY; break;
      case IMG_X_RGB:	func2 = ConvertToRGB; break;
      case IMG_X_PAL:	func2 = conv_to_palette_auto; break;
      case IMG_X_PAL4:	func2 = conv_to_palette_4; break;
      case IMG_X_PAL8:	func2 = conv_to_palette_8; break;
      case IMG_X_PAL14:	func2 = conv_to_palette_14; break;

      case IMG_C4:	func2 = conv_to_C4; break;
      case IMG_C8:	func2 = conv_to_C8; break;
      case IMG_C14X2:	func2 = conv_to_C14X2; break;

      default:
	convert_depth--;
	return ERROR0(ERR_INVALID_IFORM,
		"Conversion to image format 0x%02x [%s] not supported: %s\n",
		iform, GetImageFormatName(iform,"?"), src->path );
    }

    enumError err = func2 ? func2(dest,src,pform) : ERR_OK;
    convert_depth--;
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			Normalize IF + PF		///////////////
///////////////////////////////////////////////////////////////////////////////

image_format_t NormalizeIF
(
    image_format_t	* xform,	// not NULL: store the best IMG_X_* here
    image_format_t	iform,		// wanted image format
    image_format_t	x_default,	// default format, use if iform == AUTO
					// and return a X-format
    palette_format_t	pform		// related palette format
)
{
    switch (iform)
    {
	case IMG_I4:		// known gray formats
	case IMG_I8:
	case IMG_IA4:
	case IMG_IA8:
	case IMG_X_GRAY:
	    if (xform)
		*xform = IMG_X_GRAY;
	    return iform;

	case IMG_RGB565:	// known RGB formats
	case IMG_RGB5A3:
	case IMG_RGBA32:
	case IMG_CMPR:
	case IMG_X_RGB:
	case IMG_C4:
	case IMG_C8:
	case IMG_C14X2:
	    if (xform)
		*xform = IMG_X_RGB;
	    return iform;

	case IMG_X_PAL4:
	case IMG_X_PAL8:
	case IMG_X_PAL14:
	    if (xform)
		*xform = iform;
	    return iform;

	case IMG_X_PAL:
	    switch(x_default)
	    {
	      case IMG_I4:
		if (xform)
		    *xform = IMG_X_PAL4;
		return IMG_X_PAL4;

	      case IMG_I8:
	      case IMG_IA4:
		if (xform)
		    *xform = IMG_X_PAL8;
		return IMG_X_PAL8;

	      default:
		   if (xform)
		       *xform = IMG_X_PAL14;
		   return IMG_X_PAL14;
	    }

	case IMG_X_AUTO:
	    switch(x_default)
	    {
		case IMG_I4:
		case IMG_I8:
		case IMG_IA4:
		case IMG_IA8:
		case IMG_X_GRAY:
		    if (xform)
			*xform = IMG_X_GRAY;
		    return IMG_X_GRAY;

		case IMG_C4:
		case IMG_X_PAL4:
		    if (xform)
			*xform = IMG_X_PAL4;
		    return x_default;

		case IMG_C8:
		case IMG_X_PAL8:
		    if (xform)
			*xform = IMG_X_PAL8;
		    return x_default;

		case IMG_C14X2:
		case IMG_X_PAL14:
		    if (xform)
			*xform = IMG_X_PAL14;
		    return x_default;

		default:
		    if (xform)
			*xform = IMG_X_RGB;
		    return IMG_X_RGB;
	    }

	default:
	    return NormalizeIF(xform,x_default,IMG_X_RGB,pform);
    }
}

///////////////////////////////////////////////////////////////////////////////

palette_format_t NormalizePF
(
    image_format_t	iform,		// related image format
    palette_format_t	pform,		// wanted palette format
    palette_format_t	default_pform	// palette format for auto
)
{
    noPRINT("NormalizePF(%s,%s,%s)\n",
		GetImageFormatName(iform,"?"),
		GetPaletteFormatName(pform,"?"),
		GetPaletteFormatName(default_pform,"?") );

    switch (iform)
    {
	case IMG_C4:
	case IMG_C8:
	case IMG_C14X2:
	    switch(pform)
	    {
		case PAL_IA8:
		case PAL_RGB565:
		case PAL_RGB5A3:
		    return pform;

		default:
		    switch(default_pform)
		    {
			case PAL_IA8:
			case PAL_RGB565:
			case PAL_RGB5A3:
			    return default_pform;

			case PAL_AUTO:
			    return PAL_DEFAULT;

			default:
			    break;
		    }
		    break;
	    }
	    break;

	case IMG_X_PAL4:
	case IMG_X_PAL8:
	case IMG_X_PAL14:
	case IMG_X_PAL:
	    return PAL_X_RGB;

	default:
	    break;
    }

    return PAL_INVALID;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////

