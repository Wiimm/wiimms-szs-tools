
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
 *   Copyright (c) 2011-2020 by Dirk Clemens <wiimm@wiimm.de>              *
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
#include <png.h>
#include "lib-image.h"
#include "lib-breff.h"

///////////////////////////////////////////////////////////////////////////////

#ifdef TEST
  #define ENABLE_IMAGE_TYPE_LOG 0	// 0|1
  #define ENABLE_EXPORT_TIMER	0   	// 0|1|2
#else
  #define ENABLE_IMAGE_TYPE_LOG	0
  #define ENABLE_EXPORT_TIMER	0
#endif

//
///////////////////////////////////////////////////////////////////////////////
///////////////		    AssignIMG(), LoadIMG()		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError AssignIMG
(
    Image_t		* img,		// pointer to valid img
    int			init_img,	// <0:none, =0:reset, >0:init
    const u8		* data,		// source data
    uint		data_size,	// soze of 'data'
    uint		img_index,	// index of sub image, 0:main, >0:mipmaps
    bool		mipmaps,	// true: assign mipmaps
    const endian_func_t * endian,	// endian functions to read data
    ccp			fname		// object name, assigned
)
{
    DASSERT(img);
    DASSERT(data);
    noPRINT("ASSIGN-IMG: idx=%d mm=%d siz=%x %s\n",
		img_index, mipmaps, data_size, fname );

    if ( init_img > 0 )
	InitializeIMG(img);
    else if (!init_img)
	ResetIMG(img);

// [[analyse-magic]]
    const file_format_t fform = GetByMagicFF(data,data_size,data_size);

    image_format_t   iform = IMG_INVALID;
    palette_format_t pform = PAL_INVALID;
    uint width = 0, height = 0, n_pal = 0, psize = 0, n_img = 0;
    const u8 *idata = 0, *pdata = 0;
    bool calc_geo = false; // true: calculate geometry for mipmaps

    switch (fform)
    {
      case FF_TPL:
	{
	    const tpl_header_t * tpl;
	    const tpl_pal_header_t * tp;
	    const tpl_img_header_t * ti;

	    if (SetupPointerTPL( data, data_size, img_index,
				    &tpl, 0, &tp, &ti,
				    &pdata, &idata, &be_func ))
	    {
		iform	= be32(&ti->iform);
		width	= be16(&ti->width);
		height	= be16(&ti->height);
		n_img	= be32(&tpl->n_image);

		if (tp)
		{
		    pform = be32(&tp->pform);
		    n_pal = be16(&tp->n_entry);
		    psize = pdata < idata
				? idata - pdata
				: data + data_size - pdata;
		    noPRINT("PALETTE: %u*%02x[%s], off = 0x%zx, size = 0x%x, n= %u\n",
			n_pal, pform, GetImageFormatName(pform,"?"),
			pdata - data, psize, n_pal );
		}
	    }
	}
	break;

      case FF_BTI:
	{
	    const bti_header_t *bti = (bti_header_t*)data;
	    iform	= bti->iform;
	    idata	= data + be32(&bti->data_off);
	    width	= be16(&bti->width);
	    height	= be16(&bti->height);
	    n_img	= bti->n_image;
	    calc_geo	= true;

	    const u32 pal_off = be32(&bti->pal_off);
	    if ( pal_off && pal_off < data_size )
	    {
		pform	= be16(&bti->pform);
		pdata	= data + pal_off;
		psize	= pdata < idata ? idata - pdata : data_size - pal_off;
	    }
	}
	break;

      case FF_TEX:
      case FF_TEX_CT: // ??? [[CTCODE]] add ctcode info
	{
	    const brsub_header_t *bh	= (brsub_header_t*)data;
	    const uint n_grp = GetSectionNumBRSUB(data,data_size,endian);
	    const tex_info_t *ti	= (tex_info_t*)( bh->grp_offset + n_grp );
	    uint grp_off		= endian->rd32(&bh->grp_offset);

	    if ( grp_off < data_size )
	    {
		iform	= endian->rd32(&ti->iform);
		width	= endian->rd16(&ti->width);
		height	= endian->rd16(&ti->height);
		n_img	= endian->rd32(&ti->n_image);
		calc_geo= true;
		idata	= (u8*)data + grp_off;
	    }
	}
	break;

      case FF_BREFT_IMG:
	if ( data_size > sizeof(breft_image_t) )
	{
	    const breft_image_t * bi = (breft_image_t*)data;
	    iform	= bi->iform;
	    width	= be16(&bi->width);
	    height	= be16(&bi->height);
	    idata	= (u8*)data + sizeof(*bi);
	    n_img	= bi->n_image ? bi->n_image : 1;
	    calc_geo	= true;
	}
	break;

      default:
	return opt_ignore || fform == FF_UNKNOWN
		? ERR_WARNING
		: ERROR0(ERR_INVALID_IFORM,
			"No (supported) image file [file type=%s]: %s\n",
			GetNameFF(0,fform), fname);
    }

    const ImageGeometry_t *geo = GetImageGeometry(iform);
    if ( geo && calc_geo )
    {
	noPRINT_IF(img_index,"BASE IMAGE: %3u*%-3u %6zu\n",
		width, height, data+data_size-idata);
	uint n;
	for ( n = img_index; n > 0; n-- )
	{
	    uint img_size;
	    CalcImageGeometry(iform,width,height,0,0,0,0,&img_size);
	    idata  += img_size;
	    width  /= 2;
	    height /= 2;
	    noPRINT("NEXT IMAGE: %3u*%-3u %6zu %6u\n",
			width, height, data+data_size-idata, img_size );
	}
    }

    if ( !idata && data_size < 0x40 )
    {
	// small => only a fragment (e.g. header) => be silent
	return ERR_INVALID_IFORM;
    }

    uint delta = idata - data;
    if ( !idata || delta >= data_size )
	return ERROR0(ERR_INVALID_IFORM,
		"Invalid image format [file type=%s]: %s\n",
		GetNameFF(0,fform), fname );

    img->width		= width;
    img->height		= height;
    img->xwidth		= geo ? ALIGN32(width,geo->block_width) : EXPAND8(width);
    img->xheight	= geo ? ALIGN32(height,geo->block_height) : EXPAND8(height);
    img->alpha_status	= geo && !geo->has_alpha ? -1 : 0;

    //img->container	= set by caller if needed/wanted!
    img->data		= (u8*)idata;
    img->info_size	= data_size - delta;
    img->data_size	= geo	? img->xwidth * img->xheight * geo->bits_per_pixel / 8
				: img->info_size;
    img->data_alloced	= false;
    img->iform		= iform;
    img->info_iform	= iform;
    img->info_fform	= fform;
    img->info_n_image	= n_img;

    img->pal		= (u8*)pdata;
    img->pal_size	= psize;
    img->pal_alloced	= false;
    img->n_pal		= n_pal;
    img->pform		= pform;
    img->info_pform	= pform;

    img->endian		= endian;
    img->path		= fname;
    img->seq_num	= ++image_seq_num;

    noPRINT("-> %u*%u->%u*%u [%u=0x%x]\n",
		img->width, img->height, img->xwidth, img->xheight,
		img->data_size, img->data_size );

    if ( mipmaps && ++img_index < n_img )
    {
	DASSERT(!img->mipmap);
	img->mipmap = MALLOC(sizeof(*img->mipmap));
	AssignIMG(img->mipmap,true,data,data_size,img_index,mipmaps,endian,fname);
    }

    return PatchListIMG(img);
}

///////////////////////////////////////////////////////////////////////////////

enumError LoadIMG
(
    Image_t		* img,		// pointer to valid img
    bool		init_img,	// true: initialize 'img'
    ccp			fname,		// filename of source
    uint		img_index,	// index of sub image, 0:main, >0:mipmaps
    bool		mipmaps,	// true: load and assign mipmaps
    bool		allow_subfile,	// allow to extract szs sub files
    bool		ignore_no_file	// ignore if file does not exists
					// and return warning ERR_NOT_EXISTS
)
{
    DASSERT(img);
    DASSERT(fname);
    TRACE("LoadIMG(%p,%d,%d) fname=%s\n",img,init_img,ignore_no_file,fname);

    if (init_img)
	InitializeIMG(img);
    else
	ResetIMG(img);

    szs_extract_t eszs;
    if (allow_subfile)
    {
	enumError err = ExtractSZS(&eszs,true,fname,0,ignore_no_file);
	if (err)
	    return err;
    }
    else
	InitializeExtractSZS(&eszs);

    if (!eszs.data)
    {
	File_t F;
	enumError err = OpenFILE(&F,true,fname,ignore_no_file,false);
	if ( err || !F.f )
	    return err;

	if ( ignore_no_file && !S_ISREG(F.st.st_mode) )
	{
	    ResetFile(&F,0);
	    return ERR_WARNING;
	}

	u8 buf[0x200];
	size_t read_stat = fread(buf,1,sizeof(buf),F.f);
// [[analyse-magic]]
	const file_format_t fform = GetByMagicFF(buf,read_stat,0);
	if ( fform == FF_PNG )
	{
	    err = ReadPNG(img,mipmaps,&F,buf,read_stat);
	    img->path = F.fname;
	    F.fname = 0;
	    ResetFile(&F,0);
	    return err;
	}

	// use 'eszs' data structure to hold dynamic data
	eszs.data_size = F.st.st_size;
	eszs.data = MALLOC(eszs.data_size);
	eszs.data_alloced = true;

	memcpy(eszs.data,buf,read_stat);
	if ( read_stat < eszs.data_size )
	{
	    const uint read_len = eszs.data_size - read_stat;
	    read_stat = fread(eszs.data+read_stat,1,read_len,F.f);
	    if ( read_stat != read_len )
	    {
		ERROR1(ERR_READ_FAILED,"Can't read file: %s\n",fname);
		ResetFile(&F,0);
		return ERR_READ_FAILED;
	    }
	}
	ResetFile(&F,0);
	FreeString(eszs.fname);
	eszs.fname = STRDUP(fname);
    }

    enumError err = AssignIMG( img,-1, eszs.data, eszs.data_size,
				img_index, mipmaps, eszs.endian, eszs.fname );
    if (!err)
    {
	if (eszs.data_alloced)
	{
	    eszs.data_alloced = false;
	    img->container = eszs.data;
	}
	img->path_alloced = true;
	eszs.fname = 0;
    }

    ResetExtractSZS(&eszs);
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			MipmapOptions_t			///////////////
///////////////////////////////////////////////////////////////////////////////

void SetupMipmapOptions ( MipmapOptions_t *mmo )
{
    DASSERT(mmo);

    //memset(mmo,0,sizeof(*mmo));
    mmo->valid = true;

    if (opt_n_images)
    {
	mmo->force	= true;
	mmo->n_mipmap	= opt_n_images - 1;
	mmo->n_image	= opt_n_images;
	mmo->min_size	= 1;
    }
    else
    {
	mmo->force	= false;
	mmo->n_mipmap	= opt_max_images - 1;
	mmo->n_image	= opt_max_images;
	mmo->min_size	= opt_min_mipmap_size;
    }

    PRINT("MM-OPT/SETUP: %s\n",TextMipmapOptions(mmo));
}

///////////////////////////////////////////////////////////////////////////////

void SetupMipmapOptions1 ( MipmapOptions_t *mmo )
{
    DASSERT(mmo);

    //memset(mmo,0,sizeof(*mmo));
    mmo->valid		= true;
    mmo->force		= true;
    mmo->n_mipmap	= 0;
    mmo->n_image	= 1;
    mmo->min_size	= 1;

    PRINT("MM-OPT/SETUP1: %s\n",TextMipmapOptions(mmo));
}

///////////////////////////////////////////////////////////////////////////////

void CopyMipmapOptions ( MipmapOptions_t *dest, const MipmapOptions_t *src )
{
    DASSERT(dest);
    if (src)
    {
	memcpy(dest,src,sizeof(*dest));
	PRINT("MM-OPT/COPY: %s\n",TextMipmapOptions(dest));
    }
    else
	SetupMipmapOptions(dest);
}

///////////////////////////////////////////////////////////////////////////////

void MipmapOptionsByImage ( MipmapOptions_t *mmo, const Image_t *img )
{
    DASSERT(mmo);
    if (!mmo->valid)
	SetupMipmapOptions(mmo);

    if ( !mmo->force && img )
    {
	int n_image = img->mipmap ? CountMipmapsIMG(img) + 1 : mmo->n_image;
	if ( n_image < img->info_n_image )
	    n_image = img->info_n_image;

	if ( n_image > MAX_MIPMAPS )
	    n_image = MAX_MIPMAPS+1;
	else if ( n_image < 1 )
	    n_image = opt_max_images > 0 ? opt_max_images : 1;

	mmo->n_image  = n_image;
	mmo->n_mipmap = n_image - 1;
    }

    PRINT("MM-OPT/IMG: %s\n",TextMipmapOptions(mmo));
}

///////////////////////////////////////////////////////////////////////////////

ccp InfoMipmapOptions ( const MipmapOptions_t *mmo )
{
    if (!mmo)
	return "--";

    if (!mmo->valid)
	return "!!";

    char buf[20];
    int len = snprintf(buf,sizeof(buf),"%s%d/%d",
		mmo->force ? "f" : "m", mmo->n_mipmap, mmo->min_size );

    char *res = GetCircBuf(++len);
    memcpy(res,buf,len);
    return res;
}

///////////////////////////////////////////////////////////////////////////////

ccp TextMipmapOptions ( const MipmapOptions_t *mmo )
{
    if (!mmo)
	return "--";

    if (!mmo->valid)
	return "INVALID!";

    char buf[100];
    int len = snprintf(buf,sizeof(buf),"force=%d, nm=%d, ni=%d, minsize=%d",
		mmo->force, mmo->n_mipmap, mmo->n_image, mmo->min_size );

    char *res = GetCircBuf(++len);
    memcpy(res,buf,len);
    return res;
}

///////////////////////////////////////////////////////////////////////////////

void PrintMipmapOptions ( FILE *f, int indent, const MipmapOptions_t *mmo )
{
    DASSERT(f);
    DASSERT(mmo);

    indent = NormalizeIndent(indent);
    fprintf(f,"%*sMM-OPT: %s\n",indent, "", TextMipmapOptions(mmo));
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			mipmap helpers			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[mipmap_info_t]]

typedef struct mipmap_info_t
{
    MipmapOptions_t	mmo;		// mipmap options
    uint		n_mipmap;	// number of mipmaps to write
    uint		image_size;	// calculated size of all images
    Image_t		img;		// transformed image
    const Image_t	* src_img;	// pointer to source image

    // tpl helpers

    uint		img_head_off;	// offset of image header
    uint		img_data_off;	// offset of image data
    uint		img_data_size;	// size of image data

    uint		pal_head_off;	// offset of palette header
    uint		pal_data_off;	// offset of palette header
    uint		pal_data_size;	// size of palette data

} mipmap_info_t;

///////////////////////////////////////////////////////////////////////////////

void ResetMMI ( mipmap_info_t * mmi )
{
    DASSERT(mmi);
    ResetIMG(&mmi->img);
    memset(mmi,0,sizeof(*mmi));
    SetupMipmapOptions(&mmi->mmo);
}

///////////////////////////////////////////////////////////////////////////////

static enumError PrepareImages
(
    mipmap_info_t	* mmi,		// valid mipmap info
    const Image_t	* src_img,	// pointer to source image
    const MipmapOptions_t *mmo		// NULL or mipmap options

    // RETURNS (if return value == ERR_OK):
    //	 mmi->n_mipmap      : number of mipmaps to store
    //   mmi->image_size    : total image size (main+mipmaps)
    //	 mmi->img.info_size : data size of main image
)
{
    DASSERT(mmi);
    DASSERT(src_img);

    memset(mmi,0,sizeof(*mmi));
    mmi->src_img = src_img;

    CopyMipmapOptions(&mmi->mmo,mmo);
    MipmapOptionsByImage(&mmi->mmo,src_img);
    PRINT("PrepareImages() MMO: %s\n",TextMipmapOptions(&mmi->mmo));


    //--- copy & convert images

    CopyIMG(&mmi->img,true,src_img,false);
    Transform2InternIMG(&mmi->img);
    enumError err = ExecTransformIMG(&mmi->img);
    if (err)
	return err;


    //--- calculate image size

    const ImageGeometry_t *geo = GetImageGeometry(mmi->img.iform);
    if ( !geo || geo->is_x )
	return ERROR0(ERR_INTERNAL,0);

    uint ni, size = 0;
    uint wd = mmi->img.width;
    uint ht = mmi->img.height;
    uint n_image = mmi->mmo.n_image;

    for ( ni = 0; ni < n_image; ni++ )
    {
	size += CalcImageSize( wd, ht, geo->bits_per_pixel,
				geo->block_width, geo->block_height, 0,0,0,0 );
	if (!ni)
	    mmi->img.info_size = size;
	wd /= 2;
	ht /= 2;
	DASSERT( opt_min_mipmap_size >= 1 );
	if ( wd < mmi->mmo.min_size || ht < mmi->mmo.min_size )
	{
	    n_image = ni + 1;
	    break;
	}
    }

    mmi->n_mipmap   = n_image - 1;
    mmi->image_size = size;

    PRINT("SETUP IMAGES: N=1+%u, size=%u=0x%x, m0=%s\n",
		n_image-1, size, size, InfoMipmapOptions(&mmi->mmo) );
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError PrepareImages1
(
    mipmap_info_t	* mmi,		// valid mipmap info
    const Image_t	* src_img	// pointer to source image

    // RETURNS (if return value == ERR_OK):
    //	 mmi->n_mipmap      : number of mipmaps to store
    //   mmi->image_size    : total image size (main+mipmaps)
    //	 mmi->img.info_size : data size of main image
)
{
    DASSERT(mmi);
    DASSERT(src_img);

    MipmapOptions_t mmo;
    SetupMipmapOptions1(&mmo);
    return PrepareImages(mmi,src_img,&mmo);
}

///////////////////////////////////////////////////////////////////////////////

static enumError WriteImageData
(
    mipmap_info_t	* mmi,		// valid mipmap info
    u8			* data		// destination buffer
)
{
    DASSERT(mmi);
    DASSERT(data);

    const Image_t *img = &mmi->img;
    memcpy(data,img->data,img->info_size);
    if (!mmi->n_mipmap)
	return ERR_OK;

    const ImageGeometry_t *geo = GetImageGeometry(img->iform);
    if ( !geo || geo->is_x )
	return ERROR0(ERR_INTERNAL,0);

    Image_t temp;
    InitializeIMG(&temp);

    enumError err = ERR_OK;
    u8 * dest = data + img->info_size;
    img = mmi->src_img;
    DASSERT(img);

    uint wd = img->width;
    uint ht = img->height;
    uint ni;
    for ( ni = 0; ni < mmi->n_mipmap; ni++ )
    {
	if (img->mipmap)
	    img = img->mipmap;

	wd /= 2;
	ht /= 2;
	DASSERT( wd && ht );
	err = ResizeIMG(&temp,false,img,wd,ht);
	if (err)
	    break;
	//PRINT("RESIZE  %3u*%-3u ",wd,ht); HEXDUMP16(0,0,temp.data,16);

	err = ConvertIMG(&temp,false,0,mmi->img.iform,mmi->img.pform);
	if (err)
	    break;
	//PRINT("CONVERT %3u*%-3u ",wd,ht); HEXDUMP16(0,0,temp.data,16);

	DASSERT( dest + temp.data_size <= data + mmi->image_size );
	memcpy(dest,temp.data,temp.data_size);
	dest += temp.data_size;
    }

    ResetIMG(&temp);
    ResetIMG(&mmi->img);
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			SaveIMG()			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError SaveIMG
(
    Image_t		* img,		// pointer to valid img
    file_format_t	fform,		// file format
    const MipmapOptions_t *mmo,		// NULL or mipmap options
    ccp			fname,		// filename of source
    bool		overwrite	// true: force overwriting
)
{
    DASSERT(img);
    DASSERT(fname);

    PRINT("SaveIMG(N=%u,o=%d) %s, mo=%s, %s\n",
		CountMipmapsIMG(img), overwrite,
		PrintFormat3(fform,img->iform,img->pform),
		InfoMipmapOptions(mmo), fname );

    switch(fform)
    {
	case FF_TPL:		return SaveTPL(img,fname,overwrite);
	case FF_BTI:		return SaveBTI(img,mmo,fname,overwrite);
	case FF_TEX:		return SaveTEX(img,mmo,fname,overwrite,false);
	case FF_TEX_CT:		return SaveTEX(img,mmo,fname,overwrite,true);
	case FF_BREFT:
	case FF_BREFT_IMG:	return SaveBREFTIMG(img,mmo,fname,overwrite);
	case FF_PNG:		return SavePNG(img,true,fname,0,0,overwrite,0);

	default:
	    return ERROR0(ERR_INVALID_IFORM,
		"Can_t create image [file type=%s]: %s\n",
		GetNameFF(0,fform), fname);
    }
}

///////////////////////////////////////////////////////////////////////////////

enumError SaveTPL
(
    Image_t		* src_img,	// pointer to valid source img
    ccp			fname,		// filename of source
    bool		overwrite	// true: force overwriting
)
{
    DASSERT(src_img);
    DASSERT(fname);

    PRINT("SaveTPL(o=%d) %s\n", overwrite, fname );


    //--- setup images

    u8 *data = 0;
    const int n_image = CountMipmapsIMG(src_img) + 1;
    mipmap_info_t *mmi = CALLOC(n_image,sizeof(*mmi));
    enumError err = ERR_OK;

    const uint align	= 0x20;
    const uint tab_off	= sizeof(tpl_header_t);
    uint data_off	= tab_off + n_image * sizeof(tpl_imgtab_t);

    mipmap_info_t *m = mmi;
    const Image_t *img = src_img;


    //--- first loop: setup header offsets

    int i;
    for ( i = 0; i < n_image; i++, m++, img = img->mipmap )
    {
	DASSERT(img);
	err = PrepareImages1(m,img);
	if (err)
	    goto abort;

	m->pal_head_off = 0;
	m->img_head_off = data_off;

	if (m->img.n_pal)
	{
	    m->pal_head_off  = data_off;
	    m->pal_data_off  = m->pal_head_off + sizeof(tpl_pal_header_t);
	    m->pal_data_size = 2 * m->img.n_pal;
	    m->img_head_off  = ALIGN32( m->pal_data_off + m->pal_data_size, 4 );
	}
	data_off = ALIGN32( m->img_head_off + sizeof(tpl_img_header_t), 4 );
    }
    data_off = ALIGN32(data_off,align);


    //--- second loop: setup data offsets

    for ( i = 0, m = mmi; i < n_image; i++, m++ )
    {
	m->img_data_off  = data_off;
	m->img_data_size = m->image_size;
	data_off = ALIGN32( m->img_data_off + m->img_data_size, align );

	PRINT("%6x %6x %6x | %6x %6x %6x | %6x = %6u\n",
		m->pal_head_off, m->pal_data_off, m->pal_data_size,
		m->img_head_off, m->img_data_off, m->img_data_size,
		data_off, data_off );
    }


    //--- alloc data & setup file header

    data = CALLOC(1,data_off);
    const endian_func_t * endian = src_img->endian;

    tpl_header_t * tpl = (tpl_header_t*)data;
    endian->wr32(tpl->magic,TPL_MAGIC_NUM);
    endian->wr32(&tpl->n_image,n_image);
    endian->wr32(&tpl->imgtab_off,tab_off);

    tpl_imgtab_t * tab = (tpl_imgtab_t*)( data + tab_off );


    //--- third loop: copy data

    for ( i = 0, m = mmi; i < n_image; i++, m++ )
    {
	endian->wr32( &tab[i].image_off,   m->img_head_off );
	endian->wr32( &tab[i].palette_off, m->pal_head_off );

	if (m->img.n_pal)
	{
	    tpl_pal_header_t * tp = (tpl_pal_header_t*)( data + m->pal_head_off );
	    endian->wr16( &tp->n_entry,  m->img.n_pal );
	    endian->wr32( &tp->pform,    m->img.pform );
	    endian->wr32( &tp->data_off, m->pal_data_off );
	    memcpy( data + m->pal_data_off, m->img.pal, 2*m->img.n_pal );
	}

	tpl_img_header_t * ti = (tpl_img_header_t*)( data + m->img_head_off );
	endian->wr16( &ti->width,    m->img.width );
	endian->wr16( &ti->height,   m->img.height );
	endian->wr32( &ti->iform,    m->img.iform );
	endian->wr32( &ti->data_off, m->img_data_off );
	endian->wr32( &ti->min_filter, 1 );
	endian->wr32( &ti->mag_filter, 1 );

	err = WriteImageData( m, data + m->img_data_off );
	if (err)
	    goto abort;
    }


    //--- save to file & clean

    err = SaveFILE(fname,0,overwrite,data,data_off,0);

 abort:
    for ( i = 0; i < n_image; i++ )
	ResetMMI(mmi+i);
    FREE(mmi);
    FREE(data);
    return err;
}

///////////////////////////////////////////////////////////////////////////////

enumError SaveBTI
(
    Image_t		* src_img,	// pointer to valid source img
    const MipmapOptions_t *mmo,		// NULL or mipmap options
    ccp			fname,		// filename of source
    bool		overwrite	// true: force overwriting
)
{
    DASSERT(src_img);
    DASSERT(fname);

    PRINT0("SaveBTI(o=%d) mo=%s, %s\n", overwrite, InfoMipmapOptions(mmo), fname );


    //--- setup images

    u8 *data = 0;
    // don't know how to store palettes [[2do]]
    const bool no_pal_stat = Transform2NoPaletteIMG(src_img);

    mipmap_info_t mmi;
    enumError err = PrepareImages(&mmi,src_img,mmo);
    if (err)
	goto abort;

    static int warn_count = 3;
    if ( no_pal_stat && warn_count > 0 )
    {
	warn_count--;
	ERROR0(ERR_WARNING,
		"BTI files with palettes not supported yet. Image converted to '%s'.",
		PrintFormat3(0,mmi.img.iform,mmi.img.pform));
    }


    //--- calculate image size

    const uint data_off = sizeof(bti_header_t);
    const uint total_size = data_off + mmi.image_size;
    PRINT("TOTAL-SIZE: %x = %u\n",total_size,total_size);
    PRINT("IMAGE-SIZE: %x = %u, %u*%u, N=1+%u\n",
		mmi.img.data_size, mmi.img.data_size,
		mmi.img.width, mmi.img.height, mmi.n_mipmap );


    //--- alloc and setup data

    data = CALLOC(1,total_size);
    const endian_func_t * endian = &be_func;

    bti_header_t *bti = (bti_header_t*)data;
    bti->iform = mmi.img.iform;
    bti->n_image = mmi.n_mipmap+1;
    bti->unknown_14 = 1;
    bti->unknown_15 = 1;
    endian->wr16(&bti->width,mmi.img.width);
    endian->wr16(&bti->height,mmi.img.height);
    endian->wr32(&bti->data_off,data_off);

    ccp point = strrchr(fname,'.');
    const bool is_special = point && ( !strcasecmp(point,".btiEnv")
				    || !strcasecmp(point,".btiMat"));
    if (!is_special)
    {
	bti->unknown_01 = 2;
	bti->wrap_s = 1;
	bti->wrap_t = 1;
    }

    err = WriteImageData(&mmi,data+data_off);
    if (!err)
	err = SaveFILE(fname,0,overwrite,data,total_size,0);

 abort:
    FREE(data);
    ResetMMI(&mmi);
    return err;
}

///////////////////////////////////////////////////////////////////////////////

enumError SaveTEX
(
    Image_t		* src_img,	// pointer to valid source img
    const MipmapOptions_t *mmo,		// NULL or mipmap options
    ccp			fname,		// filename of source
    bool		overwrite,	// true: force overwriting
    bool		ctcode_support	// true: include a CT-CODE file
)
{
    DASSERT(src_img);
    DASSERT(fname);

    PRINT("SaveTEX(o=%d,ct=%d) mo=%s, %s\n",
		overwrite, ctcode_support, InfoMipmapOptions(mmo), fname );


    //--- setup images

    u8 *data = 0;
    const bool no_pal_stat = Transform2NoPaletteIMG(src_img);

    mipmap_info_t mmi;
    enumError err = PrepareImages(&mmi,src_img,mmo);
    if (err)
	goto abort;

    static int warn_count = 3;
    if ( no_pal_stat && warn_count > 0 )
    {
	warn_count--;
	ERROR0(ERR_WARNING,
		"TEX0 files don't support palettes, image converted to '%s'.",
		PrintFormat3(0,mmi.img.iform,mmi.img.pform));
    }


    //--- calculate image size

    ccp realfile = strrchr(fname,'/');
    realfile = realfile ? realfile + 1 : fname;
    const uint filelen = strlen(realfile);
    const uint grp_off = 0x40;
    const uint data_size = grp_off + mmi.image_size;
    const uint total_size = data_size + 4 + ALIGN32(filelen+1,4);

    PRINT("TOTAL-SIZE: %x = %u\n",total_size,total_size);
    PRINT("IMAGE-SIZE: %x = %u, %u*%u, N=1+%u\n",
		mmi.img.data_size, mmi.img.data_size,
		mmi.img.width, mmi.img.height, mmi.n_mipmap );


    //--- alloc and setup data

    data = CALLOC(1,total_size);
    const endian_func_t * endian = mmi.img.endian;

    brsub_header_t *bh = (brsub_header_t*)data;
    memcpy(bh->magic,TEX_MAGIC,sizeof(bh->magic));
    endian->wr32(&bh->size,data_size);
    endian->wr32(&bh->version,3);
    endian->wr32(&bh->grp_offset,grp_off);

    const uint n_grp = GetSectionNumBRSUB(data,data_size,endian);
    tex_info_t *ti = (tex_info_t*)( bh->grp_offset + n_grp );
    endian->wr32(&ti->name_off,data_size+4);
    endian->wr32(data+data_size,filelen);
    memcpy(data+data_size+4,realfile,filelen);
    endian->wr16(&ti->width,mmi.img.width);
    endian->wr16(&ti->height,mmi.img.height);
    endian->wr32(&ti->iform,mmi.img.iform);
    endian->wr32(&ti->n_image,mmi.n_mipmap+1);
    endian->wrf4(&ti->image_val,mmi.n_mipmap);

    err = WriteImageData(&mmi,data+grp_off);
    if (!err)
	err = SaveFILE(fname,0,overwrite,data,total_size,0);

 abort:
    FREE(data);
    ResetMMI(&mmi);
    return err;
}

///////////////////////////////////////////////////////////////////////////////

enumError SaveBREFTIMG
(
    Image_t		* src_img,	// pointer to valid img
    const MipmapOptions_t *mmo,		// NULL or mipmap options
    ccp			fname,		// filename of source
    bool		overwrite	// true: force overwriting
)
{
    DASSERT(src_img);
    DASSERT(fname);

    PRINT("SaveBREFTIMG(o=%d) mo=%s, %s\n",
		overwrite, InfoMipmapOptions(mmo), fname );


    //--- setup images

    u8 *data = 0;
    const bool no_pal_stat = Transform2NoPaletteIMG(src_img);

    mipmap_info_t mmi;
    enumError err = PrepareImages(&mmi,src_img,mmo);
    if (err)
	goto abort;

    static int warn_count = 3;
    if ( no_pal_stat && warn_count > 0 )
    {
	warn_count--;
	ERROR0(ERR_WARNING,
		"BREFT files don't support palettes, image converted to '%s'.",
		PrintFormat3(0,mmi.img.iform,mmi.img.pform));
    }


    //--- calculate image size

    const uint img_off = 0x20;
    const uint total_size = img_off + mmi.image_size;

    PRINT("TOTAL-SIZE: %x = %u\n",total_size,total_size);
    PRINT("IMAGE-SIZE: %x = %u, %u*%u, N=1+%u\n",
		mmi.img.data_size, mmi.img.data_size,
		mmi.img.width, mmi.img.height, mmi.n_mipmap );


    //--- alloc and setup data

    data = CALLOC(1,total_size);
    const endian_func_t * endian = mmi.img.endian;

    breft_image_t * bi = (breft_image_t*)data;
    endian->wr16(&bi->width,mmi.img.width);
    endian->wr16(&bi->height,mmi.img.height);
    endian->wr32(&bi->img_size,mmi.image_size);
    bi->iform	= mmi.img.iform;
    bi->n_image	= mmi.n_mipmap + 1;

    err = WriteImageData(&mmi,data+img_off);
    if (!err)
	err = SaveFILE(fname,0,overwrite,data,total_size,0);

 abort:
    FREE(data);
    ResetMMI(&mmi);
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			PNG callback functions		///////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct png_info_t
{
    ccp		head_msg;	// header for error messages
    File_t	*file;		// file pointer
    u8		* cache;	// cached data
    uint	cache_size;	// size of 'cache' data

} png_info_t;

///////////////////////////////////////////////////////////////////////////////

static void print_png_message ( enumError err, png_structp png_ptr, ccp message )
{
    const png_info_t * pinfo = png_get_error_ptr(png_ptr);
    if (!pinfo)
	ERROR0(err,"PNG error: %s",message);
    else if (pinfo->file)
    {
	ERROR0(err,"%s: %s: %s",pinfo->head_msg,message,pinfo->file->fname);
	RegisterFileError(pinfo->file,err);
	ResetFile(pinfo->file,false);
    }
    else
	ERROR0(err,"%s: %s",pinfo->head_msg,message);
}

///////////////////////////////////////////////////////////////////////////////

static void png_warning_func ( png_structp png_ptr, ccp message )
{
    print_png_message(ERR_PNG,png_ptr,message);
    longjmp(png_jmpbuf(png_ptr),1);
}

///////////////////////////////////////////////////////////////////////////////

static void png_error_func ( png_structp png_ptr, ccp message )
{
    print_png_message(ERR_PNG,png_ptr,message);
    longjmp(png_jmpbuf(png_ptr),1);
}

///////////////////////////////////////////////////////////////////////////////

static void png_read_func
	( png_structp png_ptr, png_bytep dest, png_size_t size )
{
    png_info_t * pinfo = png_get_io_ptr(png_ptr);
    DASSERT(pinfo);
    if ( pinfo->cache_size )
    {
	const uint max_read = size < pinfo->cache_size ? size : pinfo->cache_size;
	memcpy(dest,pinfo->cache,max_read);
	pinfo->cache	  += max_read;
	pinfo->cache_size -= max_read;
	dest		  += max_read;
	size		  -= max_read;
	errno = 0;
    }

    if ( size > 0 )
	fread(dest,1,size,pinfo->file->f);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			LoadPNG()			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError LoadPNG
(
    Image_t		* img,		// destination image
    bool		init_img,	// true: initialize 'img' first
    bool		mipmaps,	// true: try to load mipmaps

    ccp			path1,		// NULL or part #1 of path
    ccp			path2		// NULL or part #2 of path
)
{
    DASSERT(img);
    PRINT("LoadPNG(mm=%d) %s / %s\n",
		mipmaps, path1?path1:"", path2?path2:"" );

    if (init_img)
	InitializeIMG(img);
    else
	ResetIMG(img);


    //--- open file

    char pathbuf[PATH_MAX];
    ccp path = PathCatPP(pathbuf,sizeof(pathbuf),path1,path2);

    File_t f;
    enumError err = OpenFILE(&f,true,path,false,false);
    if (err)
    {
	ResetFile(&f,false);
	return err;
    }

    err = ReadPNG(img,mipmaps,&f,0,0);

    img->path = f.fname;
    f.fname = 0;
    img->path_alloced = true;
    memcpy(&img->fatt,&f.fatt,sizeof(img->fatt));

    ResetFile(&f,false);
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////

enumError ReadPNG
(
    Image_t		* img,		// destination image
    bool		mipmaps,	// true: try to load mipmaps
    File_t		* f,		// valid opened file
    u8			* cache,	// pre read data
    uint		cache_size	// size of 'cache'
)
{
    DASSERT(img);
    DASSERT(f);
    DASSERT(f->f);


    //--- setup png

    png_info_t pinfo;
    memset(&pinfo,0,sizeof(pinfo));
    pinfo.head_msg	= "Open PNG";
    pinfo.file		= f;
    pinfo.cache		= cache;
    pinfo.cache_size	= cache_size;

    png_structp png_ptr
	= png_create_read_struct( PNG_LIBPNG_VER_STRING,
					&pinfo, png_error_func, png_warning_func );
    if (!png_ptr)
	goto abort_init;

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
	png_destroy_read_struct(&png_ptr,0,0);
	goto abort_init;
    }

    png_infop end_info = png_create_info_struct(png_ptr);
    if (!end_info)
    {
	png_destroy_read_struct(&png_ptr,&info_ptr,0);
	goto abort_init;
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
	png_destroy_read_struct(&png_ptr,&info_ptr,&end_info);
	PRINT("ERR=%u\n",f->max_err);
	return f->max_err;
    }

    if (pinfo.cache_size)
    {
	DASSERT(pinfo.cache);
	png_set_read_fn(png_ptr,&pinfo,png_read_func);
    }
    else
	png_init_io(png_ptr,f->f);

    png_read_info(png_ptr, info_ptr);

    u32 width, height;
    int bit_depth, color_type, interlace;
    png_get_IHDR( png_ptr, info_ptr, &width, &height,
			&bit_depth, &color_type, &interlace, 0, 0 );
    noPRINT("--> %u*%u*%u, ct=%d, il=%d\n",
		width, height, bit_depth, color_type, interlace );

    if ( interlace != PNG_INTERLACE_NONE )
    {
	png_destroy_read_struct(&png_ptr,&info_ptr,&end_info);
	RegisterFileError(f,ERR_INVALID_IFORM);
	return ERROR0(ERR_INVALID_IFORM,
		    "Interlaced PNG not supported: %s\n",f->fname);
    }

    uint data_size;
    bool is_gray;
    img->alpha_status = -1;

    switch (color_type)
    {
	case PNG_COLOR_TYPE_GRAY_ALPHA:
	    img->alpha_status = 0;
	case PNG_COLOR_TYPE_GRAY:
	    is_gray = true;
	    data_size = EXPAND8(width) * EXPAND8(height) * 2;
	    img->iform = IMG_X_GRAY;
	    break;

	case PNG_COLOR_TYPE_RGB_ALPHA:
	case PNG_COLOR_TYPE_PALETTE:
	    img->alpha_status = 0;
	case PNG_COLOR_TYPE_RGB:
	    is_gray = false;
	    data_size = EXPAND8(width) * EXPAND8(height) * 4;
	    img->iform = IMG_X_RGB;
	    break;

	default:
	    png_destroy_read_struct(&png_ptr,&info_ptr,&end_info);
	    ERROR0(ERR_INVALID_IFORM,
			"Unsupported PNG color type: %s\n",f->fname);
	    RegisterFileError(f,ERR_INVALID_IFORM);
	    return RegisterFileError(f,ERR_INVALID_IFORM);
    }

    if ( color_type == PNG_COLOR_TYPE_PALETTE )
	png_set_palette_to_rgb(png_ptr);

    if ( png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS) )
	png_set_tRNS_to_alpha(png_ptr);

    if ( color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8 )
	png_set_expand_gray_1_2_4_to_8(png_ptr);

    if ( bit_depth == 16 )
	png_set_strip_16(png_ptr);

    png_set_add_alpha(png_ptr,0xff,PNG_FILLER_AFTER);
    png_read_update_info(png_ptr,info_ptr);

    u8 *data = MALLOC(data_size);

    img->data		= data;
    img->data_alloced	= true;
    img->data_size	= data_size;
    img->width		= width;
    img->height		= height;
    img->xwidth		= EXPAND8(width);
    img->xheight	= EXPAND8(height);
    img->info_iform	= img->iform;
    img->info_fform	= FF_PNG;


    //--- read data

    const uint rowlen = ( is_gray ? 2 : 4 ) * img->xwidth;

 #if HAVE_PRINT
    png_get_IHDR( png_ptr, info_ptr, &width, &height,
			&bit_depth, &color_type, &interlace, 0, 0 );
    noPRINT("--> %u*%u*%u, ct=%d, il=%d\n",
		width, height, bit_depth, color_type, interlace );

    const uint channels = png_get_channels(png_ptr,info_ptr);
    const uint rowbytes = png_get_rowbytes(png_ptr,info_ptr);

    PRINT("ReadPNG() %u*%u, ch=%u, gray=%d, alpha=%d, rl=%u,%u\n",
		width, height, channels, is_gray, img->alpha_status,
		rowbytes, rowlen );
 #endif

    while ( height-- > 0 )
    {
	png_read_row(png_ptr,(png_bytep)data,NULL);
	data += rowlen;
    }
    DASSERT( data <= img->data + data_size );


    //--- close png and file

    png_read_end(png_ptr, info_ptr);
    png_destroy_read_struct(&png_ptr,&info_ptr,&end_info);


    //--- mipmap support

    if (mipmaps)
    {
	ccp ext  = strrchr(f->fname,'.');
	ccp file = strrchr(f->fname,'/');
	if ( !ext || file && ext < file )
	    ext = f->fname + strlen(f->fname);

	Image_t * mm_img = img;
	uint count;
	for ( count = 1; ; count++ )
	{
	    char mm_path[PATH_MAX];
	    snprintf(mm_path,sizeof(mm_path),"%.*s.mm%u%s",
			(int)(ext-f->fname), f->fname, count, ext );
	    PRINT("Try open PNG: %s\n",mm_path);
	    File_t F;
	    enumError err = OpenFILE(&F,true,mm_path,true,false); // local err
	    if (err)
	    {
		ResetFile(&F,false);
		if ( err != ERR_NOT_EXISTS )
		    return err;
		break;
	    }

	    mm_img->mipmap = MALLOC(sizeof(*mm_img->mipmap));
	    mm_img = mm_img->mipmap;
	    InitializeIMG(mm_img);
	    err = ReadPNG(mm_img,false,&F,0,0);
	    memcpy(&mm_img->fatt,&F.fatt,sizeof(mm_img->fatt));
	    ResetFile(&F,false);
	    if (err)
		return err;
	}
	MM_COUNT(img);
    }

    return PatchListIMG(img);


    //--- abort

 abort_init:
    ERROR0(ERR_READ_FAILED,"Error while initializing PNG data: %s\n",f->fname);
    return RegisterFileError(f,ERR_READ_FAILED);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			SavePNG()			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError SavePNG
(
    Image_t		* img,		// valid image
    bool		mipmaps,	// save: save mipmaps (auto file name)
    ccp			path1,		// NULL or part #1 of path
    ccp			path2,		// NULL or part #2 of path
    int			store_alpha,	// <0:no alpha, =0:auto alpha, >0:store alpha
    bool		overwrite,	// true: force overwriting
    StringField_t	*file_list	// not NULL: store filenames of created png files

)
{
    DASSERT(img);

    //--- setup path & ...

    if ( !path2 || !*path2 )
    {
	path2 = path1;
	path1 = 0;
    }

    char pathbuf[PATH_MAX];
    ccp path = PathCatPP(pathbuf,sizeof(pathbuf),path1,path2);
    PRINT("SavePNG(mm=%d/%u) {%u,%u} alpha=%d : %s\n",
		mipmaps, CountMipmapsIMG(img),
		img->conv_count, img->seq_num, store_alpha, path );

    Transform2XIMG(img);
    enumError err = ExecTransformIMG(img);
    if (err)
	return err;

    if (!store_alpha)
	store_alpha = CheckAlphaIMG(img,false);

    if ( GetPaletteCountIF(img->iform) && ( store_alpha > 0 || img->n_pal > 0x100 ))
    {
	err = ConvertToRGB(img,img,PAL_AUTO);
	if (err)
	    return err;
    }

    //--- setup export mode

    enum export_mode
    {
	MD_F_ALPHA = 1,
	MD_F_RGB   = 2,
	MD_F_PAL   = 4,

	MD_G	= 0,
	MD_GA	= MD_F_ALPHA,
	MD_RGB	= MD_F_RGB,
	MD_RGBA	= MD_F_RGB | MD_F_ALPHA,
	MD_PAL	= MD_F_PAL,

    } export_mode;

    switch (img->iform)
    {
	case IMG_X_GRAY:
	    export_mode = store_alpha < 0 ? MD_G : MD_GA;
	    break;

	case IMG_X_RGB:
	    export_mode = store_alpha < 0 ? MD_RGB : MD_RGBA;
	    break;

	case IMG_X_PAL:
	case IMG_X_PAL4:
	case IMG_X_PAL8:
	case IMG_X_PAL14:
	    export_mode = MD_PAL;
	    break;

	default:
	    return ERROR0(ERR_INVALID_IFORM,
		"Image format 0x%02x [%s] not supported for PNG export: %s\n",
		img->iform, GetImageFormatName(img->iform,"?"), path );
    }


    //--- create file

    if (file_list)
    {
	ccp str = path + ( path1 ? strlen(path1) : 0 );
	if ( *str == '/' )
	    str++;
	InsertStringField(file_list,str,false);
    }

    File_t f;
    err = CreateFileOpt(&f,true,path,testmode, overwrite ? path : 0 );
    if ( err || !f.f )
    {
	ResetFile(&f,0);
	return err;
    }


    //--- setup png

    png_info_t pinfo;
    memset(&pinfo,0,sizeof(pinfo));
    pinfo.head_msg = "Create PNG";
    pinfo.file = &f;

    png_structp png_ptr
	= png_create_write_struct( PNG_LIBPNG_VER_STRING,
					&pinfo, png_error_func, png_warning_func );
    if (!png_ptr)
	goto abort_init;

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
	png_destroy_write_struct(&png_ptr,0);
	goto abort_init;
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
	png_destroy_write_struct(&png_ptr,&info_ptr);
	goto abort;
    }

    png_init_io(png_ptr,f.f);

    DASSERT( EXPAND8(img->width) == img->xwidth );
    DASSERT( EXPAND8(img->height) == img->xheight );

    switch (export_mode)
    {
	case MD_G:
	    png_set_IHDR( png_ptr, info_ptr,
		img->width, img->height, 8,
		PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT );
	    break;

	case MD_GA:
	    png_set_IHDR( png_ptr, info_ptr,
		img->width, img->height, 8,
		PNG_COLOR_TYPE_GRAY_ALPHA, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT );
	    break;

	case MD_RGB:
	    png_set_IHDR( png_ptr, info_ptr,
		img->width, img->height, 8,
		PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT );
	    break;

	case MD_RGBA:
	    png_set_IHDR( png_ptr, info_ptr,
		img->width, img->height, 8,
		PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT );
	    break;


	case MD_PAL:
	    png_set_IHDR( png_ptr, info_ptr,
		img->width, img->height, 8,
		PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT );

	    // iobuf to transform palette into 'no alpha'
	    {
		png_colorp dest = (png_colorp)iobuf;
		DASSERT( img->n_pal * sizeof(*dest) <= sizeof(iobuf) );
		const u8 * src = img->pal;
		uint c = img->n_pal;
		while ( c-- > 0 )
		{
		    dest->red   = *src++;
		    dest->green = *src++;
		    dest->blue  = *src++;
		    src++;
		    dest++;
		}
		png_set_PLTE( png_ptr, info_ptr, (png_colorp)iobuf, img->n_pal );
	    }
	    break;
    }


    //---- write strings

    png_text ptext[5], *pt = ptext;
    memset(ptext,0,sizeof(ptext));

    if (!opt_strip)
    {
	pt->compression	= PNG_TEXT_COMPRESSION_NONE;
	pt->key		= "creator";
	pt->text		= TOOLSET_LONG;
	pt->text_length	= strlen(pt->text);
	pt++;

	if ( img->info_fform > FF_UNKNOWN )
	{
	    pt->compression	= PNG_TEXT_COMPRESSION_NONE;
	    pt->key		= "file-format";
	    pt->text	= (char*)GetNameFF(0,img->info_fform);
	    pt->text_length	= strlen(pt->text);
	    pt++;
	}

	ccp info_text = GetImageFormatName(img->info_iform,0);
	if (info_text)
	{
	    pt->compression	= PNG_TEXT_COMPRESSION_NONE;
	    pt->key		= "image-format";
	    pt->text	= (char*)info_text;
	    pt->text_length	= strlen(pt->text);
	    pt++;
	}

	info_text = GetPaletteFormatName(img->info_pform,0);
	if (info_text)
	{
	    pt->compression	= PNG_TEXT_COMPRESSION_NONE;
	    pt->key		= "palette-format";
	    pt->text	= (char*)info_text;
	    pt->text_length	= strlen(pt->text);
	    pt++;
	}

    }

    DASSERT( pt - ptext <= sizeof(ptext)/sizeof(*ptext) );
    png_set_text(png_ptr,info_ptr,ptext,pt-ptext);
    png_write_info(png_ptr,info_ptr);

    //--- write image data

    switch (export_mode)
    {
      case MD_G:
	{
	    if ( img->xwidth > sizeof(iobuf) )
		goto abort_init;

	    const u8 * data = img->data;
	    uint ih = img->height;
	    while ( ih-- > 0 )
	    {
		u8 * dest = (u8*)iobuf;
		uint iw = img->xwidth;
		while ( iw-- > 0 )
		{
		    *dest++ = *data++;
		    data++;
		}
		png_write_row(png_ptr,(png_bytep)iobuf);
	    }
	}
	break;

      case MD_GA:
	{
	    const uint rowlen = img->xwidth * 2;
	    const u8 * data = img->data;
	    uint ih = img->height;
	    while ( ih-- > 0 )
	    {
		png_write_row(png_ptr,(png_bytep)data);
		data += rowlen;
	    }
	}
	break;

      case MD_RGB:
	{
	    if ( img->xwidth * 3 > sizeof(iobuf) )
		goto abort_init;

	    const u8 * data = img->data;
	    uint ih = img->height;
	    while ( ih-- > 0 )
	    {
		u8 * dest = (u8*)iobuf;
		uint iw = img->xwidth;
		while ( iw-- > 0 )
		{
		    *dest++ = *data++;
		    *dest++ = *data++;
		    *dest++ = *data++;
		    data++;
		}
		png_write_row(png_ptr,(png_bytep)iobuf);
	    }
	}
	break;

      case MD_RGBA:
	{
	    const uint rowlen = img->xwidth * 4;
	    const u8 * data = img->data;
	    uint ih = img->height;
	    while ( ih-- > 0 )
	    {
		png_write_row(png_ptr,(png_bytep)data);
		data += rowlen;
	    }
	}
	break;

      case MD_PAL:
	{
	    u8 * dest_end = (u8*)iobuf + img->xwidth;
	    const u16 *data = (u16*)img->data;
	    uint ih = img->height;
	    while ( ih-- > 0 )
	    {
		// transform index into single byte
		u8 * dest = (u8*)iobuf;
		while ( dest < dest_end )
		    *dest++ = *data++;
		png_write_row(png_ptr,(png_bytep)iobuf);
	    }
	}
	break;
    }


    //--- close png and file

    if (setjmp(png_jmpbuf(png_ptr)))
    {
	png_destroy_write_struct(&png_ptr,&info_ptr);
	goto abort;
    }

    png_write_end(png_ptr, info_ptr);
    png_destroy_write_struct(&png_ptr,&info_ptr);

    if (opt_preserve)
	memcpy(&f.fatt,&img->fatt,sizeof(f.fatt));
    err = ResetFile(&f,opt_preserve);

    if ( mipmaps && img->mipmap )
    {
	ccp ext  = strrchr(path2,'.');
	ccp file = strrchr(path2,'/');
	if ( !ext || file && ext < file )
	    ext = path2 + strlen(path2);

	uint count;
	for ( count = 1; !err; count++ )
	{
	    img = img->mipmap;
	    if (!img)
		break;

	    char mm_path[PATH_MAX];
	    snprintf(mm_path,sizeof(mm_path),"%.*s.mm%u%s",
		(int)(ext-path2), path2, count, ext );

	    PRINT("## SAVE -> %s\n",mm_path);
	    err = SavePNG(img,false,path1,mm_path,store_alpha,overwrite,file_list);
	}
    }
    return err;


    //--- abort

 abort_init:
    ERROR0(ERR_WRITE_FAILED,"Error while initializing PNG data: %s\n",path);
 abort:
    RegisterFileError(&f,ERR_WRITE_FAILED);
    return ResetFile(&f,false);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			ExportPNG			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError ExportPNG
(
    ccp			path1,		// NULL or part #1 of path
    ccp			path2,		// NULL or part #2 of path
    FileAttrib_t	* fatt,		// NULL or file attributes
    const void		* img_data,	// image data
    uint		img_size,	// image size, needed for validation
    uint		img_index,	// index of sub image, 0:main, >0:mipmaps
    bool		mipmaps,	// true: load and export mipmaps
    uint		* n_image,	// not null: store detected image count
    const endian_func_t * endian,	// endian functions to read data
    FormatFieldItem_t	* ffi,		// not null: store detected image+pal format
    bool		create_png,	// false: do some calculations but don't create png
    StringField_t	*file_list	// not NULL: store filenames of created png files
)
{
    DASSERT(img_data);
    DASSERT(endian);

    char pathbuf[PATH_MAX];
    ccp path = PathCatPP(pathbuf,sizeof(pathbuf),path1,path2);
    TRACE("ExportPNG(size=%u) %s\n", img_size, path );


 #if ENABLE_EXPORT_TIMER
    static u64 total_time = 0;
    u64 start_time = GetTimerUSec();
 #endif

    Image_t img;
    enumError err = AssignIMG( &img, 1, img_data, img_size,
				img_index, mipmaps, endian, path );
    if (n_image)
	*n_image = img.info_n_image;
    if (ffi)
    {
	ffi->iform = img.iform;
	ffi->pform = img.pform;
    }
    if (fatt)
	memcpy(&img.fatt,fatt,sizeof(img.fatt));

    if (!err)
    {
	#if ENABLE_IMAGE_TYPE_LOG
	{
	    static bool log_opened = false;
	    static FILE * log = 0;
	    if (!log_opened)
	    {
		log_opened = true;
		ccp fname = IsDirectory("pool",false)
			    ? "pool/_image-format.log"
			    :      "_image-format.log";
		log = fopen(fname,"wb");
		if (log)
		    printf(">>> IMAGE LOG OPENED: %s <<<\n",fname);
	    }
	    if (log)
		fprintf(log,"%02x [%-5s %-6s] : %4u * %4u : %s\n",
		    iform, GetNameFF(0,fform), GetImageFormatName(iform,"?"),
		    img.width, img.height, path );
	}
	#endif

	if (create_png)
	{
	    err = ConvertIMG(&img,false,0,IMG_X_AUTO,PAL_INVALID);
	    if (!err)
		err = SavePNG(&img,mipmaps,path1,path2,0,false,file_list);

	 #if ENABLE_EXPORT_TIMER
	    start_time = GetTimerUSec() - start_time;
	    total_time += start_time;
	    printf("\t\t--> TIME: %s [%s total]\n",
			PrintUSec(0,0,start_time,ENABLE_EXPORT_TIMER),
			PrintUSec(0,0,total_time,ENABLE_EXPORT_TIMER) );
	 #endif
	}
    }
    ResetIMG(&img);
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		    scan file and image format		///////////////
///////////////////////////////////////////////////////////////////////////////

typedef enum transform_mode_t
{
	TM_IDX_FILE,		// file format (=byte index)
	TM_IDX_IMG,		// image format (=byte index)
	TM_IDX_PAL,		// palette format (=byte index)
	TM_IDX_PALETTE,		// switch PALETTE
	TM_IDX_COLOR,		// switch COLOR
	TM_IDX_ALPHA,		// switch ALPHA

	TM_IDX_N,		// number of modes
	TM_IDX_MASK	= 7,

	TM_F_PAL	= 0x10,	// flag: Palette support

	TF_ON		= 1,
	TF_OFF		= 2,

} transform_mode_t;

///////////////////////////////////////////////////////////////////////////////

const KeywordTab_t cmdtab_transform[] =
{
	//--- file formats

	{ FF_TPL,	"TPL",		0,		TM_IDX_FILE|TM_F_PAL },
	{ FF_BTI,	"BTI",		0,		TM_IDX_FILE|TM_F_PAL },
	{ FF_TEX,	"TEX",		"TEX0",		TM_IDX_FILE },
	{ FF_BREFT_IMG,	"BREFT-IMG",	"BREFTIMG",	TM_IDX_FILE },
	 { FF_BREFT_IMG,"REFT-IMG",	"REFTIMG",	TM_IDX_FILE },
	 { FF_BREFT_IMG,"BT-IMG",	"BTIMG",	TM_IDX_FILE },
	{ FF_PNG,	"PNG",		0,		TM_IDX_FILE },


	//--- image formats

	{ IMG_I4,	"I4",		0,		TM_IDX_IMG },
	{ IMG_I8,	"I8",		0,		TM_IDX_IMG },
	{ IMG_IA4,	"IA4",		0,		TM_IDX_IMG },
	{ IMG_IA8,	"IA8",		0,		TM_IDX_IMG },
	{ IMG_RGB565,	"RGB565",	"R565",		TM_IDX_IMG },
	{ IMG_RGB5A3,	"RGB5A3",	"R3",		TM_IDX_IMG },
	{ IMG_RGBA32,	"RGBA32",	"RGBA8",	TM_IDX_IMG },
	 { IMG_RGBA32,	"R32",		"R8",		TM_IDX_IMG },
	{ IMG_C4,	"C4",		"CI4",		TM_IDX_IMG|TM_F_PAL },
	{ IMG_C8,	"C8",		"CI8",		TM_IDX_IMG|TM_F_PAL },
	{ IMG_C14X2,	"C14X2",	"CI14X2",	TM_IDX_IMG|TM_F_PAL },
	{ IMG_CMPR,	"CMPR",		0,		TM_IDX_IMG },


	//--- palette formats

	{ PAL_IA8,	"PIA8",		"P-IA8",	TM_IDX_PAL },
	 { PAL_IA8,	"P8",		"P-8",		TM_IDX_PAL },
	{ PAL_RGB565,	"PRGB565",	"P-RGB565",	TM_IDX_PAL },
	 { PAL_RGB565,	"P565",		"P-565",	TM_IDX_PAL },
	{ PAL_RGB5A3,	"PRGB5A3",	"P-RGB5A3",	TM_IDX_PAL },
	 { PAL_RGB5A3,	"P3",		"P-3",		TM_IDX_PAL },


	//--- switch PALETTE

	{ TF_ON,	"PALETTE",	0,		TM_IDX_PALETTE },
	{ TF_OFF,	"-PALETTE",	"NOPALETTE",	TM_IDX_PALETTE },


	//--- switch COLOR

	{ TF_ON,	"COLOR",	0,		TM_IDX_COLOR },
	{ TF_OFF,	"GRAY",		"GREY",		TM_IDX_COLOR },


	//--- switch ALPHA

	{ TF_ON,	"ALPHA",	0,		TM_IDX_ALPHA },
	{ TF_OFF,	"-ALPHA",	"NOALPHA",	TM_IDX_ALPHA },


	//--- end of table

	{ 0,0,0,0 }
};

///////////////////////////////////////////////////////////////////////////////

typedef struct transform_t
{
    char src[TM_IDX_N];
    char dest[TM_IDX_N];

} transform_t;

#define MAX_TRANSFORM 100

static uint n_transform = 0;
static transform_t transform[MAX_TRANSFORM];

///////////////////////////////////////////////////////////////////////////////

typedef struct transform_term_t
{
    ccp  arg;
    char res[TM_IDX_N];
    uint opt[TM_IDX_N];

} transform_term_t;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static enumError ScanTransformKeyword ( transform_term_t * term )
{
    DASSERT(term);
    ccp name = term->arg;

    while ( *name > 0 && *name <= ' ' )
	name++;

    char namebuf[20], *end = namebuf + sizeof(namebuf) - 1, *dest = namebuf;
    while ( *name >= '0' && *name <= '9'
	 || *name >= 'a' && *name <= 'z'
	 || *name >= 'A' && *name <= 'Z'
	 || *name == '-' )
    {
	if ( dest < end )
	    *dest++ = *name;
	name++;
    }
    while ( *name > 0 && *name <= ' ' )
	name++;
    term->arg = name;

    if ( dest == namebuf )
	return ERR_OK;
    *dest = 0;

    const KeywordTab_t * cmd = ScanKeyword(0,namebuf,cmdtab_transform);
    if (!cmd)
	return ERROR0(ERR_SYNTAX,
			"Invalid keyword for option --transform: %s\n",namebuf);

    uint idx = cmd->opt & TM_IDX_MASK;
    DASSERT( idx < TM_IDX_N );
    term->res[idx] = cmd->id;
    term->opt[idx] = cmd->opt;

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError ScanTransformTerm ( transform_term_t * term, ccp arg )
{
    DASSERT(term);
    memset(term,0,sizeof(*term));
    memset(term->res,-1,sizeof(term->res));
    if (!arg)
	return ERR_OK;

    for(;;)
    {
	while ( *arg > 0 && *arg <= ' ' || *arg == '.' )
	    arg++;
	term->arg = arg;
	enumError err = ScanTransformKeyword(term);
	if (err)
	    return err;
	arg = term->arg;
	if ( *arg != '.' )
	    return ERR_OK;
    }
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptTransform ( ccp arg )
{
    n_transform = 0;
    if (!arg)
	return 0;

    for(;;)
    {
	while ( *arg > 0 && *arg <= ' ' || *arg == ',' )
	    arg++;
	if (!*arg)
	    break;

	ccp src = 0, dest = arg;
	while ( *arg && *arg != ',' && *arg != '=' )
	    arg++;
	if ( *arg == '=' )
	{
	    src = dest;
	    arg++;
	    while ( *arg > 0 && *arg <= ' ' )
		arg++;
	    dest = arg;
	    while ( *arg && *arg != ',' && *arg != '=' )
		arg++;
	}

	if (!src && arg == dest )
	    goto err_abort;

	transform_term_t dterm;
	enumError err = ScanTransformTerm(&dterm,dest);
	if (err)
	    return err;
	if ( dterm.arg != arg )
	{
	    arg = dterm.arg;
	    goto err_abort;
	}
	PRINT0("DEST: %d,%d,%d [%u,%u,%u]\n",
		dterm.res[0], dterm.res[1], dterm.res[2],
		dterm.opt[0], dterm.opt[1], dterm.opt[2] );

	for(;;)
	{
	    transform_term_t sterm;
	    enumError err = ScanTransformTerm(&sterm,src);
	    if (err)
		return err;
	    PRINT0("SRC: %d,%d,%d [%u,%u,%u]\n",
		sterm.res[0], sterm.res[1], sterm.res[2],
		sterm.opt[0], sterm.opt[1], sterm.opt[2] );

	    if ( n_transform == MAX_TRANSFORM )
	    {
		ERROR0(ERR_SYNTAX,
		    "Option --transform: Only %u terms allowed!\n",MAX_TRANSFORM);
		return 1;
	    }

	    transform_t *t = transform + n_transform++;
	    memcpy(t->src,sterm.res,sizeof(t->src));
	    memcpy(t->dest,dterm.res,sizeof(t->dest));

	    src = sterm.arg;
	    if ( !src )
		break;
	    if ( *src == '+' )
		src++;
	    else if ( *src == '=' )
		break;
	    else
	    {
		arg = src;
		goto err_abort;
	    }
	}
    }
    return 0;

 err_abort:
    ERROR0(ERR_SYNTAX,
		"Invalid parameter for option --transform: %.20s\n",arg);
    return 1;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static ccp GetTransformName ( uint index, int mode, ccp not_found_text )
{
    if ( mode != -1 )
    {
	const KeywordTab_t * ct;
	for ( ct = cmdtab_transform; ct->name1; ct++ )
	    if ( ct->id == mode && ( ct->opt & TM_IDX_MASK ) == index )
		return ct->name1;
    }
    return not_found_text;
}

///////////////////////////////////////////////////////////////////////////////

void DumpTransformList ( FILE * f, int indent, bool force )
{
    DASSERT(f);
    if (!n_transform)
	return;

    fprintf(f,"\n"
	"%*s            source formats             ->         destination formats\n"
	"%*s file   image  palette pal  color alph -> file   image  palette pal  color alph\n"
	"%*s--------------------------------------------------------------------------------\n",
	indent, "", indent, "", indent, "" );
    uint i;
    for ( i = 0; i < n_transform; i++ )
    {
	const transform_t *t = transform + i;
	fprintf(f,
		"%*s %-6s %-6s %-7s %-4.4s %-5s %-4.4s"
		" -> %-6s %-6s %-7s %-4.4s %-5s %-4.4s\n",
		indent, "",
		GetTransformName(0,t->src[0],"*"),
		GetTransformName(1,t->src[1],"*"),
		GetTransformName(2,t->src[2],"*"),
		GetTransformName(3,t->src[3],"*"),
		GetTransformName(4,t->src[4],"*"),
		GetTransformName(5,t->src[5],"*"),
		GetTransformName(0,t->dest[0],"*"),
		GetTransformName(1,t->dest[1],"*"),
		GetTransformName(2,t->dest[2],"*"),
		GetTransformName(3,t->dest[3],"*"),
		GetTransformName(4,t->dest[4],"*"),
		GetTransformName(5,t->dest[5],"*") );
    }
    fprintf(f,"\n");
}

///////////////////////////////////////////////////////////////////////////////

ccp PrintTransformTuple ( ccp tuple )
{
    const uint bufsize = 50;
    char *buf  = GetCircBuf(bufsize);
    char *dest = buf, *bufend = buf + bufsize - TM_IDX_N;

    uint idx;
    for ( idx = 0; idx < TM_IDX_N; idx++ )
    {
	if ( tuple[idx] != -1 )
	{
	    ccp name = GetTransformName(idx,tuple[idx],0);
	    if (name)
	    {
		*dest++ = '.';
		dest = StringCopyE(dest,bufend++,name);
	    }
	}
    }
    *dest = 0;
    return dest == buf ? "*" : buf+1;
}

///////////////////////////////////////////////////////////////////////////////

ccp PrintFormat3
(
    file_format_t	fform,	// file format
    image_format_t	iform,	// image format
    palette_format_t	pform	// palette format
)
{
    char tuple[TM_IDX_N];
    memset(tuple,-1,sizeof(tuple));
    tuple[TM_IDX_FILE] = fform == FF_BREFT ? FF_BREFT_IMG : fform;
    if ( fform != FF_PNG )
    {
	tuple[TM_IDX_IMG] = iform;
	if (GetPaletteCountIF(iform))
	    tuple[TM_IDX_PAL] = pform;
    }
    return PrintTransformTuple(tuple);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		    transformation functions		///////////////
///////////////////////////////////////////////////////////////////////////////

void SetupTransformIMG ( Image_t * img )
{
    DASSERT(img);

    if (!img->tform_valid)
    {
	img->tform_valid   = true;
	img->tform_exec    = false;
	img->tform_gray    = false;
	img->tform_noalpha = false;
	img->tform_fform0  = img->tform_fform = FF_INVALID;
	img->tform_iform0  = img->tform_iform = img->iform;
	img->tform_pform0  = img->tform_pform = img->pform;
    }
}

///////////////////////////////////////////////////////////////////////////////

bool Transform2InternIMG ( Image_t * img )
{
    SetupTransformIMG(img);

    switch (img->tform_iform)
    {
	case IMG_X_GRAY:
	    img->tform_iform = img->tform_noalpha ? IMG_I8 : IMG_IA4;
	    return img->tform_exec = true;

	case IMG_X_RGB:
	    img->tform_iform = img->tform_noalpha ? IMG_RGB565 : IMG_RGB5A3;
	    return img->tform_exec = true;

	case IMG_X_PAL4:
	    img->tform_iform = IMG_C4;
	    return img->tform_exec = true;

	case IMG_X_PAL8:
	    img->tform_iform = IMG_C8;
	    return img->tform_exec = true;

	case IMG_X_PAL:
	case IMG_X_PAL14:
	    img->tform_iform = IMG_C14X2;
	    return img->tform_exec = true;

	default:
	    return false;
    }
}

///////////////////////////////////////////////////////////////////////////////

bool Transform2XIMG ( Image_t * img )
{
    SetupTransformIMG(img);

    switch (img->tform_iform)
    {
	case IMG_I4:
	case IMG_I8:
	case IMG_IA4:
	case IMG_IA8:
	    img->tform_iform = IMG_X_GRAY;
	    return img->tform_exec = true;

	case IMG_RGB565:
	case IMG_RGB5A3:
	case IMG_RGBA32:
	case IMG_CMPR:
	    img->tform_iform = IMG_X_RGB;
	    return img->tform_exec = true;

	case IMG_C4:
	    img->tform_iform = IMG_X_PAL4;
	    return img->tform_exec = true;

	case IMG_C8:
	    img->tform_iform = IMG_X_PAL8;
	    return img->tform_exec = true;

	case IMG_C14X2:
	    img->tform_iform = IMG_X_PAL14;
	    return img->tform_exec = true;

	default:
	    return false;
    }
}

///////////////////////////////////////////////////////////////////////////////

bool Transform2XRGB ( Image_t * img )
{
    SetupTransformIMG(img);
    if ( img->tform_iform == IMG_X_RGB )
	return false;

    img->tform_iform = IMG_X_RGB;
    return img->tform_exec = true;
}

///////////////////////////////////////////////////////////////////////////////

bool Transform2PaletteIMG ( Image_t * img )
{
    SetupTransformIMG(img);
    noPRINT("Transform2PaletteIMG(%s)\n",GetImageFormatName(img->tform_iform,"?"));

    switch (img->tform_iform)
    {
	case IMG_I4:
	    img->tform_iform = IMG_C4;
	    img->tform_pform = PAL_IA8;
	    return img->tform_exec = true;

	case IMG_I8:
	case IMG_IA4:
	case IMG_IA8:
	    img->tform_iform = IMG_C8;
	    img->tform_pform = PAL_IA8;
	    return img->tform_exec = true;

	case IMG_RGB565:
	    img->tform_iform = IMG_C8;
	    img->tform_pform = PAL_RGB565;
	    return img->tform_exec = true;

	case IMG_RGB5A3:
	case IMG_CMPR:
	    img->tform_iform = IMG_C8;
	    img->tform_pform = img->tform_noalpha ? PAL_RGB565 : PAL_RGB5A3;
	    return img->tform_exec = true;

	case IMG_RGBA32:
	    img->tform_iform = IMG_C14X2;
	    img->tform_pform = img->tform_noalpha ? PAL_RGB565 : PAL_RGB5A3;
	    return img->tform_exec = true;

	case IMG_X_GRAY:
	case IMG_X_RGB:
	    img->tform_iform = IMG_X_PAL;
	    img->tform_pform = PAL_X_RGB;
	    return img->tform_exec = true;

	default:
	    return false;
    }
}

///////////////////////////////////////////////////////////////////////////////

bool Transform2NoPaletteIMG ( Image_t * img )
{
    SetupTransformIMG(img);

    switch (img->tform_iform)
    {
	case IMG_C4:
	case IMG_C8:
	case IMG_C14X2:
	    img->tform_iform = PaletteToImageFormat(img->tform_pform,IMG_X_RGB);
	    return img->tform_exec = true;

	case IMG_X_PAL4:
	case IMG_X_PAL8:
	case IMG_X_PAL14:
	case IMG_X_PAL:
	    img->tform_iform = IMG_X_RGB;
	    return img->tform_exec = true;

	default:
	    return false;
    }
}

///////////////////////////////////////////////////////////////////////////////

bool Transform2GrayIMG ( Image_t * img )
{
    SetupTransformIMG(img);
    PRINT("TRANSFORM GRAY: %s\n",
		PrintFormat3(0,img->tform_iform,img->tform_pform));

    switch (img->tform_iform)
    {
	case IMG_RGB565:
	case IMG_RGB5A3:
	case IMG_CMPR:
	    img->tform_gray = true;
	    img->tform_iform = IMG_I8;
	    return img->tform_exec = true;

	case IMG_RGBA32:
	    img->tform_gray = true;
	    img->tform_iform = img->tform_noalpha ? IMG_I8 : IMG_IA8;
	    return img->tform_exec = true;

	case IMG_C4:
	case IMG_C8:
	case IMG_C14X2:
	    img->tform_gray = true;
	    img->tform_pform = PAL_IA8;
	    return img->tform_exec = true;

	case IMG_X_RGB:
	case IMG_X_PAL4:
	case IMG_X_PAL8:
	case IMG_X_PAL14:
	case IMG_X_PAL:
	    img->tform_gray = true;
	    img->tform_iform = IMG_X_GRAY;
	    return img->tform_exec = true;

	default:
	    return false;
    }
}

///////////////////////////////////////////////////////////////////////////////

bool Transform2ColorIMG ( Image_t * img )
{
    SetupTransformIMG(img);

    switch (img->tform_iform)
    {
	case IMG_I4:
	case IMG_I8:
	    img->tform_iform = IMG_RGB565;
	    return img->tform_exec = true;

	case IMG_IA4:
	case IMG_IA8:
	    img->tform_iform = IMG_RGB5A3;
	    return img->tform_exec = true;

	case IMG_C4:
	case IMG_C8:
	case IMG_C14X2:
	    if ( img->tform_pform != PAL_RGB565 )
		img->tform_pform = PAL_RGB5A3;
	    return img->tform_exec = true;

	case IMG_X_GRAY:
	    img->tform_iform = IMG_X_RGB;
	    return img->tform_exec = true;

	default:
	    return false;
    }
}

///////////////////////////////////////////////////////////////////////////////

bool Transform2AlphaIMG ( Image_t * img )
{
    SetupTransformIMG(img);

    switch (img->tform_iform)
    {
	case IMG_I4:
	    img->tform_iform = IMG_IA4;
	    return img->tform_exec = true;

	case IMG_I8:
	    img->tform_iform = IMG_IA8;
	    return img->tform_exec = true;

	case IMG_RGB565:
	    img->tform_iform = IMG_RGB5A3;
	    return img->tform_exec = true;

	case IMG_C4:
	case IMG_C8:
	case IMG_C14X2:
	    if ( img->tform_pform != PAL_IA8 )
		img->tform_pform = PAL_RGB5A3;
	    return img->tform_exec = true;

	default:
	    return false;
    }
}

///////////////////////////////////////////////////////////////////////////////

bool Transform2NoAlphaIMG ( Image_t * img )
{
    SetupTransformIMG(img);

    switch (img->tform_iform)
    {
	case IMG_IA4:
	    img->tform_iform = IMG_I4;
	    img->tform_noalpha = true;
	    return img->tform_exec = true;

	case IMG_IA8:
	    img->tform_iform = IMG_I8;
	    img->tform_noalpha = true;
	    return img->tform_exec = true;

	case IMG_RGB5A3:
	case IMG_RGBA32:
	    img->tform_iform = IMG_RGB565;
	    img->tform_noalpha = true;
	    return img->tform_exec = true;

	case IMG_C4:
	case IMG_C8:
	case IMG_C14X2:
	    if ( img->tform_pform != PAL_IA8 )
		img->tform_iform = IMG_RGB565;
	    img->tform_noalpha = true;
	    return img->tform_exec = true;

	case IMG_X_GRAY:
	case IMG_X_RGB:
	case IMG_X_PAL4:
	case IMG_X_PAL8:
	case IMG_X_PAL14:
	case IMG_X_PAL:
	    img->tform_noalpha = true;
	    return img->tform_exec = true;

	default:
	    return false;
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool TransformIMG
(
    // The image is NOT converted, only the planned transformations
    // are calculated. Use ExecTransformIMG() to execute transformation
    //  -> returns TRUE if a rule match

    Image_t		* img,		// destination of transforming
    int			indent		// <0:  no logging
					// >=0: log transform with indent
)
{
    SetupTransformIMG(img);

    uint i;
    for ( i = 0; i < n_transform; i++ )
    {
	const transform_t *t = transform + i;
	ccp s = t->src;
	if (   s[TM_IDX_FILE] != FF_INVALID && s[TM_IDX_FILE] != img->tform_fform0
	    || s[TM_IDX_IMG]  != FF_INVALID && s[TM_IDX_IMG]  != img->tform_iform0
	    || s[TM_IDX_PAL]  != FF_INVALID && s[TM_IDX_PAL]  != img->tform_pform0 )
	{
	    continue;
	}

	PRINT("TFORM-1: %s -> %s\n",
		PrintTransformTuple(t->src),
		PrintTransformTuple(t->dest) );

	const int palette = s[TM_IDX_PALETTE];
	if ( palette > 0 )
	{
	    if ( ( palette == TF_OFF ) == ( GetPaletteCountIF(img->iform) != 0 ) )
		continue;
	}

	const int color = s[TM_IDX_COLOR];
	if ( color > 0 )
	{
	    if ( ( color == TF_ON ) == IsGrayIMG(img) )
		continue;
	}

	const int alpha = s[TM_IDX_ALPHA];
	if ( alpha > 0 )
	{
	    if ( ( alpha == TF_ON ? -1 : 1 ) == CheckAlphaIMG(img,false) )
		continue;
	}

	noPRINT("TFORM-2: %s -> %s\n",
		PrintTransformTuple(t->src),
		PrintTransformTuple(t->dest) );

	if ( indent >= 0 )
	{
	    printf("%*s- Transform: %s -> %s\n",
		indent, "",
		PrintTransformTuple(t->src),
		PrintTransformTuple(t->dest) );
	}

	if ( t->dest[TM_IDX_PALETTE] == TF_ON )
	    Transform2PaletteIMG(img);
	else if ( t->dest[TM_IDX_PALETTE] == TF_OFF )
	    Transform2NoPaletteIMG(img);

	if ( t->dest[TM_IDX_COLOR] == TF_ON )
	    Transform2ColorIMG(img);
	else if ( t->dest[TM_IDX_COLOR] == TF_OFF )
	    Transform2GrayIMG(img);

	if ( t->dest[TM_IDX_ALPHA] == TF_ON )
	    Transform2AlphaIMG(img);
	else if ( t->dest[TM_IDX_ALPHA] == TF_OFF )
	    Transform2NoAlphaIMG(img);

	if ( t->dest[TM_IDX_FILE] != FF_INVALID )
	    img->tform_fform = t->dest[TM_IDX_FILE];

	if ( t->dest[TM_IDX_IMG] != IMG_INVALID )
	    img->tform_iform = t->dest[TM_IDX_IMG];

	if ( t->dest[TM_IDX_PAL] != PAL_INVALID )
	    img->tform_pform = t->dest[TM_IDX_PAL];

	return img->tform_exec = true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool Transform3IMG
(
    Image_t		* img,		// valid image
    file_format_t	fform,		// file format
    image_format_t	iform,		// image format
    palette_format_t	pform,		// palette format
    bool		def_base	// define *form values as base
)
{
    SetupTransformIMG(img);
    bool stat = false;

    fform = IsImageFF(fform,true);
    if ( fform != FF_UNKNOWN && img->tform_fform != fform )
    {
	stat = true; // do not set img->tform_exec
	img->tform_fform = fform;
    }

    if ( iform != IMG_INVALID  && img->tform_iform != iform )
    {
	img->tform_exec = stat = true;
	img->tform_iform = iform;
    }

    if ( pform != PAL_INVALID  && img->tform_pform != pform )
    {
	img->tform_exec = stat = true;
	img->tform_pform = pform;
    }

    if (def_base)
    {
	img->tform_fform0 = fform;
	img->tform_iform0 = iform;
	img->tform_pform0 = pform;
    }

    return stat;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

enumError ExecTransformIMG
(
    Image_t		* img		// image to transform
)
{
    DASSERT(img);

    enumError err = ERR_OK;
    if ( !img->tform_valid || !img->tform_exec )
	goto abort;

    PRINT("ExecTransformIMG() %s -> %s\n",
		PrintFormat3(0,img->iform,img->pform),
		PrintFormat3(0,img->tform_iform,img->tform_pform) );

    if ( img->tform_gray && !img->is_grayed )
    {
	err = ConvertIMG( img, false, 0, IMG_X_GRAY, PAL_AUTO );
	if (err)
	    goto abort;
    }

    if ( img->tform_noalpha && CheckAlphaIMG(img,false) >= 0 )
    {
	err = ConvertIMG( img, false, 0, IMG_X_AUTO, PAL_AUTO );
	if (err)
	    goto abort;

	switch(img->iform)
	{
	  case IMG_X_GRAY:
	    {
		uint n = img->xwidth * img->xheight;
		u8 * data = img->data + 1;
		while ( n-- > 0 )
		{
		    *data = 0xff;
		    data += 2;
		}
	    }
	    break;

	  case IMG_X_RGB:
	    {
		uint n = img->xwidth * img->xheight;
		u8 * data = img->data + 3;
		while ( n-- > 0 )
		{
		    *data = 0xff;
		    data += 4;
		}
	    }
	    break;

	  case IMG_X_PAL:
	  case IMG_X_PAL4:
	  case IMG_X_PAL8:
	  case IMG_X_PAL14:
	    if (img->n_pal)
	    {
		DASSERT(img->pal);
		uint n = img->n_pal;
		u8 * data = img->data + 1;
		while ( n-- > 0 )
		{
		    *data = 0xff;
		    data += 2;
		}
	    }
	    break;

	  default:
	    return ERROR0(ERR_INTERNAL,0);
	}
	img->alpha_status = -1;
    }

    err = ConvertIMG( img, false, 0, img->tform_iform, img->tform_pform );

 abort:
    img->tform_valid = false;
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    TPL support			///////////////
///////////////////////////////////////////////////////////////////////////////

uint GetNImagesTPL
(
    const u8			* data,		// TPL data
    uint			data_size,	// size of tpl data
    const endian_func_t		* endian	// endian functions
)
{
    DASSERT(data);
    DASSERT(endian);

    return data_size >= sizeof(tpl_header_t) && endian->rd32(data) == TPL_MAGIC_NUM
		? endian->rd32(data+4)
		: 0;
}

///////////////////////////////////////////////////////////////////////////////

bool SetupPointerTPL
(
    const u8			* data,		// TPL data
    uint			data_size,	// size of tpl data
    uint			img_index,	// index of image to extract
    const tpl_header_t		** tpl_head,	// not NULL: store pointer here
    const tpl_imgtab_t		** tpl_tab,	// not NULL: store pointer here
    const tpl_pal_header_t	** tpl_pal,	// not NULL: store pointer here
    const tpl_img_header_t	** tpl_img,	// not NULL: store pointer here
    const u8			** pal_data,	// not NULL: store pointer to pal data
    const u8			** img_data,	// not NULL: store pointer to img data
    const endian_func_t		* endian	// endian functions
)
{
    DASSERT(data);
    DASSERT(endian);

    if ( data_size >= sizeof(tpl_header_t) && endian->rd32(data) == TPL_MAGIC_NUM )
    {
      const tpl_header_t *tpl = (tpl_header_t*)data;
      const uint n_img = endian->rd32(&tpl->n_image);
      const u32 tab_off = endian->rd32(&tpl->imgtab_off);
      if ( img_index < n_img && tab_off + n_img*sizeof(tpl_imgtab_t) <= data_size )
      {
	const tpl_imgtab_t *tab = (tpl_imgtab_t*)( data + tab_off ) + img_index;
	const u32 img_off = endian->rd32(&tab->image_off);
	if ( img_off && img_off + sizeof(tpl_img_header_t) <= data_size )
	{
	    const tpl_img_header_t *img = (tpl_img_header_t*)(data+img_off);
	    const u32 img_data_off = endian->rd32(&img->data_off);
	    if ( !img_data_off || img_data_off >= data_size )
		goto abort;

	    if (tpl_head) *tpl_head = tpl;
	    if (tpl_tab)  *tpl_tab  = tab;
	    if (tpl_pal)  *tpl_pal  = 0;
	    if (tpl_img)  *tpl_img  = img;
	    if (pal_data) *pal_data = 0;
	    if (img_data) *img_data = data + img_data_off;

	    const u32 pal_off = endian->rd32(&tab->palette_off);
	    if (pal_off)
	    {
		if ( pal_off > sizeof(tpl_pal_header_t) >= data_size )
		    goto abort;

		const tpl_pal_header_t *pal = (tpl_pal_header_t*)(data+pal_off);
		const u32 pal_data_off = endian->rd32(&pal->data_off);
		if ( !pal_data_off || pal_data_off >= data_size )
		    goto abort;

		if (tpl_pal)  *tpl_pal = pal;
		if (pal_data) *pal_data = data + pal_data_off;
	    }
	    return true;
	}
      }
    }

 abort:
    if (tpl_head) *tpl_head = 0;
    if (tpl_tab)  *tpl_tab  = 0;
    if (tpl_pal)  *tpl_pal  = 0;
    if (tpl_img)  *tpl_img  = 0;
    if (pal_data) *pal_data = 0;
    if (img_data) *img_data = 0;
    return false;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    BTI support			///////////////
///////////////////////////////////////////////////////////////////////////////

uint GetNImagesBTI
(
    const u8			* data,		// bti data
    uint			data_size	// size of bti data
)
{
    DASSERT(data);
    if ( IsValidBTI(data,data_size,0,0) >= VALID_ERROR )
	return 0;

    const bti_header_t *bti = (bti_header_t*)data;
    return bti->n_image;
}

///////////////////////////////////////////////////////////////////////////////
#if 0

bool SetupPointerBTI
(
    const u8			* data,		// BTI data
    uint			data_size,	// size of bti data
    uint			img_index,	// index of image to extract
    const bti_header_t		** bti_head,	// not NULL: store pointer here
    const bti_imgtab_t		** bti_tab,	// not NULL: store pointer here
    const bti_pal_header_t	** bti_pal,	// not NULL: store pointer here
    const bti_img_header_t	** bti_img,	// not NULL: store pointer here
    const u8			** pal_data,	// not NULL: store pointer to pal data
    const u8			** img_data,	// not NULL: store pointer to img data
    const endian_func_t		* endian	// endian functions
)
{
    DASSERT(data);
    DASSERT(endian);

    if ( data_size >= sizeof(bti_header_t) && endian->rd32(data) == BTI_MAGIC_NUM )
    {
      const bti_header_t *bti = (bti_header_t*)data;
      const uint n_img = endian->rd32(&bti->n_image);
      const u32 tab_off = endian->rd32(&bti->imgtab_off);
      if ( img_index < n_img && tab_off + n_img*sizeof(bti_imgtab_t) <= data_size )
      {
	const bti_imgtab_t *tab = (bti_imgtab_t*)( data + tab_off ) + img_index;
	const u32 img_off = endian->rd32(&tab->image_off);
	if ( img_off && img_off + sizeof(bti_img_header_t) <= data_size )
	{
	    const bti_img_header_t *img = (bti_img_header_t*)(data+img_off);
	    const u32 img_data_off = endian->rd32(&img->data_off);
	    if ( !img_data_off || img_data_off >= data_size )
		goto abort;

	    if (bti_head) *bti_head = bti;
	    if (bti_tab)  *bti_tab  = tab;
	    if (bti_pal)  *bti_pal  = 0;
	    if (bti_img)  *bti_img  = img;
	    if (pal_data) *pal_data = 0;
	    if (img_data) *img_data = data + img_data_off;

	    const u32 pal_off = endian->rd32(&tab->palette_off);
	    if (pal_off)
	    {
		if ( pal_off > sizeof(bti_pal_header_t) >= data_size )
		    goto abort;

		const bti_pal_header_t *pal = (bti_pal_header_t*)(data+pal_off);
		const u32 pal_data_off = endian->rd32(&pal->data_off);
		if ( !pal_data_off || pal_data_off >= data_size )
		    goto abort;

		if (bti_pal)  *bti_pal = pal;
		if (pal_data) *pal_data = data + pal_data_off;
	    }
	    return true;
	}
      }
    }

 abort:
    if (bti_head) *bti_head = 0;
    if (bti_tab)  *bti_tab  = 0;
    if (bti_pal)  *bti_pal  = 0;
    if (bti_img)  *bti_img  = 0;
    if (pal_data) *pal_data = 0;
    if (img_data) *img_data = 0;
    return false;
}

#endif
//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////

