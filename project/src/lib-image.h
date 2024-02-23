
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

#ifndef SZS_LIB_IMAGE_H
#define SZS_LIB_IMAGE_H 1

#define _GNU_SOURCE 1

#include "lib-std.h"
#include "lib-numeric.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			relevant infos			///////////////
///////////////////////////////////////////////////////////////////////////////

// https://wiki.tockdom.de/index.php?title=Image_Formats

//
///////////////////////////////////////////////////////////////////////////////
///////////////			Color_t				///////////////
///////////////////////////////////////////////////////////////////////////////
// [[Color_t]]

typedef union Color_t
{
    struct
    {
	u8 r;		// red
	u8 g;		// green
	u8 b;		// blue
	u8 a;		// alpha
    };

    u8	rgba[4];	// array
    u32	val;		// combined value
}
__attribute__ ((packed)) Color_t;

#define SETCOLOR(C,R,G,B,A) (C).r=(R), (C).g=(G), (C).b=(B), (C).a=(A)

//
///////////////////////////////////////////////////////////////////////////////
///////////////			MipmapOptions_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[MipmapOptions_t]]

struct Image_t;

typedef struct MipmapOptions_t
{
    bool	valid;		// TRUE: setup of this record done
    bool	force;		// TRUE: force 'n_mipmap'
    bool	is_tpl;		// destination is TPL
    uint	n_mipmap;	// number of mipmaps
    uint	n_image;	// := n_mipmap+1
    uint	min_size;	// minimal mipmap width and height
}
MipmapOptions_t;

// both functions return 'dest', 'src' may be NULL
void SetupMipmapOptions	   ( MipmapOptions_t *mmo );
void SetupMipmapOptionsTPL ( MipmapOptions_t *mmo );
void CopyMipmapOptions     ( MipmapOptions_t *dest, const MipmapOptions_t *src );
void MipmapOptionsByImage  ( MipmapOptions_t *mmo, const struct Image_t *img );

ccp InfoMipmapOptions ( const MipmapOptions_t *mmo );
ccp TextMipmapOptions ( const MipmapOptions_t *mmo );
void PrintMipmapOptions ( FILE *f, int indent, const MipmapOptions_t *mmo );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			ImageGeometry_t			///////////////
///////////////////////////////////////////////////////////////////////////////

#define MAX_IMAGE_WIDTH  0xffff
#define MAX_IMAGE_HEIGHT 0xffff

//-----------------------------------------------------------------------------
// [[ImageGeometry_t]]

typedef struct ImageGeometry_t
{
    image_format_t	iform;		// image format

    bool		is_x;		// true: is a IMG_X_* format
    bool		is_gray;	// true: is a gray scale format
    bool		has_alpha;	// true: supports alpha/transparent infos
    bool		has_blocks;	// true: is structured in blocks

    u16			max_palette;	// >0: max number of palette entries
    bool		read_support;	// true: tools can read this format
    bool		write_support;	// true: tools can write this format

    uint		bits_per_pixel;	// number of bits per pixel
    uint		block_width;	// width of a block in pixel
    uint		block_height;	// height of a block in pixel
    uint		block_size;	// size of a block in bytes

    ccp			name;		// name of image
}
ImageGeometry_t;

//-----------------------------------------------------------------------------

const ImageGeometry_t * GetImageGeometry ( image_format_t iform );
int PaletteToImageFormat ( int pform, int return_if_invalid );

//-----------------------------------------------------------------------------

uint CalcImageSize
(
    uint		width,		// width of image in pixel
    uint		height,		// height of image in pixel

    uint		bits_per_pixel,	// number of bits per pixel
    uint		block_width,	// width of a single block
    uint		block_height,	// height of a single block

    uint		* x_width,	// not NULL: store extended width here
    uint		* x_height,	// not NULL: store extended height here
    uint		* h_blocks,	// not NULL: store number of horizontal blocks
    uint		* v_blocks	// not NULL: store number of vertical blocks
);

//-----------------------------------------------------------------------------

const ImageGeometry_t * CalcImageGeometry
(
    image_format_t	iform,		// image format
    uint		width,		// width of image in pixel
    uint		height,		// height of image in pixel

    uint		* x_width,	// not NULL: store extended width here
    uint		* x_height,	// not NULL: store extended height here
    uint		* h_blocks,	// not NULL: store number of horizontal blocks
    uint		* v_blocks,	// not NULL: store number of vertical blocks
    uint		* img_size	// not NULL: return image size
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    Image_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[Image_t]]

typedef struct Image_t
{
    //--- image data

    image_format_t	iform;		// current image format
    u8			* data;		// pointer to data or NULL if iform==IMG_INVALID
    uint		data_size;	// size of data in bytes
    bool		data_alloced;	// true: 'data' is alloced -> free() it

    uint		width;		// real width of image in pixel
    uint		height;		// real height of image in pixel
    bool		is_grayed;	// is grayed
    int			alpha_status;	// -1: alpha not used (all=0xff)
					//  0: unknown
					//  1: alpha channel used (at least one != 0xff)

    // X_GRAY+X_RGB: The image is always extended to multiple of 8*8
    // xwidth+xheight show this value and are generally = width,height
    uint		xwidth;		// extended width, only valid for IMG_X_ formats
    uint		xheight;	// extended height, only valid for IMG_X_ formats

    uint		seq_num;	// a unique number, changed for every conversion
					// copies will get the same number
    uint		conv_count;	// initialized with 0
					// it's incremented for every conversion

    //--- mipmap support

    struct Image_t	* mipmap;	// NULL or mipmap of next level


    //--- palette data

    palette_format_t	pform;		// palette format
    u8			* pal;		// NULL or pointer to palette
    uint		pal_size;	// size of 'pal' in bytes
    bool		pal_alloced;	// true: 'pal' is alloced -> free() it
    uint		n_pal;		// number of used palette entries


    //--- control data

    u8			* container;	// free it on reset
    const endian_func_t * endian;	// endian functions to access internal formats


    //--- additional image infos

    ccp			path;		// NULL or alloced filename of image
    bool		path_alloced;	// true: 'path' is alloced -> free() it
    bool		is_cup_icon;	// true: image contains cup icons
    bool		test_mode;	// true: enable test mode for cup icons (TPL only)
    file_format_t	info_fform;	// info about original file format
    image_format_t	info_iform;	// info about original image format
    palette_format_t	info_pform;	// info about original palette format
    uint		info_size;	// info about projected image size
    uint		info_n_image;	// number of images in original
    FileAttrib_t	fatt;		// file attributes


    //--- transforming results

    bool		tform_valid;	// false: the 'tform_' are invalid
    bool		tform_exec;	// true: job for ExecTransformIMG()
    bool		tform_gray;	// result is grayed
    bool		tform_noalpha;	// alpha channel must be cleard or removed
    file_format_t	tform_fform0;	// source value for transformation calculations
    image_format_t	tform_iform0;	//	"
    palette_format_t	tform_pform0;	//	"
    file_format_t	tform_fform;	// planned transformation
    image_format_t	tform_iform;	//	"
    palette_format_t	tform_pform;	//	"


    //--- patching infos

    bool		patch_done;	// true: image already patched
    PatchImage_t	patch_mode;	// only used by the patching list

} Image_t;

extern uint image_seq_num;

#define MM_COUNT(a) PRINT("MM-COUNT: %u [%s() @%u]\n", \
		CountMipmapsIMG(a),__FUNCTION__,__LINE__)
//#define MM_COUNT(a)

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void InitializeIMG ( Image_t * img );
void ResetIMG ( Image_t * img );
void RemoveContainerIMG ( Image_t * img );

///////////////////////////////////////////////////////////////////////////////

void FreeIMG
(
    Image_t		* img,		// destination image, dynamic data freed
    const Image_t	* src		// not NULL: copy parameters, but not data
);

///////////////////////////////////////////////////////////////////////////////

void FreeMipmapsIMG
(
    Image_t		* img		// not NULL: remove mipmaps
);

///////////////////////////////////////////////////////////////////////////////

uint CountMipmapsIMG
(
    const Image_t	* img		// count mipmaps
);

///////////////////////////////////////////////////////////////////////////////

void CopyIMG
(
    Image_t		* dest,		// destination image
    bool		init_dest,	// true: initialize 'dest' first
    const Image_t	* src,		// valid source image
    bool		copy_mipmap	// true: copy mipmaps too
);

///////////////////////////////////////////////////////////////////////////////

void ExtractIMG
(
    // mipmaps are deleted

    Image_t		* dest,		// destination image
    bool		init_dest,	// true: initialize 'dest' first
    const Image_t	* src,		// valid source image

    int			xbeg,		// x-index of first used pixel, robust
    int			xend,		// x-index of first not used pixel, robust
    int			ybeg,		// y-index of first used pixel, robust
    int			yend		// y-index of first not used pixel, robust
);

///////////////////////////////////////////////////////////////////////////////

void MoveIMG
(
    Image_t		* dest,		// destination image
    bool		init_dest,	// true: initialize 'dest' first
    Image_t		* src		// NULL or source image, is initialized
);

///////////////////////////////////////////////////////////////////////////////

void MoveDataIMG
(
    Image_t		* dest,		// valid destination image
    Image_t		* src		// valid source image, is resetted
);

///////////////////////////////////////////////////////////////////////////////

void AssignDataGRAY
(
    Image_t		* dest,		// valid destination
    const Image_t	* src,		// NULL or image template
    u8			* data		// with AllocDataIMG() alloced data
);

///////////////////////////////////////////////////////////////////////////////

void AssignDataRGB
(
    Image_t		* dest,		// valid destination
    const Image_t	* src,		// NULL or image template
    u8			* data		// with AllocDataIMG() alloced data
);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void ScanDataIMG
(
    Image_t		* img,		// destination image
    bool		init_img,	// true: initialize 'img' first
    const void		* data,		// data to scan
    uint		data_size	// size of 'data'
);

///////////////////////////////////////////////////////////////////////////////

enumError AssignIMG
(
    Image_t		* img,		// pointer to valid img
    int			init_img,	// <0:none, =0:reset, >0:init
    const u8		* data,		// source data
    uint		data_size,	// size of 'data'
    uint		img_index,	// index of sub image, 0:main, >0:mipmaps
    bool		mipmaps,	// true: assign mipmaps
    const endian_func_t * endian,	// endian functions to read data
    ccp			fname		// object name, assigned
);

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
);

///////////////////////////////////////////////////////////////////////////////

enumError SaveIMG
(
    Image_t		* img,		// pointer to valid img
    file_format_t	fform,		// file format
    const MipmapOptions_t *mmo,		// NULL or mipmap options
    FILE		*f,		// output file, if NULL then use fname+overwrite
    ccp			fname,		// filename of source
    bool		overwrite	// true: force overwriting
);

///////////////////////////////////////////////////////////////////////////////
// [[tpl_raw_t]]

typedef struct tpl_raw_t
{
    bool		valid;		// 'data' is valid
    mem_t		data;	        // raw data, alloced
    int			n_image;	// number of images including main image
    struct mipmap_info_t *mmi;		// 'n_images' info records, alloced
    const endian_func_t *endian;	// endian functions
}
tpl_raw_t;

void ResetRawTPL ( tpl_raw_t *raw );

enumError CreateRawTPL
(
    tpl_raw_t		* raw,		// results with alloced data
    Image_t		* src_img,	// pointer to valid source img
    file_format_t	fform		// FF_TPL or FF_TPLX
);

//-----------------------------------------------------------------------------

enumError SaveRawTPL
(
    tpl_raw_t		*raw,		// raw data created by CreateRawTPL()
    FILE		*f,		// output file, if NULL then use fname+overwrite
    ccp			fname,		// filename of source
    bool		overwrite	// true: allow overwriting
);

//-----------------------------------------------------------------------------

enumError SaveTPL
(
    Image_t		*src_img,	// pointer to valid source img
    file_format_t	fform,		// FF_TPL or FF_TPLX
    FILE		*f,		// output file, if NULL then use fname+overwrite
    ccp			fname,		// filename of source
    bool		overwrite	// true: allow overwriting
);

///////////////////////////////////////////////////////////////////////////////

enumError SaveBTI
(
    Image_t		* src_img,	// pointer to valid source img
    const MipmapOptions_t *mmo,		// NULL or mipmap options
    FILE		*f,		// output file, if NULL then use fname+overwrite
    ccp			fname,		// filename of source
    bool		overwrite	// true: force overwriting
);

///////////////////////////////////////////////////////////////////////////////

enumError SaveTEX
(
    Image_t		* src_img,	// pointer to valid source img
    const MipmapOptions_t *mmo,		// NULL or mipmap options
    FILE		*f,		// output file, if NULL then use fname+overwrite
    ccp			fname,		// filename of source
    bool		overwrite,	// true: force overwriting
    bool		ctcode_support	// true: include a CT-CODE file
);

///////////////////////////////////////////////////////////////////////////////

enumError SaveBREFTIMG
(
    Image_t		* src_img,	// pointer to valid img
    const MipmapOptions_t *mmo,		// NULL or mipmap options
    FILE		*f,		// output file, if NULL then use fname+overwrite
    ccp			fname,		// filename of source
    bool		overwrite	// true: force overwriting
);

///////////////////////////////////////////////////////////////////////////////

bool NormalizeFrameIMG
(
    Image_t		* img		// pointer to valid img
);

///////////////////////////////////////////////////////////////////////////////

int CheckAlphaIMG
(
    const Image_t	* img,		// pointer to valid img
    bool		force		// true: don't believe 'img->alpha_status'
);

///////////////////////////////////////////////////////////////////////////////

bool IsGrayIMG
(
    const Image_t	* img		// pointer to valid img
);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

image_format_t NormalizeIF
(
    image_format_t	* xform,	// not NULL: store the best IMG_X_* here
    image_format_t	iform,		// wanted image format
    image_format_t	x_default,	// default format, use if iform == AUTO
					// and return a X-format
    palette_format_t	pform		// related palette format
);

///////////////////////////////////////////////////////////////////////////////

palette_format_t NormalizePF
(
    image_format_t	iform,		// related image format
    palette_format_t	pform,		// wanted palette format
    palette_format_t	default_pform	// palette format for auto
);

///////////////////////////////////////////////////////////////////////////////

uint GetPaletteCountIF
(
    image_format_t	iform		// image format
);

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
);

///////////////////////////////////////////////////////////////////////////////

enumError ConvertToRGB
(
    Image_t		* dest_img,	// valid destination
    const Image_t	* src_img,	// valid source
    palette_format_t	pform		// wanted palette format => ignored
);

///////////////////////////////////////////////////////////////////////////////

enumError ConvertToGRAY
(
    Image_t		* dest_img,	// valid destination
    const Image_t	* src_img,	// valid source
    palette_format_t	pform		// wanted palette format => ignored
);

///////////////////////////////////////////////////////////////////////////////

enumError ConvertToPALETTE
(
    Image_t		* dest_img,	// valid destination
    const Image_t	* src_img,	// valid source
    uint		max_pal,	// maximum palette values, 0 = auto
    image_format_t	iform		// if a valid PAL format:
					//    convert to this and
					//    reduce 'max_pal' if neccessary
);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

enumError LoadPNG
(
    Image_t		* img,		// destination image
    bool		init_img,	// true: initialize 'img' first
    bool		mipmaps,	// true: load and export mipmaps

    ccp			path1,		// NULL or part #1 of path
    ccp			path2		// NULL or part #2 of path
);

///////////////////////////////////////////////////////////////////////////////

enumError ReadPNG
(
    Image_t		* img,		// destination image
    bool		mipmaps,	// true: try to load mipmaps
    File_t		* f,		// valid opened file
    u8			* cache,	// pre read data
    uint		cache_size	// size of 'cache'
);

///////////////////////////////////////////////////////////////////////////////

enumError SavePNG
(
    Image_t		* img,		// pointer to valid img
    bool		mipmaps,	// true: save mipmaps (auto file name)
    FILE		*f,		// output file, if NULL then use path1+path2
    ccp			path1,		// NULL or part #1 of path
    ccp			path2,		// NULL or part #2 of path
    int			store_alpha,	// <0:no alpha, =0:auto alpha, >0:store alpha
    bool		overwrite,	// true: force overwriting
    StringField_t	*file_list	// not NULL: store filenames of created png files
);

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
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			generic images			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[GenericImgParam_t]]

typedef struct BZ2Manager_t BZ2Manager_t;

typedef struct GenericImgParam_t
{
    //--- input, setup by CheckGenericIMG()

    mem_t		cmd_name;	// command
    mem_t		param;		// parameter string
    bool		ignore_unknown;	// ignore unknown commands and images

    //--- analysis, setup by CreateGenericIMG()

    const KeywordTab_t	*cmd;		// selected command
    int			force_width;	// !=128: finally resize image size to width given
    bool		test_mode;	// true: enable test mode for cup icons (TPL only)
    int			width;		// width of image
    int			height;		// height of image
    Color_t		color;		// default color
    BZ2Manager_t	*font;		// NULL or selected font (red_36|blue_40)
}
GenericImgParam_t;

//-----------------------------------------------------------------------------

bool CheckGenericIMG // returns TRUE if command syntax is ok
(
    GenericImgParam_t	* par,		// parameter to setup
    ccp			fname,		// filename to check
    bool		ignore_unknown	// true: ignore unknown (sub-)commands
);

//-----------------------------------------------------------------------------

enumError CreateGenericIMG
(
    GenericImgParam_t	* par,		// valid parameters
    Image_t		* img,		// pointer to valid img
    bool		init_img	// true: initialize 'img'
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////		    scan file and image format		///////////////
///////////////////////////////////////////////////////////////////////////////

int  ScanOptTransform ( ccp arg );
void DumpTransformList ( FILE * f, int indent, bool force );

ccp PrintFormat3 // print only output (internal) formats
(
    file_format_t	fform,	// file format
    image_format_t	iform,	// image format
    palette_format_t	pform	// palette format
);

///////////////////////////////////////////////////////////////////////////////

ccp PrintImageFormat // print any format, if needed as hex
(
    image_format_t	iform,	// image format
    palette_format_t	pform	// palette format
);

static inline ccp PrintFormatIMG ( const Image_t * img )
{
    DASSERT(img);
    return PrintImageFormat(img->iform,img->pform);
};

//
///////////////////////////////////////////////////////////////////////////////
///////////////		  image format transformations		///////////////
///////////////////////////////////////////////////////////////////////////////

bool Transform2InternIMG ( Image_t * img );
bool Transform2XIMG ( Image_t * img );
bool Transform2XRGB ( Image_t * img );
bool Transform2PaletteIMG ( Image_t * img );
bool Transform2NoPaletteIMG ( Image_t * img );
bool Transform2GrayIMG ( Image_t * img );
bool Transform2ColorIMG ( Image_t * img );
bool Transform2AlphaIMG ( Image_t * img );
bool Transform2NoAlphaIMG ( Image_t * img );

///////////////////////////////////////////////////////////////////////////////

bool Transform3IMG
(
    Image_t		* img,		// valid image
    file_format_t	fform,		// file format
    image_format_t	iform,		// image format
    palette_format_t	pform,		// palette format
    bool		def_base	// define *form values as base
);

///////////////////////////////////////////////////////////////////////////////

bool TransformIMG
(
    // The image is NOT converted, only the planned transformations
    // are calculated. Use ExecTransformIMG() to execute transformation
    //  -> returns TRUE if a rule match

    Image_t		* img,		// destination of transforming
    int			indent		// <0:  no logging
					// >=0: log transform with indent
);

///////////////////////////////////////////////////////////////////////////////

enumError ExecTransformIMG
(
    Image_t		* img		// image to transform
);

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
);

//-----------------------------------------------------------------------------

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
);

//-----------------------------------------------------------------------------

enumError DrawPointIMG
(
    Image_t		* img,		// image
    int			x,		// x position (robust)
    int			y,		// y position (robust)
    Color_t		col,		// draw color
    bool		combine		// true: combine 'col' with current one
);

//-----------------------------------------------------------------------------

enumError DrawHLineIMG
(
    Image_t		* img,		// image
    int			x1,		// start point (robust)
    int			x2,		// end point (not drawn, robust)
    int			y,		// common y coordinate (robust)
    Color_t		col		// draw color
);

//-----------------------------------------------------------------------------

enumError DrawVLineIMG
(
    Image_t		* img,		// image
    int			x,		// common x coordinate (robust)
    int			y1,		// start point (robust)
    int			y2,		// end point (not drawn, robust)
    Color_t		col		// draw color
);

//-----------------------------------------------------------------------------

enumError DrawFrameIMG
(
    Image_t		* img,		// image
    int			x1,		// left coordinate (robust)
    int			y1,		// top coordinate (robust)
    int			x2,		// right coordinate (robust)
    int			y2,		// bottom coordinate (robust)
    Color_t		col		// draw color
);

//-----------------------------------------------------------------------------

enumError FillRectIMG
(
    Image_t		* img,		// image
    int			x1,		// left coordinate (robust)
    int			y1,		// top coordinate (robust)
    int			x2,		// right coordinate (robust)
    int			y2,		// bottom coordinate (robust)
    Color_t		col,		// draw color
    bool		combine		// true: combine 'col' with current one
);

//-----------------------------------------------------------------------------

enumError FillHGradientIMG
(
    Image_t		* img,		// image
    int			x1,		// left coordinate (robust)
    int			y1,		// top coordinate (robust)
    int			x2,		// right coordinate (robust)
    int			y2,		// bottom coordinate (robust)
    Color_t		col1,		// draw color left
    Color_t		col2		// draw color right
);

//-----------------------------------------------------------------------------

enumError FillVGradientIMG
(
    Image_t		* img,		// image
    int			x1,		// left coordinate (robust)
    int			y1,		// top coordinate (robust)
    int			x2,		// right coordinate (robust)
    int			y2,		// bottom coordinate (robust)
    Color_t		col1,		// draw color top
    Color_t		col2		// draw color bottom
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			image resizing			///////////////
///////////////////////////////////////////////////////////////////////////////

extern bool fast_resize_enabled;

enumError ResizeIMG
(
    Image_t		* dest,		// dest image (may be same as src)
    bool		init_dest,	// true: initialize 'dest' first
    const Image_t	* src,		// source image (if NULL: use dest)
    uint		width,		// new width of image
    uint		height		// new height of image
					// if one of 'width' or 'height' is NULL
					//	-> keep aspect ratio
);

//-----------------------------------------------------------------------------

enumError FastResizeIMG
(
    Image_t		* dest,		// dest image (may be same as src)
    bool		init_dest,	// true: initialize 'dest' first
    const Image_t	* src,		// source image (if NULL: use dest)
    uint		width,		// new width of image
    uint		height		// new height of image
					// if one of 'width' or 'height' is NULL
					//	-> keep aspect ratio
);

//-----------------------------------------------------------------------------

enumError SmartResizeIMG
(
    Image_t		* dest,		// dest image (may be same as src)
    bool		init_dest,	// true: initialize 'dest' first
    const Image_t	* src,		// source image (if NULL: use dest)
    uint		width,		// new width of image
    uint		height		// new height of image
					// if one of 'width' or 'height' is NULL
					//	-> keep aspect ratio
);

//-----------------------------------------------------------------------------

enumError HalfIMG
(
    Image_t		* img		// source and dest
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			CMPR interface			///////////////
///////////////////////////////////////////////////////////////////////////////

#define CMPR_MAX_COL 16
#define CMPR_DATA_SIZE (4*CMPR_MAX_COL)

//-----------------------------------------------------------------------------
// [[cmpr_info_t]]

typedef struct cmpr_info_t
{
    ccp		name;			// name of method
    uint	opaque_count;		// number of opaque (=non transparent) pixels
    u8		p[4][4];		// 4 calculated values
					// p[0] + p[1]: values
					// if opaque_count < 16: p[2] == p[3]
    u8		default_vector[8];	// init by 'opt_cmpr_def'

    //--- statistics, only valid if 'fill_info' set

    u8		index[CMPR_MAX_COL];	// color index of each value
    uint	dist[CMPR_MAX_COL];	// distance of each value
    uint	total_dist;		// sum of all distances

    u8		cmpr[8];		// compressed data
    u64		usec;			// time of calculation

} cmpr_info_t;

///////////////////////////////////////////////////////////////////////////////

static inline void InitializeCmprInfo ( cmpr_info_t *info )
{
    DASSERT(info);
    memset(info,0,sizeof(*info));
    memcpy(info->default_vector,opt_cmpr_def,sizeof(info->default_vector));
};

//-----------------------------------------------------------------------------

void CMPR_wiimm
(
    const u8		*data,		// source data
    cmpr_info_t		*info		// info data structure
);

//-----------------------------------------------------------------------------

void CMPR_close_info
(
    const u8		*data,		// source data
    cmpr_info_t		*info,		// info data structure
    u8			*dest,		// store destination data here (never 0)
    bool		fill_info	// true: fill info with statistics
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			patch interface			///////////////
///////////////////////////////////////////////////////////////////////////////

bool ScanPatchImage ( ccp arg, PatchImage_t * mode );
int ScanOptPatchImage ( ccp arg );

///////////////////////////////////////////////////////////////////////////////

bool SetupPatchListIMG(); // returns true if patching enabled
void ResetPatchListIMG();

enumError PatchListIMG
(
    Image_t		* img		// image to patch
);

enumError PatchIMG
(
    Image_t		* dest_img,	// destination image, can be one of the sources
    Image_t		* src_img1,	// base source
    Image_t		* src_img2,	// patch source
    PatchImage_t	patch_mode	// patch mode
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			image lists			///////////////
///////////////////////////////////////////////////////////////////////////////

void PrintImageHead
(
    int			line_indent,	// indention of complete line
    uint		long_count	// set the verbosity
);

//-----------------------------------------------------------------------------

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
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			median cut			///////////////
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
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    TPL support			///////////////
///////////////////////////////////////////////////////////////////////////////

#define TPL_MAGIC_NUM		0x0020AF30
#define TPL_EX_MAGIC_NUM	0x54504c78

///////////////////////////////////////////////////////////////////////////////
// [[tpl_header_t]]

typedef struct tpl_header_t
{
// [[tpl-ex+]]
    u8			magic[4];	// TPL_MAGIC_NUM
    u32			n_image;	// number of images
    u32			imgtab_off;	// image table offset relative to file start
}
__attribute__ ((packed)) tpl_header_t;

///////////////////////////////////////////////////////////////////////////////
// [[tpl_header_ex_t]]

typedef struct tpl_header_ex_t
{
// [[tpl-ex+]]
    u8			magic[4];	// TPL_MAGIC_NUM
    u32			n_image;	// number of images
    u32			imgtab_off;	// image table offset relative to file start

    u32			ex_magic;	// always TPL_EX_MAGIC_NUM
    u32			ex_width;	// real (icon) width
    u32			ex_height;	// real (icon) height
    u32			ex_n_icon;	// number of icons
}
__attribute__ ((packed)) tpl_header_ex_t;

//-----------------------------------------------------------------------------

static inline bool IsTplHeaderEx ( const void *data, uint data_size )
{
    const tpl_header_ex_t *tpl = (tpl_header_ex_t*)data;
    return data_size >= sizeof(tpl_header_ex_t)
	&& be32(&tpl->imgtab_off) >= sizeof(tpl_header_ex_t)
	&& be32(&tpl->ex_magic) == TPL_EX_MAGIC_NUM;
}

///////////////////////////////////////////////////////////////////////////////
// [[tpl_imgtab_t]]

typedef struct tpl_imgtab_t
{
    u32			image_off;	// image offset relative to file start
    u32			palette_off;	// palette offset relative to file start
}
__attribute__ ((packed)) tpl_imgtab_t;

///////////////////////////////////////////////////////////////////////////////
// [[tpl_pal_header_t]]

typedef struct tpl_pal_header_t
{
    u16			n_entry;	// number of palette entries
    u16			unknown1;
    u32			pform;		// image format of palette
    u32			data_off;	// data offset relative to tpl_header_t
}
__attribute__ ((packed)) tpl_pal_header_t;

///////////////////////////////////////////////////////////////////////////////
// [[tpl_img_header_t]]

typedef struct tpl_img_header_t
{
// [[tpl-ex+]]
    u16			height;		// height of image in pixel
    u16			width;		// width of image in pixel
    u32			iform;		// image format
    u32			data_off;	// data offset relative to tpl_header_t

    // the following is unknown
    // names taken from http://hitmen.c02.at/files/yagcd/yagcd/chap15.html#sec15.35

    u32			wrap_s;		// value 0 or 1
    u32			wrap_t;		// value 0 or 1
    u32			min_filter;	// always 1
    u32			mag_filter;	// always 1
    float32		load_bias;	// always 0
    u8			edge_load;	// always 0
    u8			min_load;	// always 0
    u8			max_load;	// always 0
    u8			unpacked;	// always 0
}
__attribute__ ((packed)) tpl_img_header_t;

///////////////////////////////////////////////////////////////////////////////

uint GetNImagesTPL
(
    const u8			* data,		// TPL data
    uint			data_size,	// size of tpl data
    const endian_func_t		* endian	// endian functions
);

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
);

///////////////////////////////////////////////////////////////////////////////
// [[tpl_signature_t]]

typedef struct tpl_signature_t
{
    u8		magic1[8];		// always "LE-CODE\0"
    u32		width;			// width of image
    u32		height;			// height of image
    u32		n_icon;			// number of icons
    u16		iform;			// image format
    u16		pform;			// palette format
    u8		magic2[8];		// always "Cup Icon"
}
__attribute__ ((packed)) tpl_signature_t;

extern const tpl_signature_t TPLSignature0;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    BTI support			///////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct bti_header_t
{
    // --> unknown field names taken from http://www.amnoid.de/gc/bti.txt

    /* 00 */  u8	iform;		// image format
    /* 01 */  u8	unknown_01;	// 02 for 'posteffect.bti', 00 else
    /* 02 */  u16	width;		// width of image in pixel
    /* 04 */  u16	height;		// height of image in pixel

    /* 06 */  u8	wrap_s;		// 01 for 'posteffect.bti', 00 else
    /* 07 */  u8	wrap_t;		// 01 for 'posteffect.bti', 00 else

    /* 08 */  u16	pform;		// image format of palette
    /* 0a */  u16	n_pal;		// number of palette entries
    /* 0c */  u32	pal_off;	// palette offset relative file header

    /* 10 */  u32	unknown_10;	// always 0 in MKWii
    /* 14 */  u8	unknown_14;	// always 1 in MKWii
    /* 15 */  u8	unknown_15;	// always 1 in MKWii
    /* 16 */  u16	unknown_16;	// always 0 in MKWii
    /* 18 */  u8	n_image;	// number of images (main+mipmaps)
    /* 19 */  u8	unknown_19;	// always 0 in MKWii
    /* 1a */  u16	unknown_1a;	// always 0 in MKWii

    /* 1c */  u32	data_off;	// offset of image data relative file header
    /* 20 */
}
__attribute__ ((packed)) bti_header_t;

///////////////////////////////////////////////////////////////////////////////

uint GetNImagesBTI
(
    const u8			* data,		// bti data
    uint			data_size	// size of bti data
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////

#endif // SZS_LIB_IMAGE_H

