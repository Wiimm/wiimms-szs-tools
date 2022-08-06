
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

#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include "lib-std.h"
#include "lib-szs.h"
#include "lib-rarc.h"
#include "lib-pack.h"
#include "lib-brres.h"
#include "lib-breff.h"
#include "lib-xbmg.h"
#include "lib-kcl.h"
#include "lib-kmp.h"
#include "lib-lecode.h"
#include "lib-mdl.h"
#include "lib-pat.h"
#include "lib-image.h"
#include "lib-object.h"
#include "lib-staticr.h"
#include "lib-ctcode.h"
#include "lib-common.h"
#include "lib-bzip2.h"
#include "dclib-utf8.h"
#include "dclib-ui.h"
#include "lib-mkw.h"
#include "db-mkw.h"
#include "crypt.h"
#include "logo.inc"
#include "sha1-db.inc"

#if defined(TEST) && !defined(__APPLE__) && !defined(__CYGWIN__)
  #include <mcheck.h>
#endif

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    vars			///////////////
///////////////////////////////////////////////////////////////////////////////

ccp		opt_config		= 0;
bool		opt_no_pager		= false;
ccp		tool_name		= "?";
ccp		std_share_path		= 0;
ccp		share_path		= 0;
volatile int	verbose			= 0;
volatile int	logging			= 0;
volatile int	ext_errors		= 0;
volatile int	log_timing		= 0;
bool		allow_all		= false;
char		escape_char		= '%';
bool		use_utf8		= true;
bool		use_de			= false;
bool		ctcode_enabled		= false;
bool		lecode_enabled		= false;
uint		opt_ct_mode		= 0;	// calculated by ctcode_enabled & lecode_enabled
int		testmode		= 0;
int		force_count		= 0;
uint		opt_tiny		= 0;
bool		force_kmp		= false;
int		disable_checks		= 0;
OffOn_t		opt_battle_mode		= OFFON_AUTO;
OffOn_t		opt_export_flags	= OFFON_AUTO;
int		opt_route_options	= 0;
OffOn_t		opt_wim0		= OFFON_AUTO;
ccp		opt_source		= 0;
StringField_t	source_list		= {0};
ccp		opt_reference		= 0;
ccp		opt_dest		= 0;
bool		opt_mkdir		= false;
bool		opt_overwrite		= false;
bool		opt_number		= false;
bool		opt_remove_src		= false;
bool		opt_remove_dest		= false;
bool		opt_update		= false;
bool		opt_preserve		= false;
ccp		opt_extract		= 0;
int		opt_ignore		= 0;
bool		opt_ignore_setup	= false;
bool		opt_purge		= false;
file_format_t	opt_fform		= FF_UNKNOWN;
file_format_t	fform_compr		= FF_YAZ0;
file_format_t	fform_compr_force	= 0;
file_format_t	script_fform		= FF_TXT;
ccp		script_varname		= 0;
int		script_array		= 0;
LowerUpper_t	opt_case		= LOUP_AUTO;
int		opt_pmodes		= 0;
uint		opt_fmodes_include	= 0;
uint		opt_fmodes_exclude	= 0;
int		opt_install		= 0;
int		opt_pt_dir		= 0;
bool		opt_links		= false;
bool		opt_rm_aiparam		= false;
u32		opt_align		= 0;
u32		opt_align_u8		= U8_DEFAULT_ALIGN;
u32		opt_align_pack		= PACK_SUBFILE_ALIGN;
u32		opt_align_brres		= BRRES_DEFAULT_ALIGN;
u32		opt_align_breff		= BREFF_DEFAULT_ALIGN;
u32		opt_align_breft		= 0;
bool		print_header		= true;
bool		print_param		= true;
int		print_sections		= 0;
double		opt_tri_area		= 1.0;
double		opt_tri_height		= 1.0;
bool		opt_kmp_flags		= true;
bool		opt_id			= false;
bool		opt_base64		= false;
bool		opt_db64		= false;
EncodeMode_t	opt_coding64		= ENCODE_OFF;
bool		opt_verify		= false;
ccp		opt_cache		= 0;
ccp		opt_cname		= 0;
bool		opt_round		= false;
int		brief_count		= 0;
int		long_count		= 0;
//int		full_count		= 0;
int		pipe_count		= 0;
int		delta_count		= 0;
int		diff_count		= 0;
int		minimize_level		= 0;
int		export_count		= 0;
int		all_count		= 0;
bool		raw_mode		= false;
SortMode_t	opt_sort		= SORT_NONE;
u32		opt_max_file_size	= 100*MiB;
int		opt_compr_mode		= 0;
u32		opt_compr		= 9;
bool		opt_norm		= false;
int		need_norm		= 0;	// enabled if > 0
bool		opt_fast		= false;
char		*opt_basedir		= 0;
int		opt_recurse		= -1;
int		opt_ext			= 0;
bool		opt_decode		= false;
bool		opt_cut			= false;
bool		opt_cmpr_valid		= false;
u8		opt_cmpr_def[8]		= { 0x00,0x00, 0x00,0x20, 0xff,0xff,0xff,0xff };
uint		opt_n_images		= 0;
uint		opt_max_images		= 5;
uint		opt_min_mipmap_size	= 8;	// minimal mipmap size
int		opt_mipmaps		= 0;
bool		opt_strip		= false;
bool		opt_pre_convert		= false;
bool		opt_encode_all		= false;
bool		opt_encode_img		= false;
bool		opt_no_encode		= false;
bool		opt_no_recurse		= false;
bool		opt_auto_add		= false;
bool		opt_no_echo		= false;
bool		opt_generic		= false;
bool		opt_raw			= false;
bool		opt_tracks		= false;
bool		opt_arenas		= false;
bool		opt_order_all		= false;
ccp		opt_order_by		= 0;
ccp		opt_write_tracks	= 0;
bool		opt_no_check		= false;
CheckMode_t	global_check_mode	= CMOD_DEFAULT;
ccp		opt_flag_file		= 0;
int		opt_xtridata		= 0;
int		have_patch_count	= 0;	// <=0: disable patching at all
int		have_kmp_patch_count	= 0;
int		have_kcl_patch_count	= 0;
ParamField_t	patch_bmg_list		= {0};
FormatField_t	patch_image_list	= {0};
StringField_t	ct_dir_list		= {0};
ccp		analyze_fname		= 0;
FILE		* analyze_file		= 0;

double		epsilon_pos		= DEF_EPSILON_POS;	// 'p' Position
double		epsilon_rot		= DEF_EPSILON_ROT;	// 'r' Rotation
double		epsilon_scale		= DEF_EPSILON_SCALE;	// 's' Scale
double		epsilon_time		= DEF_EPSILON_TIME;	// 't' Time

char		iobuf [0x400000];		// global io buffer
//const char	zerobuf[0x100]	= {0};		// global zero buffer

const char indent_msg[] = "> > > > > > > > > > > > > > > > > > > > ";

const char section_sep[] =
    "\r\n"
    "#\f\r\n"
    "###############################################################################\r\n"
    "\r\n";

const char section_end[] =
    "\r\n"
    "#\f\r\n"
    "###############################################################################\r\n"
    "\r\n"
    "[END]\r\n"
    "# This section is ignored.\r\n"
    "\r\n";

// begin of warning+hint line, for checks
const char check_bowl[]   = "    + WARNING: ";
const char check_bohl[]   = "    - HINT: ";
const char check_bosl[]	  = "    * SLOT: ";
const char check_boil[]   = "    * INFO: ";
const char check_bosugl[] = "    * SUGGESTION: ";

int global_warn_count = 0;
int global_hint_count = 0;
int global_info_count = 0;

const ColorSet_t *colset = 0;
void (*print_title_func) ( FILE * f ) = 0;

const char WarnLevelNameLo[WLEVEL__N][8] =
{
    "unknown",
    "na",
    "ok",
    "fixed",
    "hint",
    "warn",
    "error",
    "fail",
    "fatal",
    "invalid",
};

const char WarnLevelNameUp[WLEVEL__N][8] =
{
    "UNKNOWN",
    "NA",
    "OK",
    "FIXED",
    "HINT",
    "WARN",
    "ERROR",
    "FAIL",
    "FATAL",
    "INVALID",
};

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    tables			///////////////
///////////////////////////////////////////////////////////////////////////////

// PAL: file offset: 3a5a08, 3 cannons, 4 floats each
//	3a5a08:    500.00000           0   6000.00000    -1.00000
//	3a5a18:    500.00000  5000.00000   6000.00000    -1.00000
//	3a5a28:    120.00000  2000.00000   1000.00000    45.00000

const u8 CannonDataLEX[4+3*16] =
{
 0,0,0,3, // number of cannons
 0x43,0xfa,0x00,0x00, 0x00,0x00,0x00,0x00, 0x45,0xbb,0x80,0x00, 0xbf,0x80,0x00,0x00,
 0x43,0xfa,0x00,0x00, 0x45,0x9c,0x40,0x00, 0x45,0xbb,0x80,0x00, 0xbf,0x80,0x00,0x00,
 0x42,0xf0,0x00,0x00, 0x44,0xfa,0x00,0x00, 0x44,0x7a,0x00,0x00, 0x42,0x34,0x00,0x00,
};

//-----------------------------------------------------------------------------

const u8 TestDataLEX[8*1] =
{
    0,	// offline_online
    0,	// n_offline
    0,	// n_online
   ~0,	// cond_bit
    0,  // game_mode
    0,	// random
    0,0
};

//-----------------------------------------------------------------------------

const char is_arena_name[ARENA__N][9] =
{
	"no",		// ARENA_NONE
	"maybe",	// ARENA_MAYBE
	"found",	// ARENA_FOUND
	"dispatch",	// ARENA_DISPATCH
};

//
///////////////////////////////////////////////////////////////////////////////
///////////////			debugging helper		///////////////
///////////////////////////////////////////////////////////////////////////////

void LogSHA1 ( ccp func, ccp file, uint line, cvp data, uint size, ccp info )
{
    if (data)
    {
	sha1_hash_t hash;
	SHA1(data,size,hash);

	sha1_hex_t hex;
	Sha1Bin2Hex(hex,hash);

	fprintf(stderr,"%s %s(), %s @%u : [%u] %s\n",
		hex, func, file, line, size, info ? info : "" );
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			   Setup			///////////////
///////////////////////////////////////////////////////////////////////////////

void SetupColors()
{
    //----- setup colors

    colorize_stdout = colorize_stdlog = opt_colorize; // reset it first
    colorize_stdout = GetFileColorized(stdout);
    colorize_stdlog = GetFileColorized(stdlog);

    colout = GetColorSet(colorize_stdout);
    collog = GetColorSet(colorize_stdlog);
    colset = collog;
    SetupStdMsg();

    PRINT("COLORIZE: out=%d[%zd], err=%d[%zd], log=%d[%zd], msg=%d[%zd]\n",
	colorize_stdout, strlen(colout->reset),
	colorize_stderr, strlen(colerr->reset),
	colorize_stdlog, strlen(collog->reset),
	colorize_stdmsg, strlen(colmsg->reset) );
}

///////////////////////////////////////////////////////////////////////////////
// [[2do]]

#if defined(TEST) || defined(DEBUG)
  #define LOG_PROGINFO 1	// 0:off, 1:on
#else
  #define LOG_PROGINFO 0	// 0:off, 1:on
#endif

//-----------------------------------------------------------------------------

void SetupLib ( int argc, char ** argv, ccp tname, ccp tvers, ccp ttitle )
{
    SetupProgname(argc,argv,tname,tvers,ttitle);
    ProgInfo.error_level = ERRLEV_HEADING;

 #if LOG_PROGINFO
    PRINT1("PROG1: %s | %s | %s\n",ProgInfo.progname,ProgInfo.progdir,ProgInfo.progpath);
    PRINT1("TOOL1: %s | %s | %s\n",ProgInfo.toolname,ProgInfo.toolversion,ProgInfo.tooltitle);
 #endif

    SetupTimezone(true);
    GetTimerMSec();
    SetupColors();

    AddDefaultToSizeofInfoMgr();
    //AddListToSizeofInfoMgr(si_list);

 #if defined(TEST) && !defined(__APPLE__) && !defined(__CYGWIN__)
    mtrace();
 #endif

    INIT_TRACE_ALLOC;
    //MALLOC(1234);

    GetErrorNameHook = LibGetErrorName;
    GetErrorTextHook = LibGetErrorText;

 #ifdef DEBUG
    if (!TRACE_FILE)
    {
	char fname[100];
	snprintf(fname,sizeof(fname),"_trace-%s.tmp",tname);
	TRACE_FILE = fopen(fname,"w");
	if (!TRACE_FILE)
	    fprintf(stderr,"open TRACE_FILE failed: %s\n",fname);
    }
 #endif

 #ifdef __BYTE_ORDER
    TRACE("__BYTE_ORDER=%d\n",__BYTE_ORDER);
 #endif
 #ifdef LITTLE_ENDIAN
    TRACE("LITTLE_ENDIAN=%d\n",LITTLE_ENDIAN);
 #endif
 #ifdef BIG_ENDIAN
    TRACE("BIG_ENDIAN=%d\n",BIG_ENDIAN);
 #endif

    // assertions

    TRACE("-\n");
    DASSERT(  1 == sizeof(u8)  );
    DASSERT(  2 == sizeof(u16) );
    DASSERT(  4 == sizeof(u32) );
    DASSERT(  8 == sizeof(u64) );
    DASSERT(  1 == sizeof(s8)  );
    DASSERT(  2 == sizeof(s16) );
    DASSERT(  4 == sizeof(s32) );
    DASSERT(  8 == sizeof(s64) );
    DASSERT(  4 == sizeof(float32) );
    DASSERT( 12 == sizeof(float3) );
    DASSERT( 24 == sizeof(double3) );

    DASSERT(     1 == sizeof(bmg_encoding_t) );
    DASSERT(    32 == sizeof(bmg_header_t) );

    DASSERT(   300 == strlen(Minus300) );
    DASSERT(     4 == sizeof(Color_t) );
    DASSERT(  0x40 == sizeof(ctcode_cup1_head_t) );
    DASSERT( 0x100 == sizeof(ctcode_cup1_data_t) );
    DASSERT(  0x40 == sizeof(ctcode_crs1_head_t) );
    DASSERT( 0x100 == sizeof(ctcode_crs1_data_t) );

    TRACE("-\n");


    //----- more assertions

    #if HAVE_ASSERT
    {
	double3 d3;
	ASSERT( (double*)&d3 ==  d3.v );
	ASSERT( d3.v+0 == &d3.x );
	ASSERT( d3.v+1 == &d3.y );
	ASSERT( d3.v+2 == &d3.z );

	DEFINE_VAR(var);
	ASSERT( &var.d  == var.v );
	ASSERT( &var.d  == var.d3.v );
	ASSERT( var.v+0 == &var.x );
	ASSERT( var.v+1 == &var.y );
	ASSERT( var.v+2 == &var.z );

	ASSERT( sizeof(FuncParam_t) <= sizeof(double3) );
	ASSERT( sizeof(FuncInfo_t)  <= sizeof(double3) );

	ASSERT( sizeof(mdl_sect1_t) == 0xd0 );
    }
    #endif


    //----- initialize data structures

    InitializeParamField(&patch_bmg_list);
    InitializeFormatField(&patch_image_list);
    stdlog = stdout;
    MySeedByTime();


    //----- setup std_share_path

 #ifndef __CYGWIN__
    static const char share[] = "/share/szs";
    std_share_path = "/usr/local/share/szs";
 #endif

    ccp progdir = ProgramDirectory();
    PRINT0("progdir=%s\n",progdir);
    if (progdir)
    {
     #ifdef __CYGWIN__
	std_share_path = progdir;
     #else
	uint plen = strlen(progdir);
	if ( plen >= 4 && !strcmp(progdir+plen-4,"/bin") )
	    std_share_path = MEMDUP2(progdir,plen-4,share,strlen(share));
     #endif
    }


    //--- misc

    SetupStandardSZS();

 #if LOG_PROGINFO
    PRINT1("PROG2: %s | %s | %s\n",ProgInfo.progname,ProgInfo.progdir,ProgInfo.progpath);
    PRINT1("TOOL2: %s | %s | %s\n",ProgInfo.toolname,ProgInfo.toolversion,ProgInfo.tooltitle);
 #endif
}

///////////////////////////////////////////////////////////////////////////////

enumError CheckEnvOptions2 ( ccp varname, check_opt_func func )
{
    const enumError err = CheckEnvOptions("WIIMMS_SZS_TOOLS",func);
    return err ? err : CheckEnvOptions(varname,func);
}

///////////////////////////////////////////////////////////////////////////////

void NormalizeOptions
(
    uint	log_level	// >0: print PROGRAM_NAME and pathes
)
{
    SetupColors();
    ProgInfo.error_level = logging > 0 || ext_errors ? ERRLEV_EXTENDED : ERRLEV_HEADING;


    //--- load configuration

    const config_t *config = GetConfig();
    if (config)
	share_path = config->share_path;


    //--- logging & log_timing

    if ( logging >= 3 && !log_timing )
	log_timing++;


    //--- diff, delta and minimize

    minimize_level = delta_count < 9 ? delta_count : 9;
    if ( diff_count > 0 )
    {
	minimize_level = 10;
	if ( delta_count < 2 )
	     delta_count = 2;
	if ( brief_count < 2 )
	     brief_count = 2;
	print_header = false;
    }


    //--- compatibility settings

    if ( compatible < COMPAT_1_46 )
	opt_kmp_flags = false;

    if ( verbose < 0 && PATCH_FILE_MODE & PFILE_F_DEFAULT )
	PATCH_FILE_MODE = PFILE_M_SILENT;

    global_check_mode = opt_no_check
			? 0
			: GetCheckMode(false,brief_count>0,verbose<0,long_count>0);


    //--- CT-CODE & LE-CODE

    opt_ct_mode = CTM_NINTENDO;
    if (lecode_enabled)
    {
	ctcode_enabled = true;
	opt_ct_mode = CTM_LECODE2;
    }
    else if (ctcode_enabled)
	opt_ct_mode = CTM_CTCODE;
    PRINT0("OPT: %s\n",GetCtLeInfo());


    //--- loglevel

    if (force_count)
	force_kmp = true;

    if ( log_level > 0 )
    {
	if (print_title_func)
	    print_title_func(stdout);

	fprintf(stdlog,"PROGRAM_NAME    = %s\n",ProgInfo.progname);
	if ( compatible != COMPAT_CURRENT )
	    fprintf(stdlog,"COMPATIBILITY   = %s\n",PrintOptCompatible());
	fprintf(stdlog,"CONFIG_FILE     = %s\n",config->config_file);

	{
	    const config_t *config = GetConfig();
	    fprintf(stdlog,"CONFIG_FILE     = %s\n",config->config_file);
	    fprintf(stdlog,"STD_SHARE_PATH  = %s\n",std_share_path);
	    fprintf(stdlog,"SHARE_PATH      = %s\n",config->share_path);

	    int i;
	    const StringField_t *sf = GetSearchList();
	    ccp *str = sf->field;
	    for ( i = 0; i < sf->used; i++, str++ )
		fprintf(stdlog,"SEARCH_PATH[%d]  = %s\n",i,*str);

	    sf = GetAutoaddList();
	    str = sf->field;
	    for ( i = 0; i < sf->used; i++, str++ )
		fprintf(stdlog,"AUTOADD_PATH[%d] = %s\n",i,*str);
	}
	fprintf(stdlog,"\n");
    }
}

///////////////////////////////////////////////////////////////////////////////

ccp GetCtLeInfo()
{
    return PrintCircBuf("ct=%d, le=%d, mode=%d[%s]",
	ctcode_enabled, lecode_enabled, opt_ct_mode,
	GetCtModeNameBMG(opt_ct_mode,true) );
}

///////////////////////////////////////////////////////////////////////////////

const KeywordTab_t * CheckCommandHelper
	( int argc, char ** argv, const KeywordTab_t * key_tab )
{
    DASSERT(argv);
    DASSERT(key_tab);
    TRACE("CheckCommandHelper(%d,) optind=%d\n",argc,optind);

    if ( optind >= argc )
    {
	ERROR0(ERR_SYNTAX,"Missing command.\n");
	return 0;
    }

    int cmd_stat;
    ccp arg = argv[optind];
    const KeywordTab_t * cmd = ScanKeyword(&cmd_stat,arg,key_tab);

    if (!cmd)
    {
	if ( cmd_stat < 2 && toupper((int)*arg) == 'C' )
	{
	    if ( arg[1] == '-' )
	    {
		arg += 2;
		cmd = ScanKeyword(&cmd_stat,arg,key_tab);
	    }
	    else
	    {
		int cmd_stat2;
		cmd = ScanKeyword(&cmd_stat2,arg+1,key_tab);
	    }
	}

	if (!cmd)
	    PrintKeywordError(key_tab,arg,cmd_stat,0,0);
	else if ( opt_colorize < COLMD_ON )
	{
	    opt_colorize = COLMD_ON;
	    SetupColors();
	}
    }

    return cmd;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			error messages			///////////////
///////////////////////////////////////////////////////////////////////////////

ccp LibGetErrorName ( int stat, ccp ret_not_found )
{
    switch(stat)
    {
	case ERR_CACHE_USED:		return "SZS CACHE USED";
	case ERR_INVALID_IFORM:		return "INVALID IMAGE FORMAT";
	case ERR_INVALID_FFORM:		return "INVALID FILE FORMAT";
	case ERR_BZIP2:			return "BZIP2 ERROR";
	case ERR_PNG:			return "PNG ERROR";
    }
    return ret_not_found;
}

///////////////////////////////////////////////////////////////////////////////

ccp LibGetErrorText ( int stat, ccp ret_not_found )
{
    switch(stat)
    {
	case ERR_CACHE_USED:		return "SZS cache used";
	case ERR_INVALID_IFORM:		return "Invalid image format";
	case ERR_INVALID_FFORM:		return "Invalid file format";
	case ERR_BZIP2:			return "BZIP2 error";
	case ERR_PNG:			return "PNG error";
    }
    return ret_not_found;
}

///////////////////////////////////////////////////////////////////////////////

void SetupPager()
{
    if ( !opt_no_pager && isatty(fileno(stdout)) )
    {
	fflush(stdout);
	opt_colorize = 1;
	SetupColors();
	StdoutToPager();
    }
}

///////////////////////////////////////////////////////////////////////////////

void PrintHelpColor ( const InfoUI_t * iu )
{
    SetupPager();
    PrintHelp(iu,stdout,0,"HELP",0,URI_HOME,first_param?first_param->arg:0);
    ClosePager();
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			compatibility			///////////////
///////////////////////////////////////////////////////////////////////////////

compatible_t compatible = COMPAT_CURRENT;

const compatible_info_t compatible_info[COMPAT__N] =
{
    { COMPAT_0,		2385, "0.01" },
    { COMPAT_1_23,	4682, "1.23" },
    { COMPAT_1_39,	6123, "1.39" },
    { COMPAT_1_44,	6558, "1.44" },
    { COMPAT_1_46,	6614, "1.46" },
    { COMPAT_2_08,	7961, "2.08" },
    { COMPAT_CURRENT, REVISION_NUM, VERSION_NUM }
};

///////////////////////////////////////////////////////////////////////////////

const compatible_info_t * ScanCompatible ( ccp arg )
{
    if ( !arg || !*arg )
	return compatible_info + COMPAT_CURRENT;

    if ( *arg == 'r' || *arg == 'R' )
    {
	char *end;
	const uint rev = strtoul(arg+1,&end,10);
	if ( end == arg || *end )
	    return 0;

	compatible_t c = COMPAT_CURRENT;
	while ( c > COMPAT_0 && rev < compatible_info[c].revision )
	    c--;
	return compatible_info + c;
    }

    if ( *arg == 'v' || *arg == 'V' )
	arg++;

    if ( *arg >= '0' && *arg <= '9' )
    {
	compatible_t c = COMPAT_CURRENT;
	while ( c > COMPAT_0 && strcmp(arg,compatible_info[c].version) < 0 )
	    c--;
	return compatible_info + c;
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptCompatible ( ccp arg )
{
    const compatible_info_t *ci = ScanCompatible(arg);
    if (!ci)
    {
	ERROR0(ERR_SYNTAX,
		"Wrong argument for option --compatible: %s\n",arg);
	return 1;
    }

    compatible = ci->compatible;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

char * PrintOptCompatible()
{
    uint buf_size = 20;
    char *buf = GetCircBuf(buf_size);

    const compatible_info_t *ci = compatible_info
		+ ( (uint)compatible < COMPAT__N ? compatible : COMPAT_CURRENT );
    snprintf( buf, buf_size, "v%s, r%u", ci->version, ci->revision );
    return buf;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			warnings			///////////////
///////////////////////////////////////////////////////////////////////////////

warn_mode_t WARN_MODE = WARN_M_DEFAULT;

///////////////////////////////////////////////////////////////////////////////

static const KeywordTab_t opt_warn_tab[] =
{
  { 0,				"NONE",			"OFF",	WARN_M_ALL | WARN_F_HIDE },
  { WARN_M_DEFAULT,		"DEFAULT",		0,	WARN_M_ALL | WARN_F_HIDE },
  { WARN_M_ALL,			"ALL",			0,	WARN_M_ALL },

  { WARN_INVALID_OFFSET,	"INVALID-OFFSET",	0,	0 },

  { WARN_LOG,			"LOG",			0,	0 },
  { WARN_TEST,			"TEST",			"T",	0 },

  { 0,0,0,0 }
};

///////////////////////////////////////////////////////////////////////////////

int ScanOptWarn ( ccp arg )
{
    if (!arg)
	return 0;

    s64 stat = ScanKeywordList(arg,opt_warn_tab,0,true,0,WARN_MODE,
				"Option --warn",ERR_SYNTAX);
    if ( stat == -1 )
	return 1;

    WARN_MODE = stat;
    if ( WARN_MODE & WARN_LOG )
    {
	char buf[200];
	PrintWarnMode(buf,sizeof(buf),WARN_MODE);
	fprintf(stderr,"WARN MODE: %s\n",buf);
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

uint PrintWarnMode ( char *buf, uint bufsize, warn_mode_t mode )
{
    DASSERT(buf);
    DASSERT(bufsize>10);
    char *dest = buf;
    char *end = buf + bufsize - 1;

    mode = mode & WARN_M_ALL | WARN_F_HIDE;
    warn_mode_t mode1 = mode;

    const KeywordTab_t *ct;
    for ( ct = opt_warn_tab; ct->name1 && dest < end; ct++ )
    {
	if ( ct->opt & WARN_F_HIDE )
	    continue;

	if ( ct->opt ? (mode & ct->opt) == ct->id : mode & ct->id )
	{
	    if ( dest > buf )
		*dest++ = ',';
	    dest = StringCopyE(dest,end,ct->name1);
	    mode &= ~(ct->id|ct->opt);
	}
    }

    if ( mode1 == (WARN_M_DEFAULT|WARN_F_HIDE) )
	dest = StringCopyE(dest,end," (default)");
    else if (!mode1)
	dest = StringCopyE(dest,end,"(none)");

    *dest = 0;
    return dest-buf;
}

///////////////////////////////////////////////////////////////////////////////

ccp GetWarnMode()
{
    static char buf[200] = {0};
    if (!*buf)
	PrintWarnMode(buf,sizeof(buf),WARN_MODE);
    return buf;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			output_mode_t			///////////////
///////////////////////////////////////////////////////////////////////////////

output_mode_t output_mode = {0};

void InitializeOutputMode ( output_mode_t * outmode )
{
    DASSERT(outmode);
    outmode->syntax	= true;
    outmode->mode	= 1;
    outmode->cross_ref	= true;
    outmode->hex	= false;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		    warn_enum_t / warn_bits_t		///////////////
///////////////////////////////////////////////////////////////////////////////

const ccp warn_szs_name[WARNSZS__N] =
{
    "itempos",		// WARNSZS_ITEMPOS
    "self-it",		// WARNSZS_SELF_ITPH
    "no-minimap",	// WARNSZS_NO_MINIMAP
};

///////////////////////////////////////////////////////////////////////////////

ccp GetWarnSZSNames ( warn_bits_t ws, char sep )
{
    static const int order[] =
    {
	WARNSZS_ITEMPOS,
	WARNSZS_SELF_ITPH,
	WARNSZS_NO_MINIMAP,
	-1
    };

    char buf[100], *dest = buf;

    const int *op;
    for ( op = order; *op >= 0; op++ )
	if ( 1 << *op & ws && dest < buf+sizeof(buf)-1 )
	{
	    *dest++ = sep;
	    dest = StringCopyE(dest,buf+sizeof(buf),warn_szs_name[*op]);
	}

    return dest == buf ? EmptyString : CopyCircBuf0(buf+1,dest-buf-1);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		    [[obsolete+]] old_container		///////////////
///////////////////////////////////////////////////////////////////////////////
// old [[container]] will be replaced by Container_t & ContainerData_t
///////////////////////////////////////////////////////////////////////////////

DataContainer_t * UseSubContainer
(
    DataContainer_t	*dc,		// old_container to reuse, if
					// InDataContainer(dc,data) && ref:count == 1
    ContainerData_t	*cdata_src	// NULL or container data source
)
{
    if ( dc && dc->container.cdata == cdata_src )
	return dc;

    if (dc)
	FreeContainer(dc);

    dc = MALLOC(sizeof(*dc));
    noPRINT("CONTAINER: NEW(%p), dc_alloced=%d\n",dc,dc_alloced);
    DASSERT(dc);
    memset(dc,0,sizeof(*dc));

    if (cdata_src)
	cdata_src->ref_count++;
    CatchContainerData(&dc->container,false,cdata_src);
    ContainerData_t *cdata = dc->container.cdata;
    if (cdata)
    {
	dc->data = cdata->data;
	dc->size = cdata->size;
    }
    dc->ref_count = 1;

    return dc;
}

///////////////////////////////////////////////////////////////////////////////

DataContainer_t * NewContainer
(
    DataContainer_t	*dc,		// old_container to reuse,
					// if InDataContainer(dc,data)
    const void		*data,		// data to copy/move/link
    uint		size,		// size of 'data'
    CopyMode_t		mode		// copy mode on creation
)
{
    if ( InDataContainer(dc,data) && dc->ref_count == 1 )
	return dc;

    if (dc)
	FreeContainer(dc);

    dc = MALLOC(sizeof(*dc));
    noPRINT("CONTAINER: NEW(%p), dc_alloced=%d\n",dc,dc_alloced);
    DASSERT(dc);
    memset(dc,0,sizeof(*dc));

    CreateContainer(&dc->container,false,data,size,mode);
    ContainerData_t *cdata = dc->container.cdata;
    if (cdata)
    {
	dc->data = cdata->data;
	dc->size = cdata->size;
    }
    dc->ref_count = 1;
    return dc;
}

///////////////////////////////////////////////////////////////////////////////
// old [[container]] will be replaced by Container_t & ContainerData_t

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
)
{
    if (InDataContainer(dup_dc,data))
    {
	if ( dup_dc->ref_count == 1 )
	{
	    if (new_data)
		*new_data = data;
	    return dup_dc;
	}

	if (include_behind)
	{
	    const uint max = dup_dc->data + dup_dc->size - (u8*)data;
	    if ( size < max )
		size = max;
	}
    }

    DataContainer_t *dc = MALLOC(sizeof(*dc));
    noPRINT("CONTAINER: NEW(%p), dc_alloced=%d\n",dc,dc_alloced);
    DASSERT(dc);
    memset(dc,0,sizeof(*dc));

    CreateContainer(&dc->container,false,data,size,CPM_COPY);
    ContainerData_t *cdata = dc->container.cdata;
    if (cdata)
    {
	dc->data = cdata->data;
	dc->size = cdata->size;
    }
    dc->ref_count = 1;

    if (dup_dc)
	FreeContainer(dup_dc);

    if (new_data)
	*new_data = dc->data;

    return dc;
};

///////////////////////////////////////////////////////////////////////////////
// old [[container]] will be replaced by Container_t & ContainerData_t

DataContainer_t * AllocContainer
(
    // increments the reference counter and returns 'dc'

    DataContainer_t	*dc		// NULL or valid old_container
)
{
    if (dc)
    {
	dc->ref_count++;
	noPRINT("CONTAINER: ALLOC(%p), ref_count=%d\n",dc,dc->ref_count);
    }
    return dc;
}

///////////////////////////////////////////////////////////////////////////////
// old [[container]] will be replaced by Container_t & ContainerData_t

DataContainer_t * FreeContainer
(
    // decrements the reference counter and returns 'NULL'
    // if reference counter == 0: free data and/or dc

    DataContainer_t	*dc		// NULL or valid old_container
)
{
    if (dc)
    {
	noPRINT("CONTAINER: FREE(%p) ref_count=%d\n", dc, dc->ref_count );
	DASSERT( dc->ref_count > 0 );

	if (!--dc->ref_count)
	{
	    ResetContainer(&dc->container);

	    if (dc->data_alloced)
		FREE((void*)dc->data);

	    if (dc->dc_alloced)
		FREE(dc);
	    else
	    {
		dc->data = (void*)EmptyString;
		dc->size = 0;
		dc->data_alloced = false;
	    }
	}
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// old [[container]] will be replaced by Container_t & ContainerData_t

bool InDataContainer
(
    // return true, if ptr is in old_container

    const DataContainer_t *dc,		// NULL or valid old_container
    const void		  *ptr		// NULL or pointer to test
)
{
    return dc && ptr && (u8*)ptr >= dc->data && (u8*)ptr <= dc->data + dc->size;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			string helpers			///////////////
///////////////////////////////////////////////////////////////////////////////

int NintendoCmp ( ccp path1, ccp path2 )
{
    noPRINT("NintendoCmp( | %s | %s | )\n",path1,path2);

    // try to sort in a nintendo like way
    //  1.) ignoring case but sort carefully directories
    //  2.) files before sub directories

    static uchar transform[0x100] = {0,0};
    if (!transform[1]) // ==> setup needed once!
    {
	// define some special characters

	uint index = 1;
	transform[(u8)'/'] = index++;
	transform[(u8)'.'] = index++;

	// define all characters until and excluding 'A'

	uint i;
	for ( i = 1; i < 'A'; i++ )
	    if (!transform[i])
		transform[i] = index++;

	// define letters

	for ( i = 'A'; i <= 'Z'; i++ )
	    transform[i] = transform[i-'A'+'a'] = index++;

	// define all other

	for ( i = 1; i < sizeof(transform); i++ )
	    if (!transform[i])
		transform[i] = index++;

	DASSERT( index <= sizeof(transform) );
     #if defined(TEST) && defined(DEBUG)
	//HexDump16(stderr,0,0,transform,sizeof(transform));
     #endif
    }


    //--- eliminate equal characters

    while ( *path1 == *path2 )
    {
	if (!*path1)
	    return 0;
	path1++;
	path2++;
    }

    //--- start the case+path test

    const uchar * p1 = (const uchar *)path1;
    const uchar * p2 = (const uchar *)path2;
    while ( *p1 || *p2 )
    {
	int ch1 = transform[*p1++];
	int ch2 = transform[*p2++];
	int stat = ch1 - ch2;
	noPRINT("%02x,%02x,%c  %02x,%02x,%c -> %2d\n",
		p1[-1], ch1, ch1, p2[-1], ch2, ch2, stat );
	if (stat)
	{
	 #ifdef WSZST // special case for Wiimms SZS Tools
	    // sort files before dirs
	    const bool indir1 = strchr((ccp)p1-1,'/') != 0;
	    const bool indir2 = strchr((ccp)p2-1,'/') != 0;
	    return  indir1 != indir2 ? indir1 - indir2 : stat;
	 #else
	    return stat;
	 #endif
	}

	if ( ch1 == 1 )
	    break;
    }
    return *path1 - *path2;
}

///////////////////////////////////////////////////////////////////////////////

void PrintNumF
(
    FILE		*f,		// output stream
    const u8		*data,		// valid pointer to data
    uint		size,		// size of 'data'
    ccp			type_str	// NULL or type string
)
{
    DASSERT(f);
    DASSERT(data);

    const u8 *end = data + size;
    while ( type_str && data < end )
    {
	switch( type_str[0] << 8 | type_str[1] )
	{
	    case 'u' << 8 | '1':
		fprintf(f," %4u",*data++);
		type_str += 2;
		break;

	    case 'u' << 8 | '2':
		fprintf(f," %4u",be16(data));
		data += 2;
		type_str += 2;
		break;

	    case 'x' << 8 | '1':
		fprintf(f," 0x%02x",*data++);
		type_str += 2;
		break;

	    case 'x' << 8 | '2':
		fprintf(f," 0x%02x",be16(data));
		data += 2;
		type_str += 2;
		break;

	    case 'f' << 8 | '4':
		fprintf(f," %6.3f",bef4(data));
		data += 4;
		type_str += 2;
		break;

	    default:
	       type_str = 0;
	       break;
	}
    }

    while ( data < end )
	fprintf(f," 0x%02x",*data++);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			word compare functions		///////////////
///////////////////////////////////////////////////////////////////////////////

uint CreateHaystackListWCMP
(
    char	* buf,		// valid pointer to output buffer
    uint	buf_size,	// soue of 'buf'
    ccp		* source	// NULL terminated list with source Strings
)
{
    DASSERT(buf);
    DASSERT( buf_size > 10 );
    DASSERT(source);

    char *dest = buf;
    char * dest_end = buf + buf_size - 4;

    uint src_count = 0;
    while (*source)
    {
	ccp src = *source++;
	if (src_count++)
	    *dest++ = '|';

	*dest++ = ' ';
	bool have_space = true;
	while ( dest < dest_end )
	{
	    const uchar ch = *src++;
	    if ( ch >= 'a' && ch <= 'z' || ch >= '0' && ch <= '9' || ch ==  '$' )
	    {
		*dest++ = ch;
		have_space = false;
	    }
	    else if ( ch >= 'A' && ch <= 'Z' )
	    {
		*dest++ = ch + ('a'-'A');
		have_space = false;
	    }
	    else if (!ch)
	    {
		break;
	    }
	    else if (!have_space)
	    {
		*dest++ = ' ';
		have_space = true;
	    }
	}
	if (!have_space)
	    *dest++ = ' ';
    }

    DASSERT( dest < buf + buf_size );
    *dest = 0;
    return dest - buf;
}

///////////////////////////////////////////////////////////////////////////////

uint CreateHaystackWCMP
(
    char	* buf,		// valid pointer to output buffer
    uint	buf_size,	// soue of 'buf'
    ccp		source		// source String
)
{
    ccp list[2] = { source, 0 };
    return CreateHaystackListWCMP(buf,buf_size,list);
}

///////////////////////////////////////////////////////////////////////////////

uint CreateNeedleWCMP
(
    char	* buf,		// valid pointer to output buffer
    uint	buf_size,	// soue of 'buf'
    ccp		source		// NULL terminated list with source Strings
)
{
    DASSERT(buf);
    DASSERT( buf_size > 10 );
    DASSERT(source);

    char *dest = buf;
    char * dest_end = buf + buf_size - 4;

    if ( *source == '*' )
	source++;
    else
	*dest++ = ' ';

    bool have_space = true;
    while ( dest < dest_end )
    {
	const uchar ch = *source++;
	if ( ch >= 'a' && ch <= 'z' || ch >= '0' && ch <= '9' || ch ==  '$' )
	{
	    *dest++ = ch;
	    have_space = false;
	}
	else if ( ch >= 'A' && ch <= 'Z' )
	{
	    *dest++ = ch + ('a'-'A');
	    have_space = false;
	}
	else if (!ch)
	{
	    break;
	}
	else if (!have_space)
	{
	    *dest++ = ' ';
	    have_space = true;
	}
    }

    DASSERT( dest < buf + buf_size );
    *dest = 0;
    return dest - buf;
}

///////////////////////////////////////////////////////////////////////////////

bool FindWCMP
(
    ccp		haystack,	// search in this string
    ccp		needle,		// try to find this string
    uint	flags,		// bit field:
				//	bit-0 set = 1: 'haystack' already in WCMP format
				//	bit-1 set = 2: 'needle' already in WCMP format
    char	* tempbuf,	// NULL or fast buffer for temporary WCMP conversions
    uint	tempbuf_size	// size of 'buf'
)
{
    if ( !haystack || !*haystack )
	return false;
    if ( !needle || !*needle )
	return true;

    const uint extra_size = 3;

    char *alloced_haystack = 0;
    if (!(flags&1))
    {
	ccp haystack_list[2] = { haystack, 0 };

	uint len = strlen(haystack) + extra_size;
	if ( tempbuf && len <= tempbuf_size )
	{
	    len = CreateHaystackListWCMP(tempbuf,tempbuf_size,haystack_list);
	    haystack = tempbuf;

	    // adjust temp buf for next access
	    tempbuf += len+1;
	    tempbuf_size -= len+1;
	}
	else
	{
	    alloced_haystack = MALLOC(len);
	    CreateHaystackListWCMP(alloced_haystack,len,haystack_list);
	    needle = alloced_haystack;
	}
    }

    char *alloced_needle = 0;
    if (!(flags&2))
    {
	uint len = strlen(needle) + extra_size;
	if ( tempbuf && len <= tempbuf_size )
	{
	    len = CreateNeedleWCMP(tempbuf,tempbuf_size,needle);
	    needle = tempbuf;

	    // adjust of temp buf not needed
	    //tempbuf += len+1;
	    //tempbuf_size -= len+1;
	}
	else
	{
	    alloced_needle = MALLOC(len);
	    CreateNeedleWCMP(alloced_needle,len,needle);
	    needle = alloced_needle;
	}
    }

    const bool stat = strstr(haystack,needle) != 0;

    if (alloced_haystack)
	FREE(alloced_haystack);
    if (alloced_needle)
	FREE(alloced_needle);

    return stat;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

search_filter_t * AllocSearchFilter ( uint n )
{
    search_filter_t *f = CALLOC(n+1,sizeof(search_filter_t));
    f->mode = n;
    return f+1;
}

void FreeSearchFilter ( search_filter_t *f )
{
    if (f)
    {
	int i, n = f[-1].mode;
	for ( i = 0; i < n; i++ )
	    FreeString(f[i].string);
	FREE(f-1);
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		     string substitutions		///////////////
///////////////////////////////////////////////////////////////////////////////

int ScanEscapeChar ( ccp arg )
{
    if ( arg && strlen(arg) > 1 )
    {
	ERROR0(ERR_SYNTAX,"Invalid character (option --esc): '%s'\n",arg);
	return -1;
    }

    escape_char = arg ? *arg : 0;
    return (unsigned char)escape_char;
}

///////////////////////////////////////////////////////////////////////////////

char * SubstString
(
    char		* buf,		// destination buffer
    size_t		bufsize,	// size of 'buf'
    SubstString_t	* tab,		// replacement table
    ccp			source,		// source text
    int			* count,	// not NULL: number of replacements
    char		escape_char	// the escape character, usually '%'
)
{
    ASSERT(buf);
    ASSERT(bufsize > 1);
    ASSERT(tab);
    TRACE("SubstString(%s)\n",source);

    char tempbuf[PATH_MAX];
    int conv_count = 0;

    char *dest = buf;
    char *end = buf + bufsize + 1;
    if (source)
	while ( dest < end && *source )
	{
	    if ( *source != escape_char && *source != 1 )
	    {
		*dest++ = *source++;
		continue;
	    }
	    if ( source[0] == source[1] )
	    {
		source++;
		*dest++ = *source++;
		continue;
	    }
	    ccp start = source++;

	    bool check_prev = *source == '?';
	    if (check_prev)
		source++;

	    u32 p1, p2, stat;
	    source = ScanRangeU32(source,&stat,&p1,&p2,0,~(u32)0);
	    if ( stat == 1 )
		p1 = 0;
	    else if ( stat < 1 )
	    {
		p1 = 0;
		p2 = ~(u32)0;
	    }

	    char ch = *source++;
	    int convert = 0;
	    if ( ch == 'u' || ch == 'U' )
	    {
		convert++;
		ch = *source++;
	    }
	    else if ( ch == 'l' || ch == 'L' )
	    {
		convert--;
		ch = *source++;
	    }
	    if (!ch)
		break;

	    size_t count = source - start;

	    SubstString_t * ptr;
	    for ( ptr = tab; ptr->c1; ptr++ )
		if ( ch == ptr->c1 || ch == ptr->c2 )
		{
		    if (ptr->str)
		    {
			const size_t slen = strlen(ptr->str);
			if ( p1 > slen )
			    p1 = slen;
			if ( p2 > slen )
			    p2 = slen;
			count = p2 - p1;
			start = ptr->str+p1;
			conv_count++;
		    }
		    else
			count = 0;
		    break;
		}

	    if (!ptr->c1) // invalid conversion
		convert = 0;

	    if ( count > sizeof(tempbuf)-1 )
		 count = sizeof(tempbuf)-1;
	    TRACE("COPY '%.*s' conv=%d\n",(int)count,start,convert);
	    if ( convert > 0 )
	    {
		char * tp = tempbuf;
		size_t copy_count = count;
		while ( copy_count-- > 0 )
		    *tp++ = toupper((int)*start++);
		*tp = 0;
	    }
	    else if ( convert < 0 )
	    {
		char * tp = tempbuf;
		size_t copy_count = count;
		while ( copy_count-- > 0 )
		    *tp++ = tolower((int)*start++); // cygwin needs the '(int)'
		*tp = 0;
	    }
	    else
	    {
		memcpy(tempbuf,start,count);
		tempbuf[count] = 0;
	    }
	    char * new_dest
		= NormalizeFileName(dest,end-dest,tempbuf,ptr->allow_slash,use_utf8,TRSL_NONE);
	    if ( check_prev && *dest == '.' )
	    {
		int len = new_dest - dest;
		if ( dest - buf > len && !memcmp(dest-len,dest,len) )
		    new_dest = dest;
	    }
	    dest = new_dest;
	}

    if (count)
	*count = conv_count;
    *dest = 0;
    return dest;
}

///////////////////////////////////////////////////////////////////////////////

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
)
{
    DASSERT(buf);
    DASSERT(source);

    if ( !dest || !*dest )
    {
	StringCopyS(buf,bufsize,source);
	return 0;
    }

    if ( source && !strcmp(source,"-") && ( !opt_dest || !strcmp(opt_dest,dest) ))
    {
	StringCopyS(buf,bufsize,"-");
	return 0;
    }


    //---- check source path

    char src_path[PATH_MAX];
    StringCopyS(src_path,sizeof(src_path),source);

    char * p = strrchr(src_path,'/');
    ccp dir, name, fname;
    if (p)
    {
	dir = src_path;
	*p++ = 0;
	name = p;
	fname = source + ( name - src_path );
    }
    else
    {
	dir = ".";
	name = src_path;
	fname = source;
    }

    p = strrchr(name,'.');
    ccp ext;
    if (p)
    {
	*p = 0;
	ext = source + ( p - src_path );
    }
    else
	ext = "";

    SubstString_t subst_tab[] =
    {
	{ 'q', 'Q', 1, source },
	{ 'p', 'P', 1, dir },
	{ 'f', 'F', 1, fname },
	{ 'n', 'N', 1, name },
	{ 'e', 'E', 1, ext },
	{ 't', 'T', 1, default_ext ? default_ext : ext },
	{0,0,0,0}
    };

 #if HAVE_PRINT0
    {
	const SubstString_t * ptr;
	for ( ptr = subst_tab; ptr->c1; ptr++ )
	    PRINT(" %c %c %u -> %s\n",
		ptr->c1, ptr->c2, ptr->allow_slash, ptr->str );
    }
 #endif

    char dest_path[PATH_MAX];
    if (dest_is_dir)
	dest = PathCatPP(dest_path,sizeof(dest_path),dest,"/");
    else if (IsDirectory(dest,0))
    {
	if (!dest_fname)
	    dest_fname = source;
	ccp p = strrchr(dest_fname,'/');
	dest = PathCatPP(dest_path,sizeof(dest_path), dest, p ? p+1 : dest_fname );
    }

    int conv_count;
    SubstString(buf,bufsize,subst_tab,dest,&conv_count,escape_char);
    if ( opt_number && !opt_overwrite )
	NumberedFilename(buf,bufsize,buf,default_ext,0,true);

    return conv_count;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			FormatField_t			///////////////
///////////////////////////////////////////////////////////////////////////////

#undef FORMAT_FIELD_GROW
#define FORMAT_FIELD_GROW(s) (s)/4+100

///////////////////////////////////////////////////////////////////////////////

void InitializeFormatField ( FormatField_t * ff )
{
    DASSERT(ff);
    memset(ff,0,sizeof(*ff));
}

///////////////////////////////////////////////////////////////////////////////

void ResetFormatField ( FormatField_t * ff )
{
    ASSERT(ff);
    if ( ff && ff->used > 0 )
    {
	ASSERT(ff->list);
	FormatFieldItem_t *ptr = ff->list, *end;
	for ( end = ptr + ff->used; ptr < end; ptr++ )
	    FreeString(ptr->key);
	FREE(ff->list);
    }
    InitializeFormatField(ff);
}

///////////////////////////////////////////////////////////////////////////////

void MoveFormatField ( FormatField_t * dest, FormatField_t * src )
{
    DASSERT(src);
    DASSERT(dest);
    if ( src != dest )
    {
	ResetFormatField(dest);
	dest->list  = src->list;
	dest->used  = src->used;
	dest->size  = src->size;
	InitializeFormatField(src);
    }
}

///////////////////////////////////////////////////////////////////////////////

static uint FindFormatFieldHelper ( const FormatField_t * ff, bool * p_found, ccp key )
{
    ASSERT(ff);

    int beg = 0;
    if ( ff && key )
    {
	int end = ff->used - 1;
	while ( beg <= end )
	{
	    uint idx = (beg+end)/2;
	    int stat = strcmp(key,ff->list[idx].key);
	    if ( stat < 0 )
		end = idx - 1 ;
	    else if ( stat > 0 )
		beg = idx + 1;
	    else
	    {
		TRACE("FindFormatFieldHelper(%s) FOUND=%d/%d/%d\n",
			key, idx, ff->used, ff->size );
		if (p_found)
		    *p_found = true;
		return idx;
	    }
	}
    }

    TRACE("FindFormatFieldHelper(%s) failed=%d/%d/%d\n",
		key, beg, ff->used, ff->size );

    if (p_found)
	*p_found = false;
    return beg;
}

///////////////////////////////////////////////////////////////////////////////

int FindFormatFieldIndex ( const FormatField_t * ff, ccp key, int not_found_value )
{
    bool found;
    const int idx = FindFormatFieldHelper(ff,&found,key);
    return found ? idx : not_found_value;
}

///////////////////////////////////////////////////////////////////////////////

FormatFieldItem_t * FindFormatField ( const FormatField_t * ff, ccp key )
{
    bool found;
    const int idx = FindFormatFieldHelper(ff,&found,key);
    return found ? ff->list + idx : 0;
}

///////////////////////////////////////////////////////////////////////////////

FormatFieldItem_t * FindFormatFieldNC ( const FormatField_t * ff, ccp key )
{
    DASSERT(ff);
    DASSERT(key);

    FormatFieldItem_t *ptr = ff->list, *end;
    for ( end = ptr + ff->used; ptr < end; ptr++ )
	if (!strcasecmp(ptr->key,key))
	    return ptr;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

FormatFieldItem_t * MatchFormatField ( const FormatField_t * ff, ccp key )
{
    DASSERT(ff);
    DASSERT(key);

    FormatFieldItem_t *ptr = ff->list, *end;
    for ( end = ptr + ff->used; ptr < end; ptr++ )
	if (MatchPattern(ptr->key,key,'/'))
	    return ptr;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

static FormatFieldItem_t * InsertFormatFieldHelper ( FormatField_t * ff, int idx )
{
    DASSERT(ff);
    DASSERT( ff->used <= ff->size );
    noPRINT("+FF: %u/%u/%u\n",idx,ff->used,ff->size);
    if ( ff->used == ff->size )
    {
	ff->size += FORMAT_FIELD_GROW(ff->size);
	ff->list = REALLOC(ff->list,ff->size*sizeof(*ff->list));
    }
    DASSERT( idx <= ff->used );
    FormatFieldItem_t * dest = ff->list + idx;
    memmove(dest+1,dest,(ff->used-idx)*sizeof(*dest));
    ff->used++;
    return dest;
}

///////////////////////////////////////////////////////////////////////////////

bool RemoveFormatField ( FormatField_t * ff, ccp key )
{
    bool found;
    uint idx = FindFormatFieldHelper(ff,&found,key);
    if (found)
    {
	ff->used--;
	ASSERT( idx <= ff->used );
	FormatFieldItem_t * dest = ff->list + idx;
	FREE((char*)dest);
	memmove(dest,dest+1,(ff->used-idx)*sizeof(*dest));
    }
    return found;
}

///////////////////////////////////////////////////////////////////////////////

static void SetFormatFieldParam
(
    FormatFieldItem_t	* item,		// valid attrib item
    ccp			key,		// comma list of keys
    ccp			keyend		// end of key
)
{
    DASSERT(item);
    DASSERT(key);
    DASSERT(keyend);

    bool first = true;
    for(;;)
    {
	while ( key < keyend && ( *key >= 0 && *key <= ' ' || *key == ',' ))
	    key++;
	char buf[100];
	char *bufptr = buf, *bufend = buf + sizeof(buf) - 1;
	while ( key < keyend && *key > ' ' && *key != ',' )
	{
	    if ( bufptr < bufend )
		*bufptr++ = toupper((int)*key);
	    key++;
	}

	if ( bufptr == buf )
	    break;
	*bufptr = 0;
	noPRINT("TEST: |%s| %zu\n",buf,bufptr-buf);

	if ( *buf >= '0' && *buf <= '9' )
	{
	    char *end;
	    const uint num = strtoul(buf,&end, buf[0]=='0' && buf[1]=='x' ? 16 : 10 );
	    if ( end > buf && !*end )
	    {
		item->num = num;
		continue;
	    }
	}

	if (first)
	{
	    first = false;
	    const file_format_t fform = GetByNameFF(buf);
	    if ( fform != FF_UNKNOWN )
	    {
		item->fform = fform;
		continue;
	    }
	}

	int iform = ScanImageFormat(buf);
	if ( iform >= 0 )
	{
	    item->iform = iform;
	    continue;
	}

	int pform = ScanPaletteFormat(buf);
	if ( pform >= 0 )
	{
	    item->pform = pform;
	    continue;
	}

	if (ScanPatchImage(buf,&item->patch_mode))
	    continue;
    }

    noPRINT("SET FORMAT: %d %d %d = %s\n",
	item->fform, item->iform, item->pform, item->key );
}

///////////////////////////////////////////////////////////////////////////////

FormatFieldItem_t * InsertFormatField
(
    FormatField_t	* ff,		// valid attrib field
    ccp			key,		// key to insert
    bool		scan_form,	// true: scan 'key' for FORM= prefix
    bool		move_key,	// true: move string, false: strdup()
    bool		* found		// not null: store found status
)
{
    if (!key)
	return 0;

    if (scan_form)
    {
	ccp eq = strchr(key,'=');
	if (eq)
	{
	    ccp slash = strchr(key,'/');
	    if ( !slash || slash > eq )
	    {
		ccp new_key = eq + 1;
		while ( *new_key > 0 && *new_key <= ' ' )
		    new_key++;
		noPRINT("INSERT: %s\n",new_key);
		FormatFieldItem_t * item = InsertFormatField(ff,new_key,false,false,found);
		SetFormatFieldParam(item,key,eq);
		if (move_key)
		    FreeString(key);
		return item;
	    }
	}
    }

    bool my_found;
    const int idx = FindFormatFieldHelper(ff,&my_found,key);
    if (found)
	*found = my_found;

    FormatFieldItem_t * item;
    if (my_found)
    {
	item = ff->list + idx;
	if (move_key)
	    FreeString(key);
    }
    else
    {
	item = InsertFormatFieldHelper(ff,idx);
	item->key = move_key ? key : STRDUP(key);
    }

    item->fform = -1;
    item->iform = -1;
    item->pform = -1;
    item->num   =  0;
    return item;
}

///////////////////////////////////////////////////////////////////////////////

FormatFieldItem_t * InsertFormatFieldFF
(
    FormatField_t	* ff,		// valid attrib field
    ccp			key,		// key to insert
    file_format_t	fform,		// file format
    bool		scan_form,	// true: scan 'key' for FORM= prefix
    bool		move_key,	// true: move string, false: strdup()
    bool		* found		// not null: store found status
)
{
    FormatFieldItem_t *item = InsertFormatField(ff,key,scan_form,move_key,found);
    DASSERT(item);
    item->fform = fform;
    return item;
}

///////////////////////////////////////////////////////////////////////////////

FormatFieldItem_t * AppendFormatField
(
    FormatField_t	* ff,		// valid attrib field
    ccp			key,		// key to insert
    bool		scan_form,	// true: scan 'key' for FORM= prefix
    bool		move_key	// true: move string, false: strdup()
)
{
    if (!key)
	return 0;

    if (scan_form)
    {
	ccp eq = strchr(key,'=');
	if (eq)
	{
	    ccp slash = strchr(key,'/');
	    if ( !slash || slash > eq )
	    {
		ccp new_key = eq + 1;
		while ( *new_key > 0 && *new_key <= ' ' )
		    new_key++;
		noPRINT("INSERT: %s\n",new_key);
		FormatFieldItem_t * item = AppendFormatField(ff,new_key,false,false);
		SetFormatFieldParam(item,key,eq);
		if (move_key)
		    FreeString(key);
		return item;
	    }
	}
    }

    DASSERT( ff->used <= ff->size );
    noPRINT(">FF: %u/%u\n",ff->used,ff->size);
    if ( ff->used == ff->size )
    {
	ff->size += FORMAT_FIELD_GROW(ff->size);
	ff->list = REALLOC(ff->list,ff->size*sizeof(*ff->list));
    }
    TRACE("AppendFormatField(%s,%d) %d/%d\n",key,move_key,ff->used,ff->size);

    FormatFieldItem_t * item = ff->list + ff->used++;
    item->key	= move_key ? key : STRDUP(key);
    item->fform	= -1;
    item->iform	= -1;
    item->pform	= -1;
    item->num   =  0;
    return item;
}

///////////////////////////////////////////////////////////////////////////////
#if 0 // [[not-needed]] not needed yet 2011-06
///////////////////////////////////////////////////////////////////////////////

enumError LoadFormatField
(
    FormatField_t	* ff,		// attrib field
    bool		init_ff,	// true: initialize 'ff' first
    bool		sort,		// true: sort 'ff'
    ccp			filename,	// filename of source file
    bool		silent		// true: don't print open/read errors
)
{
    ASSERT(ff);
    ASSERT(filename);
    ASSERT(*filename);

    TRACE("LoadFormatField(%p,%d,%d,%s,%d)\n",ff,init_att,sort,filename,silent);

    if (init_att)
	InitializeFormatField(ff);

    FILE * f = fopen(filename,"rb");
    if (!f)
    {
	if (!silent)
	    ERROR1(ERR_CANT_OPEN,"Can't open file: %s\n",filename);
	return ERR_CANT_OPEN;
    }

    while (fgets(iobuf,sizeof(iobuf)-1,f))
    {
	char * ptr = iobuf;
	while (*ptr)
	    ptr++;
	if ( ptr > iobuf && ptr[-1] == '\n' )
	{
	    ptr--;
	    if ( ptr > iobuf && ptr[-1] == '\r' )
		ptr--;
	}

	if ( ptr > iobuf )
	{
	    *ptr++ = 0;
	    const size_t len = ptr-iobuf;
	    ptr = MALLOC(len);
	    memcpy(ptr,iobuf,len);
	    if (sort)
		InsertFormatField(ff,ptr,true);
	    else
		AppendFormatField(ff,ptr,true);
	}
    }

    fclose(f);
    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////
#endif
///////////////////////////////////////////////////////////////////////////////

enumError SaveFormatField
(
    FormatField_t	* ff,		// valid attrib field
    ccp			filename,	// filename of dest file
    bool		rm_if_empty	// true: rm dest file if 'ff' is empty
)
{
    ASSERT(ff);
    ASSERT(filename);
    ASSERT(*filename);

    TRACE("SaveFormatField(%p,%s,%d)\n",ff,filename,rm_if_empty);

    if ( !ff->used && rm_if_empty )
    {
	unlink(filename);
	return ERR_OK;
    }

    FILE * f = fopen(filename,"wb");
    if (!f)
	return ERROR1(ERR_CANT_CREATE,"Can't create file: %s\n",filename);

    enumError err = WriteFormatField(f,filename,ff,0,0,0,0);
    fclose(f);
    return err;
}

///////////////////////////////////////////////////////////////////////////////

enumError WriteFormatField
(
    FILE		* f,		// open destination file
    ccp			filename,	// NULL or filename (needed on write error)
    FormatField_t	* ff,		// valid format field
    ccp			line_prefix,	// not NULL: insert prefix before each line
    ccp			key_prefix,	// not NULL: insert prefix before each key
    ccp			eol,		// end of line text (if NULL: use LF)
    uint		hex_fw		// if >0: print num as hex with field width
)
{
    if (!key_prefix)
	key_prefix = "";
    if (!eol)
	eol = "\n";

    FormatFieldItem_t *ptr = ff->list, *end;
    for ( end = ptr + ff->used; ptr < end; ptr++ )
    {
	if ( line_prefix && fprintf(f,"%s",line_prefix) < 0 )
	    goto abort;

	int n = 0;
	ccp sep = "";
	if ( ptr->fform >= 0 )
	{
	    const int stat = fprintf(f,"%s%s",sep,GetNameFF(0,ptr->fform));
	    if ( stat < 0 )
		goto abort;
	    n += stat;
	    sep = ",";
	}

	ccp iname = GetImageFormatName(ptr->iform,0);
	if (iname)
	{
	    const int stat = fprintf(f,"%s%s",sep,iname);
	    if ( stat < 0 )
		goto abort;
	    n += stat;
	    sep = ",";
	}

	ccp pname = GetPaletteFormatName(ptr->pform,0);
	if (pname)
	{
	    const int stat = fprintf(f,"%s%s",sep,pname);
	    if ( stat < 0 )
		goto abort;
	    n += stat;
	    sep = ",";
	}

	if (ptr->num)
	{
	    const int stat = hex_fw
				? fprintf(f,"%s%#*x",sep,hex_fw,ptr->num)
				: fprintf(f,"%s%u",sep,ptr->num);
	    if ( stat < 0 )
		goto abort;
	    n += stat;
	    //sep = ",";
	}

	if ( fprintf(f,"%s= %s%s%s", n<8 ? "\t\t" : "\t", key_prefix,ptr->key,eol) < 0 )
	    goto abort;
    }
    return ERR_OK;

 abort:
    return ERROR1(ERR_WRITE_FAILED,
			"Error while writing attrib field: %s\n",
			filename ? filename : "?" );
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			  string lists			///////////////
///////////////////////////////////////////////////////////////////////////////

int AtFileHelper
(
    ccp arg,
    int (*func) ( ccp arg )
)
{
    if ( !arg || !*arg || !func )
	return 0;

    TRACE("AtFileHelper(%s)\n",arg);
    if ( *arg != '@' )
	return func(arg);

    FILE * f;
    char buf[PATH_MAX];
    const bool use_stdin = arg[1] == '-' && !arg[2];

    if (use_stdin)
	f = stdin;
    else
    {
     #ifdef __CYGWIN__
	char buf[PATH_MAX];
	NormalizeFilenameCygwin(buf,sizeof(buf),arg+1);
	f = fopen(buf,"r");
     #else
	f = fopen(arg+1,"r");
     #endif
	if (!f)
	    return func(arg);
    }

    ASSERT(f);

    u32 max_stat = 0;
    while (fgets(buf,sizeof(buf)-1,f))
    {
	char * ptr = buf;
	while (*ptr)
	    ptr++;
	if ( ptr > buf && ptr[-1] == '\n' )
	    ptr--;
	if ( ptr > buf && ptr[-1] == '\r' )
	    ptr--;
	*ptr = 0;
	const u32 stat = func(buf);
	if ( max_stat < stat )
	     max_stat = stat;
    }
    fclose(f);
    return max_stat;
}

///////////////////////////////////////////////////////////////////////////////

uint n_param = 0, id6_param_found = 0;
ParamList_t * first_param = 0;
ParamList_t ** append_param = &first_param;

///////////////////////////////////////////////////////////////////////////////

static ParamList_t* GetPoolParam()
{
    static ParamList_t * pool = 0;
    static int n_pool = 0;

    if (!n_pool)
    {
	const int alloc_count = 100;
	pool = (ParamList_t*) CALLOC(alloc_count,sizeof(ParamList_t));
	n_pool = alloc_count;
    }

    n_pool--;
    return pool++;
}

///////////////////////////////////////////////////////////////////////////////

int AddParam ( ccp arg )
{
    if ( !arg || !*arg )
	return 0;

    TRACE("ARG#%02d: %s\n",n_param,arg);

    ParamList_t *param = GetPoolParam();
    param->arg = STRDUP(arg);

    while (*append_param)
	append_param = &(*append_param)->next;

    noTRACE("INS: A=%p->%p P=%p &N=%p->%p\n",
	    append_param, *append_param,
	    param, &param->next, param->next );
    *append_param = param;
    append_param = &param->next;
    noTRACE("  => A=%p->%p\n", append_param, *append_param );
    n_param++;

    return 1;
}

///////////////////////////////////////////////////////////////////////////////

void AtExpandParam ( ParamList_t ** p_param )
{
    if ( !p_param || !*p_param )
	return;

    ParamList_t *param = *p_param;
    if ( param->is_expanded || !param->arg || *param->arg != '@' )
	return;

    FILE * f;
    char buf[PATH_MAX];
    const bool use_stdin = param->arg[1] == '-' && !param->arg[2];
    if (use_stdin)
	f = stdin;
    else
    {
     #ifdef __CYGWIN__
	NormalizeFilenameCygwin(buf,sizeof(buf),param->arg+1);
	f = fopen(buf,"r");
     #else
	f = fopen(param->arg+1,"r");
     #endif
	if (!f)
	    return;
    }

    ASSERT(f);

    u32 count = 0;
    while (fgets(buf,sizeof(buf)-1,f))
    {
	char * ptr = buf;
	while (*ptr)
	    ptr++;
	if ( ptr > buf && ptr[-1] == '\n' )
	    ptr--;
	if ( ptr > buf && ptr[-1] == '\r' )
	    ptr--;
	*ptr = 0;

	if (count++)
	{
	    // insert a new item
	    ParamList_t * new_param = GetPoolParam();
	    new_param->next = param->next;
	    param->next = new_param;
	    param = new_param;
	    n_param++;
	}
	param->arg = STRDUP(buf);
	param->is_expanded = true;
    }
    fclose(f);

    if (!count)
    {
	*p_param = param->next;
	n_param--;
    }

    append_param = &first_param;
}

///////////////////////////////////////////////////////////////////////////////

void AtExpandAllParam ( ParamList_t ** p_param )
{
    if (p_param)
	for ( ; *p_param; p_param = &(*p_param)->next )
	    AtExpandParam(p_param);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			tiny support			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[TinyParam]]

const tiny_param_t TinyParam[N_TINY_MODES] =
{
    // TINY 0 == OFF
    {
	0,		// kcl_round
	0,		// kcl_mask

	999,		// pos_round
	999,		// rot_round
	999,		// scale_round

	999,		// route_round
	999,		// cannon_round
    },

    // TINY 1 == LOSSLESS
    {
	8,		// kcl_round
	0xfffffff0,	// kcl_mask

	999,		// pos_round
	999,		// rot_round
	999,		// scale_round

	999,		// route_round
	999,		// cannon_round
    },

    // TINY 2 == RECREATE
    {
	8,		// kcl_round
	0xffffffc0,	// kcl_mask

	999,		// pos_round
	999,		// rot_round
	999,		// scale_round

	999,		// route_round
	999,		// cannon_round
    },

    // TINY 3 == MINIMAL
    {
	7,		// kcl_round
	0xffffff00,	// kcl_mask

	6,		// pos_round
	3,		// rot_round
	7,		// scale_round

	0,		// route_round
	12,		// cannon_round
    },

    // TINY 4 == GOOD
    {
	6,		// kcl_round
	0xfffffc00,	// kcl_mask

	4,		// pos_round
	2,		// rot_round
	6,		// scale_round

	0,		// route_round
	11,		// cannon_round
    },

    // TINY 5 == MEDIUM
    {
	5,		// kcl_round
	0xfffff000,	// kcl_mask

	2,		// pos_round
	1,		// rot_round
	5,		// scale_round

	-1,		// route_round
	10,		// cannon_round
    },

    // TINY 6 == EXTREME
    {
	4,		// kcl_round
	0xffffc000,	// kcl_mask

	0,		// pos_round
	0,		// rot_round
	4,		// scale_round

	-2,		// route_round
	9,		// cannon_round
    },

    // TINY 7 == DANGEROUS
    {
	3,		// kcl_round
	0xffff0000,	// kcl_mask

	-2,		// pos_round
	-1,		// rot_round
	3,		// scale_round

	-3,		// route_round
	8,		// cannon_round
    },
};

///////////////////////////////////////////////////////////////////////////////

int ScanOptTiny ( ccp arg )
{
    static const KeywordTab_t tab[] =
    {
	{  0,	"TINY-0",	"OFF",		0 },
	{  1,	"TINY-1",	"LOSSLESS",	0 },
	{  2,	"TINY-2",	"RECREATE",	0 },
	{  3,	"TINY-3",	"MINIMAL",	0 },
	{  4,	"TINY-4",	"GOOD",		0 },
	{  5,	"TINY-5",	"MEDIUM",	0 },
	{  6,	"TINY-6",	"EXTREME",	0 },
	{  7,	"TINY-7",	"DANGEROUS",	0 },
	{ 0,0,0,0 }
    };

    char *end;
    const ulong num = str2ul(arg,&end,10);
    if ( end > arg && !*end && num < N_TINY_MODES )
	opt_tiny = num;
    else
    {
	const KeywordTab_t * cmd = ScanKeyword(0,arg,tab);
	if (!cmd)
	{
	    ERROR0(ERR_SYNTAX,"Invalid --tiny mode: '%s'\n",arg);
	    return 1;
	}

	opt_tiny = cmd->opt;
    }

    if ( opt_tiny && !opt_compr_mode )
    {
	opt_compr_mode	= 11;
	opt_compr	= 23 + opt_tiny;
    }

    return 0;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			general list support		///////////////
///////////////////////////////////////////////////////////////////////////////

void InitializeList ( List_t * list, uint elem_size )
{
    DASSERT(list);
    DASSERT(elem_size>0);
    memset(list,0,sizeof(*list));
    list->elem_size = elem_size;
}

///////////////////////////////////////////////////////////////////////////////

void ResetList ( List_t * list )
{
    DASSERT(list);
    FREE(list->list);
    const uint elem_size = list->elem_size;
    memset(list,0,sizeof(*list));
    list->elem_size = elem_size;
}

///////////////////////////////////////////////////////////////////////////////

void * InsertList ( List_t * list, int index )
{
    DASSERT( list );
    DASSERT( list->elem_size > 0 );
    DASSERT( list->used <= list->size );
    DASSERT( !list->list == !list->size );

    if ( list->used == list->size )
    {
	list->size = 3*list->size/2 + 100;
	list->list = REALLOC( list->list, list->size * list->elem_size ) ;
    }

    DASSERT( list->list );
    DASSERT( list->used < list->size );

    if ( index < 0 )
    {
	index += list->used++;
	if ( index < 0 )
	    index = 0;
    }
    else if ( index > list->used )
	index = list->used;
    DASSERT( index >= 0 && index <= list->used );
    u8 * ptr = list->list + index * list->elem_size;
    if ( index < list->used )
	memmove( ptr+list->elem_size, ptr, list->elem_size * (list->used-index) );
    memset(ptr,0,list->elem_size);

    list->used++;
    DASSERT( list->used <= list->size );

    return ptr;
}

///////////////////////////////////////////////////////////////////////////////

void * AppendList ( List_t * list )
{
    DASSERT( list );
    DASSERT( list->elem_size > 0 );
    DASSERT( list->used <= list->size );
    DASSERT( !list->list == !list->size );

    if ( list->used == list->size )
    {
	list->size = 3*list->size/2 + 100;
	list->list = REALLOC( list->list, list->size * list->elem_size ) ;
    }

    DASSERT( list->list );
    DASSERT( list->used < list->size );
    u8 *ptr = list->list + list->used++ * list->elem_size;
    memset(ptr,0,list->elem_size);
    return ptr;
}

///////////////////////////////////////////////////////////////////////////////

void * GrowList ( List_t * list, uint n )
{
    DASSERT( list );
    DASSERT( list->elem_size > 0 );
    DASSERT( list->used <= list->size );
    DASSERT( !list->list == !list->size );

    const uint new_used = list->used + n;
    if ( new_used > list->size || !list->size )
    {
	list->size = new_used > 10 ? new_used : 10;
	list->list = REALLOC( list->list, list->size * list->elem_size ) ;
    }

    DASSERT( list->list );
    DASSERT( new_used <= list->size );

    void *result = list->list + list->used * list->elem_size;
    list->used = new_used;
    return result;
}

///////////////////////////////////////////////////////////////////////////////

void * GrowListSize ( List_t * list, uint n, uint add_size )
{
    DASSERT( list );
    DASSERT( list->elem_size > 0 );
    DASSERT( list->used <= list->size );
    DASSERT( !list->list == !list->size );

    const uint new_used = list->used + n;
    if ( new_used > list->size || !list->size )
    {
	noPRINT("GROW-LIST: %u -> %u (size=%u)\n",
		list->size, new_used + add_size, (new_used + add_size)*list->elem_size );
	list->size = new_used + add_size;
	list->list = REALLOC( list->list, list->size * list->elem_size ) ;
    }

    DASSERT( list->list );
    DASSERT( new_used <= list->size );

    void *result = list->list + list->used * list->elem_size;
    list->used = new_used;
    return result;
}

///////////////////////////////////////////////////////////////////////////////

void * GetListElem
	( const List_t * list, int index, const void * return_not_found )
{
    DASSERT( list );
    DASSERT( list->elem_size > 0 );
    DASSERT( list->used <= list->size );
    DASSERT( !list->list == !list->size );

    if ( index < 0 )		// allow acces to elements relative to end
	index += list->used;

    return (uint)index < list->used
	? list->list + index * list->elem_size
	: (void*)return_not_found;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void InitializeMIL ( List_t * list )
{
    DASSERT(list);
    InitializeList(list,sizeof(MemItem_t));
}

///////////////////////////////////////////////////////////////////////////////

void ResetMIL ( List_t * list )
{
    DASSERT(list);

    MemItem_t *eptr = GetMemListElem(list,0,0);
    MemItem_t *eend = eptr + list->used;
    for ( ; eptr < eend; eptr++ )
    {
	if (eptr->name_alloced)
	    FreeString(eptr->name);
	if (eptr->data_alloced)
	    FREE(eptr->data);
	FREE(eptr->str_idx);
    }
    ResetList(list);
}

///////////////////////////////////////////////////////////////////////////////

static int sort_mem_list_by_idx ( const MemItem_t * a, const MemItem_t * b )
{
    int stat = a->idx1 - b->idx1;
    if (!stat)
    {
	stat = a->idx2 - b->idx2;
	if (!stat)
	    stat = a->sort_idx - b->sort_idx;
    }
    return stat;
}

//-----------------------------------------------------------------------------

static int sort_mem_list_by_sort ( const MemItem_t * a, const MemItem_t * b )
{
    int stat = a->sort_idx - b->sort_idx;
    if (!stat)
    {
	stat = a->idx1 - b->idx1;
	if (!stat)
	    stat = a->idx2 - b->idx2;
    }
    return stat;
}

//-----------------------------------------------------------------------------

void SortMIL ( List_t * list, bool use_sort_idx )
{
    DASSERT(list);
    if ( list->used > 1 )
    {
	noPRINT("qsort(%p,%d,%zd)\n", list->list, list->used, sizeof(MemItem_t) );
	qsort( list->list, list->used, sizeof(MemItem_t),
		use_sort_idx
			? (qsort_func)sort_mem_list_by_sort
			: (qsort_func)sort_mem_list_by_idx );
    }
}

///////////////////////////////////////////////////////////////////////////////

MemItem_t * AppendMIL ( List_t * list )
{
    MemItem_t * item = AppendList(list); // data is zeroed!
    item->sort_idx = 10*list->used;
    item->str_idx = 0;
    item->str_idx_size = 0;
    return item;
}

///////////////////////////////////////////////////////////////////////////////

MemItem_t * GrowMIL ( List_t * list, uint n )
{
    return GrowList(list,n);
}

///////////////////////////////////////////////////////////////////////////////

MemItem_t * GetMemListElem
    ( const List_t * list, int index, const MemItem_t * return_not_found )
{
    return GetListElem(list,index,return_not_found);
}

///////////////////////////////////////////////////////////////////////////////

u32 * GetStrIdxMIL ( MemItem_t * mi, uint n_need )
{
    DASSERT(mi);
    const uint BUF_MAX = sizeof(mi->str_idx_buf)/sizeof(*mi->str_idx_buf);

    if ( n_need <= BUF_MAX || n_need <= mi->str_idx_size )
	return mi->str_idx ? mi->str_idx : mi->str_idx_buf;

    typeof(mi->str_idx) new_list = CALLOC(n_need,sizeof(*new_list));
    if ( mi->str_idx )
    {
	memcpy(new_list,mi->str_idx,mi->str_idx_size*sizeof(*new_list));
	FREE(mi->str_idx);
    }
    else
	memcpy(new_list,mi->str_idx_buf,BUF_MAX*sizeof(*new_list));

    mi->str_idx = new_list;
    mi->str_idx_size = n_need;
    return mi->str_idx;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			modes and options		///////////////
///////////////////////////////////////////////////////////////////////////////

int ScanOptCompr ( ccp arg )
{
    static const KeywordTab_t alias[] =
    {
	{  0,	"LIST",		0,		-9 },
	{  1,	"C-LIST",	"CLIST",	-9 },

	{  0,	"UNCOMPRESSED",	0,		-1 },

	{  0,	"NOCHUNKS",	0,		1 },
	{  1,	"FAST",		0,		1 },
	{  9,	"BEST",		0,		1 },
	{ 10,	"ULTRA",	0,		1 },

	{  9,	"T2",		"TRY2",		2 },
	{  9,	"T3",		"TRY3",		3 },
	{  9,	"T4",		"TRY4",		4 },
	{  9,	"T5",		"TRY5",		5 },

	{ 0,0,0,0 }
    };

    char *end;
    const long num = str2l(arg,&end,10);
    if ( end > arg && !*end )
    {
	struct tab_t { s16 from, to, add, mode; };
	static const struct tab_t tab[] =
	{
	    {  -1,  -1,   10, -1 },
	    {   0,  10,    0,  1 },
	    {  91,  93,    0,  9 },
	    { 100, 150, -100, 11 },
	    { -9,0,0,0}
	};

	static const struct tab_t *ptr;
	for ( ptr = tab; ptr->from != -9; ptr++ )
	    if ( num >= ptr->from && num <= ptr->to )
	    {
		opt_compr	= num + ptr->add;
		opt_compr_mode	= ptr->mode;
		return 0;
	    }
    }

    const KeywordTab_t * cmd = ScanKeyword(0,arg,alias);
    if (!cmd)
    {
	ERROR0(ERR_SYNTAX,"Invalid compression level (option --compr): '%s'\n",arg);
	return 1;
    }

    opt_compr_mode = cmd->opt;
    if ( opt_compr_mode != -1 )
	opt_compr = cmd->id;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptPtDir ( ccp arg )
{
    if ( !arg || !*arg )
    {
	opt_pt_dir = 1;
	return 0;
    }

    static const KeywordTab_t tab[] =
    {
	{ -1,	"REMOVE",	"0",	0 },
	{  0,	"AUTO",		0,	0 },
	{  1,	"FORCE",	"1",	0 },

	{ 0,0,0,0 }
    };

    const KeywordTab_t * cmd = ScanKeyword(0,arg,tab);
    if (cmd)
    {
	opt_pt_dir = cmd->id;
	return 0;
    }

    ERROR0(ERR_SYNTAX,"Invalid `point directory' mode (option --pdir): '%s'\n",arg);
    return 1;
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptRecurse ( ccp arg )
{
    if ( !arg || !*arg )
    {
	opt_recurse = INT_MAX;
	return 0;
    }

    static const KeywordTab_t tab[] =
    {
	{  0,		"NONE",		0,	0 },
	{  INT_MAX,	"UNLIMITED",	"ALL",	0 },

	{ 0,0,0,0 }
    };

    const KeywordTab_t * cmd = ScanKeyword(0,arg,tab);
    if (cmd)
    {
	opt_recurse = cmd->id;
	return 0;
    }

    char * end;
    ulong num = strtoul(arg,&end,10);
    if ( end > arg && !*end && num <= INT_MAX )
    {
	opt_recurse = num;
	return 0;
    }

    ERROR0(ERR_SYNTAX,"Invalid recurse level (option --recurse): '%s'\n",arg);
    return 1;
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptCase ( ccp arg )
{
    const int stat = ScanKeywordLowerAutoUpper(arg,LOUP_AUTO,0,"Option --case");
    if ( stat == LOUP_ERROR )
	return 1;

    opt_case = stat;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptFModes ( ccp arg )
{
    opt_fmodes_include = 0;
    opt_fmodes_exclude = 0;

    if (!arg)
	return 0;

    const int NO   = 0x1000;
    const int BOTH = 0x1001;
    static const KeywordTab_t keytab[] =
    {
	{ FZM_SECTION,		"SECTION",	0,	BOTH*FZM_SECTION },
	{ FZM_VISUAL,		"VISUAL",	0,	BOTH*FZM_VISUAL },
	{ FZM_GAMEPLAY,		"GAMEPLAY",	0,	BOTH*FZM_GAMEPLAY },
	{ FZM_BATTLE,		"BATTLE",	0,	BOTH*FZM_BATTLE },
	{ FZM_RACING,		"RACING",	0,	BOTH*FZM_RACING },
	{ FZM_TIMETRIAL,	"TIMETRIAL",	0,	BOTH*FZM_TIMETRIAL },
	{ FZM_OFFLINE,		"OFFLINE",    "LOCAL",	BOTH*FZM_OFFLINE },
	{ FZM_ONLINE,		"ONLINE",	0,	BOTH*FZM_ONLINE },

	{ NO*FZM_SECTION,	"-SECTION",	0,	BOTH*FZM_SECTION },
	{ NO*FZM_VISUAL,	"-VISUAL",	0,	BOTH*FZM_VISUAL },
	{ NO*FZM_GAMEPLAY,	"-GAMEPLAY",	0,	BOTH*FZM_GAMEPLAY },
	{ NO*FZM_BATTLE,	"-BATTLE",	0,	BOTH*FZM_BATTLE },
	{ NO*FZM_RACING,	"-RACING",	0,	BOTH*FZM_RACING },
	{ NO*FZM_TIMETRIAL,	"-TIMETRIAL",	0,	BOTH*FZM_TIMETRIAL },
	{ NO*FZM_OFFLINE,	"-OFFLINE",   "-LOCAL",	BOTH*FZM_OFFLINE },
	{ NO*FZM_ONLINE,	"-ONLINE",	0,	BOTH*FZM_ONLINE },

	{0,0,0,0}
    };

    const s64 stat = ScanKeywordList(arg,keytab,0,true,0,0,
					"Option --filter",ERR_SYNTAX);
    if ( stat == -1 )
	return 1;

    opt_fmodes_include = stat & FZM__MASK;
    opt_fmodes_exclude = stat/NO & FZM__MASK;

    PRINT("--filter: %02x %02x\n",opt_fmodes_include,opt_fmodes_exclude);

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

SlotMode_t opt_slot = SLOTMD_NONE;

static const KeywordTab_t opt_slot_tab[] =
{
	{  SLOTMD_NONE,		"NONE",		"-",		0 },

	{  SLOTMD_M_DAISY,	"DAISY",	"DESERT",	0 },
	 {  SLOTMD_M_DAISY,	"3.1",		"31",		0 },
	 {  SLOTMD_M_DAISY,	"7.1",		"71",		0 },

	{  SLOTMD_M_SHERBET,	"SHERBET",	"T61",		0 },
	 {  SLOTMD_M_SHERBET,	"6.1",		"61",		0 },

	{  SLOTMD_M_SHYGUY,	"SHYGUY",	"T62",		0 },
	 {  SLOTMD_M_SHYGUY,	"6.2",		"62",		0 },

	{  SLOTMD_M_STANDARD,	"STANDARD",	"STD",		0 },

	{  SLOTMD_M_MOST,	"MOST",		"ALL",		0 },

	{  SLOTMD_RM_ICE,	"RM-ICE",	"RMICE",	1 },
	{  SLOTMD_RM_SUNDS,	"RM-SUNDS",	"RMSUNDS",	1 },
	{  SLOTMD_RM_SHYGUY,	"RM-SHYGUY",	"RMSHYGUY",	1 },
	{  SLOTMD_ADD_GICE,	"ADD-GICE",	"ADDGICE",	1 },
	{  SLOTMD_ADD_BICE,	"ADD-BICE",	"ADDBICE",	1 },
	{  SLOTMD_ADD_SHYGUY,	"ADD-SHYGUY",	"ADDSHYGUY",	1 },
	{  SLOTMD_ICE_TO_WATER,	"ICE2WATER",	"ICETOWATER",	1 },
	{  SLOTMD_WATER_TO_ICE,	"WATER2ICE",	"WATERTOICE",	1 },

	//{ -1,			"4.2",		"42",		0 },

	{ 0,0,0,0 }
};

///////////////////////////////////////////////////////////////////////////////

int ScanOptSlot ( ccp arg )
{
    if ( !arg || !*arg )
	return 0;

    const KeywordTab_t *cmd = ScanKeyword(0,arg,opt_slot_tab);
    if (!cmd)
    {
	const KeywordTab_t *c2 = ScanKeyword(0,arg,track_name_tab);
	if ( c2 && c2->id != 10 ) // 10 == moonview highway
	{
	    char buf[10];
	    snprintf(buf,sizeof(buf),"%llu",c2->id);
	    cmd = ScanKeyword(0,buf,opt_slot_tab);
	    if (!cmd)
		cmd = ScanKeyword(0,"STANDARD",opt_slot_tab);
	}
    }

    if ( cmd && cmd->id >= 0 )
    {
	if ( opt_slot & SLOTMD_JOB_RM_SZS )
	    need_norm--;
	if ( opt_slot & SLOTMD_JOB_ADD_SZS )
	    have_patch_count--;
	if ( opt_slot & SLOTMD_JOB_KMP )
	   have_patch_count--, have_kmp_patch_count--;
	if ( opt_slot & SLOTMD_JOB_KCL )
	   have_patch_count--, have_kcl_patch_count--;

	opt_slot = cmd->id;

	if ( opt_slot & SLOTMD_JOB_RM_SZS )
	    need_norm++;
	if ( opt_slot & SLOTMD_JOB_ADD_SZS )
	    have_patch_count++;
	if ( opt_slot & SLOTMD_JOB_KMP )
	   have_patch_count++, have_kmp_patch_count++;
	if ( opt_slot & SLOTMD_JOB_KCL )
	   have_patch_count++, have_kcl_patch_count++;

	return 0;
    }

    ERROR0(ERR_SYNTAX,"Invalid keyword for option --slot: '%s'\n",arg);
    return 1;
}

///////////////////////////////////////////////////////////////////////////////

ccp PrintSlotMode ( SlotMode_t mode )
{
    const KeywordTab_t *cmd;
    for ( cmd = opt_slot_tab; cmd->name1; cmd++ )
	if ( mode == (u32)cmd->id )
	    return cmd->name1;
    return "?";
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

ccp NormalizeTrackName ( char *buf, uint bufsize, ccp arg )
{
    DASSERT(buf );
    DASSERT(bufsize>10);
    DASSERT(arg);

    // skip leading separators
    while ( *arg > 1 && *arg <= ' ' || *arg == ',' || *arg == ':' || *arg == '=' )
	arg++;

    char *dest = buf, *dest_end = buf + bufsize - 2;
    while ( dest < dest_end && *arg && *arg != ',' && *arg != ':' && *arg != '=' )
    {
	int code = ScanUTF8AnsiChar(&arg);
	if (isalnum(code))
	    *dest++ = toupper(code);
	else switch(code)
	{
	    case '.':
		*dest++ = '.';
		break;

	    case 0xe4: // ä
	    case 0xc4: // Ä
		*dest++ = 'A';
		*dest++ = 'E';
		break;

	    case 0xf6: // ö
	    case 0xd6: // Ö
		*dest++ = 'O';
		*dest++ = 'E';
		break;

	    case 0xfc: // ü
	    case 0xdc: // Ü
		*dest++ = 'U';
		*dest++ = 'E';
		break;

	    case 0xdf: // ß
		*dest++ = 'S';
		*dest++ = 'S';
		break;
	}
    }

    *dest = 0;
    return arg;
}

///////////////////////////////////////////////////////////////////////////////

static int scan_track_arena
(
    ccp			arg,
    const u32		* def_tab,
    u32			* tab,
    u32			* r_tab,
    const TrackInfo_t	* info,
    uint		n,
    ccp			opt_name,
    int			(*scan_func) ( ccp name )
)
{
    DASSERT(def_tab);
    DASSERT(tab);
    DASSERT(r_tab);
    DASSERT(info);
    DASSERT(scan_func);
    DASSERT( n > 0 );

    if (!arg)
	return 0;

    memcpy(tab,def_tab,n*sizeof(*tab));

    uint track_index = 0;
    bool swap_tracks = true;

    for(;;)
    {
	char name[100];
	arg = NormalizeTrackName(name,sizeof(name),arg);
	if (!*name)
	    break;

	if (!strcmp(name,"0"))
	{
	    uint i;
	    for ( i = 0; i < n; i++ )
		tab[i] = i;
	    continue;
	}

	int index = scan_func(name);
	if ( index < 0 )
	{
	    ERROR0(ERR_SYNTAX,"Option --%ss: Not a %s name: %s\n",
			opt_name, opt_name, name);
	    return 1;
	}

	if ( *arg == ':' || *arg == '=' )
	{
	    swap_tracks = *arg == '=';
	    PRINT("INDEX --%ss: %2u -> %2u\n",
			opt_name, track_index, info[index].def_index );
	    track_index = info[index].def_index;

	    if ( *arg == ':' ) arg++;
	    if ( *arg == '=' ) arg++;
	    continue;
	}

	if ( track_index >= n )
	{
	    ERROR0(ERR_SEMANTIC,"Option --%ss: Invald track index: %u (max=%u)\n",
			opt_name, track_index, n-1 );
	    return 1;
	}

	u32 * tptr = tab;
	if (swap_tracks)
	    while ( tptr < tab+n && *tptr != index )
		tptr++;

	if ( swap_tracks && tptr < tab+n )
	{
	    PRINT("SWAP --%s:  %2u=%2u, %2zu=%2u\n",
		opt_name, track_index, *tptr,
		tptr-tab, tab[track_index] );
	    *tptr = tab[track_index];
	}
	tab[track_index] = index;
	track_index++;
    }

    uint i;
    for ( i = 0; i < n; i++ )
	r_tab[tab[i]] = i;
    //HEXDUMP16(0,0,r_tab,n*sizeof(*tab));

    return 0;
}

//-----------------------------------------------------------------------------

int ScanOptTracks ( ccp arg )
{
    opt_tracks = true;
    return scan_track_arena ( arg, track_pos_default, track_pos, track_pos_r,
			track_info, MKW_N_TRACKS, "track", ScanTrack );
}

//-----------------------------------------------------------------------------

int ScanOptArenas ( ccp arg )
{
    opt_arenas = true;
    return scan_track_arena ( arg, arena_pos_default, arena_pos, arena_pos_r,
			arena_info, MKW_N_ARENAS, "arena", ScanArena );
}

///////////////////////////////////////////////////////////////////////////////

void DumpTrackList
(
    FILE	* f,		// NULL(=stdout) or file
    const u32	* tab,		// NULL or list with track indices
    int		brief_mode	// 0: full table
				// 1: suppress header
				// 2: file names only
)
{
    if (!f)
	f = stdout;
    if (!tab)
	tab = track_pos;

    if ( brief_mode > 1 )
    {
	uint i;
	for ( i = 0; i < MKW_N_TRACKS; i++ )
	{
	    const uint idx = *tab++;
	    const TrackInfo_t * info = idx < MKW_N_TRACKS ? track_info + idx : &InvalidTrack;
	    fprintf(f,"%s\n",info->track_fname);
	}
	return;
    }

    if (brief_mode<1)
    {
	if (long_count)
	    fprintf(f,
		"\n"
		"      new  std  file name            sound standard      fast sound  track track\n"
		" idx slot slot  of track             slot  sound file name   f.name  abbr. name\n"
		);
	else
	    fprintf(f,
		"\n"
		"      new  std  file name            standard      fast sound  track\n"
		" idx slot slot  of track             sound file name   f.name  name\n"
		);
    }

    const uint width = long_count ? 99 : 87;

    uint i;
    for ( i = 0; i < MKW_N_TRACKS; i++ )
    {
	if ( brief_mode<1 &&!(i%4) )
	    fprintf(f,"%.*s\n",width,Minus300);

	const uint idx = *tab++;
	const TrackInfo_t * info = idx < MKW_N_TRACKS ? track_info + idx : &InvalidTrack;
	fprintf(f,"%3u.  %u.%u  %u.%u  %-*s  ",
		idx,
		i/4+1, i%4+1,
		info->def_slot/10, info->def_slot%10,
		FW_TRACK_FNAME, info->track_fname );

	if (long_count)
	    fprintf(f,"0x%02x  ",info->music_id);

	fprintf(f,"%-*s %s  ",
		FW_TRACK_SOUND_N, info->sound_n_fname,
		info->sound_f_fname + strlen(info->sound_f_fname) - 2 );

	if (long_count)
	    fprintf(f,"%-5s ",info->abbrev);

	fprintf(f,"%s\n", use_de ? info->name_de : info->name_en );
    }
    if (brief_mode<1)
	fprintf(f,"%.*s\n\n",width,Minus300);
}

///////////////////////////////////////////////////////////////////////////////

void DumpArenaList
(
    FILE	* f,		// NULL(=stdout) or file
    const u32	* tab,		// NULL or list with arena indices
    int		brief_mode	// 0: full table
				// 1: suppress header
				// 2: file names only
)
{
    if (!f)
	f = stdout;
    if (!tab)
	tab = arena_pos;

    if ( brief_mode > 1 )
    {
	uint i;
	for ( i = 0; i < MKW_N_ARENAS; i++ )
	{
	    const uint idx = *tab++;
	    const TrackInfo_t * info = idx < MKW_N_ARENAS ? arena_info + idx : &InvalidTrack;
	    fprintf(f,"%s\n",info->track_fname);
	}
	return;
    }

    if (brief_mode<1)
    {
	if (long_count)
	    fprintf(f,
		"\n"
		"      new  std  file name          sound standard    + fast  arena arena\n"
		" idx slot slot  of arena           slot  sound file name     abbr. name\n"
		);
	else
	    fprintf(f,
		"\n"
		"      new  std  file name          standard    + fast  arena\n"
		" idx slot slot  of arena           sound file name     name\n"
		);
    }

    const uint width = long_count ? 88 : 76;

    uint i;
    for ( i = 0; i < MKW_N_ARENAS; i++ )
    {
	if ( brief_mode<1 &&!(i%5) )
	    fprintf(f,"%.*s\n",width,Minus300);

	const uint idx = *tab++;
	const TrackInfo_t * info = idx < MKW_N_ARENAS ? arena_info + idx : &InvalidTrack;
	fprintf(f,"%3u.  %u.%u  %u.%u  %-*s  ",
		idx,
		i/5+1, i%5+1,
		info->def_slot/10, info->def_slot%10,
		FW_ARENA_FNAME, info->track_fname );

	if (long_count)
	    fprintf(f,"0x%02x  ",info->music_id);

	fprintf(f,"%-*s %s  ",
		FW_ARENA_SOUND_N, info->sound_n_fname,
		info->sound_f_fname + strlen(info->sound_f_fname) - 2 );

	if (long_count)
	    fprintf(f,"%-5s ",info->abbrev);

	fprintf(f,"%s\n", use_de ? info->name_de : info->name_en );
    }
    if (brief_mode<1)
	fprintf(f,"%.*s\n\n",width,Minus300);
}

///////////////////////////////////////////////////////////////////////////////

void DumpTrackFileList
(
    FILE	* f,		// NULL(=stdout) or file
    const u32	* track_tab,	// NULL or list with track indices
    const u32	* arena_tab	// NULL or list with arena indices
)
{
    if (!f)
	f = stdout;
    if (!track_tab)
	track_tab = track_pos;
    if (!arena_tab)
	arena_tab = arena_pos;

    uint i;
    for ( i = 0; i < MKW_N_TRACKS; i++ )
    {
	const uint idx = *track_tab++;
	const TrackInfo_t * info = idx < MKW_N_TRACKS ? track_info + idx : &InvalidTrack;

	fprintf(f,"T%u.%u %s %s %s\n",
		i/4+1, i%4+1,
		info->track_fname, info->sound_n_fname, info->sound_f_fname );
    }

    for ( i = 0; i < MKW_N_ARENAS; i++ )
    {
	const uint idx = *arena_tab++;
	const TrackInfo_t * info = idx < MKW_N_ARENAS ? arena_info + idx : &InvalidTrack;

	fprintf(f,"A%u.%u %s %s %s\n",
		i/5+1, i%5+1,
		info->track_fname, info->sound_n_fname, info->sound_f_fname );
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			sort mode			///////////////
///////////////////////////////////////////////////////////////////////////////

SortMode_t GetSortMode
(
    SortMode_t		sort_mode,	// wanted sort mode
    file_format_t	fform,		// use it if 'sort_mode==SORT_AUTO'
    SortMode_t		default_mode	// default if all other fails
)
{
    switch (sort_mode)
    {
	case SORT_NONE:
	case SORT_INAME:
	case SORT_OFFSET:
	case SORT_SIZE:
	case SORT_U8:
	case SORT_PACK:
	case SORT_BRRES:
	case SORT_BREFF:
	    return sort_mode;

	case SORT_AUTO:
	    switch(fform)
	    {
		case FF_U8:
		case FF_WU8:
		case FF_RARC:
		    return SORT_U8;

		case FF_PACK:
		    return SORT_PACK;

		case FF_BRRES:
		    return SORT_BRRES;

		case FF_BREFF:
		case FF_BREFT:
		    return SORT_BREFF;

		default:
		    return default_mode;
	    }
    }

    return SORT_INAME;
}

///////////////////////////////////////////////////////////////////////////////

static const KeywordTab_t opt_sort_tab[] =
{
	{  SORT_NONE,		"NONE",		"0",		0 },
	{  SORT_INAME,		"NAME",		"N",		0 },
	{  SORT_INAME,		"INAME",	0,		0 },
	{  SORT_OFFSET,		"OFFSET",	0,		0 },
	{  SORT_SIZE,		"SIZE",		"S",		0 },

	{  SORT_U8,		"U8",		0,		0 },
	{  SORT_PACK,		"PACK",		0,		0 },
	{  SORT_BRRES,		"BRRES",	"BRES",		0 },
	{  SORT_BREFF,		"BREFF",	"BREFT",	0 },

	{  SORT_AUTO,		"AUTO",		0,		0 },

	{ 0,0,0,0 }
};

//-----------------------------------------------------------------------------

ccp GetSortName ( int id, ccp res_not_found )
{
    return GetKewordNameById(opt_sort_tab,id,res_not_found);
}

//-----------------------------------------------------------------------------

int ScanOptSort ( ccp arg )
{
    const KeywordTab_t * cmd = ScanKeyword(0,arg,opt_sort_tab);
    if (cmd)
    {
	opt_sort = cmd->id;
	return 0;
    }

    ERROR0(ERR_SYNTAX,"Invalid sort mode (option --sort): '%s'\n",arg);
    return 1;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command version/section		///////////////
///////////////////////////////////////////////////////////////////////////////

void cmd_version_section
	( bool sect_header, ccp name_short, ccp name_long, int verbose )
{
    if (sect_header)
	fputs("[version]\n",stdout);

    const u32 base = 0x04030201;
    const u8 * e = (u8*)&base;
    const u32 endian = be32(e);

    printf(
	"prog=%s\n"
	"name=%s\n"
	"version=" VERSION "\n"
	"beta=%d\n"
	"revision=" REVISION  "\n"
	"system=" SYSTEM2 "\n"
	"endian=%u%u%u%u %s\n"
	"author=" AUTHOR "\n"
	"date=" DATE "\n"
	"url=" URI_HOME "%s\n"
	, name_short
	, name_long
	, BETA_VERSION
	, e[0], e[1], e[2], e[3]
	, endian == 0x01020304 ? "little"
	    : endian == 0x04030201 ? "big" : "mixed"
	, name_short
	);

    if ( verbose > 0 )
    printf(
	"\n"
 #ifdef _POSIX_C_SOURCE
	"posix_c_source=%s\n"
 #endif
	"have_clock_gettime=%d\n"
	"have_stattime_nsec=%d\n"
 #ifdef _POSIX_C_SOURCE
	,CONVERT_TO_STRING(_POSIX_C_SOURCE)
 #endif
	, HAVE_CLOCK_GETTIME
	, HAVE_STATTIME_NSEC
	);

    putchar('\n');
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		    command config & support		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError cmd_config()
{
    if (n_param)
	return ERROR0(ERR_SYNTAX,"CONFIG: Parameters not allowed!");

    uint i;
    search_file_list_t sfl;
    SearchConfigHelper(&sfl,0);

    PrintScript_t ps;
    SetupPrintScriptByOptions(&ps);
    const bool print_info = ps.fform == PSFF_UNKNOWN;

    if ( print_info && !brief_count )
    {
	static char comment[] = "(*:relevant, +:found, -:ignored)";
	if ( sfl.used == 1 )
	    printf("\nOne searched configuration directory or file %s:\n",comment);
	else
	    printf("\nList of %u searched configuration directories and files %s:\n",
			sfl.used, comment );
    }

    search_file_t *found = 0, *sf = sfl.list;
    for ( i = 0; i < sfl.used; i++, sf++ )
    {
	char ch;
	if ( sf->itype < INTY_REG )
	    ch = '-';
	else if ( !found && ( !opt_config || sf->hint & CONF_HINT_OPT ))
	{
	    found = sf;
	    ch = '*';
	}
	else
	    ch = '+';

	if ( print_info && !brief_count )
	    printf("  %c %s\n",ch,sf->fname);
    }

    if (print_info)
    {
	if (found)
	    printf("\nRelevant configuration file: %s\n",found->fname);
	else
	    fputs("\nNo valid configuration file found!\n",stdout);
    }

    config_t config;
    InitializeConfig(&config);
    if (found)
	ScanConfig(&config,found->fname,!opt_config);
 #ifdef TEST
    else if (opt_config)
	ScanConfig(&config,opt_config,false);
 #endif
    else
	ScanConfig(&config,0,true);

    if (print_sections)
	PrintConfigFile(stdout,&config);
    else if (print_info)
	PrintConfig( stdout, &config, verbose>0 || opt_install );
    else
	PrintConfigScript(stdout,&config);

    ResetConfig(&config);
    ResetSearchFile(&sfl);

    if (print_info)
    {
	if ( long_count > 0 )
	{
	    const StringField_t *sf = GetSearchList();
	    fputs("\nSearch list:\n",stdout);
	    ccp *str = sf->field;
	    for ( i = 0; i < sf->used; i++, str++ )
		printf("  %s\n",*str);

	    if ( long_count > 1 )
	    {
		const StringField_t *sf = GetAutoaddList();
		fputs("\nAuto-add search list:\n",stdout);
		ccp *str = sf->field;
		for ( i = 0; i < sf->used; i++, str++ )
		    printf("  %s\n",*str);
	    }
	}
	putchar('\n');
    }

    ResetPrintScript(&ps);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		    command config & support		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError cmd_install()
{
    printf("\nInstall files to the share directory: %s\n",share_path);
    if ( !share_path || !*share_path )
	return ERR_ERROR; // should never happen

    char destbuf[PATH_MAX];
    enumError max_err = ERR_OK;

    for ( ParamList_t *param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);
	mode_t mode;
	const inode_type_t itype =  GetInodeTypeByPath(param->arg,&mode);
	if ( itype == INTY_NOTFOUND )
	    max_err = ERROR0(ERR_WARNING,"File not found: %s\n",param->arg);
	else
	{
	    ccp dest = PathCatPP(destbuf,sizeof(destbuf),share_path,param->arg);
	    printf("> Install %s\n",dest);
	    enumError err = CopyFileTemp(param->arg,dest,0);
	    if (err)
	    {
		if ( max_err < err )
		    max_err = err;
		break;
	    }
	}
    }
    putchar('\n');
    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command argtest			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError cmd_argtest ( int argc, char ** argv )
{
    printf("ARGUMENT TEST: %u arguments:\n",argc);

    for ( int idx = 0; idx < argc; idx++ )
	printf("%4u: |%s|\n",idx,argv[idx]);
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command expand			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError path_found ( mem_t path, uint st_mode, void * param )
{
    printf(" > %s%s\n", path.ptr, S_ISDIR(st_mode) ? "/" : "" );
    return ERR_OK;
}

//-----------------------------------------------------------------------------

enumError cmd_expand ( int argc, char ** argv )
{
    SetupPager();
    printf("%sExpand %d Argument%s:%s\n",
	colout->caption,
	n_param, n_param == 1 ? "" : "s",
	colout->reset );

    bool allow_hidden = false;
    for ( ParamList_t *param = first_param; param; param = param->next )
    {
	if (!strcmp(param->arg,"/h"))
	{
	    allow_hidden = false;
	    printf("%sIgnore hidden files.%s\n", colout->info, colout->reset );
	}
	else if (!strcmp(param->arg,"+h"))
	{
	    allow_hidden = true;
	    printf("%sSearch hidden files too.%s\n", colout->info, colout->reset );
	}
	else
	{
	    NORMALIZE_FILENAME_PARAM(param);
	    printf("%s%s%s\n", colout->hint, param->arg, colout->reset );
	    SearchPaths(param->arg,0,allow_hidden,path_found,0);
	}
    }
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command error			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError cmd_error()
{
    if (!n_param)
    {
	if ( print_sections )
	{
	    int i;
	    for ( i=0; i<ERR__N; i++ )
		printf("\n[error-%02u]\ncode=%u\nname=%s\ntext=%s\n",
			i, i, GetErrorName(i,0), GetErrorText(i,0));
	}
	else
	{
	    if (print_header)
	    {
		if (print_title_func)
		    print_title_func(stdout);
		printf("\nList of error codes\n\n");
	    }

	    // calc max_wd
	    int i, max_wd = 0;
	    for ( i = 0; i < ERR__N; i++ )
	    {
		const int len = strlen(GetErrorName(i,""));
		if ( max_wd < len )
		    max_wd = len;
	    }

	    // print table
	    for ( i = 0; i < ERR__N; i++ )
	    {
		ccp ename = GetErrorName(i,"");
		ccp etext = GetErrorText(i,"");
		if ( *ename || *etext )
		    printf("%3d : %-*s : %s\n", i, max_wd, ename, etext );
	    }

	    if (print_header)
		printf("\n");
	}

	return ERR_OK;
    }

    int stat = ERR_OK;
    long num = ERR__N;
    if ( n_param != 1 )
	stat = ERR_SYNTAX;
    else
    {
	char buf[100];
	StringUpperS(buf,sizeof(buf),first_param->arg);
	int i;
	for ( i = 0; i < ERR__N; i++ )
	    if (!strcmp(GetErrorName(i,""),buf))
	    {
		num = i;
		break;
	    }

	if ( num == ERR__N )
	{
	    char * end;
	    num = strtoul(first_param->arg,&end,10);
	    stat = *end ? ERR_SYNTAX : num < 0 || num >= ERR__N ? ERR_SEMANTIC : ERR_OK;
	}
    }

    if (print_sections)
	printf("\n[error]\ncode=%lu\nname=%s\ntext=%s\n",
		num, GetErrorName(num,"?"), GetErrorText(num,"?"));
    else if (long_count)
	printf("%s\n",GetErrorText(num,"?"));
    else if (brief_count)
	printf("%lu\n",num);
    else
	printf("%s\n",GetErrorName(num,"?"));
    return stat;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command filetype		///////////////
///////////////////////////////////////////////////////////////////////////////

enumError cmd_filetype()
{
 #ifdef __CYGWIN__
    {
	ParamList_t *param;
	for ( param = first_param; param; param = param->next )
	    NORMALIZE_FILENAME_PARAM(param);
    }
 #endif

    if (print_header)
    {
	uint max_len = 0;
	ParamList_t *param;
	for ( param = first_param; param; param = param->next )
	{
	    uint len = strlen(param->arg);
	    if ( max_len < len )
		 max_len = len;
	}

	if ( long_count > 1 )
	    printf("\n"
		   "type    decomp    vers  valid file name\n"
		   "%.*s\n", max_len + 31, Minus300 );
	else if ( long_count )
	    printf("\n"
		   "type    decomp    vers  file name\n"
		   "%.*s\n", max_len + 25, Minus300 );
	else
	    printf("\n"
		   "type    file name\n"
		   "%.*s\n", max_len + 9, Minus300 );
    }

    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	char buf1[CHECK_FILE_SIZE], buf2[CHECK_FILE_SIZE], bufstat[10];
	FileAttrib_t fatt;
	enumError err = LoadFILE(param->arg,0,0,buf1,sizeof(buf1),1,&fatt,false);
	if ( err <= ERR_WARNING || !opt_ignore )
	{
	    file_format_t fform1;
	    if (S_ISDIR(fatt.mode))
	    {
		err = ERR_OK;
		fform1 = FF_DIRECTORY;
	    }
	    else
// [[analyse-magic]]
		fform1 = GetByMagicFF(buf1,sizeof(buf1),fatt.size);

	    ccp stat1 = err > ERR_WARNING ? "-" : GetNameFF(0,fform1);
	    ccp buf = buf1;
	    if (long_count)
	    {
		size_t written = fatt.size;
		file_format_t fform2 = fform1;
		ccp stat2 = "-";
		int version = -1;
		char vbuf[20] = "- ";
		char suffix = 0;
		bool load_full = false;
		valid_t valid = VALID_UNKNOWN_FF;

		if (IsYazFF(fform1))
		{
		    if ( fform1 == FF_XYZ )
			DecodeXYZ((u8*)buf1,(u8*)buf1,sizeof(buf1));

		    fatt.size = be32(&((yaz0_header_t*)buf1)->uncompressed_size);
		    DecompressYAZ( buf1 + sizeof(yaz0_header_t),
					sizeof(buf1) - sizeof(yaz0_header_t),
					buf2, sizeof(buf2),
					&written, param->arg,
					GetYazVersionFF(fform1), true, 0 );
		    DASSERT( written <= sizeof(buf2) );
// [[analyse-magic]]
		    fform2 = GetByMagicFF(buf2,written,written);
		    version = GetVersionFF(fform2,buf2,written,&suffix);
		    stat2 = GetNameFF(0,fform2);
		    buf = buf2;
		}
		else if ( fform1 == FF_BZ )
		{
		    load_full = true;
		    wbz_header_t *wh = (wbz_header_t*)buf1;
// [[analyse-magic]]
		    const file_format_t ff_temp
			= GetByMagicFF(wh->first_8,sizeof(wh->first_8),be32(wh->bz_data));
		    stat2 = GetNameFF(0,ff_temp);
		    version = GetVersionFF(ff_temp,wh->first_8,sizeof(wh->first_8),&suffix);
		    if ( fatt.size >= 20 )
		    {
			char compr = buf1[19];
			if ( compr >= '1' && compr <= '9' )
			{
			    snprintf(bufstat,sizeof(bufstat),"%s.%c",stat1,compr);
			    stat1 = bufstat;
			}
		    }
		}
		else if ( fform1 == FF_BZ2 )
		{
		    snprintf(bufstat,sizeof(bufstat),"BZ2.%u",IsBZIP2(buf1,sizeof(buf1)));
		    stat1 = bufstat;

		    uint wr;
		    DecodeBZIP2part(buf2,sizeof(buf2),&wr,buf1,sizeof(buf1));
// [[analyse-magic]]
		    fform2 = GetByMagicFF(buf2,wr,wr);
		    stat2 = GetNameFF(0,fform2);
		}
		else if ( fform1 == FF_PORTDB )
		{
		    const addr_port_version_t *update = (addr_port_version_t*)buf1;
		    snprintf(vbuf,sizeof(vbuf),"r%u ",ntohl(update->revision));
		    valid = VALID_UNKNOWN; // no further tests
		}
		else
		    version = GetVersionFF(fform1,(u8*)buf1,sizeof(buf1),&suffix);

		if ( version >= 0 )
		    snprintf(vbuf,sizeof(vbuf),"%6u%c", version, suffix ? suffix : ' ' );

		if ( long_count > 1 )
		{
		    if ( valid != VALID_UNKNOWN )
		    {
			if (load_full)
			{
			    szs_file_t szs;
			    InitializeSZS(&szs);
			    if (!LoadSZS(&szs,param->arg,true,true,true))
				valid = IsValidSZS(&szs,false);
			    ResetSZS(&szs);
			}
			else
			    valid = IsValid(buf,written,fatt.size,0,fform2,param->arg);
		    }
		    printf("%-7s %-7s %7s %-5s %s\n",
				stat1, stat2, vbuf, valid_text[valid], param->arg );
		}
		else
		    printf("%-7s %-7s %7s %s\n",stat1,stat2,vbuf,param->arg);
	    }
	    else
		printf("%-7s %s\n",stat1,param->arg);
	}
    }

    if (print_header)
	putchar('\n');

    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command fileattrib		///////////////
///////////////////////////////////////////////////////////////////////////////

const KeywordTab_t cmdtab_file_attrib[] =
{
    { FFT_VALID,	"VALID",	0,		0 },
    { FFT_VALID,	"INVALID",	"DIRECTORY",	1 },

    { FFT_COMPRESS,	"COMPRESSED",	0,		0 },
    { FFT_ARCHIVE,	"ARCHIVES",	0,		0 },
    { FFT_GRAPHIC,	"GRAPHICS",	"IMAGES",	0 },
    { FFT_TEXT,		"TEXT",		"TXT",		0 },
    { FFT_M_FTYPE,	"ETC",		0,		1 },

    { FFT_BRSUB,	"BRSUB",	"BR-SUB",	0 },
     { FFT_BRSUB,	"BRESSUB",	"BRES-SUB",	0 },
     { FFT_BRSUB,	"BRRESSUB",	"BRRES-SUB",	0 },
    { FFT_BRSUB2,	"BRSUB2",	"BR-SUB2",	0 },
     { FFT_BRSUB2,	"BRESSUB2",	"BRES-SUB2",	0 },
     { FFT_BRSUB2,	"BRRESSUB2",	"BRRES-SUB2",	0 },
    { FFT_EXTERNAL,	"EXTERNALS",	0,		0 },
    { FFT_EXTERNAL,	"INTERNALS",	0,		1 },

    { FFT_CREATE,	"CREATE",	0,		0 },
    { FFT_EXTRACT,	"EXTRACT",	0,		0 },
    { FFT_CUT,		"CUT",		0,		0 },
    { FFT_DECODE,	"DECODE",	0,		0 },
    { FFT_ENCODE,	"ENCODE",	0,		0 },
    { FFT_PARSER,	"PARSER",	0,		0 },

    { 0,0,0,0 }
};

//-----------------------------------------------------------------------------

enumError cmd_fileattrib()
{
    enumError stat = ERR_OK;

    u8 show[FF_N];
    if (n_param)
    {
	memset(show,0,sizeof(show));
	ParamList_t *param;
	for ( param = first_param; param; param = param->next )
	{
	    uint fform = GetByNameFF(param->arg);
	    if ( fform != FF_UNKNOWN && fform < FF_N )
		show[fform] = 1;
	    else
	    {
		const KeywordTab_t * cmd
		    = ScanKeyword(0,param->arg,cmdtab_file_attrib);
		if (!cmd)
		    stat = ERROR0(ERR_WARNING,
				"Unknown keyword ignored: %s\n", param->arg);
		else if (cmd->opt)
		{
		    int i;
		    for ( i = 0; i < FF_N; i++ )
			if ( !( GetAttribFF(i) & cmd->id ) )
			    show[i] = 1;
		}
		else
		{
		    int i;
		    for ( i = 0; i < FF_N; i++ )
			if ( GetAttribFF(i) & cmd->id )
			    show[i] = 1;
		}
	    }
	}
    }
    else
	memset(show,1,sizeof(show));

    if (print_header)
	printf("\n"
		"type    classification       possible operations%30s\n"
		"%.80s\n",
		"file extensions", Minus300 );

    ccp last = "";
    for(;;)
    {
	ccp found = 0;
	int i, found_idx = 0;
	for ( i = 1; i < FF_N; i++ )
	{
	    if (show[i])
	    {
		ccp name = GetNameFF(0,i);
		if ( strcmp(name,last) > 0 && ( !found || strcmp(name,found) < 0 ) )
		{
		    found = name;
		    found_idx = i;
		}
	    }
	}

	if (!found)
	    break;

	printf("%-7s",GetNameFF(0,found_idx));

	const file_type_t *fft = GetFileTypeFF(found_idx);
	ASSERT(fft);
	ff_attrib_t fa = fft->attrib;
	ccp info;

	switch ( fa & (FFT_VALID|FFT_M_FTYPE) )
	{
	    case 0:				info = "directory"; break;
	    case FFT_VALID|FFT_COMPRESS:	info = "compress"; break;
	    case FFT_VALID|FFT_ARCHIVE:		info = "archive"; break;
	    case FFT_VALID|FFT_GRAPHIC:		info = "graphic"; break;
	    case FFT_VALID|FFT_TEXT:		info = "text"; break;
	    default:				info = "misc"; break;
	}
	printf(" %-9s",info);

	switch ( fa & FFT_M_CLASS )
	{
	    case FFT_BRSUB|FFT_BRSUB2:	info = "bres-sub2"; break;
	    case FFT_BRSUB:		info = "bres-sub"; break;
	    case FFT_EXTERNAL:		info = "external"; break;
	    default:			info = "-"; break;
	}
	printf(" %-9s",info);

	printf("  %s %s %s %s %s",
		fa & FFT_CREATE   ? "create"  : "-     ",
		fa & FFT_EXTRACT  ? "extract" : "-      ",
		fa & FFT_CUT      ? "cut"     : "-  ",
		fa & FFT_DECODE   ? "decode"
		: fa & FFT_LINK   ? "link  "  : "-     ",
		fa & FFT_PARSER   ? "parser"
		: fa & FFT_ENCODE ? "encode"
		: fa & FFT_PATCH  ? "patch "  : "-     " );

	printf("  %-6s",fft->ext);
	if ( fa & FFT_ARCHIVE && strcmp(fft->ext_compr,fft->ext) )
	    printf(" %s",fft->ext_compr);
	if ( strcmp(fft->ext_magic,fft->ext) )
	    printf(" %s",fft->ext_magic);
	putchar('\n');

	last = found;
    }

    if (print_header)
	printf("%.80s\n\n",Minus300);

    return stat;
}

///////////////////////////////////////////////////////////////////////////////

#undef  DEF_ENTRY
#define DEF_ENTRY(a) { FFT_##a, #a },

void ExportFileAttribMakedoc ( FILE * f )
{
    DASSERT(f);

    fprintf(f,"\n#\f\n%.79s\n%.15s%17s%-32s%.15s\n%.79s\n\n",
	Hash200, Hash200, "", "File Attributes", Hash200, Hash200 );

    struct info_t { int val; ccp name; };
    static const struct info_t info_tab[] =
    {
	DEF_ENTRY(VALID)

	DEF_ENTRY(COMPRESS)
	DEF_ENTRY(ARCHIVE)
	DEF_ENTRY(GRAPHIC)
	DEF_ENTRY(TEXT)
	 DEF_ENTRY(M_FTYPE)

	DEF_ENTRY(BRSUB)
	DEF_ENTRY(BRSUB2)
	DEF_ENTRY(EXTERNAL)
	 DEF_ENTRY(M_CLASS)

	DEF_ENTRY(CREATE)
	DEF_ENTRY(EXTRACT)
	DEF_ENTRY(CUT)
	DEF_ENTRY(DECODE)
	DEF_ENTRY(ENCODE)
	DEF_ENTRY(PARSER)
	 DEF_ENTRY(M_SUPPORT)

	DEF_ENTRY(M_ALL)

	{0,0}
    };


    fprintf(f,"#!--- define file attribute flags\n\n");
    const struct info_t *info_ptr;
    for ( info_ptr = info_tab; info_ptr->name; info_ptr++ )
	fprintf(f,"#gdef FILEATT_%-10s = %#10x\n",info_ptr->name,info_ptr->val);

    fprintf(f,	"\n#!--- define file attribute map\n\n"
		"#gdef fileatt$flag = @map\n\n");
    for ( info_ptr = info_tab; info_ptr->name; info_ptr++ )
	fprintf(f,"#gdef fileatt$flag['%s'] %*s %#10x\n",
		info_ptr->name,
		(int)( 10 - strlen(info_ptr->name) ), "=",
		info_ptr->val );


    fprintf(f,	"\n#!--- define file attributes\n\n"
		"#gdef $fileatt = @map\n\n");

    int fform;
    for ( fform = 1; fform < FF_N; fform++ )
    {
	ccp name = GetNameFF(0,fform);
	fprintf(f,
		"#pdef d = @map\n"
		"#pdef d['idx']    = %2u\n"
		"#pdef d['name']   = '%s'\n"
		"#pdef d['ext']    = '%s'\n"
		"#pdef d['c-ext']  = '%s'\n"
		"#pdef d['attrib'] = %#x\n"
		"#gdef $fileatt['%s'] = move(d)\n\n"
		,fform
		,name
		,GetExtFF(0,fform)
		,GetExtFF(fform_compr,fform)
		,GetAttribFF(fform)
		,name
		);
    }
}

#undef  DEF_ENTRY

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command float			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError cmd_float()
{
    u16 w4[2], w8[4];
    memset(w4,0,sizeof(w4));
    memset(w8,0,sizeof(w8));

    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	ccp arg = param->arg;
	if (!arg)
	    continue;
	while ( *arg && (uchar)*arg <= ' ' )
	    arg++;

	if (!strcmp(arg,"+"))
	{
	    write_be32(w4,be32(w4)+1);
	    write_be64(w8,be64(w8)+1);
	}
	else if (!strcmp(arg,"-"))
	{
	    write_be32(w4,be32(w4)-1);
	    write_be64(w8,be64(w8)-1);
	}
	else
	{
	    memset(w4,0,sizeof(w4));
	    memset(w8,0,sizeof(w8));

	    uint skip;
	    if ( arg[0] == '0' && ( arg[1] == 'x' || arg[1] == 'X' ))
		skip = 2;
	    else if ( arg[0] == 'x' || arg[0] == 'X' )
		skip = 1;
	    else
		skip = 0;

	    if (skip)
	    {
		ccp src = arg + skip;
		ScanHexString(w8,sizeof(w8),&src,0,true);
		memcpy(w4,w8,sizeof(w4));
	    }
	    else
	    {
		double d = strtod(arg,0);
		write_bef4(w4,d);
		write_bef8(w8,d);
	    }
	}

	printf(" %04x %04x = %12.9g : %04x %04x %04x %04x = %22.18g : |%s|\n",
		be16(w4+0), be16(w4+1), bef4(w4),
		be16(w8+0), be16(w8+1), be16(w8+2), be16(w8+3), bef8(w8),
		arg );

	if (opt_round)
	{
	    u16 r4[2], r8[4];

	    write_bef4(r4,RoundF3bytes(bef4(w4)));
	    write_bef8(r8,RoundD7bytes(bef8(w8)));
	    printf(" %04x %04x = %12.9g : %04x %04x %04x %04x = %22.18g"
		   " : rounded to 3|7 bytes\n",
		be16(r4+0), be16(r4+1), bef4(r4),
		be16(r8+0), be16(r8+1), be16(r8+2), be16(r8+3), bef8(r8) );

	    write_bef4(r4,RoundF2bytes(bef4(w4)));
	    write_bef8(r8,RoundD6bytes(bef8(w8)));
	    printf(" %04x %04x = %12.9g : %04x %04x %04x %04x = %22.18g"
		   " : rounded to 2|6 bytes\n\n",
		be16(r4+0), be16(r4+1), bef4(r4),
		be16(r8+0), be16(r8+1), be16(r8+2), be16(r8+3), bef8(r8) );
	}
    }
    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command vr-calc			///////////////
///////////////////////////////////////////////////////////////////////////////

static int get_vr_value ( ccp arg, int res_on_err )
{
    if (!arg)
	return res_on_err;

    while ( *arg == ' ' || *arg == '\t' )
	arg++;

    if (!*arg)
	return res_on_err;

    bool neg = *arg == '-' || *arg == '/';
    if (neg)
	arg++;
    int val = str2ul(arg,0,10);
    if ( val > 99999 )
	val = 99999;

    return neg ? -val : val;
}

///////////////////////////////////////////////////////////////////////////////

enumError cmd_vr_calc()
{
 #if HAVE_PRINT
    GetVrDiffByTab(0); // force logging here and now
 #endif

    if (brief_count)
    {
	ParamList_t *param;
	for ( param = first_param; param; param = param->next )
	{
	    ccp arg = param->arg;
	    printf("%4u\n",GetVrDiffByTab(get_vr_value(arg,0)));
	}
    }
    else
    {
	const uint seplen = 3*27;
	if (print_header)
	    printf(
		"\n"
		"%s%.*s%s\n"
		"%s     vr   vr-increment for%s\n"
		"%s  delta  win-lose lose-win%s\n"
		"%s%.*s%s\n",
		colout->heading, seplen, ThinLine300_3, colout->reset,
		colout->heading, colout->reset,
		colout->heading, colout->reset,
		colout->heading, seplen, ThinLine300_3, colout->reset );

	ParamList_t *param;
	for ( param = first_param; param; param = param->next )
	{
	    ccp arg = param->arg;
	    const int delta = get_vr_value(arg,0);
	    printf("%7d %6u %9u\n",
		delta, GetVrDiffByTab(delta), GetVrDiffByTab(-delta) );
	}

	if (print_header)
	    printf("%s%.*s%s\n\n",
		colout->heading, seplen, ThinLine300_3, colout->reset );
    }

    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command vr-race			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError cmd_vr_race()
{
 #if HAVE_PRINT
    GetVrDiffByTab(0); // force logging here and now
 #endif

    enum { MAX = 30 };
    int n = 0, vr0[MAX], delta[MAX] = {0}, delta2[MAX][MAX] = {{0}};

    //--- scan param

    ParamList_t *param;
    for ( param = first_param; param && n < MAX; param = param->next )
    {
	ccp arg = param->arg;
	int repeat = 1;
	ccp star = strchr(arg,'*');
	if (star)
	{
	   repeat = str2l(arg,0,10);
	   arg = star+1;
	}

	const int vr = get_vr_value(arg,-1);
	if ( vr < 0 )
	    continue;

	while ( repeat-- > 0 && n < MAX )
	{
	    vr0[n] = vr;
	    n++;
	}
    }

    if ( n < 2 )
	return ERROR0(ERR_SEMANTIC,"Need VR values of 2..%u players.",MAX);


    //--- calculate delta

    int i, j;
    for ( i = 0; i < n-1; i++ )
    {
	for ( j = i+1; j < n; j++ )
	{
	    const int d = GetVrDiffByTab( vr0[i] - vr0[j] );
	    delta[i] += d;
	    delta[j] -= d;
	    delta2[i][j] = d;
	    delta2[j][i] = -d;
	}
    }


    //--- print table

    if (long_count)
    {
	const uint seplen = 3*(20+5*n);
	printf( "\n"
		"%s%.*s%s\n"
		"%s       vr : %*s=    vr%s\n"
		"%srk. start : %*s= final%s\n"
		"%s%.*s%s\n",
		colout->heading, seplen, ThinLine300_3, colout->reset,
		colout->heading, -5*n, "   vr", colout->reset,
		colout->heading, -5*n, "delta...", colout->reset,
		colout->heading, seplen, ThinLine300_3, colout->reset );

	for ( i = 0; i < n; i++ )
	{
	    printf("%2d. %5d :", i+1, vr0[i] );
	    ccp col = colout->reset;
	    for ( j = 0; j < n; j++ )
	    {
		if ( i == j )
		{
		   printf("%s    •",colout->b_cyan);
		   col = colout->b_cyan;
		}
		else
		{
		    const int d = delta2[i][j];
		    ccp next_col = d < 0 ? colout->b_red
				 : d > 0 ? colout->b_green : colout->reset;
		    if ( col != next_col )
		    {
			col = next_col;
			printf(" %s%4d",col,d);
		    }
		    else
			printf(" %4d",d);
		}
	    }


	    const int d = delta[i];
	    ccp next_col = d < 0 ? colout->b_red  : d > 0 ? colout->b_green : "";

	    printf(" %s= %s%5d%s\n",
		col == colout->reset ? "" : colout->reset,
		next_col, vr0[i] + delta[i],
		*next_col ? colout->reset : "" );
	}
	printf("%s%.*s%s\n\n",
		colout->heading, seplen, ThinLine300_3, colout->reset );
    }
    else
    {
	const uint seplen = 3*28;
	printf( "\n"
		"%s%.*s%s\n"
		"%s          vr     vr      vr%s\n"
		"%s rank  start  delta   final%s\n"
		"%s%.*s%s\n",
		colout->heading, seplen, ThinLine300_3, colout->reset,
		colout->heading, colout->reset,
		colout->heading, colout->reset,
		colout->heading, seplen, ThinLine300_3, colout->reset );

	for ( i = 0; i < n; i++ )
	    printf("%4d. %6d %+6d = %5d\n",
		    i+1, vr0[i], delta[i], vr0[i]+delta[i] );

	printf("%s%.*s%s\n\n",
		colout->heading, seplen, ThinLine300_3, colout->reset );
    }

    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command rawdump			///////////////
///////////////////////////////////////////////////////////////////////////////

enumError cmd_rawdump ( const data_tab_t *data_tab )
{
    CheckOptDest( first_param ? first_param->arg : "rawdump.tmp/", true );
    opt_mkdir = true;

    const data_tab_t *dt;
    for ( dt = data_tab; dt->name; dt++ )
    {
	char path_buf[PATH_MAX];
	ccp dest = PathCatPPE(path_buf,sizeof(path_buf),opt_dest,dt->name,".bin");

	const u8 *data = dt->data;
	uint size = dt->size;
	if ( dt->mode & 1 )
	    DecodeBZIP2((u8**)&data,&size,0,dt->data,dt->size);

	printf("SAVE %7u = %6x  %x %s\n",size,size,dt->mode,dest);
	SaveFILE(dest,0,true,data,size,0);

	u8  *bz_data = 0;
	uint bz_size = 0, bz_compr = 0;
	uint compr;

	for ( compr = 1; compr <= 9; compr++ )
	{
	    u8  *temp_data;
	    uint temp_size;
	    enumError err = EncodeBZIP2( &temp_data, &temp_size, true,
					0, data, size, compr );
	    if ( !err && ( !bz_data || temp_size < bz_size ) )
	    {
		FREE(bz_data);
		bz_data  = temp_data;
		bz_size  = temp_size;
		bz_compr = compr;
	    }
	    else
		FREE(temp_data);
	}

	if (bz_data)
	{
	    dest = PathCatPPE(path_buf,sizeof(path_buf),opt_dest,dt->name,".bz-bin");
	    printf("SAVE %7u = %6x    %s [c%u,%u%%]\n",
		bz_size, bz_size, dest, bz_compr, bz_size*100/size );
	    SaveFILE(dest,0,true,bz_data,bz_size,0);
	}

	if ( data != dt->data )
	    FREE((u8*)data);
    }

    return ERR_OK;
}


///////////////////////////////////////////////////////////////////////////////
///////////////			export keys			///////////////
///////////////////////////////////////////////////////////////////////////////

export_mode_t ScanExportMode() // scan parameters
{
    static const KeywordTab_t tab[] =
    {
	{  -1,			"HELP",		"?",		0 },

//	{  EXPORT_C,		"C",		0,		EXPORT_CMD_MASK },
	{  EXPORT_PHP,		"PHP",		0,		EXPORT_CMD_MASK },
	{  EXPORT_MAKEDOC,	"MAKEDOC",	0,		EXPORT_CMD_MASK },

	{  EXPORT_F_FUNCTIONS,	"FUNCTIONS",	0,		0 },
	{  EXPORT_F_FILEATTRIB,	"FILEATTRIB",	"FA",		0 },
	{  EXPORT_F_TRACKS,	"TRACKS",	"T",		0 },

	{ 0,0,0,0 }
    };

    export_mode_t exmode = 0;

    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);
	ccp arg = param->arg;
	if (!*arg)
	    continue;

	const KeywordTab_t * cmd = ScanKeyword(0,arg,tab);
	if (!cmd)
	    continue;

	if ( cmd->id == -1 )
	{
	    printf("\nList of kexwords:\n\n");
	    const KeywordTab_t *ptr;
	    for ( ptr = tab; ptr->name1; ptr++ )
	    {
		if (ptr->name2)
		    printf("   %s = %s\n",ptr->name1,ptr->name2);
		else
		    printf("   %s\n",ptr->name1);
	    }
	    putchar('\n');
	    return 0;
	}

	if (cmd->opt)
	    exmode &= ~cmd->opt;
	exmode |= cmd->id;
    }

    if (!( exmode & EXPORT_FLAG_MASK ))
	exmode |= EXPORT_FLAG_MASK;

    if (!( exmode & EXPORT_CMD_MASK ))
    {
	ERROR0(ERR_SYNTAX,"Export: No ouput format entered!\n");
	exmode = 0;
    }

    return exmode;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			repair magic			///////////////
///////////////////////////////////////////////////////////////////////////////
// [[repair_mode_t]]

// vars for --repair-magic
repair_mode_t repair_magic = REPMD_OFF;
repair_mode_t repair_magic_flags = 0;

///////////////////////////////////////////////////////////////////////////////

int ScanOptRepairMagic ( ccp arg )
{
    if (!arg)
    {
	repair_magic = REPMD_REPAIR;
	repair_magic_flags = 0;
	return 0;
    }

    static const KeywordTab_t keytab[] =
    {
	{ REPMD_OFF,		"OFF",		0,	REPMD_M_MODES },
	{ REPMD_ANALYZE,	"ANALYZE",	0,	REPMD_M_MODES },
	{ REPMD_REPAIR,		"REPLACE",	"ON",	REPMD_M_MODES },
	{ REPMD_F_FNAME_FIRST,	"FNAME",	0,	0 },

	{0,0,0,0}
    };

    const s64 stat = ScanKeywordList(arg,keytab,0,true,0,0,
					"Option --repair-magic",ERR_SYNTAX);
    if ( stat == -1 )
	return 1;

    repair_magic	= stat & REPMD_M_MODES;
    repair_magic_flags	= stat & REPMD_M_FLAGS;
    PRINT("%llx => repair_magic=%x,%x\n",stat,repair_magic,repair_magic_flags);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

repair_ff_t * ResetRepairMagic ( repair_ff_t * data, repair_ff_t * fallback )
{
    static repair_ff_t falback2;
    if (!data)
	data = fallback ? fallback : &falback2;

    memset(data,0,sizeof(*data));
    data->mode	= REPMD_OFF;
    data->fform	= FF_UNKNOWN;

    return data;
}

///////////////////////////////////////////////////////////////////////////////

ccp GetRepairMagicInfo ( const repair_ff_t *rep )
{
    if (!rep)
	return NULL;

    char buf[80];
    int len = snprintfS(buf,sizeof(buf),
		"smode=%u%s%s%s, fform=%s, ext=%s, magic=%s/%s",
		rep->mode,
		rep->flags & REPMD_F_FNAME_FIRST? "f" : "",
		rep->flags & REPMD_R_FNAME	? "F" : "",
		rep->flags & REPMD_R_FALLBACK	? "X" : "",
		GetNameFF(rep->fform,0), rep->file_ext,
		PrintID(rep->orig_magic,rep->magic_len,0),
		PrintID(rep->magic,rep->magic_len,0)
		) + 1;

    char *res = GetCircBuf(len);
    memcpy(res,buf,len);
    return res;
}

///////////////////////////////////////////////////////////////////////////////

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
)
{
    //fprintf(stderr,">>> %s\n",fname); HexDump16(stderr,0,0,data,16);

    repair_ff_t local_rep;
    rep = ResetRepairMagic(rep,&local_rep);
    DASSERT(rep);

    memcpy( rep->magic, data,
		data_size < sizeof(rep->magic) ? data_size : sizeof(rep->magic) );
    memcpy( rep->orig_magic, rep->magic, sizeof(rep->orig_magic) );

    if (!be32(rep->magic))
	fname_first = true;
    rep->flags = fname_first ? REPMD_F_FNAME_FIRST : 0;

    if (fname)
    {
	ccp point = strrchr(fname,'.');
	if (point)
	{
	    ccp slash = strrchr(fname,'/');
	    if ( !slash || point > slash && strlen(point) < sizeof(rep->file_ext) )
		StringLowerS(rep->file_ext,sizeof(rep->file_ext),point);
	}
    }

    repair_mode_t status = 0;
    file_format_t ff_magic = GetByMagicFF(data,data_size,file_size);
    if ( ff_magic == FF_UNKNOWN && ff_fallback != FF_UNKNOWN )
    {
	ff_magic = ff_fallback;
	status = REPMD_R_FALLBACK;
    }

    rep->magic_len = FileTypeTab[ff_magic].magic_len;
    if ( ff_magic != FF_UNKNOWN && !fname_first )
    {
	rep->flags |= status;
	return rep->fform = ff_magic;
    }

    const file_type_t *ft = GetFileTypeByExt(rep->file_ext,true);
    if (!ft)
	ft = GetFileTypeBySubdir(fname,true);
    rep->flags |= ft ? REPMD_R_FNAME : status;
    if ( !ft || ft->fform == ff_magic )
	return rep->fform = ff_magic;

    //fprintf(stderr,">>> ff: %d > %d : %s\n",ff_magic,ft->fform,fname);
    DASSERT(ft);
    rep->fform = ft->fform;
    const uint mlen = ft->magic_len < sizeof(rep->magic)
		    ? ft->magic_len : sizeof(rep->magic);
    rep->magic_len  = mlen;
    memset(rep->magic,0,sizeof(rep->magic));
    memcpy(rep->magic,ft->magic,mlen);
    rep->mode = REPMD_ANALYZE;

    PRINT("AnalyseMagic() %s\n",GetRepairMagicInfo(rep));
    return rep->fform;
}

///////////////////////////////////////////////////////////////////////////////

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
)
{
    repair_ff_t local_rep;
    if (!rep)
	rep = &local_rep;
    AnalyseMagic(rep,data,data_size,file_size,ff_fallback,fname,fname_first);

    if ( rep->mode == REPMD_ANALYZE && rep->magic_len )
    {
	if ( logging >= 2 )
	    fprintf(stderr,"REPAIR MAGIC: %s -> %s (%s) : %s\n",
		PrintID(data,rep->magic_len,0),
		PrintID(rep->magic,rep->magic_len,0),
		GetNameFF(0,rep->fform), fname );

	rep->mode = REPMD_REPAIR;
	memcpy(data,rep->magic,rep->magic_len);
    }

    PRINT("RepairMagic() %s\n",GetRepairMagicInfo(rep));
    return rep->fform;
}

///////////////////////////////////////////////////////////////////////////////

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
)
{
    repair_ff_t local_rep;
    rep = ResetRepairMagic(rep,&local_rep);
    DASSERT(rep);

    const bool fname_first = ( repair_magic_flags & REPMD_F_FNAME_FIRST ) != 0;
    switch(repair_magic)
    {
	case REPMD_ANALYZE:
	case REPMD_REPAIR:
	    AnalyseMagic(rep,data,data_size,file_size,ff_fallback,fname,fname_first);
	    break;

	default:
	    rep->fform = GetByMagicFF(data,data_size,file_size);
	    if ( rep->fform == FF_UNKNOWN && ff_fallback != FF_UNKNOWN )
	    {
		rep->fform = ff_fallback;
		rep->flags |= REPMD_R_FALLBACK;
	    }
	    break;
    }
    return rep->fform;
}

///////////////////////////////////////////////////////////////////////////////

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
)
{
    repair_ff_t local_rep;
    rep = ResetRepairMagic(rep,&local_rep);
    DASSERT(rep);

    const bool fname_first = ( repair_magic_flags & REPMD_F_FNAME_FIRST ) != 0;
    switch(repair_magic)
    {
	case REPMD_ANALYZE:
	    AnalyseMagic(rep,data,data_size,file_size,ff_fallback,fname,fname_first);
	    break;

	case REPMD_REPAIR:
	    RepairMagic(rep,data,data_size,file_size,ff_fallback,fname,fname_first);
	    break;

	default:
	    rep->fform = GetByMagicFF(data,data_size,file_size);
	    if ( rep->fform == FF_UNKNOWN && ff_fallback != FF_UNKNOWN )
	    {
		rep->fform = ff_fallback;
		rep->flags |= REPMD_R_FALLBACK;
	    }
	    break;
    }
    return rep->fform;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			SHA1 support			///////////////
///////////////////////////////////////////////////////////////////////////////

const sha1_db_t * GetSha1DbHex ( sha1_type_t type, ccp hex, ccp end )
{
    sha1_hash_t hash;
    Sha1Hex2Bin(hash,hex,end);
    return GetSha1DbBin(type,hash);
}

///////////////////////////////////////////////////////////////////////////////

const sha1_db_t * GetSha1DbBin ( sha1_type_t type, cvp hash )
{
    int beg = 0;
    int end = SHA1_DB_N -1;
    while ( beg <= end )
    {
	const int idx = (beg+end)/2;
	const int stat = memcmp(hash,sha1_db[idx].sha1,sizeof(sha1_db->sha1));
	if ( stat < 0 )
	    end = idx - 1 ;
	else if ( stat > 0 )
	    beg = idx + 1;
	else
	{
	    const sha1_db_t *res =  sha1_db + idx;
	    return res->type & type ? res : 0;
	}
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

u8 GetSha1Slot ( sha1_type_t type, cvp hash )
{
    const sha1_db_t *res = GetSha1DbBin(type,hash);
    return res ? res->slot : 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SHA1_to_ID ( sha1_id_t id, const sha1_hash_t hash )
{
    //--- create a smaller hash value (20->5 bytes with 3 bytes padding)

    u8 buf[8];
    u8 * dest = buf;
    const u8 * src = hash;

    uint i;
    for ( i = 0; i < 5; i++ )
    {
	*dest++ = src[0] ^ src[5] ^ src[10] ^ src[15];
	src++;
    }

    //--- now create a printable and case insenitive ID

    static const u8 ctab[] = "0123456789abcdefghijklmnopqrstuv";
    u64 val = le64(buf);
    dest = id;
    for ( i = 0; i < 8; i++ )
    {
	*dest++ = ctab[ val & 0x1f ];
	val >>= 5;
    }
    *dest = 0;
}

///////////////////////////////////////////////////////////////////////////////

ccp GetSha1Data ( cvp data, uint size )
{
    sha1_hash_t hash;
    SHA1(data,size,hash);
    return GetSha1Hex(hash);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    misc			///////////////
///////////////////////////////////////////////////////////////////////////////

void SetupPrintScriptByOptions ( PrintScript_t *ps )
{
    DASSERT(ps);

    InitializePrintScript(ps);
    ps->force_case	= opt_case;
    ps->create_array	= script_array > 0;
    ps->var_name	= script_varname ? script_varname : "res";
    ps->var_prefix	= script_varname ? script_varname : "res_";
    ps->eq_tabstop	= 2;
    ps->ena_empty	= brief_count < 2;
    ps->ena_comments	= brief_count < 1;

    switch(script_fform)
    {
      case FF_JSON:	ps->fform = PSFF_JSON; break;
      case FF_BASH:	ps->fform = PSFF_BASH; break;
      case FF_SH:	ps->fform = PSFF_SH; break;
      case FF_PHP:	ps->fform = PSFF_PHP; break;
      case FF_MAKEDOC:	ps->fform = PSFF_MAKEDOC; break;
      default:		ps->fform = print_sections ? PSFF_CONFIG : PSFF_UNKNOWN; break;
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool IsExtractEnabled()
{
    static bool enabled = false;
    if ( opt_extract && *opt_extract && !enabled )
    {
	if ( CreatePath(opt_extract,true) == ERR_OK )
	    enabled = true;
    }
    return enabled;
}

//-----------------------------------------------------------------------------

bool PrepareExtract
(
    // return FALSE on disabled or failure, otherwise return TRUE

    File_t	*f,		// file structure, will be initialized
    int		mode,		// convert filename: <0=to lower, >0=to upper
    ccp		format,		// format of filename
    ...				// arguments
)
{
    DASSERT(f);
    DASSERT(format);

    if (!IsExtractEnabled())
    {
	InitializeFile(f);
	return 0;
    }

    char fname[1000];
    va_list arg;
    va_start(arg,format);
    vsnprintf(fname,sizeof(fname),format,arg);
    va_end(arg);

    if (mode)
    {
	char *ptr = fname;
	if ( mode < 0 )
	    while ( ( *ptr = tolower((int)*ptr) ) != 0 )
		ptr++;
	else
	    while ( ( *ptr = toupper((int)*ptr) ) != 0 )
		ptr++;
    }

    char pathbuf[PATH_MAX];
    ccp path = PathCatPP(pathbuf,sizeof(pathbuf),opt_extract,fname);
    CreateFile(f,true,path,FM_REMOVE);
    return f->f != NULL;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

CheckMode_t GetCheckMode
(
    bool	no_head,	// true: suppress header
    bool	no_hints,	// true: suppress hints
    bool	quiet,		// true: be quiet if no hint|warning is printed
    bool	verbose		// true: print some more hints
)
{
    CheckMode_t mode = CMOD_DEFAULT;
    if (no_head)
	 mode &= ~(CMOD_HEADER|CMOD_FORCE_HEADER);
    if (no_hints)
	 mode &= ~CMOD_HINT;
    if (quiet)
	 mode &= ~(CMOD_FORCE_HEADER|CMOD_FORCE_FOOTER);
    if (verbose)
	mode |= CMOD_INFO|CMOD_VERBOSE;
    return mode;
}

///////////////////////////////////////////////////////////////////////////////

CheckMode_t GetMainCheckMode ( CheckMode_t mode, bool reset_counter )
{
    if (reset_counter)
    {
	global_warn_count = 0;
	global_hint_count = 0;
	global_info_count = 0;
    }

    return mode | CMOD_FORCE_HEADER;
}

///////////////////////////////////////////////////////////////////////////////

CheckMode_t GetSubCheckMode ( CheckMode_t mode, bool reset_counter )
{
    return mode & ~(CMOD_FOOTER|CMOD_FORCE_FOOTER)
		| CMOD_FORCE_HEADER
		| CMOD_GLOBAL_STAT;
}

///////////////////////////////////////////////////////////////////////////////

uint GetMkwMusicSlot ( uint tid )
{
    if ( tid < MKW_N_TRACKS )
	return track_info[tid].music_id;

    if ( tid - MKW_N_TRACKS < MKW_N_ARENAS )
	return arena_info[tid-MKW_N_TRACKS].music_id;

    switch (tid)
    {
	case 0x36: return 0xc9; // Galaxy Stadium
	default:   return 0x75;
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			apple only			///////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef __APPLE__
///////////////////////////////////////////////////////////////////////////////

int ____chkstk_darwin() { return 0; }

///////////////////////////////////////////////////////////////////////////////
#endif // __APPLE__
//
///////////////////////////////////////////////////////////////////////////////
///////////////			ImageGeometry_t			///////////////
///////////////////////////////////////////////////////////////////////////////

static const ImageGeometry_t img_geo_I4 =
{
    IMG_I4,		// iform
    0, 1, 0, 1,		// is_x, is_gray, has_alpha, has_blocks
    0, 1, 1,		// max_palette, read_support, write_support
    4, 8, 8, 32,	// bits_per_pixel, block: width,height,size
    "I4"		// name
};

//-----------------------------------------------------------------------------

static const ImageGeometry_t img_geo_I8 =
{
    IMG_I8,		// iform
    0, 1, 0, 1,		// is_x, is_gray, has_alpha, has_blocks
    0, 1, 1,		// max_palette, read_support, write_support
    8, 8, 4, 32,	// bits_per_pixel, block: width,height,size
    "I8"		// name
};

//-----------------------------------------------------------------------------

static const ImageGeometry_t img_geo_IA4 =
{
    IMG_IA4,		// iform
    0, 1, 1, 1,		// is_x, is_gray, has_alpha, has_blocks
    0, 1, 1,		// max_palette, read_support, write_support
    8, 8, 4, 32,	// bits_per_pixel, block: width,height,size
    "IA4"		// name
};

//-----------------------------------------------------------------------------

static const ImageGeometry_t img_geo_IA8 =
{
    IMG_IA8,		// iform
    0, 1, 1, 1,		// is_x, is_gray, has_alpha, has_blocks
    0, 1, 1,		// max_palette, read_support, write_support
    16, 4, 4, 32,	// bits_per_pixel, block: width,height,size
    "IA8"		// name
};

//-----------------------------------------------------------------------------

static const ImageGeometry_t img_geo_RGB565 =
{
    IMG_RGB565,		// iform
    0, 0, 0, 1,		// is_x, is_gray, has_alpha, has_blocks
    0, 1, 1,		// max_palette, read_support, write_support
    16, 4, 4, 32,	// bits_per_pixel, block: width,height,size
    "RGB565"		// name
};

//-----------------------------------------------------------------------------

static const ImageGeometry_t img_geo_RGB5A3 =
{
    IMG_RGB5A3,		// iform
    0, 0, 1, 1,		// is_x, is_gray, has_alpha, has_blocks
    0, 1, 1,		// max_palette, read_support, write_support
    16, 4, 4, 32,	// bits_per_pixel, block: width,height,size
    "RGB5A3"		// name
};

//-----------------------------------------------------------------------------

static const ImageGeometry_t img_geo_RGBA32 =
{
    IMG_RGBA32,		// iform
    0, 0, 1, 1,		// is_x, is_gray, has_alpha, has_blocks
    0, 1, 1,		// max_palette, read_support, write_support
    32, 4, 4, 64,	// bits_per_pixel, block: width,height,size
    "RGBA32"		// name
};

//-----------------------------------------------------------------------------

static const ImageGeometry_t img_geo_C4 =
{
    IMG_C4,		// iform
    0, 0, 1, 1,		// is_x, is_gray, has_alpha, has_blocks
    0x10, 1, 0,		// max_palette, read_support, write_support
    4, 8, 8, 32,	// bits_per_pixel, block: width,height,size
    "C4"		// name
};

//-----------------------------------------------------------------------------

static const ImageGeometry_t img_geo_C8 =
{
    IMG_C8,		// iform
    0, 0, 1, 1,		// is_x, is_gray, has_alpha, has_blocks
    0x100, 1, 0,	// max_palette, read_support, write_support
    8, 8, 4, 32,	// bits_per_pixel, block: width,height,size
    "C8"		// name
};

//-----------------------------------------------------------------------------

static const ImageGeometry_t img_geo_C14X2 =
{
    IMG_C14X2,		// iform
    0, 0, 1, 1,		// is_x, is_gray, has_alpha, has_blocks
    0x4000, 1, 0,	// max_palette, read_support, write_support
    16, 4, 4, 32,	// bits_per_pixel, block: width,height,size
    "C14X2"		// name
};

//-----------------------------------------------------------------------------

static const ImageGeometry_t img_geo_CMPR =
{
    IMG_CMPR,		// iform
    0, 0, 1, 1,		// is_x, is_gray, has_alpha, has_blocks
    0, 1, 1,		// max_palette, read_support, write_support
    4, 8, 8, 32,	// bits_per_pixel, block: width,height,size
    "CMPR"		// name
};

//-----------------------------------------------------------------------------

static const ImageGeometry_t img_geo_X_GRAY =
{
    IMG_X_GRAY,		// iform
    1, 1, 1, 0,		// is_x, is_gray, has_alpha, has_blocks
    0, 1, 1,		// max_palette, read_support, write_support
    16, 1, 1, 2,	// bits_per_pixel, block: width,height,size
    "X-GRAY"		// name
};

//-----------------------------------------------------------------------------

static const ImageGeometry_t img_geo_X_RGB =
{
    IMG_X_RGB,		// iform
    1, 0, 1, 0,		// is_x, is_gray, has_alpha, has_blocks
    0, 1, 1,		// max_palette, read_support, write_support
    32, 1, 1, 4,	// bits_per_pixel, block: width,height,size
    "X-RGB"		// name
};

//-----------------------------------------------------------------------------

static const ImageGeometry_t img_geo_X_PAL_4 =
{
    IMG_X_PAL4,		// iform
    1, 0, 1, 0,		// is_x, is_gray, has_alpha, has_blocks
    0x10, 1, 1,		// max_palette, read_support, write_support
    16, 1, 1, 2,	// bits_per_pixel, block: width,height,size
    "X-PAL4"		// name
};

//-----------------------------------------------------------------------------

static const ImageGeometry_t img_geo_X_PAL_8 =
{
    IMG_X_PAL8,		// iform
    1, 0, 1, 0,		// is_x, is_gray, has_alpha, has_blocks
    0x100, 1, 1,	// max_palette, read_support, write_support
    16, 1, 1, 2,	// bits_per_pixel, block: width,height,size
    "X-PAL8"		// name
};

//-----------------------------------------------------------------------------

static const ImageGeometry_t img_geo_X_PAL_14 =
{
    IMG_X_PAL14,	// iform
    1, 0, 1, 0,		// is_x, is_gray, has_alpha, has_blocks
    0x4000, 1, 1,	// max_palette, read_support, write_support
    16, 1, 1, 2,	// bits_per_pixel, block: width,height,size
    "X-PAL14"		// name
};

///////////////////////////////////////////////////////////////////////////////

const ImageGeometry_t * GetImageGeometry ( image_format_t iform )
{
    switch (iform)
    {
	case IMG_I4:	  return &img_geo_I4;
	case IMG_I8:	  return &img_geo_I8;
	case IMG_IA4:	  return &img_geo_IA4;
	case IMG_IA8:	  return &img_geo_IA8;
	case IMG_RGB565:  return &img_geo_RGB565;
	case IMG_RGB5A3:  return &img_geo_RGB5A3;
	case IMG_RGBA32:  return &img_geo_RGBA32;
	case IMG_C4:	  return &img_geo_C4;
	case IMG_C8:	  return &img_geo_C8;
	case IMG_C14X2:	  return &img_geo_C14X2;
	case IMG_CMPR:	  return &img_geo_CMPR;
	case IMG_X_GRAY:  return &img_geo_X_GRAY;
	case IMG_X_RGB:	  return &img_geo_X_RGB;
	case IMG_X_PAL4:  return &img_geo_X_PAL_4;
	case IMG_X_PAL8:  return &img_geo_X_PAL_8;
	case IMG_X_PAL14: return &img_geo_X_PAL_14;
	case IMG_X_PAL:   return &img_geo_X_PAL_14;

	case IMG_N_INTERN:
	case IMG_INVALID:
	case IMG_X_AUTO:
	    return 0;
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

ccp PrintImageFormat
(
    image_format_t	iform,	// image format
    palette_format_t	pform	// palette format
)
{
    const ImageGeometry_t *geo = GetImageGeometry(iform);
    if (geo)
    {
	if (!geo->max_palette)
	    return geo->name;

	ccp pal;
	switch (pform)
	{
	    case PAL_IA8:	pal = "IA8";	break;
	    case PAL_RGB565:	pal = "RGB565";	break;
	    case PAL_RGB5A3:	pal = "RGB5A3";	break;
	    case PAL_X_RGB:	pal = "X-RGB";	break;
	    default:		pal = 0;
	}

	const uint bufsize = 20;
	char *buf = GetCircBuf(bufsize);
	if (pal)
	    snprintf(buf,bufsize,"%s.%s",geo->name,pal);
	else
	    snprintf(buf,bufsize,"%s.#%x",geo->name,pform);
	return buf;
    }

    const uint bufsize = 20;
    char *buf = GetCircBuf(bufsize);
    snprintf(buf,bufsize,"#%x",iform);
    return buf;
}

///////////////////////////////////////////////////////////////////////////////

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
)
{
    const uint hblocks = ( width + block_width - 1 ) / block_width;
    if (h_blocks)
	*h_blocks = hblocks;

    const uint xwidth = hblocks * block_width;
    if (x_width)
	*x_width = xwidth;

    const uint vblocks = ( height + block_height - 1 ) / block_height;
    if (v_blocks)
	*v_blocks = vblocks;

    const uint xheight = vblocks * block_height;
    if (x_height)
	*x_height = xheight;

    return xwidth * xheight * bits_per_pixel / 8;
}

///////////////////////////////////////////////////////////////////////////////

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
)
{
    const ImageGeometry_t *geo = GetImageGeometry(iform);
    if (geo)
    {
	const uint size
	    = CalcImageSize( width, height,
			geo->bits_per_pixel, geo->block_width, geo->block_height,
			x_width, x_height, h_blocks, v_blocks );
	if (img_size)
	    *img_size = size;
	return geo;
    }

    if (x_width)  *x_width  = width;
    if (x_height) *x_height = height;
    if (h_blocks) *h_blocks = width;
    if (v_blocks) *v_blocks = height;
    if (img_size) *img_size = 0;
    return 0;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			   image formats		///////////////
///////////////////////////////////////////////////////////////////////////////

const KeywordTab_t cmdtab_image_format[] =
{
    { IMG_I4,		"I4",		0,		CF_IMAGE },
    { IMG_I8,		"I8",		0,		CF_IMAGE },
    { IMG_IA4,		"IA4",		0,		CF_IMAGE },
    { IMG_IA8,		"IA8",		0,		CF_IMAGE },
    { IMG_RGB565,	"RGB565",	0,		CF_IMAGE },
    { IMG_RGB5A3,	"RGB5A3",	0,		CF_IMAGE },
    { IMG_RGBA32,	"RGBA32",	"RGBA8",	CF_IMAGE },
    { IMG_C4,		"C4",		"CI4",		CF_IMAGE|CF_PALETTE },
    { IMG_C8,		"C8",		"CI8",		CF_IMAGE|CF_PALETTE },
    { IMG_C14X2,	"C14X2",	"CI14X2",	CF_IMAGE|CF_PALETTE },
    { IMG_CMPR,		"CMPR",		0,		CF_IMAGE },

    { IMG_X_AUTO,	"AUTO",		0,		CF_AUTO },
    { IMG_X_GRAY,	"X-GRAY",	"GRAY",		0 },
    { IMG_X_RGB,	"X-RGB",	"RGB",		0 },

    { IMG_X_PAL4,	"X-PAL4",	"PAL4",		0 },
    { IMG_X_PAL8,	"X-PAL8",	"PAL8",		0 },
    { IMG_X_PAL14,	"X-PAL14",	"PAL14",	0 },
    { IMG_X_PAL,	"X-PAL",	"PAL",		CF_AUTO },
     { IMG_X_PAL,	"X-PALETTE",	"PALETTE",	CF_AUTO },

    { 0,0,0,0 }
};

///////////////////////////////////////////////////////////////////////////////

image_format_t ScanImageFormat ( ccp arg )
{
    const KeywordTab_t * cmd = ScanKeyword(0,arg,cmdtab_image_format);
    return cmd ? cmd->id : IMG_INVALID;
}

///////////////////////////////////////////////////////////////////////////////

ccp GetImageFormatName ( image_format_t iform, ccp unknown_value )
{
    if ( iform >= 0 )
    {
	const KeywordTab_t * ct;
	for ( ct = cmdtab_image_format; ct->name1; ct++ )
	    if ( ct->id == iform )
		return ct->name1;
    }
    return unknown_value;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			palette formats			///////////////
///////////////////////////////////////////////////////////////////////////////

const KeywordTab_t cmdtab_palette_format[] =
{
    { PAL_IA8,		"P-IA8",	"PIA8",		0 },
    { PAL_IA8,		"GRAY",		"IA8",		0 },

    { PAL_RGB565,	"P-RGB565",	"PRGB565",	0 },
    { PAL_RGB565,	"COLOR",	"RGB565",	0 },

    { PAL_RGB5A3,	"P-RGB5A3",	"PRGB5A3",	0 },
    { PAL_RGB5A3,	"ALPHA",	"RGB5A3",	0 },

    { PAL_X_RGB,	"P-RGB",	"RGB",		0 },

    { PAL_AUTO,		"P-AUTO",	"AUTO",		0 },

    { 0,0,0,0 }
};

///////////////////////////////////////////////////////////////////////////////

palette_format_t ScanPaletteFormat ( ccp arg )
{
    const KeywordTab_t * cmd = ScanKeyword(0,arg,cmdtab_palette_format);
    return cmd ? cmd->id : PAL_INVALID;
}

///////////////////////////////////////////////////////////////////////////////

ccp GetPaletteFormatName ( palette_format_t pform, ccp unknown_value )
{
    if ( pform >= 0 )
    {
	const KeywordTab_t * ct;
	for ( ct = cmdtab_palette_format; ct->name1; ct++ )
	    if ( ct->id == pform )
		return ct->name1;
    }
    return unknown_value;
}

///////////////////////////////////////////////////////////////////////////////

int PaletteToImageFormat ( int pform, int return_if_invalid )
{
    switch (pform)
    {
	case PAL_IA8:     return IMG_IA8;
	case PAL_RGB565:  return IMG_RGB565;
	case PAL_RGB5A3:  return IMG_RGB5A3;
	case PAL_X_RGB:   return IMG_X_RGB;
    }
    return return_if_invalid;
}

///////////////////////////////////////////////////////////////////////////////

uint GetPaletteCountIF
(
    image_format_t	iform		// image format
)
{
    switch (iform)
    {
	case IMG_C4:
	case IMG_X_PAL4:
	    return 0x10;

	case IMG_C8:
	case IMG_X_PAL8:
	    return 0x100;

	case IMG_C14X2:
	case IMG_X_PAL14:
	case IMG_X_PAL:
	    return 0x4000;

	default:
	    return 0;
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			scan patch options		///////////////
///////////////////////////////////////////////////////////////////////////////

static const KeywordTab_t cmdtab_patch_image[] =
{
    // order is important for back calculations

    { 0,		"DEFAULT",	"D",		PIM_MASK },
    { PIM_CENTER,	"CENTER",	"C",		PIM_M_V | PIM_M_H },

    { PIM_VCENTER,	"VCENTER",	"V",		PIM_M_V },
    { PIM_TOP,		"TOP",		"T",		PIM_M_V },
    { PIM_BOTTOM,	"BOTTOM",	"B",		PIM_M_V },
    { PIM_INS_TOP,	"INS-TOP",	"IT",		PIM_M_V },
    { PIM_INS_BOTTOM,	"INS-BOTTOM",	"IB",		PIM_M_V },

    { PIM_HCENTER,	"HCENTER",	"H",		PIM_M_H },
    { PIM_LEFT,		"LEFT",		"L",		PIM_M_H },
    { PIM_RIGHT,	"RIGHT",	"R",		PIM_M_H },
    { PIM_INS_LEFT,	"INS-LEFT",	"IL",		PIM_M_H },
    { PIM_INS_RIGHT,	"INS-RIGHT",	"IR",		PIM_M_H },

    { PIM_MIX,		"MIX",		"MX",		PIM_M_COPY },
    { PIM_FOREGROUND,	"FOREGROUND",	"FG",		PIM_M_COPY },
    { PIM_BACKGROUND,	"BACKGROUND",	"BG",		PIM_M_COPY },
    { PIM_COPY,		"COPY",		"CP",		PIM_M_COPY },

    { PIM_LEAVE,	"LEAVE",	"LV",		PIM_M_SIZE },
    { PIM_SHRINK,	"SHRINK",	"SH",		PIM_M_SIZE },
    { PIM_GROW,		"GROW",		"GR",		PIM_M_SIZE },

    {0,0,0,0}
};

///////////////////////////////////////////////////////////////////////////////

bool ScanPatchImage ( ccp arg, PatchImage_t * mode )
{
    const KeywordTab_t * cmd = ScanKeyword(0,arg,cmdtab_patch_image);
    if (!cmd)
	return false;

    if (mode)
	*mode = *mode & ~cmd->opt | cmd->id;
    return true;
}

///////////////////////////////////////////////////////////////////////////////

int ScanOptPatchImage ( ccp arg )
{
    ccp eq = strchr(arg,'=');
    if (!eq)
    {
	ERROR0(ERR_SYNTAX,"Missing equal sign: %s\n",arg);
	return 1;
    }

    int stat = 0;
    if ( eq > arg )
    {
	char keyword[1000];
	StringCopyS(keyword,sizeof(keyword),arg);
	if ( eq-arg < sizeof(keyword) )
	    keyword[eq-arg] = 0;

	stat = ScanKeywordList(keyword,cmdtab_patch_image,0,true,0,0,
				"Option --patch",ERR_SYNTAX);
	if ( stat == -1 )
	    return 1;
    }

    PRINT("ADD PATCH: %02x %s\n",stat,eq+1);
    FormatFieldItem_t * ffi = AppendFormatField(&patch_image_list,eq+1,false,false);
    DASSERT(ffi);
    ffi->patch_mode = stat;

    return 0;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			SZS file list			///////////////
///////////////////////////////////////////////////////////////////////////////

int IterateTrackFiles
(
    FileList_t	mode,		// bit field with requested files
    IterateTrackFilesFunc
		func,		// call back function
    void	*param		// user defined parameter
)
{
    int count = 0;
    char idbuf[10], fbuf[50];

    if ( mode & FLT_TRACK )
    {
	int i;
	for ( i = 0; i < MKW_N_TRACKS; i++ )
	{
	    const TrackInfo_t *ti = track_info + track_pos[i];
	    if ( mode & FLT_STD )
	    {
		snprintf(idbuf,sizeof(idbuf),"T%u%u",i/4+1,i%4+1);
		int stat = func(ti->track_fname,idbuf,FLT_TRACK|FLT_STD,ti,param);
		if ( stat < 0 )
		    return stat;
		count += stat;
	    }
	    if ( mode & FLT_D )
	    {
		snprintf(idbuf,sizeof(idbuf),"T%u%ud",i/4+1,i%4+1);
		StringCat2S(fbuf,sizeof(fbuf),ti->track_fname,"_d");
		int stat = func(fbuf,idbuf,FLT_TRACK|FLT_D,ti,param);
		if ( stat < 0 )
		    return stat;
		count += stat;
	    }
	}
    }

    if ( mode & FLT_ARENA )
    {
	int i;
	for ( i = 0; i < MKW_N_ARENAS; i++ )
	{
	    const TrackInfo_t *ti = arena_info + arena_pos[i];
	    if ( mode & FLT_STD )
	    {
		snprintf(idbuf,sizeof(idbuf),"A%u%u",i/5+1,i%5+1);
		int stat = func(ti->track_fname,idbuf,FLT_ARENA|FLT_STD,ti,param);
		if ( stat < 0 )
		    return stat;
		count += stat;
	    }
	    if ( mode & FLT_D )
	    {
		snprintf(idbuf,sizeof(idbuf),"A%u%ud",i/5+1,i%5+1);
		StringCat2S(fbuf,sizeof(fbuf),ti->track_fname,"_d");
		int stat = func(fbuf,idbuf,FLT_ARENA|FLT_D,ti,param);
		if ( stat < 0 )
		    return stat;
		count += stat;
	    }
	}
    }

    if ( mode & (FLT_OTHER|FLT_OLD) )
    {
	struct tab_t { bool have_d; uint mode; char id[4]; ccp fname; };
	static const struct tab_t tab[] =
	{
	    { 1, FLT_OTHER, "WIN", "winningrun_demo" },
	    { 1, FLT_OTHER, "LOS", "loser_demo" },
	    { 1, FLT_OTHER, "END", "ending_demo" },
	    { 1, FLT_OTHER, "MAR", "old_mario_gc_b" },
	    { 1, FLT_OTHER, "RNG", "ring_mission" },

	    { 1, FLT_OLD,   "DRW", "draw_demo" },
	    { 0, FLT_OLD,   "HAY", "old_mario_gc_hayasi" },
	    { 0, FLT_OLD,   "NAR", "old_mario_gc_narita" },
	    { 0, FLT_OLD,   "YAB", "old_mario_gc_yabuki" },

	    {0,0,"",0}
	};

	const struct tab_t *ptr;
	for ( ptr = tab; ptr->fname; ptr++ )
	{
	    if (!(mode&ptr->mode))
		continue;

	    if ( mode & FLT_STD )
	    {
		int stat = func(ptr->fname,ptr->id,ptr->mode|FLT_STD,0,param);
		if ( stat < 0 )
		    return stat;
		count += stat;
	    }
	    if ( mode & FLT_D && ptr->have_d )
	    {
		snprintf(idbuf,sizeof(idbuf),"%sd",ptr->id);
		StringCat2S(fbuf,sizeof(fbuf),ptr->fname,"_d");
		int stat = func(fbuf,idbuf,ptr->mode|FLT_D,0,param);
		if ( stat < 0 )
		    return stat;
		count += stat;
	    }
	}
    }

    return count;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			IsValid()			///////////////
///////////////////////////////////////////////////////////////////////////////

ccp valid_text[VALID__N] =
{
    "ok+",	// VALID_OK
    "ok",	// VALID_NO_TEST
    "warn",	// VALID_WARNING
    "fail",	// VALID_ERROR
    "type",	// VALID_WRONG_FF
    "-",	// VALID_UNKNOWN_FF
    "-",	// VALID_UNKNOWN
};

///////////////////////////////////////////////////////////////////////////////

valid_t IsValidSZS
(
    struct szs_file_t	*szs,		// valid szs archive
    bool		warnings	// true: print warnings
)
{
    DASSERT(szs);
    return IsValid( szs->data, szs->size, szs->file_size,
			szs, szs->fform_current, warnings ? szs->fname : 0 );
}

///////////////////////////////////////////////////////////////////////////////

valid_t IsValid
(
    const void		*data,		// valid data to test
    uint		data_size,	// size of 'data'
    uint		file_size,	// not NULL: size of complete file
    szs_file_t		*szs,		// not NULL: base archive
    file_format_t	fform,		// known file format
					// if FF_UNKNOWN: call GetByMagicFF()
    ccp			fname		// not NULL: print warnings with file ref
)
{
    DASSERT( !szs || data_size <= szs->size );

// [[analyse-magic]]
    file_format_t real_fform = GetByMagicFF(data,data_size,file_size);
    if ( fform == FF_UNKNOWN )
	fform = real_fform;
    else if ( fform == FF_DIRECTORY )
	return VALID_UNKNOWN_FF;
    else if ( fform != real_fform )
	return VALID_WRONG_FF;

    switch (fform)
    {
	case FF_UNKNOWN:
	    return VALID_UNKNOWN_FF;

	case FF_BTI:
	    return IsValidBTI(data,data_size,file_size,fname);

	case FF_KCL:
	    return IsValidKCL(0,data,data_size,file_size,fname);

	case FF_KMP:
	    return IsValidKMP(data,data_size,file_size,fname);

	case FF_LEX:
	    return IsValidLEX(data,data_size,file_size,fname);

	case FF_MDL:
	    return IsValidMDL(data,data_size,file_size,szs,fname,0);

	case FF_PAT:
	    return IsValidPAT(data,data_size,file_size,szs,fname,0,0);

	case FF_STATICR:
	    return IsValidSTATICR(data,data_size,file_size,fname);

	case FF_TEX_CT:
	    return IsValidTEXCT(data,data_size,file_size,szs,fname);

	case FF_CT1_DATA:
	    return IsValidCTCODE(data,data_size,file_size,fname);

	default:
	    break;
    }

    const ff_attrib_t fa = GetAttribFF(fform);
    if ( fa & FFT_BRSUB )
	return IsValidBRSUB(data,data_size,file_size,szs,fform,true,fname,0);

    if ( fa & FFT_TEXT )
	return VALID_OK;

    return VALID_NO_TEST;
}

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
)
{
    DASSERT( !szs || szs->file_size >= szs->size );

    if ( !data || data_size < sizeof(brsub_header_t) )
	return VALID_WRONG_FF;

    if (!endian)
	endian = szs ? szs->endian : &be_func;
    DASSERT(endian);

    if ( fform == FF_UNKNOWN || check_fform )
    {
// [[analyse-magic]]
	file_format_t real_fform = GetByMagicFF(data,data_size,file_size);
	if ( fform == FF_UNKNOWN )
	    fform = real_fform;
	else if ( fform != real_fform )
	    return VALID_WRONG_FF;
    }

    const brsub_header_t * bh	= data;
    const u32  size		= endian->rd32(&bh->size);

    if ( data_size < sizeof(*bh)
		//|| data_size < size
		|| file_size && file_size < size )
    {
	PRINT("INVALID! data_size=%x, size=%x, version=%u\n",
		data_size, size, endian->rd32(&bh->version) );
	return VALID_ERROR;
    }

    if (szs)
    {
	if (!szs->data)
	{
	    PRINT("INVALID BRSUB: no data\n");
	    return VALID_ERROR;
	}

	if ( (u8*)data + data_size > szs->data + szs->file_size )
	{
	    PRINT("INVALID BRSUB: invalid data size: %zx > %zx\n",
			(u8*)data + data_size - szs->data, szs->file_size );
	    return VALID_ERROR;
	}

	const u32 brres_offset = endian->rd32(&bh->brres_offset);
	if ( brres_offset != (u32)( szs->data - (u8*)data )
	    && ( !szs->parent || brres_offset != (u32)( szs->parent->data - (u8*)data ) ))
	{
	    PRINT("INVALID BRSUB: Invalid back offset: %zx != %x\n",
			szs->data - (u8*)data,
			endian->rd32(&bh->brres_offset) );
	 #ifdef TEST // [[2do]]
	    ERROR0(ERR_WARNING,
		"Invalid subfile back reference to BRRES archive file: Offset %x @%s\n",
		(u32)( (u8*)data - szs->data ), szs->fname );
	 #else
	    return VALID_ERROR;
	 #endif
	}
    }
    else
    {
	const u32 off = endian->rd32(&bh->brres_offset);
	if ( off && off < 0xc0000000 ) // 1GB limit
	{
	    PRINT("INVALID BRSUB: limit %u\n",off);
	    return VALID_ERROR;
	}
    }

    noPRINT("BRSUB: %d.%d.%d.%d\n",
		isalnum( (int)(bh->magic[0]) ),
		isalnum( (int)(bh->magic[1]) ),
		isalnum( (int)(bh->magic[2]) ),
		isalnum( (int)(bh->magic[3]) ) );

    return isalnum( (int)(bh->magic[0]) )
	&& isalnum( (int)(bh->magic[1]) )
	&& isalnum( (int)(bh->magic[2]) )
	&& isalnum( (int)(bh->magic[3]) )
		? VALID_OK : VALID_ERROR;
}

///////////////////////////////////////////////////////////////////////////////

valid_t IsValidBTI
(
    const void		* data,		// data
    uint		data_size,	// size of 'data'
    uint		file_size,	// NULL or size of complete file
    ccp			fname		// not NULL: print warnings with file ref
)
{
    //--- First make a plausibility checks and return VALID_WRONG_FF on fail.
    //--- This phase is done because of missing a magic

    if ( !data || data_size < sizeof(bti_header_t) )
	return VALID_WRONG_FF;

    const bti_header_t *bti = (bti_header_t*)data;
    u32 data_off = be32(&bti->data_off);
    if ( !bti->n_image || data_off < sizeof(bti_header_t) || data_off & 3 )
	return VALID_WRONG_FF;

    const image_format_t	iform	= bti->iform;
    const ImageGeometry_t	*geo	= GetImageGeometry(iform);
    if ( !geo || geo->is_x )
	return VALID_WRONG_FF;

    const u32			pal_off	= be32(&bti->pal_off);
    const palette_format_t	pform	= be16(&bti->pform);
    const uint			n_pal	= be16(&bti->n_pal);
    if ( geo->max_palette > 0 )
    {
	if ( pal_off < sizeof(bti_header_t) || pal_off & 3 )
	    return VALID_WRONG_FF;
	if (PaletteToImageFormat(pform,IMG_X_RGB) == IMG_X_RGB )
	    return VALID_WRONG_FF;
	const uint max_pal = GetPaletteCountIF(iform);
	if ( !n_pal || n_pal > max_pal )
	    return VALID_WRONG_FF;
    }
    else
    {
	if ( pform || n_pal || pal_off )
	    return VALID_WRONG_FF;
    }

    const uint width  = be16(&bti->width);
    const uint height = be16(&bti->height);
    if ( !width || !height )
	return VALID_WRONG_FF;

    const uint image_size = geo->bits_per_pixel * width * height / 8;
    if ( data_off + image_size > data_size )
	return VALID_WRONG_FF;


    //--- Now we think it's a BTI => make content checks

    if (file_size)
    {
	// [[2do]]
	// return VALID_ERROR;
	// return VALID_OK;
    }

    return VALID_NO_TEST; // [[2do]] or VALID_OK?
}

///////////////////////////////////////////////////////////////////////////////

valid_t IsValidTEXCT
(
    const void		* data,		// data
    uint		data_size,	// size of 'data'
    uint		file_size,	// NULL or size of complete file
    szs_file_t		*szs,		// not NULL: base archive
    ccp			fname		// not NULL: print warnings with file ref
)
{
    const valid_t brsub_valid
	= IsValidBRSUB(data,data_size,file_size,szs,FF_TEX,false,fname,0);
    if ( brsub_valid >= VALID_ERROR )
	return brsub_valid;

    const brsub_header_t *bh = (brsub_header_t*)data;
    if ( be32(&bh->version) != 3 )
	return VALID_WRONG_FF;

    const u32 img_off = be32(bh->grp_offset);
    if ( img_off <= CT_CODE_TEX_OFFSET || file_size && img_off >= file_size )
	return VALID_WRONG_FF;

    return IsValidCTCODE( data + CT_CODE_TEX_OFFSET,
			  img_off - CT_CODE_TEX_OFFSET,
			  file_size ? file_size - CT_CODE_TEX_OFFSET : 0,
			  fname );
}

///////////////////////////////////////////////////////////////////////////////

valid_t IsValidCTCODE
(
    const void		* data,		// data
    uint		data_size,	// size of 'data'
    uint		file_size,	// NULL or size of complete file
    ccp			fname		// not NULL: print warnings with file ref
)
{
    if ( data_size < sizeof(ctcode_header_t) )
	return VALID_WRONG_FF;

    const ctcode_header_t *ch = (ctcode_header_t*)data;
    const uint n_sect = be32(&ch->n_sect);
    const u32 min_data_off = sizeof(ctcode_header_t) + n_sect * sizeof(ctcode_sect_info_t);
    if	(  n_sect < 2
	|| data_size < min_data_off
	|| be32(&ch->magic) != CT1_DATA_MAGIC_NUM
	|| memcmp(ch->sect_info[0].name,"CUP1",4)
	|| memcmp(ch->sect_info[1].name,"CRS1",4)
	)
    {
	return VALID_WRONG_FF;
    }

    // ??? [[2do]] [[ctcode]]
    return VALID_NO_TEST;
}

///////////////////////////////////////////////////////////////////////////////

valid_t IsValidKCL
(
    kcl_analyze_t	* ka,		// not NULL: init and store stats
    const void		* data,		// data
    uint		data_size,	// size of 'data'
    uint		file_size,	// NULL or size of complete file
    ccp			fname		// not NULL: print warnings with file ref
)
{
    kcl_analyze_t local_ka;
    if (!ka)
	ka = &local_ka;
    memset(ka,0,sizeof(*ka));

    noPRINT("IsValidKCL(%u,%u,%s)\n",data_size,file_size,fname);
    if ( !data || data_size < sizeof(kcl_head_t) )
	return ka->valid = VALID_WRONG_FF;

    ka->head_size = sizeof(kcl_head_t);


    //--- setup memory map

    MemMap_t mm;
    InitializeMemMap(&mm);
    if (file_size)
    {
	ka->file_size = file_size;
	MemMapItem_t *mi = InsertMemMap(&mm,file_size,0);
	mi->index = N_KCL_SECT;
	StringCopyS(mi->info,sizeof(mi->info),"End of file");
    }

    uint sect;
    const kcl_head_t * kcl = data;

    for ( sect = 0; sect < N_KCL_SECT; sect++ )
    {
	u32 offset = be32(kcl->sect_off+sect);
	if ( sect == 2 )
	    offset += 0x10;

	if ( offset == sizeof(kcl_head_t) - 4 )
	    ka->head_size = offset;

	ka->off[sect] = offset;
	if ( offset & 3
		|| offset < sizeof(kcl_head_t) - 4 // unknown_0x38 is optional
		|| file_size && offset > file_size
	   )
	{
	    noPRINT("INVALID KCL: sect=%d, off=%x, headsize=%zx, filesize=%x\n",
			sect, offset, sizeof(kcl_head_t), file_size );
	    return ka->valid = VALID_ERROR;
	}

	MemMapItem_t *mi = InsertMemMap(&mm,offset,0);
	DASSERT(mi);
	mi->index = sect;
	snprintf(mi->info,sizeof(mi->info),"Section #%u",sect+1);
    }


    //--- calculate section sizes

    static const uint elem_size[N_KCL_SECT]
	= { sizeof(float3), sizeof(float3), sizeof(kcl_triangle_t) };

    uint i, err = 0, order = 0;
    for ( i = 0; i < mm.used - 1; i++ )
    {
	MemMapItem_t *p1 = mm.field[i];
	MemMapItem_t *p2 = mm.field[i+1];
	p1->size = p2->off - p1->off;
	order = order * 10 + p1->index + 1;

	const uint esize = elem_size[p1->index];
	if (esize)
	{
	    const uint n = p1->size / esize;
	    ka->n[p1->index] = n;
	    if ( p1->size != n * esize )
	    {
		p1->size = n * esize;
		err++;
	    }
	}
	ka->size[p1->index] = p1->size;
    }
    ka->order_value = order;
    ka->order_ok = order == 1234;


    //--- logging

    if ( logging >= 2 )
    {
	fprintf(stdlog,"\nMemory map of KCL sections: %s\n", fname ? fname : "" );
	PrintMemMap(&mm,stdlog,3,"info");
	fputc('\n',stdlog);
    }

    // [[2do]] [[kcl]] more tests?

    noPRINT_IF(err,"INVALID KCL: %s\n",GetValidInfoKCL(ka));
    return ka->valid = err ? VALID_ERROR : VALID_OK;
}

//-----------------------------------------------------------------------------

ccp GetValidInfoKCL ( kcl_analyze_t * ka )
{
    if (!ka)
	return 0;

    char order[20];
    if (!ka->order_ok)
	snprintf(order,sizeof(order),"order=%u / ",ka->order_value);
    else
	*order = 0;

    char buf[150];
    uint len = snprintfS(buf,sizeof(buf),
	"%ssect=%x+%x, %x+%x, %x+%x, %x+%x, %x / N=%d,%d,%d,%d",
	order,
	ka->off[0], ka->size[0],
	ka->off[1], ka->size[1],
	ka->off[2], ka->size[2],
	ka->off[3], ka->size[3],
	ka->file_size,
	ka->n[0], ka->n[1], ka->n[2], ka->n[3] ) + 1;

    char *res = GetCircBuf(len);
    memcpy(res,buf,len);
    return res;
}

///////////////////////////////////////////////////////////////////////////////

void ScanHeadInfoKMP ( kmp_head_info_t *hi, cvp data, uint data_size )
{
    DASSERT(hi);
    DASSERT(data||!data_size);

    memset(hi,0,sizeof(*hi));
    hi->data		= data;
    hi->end		= hi->data + data_size;
    hi->data_size	= data_size;

    const kmp_file_gen_t * kfile = data;
    hi->file_size	= ntohl(kfile->file_size);
    hi->max_size	= hi->file_size < data_size ? hi->file_size : data_size;

    hi->head_size	= ntohs(kfile->head_size);
    hi->max_off		= data_size - hi->head_size - 4;
    hi->n_sect		= ntohs(kfile->n_sect);

    const uint max_sect = ( hi->head_size - 0x10 ) / 4;
    if ( hi->n_sect > max_sect )
	 hi->n_sect = 0;
    hi->sect_off = (u32*) ((u8*)data + hi->head_size - 4*hi->n_sect);
}

//-----------------------------------------------------------------------------

valid_t IsValidKMP
(
    const void		* data,		// data
    uint		data_size,	// size of 'data'
    uint		file_size,	// NULL or size of complete file
    ccp			fname		// not NULL: print warnings with file ref
)
{
    if ( !data || data_size < sizeof(kmp_file_gen_t) )
	return VALID_WRONG_FF;

    kmp_head_info_t hi;
    ScanHeadInfoKMP(&hi,data,data_size);
    if (memcmp(data,"RKMD",4))
	return VALID_WRONG_FF;

    PRINT("M=%.4s n-sect=%d, h-size=%u,%u,%zu  f-size=%u,%u\n",
		(ccp)data, hi.n_sect,
		hi.head_size, GetFileHeadSizeKMP(hi.n_sect), sizeof(kmp_file_mkw_t),
		hi.file_size, file_size );

    if ( force_kmp && hi.file_size < file_size )
	hi.file_size = file_size;
    if ( hi.head_size < GetFileHeadSizeKMP(hi.n_sect) || hi.head_size >= hi.file_size )
	return VALID_ERROR;

    valid_t stat = VALID_OK;
    if ( file_size && hi.file_size > file_size )
    {
	if (!force_kmp)
	    return VALID_ERROR;

	stat = VALID_WARNING;
	if (fname)
	    ERROR0(ERR_WARNING,
		"KMP header declares file size as %u bytes, but it has only %u bytes: %s",
		hi.file_size, file_size, fname );
	hi.file_size = file_size;

	kmp_file_gen_t *kfile = (kmp_file_gen_t*)data;
	write_be32(&kfile->file_size,hi.file_size);
    }

    const uint max_off = hi.file_size - hi.head_size;
    uint warn_count = 0, sect;
    for ( sect = 0; sect < hi.n_sect; sect++ )
    {
	const u32 off = ntohl(hi.sect_off[sect]);
	noPRINT("OFF #%02u: %4x/%4x/%4x\n", sect, off, max_off, file_size );
	if ( off >= max_off || file_size && off > file_size )
	    warn_count++;
    }

    if ( warn_count > 0 && fname && *fname )
	ERROR0(ERR_WARNING,"KMP: %u segment%s behind end of file marker: %s\n",
		warn_count, warn_count == 1 ? "" : "s", fname );

    return warn_count ? VALID_WARNING : stat;
}

///////////////////////////////////////////////////////////////////////////////

valid_t IsValidLEX
(
    const void		* data,		// data
    uint		data_size,	// size of 'data'
    uint		file_size,	// NULL or size of complete file
    ccp			fname		// not NULL: print warnings with file ref
)
{
    if ( !data || data_size < sizeof(lex_header_t) )
	return VALID_WRONG_FF;

    const lex_header_t * lex = data;
    if ( memcmp(lex->magic,"LE-X",sizeof(lex->magic)))
	return VALID_WRONG_FF;

    // [[todo]] [[lex]] ???
    return VALID_OK;
}

///////////////////////////////////////////////////////////////////////////////

valid_t IsValidMDL
(
    const void		* data,		// data
    uint		data_size,	// size of 'data'
    uint		file_size,	// NULL or size of complete file
    struct szs_file_t	*szs,		// not NULL: base archive
    ccp			fname,		// not NULL: print warnings with file ref
    const endian_func_t	*endian		// not NULL: use this endian functions
)
{
    valid_t brsub_valid
	= IsValidBRSUB(data,data_size,file_size,szs,FF_MDL,true,fname,endian);
    if ( brsub_valid >= VALID_ERROR )
	return brsub_valid;

    // [[2do]] [[mdl]] ???
    return brsub_valid;
}

///////////////////////////////////////////////////////////////////////////////

valid_t IsValidPAT
(
    const void		* data,		// data
    uint		data_size,	// size of 'data'
    uint		file_size,	// NULL or size of complete file
    struct szs_file_t	*szs,		// not NULL: base archive
    ccp			fname,		// not NULL: print warnings with file ref
    const endian_func_t	*endian,	// not NULL: use this endian functions
    struct pat_analyse_t *ana		// not NULL: store analysis data here
)
{
    pat_analyse_t ana0;
    if (!ana)
	ana = &ana0;
    memset(ana,0,sizeof(*ana));
    ana->data = data;
    ana->data_size = data_size;

    ana->valid = IsValidBRSUB(data,data_size,file_size,szs,FF_PAT,true,fname,endian);
    if ( ana->valid >= VALID_ERROR )
	return ana->valid;


    //--- start with PAT analysis

    //HEXDUMP16(0,0,data,data_size);

    if (!file_size)
	file_size = szs ? szs->size : data_size;


    //--- check header

    if ( file_size < PAT_HEAD_OFFSET + sizeof(pat_head_t) )
	return ana->valid = VALID_ERROR;
    if ( data_size < PAT_HEAD_OFFSET + sizeof(pat_head_t) )
	return ana->valid = VALID_WARNING;

    ana->head  = (pat_head_t*)(data + PAT_HEAD_OFFSET);
    ana->n_sect0 = be16(&ana->head->n_sect0);
    ana->n_sect1 = be16(&ana->head->n_sect1);
    ana->data_complete = true;


    //--- check existence of sections 1 and 3

    const brsub_header_t *bh = data;
    u32 off = be32(bh->grp_offset+1);
    u32 end_off = off + 4*ana->n_sect1;
    if (!off)
	ana->data_complete = false;
    else if ( end_off > file_size )
    {
	ana->data_complete = false;
	ana->valid = VALID_ERROR;
    }
    else if ( end_off <= data_size )
	ana->s1_list = (u32*)(ana->data + off);

    off = be32(bh->grp_offset+3);
    end_off = off + 4*ana->n_sect1;
    if (!off)
	ana->data_complete = false;
    else if ( end_off > file_size )
    {
	ana->data_complete = false;
	ana->valid = VALID_ERROR;
    }
    else if ( end_off <= data_size )
	ana->s3_list = (u32*)(ana->data + off);


    //--- now analyse section 0

    off = be32(bh->grp_offset+0);
    if ( off + sizeof(pat_s0_bhead_t) > data_size )
    {
     invalid:
	ana->data_complete = false;
	ana->valid = VALID_ERROR;
    }
    else
    {
	pat_s0_bhead_t *bhead = (pat_s0_bhead_t*)(ana->data + off);
	u32 bhead_end = off + be32(&bhead->size);
	if ( bhead_end > data_size )
	    goto invalid;
	ana->s0_base = bhead;
	if ( be16(&bhead->n_elem) != ana->n_sect0 )
	{
	    PRINT("N-SECT: %d %d %d\n",
		ana->n_sect0, be16(&bhead->n_elem), be16(&bhead->n_unknown) );
	    ana->valid = VALID_WARNING;
	}

	uint i;
	for ( i = 0; i < ana->n_sect0; i++ )
	{
	    pat_s0_belem_t *belem = bhead->elem + i;
	    const u32 ref_off = off + be32(&belem->offset_strref);
	    if ( ref_off + sizeof(pat_s0_sref_t) > data_size )
		goto invalid;

	    pat_s0_sref_t *sref = (pat_s0_sref_t*)( ana->data + ref_off );
	    const u32 str_off = ref_off + be32(&sref->offset_strlist);
	    if ( str_off + 4 > data_size )
		goto invalid;

	    pat_s0_shead_t *shead = (pat_s0_shead_t*)( ana->data + str_off );
	    uint n_slist = be16(&shead->n_elem);
	    if ( str_off + 4 * (n_slist+1) > data_size )
		goto invalid;

	    if ( i < PAT_MAX_ELEM )
	    {
		ana->n_s0_list[i] = n_slist;
		ana->s0_sref[i]   = sref;
		ana->s0_shead[i]  = shead;
	    }
	}
    }

    //--- 'sref' order

    const uint max = ana->n_sect0 < PAT_MAX_ELEM ? ana->n_sect0 : PAT_MAX_ELEM;
    pat_s0_sref_t *last_sr = 0;
    u8 *dest_sr = ana->s0_sref_order;
    for(;;)
    {
	uint i;
	pat_s0_sref_t *found_sr = 0, **sr = ana->s0_sref;
	for ( i = 0; i < max; i++, sr++ )
	    if ( *sr > last_sr && ( !found_sr || *sr < found_sr ))
		found_sr = *sr;
	if (!found_sr)
	    break;

	sr = ana->s0_sref;
	for ( i = 0; i < max; i++, sr++ )
	    if ( *sr == found_sr )
		*dest_sr++ = i;
	last_sr = found_sr;
    }


    //--- 's0_shead' order

    pat_s0_shead_t *last_sh = 0;
    u8 *dest_sh = ana->s0_shead_order;
    for(;;)
    {
	uint i;
	pat_s0_shead_t *found_sh = 0, **sh = ana->s0_shead;
	for ( i = 0; i < max; i++, sh++ )
	    if ( *sh > last_sh && ( !found_sh || *sh < found_sh ))
		found_sh = *sh;
	if (!found_sh)
	    break;

	sh = ana->s0_shead;
	for ( i = 0; i < max; i++, sh++ )
	    if ( *sh == found_sh )
		*dest_sh++ = i;
	last_sh = found_sh;
    }


    //--- terminate

    PRINT("ORDER:\n");
    HEXDUMP16(10,0,ana->s0_sref_order,sizeof(ana->s0_sref_order));
    HEXDUMP16(10,0,ana->s0_shead_order,sizeof(ana->s0_shead_order));

    DASSERT( PAT_MAX_ELEM >= 5 );
    PRINT("ANA(PAT): valid=%d,%d, N=%d,%d,[%d,%d,%d,%d,%d], "
		" ptr=%zx,%zx,[%zx,%zx,%zx,%zx,%zx],[%zx,%zx,%zx,%zx,%zx],%zx,%zx\n",
	ana->valid, ana->data_complete,
	ana->n_sect0, ana->n_sect1,

	ana->n_s0_list[0],
	ana->n_s0_list[1],
	ana->n_s0_list[2],
	ana->n_s0_list[3],
	ana->n_s0_list[4],

	ana->head	? (u8*)ana->head	- ana->data : 0,
	ana->s0_base	? (u8*)ana->s0_base	- ana->data : 0,

	ana->s0_sref[0]	? (u8*)ana->s0_sref[0]	- ana->data : 0,
	ana->s0_sref[1]	? (u8*)ana->s0_sref[1]	- ana->data : 0,
	ana->s0_sref[2]	? (u8*)ana->s0_sref[2]	- ana->data : 0,
	ana->s0_sref[3]	? (u8*)ana->s0_sref[3]	- ana->data : 0,
	ana->s0_sref[4]	? (u8*)ana->s0_sref[4]	- ana->data : 0,

	ana->s0_shead[0]? (u8*)ana->s0_shead[0]	- ana->data : 0,
	ana->s0_shead[1]? (u8*)ana->s0_shead[1]	- ana->data : 0,
	ana->s0_shead[2]? (u8*)ana->s0_shead[2]	- ana->data : 0,
	ana->s0_shead[3]? (u8*)ana->s0_shead[3]	- ana->data : 0,
	ana->s0_shead[4]? (u8*)ana->s0_shead[4]	- ana->data : 0,

	ana->s1_list	? (u8*)ana->s1_list	- ana->data : 0,
	ana->s3_list	? (u8*)ana->s3_list	- ana->data : 0 );

    return ana->valid;
}

///////////////////////////////////////////////////////////////////////////////

valid_t IsValidSTATICR
(
    const void		* data,		// data
    uint		data_size,	// size of 'data'
    uint		file_size,	// NULL or size of complete file
    ccp			fname		// not NULL: print warnings with file ref
)
{
    //--- First make a plausibility checks and return VALID_WRONG_FF on fail.
    //--- This phase is done because of missing a magic

    if ( !data || data_size < 0x100 )
	return VALID_WRONG_FF;

    static const u8 identification[] =
    {
	 /*    80 */  0x00, 0x00, 0x78, 0xb0,  0x00, 0x00, 0x00, 0x00,
	 /*    88 */  0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00,
	 /*    90 */  0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00,
	 /*    98 */  0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00,
	 /*    a0 */  0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00,
	 /*    a8 */  0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00,
	 /*    b0 */  0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00,
	 /*    b8 */  0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00,
	 /*    c0 */  0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00,
	 /*    c8 */  0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00,
	 /*    d0 */  0x00, 0x00, 0x00, 0x00,  0x80, 0xa3, 0x00, 0x04,
	 /*    d8 */  0x80, 0x04, 0x00, 0x04,  0x7c, 0x65, 0x00, 0x50,
	 /*    e0 */  0x7c, 0x00, 0x28, 0x50,  0x7c, 0x60, 0x03, 0x78,
	 /*    e8 */  0x54, 0x03, 0x0f, 0xfe,  0x4e, 0x80, 0x00, 0x20,
	 /*    f0 */  0x94, 0x21, 0xff, 0xf0,  0x7c, 0x08, 0x02, 0xa6,
	 /*    f8 */  0x2c, 0x03, 0x00, 0x00,  0x90, 0x01, 0x00, 0x14,
    };

    if (memcmp(data+0x80,identification,sizeof(identification)))
	return VALID_WRONG_FF;

    // ??? [[2do]]
    return VALID_NO_TEST; // [[2do]] or VALID_OK?
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			xsrc/x-lib			///////////////
///////////////////////////////////////////////////////////////////////////////

#if HAVE_XSRC
 #include "xsrc/x-std.c"
#endif

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////

