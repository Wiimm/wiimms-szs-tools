
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

#ifndef SZS_LIB_STD_H
#define SZS_LIB_STD_H 1

#define _GNU_SOURCE 1

#include "version.h"
#include "dclib-types.h"

#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "dclib-system.h"
#include "dclib-basics.h"
#include "dclib-file.h"
#include "dclib-debug.h"
#include "lib-dol.h"
#include "file-type.h"
#include "lib-numeric.h"
#include "lib-mkw-def.h"
#include "lib-mkw.h"

#if HAVE_XSRC
 #include "xsrc/x-std.h"
#endif

//
///////////////////////////////////////////////////////////////////////////////
///////////////			debugging helper		///////////////
///////////////////////////////////////////////////////////////////////////////

#undef LOG_SHA1_ENABLED
#undef LOG_SHA1

#if defined(TEST) || defined(DEBUG)
  #define LOG_SHA1_ENABLED 1
#elif HAVE_WIIMM_EXT
  #define LOG_SHA1_ENABLED 0
#else
  #define LOG_SHA1_ENABLED 0
#endif

#if LOG_SHA1_ENABLED
  #define LOG_SHA1(d,s,i) LogSHA1(__FUNCTION__,__FILE__,__LINE__,d,s,i)
#else
  #define LOG_SHA1(d,s,i)
#endif
#define LOG_SHA11(d,s,i) LogSHA1(__FUNCTION__,__FILE__,__LINE__,d,s,i)

void LogSHA1 ( ccp func, ccp file, uint line, cvp data, uint size, ccp info );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  defines			///////////////
///////////////////////////////////////////////////////////////////////////////
// also used by UI

#define CONFIG_FILE		"wiimms-szs-tools.conf"
#define SZS_SETUP_FILE		"wszst-setup.txt"
#define NODE_LIST_FILE		"node-list.bin"
#define EXT_LIST_FILE		"ext-list.bin"
#define CHECK_FILE_SIZE		0x800
#define OPT_PNG_TYPE_CLASS	0x7fef0bff

#if BETA_VERSION && 0
  #define MAX_STGI_LAPS		255
#else
  #define MAX_STGI_LAPS		9
#endif

#define DEF_EPSILON_POS		0.01
#define DEF_EPSILON_ROT		0.01
#define DEF_EPSILON_SCALE	0.01
#define DEF_EPSILON_SIZE	0.01
#define DEF_EPSILON_VEL		0.01
#define DEF_EPSILON_TIME	0.01

#define M1(a) ( (typeof(a))~0 )
#define IS_M1(a) ( (a) == (typeof(a))~0 )

//
///////////////////////////////////////////////////////////////////////////////
///////////////			   Setup			///////////////
///////////////////////////////////////////////////////////////////////////////

void SetupColors();
void SetupLib ( int argc, char ** argv, ccp tname, ccp tvers, ccp ttitle );

typedef enumError (*check_opt_func) ( int argc, char ** argv, bool mode );
enumError CheckEnvOptions2 ( ccp varname, check_opt_func func );

void NormalizeOptions
(
    uint	log_level	// >0: print PROGRAM_NAME and pathes
);

ccp GetCtLeInfo();

const KeywordTab_t * CheckCommandHelper
	( int argc, char ** argv, const KeywordTab_t * key_tab );

extern const char text_logo[];
extern void (*print_title_func) ( FILE * f );

//
///////////////////////////////////////////////////////////////////////////////
///////////////                  error interface                ///////////////
///////////////////////////////////////////////////////////////////////////////

enum
{
    ERR_CACHE_USED	= ERU_SOURCE_FOUND,
    ERR_ENCODING_WARN	= ERR_SUBJOB_WARNING,

    ERR_INVALID_IFORM	= ERU_ERROR1_01,
    ERR_INVALID_FFORM	= ERU_ERROR1_02,

    ERR_BZIP2		= ERU_ERROR2_01,
    ERR_LZMA		= ERU_ERROR2_02,
    ERR_XZ		= ERU_ERROR2_03,
    ERR_PNG		= ERU_ERROR2_04,
};

//-----------------------------------------------------------------------------

int FixExitStatus ( int stat );
void ExitFixed ( int stat )  __attribute__ ((noreturn));

ccp LibGetErrorName ( int stat, ccp ret_not_found );
ccp LibGetErrorText ( int stat, ccp ret_not_found );

void SetupPager(void);
static inline void ClosePager() { CloseStdoutToPager(); }

struct InfoUI_t;
void PrintHelpColor ( const struct InfoUI_t * iu );

extern volatile int verbose;
static inline bool ErrorLogDisabled()
	{ return verbose <= -3; }
static inline bool ErrorLogEnabled()
	{ return verbose >= -2; }

//
///////////////////////////////////////////////////////////////////////////////
///////////////			compatibility			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[compatible_t]]

typedef enum compatible_t
{
    COMPAT_0,		// legacy mode
    COMPAT_1_23,	// BMG colors			=> opt_bmg_colors = 0
    COMPAT_1_39,	// BMG inline support		=> opt_bmg_inline_attrib = false
    COMPAT_1_44,	// new BMG escapes + attributes	=> opt_bmg_old_escapes = true
    COMPAT_1_46,	// KMP flags			=> opt_kmp_flags = false;
    COMPAT_2_08,	// BMG slots and sections	=> opt_bmg_use_* = false
    COMPAT_2_40,	// BMG @?, new syntax		=> opt_bmg_use_new_cond = false
    COMPAT_CURRENT,	// current version

    COMPAT__N		// number of elements of 'compatible_info'
}
compatible_t;

//-----------------------------------------------------------------------------
// [[compatible_info_t]]

typedef struct compatible_info_t
{
    compatible_t compatible;	// selector
    u32		revision;	// revision number
    char	version[8];	// version string of format "1.23" or "1.23a"
}
compatible_info_t;

//-----------------------------------------------------------------------------

extern compatible_t compatible; // = COMPAT_CURRENT
extern const compatible_info_t compatible_info[COMPAT__N];

const compatible_info_t * ScanCompatible ( ccp arg );
int ScanOptCompatible ( ccp arg );
char * PrintOptCompatible(void); // circ-buf

//-----------------------------------------------------------------------------

uint GetNumericVersion(void);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			 warn_mode_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[warn_mode_t]]

typedef enum warn_mode_t
{
    //--- general mode

    WARN_INVALID_OFFSET		= 0x0000001,


    //--- hidden options

    WARN_LOG		= 1<<28,	// enable kcl trace log to stdout
     WARN_M_HIDDEN	 = WARN_LOG,

    WARN_F_HIDE	= 1<<29,	// help flag to hide modes in PrintWarnMode()


    //--- test mode

    WARN_TEST		= 1<<30,	// reserved for testing


    //--- summaries

    WARN_M_DEFAULT	= WARN_INVALID_OFFSET	// default settings
			| 0,

    WARN_M_ALL		= WARN_INVALID_OFFSET	// all relevant bits
			| 0,

    WARN_M_PRINT	= WARN_M_ALL		// all relevant bits for obj output
			& ~WARN_M_HIDDEN,
}
warn_mode_t;

//-----------------------------------------------------------------------------

extern warn_mode_t WARN_MODE;

int  ScanOptWarn ( ccp arg );
uint PrintWarnMode( char *buf, uint bufsize, warn_mode_t mode );
ccp  GetWarnMode();

//
///////////////////////////////////////////////////////////////////////////////
///////////////			warn level			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[WarnLevel_t]] Same as CTW: WarnLevel

typedef enum WarnLevel_t
{
    WLEVEL_UNKNOWN,	// unknown
    WLEVEL_NA,		// not applicable
    WLEVEL_OK,		// ok
    WLEVEL_FIXED,	// fixed
    WLEVEL_HINT,	// hint
    WLEVEL_WARN,	// warning
    WLEVEL_ERROR,	// error
    WLEVEL_FAIL,	// failed
    WLEVEL_FATAL,	// fatal error
    WLEVEL_INVALID,	// invalid value

    WLEVEL__N
}
__attribute__ ((packed)) WarnLevel_t;

extern const char WarnLevelNameLo[WLEVEL__N][8]; // lower case
extern const char WarnLevelNameUp[WLEVEL__N][8]; // upper case

//
///////////////////////////////////////////////////////////////////////////////
///////////////			warn_enum_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[warn_enum_t]] [[warn_bits_t]]
// order is important, compare warn_szs_name[] and ct.wiimm.de "warning bits"

typedef enum warn_enum_t
{
    WARNSZS_ITEMPOS,		// item-pos bug
    WARNSZS_SELF_ITPH,		// self linked ITPT route
    WARNSZS_NO_MINIMAP,		// no minimap found
    WARNSZS__N
}
warn_enum_t;

typedef uint warn_bits_t;

extern const ccp warn_szs_name[WARNSZS__N];
ccp GetWarnSZSNames ( warn_bits_t ws, char sep );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			sub file system			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[SubFile_t]]

typedef struct SubFile_t
{
    ccp			fname;		// filename
    u8			*data;		// pointer to data
    uint		size;		// file size
    uint		type;		// file type (user data)

    bool		fname_alloced;	// true: 'fname' is alloced
    bool		data_alloced;	// true: 'data' is alloced
}
SubFile_t;

//-----------------------------------------------------------------------------
// [[SubFileList_t]]

typedef struct SubFileList_t
{
    SubFile_t		**list;		// file list
    uint		used;		// used elements of 'list'
    uint		size;		// allooced elements of 'list'
}
SubFileList_t;

///////////////////////////////////////////////////////////////////////////////
// [[SubDirList_t]]

struct SubDir_t;

typedef struct SubDirList_t
{
    struct SubDir_t	**list;		// dir list
    uint		used;		// used elements of 'list'
    uint		size;		// allooced elements of 'list'
}
SubDirList_t;

//-----------------------------------------------------------------------------
// use [[SubDir_t]] as top level of a file system

typedef struct SubDir_t
{
    ccp			dname;		// directory name
    bool		dname_alloced;	// true: 'dname' is alloced
    SubDirList_t	dir;		// sub-directories
    SubFileList_t	file;		// sub-files
}
SubDir_t;

///////////////////////////////////////////////////////////////////////////////

static inline void InitializeSubDirList ( SubDirList_t *s )
	{ DASSERT(s); memset(s,0,sizeof(*s)); }
static inline void InitializeSubDir ( SubDir_t *s )
	{ DASSERT(s); memset(s,0,sizeof(*s)); s->dname = EmptyString; }
static inline void InitializeSubFileList ( SubFileList_t *s )
	{ DASSERT(s); memset(s,0,sizeof(*s)); }
static inline void InitializeSubFile ( SubFile_t *s )
	{ DASSERT(s); memset(s,0,sizeof(*s)); }

void ResetSubDirList  ( SubDirList_t *sdl );
void ResetSubDir      ( SubDir_t *sd );
void ResetSubFileList ( SubFileList_t *sfl );
void ResetSubFile     ( SubFile_t *sf );

void DumpSubDir      ( FILE *f, int indent, const SubDir_t *sd );
void DumpSubDirList  ( FILE *f, int indent, const SubDirList_t *sdl );
void DumpSubFileList ( FILE *f, int indent, const SubFileList_t *sfl );

///////////////////////////////////////////////////////////////////////////////

SubDir_t  * FindSubDir  ( const SubDir_t *sd, mem_t path );
SubFile_t * FindSubFile ( const SubDir_t *sd, mem_t path );

SubDir_t  * InsertSubDir  ( SubDir_t *sd, mem_t path, bool *new_dir  );
SubFile_t * InsertSubFile ( SubDir_t *sd, mem_t path, bool *new_file );

bool RemoveSubDir  ( SubDir_t *sd, mem_t path );
bool RemoveSubFile ( SubDir_t *sd, mem_t path );

FILE * OpenSubFile ( const SubDir_t *sd, mem_t path );

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// [[SubFileFunc]]

struct SubFileIterator_t;

typedef int (*SubFileFunc)
(
    struct
    SubFileIterator_t	*sfi,		// iterator data
    uint		mode,		// 0:file, 1:open dir, 2:close dir
    SubDir_t		*dir,		// current directory
    SubFile_t		*file		// NULL or current file
);

///////////////////////////////////////////////////////////////////////////////
// [[SubFileIterator_t]]

typedef struct SubFileIterator_t
{
    SubFileFunc	func;		// function to call
    int		user_int;	// user int
    void	*user_ptr;	// user pointer

    uint	depth;		// dir depth
    char	*path_ptr;	// pointer to NULL term of 'fullpath'
    char	path[2000];	// full path
}
SubFileIterator_t;

///////////////////////////////////////////////////////////////////////////////

int IterateSubDir ( SubDir_t *sd,  SubFileIterator_t *sfi );
enumError SaveSubFiles ( SubDir_t *sd, ccp basepath );

//
///////////////////////////////////////////////////////////////////////////////
///////////////		    [[obsolete+]] old_container		///////////////
///////////////////////////////////////////////////////////////////////////////
// old [[container]] will be replaced by Container_t & ContainerData_t
// [[DataContainer_t]]

#ifdef TEST // [[container]] [[obsolete+]]
 #define USE_NEW_CONTAINER_MDL	1
 #define USE_NEW_CONTAINER_PAT	1
 #define USE_NEW_CONTAINER_CTC	1
#elif HAVE_WIIMM_EXT
 #define USE_NEW_CONTAINER_MDL	1
 #define USE_NEW_CONTAINER_PAT	1
 #define USE_NEW_CONTAINER_CTC	1
#else // activated in 2020-01 -> [[obsilete]] at mid 2020
 #define USE_NEW_CONTAINER_MDL	1
 #define USE_NEW_CONTAINER_PAT	1
 #define USE_NEW_CONTAINER_CTC	1
#endif

///////////////////////////////////////////////////////////////////////////////

typedef struct DataContainer_t
{
    Container_t		container;	// low level container

    const u8		*data;		// data
    uint		size;		// size of 'data'
    int			ref_count;	// reference counter
    bool		data_alloced;	// true: free data
    bool		dc_alloced;	// true: container itself is alloced

} DataContainer_t;

///////////////////////////////////////////////////////////////////////////////

#if USE_NEW_CONTAINER_MDL
 typedef ContainerData_t ContainerMDL_t;
#else
 typedef DataContainer_t ContainerMDL_t;
#endif

#if USE_NEW_CONTAINER_PAT
 typedef ContainerData_t ContainerPAT_t;
#else
 typedef DataContainer_t ContainerPAT_t;
#endif

#if USE_NEW_CONTAINER_CTC
 typedef ContainerData_t ContainerCTC_t;
#else
 typedef DataContainer_t ContainerCTC_t;
#endif

///////////////////////////////////////////////////////////////////////////////

DataContainer_t * UseSubContainer
(
    DataContainer_t	*dc,		// old_container to reuse, if
					// InDataContainer(dc,data) && ref:count == 1
    ContainerData_t	*cdata_src	// NULL or container data source
);

///////////////////////////////////////////////////////////////////////////////

DataContainer_t * NewContainer
(
    DataContainer_t	*dc,		// old_container to reuse, if
					// InDataContainer(dc,data) && ref:count == 1
    const void		*data,		// data to copy/move/link
    uint		size,		// size of 'data'
    CopyMode_t		mode		// copy mode on creation
);

///////////////////////////////////////////////////////////////////////////////

DataContainer_t * DupContainer
(
    DataContainer_t	*dup_dc,	// old_container to reuse, if
					// InDataContainer(dc,data) && ref:count == 1
					//  => complete dc->data is copied
    const void		*data,		// data to copy/move/link
    uint		size,		// size of 'data'
    bool		include_behind,	// on creation: include 'dc->data'
					// behind 'data+size' if possible
    const void		**new_data	// not NULL: store new data pointer here
);

///////////////////////////////////////////////////////////////////////////////

DataContainer_t * AllocContainer
(
    // increments the reference counter and returns 'ct'

    DataContainer_t	*dc		// NULL or valid old_container
);

///////////////////////////////////////////////////////////////////////////////

DataContainer_t * FreeContainer
(
    // decrements the reference counter and returns 'NULL'
    // if reference counter == 0: free data and/or ct

    DataContainer_t	*dc		// NULL or valid old_container
);

///////////////////////////////////////////////////////////////////////////////

bool InDataContainer
(
    // return true, if ptr is in old_container

    const DataContainer_t *dc,		// NULL or valid old_container
    const void		  *ptr		// NULL or pointer to test
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			string helpers			///////////////
///////////////////////////////////////////////////////////////////////////////

int NintendoCmp ( ccp path1, ccp path2 );

void PrintNumF
(
    FILE		*f,		// output stream
    const u8		*data,		// valid pointer to data
    uint		size,		// size of 'data'
    ccp			type_str	// NULL or type string
);

// insert "_d" before extension
char * Insert_d ( char * buf, uint bufsize, ccp path );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			word compare functions		///////////////
///////////////////////////////////////////////////////////////////////////////

uint CreateHaystackListWCMP
(
    char	* buf,		// valid pointer to output buffer
    uint	buf_size,	// soue of 'buf'
    ccp		* source	// NULL terminated list with source Strings
);

uint CreateHaystackWCMP
(
    char	* buf,		// valid pointer to output buffer
    uint	buf_size,	// soue of 'buf'
    ccp		source		// source String
);

uint CreateNeedleWCMP
(
    char	* buf,		// valid pointer to output buffer
    uint	buf_size,	// soue of 'buf'
    ccp		source		// NULL terminated list with source Strings
);

bool FindWCMP
(
    ccp		haystack,	// search in this string
    ccp		needle,		// try to find this string
    uint	flags,		// bit field:
				//	bit-0 set = 1: 'haystack' already in WCMP format
				//	bit-1 set = 2: 'needle' already in WCMP format
    char	* tempbuf,	// NULL or fast buffer for temporary WCMP conversions
    uint	tempbuf_size	// size of 'buf'
);

///////////////////////////////////////////////////////////////////////////////

typedef struct search_filter_t
{
    ccp		string;		// alloced wcmp string
    int		mode;		// user defined mode

} search_filter_t;

search_filter_t * AllocSearchFilter ( uint n );
void FreeSearchFilter ( search_filter_t *f );

//
///////////////////////////////////////////////////////////////////////////////
///////////////		     string substitutions		///////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct SubstString_t
{
	char c1, c2;		// up to 2 codes (lower+upper case)
	bool allow_slash;	// true: allow slash in replacement
	ccp  str;		// pointer to string

} SubstString_t;

//-----------------------------------------------------------------------------

int ScanEscapeChar ( ccp arg );

char * SubstString
(
    char		* buf,		// destination buffer
    size_t		bufsize,	// size of 'buf'
    SubstString_t	* tab,		// replacement table
    ccp			source,		// source text
    int			* count,	// not NULL: number of replacements
    char		escape_char	// the escape character, usually '%'
);

int SubstDest
(
    char		* buf,		// destination buffer
    size_t		bufsize,	// size of 'buf'
    ccp			source,		// source text
    ccp			dest,		// destination path
    ccp			dest_fname,	// if 'dest' has no file part
					// then extract filename from here or source
    ccp			default_ext,	// NULL or default extension (e.g. '.ext')
    bool		dest_is_dir	// destination is a directory
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			file support			///////////////
///////////////////////////////////////////////////////////////////////////////

#undef CF_PALETTE // for windows

typedef enum cmd_flags_t
{
    CF_IMAGE	= 0x0001,	// valid if scanning image type
    CF_PALETTE	= 0x0002,	// img format has palette
    CF_AUTO	= 0x0004,	// auto mode

} cmd_flags_t;

//-----------------------------------------------------------------------------

#define PNG_MAGIC8_NUM	0x89504e470d0a1a0aull

//-----------------------------------------------------------------------------

ccp GetNameFF		( file_format_t ff_arch, file_format_t ff_data );
ccp GetNameFFv		( file_format_t ff1, file_format_t ff2, int version );
ccp GetExtFF		( file_format_t ff1, file_format_t ff2 );
ccp GetBinExtFF		( file_format_t ff );
ccp GetTextExtFF	( file_format_t ff );
ccp GetMagicExtFF	( file_format_t ff1, file_format_t ff2 );
ccp GetMagicStringFF	( file_format_t ff );

file_format_t GetBinFF ( file_format_t ff );
file_format_t GetTextFF ( file_format_t ff );
file_format_t GetByNameFF ( ccp name );

file_format_t GetByMagicFF
(
    const void	*data,		// pointer to data
    uint	data_size,	// size of data
    uint	file_size	// NULL or total size of file
);

file_format_t GetFileTypeByMagic
(
    // returns FF_INVALID on read err, or detected FF.

    ccp			fname,		// file to open
    FileAttrib_t	* fatt		// not NULL: store file attributes
);

const file_type_t * GetFileTypeByExt
(
    ccp		ext,		// NULL or file extension with optional preceeding point
    bool	need_magic	// filter only file types with magic
);

const file_type_t * GetFileTypeBySubdir
(
    ccp		path,		// NULL or file extension with optional preceeding point
    bool	need_magic	// filter only file types with magic
);

const file_type_t * GetFileTypeFF ( file_format_t ff );

ff_attrib_t GetAttribFF ( file_format_t ff );
ff_attrib_t GetAttribByMagicFF
(
    const void		* data,		// valid pointer to data
    uint		data_size,	// size of 'data'
    file_format_t	*ff		// not NULL: store file format here
);

int GetVersionFF
(
    // returns -1 for unknown, or the version number

    file_format_t	fform,		// valid firl format
    const void		*data,		// data pointer
    uint		data_size,	// size of available data
    char		*res_suffix	// not NULL: return 1 char as suffix, or 0
);

int GetVersionByMagic
(
    // returns -1 for unknown, or the version number

    const void		*data,		// data pointer
    uint		data_size,	// size of available data
    char		*res_suffix	// not NULL: return 1 char as suffix, or 0
);

bool IsBRSUB		( file_format_t ff );
bool IsByMagicBRSUB	( const void * data, uint data_size );

bool CanBeTrackFF	( file_format_t ff );
bool IsArchiveFF	( file_format_t ff );
bool IsCompressFF	( file_format_t ff );
bool IsTrackCompressFF	( file_format_t ff );
bool IsTextFF		( file_format_t ff );
bool IsGeoHitKartFF	( file_format_t ff );
bool IsGeoHitObjFF	( file_format_t ff );

bool IsYazFF			( file_format_t ff );
int  GetYazVersionFF		( file_format_t ff );
bool IsCompressedFF		( file_format_t ff );
file_format_t GetYazFF		( file_format_t ff );
file_format_t GetCompressedFF	( file_format_t ff );

void SetCompressionFF
(
    file_format_t	ff_arch,	// if >=0: set 'opt_fform'
    file_format_t	ff_compr	// if >=0: set 'fform_compr'
					//         and 'fform_compr_force'
);

static inline bool IsTplFF ( file_format_t ff )
	{ return ff == FF_TPL || ff == FF_TPLX || ff == FF_CUPICON; }

file_format_t IsImageFF
(
    // returns the normalized FF for a valid image or NULL (=FF_UNKNOWN)

    file_format_t	fform,		// file format to check
    bool		allow_png	// true: allow FF_PNG
);

file_format_t GetImageFFByFName
(
    // returns the normalized FF for a valid image or NULL (=FF_UNKNOWN)

    ccp			fname,		// NULL or filename to analyse extension
    file_format_t	default_fform,	// use this as default/fallback
    bool		allow_png	// true: allow FF_PNG
);

file_format_t GetImageFF
(
    file_format_t	fform1,		// if a valid image ff, use this one
    file_format_t	fform2,		// if a valid image ff, use this one
    ccp			path,		// not NULL and exists: use file format
    file_format_t	default_fform,	// use this as default, if valid ff
    bool		allow_png,	// true: allow FF_PNG
    file_format_t	not_found	// return this, if all other fails
);

///////////////////////////////////////////////////////////////////////////////

#define BRSUB_MODE2(a,b)   ( (a) << 24 | (b) << 5 )
#define BRSUB_MODE3(a,b,c) ( (a) << 24 | (b) << 5 | (c) )

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

enumError OpenFILE
(
    File_t		* f,		// file structure
    bool		initialize,	// true: initialize 'f'
    ccp			fname,		// file to open
    bool		ignore_no_file,	// ignore if file doeas not exists
					// and return warning ERR_NOT_EXISTS
    bool		ignore_limit	// ignore opt_max_file_size
);

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
);

FileMode_t GetFileModeByOpt
(
    int			testmode,	// test mode
    ccp			fname1,		// NULL or dest filename
    ccp			fname2		// NULL or source filename
);

enumError CreateFileOpt
(
    File_t		* f,		// file structure
    bool		initialize,	// true: initialize 'f'
    ccp			fname,		// file to open
    bool		testmode,	// true: don't open
    ccp			srcfile		// NULL or src file name
					// if fname == srcfile: overwrite enabled
);

enumError CreateFileOptAppend
(
    File_t		* f,		// file structure
    bool		initialize,	// true: initialize 'f'
    ccp			fname,		// file to open
    bool		testmode,	// true: don't open
    ccp			srcfile		// NULL or src file name
					// if fname == srcfile: overwrite enabled
);

enumError AppendFILE
(
    File_t		* f,		// file structure
    bool		initialize,	// true: initialize 'f'
    ccp			fname,		// file to open
    bool		testmode,	// true: don't open
    bool		create_path	// create path if necessary
);

///////////////////////////////////////////////////////////////////////////////

FILE * CreateAnalyzeFile();
void CloseAnalyzeFile();

///////////////////////////////////////////////////////////////////////////////

int ScanOptChdir ( ccp arg );

char * SplitSubPath
(
    char		* buf,		// destination buffer
    size_t		buf_size,	// size of 'buf'
    ccp			path		// source path, if NULL: use 'buf'
);

enumError OpenReadFILE
(
    ccp			path1,		// NULL or part #1 of path
    ccp			path2,		// NULL or part #2 of path
    bool		silent,		// suppress all error messages
    u8			** res_data,	// store alloced data here
    uint		* res_size,	// not NULL: store data size
    ccp			* res_fname,	// not NULL: store alloced filename
    FileAttrib_t	* res_fatt	// not NULL: store file attributes
);

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
);


enumError WriteFILE
(
    FILE		*f,		// valid file for output
    ccp			path1,		// NULL or part #1 of path for messages
    ccp			path2,		// NULL or part #2 of path for messages
    const void		* data,		// data to write
    uint		data_size	// size of 'data'
);

enumError SaveFILE
(
    ccp			path1,		// NULL or part #1 of path
    ccp			path2,		// NULL or part #2 of path
    bool		overwrite,	// true: force overwriting
    const void		* data,		// data to write
    uint		data_size,	// size of 'data'
    FileAttrib_t	* fatt		// not NULL: set timestamps using this attribs
);

enumError SaveFILE2
(
    FILE		* f,		// output file, if NULL use path1+path2 
    ccp			path1,		// NULL or part #1 of path
    ccp			path2,		// NULL or part #2 of path
    bool		overwrite,	// true: force overwriting
    const void		* data,		// data to write
    uint		data_size,	// size of 'data'
    FileAttrib_t	* fatt		// not NULL: set timestamps using this attribs
);

uint IsolateFilename
(
    // return length of found and copied string

    char		* buf,		// destination buffer
    size_t		bufsize,	// size of 'buf'
    ccp			path,		// path to analyze
    file_format_t	fform		// not NULL: remove related file extension
);

///////////////////////////////////////////////////////////////////////////////

bool IsAutoAddAvailable();
void DefineAutoAddPath ( ccp path );
s64  FindAutoAdd ( ccp fname, ccp ext, char *buf, uint buf_size );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			CompressManager_t		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[CompressManager_t]]

typedef struct CompressManager_t
{
    //--- source

    file_format_t fform;	// file format of source
    cvp		src_data;	// source data (BZIP2 or LZMA), never NULL
    uint	src_size;	// size of 'src_data'

    //--- decoded

    u8		*data;		// NULL or data, alloced if not part of 'src_data'
    uint	size;		// size of 'data'
}
CompressManager_t;

//-----------------------------------------------------------------------------

enumError DecompressManager
(
    CompressManager_t	*mgr	// manager data
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			repair magic			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[repair_mode_t]]

typedef enum repair_mode_t
{
    REPMD_OFF,		// repair is OFF
    REPMD_ANALYZE,	// repair is ON, but only for analysis
    REPMD_REPAIR,	// repair is ON & replace correct magic

    REPMD_F_FNAME_FIRST	= 0x10,	// flag: Use file extension first

    REPMD_R_FALLBACK	= 0x20,	// result: 'ff_fallback' used
    REPMD_R_FNAME	= 0x40,	// result: file extension used

    REPMD_M_MODES	= 0x07,	// mask for modes
    REPMD_M_FLAGS	= 0x10,	// mask for flags
    REPMD_M_RESULTS	= 0x60,	// mask for flags
}
repair_mode_t;

//-----------------------------------------------------------------------------

extern repair_mode_t	repair_magic;
extern repair_mode_t	repair_magic_flags;

int ScanOptRepairMagic ( ccp arg );
static inline bool IsRepairMagic() { return repair_magic > REPMD_OFF; }

///////////////////////////////////////////////////////////////////////////////
// [[repair_ff_t]]

typedef struct repair_ff_t
{
    repair_mode_t	mode;		// repair mode without flags
    repair_mode_t	flags;		// result flags
    file_format_t	fform;		// file format, original or repaired
    char		magic[8];	// magic, original or repaired
    uint		magic_len;	// length of 'magic'

    char		orig_magic[8];	// magig of original
    char		file_ext[8];	// relevant file extension
}
repair_ff_t;

//-----------------------------------------------------------------------------

repair_ff_t * ResetRepairMagic ( repair_ff_t * data, repair_ff_t * fallback );
ccp GetRepairMagicInfo ( const repair_ff_t *rep );

//-----------------------------------------------------------------------------

file_format_t AnalyseMagic
(
    // returns rep->fform

    repair_ff_t	*rep,		// not NULL: result, will be initalized
    const void	*data,		// pointer to data
    uint	data_size,	// size of data
    uint	file_size,	// NULL or total size of file
    file_format_t ff_fallback,	// FF_UNKNOWN or fallback FF if GetByMagicFF() fails
    ccp		fname,		// filename to analyse if GetByMagicFF() fails
    bool	fname_first	// analyse the filename first.
);

//-----------------------------------------------------------------------------

file_format_t RepairMagic
(
    // returns rep->fform

    repair_ff_t	*rep,		// not NULL: result, will be initalized
    void	*data,		// pointer to data, magic will be replaced
    uint	data_size,	// size of data
    uint	file_size,	// NULL or total size of file
    file_format_t ff_fallback,	// FF_UNKNOWN or fallback FF if GetByMagicFF() fails
    ccp		fname,		// filename to analyse if GetByMagicFF() fails
    bool	fname_first	// analyse the filename first.
);

//-----------------------------------------------------------------------------

file_format_t AnalyseMagicByOpt
(
    // returns rep->fform
    // mode & fname_first depends on --repair-magic, but is never REPMD_REPAIR

    repair_ff_t	*rep,		// not NULL: result, will be initalized
    const void	*data,		// pointer to data
    uint	data_size,	// size of data
    uint	file_size,	// NULL or total size of file
    file_format_t ff_fallback,	// FF_UNKNOWN or fallback FF if GetByMagicFF() fails
    ccp		fname		// filename to analyse if GetByMagicFF() fails
);

//-----------------------------------------------------------------------------

file_format_t RepairMagicByOpt
(
    // returns rep->fform
    // mode & fname_first depends on --repair-magic

    repair_ff_t	*rep,		// not NULL: result, will be initalized
    void	*data,		// pointer to data, magic will be replaced
    uint	data_size,	// size of data
    uint	file_size,	// NULL or total size of file
    file_format_t ff_fallback,	// FF_UNKNOWN or fallback FF if GetByMagicFF() fails
    ccp		fname		// filename to analyse if GetByMagicFF() fails
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    file class			///////////////
///////////////////////////////////////////////////////////////////////////////

typedef enum patch_file_t
{
    PFILE_NO_FILENAME	= 0x0001,  // no filename or an empty filename specified
    PFILE_UNKNOWN	= 0x0002,  // unknown (not supported) file type

    //--- the following values are always >PFILE_UNKNOWN

    PFILE_COURSE	= 0x0010,  // KMP file 'course.txt', unsure if KMP or KCL
    PFILE_COURSE_KMP	= 0x0020,  // KMP file 'course.{kmp,kmp.txt,txt}'
    PFILE_COURSE_KCL	= 0x0040,  // KCL file 'course.{kcl,kcl.txt,txt}'
    PFILE_OTHER_KCL	= 0x0080,  // other KCL files

    PFILE_MODEL		= 0x0100,  // BRRES file 'course_[d_]model.brres'
    PFILE_MINIMAP	= 0x0200,  // BRRES file 'map_model.brres'
    PFILE_VRCORN	= 0x0400,  // BRRES file 'vrcorn_model.brres'
    PFILE_OTHER_BRRES	= 0x0800,  // other BRRES files


    //--- flags

    PFILE_F_DEFAULT	= 0x1000,  // flags for default settings
    PFILE_F_LOG		= 0x2000,  // enable logging
    PFILE_F_LOG_ALL	= 0x4000,  // enable logging -> inherit by KCP+KMP+MDL
    PFILE_F_HIDE	= 0x8000,  // help flag to hide modes in PrintFileClassInfo()


    //--- masks

    PFILE_M_NONE	= PFILE_UNKNOWN
			| PFILE_NO_FILENAME,

    PFILE_M_KCL	= PFILE_COURSE_KCL
			| PFILE_OTHER_KCL,

    PFILE_M_BRRES	= PFILE_MODEL
			| PFILE_MINIMAP
			| PFILE_VRCORN
			| PFILE_OTHER_BRRES,

    PFILE_M_TRACK	= PFILE_COURSE_KMP	// mask for 'TRACK' keyword
			| PFILE_COURSE_KCL
			| PFILE_MODEL
			| PFILE_MINIMAP
			| PFILE_VRCORN,

    PFILE_M_ALL	= PFILE_M_NONE
			| PFILE_COURSE
			| PFILE_COURSE_KMP
			| PFILE_M_KCL
			| PFILE_M_BRRES
			| PFILE_F_LOG
			| PFILE_F_LOG_ALL,
			// not PFILE_F_HIDE

    PFILE_M_SILENT	= PFILE_M_TRACK,

    PFILE_M_DEFAULT	= PFILE_M_SILENT
			| PFILE_F_LOG,

} patch_file_t;

//-----------------------------------------------------------------------------

extern patch_file_t PATCH_FILE_MODE;	// = PFILE_M_DEFAULT|PFILE_F_DEFAULT
extern int disable_patch_on_load;	// if >0: disable patching after loading

int ScanOptPatchFiles ( ccp arg );
void SetPatchFileModeReadonly();	// switch to readonly default flags (silent),
					// if not set by user

uint PrintFileClassInfo ( char *buf, uint bufsize, patch_file_t mode );
ccp GetFileClassInfo();

patch_file_t GetFileClass
(
    file_format_t	fform,		// not FF_UNKNOWN: check file type
    ccp			fname		// NULL or file name to check
);

bool PatchFileClass
(
    file_format_t	fform,		// not FF_UNKNOWN: check file type
    ccp			fname		// NULL or file name to check
);

extern int patch_action_log_disabled;
bool PATCH_ACTION_LOG ( ccp action, ccp object, ccp format, ... )
	__attribute__ ((__format__(__printf__,3,4)));

//
///////////////////////////////////////////////////////////////////////////////
///////////////			   image formats		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[image_format_t]]

typedef enum image_format_t
{
    //--- nintendo formats

    IMG_I4		= 0x00,
    IMG_I8		= 0x01,
    IMG_IA4		= 0x02,
    IMG_IA8		= 0x03,
    IMG_RGB565		= 0x04,
    IMG_RGB5A3		= 0x05,
    IMG_RGBA32		= 0x06,
    IMG_C4		= 0x08,
    IMG_C8		= 0x09,
    IMG_C14X2		= 0x0a,
    IMG_CMPR		= 0x0e,

    IMG_N_INTERN,			// number intern formats


    //--- non nintendo formats

    IMG_X_AUTO		= 0x7c00,	// auto format (one of below)

    IMG_X_GRAY,				// 2 bytes: gray with alpha
    IMG_X_RGB,				// 4 bytes: RGB with alpha

			// palette formats: 2 bytes index into 4 bytes palette
    IMG_X_PAL4,				//  4 bit index (max 16 palette entries)
    IMG_X_PAL8,				//  8 bit index (max 256 palette entries)
    IMG_X_PAL14,			// 14 bit index (max 16386 palette entries)
    IMG_X_PAL,				// one of above


    //--- valid X formats as range

    IMG_X__MIN		= IMG_X_GRAY,
    IMG_X__MAX		= IMG_X_PAL,

    IMG_X_PAL__MIN	= IMG_X_PAL4,
    IMG_X_PAL__MAX	= IMG_X_PAL,


    //--- special values

    IMG_INVALID		= -1,		// no image

} image_format_t;

//-----------------------------------------------------------------------------

extern const KeywordTab_t cmdtab_image_format[];
image_format_t ScanImageFormat ( ccp arg );
ccp GetImageFormatName ( image_format_t iform, ccp unknown_value );

#define MAX_MIPMAPS 20

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    palette formats		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[palette_format_t]]

typedef enum palette_format_t
{
    PAL_IA8		= 0x00,
    PAL_RGB565		= 0x01,
    PAL_RGB5A3		= 0x02,

    PAL_X_RGB		= 0x7c01,

    PAL_N,				// number of formats
    PAL_AUTO		= PAL_N,	// auto format
    PAL_DEFAULT		= PAL_RGB5A3,	// default value
    PAL_INVALID		= -1,		// no image

} palette_format_t;

//-----------------------------------------------------------------------------

extern const KeywordTab_t cmdtab_palette_format[];
palette_format_t ScanPaletteFormat ( ccp arg );
ccp GetPaletteFormatName ( palette_format_t pform, ccp unknown_value );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			sort mode			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[SortMode_t]]

typedef enum SortMode_t
{
    SORT_NONE,		// do not sort

    SORT_INAME,		// sort with strcasecmp()
    SORT_OFFSET,	// sort by offset and size
    SORT_SIZE,		// sort by size (and offset)

    SORT_U8,		// sort like Nintendo sort U8 archives
    SORT_PACK,		// sort like Nintendo sort U8 archives (inexact)
    SORT_BRRES,		// sort like Nintendo sort BRRES archives
    SORT_BREFF,		// sort like Nintendo sort BREFF and BREFT archives

    SORT_AUTO		// auto sorting, using 'fform_arch'

} SortMode_t;

//-----------------------------------------------------------------------------

SortMode_t GetSortMode
(
    SortMode_t		sort_mode,	// wanted sort mode
    file_format_t	fform,		// use it if 'sort_mode==SORT_AUTO'
    SortMode_t		default_mode	// default if all other fails
);

//-----------------------------------------------------------------------------

ccp GetSortName ( int id, ccp res_not_found );
int ScanOptSort ( ccp arg );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			slot mode			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[SlotMode_t]]

typedef enum SlotMode_t
{
    //--- no flag set

    SLOTMD_NONE		= 0,


    //--- jobs

    SLOTMD_RM_ICE	= 0x0001,   // remove ice.brres from SZS+KMP
    SLOTMD_RM_SUNDS	= 0x0002,   // remove 'sunDS' and 'pylon01' from SZS+KMP
    SLOTMD_RM_SHYGUY	= 0x0004,   // remove 'HeyhoShipGBA'+'HeyhoBallGBA' from SZS+KMP

    SLOTMD_ADD_GICE	= 0x0010,   // add 'ice' to KMP/GOBJ
    SLOTMD_ADD_BICE	= 0x0020,   // add 'ice.brres' to SZS
    SLOTMD_ADD_SHYGUY	= 0x0040,   // add 'HeyhoShipGBA' to SZS+KMP

    SLOTMD_ICE_TO_WATER	= 0x0100,   // change KCL flag 0x70 to 0x30
    SLOTMD_WATER_TO_ICE	= 0x0200,   // change KCL flag 0x30 to 0x70


    //--- job classes

    SLOTMD_JOB_RM_SZS	= SLOTMD_RM_ICE
			| SLOTMD_RM_SUNDS
			| SLOTMD_RM_SHYGUY,

    SLOTMD_JOB_ADD_SZS	= SLOTMD_ADD_BICE
			| SLOTMD_ADD_SHYGUY,

    SLOTMD_JOB_KCL	= SLOTMD_ICE_TO_WATER
			| SLOTMD_WATER_TO_ICE,

    SLOTMD_JOB_KMP	= SLOTMD_RM_ICE
			| SLOTMD_RM_SUNDS
			| SLOTMD_RM_SHYGUY
			| SLOTMD_ADD_GICE
			| SLOTMD_ADD_SHYGUY,


    //--- mode for --slot

    SLOTMD_M_DAISY	= SLOTMD_RM_ICE
			| SLOTMD_RM_SHYGUY
			| SLOTMD_ICE_TO_WATER,

    SLOTMD_M_SHERBET	= SLOTMD_RM_SUNDS
			| SLOTMD_RM_SHYGUY
			| SLOTMD_ADD_GICE
			| SLOTMD_ADD_BICE
			| SLOTMD_WATER_TO_ICE,

    SLOTMD_M_SHYGUY	= SLOTMD_RM_ICE
			| SLOTMD_RM_SUNDS
			| SLOTMD_ADD_SHYGUY
			| SLOTMD_ICE_TO_WATER,

    SLOTMD_M_STANDARD	= SLOTMD_RM_ICE
			| SLOTMD_RM_SUNDS
			| SLOTMD_RM_SHYGUY
			| SLOTMD_ICE_TO_WATER,

    SLOTMD_M_MOST	= SLOTMD_RM_SUNDS
			| SLOTMD_RM_SHYGUY
			| SLOTMD_ADD_BICE
			| SLOTMD_ICE_TO_WATER,
}
SlotMode_t;

extern SlotMode_t opt_slot;
int ScanOptSlot ( ccp arg );
ccp PrintSlotMode ( SlotMode_t mode );

///////////////////////////////////////////////////////////////////////////////
///////////////			PatchImage_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[PatchImage_t]]

typedef enum PatchImage_t
{
    PIM_VCENTER		= 0x0000,
    PIM_TOP		= 0x0001,
    PIM_BOTTOM		= 0x0002,
    PIM_INS_TOP		= 0x0003,
    PIM_INS_BOTTOM	= 0x0004,
     PIM_M_V		= 0x0007,

    PIM_HCENTER		= 0x0000,
    PIM_LEFT		= 0x0010,
    PIM_RIGHT		= 0x0020,
    PIM_INS_LEFT	= 0x0030,
    PIM_INS_RIGHT	= 0x0040,
     PIM_M_H		= 0x0070,
     PIM_SHIFT_V_H	= 4,	  // shift value from vertical to horizontal values

    PIM_CENTER		= PIM_VCENTER | PIM_HCENTER,

    PIM_MIX		= 0x0000,
    PIM_FOREGROUND	= 0x0100,
    PIM_BACKGROUND	= 0x0200,
    PIM_COPY		= 0x0300,
     PIM_M_COPY		= 0x0300,

    PIM_LEAVE		= 0x0000,
    PIM_SHRINK		= 0x1000,
    PIM_GROW		= 0x2000,
     PIM_M_SIZE		= 0x3000,

    PIM_MASK		= PIM_M_V | PIM_M_H | PIM_M_COPY | PIM_M_SIZE

} PatchImage_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			slot by attribute		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[slot_info_type_t]]

typedef enum slot_info_type_t
{
    SIT_NOT,		// not defined
    SIT_GENERIC,	// generic slot: "arena" or music by race/arena slot
    SIT_RECOMMEND,	// recommended slot: "r##" | "a##" | "e*" | "m*"
    SIT_MANDATORY3,	// mandatory alternative: "31+42+71"
    SIT_MANDATORY2,	// mandatory alternative: "31+71"
    SIT_MANDATORY,	// mandatory slot: "##"
}
__attribute__ ((packed)) slot_info_type_t;

//-------------------------------------------------------------------------
// [[slot_info_t]]

typedef struct slot_info_t
{
    mem_t		source;		// reference to last source of analysis
					// becomes invalid when source becomes invalid

    u16			race_slot;	// 0 or 11..84
    u16			arena_slot;	// 0 or 11..25
    u16			edit_slot;	// 0 or 11..84 or 111..125
    u16			music_index;	// 0 or 0x75..

    slot_info_type_t	race_mode;	// 0:not,            2:"r##", 3:"##"
    slot_info_type_t	arena_mode;	// 0:not, 1:"arena", 2:"a##"
    slot_info_type_t	edit_mode;	// 0:not,            2:"e##" or "ea##"
    slot_info_type_t	music_mode;	// 0:not, 1:generic, 2:"m*"

    bool		have_31_71;	// true: "31+71" found!
    bool		have_31_42_71;	// true: "31+42+71" found!

    char		race_info[9];	// race slot as text:  "##" | "r##" | "31+71" | "31+41+71"
    char		arena_info[6];	// arena slot as text: "arena" | "a##"
    char		edit_info[6];	// edit slot as text:  "e##" | "ea##"
    char		music_info[5];	// music slot as text: "m##" | "ma##"
    char		slot_attrib[24];// normalized slot atribute, combi of race+arena+edit+music

}
slot_info_t;

//-----------------------------------------------------------------------------

void DumpSlotInfo
(
    FILE		*f,	    // valid output stream
    int			indent,	    // robust indention of dump
    const slot_info_t	*si,	    // data to dump
    bool		src_valid   // FALSE: si.source if definitly invalid
);


//-----------------------------------------------------------------------------

void AnalyzeSlotAttrib ( slot_info_t *si, bool reset_si, mem_t attrib );
void AnalyzeSlotByName ( slot_info_t *si, bool reset_si, mem_t name );
void FinalizeSlotInfo  ( slot_info_t *si, bool minus_for_empty );

slot_info_t GetSlotByAttrib ( mem_t attrib, bool minus_for_empty );
slot_info_t GetSlotByName   ( mem_t name,   bool minus_for_empty );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			struct FormatField_t		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[FormatFieldItem_t]]

typedef struct FormatFieldItem_t
{
    ccp			key;		// string key of object
    file_format_t	fform;		// -1 or file format
    image_format_t	iform;		// -1 or image format
    palette_format_t	pform;		// -1 or palette format
    PatchImage_t	patch_mode;	// 0 or alignment
    uint		num;		// free number, printed if >0

} FormatFieldItem_t;

//-----------------------------------------------------------------------------
// [[FormatField_t]]

typedef struct FormatField_t
{
    FormatFieldItem_t	* list;		// pointer to the item list
    uint		used;		// number of used elements of 'list'
    uint		size;		// number of allocated elements of 'list'

} FormatField_t;

//-----------------------------------------------------------------------------

void InitializeFormatField ( FormatField_t * ff );
void ResetFormatField ( FormatField_t * ff );
void MoveFormatField ( FormatField_t * dest, FormatField_t * src );

int FindFormatFieldIndex ( const FormatField_t * ff, ccp key, int not_found_value );
FormatFieldItem_t * FindFormatField ( const FormatField_t * ff, ccp key );

// return first matching element
FormatFieldItem_t * FindFormatFieldNC ( const FormatField_t * ff, ccp key );
FormatFieldItem_t * MatchFormatField ( const FormatField_t * ff, ccp key );

bool RemoveFormatField ( FormatField_t * ff, ccp key ); // return: true if item deleted

FormatFieldItem_t * InsertFormatField
(
    FormatField_t	* ff,		// valid attrib field
    ccp			key,		// key to insert
    bool		scan_form,	// true: scan 'key' for FORM= prefix
    bool		move_key,	// true: move string, false: strdup()
    bool		* found		// not null: store found status
);

FormatFieldItem_t * InsertFormatFieldFF
(
    FormatField_t	* ff,		// valid attrib field
    ccp			key,		// key to insert
    file_format_t	fform,		// file format
    bool		scan_form,	// true: scan 'key' for FORM= prefix
    bool		move_key,	// true: move string, false: strdup()
    bool		* found		// not null: store found status
);

FormatFieldItem_t * AppendFormatField
(
    FormatField_t	* ff,		// valid attrib field
    ccp			key,		// key to insert
    bool		scan_form,	// true: scan 'key' for FORM= prefix
    bool		move_key	// true: move string, false: strdup()
);

#if 0 // [[not-needed]] not needed yet 2011-06
enumError LoadFormatField
(
    FormatField_t	* ff,		// attrib field
    bool		init_ff,	// true: initialize 'ff' first
    bool		sort,		// true: sort 'ff'
    ccp			filename,	// filename of source file
    bool		silent		// true: don't print open/read errors
);
#endif

enumError SaveFormatField
(
    FormatField_t	* ff,		// valid attrib field
    ccp			filename,	// filename of dest file
    bool		rm_if_empty	// true: rm dest file if 'ff' is empty
);

enumError WriteFormatField
(
    FILE		* f,		// open destination file
    ccp			filename,	// NULL or filename (needed on write error)
    FormatField_t	* ff,		// valid format field
    ccp			line_prefix,	// not NULL: insert prefix before each line
    ccp			key_prefix,	// not NULL: insert prefix before each key
    ccp			eol,		// end of line text (if NULL: use LF)
    uint		hex_fw		// if >0: print num as hex with field width
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			tiny support			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[tiny_param_t]]

#define N_TINY_MODES	8

typedef struct tiny_param_t
{
    // HINT: Value 0: function is off!

    //--- KCL
    int		kcl_round;	// positions: shift value for RoundF()/RoundD()
    int		kcl_mask;	// normals: value for MaskF() and normals
				// also condition for 'kcl_round'

    //--- position calculations, 999==Off
    int		pos_round;	// positions: shift value for RoundF()/RoundD()
    int		rot_round;	// rotations: shift value for RoundF()/RoundD()
    int		scale_round;	// scales:    shift value for RoundF()/RoundD()

    int		route_round;	// route positions: shift value for RoundF()/RoundD()
    int		cannon_round;	// cannon direction: shift value for RoundF()/RoundD()
}
tiny_param_t;

extern const tiny_param_t TinyParam[N_TINY_MODES];

//-----------------------------------------------------------------------------

int ScanOptTiny ( ccp arg );

static inline float RoundF( float num, int shift )
	{ return ldexpf(roundf(ldexpf(num,shift)),-shift); }

static inline double RoundD( double num, int shift )
	{ return ldexp(round(ldexp(num,shift)),-shift); }

static inline float MaskF( float num, u32 mask )
	{ union { float f; u32 u; } x; x.f = num; x.u &= mask; return x.f; }

static inline float RoundMaskF( float num, int shift, u32 mask )
{
    union { float f; u32 u; } x;
    x.f	= ldexpf(roundf(ldexpf(num,shift)),-shift);
    x.u	&= mask;
    return x.f;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			string lists			///////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct ParamList_t
{
	char * arg;
	bool is_expanded;
	int count;
	struct ParamList_t * next;

} ParamList_t;

extern uint n_param;
extern ParamList_t * first_param;
extern ParamList_t ** append_param;

///////////////////////////////////////////////////////////////////////////////

int AtFileHelper ( ccp arg, int (*func)(ccp arg) );

int AddParam ( ccp arg );
void AtExpandParam ( ParamList_t ** param );
void AtExpandAllParam ( ParamList_t ** first_param );

//-----------------------------------------------------------------------------

void CollectExpandParam
(
    StringField_t	*plist,		// insert or append (inorder_count>0) to this list
    ParamList_t		*param,		// first param to add
    int			max,		// max num of param to add; -1: add all param
    wildcard_mode_t	wc_mode		// bit field: modes for wildcard handling
);

enumError cmd_wildcards ( int argc, char ** argv );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			general list support		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[List_t]]

typedef struct List_t
{
    u8		*list;		// list of data
    uint	used;		// used elements of 'list'
    uint	size;		// alloced elements of 'list'
    uint	elem_size;	// size of 1 element

} List_t;

void InitializeList ( List_t * list, uint elem_size );
void ResetList      ( List_t * list );
void * InsertList   ( List_t * list, int index );
void * AppendList   ( List_t * list );
void * GrowList     ( List_t * list, uint n );
void * GrowListSize ( List_t * list, uint n, uint add_size );
void * GetListElem
	( const List_t * list, int index, const void * return_not_found );

///////////////////////////////////////////////////////////////////////////////
// [[MemItem_t]]

#define NAME_IDX_SIZE 10

typedef struct MemItem_t
{
    int		idx1;		// first user index
    int		idx2;		// second user index
    int		sort_idx;	// a sorting index
    ccp		name;		// NULL or pointer to name
    u8		*data;		// NULL or pointer to data
    uint	size;		// size of 'data'

    //--- string index support

    u32		str_idx_buf[NAME_IDX_SIZE];
				// standard buffer for 'str_idx'
    u32		*str_idx;	// NULL or pointer to alloced string index list
    uint	str_idx_size;	// number of alloced elements of 'str_idx'

    //--- flags

    bool	name_alloced;	// true: 'name' is alloced
    bool	data_alloced;	// true: 'data' is alloced

} MemItem_t;

//-----------------------------------------------------------------------------

void InitializeMIL ( List_t * list );
void ResetMIL      ( List_t * list );
void SortMIL       ( List_t * list, bool use_sort_idx );

MemItem_t * AppendMIL ( List_t * list );
MemItem_t * GrowMIL   ( List_t * list, uint n );
MemItem_t * GetMemListElem
	( const List_t * list, int index, const MemItem_t * return_not_found );

u32 * GetStrIdxMIL ( MemItem_t * mi, uint n_need );

static inline const u32 * GetStrIdxListMIL ( const MemItem_t * mi )
{
    DASSERT(mi);
    return mi->str_idx ? mi->str_idx : mi->str_idx_buf;
}

static inline uint GetStrIdxCountMIL ( const MemItem_t * mi )
{
    DASSERT(mi);
    return mi->str_idx_size ? mi->str_idx_size
		: sizeof(mi->str_idx_buf)/sizeof(*mi->str_idx_buf);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			cygwin support			///////////////
///////////////////////////////////////////////////////////////////////////////

#ifdef __CYGWIN__

  void NormalizeFilenameParam ( ParamList_t *param );
  #define NORMALIZE_FILENAME_PARAM(p) NormalizeFilenameParam(p)

  void NormalizeFilenameParamList ( ParamList_t *param );
  #define NORMALIZE_FILENAME_PARAM_LIST(p) NormalizeFilenameParamList(p)

#else

  #define NORMALIZE_FILENAME_PARAM(p)
  #define NORMALIZE_FILENAME_PARAM_LIST(p)

#endif // __CYGWIN__

//-----------------------------------------------------------------------------

void SetSource ( ccp source );
void SetIdList ( ccp source );
void SetReference ( ccp source );
void SetDest ( ccp dest, bool mkdir );
void CheckOptDest ( ccp default_dest, bool default_mkdir );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			modes and options		///////////////
///////////////////////////////////////////////////////////////////////////////

int ScanOptCompr ( ccp arg );
int GetComprByFF ( file_format_t ff, int compr );
int ScanOptTracks ( ccp arg );
int ScanOptArenas ( ccp arg );
ccp NormalizeTrackName ( char *buf, uint bufsize, ccp arg );

void DumpTrackList
(
    FILE	* f,		// NULL(=stdout) or file
    const u32	* tab,		// NULL or list with track indices
    int		brief_mode	// 0: full table
				// 1: suppress header
				// 2: file names only
);

void DumpArenaList
(
    FILE	* f,		// NULL(=stdout) or file
    const u32	* tab,		// NULL or list with arena indices
    int		brief_mode	// 0: full table
				// 1: suppress header
				// 2: file names only
);

void DumpTrackFileList
(
    FILE	* f,		// NULL(=stdout) or file
    const u32	* track_tab,	// NULL or list with track indices
    const u32	* arena_tab	// NULL or list with arena indices
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			scan setup files		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[SetupDef_t]]

typedef struct SetupDef_t
{
    ccp			name;		// name of parameter, NULL=list terminator
    u32			factor;		// alignment factor; 0: text param
    ccp			param;		// alloced text param
    u64			value;		// numeric value of param

} SetupDef_t;

//-----------------------------------------------------------------------------
// [[ListDef_t]]

typedef struct ListDef_t
{
    ccp			name;		// section name
    bool		append;		// true: append string instead of insert
    StringField_t	*sf;		// NULL or valid pointer to string field
    exmem_list_t	*eml;		// NULL or valid pointer to exmem list

} ListDef_t;

//-----------------------------------------------------------------------------

size_t ResetSetup
(
    SetupDef_t	* list		// object list terminated with an element 'name=NULL'
);

enumError ScanSetupFile
(
    SetupDef_t		* sdef,		// object list terminated with an element 'name=NULL'
    const ListDef_t	* ldef,		// NULL or object list terminated
					//	with an element 'name=NULL'
    ccp			path1,		// filename of text file, part 1
    ccp			path2,		// filename of text file, part 2
    SubDir_t		* sdir,		// not NULL: use it and ignore 'path1'
    bool		silent		// true: suppress error message if file not found
);

///////////////////////////////////////////////////////////////////////////////
// [[SetupParam_t]]

typedef struct SetupParam_t
{
    ccp			object_name;	// name of object, alloced
    u32			object_id;	// id of object
    bool		object_id_used;	// 'object_id' set and valid
    file_format_t	fform_file;	// file format
    file_format_t	fform_arch;	// archive format
    int			version;	// a version number
    int			compr_mode;	// -1:no compr, 0:auto, 1:compr
    int			have_pt_dir;	// -1:no, 0:unknown, 1:yes
    u32			min_data_off;	// minimum found data offset
    u32			max_data_off;	// maximum found data offset
    u32			data_align;	// data alignment

    StringField_t	include_pattern;// include usually hidden files, pattern list
    StringField_t	include_name;	// include usually hidden files, name list
    StringField_t	exclude_pattern;// exclude files, pattern list
    StringField_t	exclude_name;	// exclude files, name list
    FormatField_t	encode_list;	// encode before including
    StringField_t	create_list;	// create sub file before including
    FormatField_t	order_list;	// use this list as sorting reference

} SetupParam_t;

//-----------------------------------------------------------------------------

void InitializeSetupParam ( SetupParam_t * sp );
void ResetSetupParam ( SetupParam_t * sp );

//-----------------------------------------------------------------------------

enumError ScanSetupParam
(
    SetupParam_t	* sp,		// setup param to scan
    bool		use_options,	// true: use options to override values
    ccp			path1,		// NULL or part #1 of path
    ccp			path2,		// NULL or part #2 of path
    SubDir_t		* sdir,		// not NULL: use it and ignore 'path1'
    bool		silent		// true: suppress printing of error messages
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			export keys			///////////////
///////////////////////////////////////////////////////////////////////////////

typedef enum export_mode_t
{
    EXPORT_NONE,
    EXPORT_C,
    EXPORT_PHP,
    EXPORT_MAKEDOC,
     EXPORT_CMD_MASK	= 0x00f,

    EXPORT_F_FUNCTIONS	= 0x100,
    EXPORT_F_FILEATTRIB	= 0x200,
    EXPORT_F_TRACKS	= 0x400,
     EXPORT_FLAG_MASK	= 0x700

} export_mode_t;

///////////////////////////////////////////////////////////////////////////////

export_mode_t ScanExportMode(); // scan parameters

//
///////////////////////////////////////////////////////////////////////////////
///////////////			SZS file list			///////////////
///////////////////////////////////////////////////////////////////////////////

typedef enum FileList_t
{
    FLT_STD		= 0x01, // non '_d' files
    FLT_D		= 0x02, // '_d' files

    FLT_TRACK		= 0x10, // 32 track files
    FLT_ARENA		= 0x20, // 10 arena files
    FLT_OTHER		= 0x40, //  5 other files
    FLT_OLD		= 0x80, //  4 old files (old BRSUB formats)

    FLT_M_CLASS		= FLT_STD | FLT_D,
    FLT_M_TYPE		= FLT_TRACK | FLT_ARENA | FLT_OTHER | FLT_OLD,
    FLT_M_ALL		= FLT_M_CLASS | FLT_M_TYPE,

} FileList_t;

//-----------------------------------------------------------------------------

typedef struct TrackInfo_t TrackInfo_t;

typedef int (*IterateTrackFilesFunc)
(
    ccp		fname,		// name of current file without '.szs' extension
    ccp		id,		// unique ID of current file
    FileList_t	mode,		// mode of current file
    const TrackInfo_t *ti,	// NULL or related track info
    void	*param		// user defined parameter
);

//-----------------------------------------------------------------------------

int IterateTrackFiles
(
    FileList_t	mode,		// bit field with requested files
    IterateTrackFilesFunc
		func,		// call back function
    void	*param		// user defined parameter
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			file type validation		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[valid_t]]

typedef enum valid_t
{
    VALID_OK,		// validation success
    VALID_NO_TEST,	// no validation test available
    VALID_WARNING,	// warnings, repairable
    VALID_ERROR,	// error in data structures
    VALID_WRONG_FF,	// wrong file type
    VALID_UNKNOWN_FF,	// unknown file type
    VALID_UNKNOWN,	// unknown status

    VALID__N		// number of status modes

} valid_t;

extern ccp valid_text[VALID__N];

///////////////////////////////////////////////////////////////////////////////

struct szs_file_t;

valid_t IsValidSZS
(
    struct szs_file_t	*szs,		// valid szs archive
    bool		warnings	// true: print warnings
);

///////////////////////////////////////////////////////////////////////////////

valid_t IsValid
(
    const void		*data,		// valid data to test
    uint		data_size,	// size of 'data'
    uint		file_size,	// not NULL: size of complete file
    struct szs_file_t	*szs,		// not NULL: base archive
    file_format_t	fform,		// known file format
					// if FF_UNKNOWN: call GetByMagicFF()
    ccp			fname		// not NULL: print warnings with file ref
);

///////////////////////////////////////////////////////////////////////////////

valid_t IsValidBRSUB
(
    const void		* data,		// data
    uint		data_size,	// size of 'data'
    uint		file_size,	// NULL or size of complete file
    struct szs_file_t	*szs,		// not NULL: base archive
    file_format_t	fform,		// known file format
					// if FF_UNKNOWN: call GetByMagicFF()
    bool		check_fform,	// true: confirm 'fform' by calling GetByMagicFF()
    ccp			fname,		// not NULL: print warnings with file ref
    const endian_func_t	*endian		// not NULL: use this endian functions
);

///////////////////////////////////////////////////////////////////////////////

valid_t IsValidBTI
(
    const void		* data,		// data
    uint		data_size,	// size of 'data'
    uint		file_size,	// NULL or size of complete file
    ccp			fname		// not NULL: print warnings with file ref
);

//-----------------------------------------------------------------------------

struct kcl_analyze_t;

valid_t IsValidKCL
(
    struct
	kcl_analyze_t	* ka,		// not NULL: init and store stats
    const void		* data,		// data
    uint		data_size,	// size of 'data'
    uint		file_size,	// NULL or size of complete file
    ccp			fname		// not NULL: print warnings with file ref
);

//-----------------------------------------------------------------------------

valid_t IsValidKMP
(
    const void		* data,		// data
    uint		data_size,	// size of 'data'
    uint		file_size,	// NULL or size of complete file
    ccp			fname		// not NULL: print warnings with file ref
);

//-----------------------------------------------------------------------------

valid_t IsValidLEX
(
    const void		* data,		// data
    uint		data_size,	// size of 'data'
    uint		file_size,	// NULL or size of complete file
    ccp			fname		// not NULL: print warnings with file ref
);

//-----------------------------------------------------------------------------

valid_t IsValidMDL
(
    const void		* data,		// data
    uint		data_size,	// size of 'data'
    uint		file_size,	// NULL or size of complete file
    struct szs_file_t	*szs,		// not NULL: base archive
    ccp			fname,		// not NULL: print warnings with file ref
    const endian_func_t	*endian		// not NULL: use this endian functions
);

//-----------------------------------------------------------------------------

struct pat_analyse_t;

valid_t IsValidPAT
(
    const void		* data,		// data
    uint		data_size,	// size of 'data'
    uint		file_size,	// NULL or size of complete file
    struct szs_file_t	*szs,		// not NULL: base archive
    ccp			fname,		// not NULL: print warnings with file ref
    const endian_func_t	*endian,	// not NULL: use this endian functions
    struct pat_analyse_t *ana		// not NULL: store analysis data here
);

//-----------------------------------------------------------------------------

valid_t IsValidSTATICR
(
    const void		* data,		// data
    uint		data_size,	// size of 'data'
    uint		file_size,	// NULL or size of complete file
    ccp			fname		// not NULL: print warnings with file ref
);

//-----------------------------------------------------------------------------

valid_t IsValidTEXCT
(
    const void		* data,		// data
    uint		data_size,	// size of 'data'
    uint		file_size,	// NULL or size of complete file
    struct szs_file_t	*szs,		// not NULL: base archive
    ccp			fname		// not NULL: print warnings with file ref
);

//-----------------------------------------------------------------------------

valid_t IsValidCTCODE
(
    const void		* data,		// data
    uint		data_size,	// size of 'data'
    uint		file_size,	// NULL or size of complete file
    ccp			fname		// not NULL: print warnings with file ref
);

//-----------------------------------------------------------------------------

valid_t IsValidLTA
(
    const void		* data,		// data
    uint		data_size,	// size of 'data'
    uint		file_size,	// NULL or size of complete file
    ccp			fname		// not NULL: print warnings with file ref
);

//
///////////////////////////////////////////////////////////////////////////////
///////////////			scan configuration		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[config_t]]

typedef struct config_t
{
    ccp config_file;
    ccp base_path;
    ccp install_path;
    ccp install_config;
    ccp share_path;
    ccp autoadd_path;
}
config_t;

//-----------------------------------------------------------------------------

static inline void InitializeConfig ( config_t *config )
	{ DASSERT(config); memset(config,0,sizeof(*config)); }

void ResetConfig ( config_t *config );

enumError ScanConfig
(
    config_t		*config,	// store configuration here
    ccp			path,		// NULL or path
    bool		silent		// true: suppress printing of error messages
);

void SearchConfigHelper ( search_file_list_t *sfl, int stop_mode );
const config_t * GetConfig(void);
void PrintConfig ( FILE *f, const config_t *config, bool verbose );
void PrintConfigScript ( FILE *f, const config_t *config );
void PrintConfigFile ( FILE *f, const config_t *config );

const StringField_t * GetSearchList(void);
const StringField_t * GetAutoaddList(void);

extern ccp autoadd_destination;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			SHA1 support			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[sha1_type_t]]

typedef enum sha1_type_t
{
    SHA1T_KCL	= 0x01,
    SHA1T_KMP	= 0x02,
    SHA1T_MAP	= 0x04,

    SHA1T__ALL	= 0x07
}
__attribute__ ((packed)) sha1_type_t;

//-----------------------------------------------------------------------------
// [[sha1_db_t]]

typedef struct sha1_db_t
{
    u8		slot;	// 11..84, 111..125
    sha1_type_t	type;
    sha1_hash_t sha1;
}
__attribute__ ((packed)) sha1_db_t;

//-----------------------------------------------------------------------------

const sha1_db_t * GetSha1DbHex ( sha1_type_t type, ccp hex, ccp end );
const sha1_db_t * GetSha1DbBin ( sha1_type_t type, cvp hash );
u8 GetSha1Slot ( sha1_type_t type, cvp hash );

void SHA1_to_ID ( sha1_id_t id, const sha1_hash_t hash );
ccp GetSha1Data ( cvp data, uint size );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			mkw_prefix_flags_t		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[mkw_prefix_flags_t]]

typedef enum mkw_prefix_flags_t
{
	MPF_NINTENDO	= 0x01,  // N: by Nintendo
	MPF_MARIOKART	= 0x02,  // M: Mario Kaert series
	MPF_PREFIX1	= 0x04,  // 1: can be used as part 1 for a prefix pair
	MPF_PREFIX2	= 0x08,  // 2: can be used as part 2 for a prefix pair
}
__attribute__ ((packed)) mkw_prefix_flags_t;

//
///////////////////////////////////////////////////////////////////////////////
///////////////			mkw_prefix_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[mkw_prefix_t]]
// search for [[mtcat]]

typedef struct mkw_prefix_t
{
    ccp			info;		// if NULL: list terminator
    char		prefix[12];
    mkw_prefix_flags_t	flags;
    u8			order;
    u8			color;
}
mkw_prefix_t;

///////////////////////////////////////////////////////////////////////////////

int ScanOptLoadPrefix ( ccp arg );

struct ScanText_t;
const mkw_prefix_t * ScanPrefixTable ( struct ScanText_t *st );
const mkw_prefix_t * DefinePrefixTable ( cvp source, uint len );
const mkw_prefix_t * GetPrefixTable(void);

// use strlen() if pre_len|name_len <0
const mkw_prefix_t * FindPrefix ( ccp pre, int pre_len );
const mkw_prefix_t * GetPrefix ( ccp name, int name_len );

// if tab==0: Use GetPrefixTable()
enumError SavePrefixTable   ( FILE *f,   const mkw_prefix_t * tab );
enumError SavePrefixTableFN ( ccp fname, const mkw_prefix_t * tab );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			mkw_category_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[mkw_category_t]]

typedef struct mkw_category_t
{
    s8			mtcat;		// -1: not used,  -2: end of list
    u8			mode;
    u8			ban_type;
    char		ch1[4];
    char		ch2[4];
    char		abbrev[8];
    u32			fg_color;
    u32			bg_color;
    ccp			color_name;	// alloced
    ccp			name;		// alloced
    ccp			info;		// alloced
    ccp			attrib;		// alloced
}
mkw_category_t;

//-----------------------------------------------------------------------------

enum
{
    MKW_CAT_SPECIAL	= 0x100,
    MKW_CAT_UNKNOWN	= MKW_CAT_SPECIAL | G_MTCAT_MD_UNKNOWN,
    MKW_CAT_DEFAULT	= MKW_CAT_SPECIAL | G_MTCAT_MD_DEFAULT,
    MKW_CAT_CUSTOM	= MKW_CAT_SPECIAL | G_MTCAT_MD_CUSTOM,
    MKW_CAT_NINTENDO	= MKW_CAT_SPECIAL | G_MTCAT_MD_NINTENDO,
};

int TranslateSpecialCategory ( int special );
const mkw_category_t * GetCategory ( int cat, int fallback );

//-----------------------------------------------------------------------------
// [[mkw_category_list_t]]

enum { MAX_MTCAT = 100, MAX_MTCAT_MODE = 3 };

typedef struct mkw_category_list_t
{
    mkw_category_t	*list;
    int			used;
    int			size;	// limited by MAX_MTCAT

    KeywordTabManager_t	atm;	// attribute manager
}
mkw_category_list_t;

extern mkw_category_list_t mkw_category_list;

///////////////////////////////////////////////////////////////////////////////
// search for [[mtcat]]

int ScanOptLoadCategory ( ccp arg );

void ResetCategoryList ( mkw_category_list_t *clist );
const mkw_category_list_t * GetCategoryList(void);

struct ScanText_t;
mkw_category_list_t ScanCategoryList ( struct ScanText_t *st );
mkw_category_list_t DefineCategoryList ( cvp source, uint len );

// if clist==0: Use GetCategoryList()
const KeywordTab_t * GetCategoryKeywordTab ( mkw_category_list_t * clist );
enumError SaveCategoryList   ( FILE *f,   const mkw_category_list_t * clist );
enumError SaveCategoryListFN ( ccp fname, const mkw_category_list_t * clist );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			sha1_reference_t		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[sha1_reference_t]]

typedef struct sha1_reference_t
{
    sha1_hash_t	sha1;
    union
    {
	u16 ref[3];
	struct
	{
	    u16 track;
	    u16 family;
	    u16 clan;
	};
    };
    TrackClass_t tclass;
}
sha1_reference_t;

//-----------------------------------------------------------------------------

extern sha1_reference_t	* s1ref_list;		// NULL or list with records
extern uint		s1ref_used;		// used elements of 's1ref_list'
extern uint		s1ref_size;		// alloced elements for 's1ref_list'
extern uint		s1ref_max1_track;	// max(s1ref_list->track) + 1
extern uint		s1ref_max1_family;	// max(s1ref_list->family) + 1
extern uint		s1ref_max1_clan;	// max(s1ref_list->clan) + 1

//-----------------------------------------------------------------------------

void ResetSha1Ref(void);
void ScanSha1Ref   ( struct ScanText_t *st );
void DefineSha1Ref ( cvp source, uint len );

const sha1_reference_t * FindSha1RefBin ( sha1_hash_t sha1 );
const sha1_reference_t * FindSha1RefHex ( ccp sha1, ccp end );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			split_filename_t		///////////////
///////////////////////////////////////////////////////////////////////////////
// [[split_filename_t]]

typedef struct split_filename_t
{
    mem_t directory;		// %y leading directories
    mem_t source;		// %Y full source without leading directories

    //--- split source into 3 parts

    mem_t f_name;		// %F source name, stripped
    mem_t f_d;			// %D empty or "_d"
    mem_t f_ext;		// %E file extension

    //--- split name into N parts

    mem_t norm;			// %O normalized name, alloced

    mem_t plus;			// %P plus prefix
    mem_t boost;		// %b boost prefix
    mem_t game1;		// %g first game prefix
    mem_t game2;		// %G second game prefix
    mem_t game;			// %p combined game prefix
    mem_t name;			// %n name
    mem_t extra;		// %e extra info
    mem_t version;		// %v version nummber
    mem_t authors;		// %a list of authors
    mem_t editors;		// %d list of editors
				// $c := %a%d
    mem_t attribs;		// %A list of attributes
    mem_t le_group;		// -- name of head= or grp= or group=


    //--- misc

    int			attrib_order;
    int			plus_order;
    int			game_order;
    uint		game1_color;
    uint		game2_color;

    DistribFlags_t	distrib_flags;	// -- distribution flags G_DTA_*
    le_flags_t		le_flags;	// -- LE-CODE flags "number string"
    u8			track_cat;	// -- track category "number mode string" 
    u8			lap_count;	// %l number of laps, valid if >0.0
    float		speed_factor;	// %s speed factor, valid if >0.0

    ccp			input;
    bool		input_alloced;
}
split_filename_t;

///////////////////////////////////////////////////////////////////////////////
// [[print_split_par_t]]

typedef struct print_split_par_t
{
    ccp			plus_mode;	// plus mode, only needed by PrintNameSPF()
    int			split_level;	// >0: use it to get a default 'format' 
    ccp			format;		// format string

    u8			lap_count;	// number of laps, valid if >0.0
    float		speed_factor;	// speed factor, valid if >0.0

    int			u_skip_mode;	// skip '+': <0:never, 0:if unique, >0: always
    int			u_skip_count;	// number of skipped tags
    bool		u_use_list;	// true: use 'u_list' to find multi-used names
    exmem_list_t	u_list;		// list of keywords until first '%+'
    exmem_key_t		*u_item;	// last inderted/used element of 'u_list'
}
print_split_par_t;

///////////////////////////////////////////////////////////////////////////////

extern ccp opt_plus;
extern int opt_split;
extern ccp opt_printf;

static inline void InitializeSPF ( split_filename_t *spf )
	{ DASSERT(spf); memset(spf,0,sizeof(*spf)); }

void ResetSPF ( split_filename_t *spf );

void AnalyseSPF
(
    split_filename_t	*spf,		// valid pointer
    bool		init_spf,	// true: InitializeSPF(), false: ResetSPF()
    ccp			source,		// NULL or ponter to name
    ccp			src_end,	// end of source. if 0 then use strlen().
    CopyMode_t		cpm,		// copy mode for 'source'
    ccp			plus_mode	// NULL or priority string
);

//-----------------------------------------------------------------------------

void ResetPSP ( print_split_par_t *psp );

exmem_t PrintSPF
(
    exmem_dest_t	*dest,		// Kind of destination, if NULL then MALLOC()
    const split_filename_t
			*spf,		// Valid and initialized source
    print_split_par_t	*par		// valid struct with parameters
);

exmem_t PrintNameSPF
(
    exmem_dest_t	*dest,		// Kind of destination, if NULL then MALLOC()
    ccp			source,		// NULL or pointer to name to analyze
    ccp			src_end,	// End of source. If 0 then use strlen().
    print_split_par_t	*par		// valid struct with parameters
);

// Convertion types in alphabertic order:
//
//	%a  authors,	list of authors, format "(authors)", options=0, curly braces
//	%A  attribs,	list of attributes, format "(authors)", options=0
//	%b  boost,	boost prefix
//	%c  auth+ed,	combined authors + editors, format "(authors,,editors)", options=0
//	%d  editors,	list of editors, format "(editors)", options=0
//	%D  f_d,	empty or "_d"
//	%e  extra,	extra info, format "(extra)", options=0, curly braces
//	%E  f_ext,	file extension, format ".ext", options=0
//	%g  game1,	first game prefix
//	%G  game2,	second game prefix
//	%F  f_name,	file name, stripped
//	%l  lap_count
//	%L  LOTTERY	ALIAS
//	%n  name,	name
//	%N  NAME	ALIAS for plus+boost+pre+name
//	%O  norm,	normalized file name, alloced
//	%p  game,	combined game prefix
//	%P  plus,	plus prefix
//	%R  RACE	ALIAS
//	%s  speed_factor
//	%v  version,	version nummber

// Format options:
//	'-'	align left
//	' '	add leading space if not empty
//	'0'	leading space
//	'!'	suppress braces
//	'({[<'	force a kind of braces
//	'c'  insert color escape if known

//
///////////////////////////////////////////////////////////////////////////////
///////////////			output_mode_t			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[output_mode_t]]

typedef struct output_mode_t
{
    bool		syntax;		// true: print syntax infos
    u8			mode;		// 1:list, 2:ref, 3:full
    bool		cross_ref;	// true: print cross reference infos
    bool		hex;		// true: force hex out
    bool		lecode;		// true: print LE-CODE specific data
}
output_mode_t;

extern output_mode_t output_mode;
void InitializeOutputMode ( output_mode_t * outmode );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    misc			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[IsArena_t]]

typedef enum IsArena_t
{
    ARENA_NONE	= 0,	// no arena found
    ARENA_MAYBE,	// maybe an arena (some hints detected)
    ARENA_FOUND,	// arena detected
    ARENA_DISPATCH,	// arena detected with ENPT dispatch points
    ARENA__N
}
IsArena_t;

extern const char is_arena_name[ARENA__N][9];

///////////////////////////////////////////////////////////////////////////////
// [[data_tab_t]]

typedef struct data_tab_t
{
    ccp		name;		// name of data
    uint	mode;		// bit filed:
				//  1: source is bzip'ed
				//  2: base data is bzip'ed

    const void	*data;		// valid pointer to data
    uint	size;		// size of 'data'

    const u32	*patch;		// not NULL: use this patch to get real data
}
data_tab_t;

///////////////////////////////////////////////////////////////////////////////

void cmd_version_section
	( bool sect_header, ccp name_short, ccp name_long, int verbose );

enumError cmd_config();
enumError cmd_install();
enumError cmd_argtest ( int argc, char ** argv );
enumError cmd_expand ( int argc, char ** argv );
enumError cmd_error();
enumError cmd_filetype();
enumError cmd_fileattrib();
enumError cmd_float();
enumError cmd_vr_calc();
enumError cmd_vr_race();
enumError cmd_rawdump ( const data_tab_t *data_tab );

void ExportFileAttribMakedoc ( FILE * f );

extern uint opt_analyze_mode;
int ScanOptAnalyzeMode ( ccp arg );

struct szs_file_t;
void AnalyzeBRSUB ( struct szs_file_t * szs, u8 * data, uint data_size, ccp name );
void AnalyzeBREFF ( struct szs_file_t * szs, u8 * data, uint data_size, ccp name );
void AnalyzeTPL   ( struct szs_file_t * szs, u8 * data, uint data_size, ccp name );

struct kmp_t;
void AnalyzeKMP  ( const struct kmp_t * kmp, ccp base_fname );

struct kcl_t;
void AnalyzeKCL  ( const struct kcl_t * kcl );

///////////////////////////////////////////////////////////////////////////////

int GetCompressionLevel
(
    // return	-1:	invalid ff,
    //		0:	compression level unknown
    //		1..9:	compression level unknown

    file_format_t	ff,	// file format, call GetByMagicFF() if FF_UNKNOWN
    cvp			data,	// NULL or pointer to data
    uint		size	// size of data
);

//-----------------------------------------------------------------------------

ccp GetCompressionSuffix
(
    // returns EmptyString[] or CircBuf of format ".#"

    file_format_t	ff,	// file format, call GetByMagicFF() if FF_UNKNOWN
    cvp			data,	// NULL or pointer to data
    uint		size	// size of data
);

//-----------------------------------------------------------------------------

ccp GetCompressionNameFF
(
    // returns const string or CircBuf of format "FF.#"

    file_format_t	ff_arch, // FF_UNKNOWN or file format of archive
    file_format_t	ff_data, // file format of data, call GetByMagicFF() if FF_UNKNOWN
    cvp			data,	// NULL or pointer to data
    uint		size	// size of data
);

///////////////////////////////////////////////////////////////////////////////

bool IsExtractEnabled(); // return true if enabled and mkdir() successful

bool PrepareExtract
(
    // return FALSE on disabled or failure, otherwise return TRUE

    File_t	*f,		// file structure, will be initialized
    int		mode,		// convert filename: <0=to lower, >0=to upper
    ccp		format,		// format of filename
    ...				// arguments
)
__attribute__ ((__format__(__printf__,3,4)));

///////////////////////////////////////////////////////////////////////////////
// [[CheckMode_t]]

typedef enum CheckMode_t
{
    CMOD_HEADER		=  0x01,  // print header line before first message
    CMOD_FOOTER		=  0x02,  // print footer line (summary) behind last message
    CMOD_FORCE_HEADER	=  0x04,  // print header line always
    CMOD_FORCE_FOOTER	=  0x08,  // print footer line always

    CMOD_SILENT		=     0,  // print nothing
    CMOD_WARNING	=  0x10,  // print warnings
    CMOD_HINT		=  0x20,  // print hints
    CMOD_SLOT		=  0x40,  // print slot infos
    CMOD_INFO		=  0x80,  // simple info or statistic
    CMOD_VERBOSE	= 0x100,  // be verbose
    CMOD_LECODE		= 0x200,  // only relevant for LE-CODE

    CMOD_GLOBAL_STAT	= 0x100,  // store global statistics

    CMOD_PRINT		= CMOD_WARNING | CMOD_HINT | CMOD_SLOT,
    CMOD_DEFAULT	= CMOD_HEADER | CMOD_FOOTER | CMOD_PRINT,
    CMOD_FORCE		= CMOD_FORCE_HEADER | CMOD_FORCE_FOOTER,

} CheckMode_t;

//-----------------------------------------------------------------------------

CheckMode_t GetCheckMode
(
    bool	no_head,	// true: suppress header
    bool	no_hints,	// true: suppress hints
    bool	quiet,		// true: be quiet if no hint|warning is printed
    bool	verbose		// true: print some more hints
);

CheckMode_t GetMainCheckMode ( CheckMode_t mode, bool reset_counter );
CheckMode_t GetSubCheckMode  ( CheckMode_t mode, bool reset_counter );

extern const char check_bowl[];	  // begin of 'warning' line, for checks
extern const char check_bohl[];   // begin of 'hint' line, for checks
extern const char check_bosl[];   // begin of 'slot' line, for checks
extern const char check_boil[];   // begin of 'info' line, for checks
extern const char check_bosugl[]; // begin of 'suggestion' line, for checks

extern int global_warn_count;
extern int global_hint_count;
extern int global_info_count;

extern const ColorSet_t *colset;

///////////////////////////////////////////////////////////////////////////////

extern char *TableDecode64BySetup;
extern const char *TableEncode64BySetup;

int ScanOptCoding64 ( ccp arg );
EncodeMode_t SetupCoding64 ( EncodeMode_t mode, EncodeMode_t fallback );

void SetupPrintScriptByOptions ( PrintScript_t *ps );

///////////////////////////////////////////////////////////////////////////////

uint GetMkwMusicSlot	( uint tid );

//-----------------------------------------------------------------------------

static inline ccp PrintU8M1 ( u8 num )
{
    return num == M1(num) ? "-1" : PrintCircBuf("%u",num);
}

static inline ccp PrintU16M1 ( u16 num )
{
    return num == M1(num) ? "-1" : PrintCircBuf("%u",num);
}

///////////////////////////////////////////////////////////////////////////////

int FindTrackSlot ( ccp name, bool allow_arenas );
int FindArenaSlot ( ccp name, bool allow_tracks );

const TrackInfo_t * FindTrackInfo ( ccp name, bool allow_arenas );
const TrackInfo_t * FindArenaInfo ( ccp name, bool allow_tracks );

//
///////////////////////////////////////////////////////////////////////////////
///////////////			apple only			///////////////
///////////////////////////////////////////////////////////////////////////////

#ifdef __APPLE__

 void * memmem ( const void *l, size_t l_len, const void *s, size_t s_len );

#endif // __APPLE__

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    tables			///////////////
///////////////////////////////////////////////////////////////////////////////

extern const u8 CannonDataLEX[4+3*16];
extern const u8 TestDataLEX[8*1];

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    vars			///////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct Image_t Image_t;

extern ccp		opt_config;
extern bool		opt_no_pager;
extern int		opt_zero;
extern ccp		config_path;
extern ccp		tool_name;
extern ccp		std_share_path;
extern ccp		share_path;
//extern volatile int	verbose;	    // see above
extern volatile int	logging;
extern volatile int	ext_errors;
extern volatile int	log_timing;
extern bool		allow_all;
extern char		escape_char;
extern bool		use_utf8;
extern bool		use_de;
extern bool		ctcode_enabled;
extern bool		lecode_enabled;
extern uint		opt_ct_mode;		// calculated by ctcode_enabled & lecode_enabled
extern bool		lecode_04x;
extern int		testmode;
extern int		force_count;
extern uint		opt_tiny;
extern bool		force_kmp;
extern int		disable_checks;
extern OffOn_t		opt_battle_mode;
extern OffOn_t		opt_export_flags;
extern int		opt_route_options;
extern OffOn_t		opt_wim0;
extern ccp		opt_source;
extern StringField_t	source_list;
extern ccp		opt_id_list;
extern ccp		opt_reference;
extern ccp		opt_dest;
extern bool		opt_mkdir;
extern bool		opt_append;
extern bool		opt_overwrite;
extern bool		opt_number;
extern bool		opt_remove_src;
extern bool		opt_remove_dest;
extern bool		opt_update;
extern bool		opt_preserve;
extern ccp		opt_extract;
extern int		opt_ignore;
extern bool		opt_ignore_setup;
extern bool		opt_purge;
extern file_format_t	opt_fform;
extern file_format_t	fform_compr;
extern file_format_t	fform_compr_force;
extern file_format_t	script_fform;
extern ccp		script_varname;
extern int		script_array;
extern LowerUpper_t	opt_case;
extern int		opt_pmodes;
extern uint		opt_fmodes_include;
extern uint		opt_fmodes_exclude;
extern int		opt_install;
extern int		opt_pt_dir;
extern bool		opt_links;
extern bool		opt_rm_aiparam;
extern u32		opt_align;
extern u32		opt_align_u8;
extern u32		opt_align_lta;
extern u32		opt_align_pack;
extern u32		opt_align_brres;
extern u32		opt_align_breff;
extern u32		opt_align_breft;
extern bool		print_header;
extern bool		print_param;
extern int		print_sections;
extern double		opt_tri_area;
extern double		opt_tri_height;
extern bool		opt_kmp_flags;
extern bool		opt_id;
extern bool		opt_base64;
extern bool		opt_db64;
extern EncodeMode_t	opt_coding64;
extern bool		opt_verify;
extern ccp		opt_cache;
extern ccp		opt_cname;
extern ccp		opt_log_cache;
extern int		parallel_count;
extern bool		opt_round;
extern int		brief_count;
extern int		long_count;
//extern int		full_count;
extern int		no_wildcards_count;
extern int		inorder_count;
extern int		pipe_count;
extern int		delta_count;
extern int		diff_count;
extern int		minimize_level;
extern int		export_count;
extern int		all_count;
extern bool		raw_mode;
extern SortMode_t	opt_sort;
extern bool		opt_le_menu;
extern bool		opt_9laps;
extern ccp		opt_ui_source;
extern ccp		opt_cup_icons;
extern ccp		opt_title_screen;
extern u32		opt_max_file_size;
extern int		opt_compr_mode;
extern u32		opt_compr;
extern bool		opt_norm;
extern int		need_norm;		// enabled if > 0
extern bool		opt_no_copy;
extern bool		opt_fast;
extern char		*opt_basedir;
extern int		opt_recurse;
extern int		opt_ext;
extern bool		opt_decode;
extern bool		opt_cut;
extern bool		opt_cmpr_valid;
extern u8		opt_cmpr_def[8];
extern uint		opt_n_images;
extern uint		opt_max_images;
extern uint		opt_min_mipmap_size;	// minimal mipmap size
extern int		opt_mipmaps;
extern bool		opt_strip;
extern bool		opt_pre_convert;
extern bool		opt_encode_all;
extern bool		opt_encode_img;
extern bool		opt_no_encode;
extern bool		opt_no_recurse;
extern bool		opt_auto_add;
extern bool		opt_no_echo;
extern bool		opt_generic;
extern bool		opt_raw;
extern bool		opt_tracks;
extern bool		opt_arenas;
extern bool		opt_order_all;
extern ccp		opt_order_by;
extern ccp		opt_write_tracks;
extern bool		opt_no_check;
extern CheckMode_t	global_check_mode;
extern ccp		opt_flag_file;
extern int		opt_xtridata;
extern int		have_patch_count;	// <=0: disable patching at all
extern ParamField_t	patch_bmg_list;
extern FormatField_t	patch_image_list;
extern StringField_t	ct_dir_list;

extern double		epsilon_pos;		// 'p' Position
extern double		epsilon_rot;		// 'r' Rotation
extern double		epsilon_scale;		// 's' Scale
extern double		epsilon_time;		// 't' Time

extern ccp		analyze_fname;
extern FILE		* analyze_file;

extern       char	iobuf [0x400000];	// global io buffer
//extern const char	zerobuf[0x100];		// global zero buffer

extern const char indent_msg[]; //   N * "> " + NULL
extern const char section_sep[];
extern const char section_end[];

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////

#endif // SZS_LIB_STD_H
