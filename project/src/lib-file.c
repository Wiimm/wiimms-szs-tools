
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

#define _GNU_SOURCE 1

#include <sys/types.h>
#include <sys/ioctl.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <utime.h>
#include <dirent.h>

#include "lib-std.h"
#include "lib-szs.h"
#include "lib-rarc.h"
#include "lib-pack.h"
#include "lib-brres.h"
#include "lib-breff.h"
#include "lib-xbmg.h"
#include "lib-kcl.h"
#include "lib-kmp.h"
#include "lib-rkc.h"
#include "lib-rkg.h"
#include "lib-image.h"
#include "lib-staticr.h"
#include "lib-ctcode.h"
#include "lib-lecode.h"
#include "lib-common.h"
#include "lib-bzip2.h"
#include "lib-checksum.h"

//
///////////////////////////////////////////////////////////////////////////////
///////////////			cygwin support			///////////////
///////////////////////////////////////////////////////////////////////////////

#ifdef __CYGWIN__

 void NormalizeFilenameParam ( ParamList_t *param )
 {
    if ( param->arg && *param->arg )
    {
	char *res = AllocNormalizedFilenameCygwin(param->arg);
	FREE(param->arg);
	param->arg = res;
    }
 }

#endif // __CYGWIN__

///////////////////////////////////////////////////////////////////////////////

void SetSource ( ccp source )
{
 #ifdef __CYGWIN__
    opt_source = IsWindowsDriveSpec(source)
		? AllocNormalizedFilenameCygwin(source)
		: source;
 #else
    opt_source = source;
 #endif

    //if ( source && *source )
	AppendStringField(&source_list,source,false);
    noPRINT("SetSource(%s) N=%u/%u\n",source,source_list.used,source_list.size);
}

///////////////////////////////////////////////////////////////////////////////

void SetDest ( ccp dest, bool mkdir )
{
 #ifdef __CYGWIN__
    opt_dest = IsWindowsDriveSpec(dest)
		? AllocNormalizedFilenameCygwin(dest)
		: dest;
 #else
    opt_dest = dest;
 #endif
    opt_mkdir = mkdir;
}

///////////////////////////////////////////////////////////////////////////////

void CheckOptDest ( ccp default_dest, bool default_mkdir )
{
    if (!opt_dest)
    {
	opt_dest = default_dest;
	if (!opt_mkdir)
	    opt_mkdir = default_mkdir;
    }

    if ( opt_dest && opt_dest[0] == '-' && !opt_dest[1] )
	stdlog = stderr;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			struct File_t			///////////////
///////////////////////////////////////////////////////////////////////////////

static bool is_seekable
(
    File_t		* F		// file structure
)
{
    mode_t mode = F->st.st_mode;
    F->is_seekable = ( S_ISREG(mode) || S_ISBLK(mode) || S_ISCHR(mode) )
		&& F->st.st_size
		&& lseek(fileno(F->f),0,SEEK_SET) != (off_t)-1;
    return F->is_seekable;
}

///////////////////////////////////////////////////////////////////////////////

enumError OpenFILE
(
    File_t		* f,		// file structure
    bool		initialize,	// true: initialize 'f'
    ccp			fname,		// file to open
    bool		ignore_no_file,	// ignore if file does not exists
					// and return warning ERR_NOT_EXISTS
    bool		ignore_limit	// ignore opt_max_file_size
)
{
    FileMode_t fmode = FM_STDIO;
    if (ignore_no_file)
	fmode |= FM_IGNORE;
    return OpenFile( f,initialize, fname,fmode,
	ignore_limit ? 0 : opt_max_file_size,
	"You can change this security limit with --max-file-size=MiB.\n" );
}

///////////////////////////////////////////////////////////////////////////////

enumError CreateFILE
(
    File_t		* f,		// file structure
    bool		initialize,	// true: initialize 'f'
    ccp			fname,		// file to open
    bool		testmode,	// true: don't open
    bool		create_path,	// create path if necessary
    bool		overwrite,	// overwrite existing files without warning
    bool		remove_dest,	// remove existing file before creating
    bool		update		// don't create new files
)
{
    FileMode_t fmode = FM_STDIO;
    if (testmode)
	fmode |= FM_TEST;
    if (create_path)
	fmode |= FM_MKDIR;
    if (overwrite)
	fmode |= FM_OVERWRITE;
    if (opt_number)
	fmode |= FM_NUMBER;
    if (remove_dest)
	fmode |= FM_REMOVE;
    if (update)
	fmode |= FM_UPDATE;
    return CreateFile(f,initialize,fname,fmode);
}

///////////////////////////////////////////////////////////////////////////////

FileMode_t GetFileModeByOpt
(
    int			testmode,	// test mode
    ccp			fname1,		// NULL or dest filename
    ccp			fname2		// NULL or source filename
)
{
    FileMode_t fmode = FM_STDIO;

    if ( testmode )		fmode |= FM_TEST;
    if ( opt_mkdir )		fmode |= FM_MKDIR;
    if ( opt_number )		fmode |= FM_NUMBER;
    if ( opt_remove_dest )	fmode |= FM_REMOVE;
    if ( opt_update )		fmode |= FM_UPDATE;

    if ( opt_overwrite || fname1 && *fname1 && fname2 && !strcmp(fname1,fname2) )
	fmode |= FM_OVERWRITE;

    return fmode;
}

///////////////////////////////////////////////////////////////////////////////

enumError CreateFileOpt
(
    File_t		* f,		// file structure
    bool		initialize,	// true: initialize 'f'
    ccp			fname,		// file to open
    bool		testmode,	// true: don't open
    ccp			srcfile		// NULL or src file name
					// if fname == srcfile: overwrite enabled
)
{
    DASSERT(f);
    DASSERT(fname);

    FileMode_t fmode = GetFileModeByOpt(testmode,fname,srcfile) | FM_TOUCH;
    return CreateFile(f,initialize,fname,fmode);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

enumError AppendFILE
(
    File_t		* f,		// file structure
    bool		initialize,	// true: initialize 'f'
    ccp			fname,		// file to open
    bool		testmode,	// true: don't open
    bool		create_path	// create path if necessary
)
{
    FileMode_t fmode = FM_STDIO | FM_APPEND;
    if (testmode)
	fmode |= FM_TEST;
    if (create_path)
	fmode |= FM_MKDIR;
    return CreateFile(f,initialize,fname,fmode);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FILE * CreateAnalyzeFile()
{
    static bool done = false;
    if (done)
	return analyze_file;
    done = true;

    if ( !analyze_fname || !*analyze_fname )
	return 0;

    if (!strcmp(analyze_fname,"-"))
	analyze_file = stdout;
    else
	analyze_file = fopen(analyze_fname,"w");
    if (!analyze_file)
	ERROR1(ERR_CANT_CREATE,"Can't create analyse log file: %s\n",analyze_fname);

    return analyze_file;
}

///////////////////////////////////////////////////////////////////////////////

void CloseAnalyzeFile()
{
    if ( analyze_file && analyze_file != stdout )
	fclose(analyze_file);
    analyze_file = 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

char * SplitSubPath
(
    char		* buf,		// destination buffer
    size_t		buf_size,	// size of 'buf'
    ccp			path		// source path, if NULL: use 'buf'
)
{
    char * end = path && path != buf
			? StringCopyS(buf,buf_size,path)
			: buf + strlen(buf);

    if ( end >= buf + buf_size - 2 )
    {
	end = buf + buf_size - 2;
	*end = 0;
    }

    struct stat st;
    if (!stat(buf,&st))
	return end;

    char * ptr = end;
    while ( ptr > buf )
    {
	ptr--;
	while ( ptr > buf && *ptr != '/' && *ptr != '\\' )
	    ptr--;
	if ( ptr == buf )
	    break;

	*ptr = 0;
	noPRINT("TEST: |%s|\n",buf);
	if (!stat(buf,&st))
	{
	    noPRINT("FOUND: |%s|\n",buf);
	    if (S_ISREG(st.st_mode))
	    {
		memmove(ptr+1,ptr,end-ptr+1);
		*++ptr = '/';
		return ptr;
	    }
	    *ptr = '/';
		return 0;
	}
	*ptr = '/';
    }
    return 0;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			more file support		///////////////
///////////////////////////////////////////////////////////////////////////////

int ScanOptChdir ( ccp arg )
{
    PRINT("CHDIR: %s\n",arg);
    if ( !arg || !*arg || !chdir(arg) )
	return 0;
    ERROR1(ERR_CANT_OPEN,"Can't change dir: %s\n",arg);
    return 1;
}

///////////////////////////////////////////////////////////////////////////////

enumError OpenReadFILE
(
    ccp			path1,		// NULL or part #1 of path
    ccp			path2,		// NULL or part #2 of path
    bool		silent,		// suppress all error messages
    u8			** res_data,	// store alloced data here
    uint		* res_size,	// not NULL: store data size
    ccp			* res_fname,	// not NULL: store alloced filename
    FileAttrib_t	* res_fatt	// not NULL: store file attributes
)
{
    DASSERT( path1 || path2 );
    DASSERT(res_data);

    *res_data = 0;
    if (res_size)
	*res_size = 0;
    if (res_fname)
	*res_fname = 0;
    if (res_fatt)
	memset(res_fatt,0,sizeof(*res_fatt));

    char pathbuf[PATH_MAX];
    ccp path = PathCatPP(pathbuf,sizeof(pathbuf),path1,path2);
    PRINT("OpenReadFILE(%s,%d)\n",path,silent);

    File_t F;
    enumError err = OpenFILE(&F,true,path,silent,false);
    if (err)
	return err;

    uint data_size = F.st.st_size;
    if (!F.is_seekable)
	data_size = opt_max_file_size;
    u8 *data = MALLOC(data_size+1);

    size_t read_size = fread(data,1,data_size,F.f);
    if ( read_size && !F.is_seekable )
    {
	PRINT("non seekable read: %zu\n",read_size);
	data_size = read_size;
	data = REALLOC(data,data_size+1);
    }
    data[data_size] = 0; // termination for text files

    if ( read_size != data_size )
    {
	if (!silent)
	    FILEERROR1(&F,ERR_READ_FAILED,"Read failed: %s\n",path);
	ResetFile(&F,false);
	FREE(data);
	return ERR_READ_FAILED;
    }


    *res_data = data;
    if (res_size)
	*res_size = data_size;
    if (res_fname)
    {
	*res_fname = F.fname;
	F.fname = EmptyString;
    }
    if (res_fatt)
	SetFileAttrib(res_fatt,&F.fatt,0);

    ResetFile(&F,false);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

enumError LoadFILE
(
    ccp			path1,		// NULL or part #1 of path
    ccp			path2,		// NULL or part #2 of path
    size_t		skip,		// skip num of bytes before reading
    void		* data,		// destination buffer, size = 'size'
    size_t		size,		// size to read
    int			silent,		// 0: print all error messages
					// 1: suppress file size warning
					// 2: suppress all error messages
    FileAttrib_t	* fatt,		// not NULL: store file attributes
    bool		fatt_max	// true: store *max* values to 'fatt'
)
{
    ASSERT(data);
    if (!size)
	return ERR_OK;

    char pathbuf[PATH_MAX];
    ccp path = PathCatPP(pathbuf,sizeof(pathbuf),path1,path2);
    TRACE("LoadFILE(%s,%zu,%zu,%d)\n",path,skip,size,silent);

    if (fatt)
    {
	struct stat st;
	if (!stat(path,&st))
	    UseFileAttrib(fatt,0,&st,fatt_max);
    }

    FILE * f = fopen(path,"rb");
    if (!f)
    {
	if (silent<2)
	    ERROR1(ERR_CANT_OPEN,"Can't open file: %s\n",path);
	return ERR_CANT_OPEN;
    }

    if ( skip > 0 )
	fseek(f,skip,SEEK_SET);

    size_t read_stat = fread(data,1,size,f);
    fclose(f);

    if ( read_stat == size )
	return ERR_OK;

    enumError err = ERR_READ_FAILED;
    if ( silent == 1 && size > 0 )
	err = ERR_WARNING;
    else if ( silent < 2 )
	ERROR1(ERR_READ_FAILED,"Can't read file: %s\n",path);

    noPRINT("D=%p, s=%zu/%zu: %s\n",data,read_stat,size,path);
    if ( read_stat >= 0 && read_stat < size )
	memset((char*)data+read_stat,0,size-read_stat);

    return err;
}

///////////////////////////////////////////////////////////////////////////////

enumError SaveFILE
(
    ccp			path1,		// NULL or part #1 of path
    ccp			path2,		// NULL or part #2 of path
    bool		overwrite,	// true: force overwriting
    const void		* data,		// data to write
    uint		data_size,	// size of 'data'
    FileAttrib_t	* fatt		// not NULL: set timestamps using this attribs
)
{
    DASSERT(data);

    char pathbuf[PATH_MAX];
    ccp path = PathCatPP(pathbuf,sizeof(pathbuf),path1,path2);
    TRACE("SaveFILE(%d) %s\n",overwrite,path);

    File_t F;
    enumError err = CreateFileOpt(&F,true,path,testmode, overwrite ? path : 0 );
    if ( err || !F.f )
    {
	ResetFile(&F,0);
	return err;
    }

    const uint written = fwrite(data,1,data_size,F.f);
    if ( written != data_size )
    {
	ERROR1(ERR_WRITE_FAILED,"Write to file failed: %s\n",path);
	ResetFile(&F,0);
	return ERR_WRITE_FAILED;
    }

    if (fatt)
	memcpy(&F.fatt,fatt,sizeof(F.fatt));
    return ResetFile(&F,true);
}

///////////////////////////////////////////////////////////////////////////////

uint IsolateFilename
(
    // return length of found and copied string

    char		* buf,	    // destination buffer
    size_t		bufsize,    // size of 'buf'
    ccp			path,	    // path to analyze
    file_format_t	fform	    // not NULL: remove related file extension
)
{
    DASSERT(buf);
    DASSERT(path);

    uint len;
    path = FindFilename(path,&len);
    if ( len >= bufsize )
	len = bufsize-1;
    memcpy(buf,path,len);
    buf[len] = 0;
    if (fform)
    {
	ccp ext = GetExtFF(0,fform);
	uint elen = strlen(ext);
	char *bufext = buf+len-elen;
	if ( elen < len && !strcasecmp(ext,bufext) )
	{
	    *bufext = 0;
	    len = bufext - buf;
	}
	else
	{
	    ext = GetMagicExtFF(0,fform);
	    elen = strlen(ext);
	    bufext = buf+len-elen;
	    if ( elen < len && !strcasecmp(ext,bufext) )
	    {
		*bufext = 0;
		len = bufext - buf;
	    }
	}
    }
    return len;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

enum { N_NAME_BUF = 16, NAME_BUF_SIZE = 24 };

//-----------------------------------------------------------------------------

static char *GetNameBuf()
{
    static char buffers[N_NAME_BUF][NAME_BUF_SIZE];
    static int buf_idx = 0;

    if ( --buf_idx < 0 )
	buf_idx = N_NAME_BUF - 1;
    return buffers[buf_idx];
}

//-----------------------------------------------------------------------------

ccp GetNameFF ( file_format_t ff1, file_format_t ff2 )
{
    if ( ff1 == FF_DIRECTORY || IsCompressedFF(ff1) )
    {
	ccp prefix = FileTypeTab[ff1].name;
	if ( !ff2 || (uint)ff2 >= FF_N || ff2 == ff1 )
	{
	    noPRINT("GetNameFF(%u,%u) -> '%s' [ff1]\n",ff1,ff2,prefix);
	    return prefix;
	}

	char *buf = GetNameBuf();
	snprintf(buf,NAME_BUF_SIZE,"%s.%s",prefix,FileTypeTab[ff2].name);
	noPRINT("GetNameFF(%u,%u) -> '%s' [1+2]\n",ff1,ff2,prefix);
	return buf;
    }

    noPRINT("GetNameFF(%u,%u) -> '%s' [tab]\n", ff1, ff2,
	FileTypeTab[ ff2 && (uint)ff2 < FF_N ? ff2 : (uint)ff1 < FF_N ? ff1 : 0].name );
    return FileTypeTab[ ff2 && (uint)ff2 < FF_N ? ff2 : (uint)ff1 < FF_N ? ff1 : 0].name;
}

//-----------------------------------------------------------------------------

ccp GetNameFFv ( file_format_t ff1, file_format_t ff2, int version )
{
    ccp name = GetNameFF(ff1,ff2);
    if ( version > 0 )
    {
	char *buf = GetNameBuf();
	snprintf(buf,NAME_BUF_SIZE,"%s.v%u",name,version);
	name = buf;
    }
    return name;
}

///////////////////////////////////////////////////////////////////////////////

ccp GetExtFF ( file_format_t ff1, file_format_t ff2 )
{
    if ( IsCompressedFF(ff1) && ff1 != FF_BZ2 )
    {
	return ff1 == FF_BZ
		? (  ff2 == FF_WU8 ? ".wbz" : ".bz" )
		: (uint)ff2 < FF_N ? FileTypeTab[ff2].ext_compr : ".szs";
    }

    if ( ff1 <= FF_UNKNOWN || ff1 >= FF_N )
	ff1 = ff2;

    return (uint)ff1 < FF_N
		? FileTypeTab[ff1].ext
		: ".bin";
}

//-----------------------------------------------------------------------------

ccp GetBinExtFF ( file_format_t ff )
{
    if ( ff > FF_UNKNOWN && ff < FF_N && FileTypeTab[ff].fform_bin )
	return GetExtFF(0,FileTypeTab[ff].fform_bin);

    return ".bin";
}

//-----------------------------------------------------------------------------

ccp GetTextExtFF ( file_format_t ff )
{
    if ( ff > FF_UNKNOWN && ff < FF_N && FileTypeTab[ff].fform_txt )
	return GetExtFF(0,FileTypeTab[ff].fform_txt);

    return ".txt";
}

//-----------------------------------------------------------------------------

ccp GetMagicStringFF ( file_format_t ff )
{
    return ff > FF_UNKNOWN && ff < FF_N && FileTypeTab[ff].magic_len
	? (ccp)(FileTypeTab[ff].magic)
	: 0;
}

///////////////////////////////////////////////////////////////////////////////

file_format_t GetBinFF ( file_format_t ff )
{
    return ff > FF_UNKNOWN && ff < FF_N && FileTypeTab[ff].fform_bin
	? FileTypeTab[ff].fform_bin
	: FF_UNKNOWN;
}

//-----------------------------------------------------------------------------

file_format_t GetTextFF ( file_format_t ff )
{
    return ff > FF_UNKNOWN && ff < FF_N && FileTypeTab[ff].fform_txt
	? FileTypeTab[ff].fform_txt
	: FF_UNKNOWN;
}

///////////////////////////////////////////////////////////////////////////////

ccp GetMagicExtFF ( file_format_t ff1, file_format_t ff2 )
{
    if (IsCompressedFF(ff1))
	return (uint)ff2 < FF_N ? FileTypeTab[ff2].ext_compr : ".szs";

    if ( ff1 <= FF_UNKNOWN || ff1 >= FF_N )
	ff1 = ff2;

    return (uint)ff1 < FF_N
		? FileTypeTab[ff1].ext_magic
		: ".bin";
}

///////////////////////////////////////////////////////////////////////////////
//  [[GetByMagicFF]]

file_format_t GetByMagicFF
(
    const void	*data,	    // pointer to data
    uint	data_size,  // size of data
    uint	file_size   // NULL or total size of file
)
{
    if ( !data || !data_size )
	return FF_UNKNOWN;

    if ( file_size && data_size > file_size )
	data_size = file_size;
    if (!file_size)
	file_size = data_size;

    const u8 *data8 = (u8*)data;
    if ( data_size >= 8 )
    {
	if ( IsBZIP2(data,data_size) )
	    return FF_BZ2;

	const u64 magic64 = be64(data);
	switch(magic64)
	{
	    case BMG_MAGIC8_NUM:		return FF_BMG;
	    case PNG_MAGIC8_NUM:		return FF_PNG;
	    case GCT_MAGIC8_NUM:		return FF_GCT;
	    case LE_LPAR_MAGIC_NUM:		return FF_LPAR;

	    //--- see below for BOM support
	    case CT_TEXT_MAGIC8_NUM:		return FF_CT_TEXT;
	    case DISTRIB_MAGIC8_NUM:		return FF_DISTRIB;
	    case ITEMSLT_TEXT_MAGIC_NUM:	return FF_ITEMSLT_TXT;
	    case OBJFLOW_TEXT_MAGIC8_NUM:	return FF_OBJFLOW_TXT;
	    case GH_ITEM_TEXT_MAGIC8_NUM:	return FF_GH_ITEM_TXT;
	    case GH_IOBJ_TEXT_MAGIC8_NUM:	return FF_GH_IOBJ_TXT;
	    case GH_KART_TEXT_MAGIC8_NUM:	return FF_GH_KART_TXT;
	    case GH_KOBJ_TEXT_MAGIC8_NUM:	return FF_GH_KOBJ_TXT;
	}
    }

    if ( data_size >= 4 )
    {
	const u32 magic32 = be32(data);
	switch(magic32)
	{
	    //---  see below for BOM support
	    case BMG_TEXT_MAGIC_NUM:	return FF_BMG_TXT;
	    case KCL_TEXT_MAGIC_NUM:	return FF_KCL_TXT;
	    case KMP_TEXT_MAGIC_NUM:	return FF_KMP_TXT;
	    case MDL_TEXT_MAGIC_NUM:	return FF_MDL_TXT;
	    case PAT_TEXT_MAGIC_NUM:	return FF_PAT_TXT;
	    case KMG_TEXT_MAGIC_NUM:	return FF_KMG_TXT;
	    case KRT_TEXT_MAGIC_NUM:	return FF_KRT_TXT;
	    case KRM_TEXT_MAGIC_NUM:	return FF_KRM_TXT;

	    case YAZ0_MAGIC_NUM:	return FF_YAZ0;
	    case YAZ1_MAGIC_NUM:	return FF_YAZ1;
	    case XYZ0_MAGIC_NUM:	return FF_XYZ;
	    case BZ_MAGIC_NUM:		return FF_BZ;
	    case GCH_MAGIC_NUM:		return FF_GCH;
	    case WCH_MAGIC_NUM:		return FF_WCH;
	    case WPF_MAGIC_NUM:		return FF_WPF;
	    case XPF_MAGIC_NUM:		return FF_XPF;
	    case RKG_MAGIC_NUM:		return FF_RKG;

	    case KMG_MAGIC_NUM:		return FF_KMG;
	    case KRT_MAGIC_NUM:		return FF_KRT;
	    case KRM_MAGIC_NUM:		return FF_KRM;
// [[obj-type+]] 3x
//	    case OBJFLOW_MAGIC_NUM:	return FF_OBJFLOW;
//	    case GH_ITEM_MAGIC_NUM:	return FF_GH_ITEM;
//	    case GH_KART_MAGIC_NUM:	return FF_GH_KART;

	    case U8_MAGIC_NUM:		return FF_U8;
	    case WU8_MAGIC_NUM:		return FF_WU8;
	    case RARC_MAGIC_NUM:	return FF_RARC;
	    case RKCT_MAGIC_NUM:	return FF_RKC;
	    case RKCO_MAGIC_NUM:	return FF_RKCO;
	    case PACK_MAGIC_NUM:	return FF_PACK;

	    case BRRES_MAGIC_NUM:	return FF_BRRES;

	    case TPL_MAGIC_NUM:		return FF_TPL;
	    case KMP_MAGIC_NUM:		return FF_KMP;
	    case BRASD_MAGIC_NUM:	return FF_BRASD;

	    case LE_BINARY_MAGIC_NUM:	return FF_LE_BIN;
	    case LEX_BIN_MAGIC_NUM:	return FF_LEX;
	    case LEX_TEXT_MAGIC_NUM:	return FF_LEX_TXT;

	    case CT0_CODE_MAGIC_NUM:	return FF_CT0_CODE;
	    case CT0_DATA_MAGIC_NUM:	return FF_CT0_DATA;
	    case CT1_CODE_MAGIC_NUM:	return FF_CT1_CODE;
	    case CT1_DATA_MAGIC_NUM:	return FF_CT1_DATA;

	    case CT_CUP1_MAGIC_NUM:	return FF_CUP1;
	    case CT_CRS1_MAGIC_NUM:	return FF_CRS1;
	    case CT_MOD1_MAGIC_NUM:	return FF_MOD1;
	    case CT_MOD2_MAGIC_NUM:	return FF_MOD2;
	    case CT_OVR1_MAGIC_NUM:	return FF_OVR1;

	    case BREFF_MAGIC_NUM:
		if ( file_size >= 0x20 )
		    return data_size < 0x14 || !memcmp(data+0x10,BREFF_MAGIC,4)
				? FF_BREFF : FF_UNKNOWN;
		break;

	    case BREFT_MAGIC_NUM:
		if ( file_size >= 0x20 )
		    return data_size < 0x14 || !memcmp(data+0x10,BREFT_MAGIC,4)
				? FF_BREFT : FF_UNKNOWN;
		break;
	}

	if ( data_size >= 0x20 )
	{
	    // size => avoid BRSUB.header detection
	    switch(magic32)
	    {
		case CHR_MAGIC_NUM:	return FF_CHR;
		case CLR_MAGIC_NUM:	return FF_CLR;
		case MDL_MAGIC_NUM:	return FF_MDL;
		case PAT_MAGIC_NUM:	return FF_PAT;
		case SCN_MAGIC_NUM:	return FF_SCN;
		case SHP_MAGIC_NUM:	return FF_SHP;
		case SRT_MAGIC_NUM:	return FF_SRT;
		case TEX_MAGIC_NUM:
		    return IsValidTEXCT(data,data_size,0,0,0) < VALID_ERROR
				    ? FF_TEX_CT : FF_TEX;
	    }
	}
    }

    const uint bom_len = GetTextBOMLen(data,data_size);
    if ( bom_len > 0 )
    {
	if ( data_size >= 8 + bom_len )
	{
	    const u64 magic64 = be64(data+bom_len);
	    switch(magic64)
	    {
		case CT_TEXT_MAGIC8_NUM:	return FF_CT_TEXT;
		case DISTRIB_MAGIC8_NUM:	return FF_DISTRIB;
		case OBJFLOW_TEXT_MAGIC8_NUM:	return FF_OBJFLOW_TXT;
// [[obj-type+]]
		case GH_ITEM_TEXT_MAGIC8_NUM:	return FF_GH_ITEM_TXT;
		case GH_IOBJ_TEXT_MAGIC8_NUM:	return FF_GH_IOBJ_TXT;
		case GH_KART_TEXT_MAGIC8_NUM:	return FF_GH_KART_TXT;
		case GH_KOBJ_TEXT_MAGIC8_NUM:	return FF_GH_KOBJ_TXT;
	    }
	}

	if ( data_size >= 4 + bom_len )
	{
	    const u32 magic32 = be32(data+bom_len);
	    switch(magic32)
	    {
		case BMG_TEXT_MAGIC_NUM:	return FF_BMG_TXT;
		case KCL_TEXT_MAGIC_NUM:	return FF_KCL_TXT;
		case KMP_TEXT_MAGIC_NUM:	return FF_KMP_TXT;
		case MDL_TEXT_MAGIC_NUM:	return FF_MDL_TXT;
		case PAT_TEXT_MAGIC_NUM:	return FF_PAT_TXT;
		case KMG_TEXT_MAGIC_NUM:	return FF_KMG_TXT;
		case KRT_TEXT_MAGIC_NUM:	return FF_KRT_TXT;
		case KRM_TEXT_MAGIC_NUM:	return FF_KRM_TXT;
	    }
	}
    }


    //--- test for FF_OBJFLOW and FF_GH_*

    object_type_t ot;
    if (ScanObjType(&ot,data,data_size,file_size))
    {
	PRINT("GetByMagicFF(0x%x,0x%x) = %s\n",data_size,file_size,LogObjType(&ot));
	return ot.fform;
    }

    //--- test for FF_BREFT_IMG

    if ( data_size >= sizeof(breft_image_t) && !be32(data) )
    {
	const breft_image_t * bi = (breft_image_t*)data;
	const ImageGeometry_t * geo = GetImageGeometry(bi->iform);
	if ( geo && !geo->is_x )
	{
	    const uint width	= be16(&bi->width);
	    const uint xwidth	= ( width + geo->block_width - 1 )
				/ geo->block_width * geo->block_width;
	    const uint height = be16(&bi->height);
	    const uint xheight	= ( height + geo->block_height - 1 )
				/ geo->block_height * geo->block_height;
	    const uint size   = be32(&bi->img_size);
	    noPRINT("RAW IMG FOUND: %02x, %u*%u -> %u*%u, %u/%u\n",
			bi->iform, width, height, xwidth, xheight,
			xwidth * xheight * geo->bits_per_pixel / 8, size );
	    if ( xwidth > 0 && xheight > 0
			&& xwidth * xheight * geo->bits_per_pixel <= 8 * size )
	    {
		return FF_BREFT_IMG;
	    }
	}
    }

    if ( data_size > 3 && data8[1] == 12 && data8[2] == 19 )
    {
	// mabye ItemSlot
	if ( data8[0] ==  6 && file_size == ITEMSLT_SIZE6
	  || data8[0] == 12
		&& ( file_size == ITEMSLT_SIZE12 || file_size == ITEMSLT_SIZE6 ))
	{
	    return FF_ITEMSLT;
	}
    }

    if ( data_size > 4 && file_size == 4+27*396 && be32(data8) == 27 )
	return FF_DRIVER;

    if ( data_size > 4 && file_size == 4+36*396 && be32(data8) == 36 )
	return FF_VEHICLE;

    if (IsDolHeader(data,data_size,0))
	return FF_DOL;

    if ( IsValidKCL(0,data,data_size,file_size,0) < VALID_ERROR )
	return FF_KCL;

    if ( IsValidCTCODE(data,data_size,0,0) < VALID_ERROR )
	return FF_CT1_DATA;

    // test FF_WAV_OBJ & FF_SKP_OBJ
    {
	ccp ptr = data, end = ptr + data_size;
	for(;;)
	{
	    while ( ptr < end && ( *ptr == ' ' || *ptr == '\t'
				    || *ptr == '\r' || *ptr == '\n' ))
		ptr++;

	    if ( *ptr == 'g' )
	    {
		// ignore group commands
		while ( ptr < end && *ptr != '\n' )
		    ptr++;
		continue;
	    }

	    if ( *ptr != '#' )
		break;

	    bool have_obj = false;
	    bool have_wavefront = false;
	    bool have_model = false;
	    while ( ptr < end && *ptr != '\n' )
	    {
		have_obj	= have_obj	 || !strncasecmp(ptr,"obj",3);
		have_wavefront	= have_wavefront || !strncasecmp(ptr,"wavefront",9);
		have_model	= have_model	 || !strncasecmp(ptr,"model",5);

		if ( have_obj && have_wavefront )
		    return FF_WAV_OBJ;
		if ( have_obj && have_model )
		    return FF_SKP_OBJ;
		ptr++;
	    }
	}

	if ( ( !memcmp(ptr,"mtllib",6) || !memcmp(ptr,"usemtl",6) )
		&& ( ptr[6] == ' ' || ptr[6] == '\t' ))
	{
	    return FF_WAV_OBJ;
	}

	if ( *ptr == 'v' && ptr[1] == ' ' )
	{
	    ptr += 2;
	    int i;
	    bool ok = true;
	    for ( i = 0; i < 3 && ok; i++ )
	    {
		char *endscan;
		strtod_comma(ptr,&endscan);
		ok = endscan > ptr;
		ptr = endscan;
	    }
	    while ( ptr < end && ( *ptr == ' ' || *ptr == '\r' ))
		ptr++;
	    if ( ptr < end && *ptr == '\n' )
		return FF_WAV_OBJ;
	}
    }

    if ( IsValidSTATICR(data,data_size,0,0) < VALID_ERROR )
	return FF_STATICR;

    if ( IsValidBTI(data,data_size,0,0) < VALID_ERROR )
	return FF_BTI;


    //--- test for scripts

    ccp beg = data + bom_len;
    ccp end = data + data_size;
    if ( beg < end )
    {
	bool is_text = false;
	ccp ptr = beg;
	while ( ptr < end )
	{
	    const u8 ch = *ptr++;
	    if ( ch == '\r' || ch == '\n' )
	    {
		is_text = true;
		ptr--;
		break;
	    }

	    if ( ch < ' ' && ch != '\f' && ch != '\v' && ch != '\t' )
		break;
	}

	if (is_text)
	{
	    while ( ptr > beg && ptr[-1] <= ' ' )
		ptr--;
	    ccp name = ptr;
	    while ( name > beg && isalnum((int)(name[-1])) )
		name--;
	    noPRINT("|%.*s|%.*s|\n",(int)(name-beg),beg,(int)(ptr-name),name);
	    if (!memcmp(beg,"#!",2))
	    {
		switch( ptr - name )
		{
		    case 2:
			if (!strncasecmp(name,"sh",2)) return FF_SH;
			break;

		    case 3:
			if (!strncasecmp(name,"php",3)) return FF_PHP;
			break;

		    case 4:
			if (!strncasecmp(name,"bash",4)) return FF_BASH;
			break;

		    case 7:
			if (!strncasecmp(name,"makedoc",7)) return FF_MAKEDOC;
			break;
		}
		return FF_SCRIPT;
	    }
	    if ( !memcmp(beg,"<?",2) && !strncasecmp(name,"php",3) )
		return FF_PHP;
	}
    }

    return FF_UNKNOWN;
}

///////////////////////////////////////////////////////////////////////////////
// [[GetFileTypeByExt]]

const file_type_t * GetFileTypeByExt
(
    ccp		ext,		// NULL  or file extension with optional preceeding point
    bool	need_magic	// filter only file types with magic
)
{
    if (!ext)
	return 0;

    while ( *ext == '.' )
	ext++;

    if (!*ext)
	return 0;

    const file_type_t *p;

    // first: try member 'ext'
    for ( p = FileTypeTab; p->name; p++ )
	if ( ( !need_magic || p->magic_len )
			&& p->ext && !strcasecmp(ext,p->ext+1) )
	    return p;

    // second: try member 'ext_compr'
    for ( p = FileTypeTab; p->name; p++ )
	if ( ( !need_magic || p->magic_len )
			&& p->ext_compr && !strcasecmp(ext,p->ext_compr+1) )
	    return p;

    // third: try member 'ext_magic'
    for ( p = FileTypeTab; p->name; p++ )
	if ( ( !need_magic || p->magic_len )
			&& p->ext_magic && !strcasecmp(ext,p->ext_magic+1) )
	    return p;

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// [[GetFileTypeByExt]]

const file_type_t * GetFileTypeBySubdir
(
    ccp		path,		// NULL  or file extension with optional preceeding point
    bool	need_magic	// filter only file types with magic
)
{
    if (!path)
	return 0;

    ccp p1 = 0, p2 = 0;
    for(;;)
    {
	while ( *path == '/' )
	    path++;

	ccp end = strchr(path,'/');
	if (!end)
	    break;
	p1 = path;
	p2 = end;
	path = p2;
    }

    if (p1)
    {
	DASSERT(p2);

	char name[20];
	const uint len = p2 - p1;
	if ( len < sizeof(name) )
	{
	    memcpy(name,p1,len);
	    name[len] = 0;
	    noPRINT("### %s : %s\n",name,p1);

	    const file_type_t *p;
	    for ( p = FileTypeTab; p->name; p++ )
		if ( ( !need_magic || p->magic_len )
				&& p->subdir && !strcasecmp(name,p->subdir) )
		    return p;
	}
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// [[GetByNameFF]]

file_format_t GetByNameFF ( ccp name )
{
    const KeywordTab_t * cmd = ScanKeyword(0,name,cmdtab_FileType);
    return cmd ? cmd->id : FF_UNKNOWN;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

file_format_t IsImageFF
(
    // returns the normalized FF for a valid image or NULL (=FF_UNKNOWN)

    file_format_t	fform,		// file format to check
    bool		allow_png	// true: allow PNG
)
{
    switch (fform)
    {
	case FF_TPL:
	case FF_BTI:
	case FF_TEX:
	case FF_TEX_CT:
	case FF_BREFT_IMG:
	    return fform;

	case FF_BREFT:
	    return FF_BREFT_IMG;

	case FF_PNG:
	    return allow_png ? FF_PNG : FF_UNKNOWN;

	default:
	    return FF_UNKNOWN;
    }
}

///////////////////////////////////////////////////////////////////////////////

file_format_t GetImageFF
(
    file_format_t	fform1,		// if a valid image ff, use this one
    file_format_t	fform2,		// if a valid image ff, use this one
    ccp			path,		// not NULL and exists: use file format
    file_format_t	default_fform,	// use this as default, if valid ff
    bool		allow_png,	// true: allow PNG
    file_format_t	not_found	// return this, if all other fails
)
{
    PRINT("GetImageFF(%s,%s,%s,%d,%s) %s\n",
	GetNameFF(0,fform1), GetNameFF(0,fform2), GetNameFF(0,default_fform),
	allow_png, GetNameFF(0,not_found), path ? path : "-" );

    //--- analyze 'fform*'

    fform1 = IsImageFF(fform1,allow_png);
    if ( fform1 != FF_UNKNOWN )
	return fform1;

    fform2 = IsImageFF(fform2,allow_png);
    if ( fform2 != FF_UNKNOWN )
	return fform2;


    //--- analyse path

    if ( path && *path )
    {
	//--- analyse file content

	char buf[CHECK_FILE_SIZE];
	memset(buf,0,sizeof(buf));
	enumError err = LoadFILE(path,0,0,buf,sizeof(buf),2,0,false);
	if (!err)
	{
// [[analyse-magic]]
	    const file_format_t fform
		= IsImageFF( GetByMagicFF(buf,sizeof(buf),0), allow_png );
	    if ( fform != FF_UNKNOWN )
		return fform;
	}

	//--- analyse file extension

	ccp point = strrchr(path,'.');
	if (point)
	{
	    const file_format_t fform = IsImageFF(GetByNameFF(point+1),true);
	    if ( fform != FF_UNKNOWN )
		return fform;
	}

	//--- analyse last directory

	ccp p1 = 0, p2 = path, ptr = path;
	while (*ptr)
	{
	    if ( *ptr++ == '/' || *ptr++ == '\\' )
	    {
		p1 = p2;
		p2 = ptr; // position behind '/'
	    }
	}
	if (p1)
	{
	    if (!strncasecmp(p1,"files/",6))
		return FF_BREFT_IMG;
	    if (!strncasecmp(p1,"Textures(NW4R)/",15))
		return FF_TEX;
	}
    }


    //--- use default

    default_fform = IsImageFF(default_fform,allow_png);
    return default_fform != FF_UNKNOWN ? default_fform : not_found;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			object name/type		///////////////
///////////////////////////////////////////////////////////////////////////////

ccp GetNewObjectName ( new_object_t no )
{
    switch (no)
    {
	case NEWOBJ_OFF:	return "OFF";
	case NEWOBJ_REPLACE:	return "REPLACE";
	case NEWOBJ_SHRINK:	return "SHRING";
	case NEWOBJ_GROW:	return "GROW";
	default:		return "?";
    }
}

///////////////////////////////////////////////////////////////////////////////

void InitializeObjType ( object_type_t *ot, file_format_t fform )
{
    DASSERT(ot);
    memset(ot,0,sizeof(*ot));

    ot->new_object	= NEWOBJ_OFF;
    ot->object_slots	= OBJMGR_USUAL_SLOTS;
    ot->max_object_id	= OBJMGR_USUAL_MAX_OBJID;

    SetObjTypeByFF(ot,fform);
}

///////////////////////////////////////////////////////////////////////////////

void SetObjTypeByFF ( object_type_t *ot, file_format_t fform )
{
    DASSERT(ot);

    switch(fform)
    {
     case FF_OBJFLOW:
     case FF_OBJFLOW_TXT:
	ot->is_objflow	= true;
	ot->is_geohit	= false;
	ot->is_kart	= false;
	ot->is_obj	= false;
	ot->row_size	= 0x74;
	ot->slot_offset	= 2;
	break;

     case FF_GH_ITEM:
     case FF_GH_ITEM_TXT:
	ot->is_objflow	= false;
	ot->is_geohit	= true;
	ot->is_kart	= false;
	ot->is_obj	= false;
	ot->row_size	= 0x20;
	ot->slot_offset	= 4;
	break;

     case FF_GH_IOBJ:
     case FF_GH_IOBJ_TXT:
	ot->is_objflow	= false;
	ot->is_geohit	= true;
	ot->is_kart	= false;
	ot->is_obj	= true;
	ot->row_size	= 0x20;
	ot->slot_offset	= 4;
	break;

     case FF_GH_KART:
     case FF_GH_KART_TXT:
	ot->is_objflow	= false;
	ot->is_geohit	= true;
	ot->is_kart	= true;
	ot->is_obj	= false;
	ot->row_size	= 0x0a;
	ot->slot_offset	= 4;
	break;

     case FF_GH_KOBJ:
     case FF_GH_KOBJ_TXT:
	ot->is_objflow	= false;
	ot->is_geohit	= true;
	ot->is_kart	= true;
	ot->is_obj	= true;
	ot->row_size	= 0x0a;
	ot->slot_offset	= 4;
	break;

     default:
	ot->is_objflow	= false;
	ot->is_geohit	= false;
	ot->is_kart	= false;
	ot->is_obj	= false;
	ot->row_size	= 0;
	ot->slot_offset	= 0;
	break;
    }

    ot->ref_offset = ot->slot_offset + ot->object_slots * ot->row_size;
}

///////////////////////////////////////////////////////////////////////////////

bool ScanObjType
	( object_type_t *ot, cvp data, uint data_size, uint file_size )
{
    //--- check for valid data

    DASSERT(ot);
    memset(ot,0,sizeof(*ot));
    if ( !data || data_size < 4 || file_size&1 )
	return false;

    const uint n_slot = be16(data);
    if ( !n_slot || n_slot > OBJMGR_MAX_SLOTS )
	return false;


    //--- determine possible file types

    file_format_t ff_std, ff_obj;

    const uint n_param = be16((u8*)data+2);
    switch (n_param)
    {
     case 4:
	ff_std		= FF_GH_KART;
	ff_obj		= FF_GH_KOBJ;
	ot->is_geohit	= true;
	ot->is_kart	= true;
	ot->row_size	= 0x0a;
	ot->slot_offset	= 4;
	break;

     case 15:
	ff_std		= FF_GH_ITEM;
	ff_obj		= FF_GH_IOBJ;
	ot->is_geohit	= true;
	ot->row_size	= 0x20;
	ot->slot_offset	= 4;
	break;

     default:
	ff_std		= FF_OBJFLOW;
	ff_obj		= FF_UNKNOWN;
	ot->is_objflow	= true;
	ot->row_size	= 0x74;
	ot->slot_offset	= 2;
	break;
    }

    int max_ref_obj_id = OBJMGR_MAX_OBJID;
    ot->ref_offset = ot->slot_offset + n_slot * ot->row_size;
    if (file_size)
    {
	if ( file_size < ot->ref_offset + 2 )
	    return false;

	max_ref_obj_id = ( file_size - ot->ref_offset ) / 2 - 1;
	if ( max_ref_obj_id > OBJMGR_MAX_OBJID )
	     max_ref_obj_id = OBJMGR_MAX_OBJID;
    }


    //--- determine max_object_id

    const u8 *bin, *bin0 = (u8*)data + ot->slot_offset;
    uint slots_avail = ( (u8*)data + data_size - bin0 ) / ot->row_size;
    if ( slots_avail > n_slot )
	 slots_avail = n_slot;

    uint slot, max_obj_id = 0;
    for ( slot = 0, bin = bin0; slot < slots_avail; slot++, bin += ot->row_size )
    {
	uint obj_id = be16(bin);
	if ( !obj_id || obj_id > max_ref_obj_id )
	    return false;

	if ( max_obj_id < obj_id )
	     max_obj_id = obj_id;
    }

    ot->object_slots  = n_slot;
    ot->max_object_id = max_ref_obj_id;


    //--- determine is_object

    if (ot->is_geohit)
    {
	if (ot->is_kart)
	{
	    uint n7x = 0, n36 = 0;
	    for ( slot = 0, bin = bin0; slot < slots_avail; slot++, bin += ot->row_size )
	    {
		const geohit_bin_t *gb = (geohit_bin_t*)bin;
		uint ip;
		for ( ip = 0; ip < n_param; ip++ )
		{
		    const u16 p = ntohs(gb->param[ip]);
		    if ( p >= 7 )
			n7x++;
		    else if ( p >= 3 && p <= 6 )
			n36++;
		}
	    }
	    ot->is_obj = n7x < n36;
	    PRINT("KART: N7x=%u, N36=%u -> obj=%u\n",n7x,n36,ot->is_obj);
	}
	else
	{
	    uint n12 = 0, n36 = 0;
	    for ( slot = 0, bin = bin0; slot < slots_avail; slot++, bin += ot->row_size )
	    {
		const geohit_bin_t *gb = (geohit_bin_t*)bin;
		uint ip;
		for ( ip = 0; ip < n_param; ip++ )
		{
		    const u16 p = ntohs(gb->param[ip]);
		    if ( p >= 1 && p <= 2 )
			n12++;
		    else if ( p >= 3 && p <= 6 )
			n36++;
		}
	    }
	    ot->is_obj = n12 < n36;
	    PRINT("ITEM: N12=%u, N36=%u -> obj=%u\n",n12,n36,ot->is_obj);
	}
    }


    //--- terminate

    ot->fform = ot->is_obj ? ff_obj : ff_std;

    PRINT("ScanObjType(0x%x,0x%x) = %s\n",
		data_size, file_size, LogObjType(ot) );
    return true;
}

///////////////////////////////////////////////////////////////////////////////

ccp LogObjType ( const object_type_t *ot )
{
    DASSERT(ot);
    if (!ot)
	return "?";

    char buf[100];
    const uint len
	= snprintf(buf,sizeof(buf),
			"%s[%s%s%s%s] rs=%u off:%x:%x %s,ns:%x,mo:%x",
			GetNameFF(0,ot->fform),
			ot->is_objflow	? "O" : "",
			ot->is_geohit	? "G" : "",
			ot->is_kart	? "K" : ot->is_objflow ? "" : "I",
			ot->is_obj	? "o" : "",
			ot->row_size,
			ot->slot_offset,
			ot->ref_offset,
			GetNewObjectName(ot->new_object),
			ot->object_slots,
			ot->max_object_id );

    return CopyCircBuf(buf,len+1);
}

///////////////////////////////////////////////////////////////////////////////

file_format_t GetObjTypeFF ( const object_type_t *ot, bool want_text )
{
    DASSERT(ot);
    if (ot->is_objflow)
	return want_text ? FF_OBJFLOW_TXT : FF_OBJFLOW;

    if (ot->is_geohit)
    {
	if (ot->is_kart)
	{
	    return ot->is_obj
		? ( want_text ? FF_GH_KOBJ_TXT : FF_GH_KOBJ )
		: ( want_text ? FF_GH_KART_TXT : FF_GH_KART );
	}
	else
	{
	    return ot->is_obj
		? ( want_text ? FF_GH_IOBJ_TXT : FF_GH_IOBJ )
		: ( want_text ? FF_GH_ITEM_TXT : FF_GH_ITEM );
	}
    }

    return FF_UNKNOWN;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    file class			///////////////
///////////////////////////////////////////////////////////////////////////////

patch_file_t PATCH_FILE_MODE = PFILE_M_DEFAULT | PFILE_F_DEFAULT;
int disable_patch_on_load = 0;	// if >0: disable patching after loading

///////////////////////////////////////////////////////////////////////////////

static const KeywordTab_t opt_patch_tab[] =
{
  { 0,			"CLEAR",	"RESET",	PFILE_M_ALL | PFILE_F_HIDE },
  { PFILE_M_DEFAULT,	"DEFAULT",	0,		PFILE_M_ALL | PFILE_F_HIDE },
  { PFILE_M_ALL,	"ALL",		0,		PFILE_M_ALL | PFILE_F_HIDE },
  { PFILE_F_LOG,	"LOG",		0,		0 },
  { PFILE_F_LOG_ALL,	"LOG-ALL",	"LOGALL",	0 },

  { PFILE_COURSE_KMP,	"KMP",		0,		0 },

  { PFILE_M_KCL,	"ALL-KCL",	"ALLKCL",	PFILE_M_KCL },
  { PFILE_COURSE_KCL,	"KCL",		0,		0 },
  { PFILE_OTHER_KCL,	"OTHER-KCL",	"OTHERKCL",	0 },

  { PFILE_M_BRRES,	"ALL-BRRES",	"ALLBRRES",	PFILE_M_BRRES },
  { PFILE_MODEL,	"MODEL",	0,		0 },
  { PFILE_MINIMAP,	"MINIMAP",	"MAP",		0 },
  { PFILE_VRCORN,	"VRCORN",	0,		0 },
  { PFILE_OTHER_BRRES,	"OTHER-BRRES",	"OTHERBRRES",	0 },

  { PFILE_M_TRACK,	"TRACK",	0,		PFILE_M_TRACK | PFILE_F_HIDE },

  { 0,0,0,0 }
};

///////////////////////////////////////////////////////////////////////////////

int ScanOptPatchFile ( ccp arg )
{
    if (!arg)
	return 0;

    static bool done = false;
    if (!done)
    {
	done = true;
	if ( *arg != '+' && *arg != '-' )
	    PATCH_FILE_MODE = 0;
    }

    s64 stat = ScanKeywordList(arg,opt_patch_tab,0,true,0,PATCH_FILE_MODE,
				"Option --patch-file",ERR_SYNTAX);
    if ( stat != -1 )
    {
	PATCH_FILE_MODE = stat & PFILE_M_ALL;
	return 0;
    }
    return 1;
}

///////////////////////////////////////////////////////////////////////////////

void SetPatchFileModeReadonly()
{
    // switch to readonly default flags (silent), if not set by user

    if ( PATCH_FILE_MODE & PFILE_F_DEFAULT )
	PATCH_FILE_MODE = PFILE_M_SILENT;
}

///////////////////////////////////////////////////////////////////////////////

uint PrintFileClassInfo ( char *buf, uint bufsize, patch_file_t mode )
{
    DASSERT(buf);
    DASSERT(bufsize>10);
    char *dest = buf;
    char *end = buf + bufsize - 1;

    mode = mode & PFILE_M_ALL | PFILE_F_HIDE;
    patch_file_t mode1 = mode;

    const KeywordTab_t *ct;
    for ( ct = opt_patch_tab; ct->name1 && dest < end; ct++ )
    {
	if ( ct->opt & PFILE_F_HIDE )
	    continue;

	if ( ct->opt ? (mode & ct->opt) == ct->id : mode & ct->id )
	{
	    if ( dest > buf )
		*dest++ = ',';
	    dest = StringCopyE(dest,end,ct->name1);
	    mode &= ~(ct->id|ct->opt);
	}
    }

    if ( mode1 == (PFILE_M_DEFAULT|PFILE_F_HIDE) )
	dest = StringCopyE(dest,end," (default)");
    else if (!mode1)
	dest = StringCopyE(dest,end,"(none)");

    *dest = 0;
    return dest-buf;
}

///////////////////////////////////////////////////////////////////////////////

ccp GetFileClassInfo()
{
    static char buf[100] = {0};
    if (!*buf)
	PrintFileClassInfo(buf,sizeof(buf),PATCH_FILE_MODE);
    return buf;
}

///////////////////////////////////////////////////////////////////////////////

patch_file_t GetFileClass
(
    file_format_t	fform,		// not FF_UNKNOWN: check file type
    ccp			fname		// NULL or file name to check
)
{
    if ( !fname || !*fname )
	return PFILE_NO_FILENAME;

    ccp slash = strrchr(fname,'/');
    if (slash)
	fname = slash+1;

    if ( fform == FF_UNKNOWN && !strcasecmp(fname,"course.txt") )
	return PFILE_COURSE;

    if ( fform == FF_UNKNOWN || fform == FF_KMP || fform == FF_KMP_TXT )
    {
	if (   !strcasecmp(fname,"course.kmp")
	    || !strcasecmp(fname,"course.txt")
	    || !strcasecmp(fname,"course.kmp.txt") )
	{
	    return PFILE_COURSE_KMP;
	}
    }

    if ( fform == FF_UNKNOWN || fform == FF_KCL || fform == FF_KCL_TXT )
    {
	if (   !strcasecmp(fname,"course.kcl")
	    || !strcasecmp(fname,"course.txt")
	    || !strcasecmp(fname,"course.kcl.txt") )
	{
	    return PFILE_COURSE_KCL;
	}

	if ( fform == FF_KCL || fform == FF_KCL_TXT )
	    return PFILE_OTHER_KCL;
    }

    if ( fform == FF_UNKNOWN || fform == FF_BRRES )
    {
	if (    !strcasecmp(fname,"course_model.brres")
	     || !strcasecmp(fname,"course_d_model.brres")
	     || !strcasecmp(fname,"course_model.d")
	     || !strcasecmp(fname,"course_d_model.d") )
	{
	    return PFILE_MODEL;
	}

	if (    !strcasecmp(fname,"map_model.brres")
	     || !strcasecmp(fname,"map_model.d") )
	{
	    return PFILE_MINIMAP;
	}

	if (    !strcasecmp(fname,"vrcorn_model.brres")
	     || !strcasecmp(fname,"vrcorn_model.d") )
	{
	    return PFILE_VRCORN;
	}

	if ( fform == FF_BRRES )
	    return PFILE_OTHER_BRRES;
    }

    return PFILE_UNKNOWN;
}

///////////////////////////////////////////////////////////////////////////////

bool PatchFileClass
(
    file_format_t	fform,		// not FF_UNKNOWN: check file type
    ccp			fname		// NULL or file name to check
)
{
    const patch_file_t fc = GetFileClass(fform,fname);
    return 0 != ( fc & PATCH_FILE_MODE );
}

///////////////////////////////////////////////////////////////////////////////

int patch_action_log_disabled = 0;

bool PATCH_ACTION_LOG ( ccp action, ccp object, ccp format, ... )
{
    #if HAVE_PRINT
    {
	char buf[200];
	snprintf(buf,sizeof(buf),">>>[PATCH]<<< %s",format);
	va_list arg;
	va_start(arg,format);
	PRINT_ARG_FUNC(buf,arg);
	va_end(arg);
    }
    #endif

    if ( patch_action_log_disabled <= 0
	&& ( verbose > 2 || PATCH_FILE_MODE & (PFILE_F_LOG|PFILE_F_LOG_ALL) ))
    {
	fflush(stdout);
	fprintf(stdlog,"    %s>[PATCH] %-6s %-7s%s ",
		colset->heading, action, object, colset->info );
	if (format)
	{
	    va_list arg;
	    va_start(arg,format);
	    vfprintf(stdlog,format,arg);
	    va_end(arg);
	    fputs(colset->reset,stdlog);
	}
	else
	    fprintf(stdlog,"%s\n",colset->reset);
	fflush(stdlog);
	return true;
    }

    return false;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			auto add support		///////////////
///////////////////////////////////////////////////////////////////////////////

bool SetupAutoAdd()
{
    static bool done = false;
    if (!done)
    {
	done = true;

	ccp *spath = search_path;
	ccp *apath = auto_add_path;
	while (*spath)
	{
	    char path_buf[PATH_MAX];
	    ccp path = PathCatPP(path_buf,sizeof(path_buf),*spath++,"auto-add");
	    if (IsDirectory(path,0))
	    {
		PRINT("AUTO-PATH[%u] = %s\n",(int)(apath-auto_add_path),path);
		*apath++ = STRDUP(path);
		DASSERT( apath - auto_add_path
			< sizeof(auto_add_path)/sizeof(*auto_add_path) );
	    }
	}
	*apath = 0;
    }

    return auto_add_path[0] != 0;
}

///////////////////////////////////////////////////////////////////////////////

bool DefineAutoAddPath ( ccp path )
{
    SetupAutoAdd();
    if (path)
    {
	uint i;
	for ( i = 0; i < sizeof(auto_add_path)/sizeof(*auto_add_path); i++ )
	{
	    FreeString(auto_add_path[i]);
	    auto_add_path[i] = 0;
	}
	auto_add_path[0] = STRDUP(path);
    }

    return auto_add_path[0] != 0;
}

///////////////////////////////////////////////////////////////////////////////

s64 FindAutoAdd ( ccp fname, ccp ext, char *buf, uint buf_size )
{
    DASSERT(fname);
    DASSERT(buf);
    DASSERT( buf_size > 100 );

    if (!SetupAutoAdd())
	return -1;

    ccp *apath = auto_add_path;
    while (*apath)
    {
	PathCatBufPPE(buf,buf_size,*apath++,fname,ext);
	struct stat st;
	if ( !stat(buf,&st) && S_ISREG(st.st_mode) )
	{
	    noPRINT("FindAutoAdd(%s) -> %llu %s\n",fname,(u64)st.st_size,buf);
	    return st.st_size;
	}
    }

    noPRINT("FindAutoAdd(%s) FAILED\n",fname);
    *buf = 0;
    return -1;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			file format atribs		///////////////
///////////////////////////////////////////////////////////////////////////////

const file_type_t * GetFileTypeFF ( file_format_t ff )
{
    return (uint)ff < FF_N ? FileTypeTab + ff : 0;
}

///////////////////////////////////////////////////////////////////////////////

ff_attrib_t GetAttribFF ( file_format_t ff )
{
    return (uint)ff < FF_N
		? FileTypeTab[ff].attrib
		: (ff_attrib_t)0;
}

///////////////////////////////////////////////////////////////////////////////

ff_attrib_t GetAttribByMagicFF
(
    const void		* data,		// valid pointer to data
    uint		data_size,	// size of 'data'
    file_format_t	*ff		// not NULL: store file format here
)
{
// [[analyse-magic]]
    const file_format_t fform = GetByMagicFF(data,data_size,0);
    if (ff)
	*ff = fform;
    return GetAttribFF(fform);
}

///////////////////////////////////////////////////////////////////////////////

int GetVersionFF
(
    // returns -1 for unknown, or the version number

    file_format_t	fform,		// valid firl format
    const void		*data,		// data pointer
    uint		data_size,	// size of available data
    char		*res_suffix	// not NULL: return 1 char as suffix, or 0
)
{
    if (res_suffix)
	*res_suffix = 0;

    switch(fform)
    {
 #if 0
	case FF_YAZ0:
	case FF_XYZ:
	    return 0;

	case FF_YAZ1:
	    return 1;

	case FF_BZ:
	    return ((u8*)data)[3] - 'a' + 1;
 #endif

	case FF_BREFF:
	case FF_BREFT:
	    if ( data_size >= 0x08 )
		return be16((u8*)data+0x06);
	    break;

	case FF_CHR:
	case FF_CLR:
	case FF_MDL:
	case FF_PAT:
	case FF_SCN:
	case FF_SHP:
	case FF_SRT:
	case FF_TEX:
	case FF_TEX_CT: // ??? [[CTCODE]]
	    if ( data_size >= 0x0c )
		return be32((u8*)data+0x08);
	    break;

 #if 0
	case FF_KMP:
	    if ( data_size >= 0x10 )
		return be32((u8*)data+0x0c);
	    break;
 #endif

	case FF_BMG: // use n_sections & encoding
	    if ( data_size >= sizeof(bmg_header_t) )
	    {
		const bmg_header_t *bh = (bmg_header_t*)data;
		return ntohl(bh->n_sections)*10 + bh->encoding;
	    }
	    break;

	case FF_LE_BIN:
	    {
		const int vers = be32((u8*)data+4);
		if (res_suffix)
		{
		    if ( vers == 4 && data_size >= sizeof(le_binary_head_v4_t) )
		    {
			const le_binary_head_v4_t *leh = (le_binary_head_v4_t*)data;
			*res_suffix = leh->debug == 'R'
					? toupper(leh->region)
					: tolower(leh->region);
		    }
		}
		return vers;
	    }
	    break;

	default:
	    //printf("x[%u]\n",fform);
	    {
		const ff_attrib_t att = GetAttribFF(fform);
		if ( att & FFT_BRSUB && data_size >= 0x0c )
		    return be32((u8*)data+0x08);
	    }
	    break;
    }
    return -1;
}

///////////////////////////////////////////////////////////////////////////////

int GetVersionByMagic
(
    // returns -1 for unknown, or the version number

    const void		*data,		// data pointer
    uint		data_size,	// size of available data
    char		*res_suffix	// not NULL: return 1 char as suffix, or 0
)
{
// [[analyse-magic]]
    return GetVersionFF( GetByMagicFF(data,data_size,0),
				data, data_size, res_suffix );
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool IsBRSUB ( file_format_t ff )
{
    return (uint)ff < FF_N && FileTypeTab[ff].attrib & FFT_BRSUB;
}

//-----------------------------------------------------------------------------

bool IsByMagicBRSUB ( const void * data, uint data_size )
{
    return GetAttribByMagicFF(data,data_size,0) & FFT_BRSUB;
}

///////////////////////////////////////////////////////////////////////////////

bool IsArchiveFF ( file_format_t ff )
{
    return (uint)ff < FF_N && FileTypeTab[ff].attrib & FFT_ARCHIVE;
}

//-----------------------------------------------------------------------------

bool IsTextFF ( file_format_t ff )
{
    return (uint)ff < FF_N && FileTypeTab[ff].attrib & FFT_TEXT;
}

//-----------------------------------------------------------------------------

bool IsGeoHitKartFF ( file_format_t ff )
{
    return ff == FF_GH_KART
	|| ff == FF_GH_KART_TXT
	|| ff == FF_GH_KOBJ
	|| ff == FF_GH_KOBJ_TXT;
}

//-----------------------------------------------------------------------------

bool IsGeoHitObjFF ( file_format_t ff )
{
    return ff == FF_GH_IOBJ
	|| ff == FF_GH_IOBJ_TXT
	|| ff == FF_GH_KOBJ
	|| ff == FF_GH_KOBJ_TXT;
}

///////////////////////////////////////////////////////////////////////////////

bool IsYazFF ( file_format_t ff )
{
    return ff == FF_YAZ0 || ff == FF_YAZ1 || ff == FF_XYZ;
}

//-----------------------------------------------------------------------------

int GetYazVersionFF ( file_format_t ff )
{
    switch (ff)
    {
	case FF_YAZ0:
	case FF_XYZ:
	    return 0;

	case FF_YAZ1:
	    return 1;

	default:
	    return -1;
    }
}

//-----------------------------------------------------------------------------

file_format_t GetYazFF ( file_format_t ff )
{
    return fform_compr_force && ( ff == FF_YAZ0 || ff == FF_YAZ1 || ff == FF_XYZ )
	? fform_compr_force
	: IsYazFF(ff)
		? ff
		: FF_YAZ0;
}

///////////////////////////////////////////////////////////////////////////////

bool IsCompressedFF ( file_format_t ff )
{
    return (uint)ff < FF_N && FileTypeTab[ff].attrib & FFT_COMPRESS;
}

//-----------------------------------------------------------------------------

file_format_t GetCompressedFF ( file_format_t ff )
{
    return fform_compr_force
	&& (uint)ff < FF_N
	&& FileTypeTab[ff].attrib & FFT_COMPRESS
		? fform_compr_force
		: ff;
}

//-----------------------------------------------------------------------------

void SetCompressionFF
(
    file_format_t	ff_arch,	// if >0: set 'opt_fform'
    file_format_t	ff_compr	// if >0: set 'fform_compr'
					//        and  'fform_compr_force'
)
{
    if ( ff_arch > 0 )
	opt_fform = ff_arch;

    if ( ff_compr > 0 )
	fform_compr = fform_compr_force = ff_compr;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			scan setup files		///////////////
///////////////////////////////////////////////////////////////////////////////

size_t ResetSetup
(
    SetupDef_t	* list		// object list terminated with an element 'name=NULL'
)
{
    DASSERT(list);
    size_t count;
    for ( count = 0; list->name; list++, count++ )
    {
	FreeString(list->param);
	list->param = 0;
	list->value = 0;
    }
    return count;
}

///////////////////////////////////////////////////////////////////////////////

static char * trim_line ( char * ptr, char ** begin )
{
    DASSERT(ptr);

    while ( *ptr > 0 && *ptr <= ' ' )
	ptr++;

    char *start = ptr;
    if (begin)
	*begin = ptr;

    while (*ptr)
	ptr++;

    ptr--;
    while ( *ptr > 0 && *ptr <= ' ' && ptr > start )
	ptr--;
    *++ptr = 0;
    return ptr;
}

//-----------------------------------------------------------------------------

enumError ScanSetupFile
(
    SetupDef_t	* sdef,		// object list terminated with an element 'name=NULL'
    ListDef_t	* ldef,		// NULL or object list terminated
				// with an element 'name=NULL'
    ccp		path1,		// filename of text file, part 1
    ccp		path2,		// filename of text file, part 2
    SubDir_t	* sdir,		// not NULL: use it and ignore 'path1'
    bool	silent		// true: suppress error message if file not found
)
{
    DASSERT(sdef);
    DASSERT(path1||path2);

    ResetSetup(sdef);

    char pathbuf[PATH_MAX];
    ccp path;
    FILE * f;

    if (sdir)
    {
	path = path2;
	f = OpenSubFile(sdir,MemByString(path2));
    }
    else
    {
	path = PathCatPP(pathbuf,sizeof(pathbuf),path1,path2);
	f = fopen(path,"rb");
    }

    if (!f)
    {
	if (!silent)
	    ERROR1(ERR_CANT_OPEN,"Can't open file: %s\n",path);
	return ERR_CANT_OPEN;
    }

    bool section_found = false;
    uint linenum = 0;
    while (fgets(iobuf,sizeof(iobuf)-1,f))
    {
	char * ptr = iobuf;

	//----- check BOM

	if (!linenum++)
	    ptr += GetTextBOMLen(ptr,3);

	//----- skip spaces

	while ( *ptr > 0 && *ptr <= ' ' )
	    ptr++;

	if ( *ptr == '!' || *ptr == '#' )
	    continue;

	if ( *ptr == '[' )
	{
	    ptr = trim_line(ptr,0);
	    if ( ptr[-1] == ']' )
	    {
		section_found = true;
		break;
	    }
	    continue;
	}

	//----- find end of name

	char * name = ptr;
	while ( isalnum((int)*ptr) || *ptr == '-' )
	    ptr++;
	if (!*ptr)
	    continue;

	char * name_end = ptr;

	//----- skip spaces and check for '='

	while ( *ptr > 0 && *ptr <= ' ' )
	    ptr++;

	if ( *ptr != '=' )
	    continue;

	*name_end = 0;

	//----- check if name is a known parameter

	SetupDef_t * item;
	for ( item = sdef; item->name; item++ )
	    if (!strcmp(item->name,name))
		break;
	if (!item->name)
	    continue;

	//----- trim parameter

	ptr++; // skip '='
	char * param;
	ptr = trim_line(ptr,&param);

	item->param = STRDUP(param);
	if (item->factor)
	{
	    ScanSizeU64(&item->value,param,1,1,0);
	    if ( item->factor > 1 )
		item->value = item->value / item->factor * item->factor;
	}
    }

    while ( section_found && ldef )
    {
	char * beg;
	char * ptr = trim_line(iobuf,&beg) - 1;
	if ( *beg != '[' || *ptr != ']' )
	    break;

	*ptr = 0;
	beg++;
	TRACE("SECTION |%s|\n",beg);
	StringField_t *sf = 0;
	bool append = false;
	ListDef_t * ld;
	for ( ld = ldef; ld->name; ld++ )
	    if (!strcmp(beg,ld->name))
	    {
		sf = ld->sf;
		append = ld->append;
		break;
	    }

	while (fgets(iobuf,sizeof(iobuf)-1,f))
	{
	    char * ptr = trim_line(iobuf,&beg);
	    if ( *beg == '[' )
	    {
		if ( ptr[-1] == ']' )
		    break;
		continue;
	    }

	    if ( *beg == '.' && beg[1] == '/' )
		beg += 2;
	    if ( !sf || !*beg || *beg == '#' )
		continue;
	    TRACE("INSERT |%s|\n",beg);
	    if (append)
		AppendStringField(sf,beg,false);
	    else
		InsertStringField(sf,beg,false);
	}
    }

    fclose(f);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			 SetupParam_t			///////////////
///////////////////////////////////////////////////////////////////////////////

void InitializeSetupParam ( SetupParam_t * sp )
{
    DASSERT(sp);
    memset(sp,0,sizeof(*sp));

    InitializeStringField(&sp->include_pattern);
    InitializeStringField(&sp->include_name);
    InitializeStringField(&sp->exclude_pattern);
    InitializeStringField(&sp->exclude_name);
    InitializeFormatField(&sp->encode_list);
    InitializeStringField(&sp->create_list);
    InitializeFormatField(&sp->order_list);
}

///////////////////////////////////////////////////////////////////////////////

void ResetSetupParam ( SetupParam_t * sp )
{
    if (sp)
    {
	ResetStringField(&sp->include_pattern);
	ResetStringField(&sp->include_name);
	ResetStringField(&sp->exclude_pattern);
	ResetStringField(&sp->exclude_name);
	ResetFormatField(&sp->encode_list);
	ResetStringField(&sp->create_list);
	ResetFormatField(&sp->order_list);
	FreeString(sp->object_name);

	InitializeSetupParam(sp);
    }
}

///////////////////////////////////////////////////////////////////////////////

static void SetupParamOptions
(
    SetupParam_t	* sp		// setup param to scan
)
{
    //--- use options?

    if ( opt_fform != FF_UNKNOWN )
	sp->fform_arch = opt_fform;
    if ( opt_compr_mode )
    {
	sp->compr_mode = opt_compr_mode;
	if ( opt_compr_mode < 0 )
	    sp->fform_file = FF_UNKNOWN;
    }

    if ( sp->fform_arch == FF_UNKNOWN )
	sp->fform_arch = FF_U8;
    if ( sp->fform_file == FF_UNKNOWN )
	sp->fform_file = opt_compr_mode < 0 ? sp->fform_arch : fform_compr;
}

///////////////////////////////////////////////////////////////////////////////

enumError ScanSetupParam
(
    SetupParam_t	* sp,		// setup param to scan
    bool		use_options,	// true: use options to override values
    ccp			path1,		// NULL or part #1 of path
    ccp			path2,		// NULL or part #2 of path
    SubDir_t		* sdir,		// not NULL: use it and ignore 'path1'
    bool		silent		// true: suppress printing of error messages
)
{
    DASSERT(sp);
    memset(sp,0,sizeof(*sp));

    SetupDef_t setup_def[] =
    {
	{ "object-name",	0 },
	{ "object-id",		1 },
	{ "archive-format",	0 },
	{ "file-format",	0 },
	{ "version",		1 },
	{ "have-pt-dir",	1 },
	{ "min-data-offset",	1 },
	{ "max-data-offset",	1 },
	{ "data-align",		1 },
	{0,0}
    };

    if (!opt_ignore_setup)
    {
	StringField_t encode_helper, order_helper;
	InitializeStringField(&encode_helper);
	InitializeStringField(&order_helper);

	ListDef_t list_def[] =
	{
	    { "include-pattern", 0, &sp->include_pattern },
	    { "include",	 0, &sp->include_name },
	    { "exclude-pattern", 0, &sp->exclude_pattern },
	    { "exclude",	 0, &sp->exclude_name },
	    { "encode",		 0, &encode_helper },
	    { "create",		 0, &sp->create_list },
	    { "order",		 1, &order_helper },
	    {0,0}
	};

	enumError err = ScanSetupFile(setup_def,list_def,path1,path2,sdir,silent);
	if (err)
	{
	    if (use_options)
		SetupParamOptions(sp);
	    return err;
	}

	ccp *ptr = encode_helper.field, *end;
	for ( end = ptr + encode_helper.used; ptr < end; ptr++ )
	{
	    InsertFormatField(&sp->encode_list,*ptr,true,true,0);
	    *ptr = 0;
	}
	ResetStringField(&encode_helper);

	ptr = order_helper.field;
	for ( end = ptr + order_helper.used; ptr < end; ptr++ )
	{
	    AppendFormatField(&sp->order_list,*ptr,true,true);
	    *ptr = 0;
	}
	ResetStringField(&order_helper);
    }


    //--- object-name

    SetupDef_t * sd = setup_def;
    noPRINT("object-name=%s\n",sd->param);
    if (sd->param)
    {
	sp->object_name = sd->param;
	sd->param = 0;
    }

    //--- object-id

    sd++;
    noPRINT("object-id=%s\n",sd->param);
    if (sd->param)
    {
	sp->object_id_used = true;
	sp->object_id = sd->value;
    }

    //--- archive-format

    sd++;
    noPRINT("ff-arch=%s\n",sd->param);
    if (sd->param)
	sp->fform_arch = GetByNameFF(sd->param);


    //--- file-format

    sd++;
    noPRINT("ff-file=%s\n",sd->param);
    if ( sd->param && *sd->param )
    {
	sp->fform_file = GetByNameFF(sd->param);
	if (IsCompressedFF(sp->fform_file))
	{
	    sp->compr_mode = 1;
	    sp->fform_file = GetCompressedFF(sp->fform_file);
	}
	else
	{
	    sp->fform_file = sp->fform_arch;
	    sp->compr_mode = -1;
	}
    }

    //--- version

    sd++;
    noPRINT("version=%s\n",sd->param);
    if (sd->param)
	sp->version = sd->value;

    //--- have-pt-dir

    sd++;
    noPRINT("pt-dir=%s\n",sd->param);
    if (sd->param)
	sp->have_pt_dir = sd->value > 0 ? 1 : -1;

    //--- min-data-size

    sd++;
    noPRINT("pt-dir=%s\n",sd->param);
    if (sd->param)
	sp->min_data_off = sd->value;

    //--- max-data-size

    sd++;
    noPRINT("pt-dir=%s\n",sd->param);
    if (sd->param)
	sp->max_data_off = sd->value;

    //--- data-align

    sd++;
    noPRINT("pt-dir=%s\n",sd->param);
    if (sd->param)
	sp->data_align = sd->value;

    //--- terminate

    if (use_options)
	SetupParamOptions(sp);
    ResetSetup(setup_def);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			sub file system			///////////////
///////////////////////////////////////////////////////////////////////////////

void ResetSubDirList ( SubDirList_t *sdl )
{
    if (sdl)
    {
	uint i;
	for ( i = 0; i < sdl->used; i++ )
	    ResetSubDir(sdl->list[i]);
	FREE(sdl->list);
	InitializeSubDirList(sdl);
    }
}

///////////////////////////////////////////////////////////////////////////////

void ResetSubDir ( SubDir_t *sd )
{
    if (sd)
    {
	if (sd->dname_alloced)
	    FreeString(sd->dname);
	ResetSubFileList(&sd->file);
	ResetSubDirList(&sd->dir);
    }
}

///////////////////////////////////////////////////////////////////////////////

void ResetSubFileList ( SubFileList_t *sfl )
{
    if (sfl)
    {
	uint i;
	for ( i = 0; i < sfl->used; i++ )
	    ResetSubFile(sfl->list[i]);
	FREE(sfl->list);
	InitializeSubFileList(sfl);
    }
}

///////////////////////////////////////////////////////////////////////////////

void ResetSubFile ( SubFile_t *sf )
{
    if (sf)
    {
	if (sf->fname_alloced)
	    FreeString(sf->fname);
	if (sf->data_alloced)
	    FREE(sf->data);
	InitializeSubFile(sf);
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void DumpSubDir ( FILE *f, int indent, const SubDir_t *sd )
{
    DASSERT(f);
    DASSERT(sd);

    indent = NormalizeIndent(indent);
    fprintf(f,"%*sDIR %s/\n",indent,"",sd->dname);

    indent += 2;
    DumpSubFileList(f,indent,&sd->file);
    DumpSubDirList(f,indent,&sd->dir);
}

///////////////////////////////////////////////////////////////////////////////

void DumpSubDirList ( FILE *f, int indent, const SubDirList_t *sdl )
{
    DASSERT(f);
    DASSERT(sdl);

    uint i;
    for ( i = 0; i < sdl->used; i++ )
	DumpSubDir(f,indent,sdl->list[i]);
}

///////////////////////////////////////////////////////////////////////////////

void DumpSubFileList ( FILE *f, int indent, const SubFileList_t *sfl )
{
    DASSERT(f);
    DASSERT(sfl);
    indent = NormalizeIndent(indent);

    uint i;
    for ( i = 0; i < sfl->used; i++ )
    {
	const SubFile_t *sf = sfl->list[i];
	fprintf(f,"%*s> %s [%u,t=%d]\n",indent,"",sf->fname,sf->size,sf->type);
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static uint FindSubDirHelper
	( const SubDirList_t *sdl, mem_t path, bool * p_found )
{
    int beg = 0;
    if ( sdl && path.len )
    {
	int end = sdl->used - 1;
	while ( beg <= end )
	{
	    const uint idx = (beg+end)/2;
	    int stat = StrCmpMem(path,sdl->list[idx]->dname);
	    if ( stat < 0 )
		end = idx - 1 ;
	    else if ( stat > 0 )
		beg = idx + 1;
	    else
	    {
		noPRINT("InsertSubDirHelper(%.*s) FOUND=%d/%d/%d\n",
			path.len, path.ptr, idx, sdl->used, sdl->size );
		if (p_found)
		    *p_found = true;
		return idx;
	    }
	}
    }

    noPRINT("InsertSubDirHelper(%.*s) failed=%d/%d/%d\n",
		path.len, path.ptr, beg, sdl->used, sdl->size );

    if (p_found)
	*p_found = false;
    return beg;
}

///////////////////////////////////////////////////////////////////////////////

static SubDir_t * InsertSubDirHelper ( SubDirList_t * sdl, int idx )
{
    DASSERT(sdl);
    DASSERT( sdl->used <= sdl->size );
    noPRINT("+SF: %u/%u/%u\n",idx,sdl->used,sdl->size);
    if ( sdl->used == sdl->size )
    {
	sdl->size = 3*sdl->size/2 + 0x20;
	sdl->list = REALLOC(sdl->list,sizeof(*sdl->list)*sdl->size);
    }
    DASSERT( idx <= sdl->used );
    SubDir_t ** dest = sdl->list + idx;
    memmove(dest+1,dest,(sdl->used-idx)*sizeof(*dest));
    sdl->used++;

    *dest = MALLOC(sizeof(**dest));
    InitializeSubDir(*dest);
    return *dest;
}

///////////////////////////////////////////////////////////////////////////////

SubDir_t * InsertSubDir ( SubDir_t *sd, mem_t path, bool *new_dir )
{
    DASSERT(sd);
    ccp ptr = path.ptr;
    ccp end = ptr + path.len;

    while ( ptr < end )
    {
	while ( ptr < end && *ptr == '/' )
	    ptr++;
	ccp start = ptr;
	while ( ptr < end && *ptr && *ptr != '/' )
	    ptr++;
	noPRINT("\t\tDIR:  |%.*s|\n",(int)(ptr-start),start);

	const uint len = ptr - start;
	if (!len)
	    continue;

	bool found;
	const uint idx = FindSubDirHelper(&sd->dir,MemByStringS(start,len),&found);
	if (found)
	{
	    noPRINT("\t-> DIR FOUND\n");
	    sd = sd->dir.list[idx];
	}
	else
	{
	    noPRINT("\t\tDIR NOT FOUND\n");
	    sd = InsertSubDirHelper(&sd->dir,idx);
	    sd->dname = MEMDUP(start,len);
	    sd->dname_alloced = true;
	}
    }

    return sd;
}

///////////////////////////////////////////////////////////////////////////////

SubDir_t * FindSubDir ( const SubDir_t *sd, mem_t path )
{
    if ( !sd || !path.len )
	return 0;

    ccp ptr = path.ptr;
    ccp end = ptr + path.len;

    while ( ptr < end )
    {
	while ( ptr < end && *ptr == '/' )
	    ptr++;
	ccp start = ptr;
	while ( ptr < end && *ptr && *ptr != '/' )
	    ptr++;

	const uint len = ptr - start;
	if (!len)
	    continue;

	bool found;
	const uint idx = FindSubDirHelper(&sd->dir,MemByStringS(start,len),&found);
	noPRINT("\t\tDIR:  |%.*s| %s\n",len,start,found?"FOUND":"");
	if (!found)
	    return 0;

	sd = sd->dir.list[idx];
    }
    return (SubDir_t*)sd;
}

///////////////////////////////////////////////////////////////////////////////

bool RemoveSubDir ( SubDir_t *sd, mem_t path )
{
    // [[2do]] ???
    return false;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static uint FindSubFileHelper
	( const SubFileList_t *sfl, mem_t path, bool * p_found )
{
    int beg = 0;
    if ( sfl && path.len )
    {
	int end = sfl->used - 1;
	while ( beg <= end )
	{
	    const uint idx = (beg+end)/2;
	    int stat = StrCmpMem(path,sfl->list[idx]->fname);
	    if ( stat < 0 )
		end = idx - 1 ;
	    else if ( stat > 0 )
		beg = idx + 1;
	    else
	    {
		noPRINT("InsertSubFileHelper(%.*s) FOUND=%d/%d/%d\n",
			path.len, path.ptr, idx, sfl->used, sfl->size );
		if (p_found)
		    *p_found = true;
		return idx;
	    }
	}
    }

    noPRINT("InsertSubFileHelper(%.*s) failed=%d/%d/%d\n",
		path.len, path.ptr, beg, sfl->used, sfl->size );

    if (p_found)
	*p_found = false;
    return beg;
}

///////////////////////////////////////////////////////////////////////////////

static SubFile_t * InsertSubFileHelper ( SubFileList_t * sfl, int idx )
{
    DASSERT(sfl);
    DASSERT( sfl->used <= sfl->size );
    noPRINT("+SF: %u/%u/%u\n",idx,sfl->used,sfl->size);
    if ( sfl->used == sfl->size )
    {
	sfl->size = 3*sfl->size/2 + 0x20;
	sfl->list = REALLOC(sfl->list,sizeof(*sfl->list)*sfl->size);
    }
    DASSERT( idx <= sfl->used );
    SubFile_t ** dest = sfl->list + idx;
    memmove(dest+1,dest,(sfl->used-idx)*sizeof(*dest));
    sfl->used++;

    *dest = MALLOC(sizeof(**dest));
    InitializeSubFile(*dest);
    return *dest;
}

///////////////////////////////////////////////////////////////////////////////

SubFile_t * InsertSubFile ( SubDir_t *sd, mem_t path, bool *new_file )
{
    DASSERT(sd);
    ccp beg = path.ptr;
    ccp end = beg + path.len;
    ccp ptr = end;

    while ( ptr > beg && *ptr != '/' )
	ptr--;
    if ( ptr > beg )
    {
	sd = InsertSubDir(sd,MemByStringE(beg,ptr),0);
	if ( ptr < end )
	    ptr++;
    }
    DASSERT(sd);

    noPRINT("\t-> FILE: |%.*s|\n",(int)(end-ptr),ptr);

    const uint len = end-ptr;
    bool found;
    const uint idx = FindSubFileHelper(&sd->file,MemByStringS(ptr,len),&found);
    if (found)
    {
	noPRINT("\t-> FILE FOUND\n");
	if (new_file)
	    *new_file = false;
	return sd->file.list[idx];
    }

    noPRINT("\t\tFILE NOT FOUND\n");
    if (new_file)
	*new_file = true;

    SubFile_t *sf = InsertSubFileHelper(&sd->file,idx);
    sf->fname = MEMDUP(ptr,len);
    sf->fname_alloced = true;
    return sf;
}

///////////////////////////////////////////////////////////////////////////////

SubFile_t * FindSubFile ( const SubDir_t *sd, mem_t path )
{
    if ( !sd || !path.len )
	return 0;

    ccp beg = path.ptr;
    ccp end = beg + path.len;
    ccp ptr = end;

    while ( ptr > beg && *ptr != '/' )
	ptr--;
    if ( ptr > beg )
    {
	sd = FindSubDir(sd,MemByStringE(beg,ptr));
	if (!sd)
	    return 0;
	if ( ptr < end )
	    ptr++;
    }
    DASSERT(sd);

    const uint len = end - ptr;
    bool found;
    const uint idx = FindSubFileHelper(&sd->file,MemByStringS(ptr,len),&found);
    noPRINT("\t\tFILE: |%.*s| %s\n",len,ptr,found?"FOUND":"");
    return found ? sd->file.list[idx] : 0;
}

///////////////////////////////////////////////////////////////////////////////

bool RemoveSubFile ( SubDir_t *sd, mem_t path )
{
    // [[2do]] ???
    return false;
}

///////////////////////////////////////////////////////////////////////////////

FILE * OpenSubFile ( const SubDir_t *sd, mem_t path )
{
 #ifdef __APPLE__
    ERROR0(ERR_FATAL,"OpenSubFile() not supported by MAC version.");
    return 0;
 #else
    SubFile_t *sf = FindSubFile(sd,path);
    return sf && sf->data ? fmemopen(sf->data,sf->size,"rb") : 0;
 #endif
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int IterateSubDirHelper ( SubDir_t *sd, SubFileIterator_t *sfi )
{
    DASSERT(sd);
    DASSERT(sfi);

    char *path_end = sfi->path + sizeof(sfi->path) - 1;
    char *path_old = sfi->path_ptr;
    char *path_ptr = StringCopyE(path_old,path_end,sd->dname);

    int stat = 0;
    if (sfi->func)
    {
	sfi->path_ptr = path_ptr;
	const int xstat = sfi->func(sfi,1,sd,0);
	if ( xstat < 0 )
	    return xstat;
	if ( stat < xstat )
	     stat = xstat;
    }

    sfi->depth++;
    char *path_file = path_ptr;
    if ( sd->dname && *sd->dname && path_file < path_end )
	*path_file++ = '/';

    uint i;
    for ( i = 0; i < sd->file.used; i++ )
    {
	SubFile_t *sf = sd->file.list[i];
	sfi->path_ptr = StringCopyE(path_file,path_end,sf->fname);
	if ( sfi->func)
	{
	    const int xstat = sfi->func(sfi,0,sd,sf);
	    if ( xstat < 0 )
		return xstat;
	    if ( stat < xstat )
		 stat = xstat;
	}
    }

    sfi->path_ptr = path_file;
    for ( i = 0; i < sd->dir.used; i++ )
    {
	const int xstat = IterateSubDirHelper(sd->dir.list[i],sfi);
	if ( xstat < 0 )
	    return xstat;
	if ( stat < xstat )
	     stat = xstat;
    }

    sfi->depth--;
    *path_ptr = 0;

    if ( sfi->func)
    {
	sfi->path_ptr = path_ptr;
	const int xstat = sfi->func(sfi,2,sd,0);
	if ( xstat < 0 )
	    return xstat;
	if ( stat < xstat )
	     stat = xstat;
    }

    sfi->path_ptr = path_old;
    return stat;
}

///////////////////////////////////////////////////////////////////////////////

int IterateSubDir ( SubDir_t *sd, SubFileIterator_t *sfi )
{
    DASSERT(sd);
    DASSERT(sfi);

    sfi->depth = 0;
    sfi->path_ptr = sfi->path;
    return IterateSubDirHelper(sd,sfi);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int SaveSubFile
(
    SubFileIterator_t	*sfi,		// iterator data
    uint		mode,		// 0:file, 1:open dir, 2:close dir
    SubDir_t		*dir,		// current directory
    SubFile_t		*file		// NULL or current file
)
{
    if ( !mode && file && file->data )
    {
	DASSERT(file);
	char pathbuf[PATH_MAX];
	ccp path = PathCatPP(pathbuf,sizeof(pathbuf),(ccp)sfi->user_ptr,sfi->path);
	printf(" >SAVE %s\n",path);

	return SaveFile( (ccp)sfi->user_ptr, sfi->path,
			FM_REMOVE|FM_OVERWRITE|FM_MKDIR,
			file->data, file->size, 0 );
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

enumError SaveSubFiles ( SubDir_t *sd, ccp basepath )
{
    SubFileIterator_t it;
    memset(&it,0,sizeof(it));
    it.func = SaveSubFile;
    it.user_ptr = (void*)basepath;
    return IterateSubDir(sd,&it);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			Base64 coding			///////////////
///////////////////////////////////////////////////////////////////////////////

char *TableDecode64BySetup	 = TableDecode64;
const char *TableEncode64BySetup = TableEncode64;

//-----------------------------------------------------------------------------

int ScanOptCoding64 ( ccp arg )
{
    if ( !arg || !*arg )
	return 0;

    static const KeywordTab_t ktab[] =
    {
	{ ENCODE_OFF,		"DEFAULT",	0,	0 },
	{ ENCODE_BASE64,	"STANDARD",	"STD",	0 },
	{ ENCODE_BASE64URL,	"URL",		0,	0 },
	{ ENCODE_BASE64STAR,	"STAR",		0,	0 },
	{ ENCODE_BASE64XML,	"XML",		0,	0 },
	{ 0,0,0,0 }
    };

    const KeywordTab_t *key = ScanKeyword(0,arg,ktab);
    if (!key)
    {
	ERROR0(ERR_SYNTAX,"Invalid --coding mode: '%s'\n",arg);
	return 1;
    }

    opt_coding64 = key->id;
    PRINT("opt_coding64=%u\n",opt_coding64);
    return 0;
}

//-----------------------------------------------------------------------------

EncodeMode_t SetupCoding64 ( EncodeMode_t mode, EncodeMode_t fallback )
{
    PRINT("SetupCoding64(%d,%d) std=%d, url=%d, star=%d, xml=%d\n",
		mode, fallback,
		ENCODE_BASE64, ENCODE_BASE64URL, ENCODE_BASE64STAR, ENCODE_BASE64XML );

    switch(mode)
    {
	case ENCODE_BASE64:
	    PRINT("USE ENCODE_BASE64\n");
	    TableDecode64BySetup = TableDecode64;
	    TableEncode64BySetup = TableEncode64;
	    break;

	case ENCODE_BASE64URL:
	    PRINT("USE ENCODE_BASE64URL\n");
	    TableDecode64BySetup = TableDecode64url;
	    TableEncode64BySetup = TableEncode64url;
	    break;

	case ENCODE_BASE64STAR:
	    PRINT("USE ENCODE_BASE64STAR\n");
	    TableDecode64BySetup = TableDecode64url;
	    TableEncode64BySetup = TableEncode64star;
	    break;

	case ENCODE_BASE64XML:
	    PRINT("USE ENCODE_BASE64XML\n");
	    TableDecode64BySetup = TableDecode64xml;
	    TableEncode64BySetup = TableEncode64xml;
	    break;

	default:
	    PRINT("BASE64 -> FALLBACK\n");
	    return SetupCoding64( fallback == ENCODE_OFF
					? ENCODE_BASE64 : fallback, ENCODE_OFF );
    }
    return mode;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			slot by attribute		///////////////
///////////////////////////////////////////////////////////////////////////////

void DumpSlotInfo
(
    FILE		*f,	    // valid output stream
    int			indent,	    // robust indention of dump
    const slot_info_t	*si,	    // data to dump
    bool		src_valid   // FALSE: si.source if definitly invalid
)
{
    if ( !f || !si )
	return;

    indent = NormalizeIndent(indent);
    if ( src_valid && si->source.ptr && si->source.len )
    fprintf(f,"%*s" "Source:      %.*s\n",
		indent,"", si->source.len, si->source.ptr );

    fprintf(f,
	"%*s" "Slot attrib: %s\n"
	"%*s" "Race info:   %4u [%u] %s\n"
	"%*s" "Arena info:  %4u [%u] %s\n"
	"%*s" "Music info:  %#4x [%u] %s\n"
	,indent,"" ,si->slot_attrib
	,indent,"" ,si->race_slot   ,si->race_mode  ,si->race_info
	,indent,"" ,si->arena_slot  ,si->arena_mode ,si->arena_info
	,indent,"" ,si->music_index ,si->music_mode ,si->music_info
	);
}

//-----------------------------------------------------------------------------

void AnalyzeSlotAttrib ( slot_info_t *si, bool reset_si, mem_t attrib )
{
    DASSERT(si);
    PRINT("AnalyzeSlotAttrib(,%d,\"%.*s\")\n",reset_si,attrib.len,attrib.ptr);

    if (reset_si)
	memset(si,0,sizeof(*si));
    si->source = attrib;

    const uint MAX_ATTRIB = 100;
    mem_t list[MAX_ATTRIB];
    const uint n_attrib = SplitByCharMem(list,MAX_ATTRIB,attrib,',');

    uint i;
    for ( i = 0; i < n_attrib; i++ )
    {
	ccp  src = list[i].ptr;
	uint len = list[i].len;
	PRINT0("%3u: |%.*s|\n",i,len,src);

	switch(len)
	{
	 case 2:
	    if ( src[0] >= '1' && src[0] <= '8' && src[1] >= '1' && src[1] <= '4' )
	    {
		si->race_mode = SIT_MANDATORY;
		si->race_slot = strtoul(src,0,10);
		StringCopySM(si->race_info,sizeof(si->race_info),src,2);
	    }
	    break;

	 case 3:
	    if ( src[0]=='r' && src[1]>='1' && src[1]<='8' && src[2]>='1' && src[2]<='4' )
	    {
		const u16 slot = strtoul(src+1,0,10);
		if (!si->have_31_71)
		{
		    si->race_mode = SIT_RECOMMEND;
		    si->race_slot = slot;
		    StringCopySM(si->race_info,sizeof(si->race_info),src,3);
		}
		else if ( slot == 31 || slot == 71 )
		{
		    si->race_mode = SIT_MANDATORY;
		    si->race_slot = slot;
		    snprintf(si->race_info,sizeof(si->race_info),"%u",slot);
		}
		// else ignore
	    }
	    else if ( src[0]=='a' && src[1]>='1' && src[1]<='2' && src[2]>='1' && src[2]<='5' )
	    {
		si->arena_mode = SIT_RECOMMEND;
		si->arena_slot = strtoul(src+1,0,10);
		StringCopySM(si->arena_info,sizeof(si->arena_info),src,3);
	    }
	    else if ( src[0]=='m' && src[1]>='1' && src[1]<='8' && src[2]>='1' && src[2]<='4' )
	    {
		si->music_mode = SIT_RECOMMEND;
		StringCopySM(si->music_info,sizeof(si->music_info),src,3);
	    }
	    else if (!memcmp(src,"mlc",3))
	    {
		si->music_mode = SIT_MANDATORY;
		StringCopySM(si->music_info,sizeof(si->music_info),src,3);
	    }
	    break;

	 case 4:
	    if ( src[0] == 'm' && src[1] == 'a'
			&& src[2] >= '1' && src[2] <= '2' && src[3] >= '1' && src[3] <= '5' )
	    {
		si->music_mode = SIT_MANDATORY;
		StringCopySM(si->music_info,sizeof(si->music_info),src,4);
	    }
	    else if ( src[0] == 'm' && src[1] == 't'
			&& src[2] >= '1' && src[2] <= '8' && src[3] >= '1' && src[3] <= '4' )
	    {
		si->music_mode = SIT_MANDATORY;
		StringCopySM(si->music_info,sizeof(si->music_info),src+1,3);
		si->music_info[0] = 'm';
	    }
	    break;

	 case 5:
	    if (!memcmp(src,"31+71",5))
	    {
		si->have_31_71 = true;
		if ( si->race_mode != SIT_MANDATORY2
			&& (si->race_slot == 31 || si->race_slot == 71 ))
		{
		    si->race_mode = SIT_MANDATORY;
		    snprintf(si->race_info,sizeof(si->race_info),"%u",si->race_slot);
		}
		else
		{
		    si->race_mode = SIT_MANDATORY2;
		    si->race_slot = 31;
		    StringCopySM(si->race_info,sizeof(si->race_info),src,5);
		}
	    }
	    else if ( !si->arena_mode && !memcmp(src,"arena",5) )
	    {
		si->arena_mode = SIT_GENERIC;
		si->arena_slot = 0;
		StringCopySM(si->arena_info,sizeof(si->arena_info),src,5);
	    }
	    break;
	}
    }
}

//-----------------------------------------------------------------------------

void AnalyzeSlotByName ( slot_info_t *si, bool reset_si, mem_t name )
{
    DASSERT(si);

    if (reset_si)
	memset(si,0,sizeof(*si));

    ccp src = name.ptr;
    ccp slash = memrchr(src,'/',name.len);
    if (slash)
	src = slash+1;

    ccp end = name.ptr + name.len;
    ccp brack1 = memrchr(src,'[',end-src);
    if (brack1)
    {
	ccp brack2 = memchr(brack1,']',end-brack1);
	if (brack2)
	    AnalyzeSlotAttrib(si,false,MemByE(brack1+1,brack2));
    }
}

//-----------------------------------------------------------------------------

void FinalizeSlotInfo ( slot_info_t *si, bool minus_for_empty )
{
    DASSERT(si);

    //--- generic music support

    char music_info[8];
    if (si->race_slot)
	snprintf(music_info,sizeof(music_info),"m%u",(u8)si->race_slot);
    else if (si->arena_slot)
	snprintf(music_info,sizeof(music_info),"ma%u",(u8)si->arena_slot);
    else
	*music_info = 0;

    if ( !si->music_mode && *music_info )
    {
	si->music_mode = SIT_GENERIC;
	StringCopyS(si->music_info,sizeof(si->music_info),music_info);
    }

    if (si->music_info[0])
    {
	const KeywordTab_t *kt = ScanKeyword(0,si->music_info,music_keyword_tab);
	if (kt)
	    si->music_index = kt->id;
    }


    //--- create slot info

    char slot_attrib[20], *dest = slot_attrib, *end = slot_attrib + sizeof(slot_attrib);
    if (si->race_info[0])
	dest = StringCat2E(dest,end,",",si->race_info);
    if (si->arena_info[0])
	dest = StringCat2E(dest,end,",",si->arena_info);
    if ( si->music_info[0] && strcmp(si->music_info,music_info) )
	dest = StringCat2E(dest,end,",",si->music_info);
    if ( dest > slot_attrib )
	StringCopyS(si->slot_attrib,sizeof(si->slot_attrib),slot_attrib+1);


    //--- options

    if (minus_for_empty)
    {
	if (!*si->race_info)  si->race_info[0]  = '-', si->race_info[1] = 0;
	if (!*si->arena_info) si->arena_info[0] = '-', si->arena_info[1] = 0;
	if (!*si->music_info) si->music_info[0] = '-', si->music_info[1] = 0;
    }
}

//-----------------------------------------------------------------------------

slot_info_t GetSlotByAttrib ( mem_t attrib, bool minus_for_empty )
{
    PRINT("GetSlotByAttrib(\"%.*s\")\n",attrib.len,attrib.ptr);

    slot_info_t si = {{0,0}};
    AnalyzeSlotAttrib(&si,false,attrib);
    FinalizeSlotInfo(&si,minus_for_empty);
    return si;
}

//-----------------------------------------------------------------------------

slot_info_t GetSlotByName ( mem_t name, bool minus_for_empty )
{
    PRINT("GetSlotByName(\"%.*s\")\n",name.len,name.ptr);

    slot_info_t si = {{0,0}};
    AnalyzeSlotByName(&si,false,name);
    FinalizeSlotInfo(&si,minus_for_empty);
    return si;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////
