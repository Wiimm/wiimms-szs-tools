
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

#include "lib-staticr.h"
#include "db-mkw.h"
#include "lib-mkw.h"
#include "dclib-utf8.h"
#include "dclib-xdump.h"
#include "ui.h" // [[dclib]] wrapper
#include "ui-wstrt.c"

//
///////////////////////////////////////////////////////////////////////////////
///////////////		functions not neded by wstrt		///////////////
///////////////////////////////////////////////////////////////////////////////

#if !SZS_WRAPPER
 bool DefineIntVar ( VarMap_t * vm, ccp varname, int value ) { return false; }
#endif

//
///////////////////////////////////////////////////////////////////////////////
///////////////			definitions			///////////////
///////////////////////////////////////////////////////////////////////////////

#define TITLE WSTRT_SHORT ": " WSTRT_LONG " v" VERSION " r" REVISION \
	" " SYSTEM2 " - " AUTHOR " - " DATE

//
///////////////////////////////////////////////////////////////////////////////

static void help_exit ( bool xmode )
{
    SetupPager();
    fputs( TITLE "\n", stdout );

    if (xmode)
    {
	int cmd;
	for ( cmd = 0; cmd < CMD__N; cmd++ )
	    PrintHelpCmd(&InfoUI_wstrt,stdout,0,cmd,0,0,URI_HOME);
    }
    else
	PrintHelpCmd(&InfoUI_wstrt,stdout,0,0,"HELP",0,URI_HOME);

    ClosePager();
    exit(ERR_OK);
}

///////////////////////////////////////////////////////////////////////////////

static void print_version_section ( bool print_sect_header )
{
    cmd_version_section(print_sect_header,WSTRT_SHORT,WSTRT_LONG,long_count-1);
}

///////////////////////////////////////////////////////////////////////////////

static void version_exit()
{
    if ( brief_count > 1 )
	fputs( VERSION "\n", stdout );
    else if (brief_count)
	fputs( VERSION " r" REVISION " " SYSTEM2 "\n", stdout );
    else if (print_sections)
	print_version_section(true);
    else if (long_count)
	print_version_section(false);
    else
	fputs( TITLE "\n", stdout );

    exit(ERR_OK);
}

///////////////////////////////////////////////////////////////////////////////

static void print_title ( FILE * f )
{
    static bool done = false;
    if (!done)
    {
	done = true;
	if (print_sections)
	    print_version_section(true);
	else if ( verbose >= 1 && f == stdout )
	    fprintf(f,"\n%s\n\n",TITLE);
	else
	    fprintf(f,"*****  %s  *****\n",TITLE);
    }
}

///////////////////////////////////////////////////////////////////////////////

static const KeywordTab_t * current_command = 0;

static void hint_exit ( enumError stat )
{
    if ( current_command )
	fprintf(stderr,
	    "-> Type '%s help %s' (pipe it to a pager like 'less') for more help.\n\n",
	    ProgInfo.progname, CommandInfo[current_command->id].name1 );
    else
	fprintf(stderr,
	    "-> Type '%s -h' or '%s help' (pipe it to a pager like 'less') for more help.\n\n",
	    ProgInfo.progname, ProgInfo.progname );
    exit(stat);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			helpers				///////////////
///////////////////////////////////////////////////////////////////////////////

static int ScanOptU64 ( u64 *num, ccp name, ccp arg )
{
    DASSERT(num);
    DASSERT(name);

    return ERR_OK != ScanSizeOptU64(
			num,			// u64 * num
			arg,			// ccp source
			1,			// default_factor1
			0,			// int force_base
			name,			// ccp opt_name
			0,			// u64 min
			M1(*num),		// u64 max
			1,			// u32 multiple
			0,			// u32 pow2
			true			// bool print_err
			);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command test			///////////////
///////////////////////////////////////////////////////////////////////////////

static void print_vsbt ( enum VsBtMode mode, ccp str, ccp std_str )
{
    switch (mode)
    {
	case VSBT_OFF:
	    break;

	case VSBT_PATCH:
	    printf("%4s%-18c0x%02x 0x%02x = %c%c\n",
			std_str, ':', str[0], str[1], str[0], str[1] );
	    break;

	case VSBT_TEST:
	    printf("%4s%-23cTEST = %c1 %c2 %c3 %c4\n",
			std_str, ':',
			std_str[0], std_str[0], std_str[0], std_str[0] );
	    break;
    }
}

///////////////////////////////////////////////////////////////////////////////

static enumError cmd_test_options()
{
    printf("\nOptions (compatibility: %s; format: hex=dec):\n",PrintOptCompatible());

    printf("  test:        %16x = %12d\n",testmode,testmode);
    printf("  verbose:     %16x = %12d\n",verbose,verbose);
    printf("  width:       %16x = %12d\n",opt_width,opt_width);
    printf("  escape-char: %16x = %12d\n",escape_char,escape_char);
    printf("  all-ranks:   %16x = %12d\n",opt_all_ranks,opt_all_ranks);

    printf("  vs-region:   %16x = %12d  [%c%c]\n",
		opt_str_vs_region, opt_str_vs_region,
		opt_str_xvs_region ? 'x' : '-',
		opt_str_tvs_region ? 't' : '-' );
    printf("  bt-region:   %16x = %12d  [%c%c]\n",
		opt_str_bt_region, opt_str_bt_region,
		opt_str_xbt_region ? 'x' : '-',
		opt_str_tbt_region ? 't' : '-' );

    if (opt_tracks)
	DumpTrackList(0,0,0);
    if (opt_arenas)
	DumpArenaList(0,0,0);

    if (opt_https)
    {
	printf("  https:       %16x = '%s'\n",
		opt_https, GetHttpsName(opt_https,"?") );
	printf("  domain:%14s%s\n","",mkw_domain);
    }

    print_vsbt(patch_vs_mode,patch_vs_str,"vs");
    print_vsbt(patch_bt_mode,patch_bt_str,"bt");

    if (opt_move_d8)
	printf("  move-d8:                 true\n");

    if (n_section_list)
    {
	printf("  create %u sections:\n",n_section_list);
	uint s;
	for ( s = 0; s < n_section_list; s++ )
	{
	    const dol_sect_select_t *sl = section_list + s;
	    printf("\t%s: %#10x %c : %s\n",
		sl->name, sl->addr, sl->use_param ? 'P' : '-', sl->fname );
	}
    }

    return ERR_OK;
}

///////////////////////////////////////////////////////////////////////////////

static enumError cmd_test()
{
 #if 1 || !defined(TEST) // test options

    return cmd_test_options();

 #elif 0

    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	//NORMALIZE_FILENAME_PARAM(param);
    }
    return ERR_OK;

 #endif
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command dump			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_dump ( bool use_c )
{
    enumError max_err = ERR_OK;
    StringField_t plist = {0};
    CollectExpandParam(&plist,first_param,-1,WM__DEFAULT);

    for ( int argi = 0; argi < plist.used; argi++ )
    {
	ccp arg = plist.field[argi];
	staticr_t str;
	enumError err = LoadSTR(&str,true,arg,opt_ignore>0);

	if ( err <= ERR_WARNING && err != ERR_NOT_EXISTS )
	{
	    if ( verbose >= 0 )
		printf("\n%sDUMP of %s:%s\n", use_c ? "C-" : "",
			GetNameFF(str.fform,0), arg );
	    DumpSTR(stdout,2,&str,use_c);
	}

	if ( max_err < err )
	     max_err = err;
	ResetSTR(&str);
    }
    putchar('\n');

    ResetStringField(&plist);
    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command hexdump			///////////////
///////////////////////////////////////////////////////////////////////////////

static XDump_t xparam;
static MemMap_t vaddr = {0};
static MemMap_t faddr = {0};
static uint dol_sections = 0;

///////////////////////////////////////////////////////////////////////////////

static enumError cmd_hexdump()
{
    //--- list addresses of option --vaddr and --faddr

    if ( logging >= 2 )
    {
	if (dol_sections)
	    fprintf(stdlog,"\nSections: %s\n",
			DolSectionList2Text(dol_sections));

	if (vaddr.used)
	{
	    fputs("\nList of virtual addresses (option --vaddr):\n\n",stdlog);
	    PrintMemMap(&vaddr,stdlog,5,0);
	}

	if (faddr.used)
	{
	    fputs("\nList of file addresses (option --faddr):\n\n",stdlog);
	    PrintMemMap(&faddr,stdlog,5,0);
	}
    }

    SetupXDump(&xparam,XDUMPC_DUMP);

    enumError max_err = ERR_OK;
    StringField_t plist = {0};
    CollectExpandParam(&plist,first_param,-1,WM__DEFAULT);

    for ( int argi = 0; argi < plist.used; argi++ )
    {
	ccp arg = plist.field[argi];
	staticr_t str;
	enumError err = LoadSTR(&str,true,arg,opt_ignore>0);

	bool is_ok = err <= ERR_WARNING && err != ERR_NOT_EXISTS;
	if ( is_ok && !str.is_dol )
	{
	    is_ok = false;
	    err = ERROR0(ERR_WRONG_FILE_TYPE,
			"DOL expected, file ignored: %s\n",arg);
	}

	if (is_ok)
	{
	    const dol_header_t *dol = (dol_header_t*)str.data;

	    MemMap_t mm;
	    InitializeMemMap(&mm);
	    if ( vaddr.used || faddr.used || dol_sections )
	    {
		CopyMemMap(&mm,&vaddr,true);
		TranslateDolOffsets(dol,&mm,true,&faddr);
		TranslateDolSections(dol,&mm,true,dol_sections);
	    }
	    else
		TranslateAllDolOffsets(dol,&mm,true);

	    if ( logging >= 2 )
	    {
		fputs("\nList of virtual addresses to dump:\n\n",stdlog);
		PrintMemMap(&mm,stdlog,5,0);
	    }

	    if ( verbose >= 0 )
		printf("\nHEXDUMP of %s:%s\n",
			GetNameFF(str.fform,0), arg );

	    uint mi;
	    for ( mi = 0; mi < mm.used; mi++ )
	    {
		const MemMapItem_t *src = mm.field[mi];
		uint addr = src->off;
		uint size = src->size;
		while ( size > 0 )
		{
		    u32 valid_size;
		    uint off = GetDolOffsetByAddr(dol,addr,size,&valid_size);
		    if ( !off || !valid_size )
			break;
		    xparam.start_addr = addr;
		    XDump(&xparam,str.data+off,valid_size,true);
		    addr += valid_size;
		    size -= valid_size;
		}
	    }
	    ResetMemMap(&mm);
	}

	if ( max_err < err )
	     max_err = err;
	ResetSTR(&str);
    }

    ResetStringField(&plist);
    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////		helpers for PORT and WHERE		///////////////
///////////////////////////////////////////////////////////////////////////////

static void setup_order ( int index[STR_ZBI_N], int delta )
{
    int dest_index = 0;
    char used[STR_ZBI_N] = {0};

    if (opt_order)
    {
	ccp ptr = opt_order;
	while ( *ptr && dest_index < STR_ZBI_N )
	{
	    int idx = -1;
	    switch(*ptr++)
	    {
		case 'p': case 'P': idx = STR_M_PAL - STR_ZBI_FIRST; break;
		case 'e': case 'E':
		case 'u': case 'U': idx = STR_M_USA - STR_ZBI_FIRST; break;
		case 'j': case 'J': idx = STR_M_JAP - STR_ZBI_FIRST; break;
		case 'k': case 'K': idx = STR_M_KOR - STR_ZBI_FIRST; break;
	    }

	    if ( idx >= 0 && !used[idx] )
	    {
		used[idx]++;
		index[dest_index++] = idx + STR_ZBI_FIRST - delta;
	    }
	}
    }

    for ( int i = 0; i < STR_ZBI_N && dest_index < STR_ZBI_N; i++ )
	if (!used[i])
	    index[dest_index++] = i + STR_ZBI_FIRST - delta;

    if ( logging >= 3 )
	fprintf(stdlog,"# Region Order: %d:%s %d:%s %d:%s %d:%s\n",
		index[0], GetStrModeName(index[0]+delta),
		index[1], GetStrModeName(index[1]+delta),
		index[2], GetStrModeName(index[2]+delta),
		index[3], GetStrModeName(index[3]+delta) );
}

///////////////////////////////////////////////////////////////////////////////

typedef struct hex_in_t
{
    u32 scanned;	// scanned number
    u32 num;		// modified number
    u32 cheat;		// if is_cheat: highest 7 bits
    bool is_dol_offset;	// true: prefix 'M' used
    bool is_rel_offset;	// true: prefix 'S' used
    bool is_cheat;	// it's a cheat code, where only lowest 25 bits are relevant
    bool is_6hex;	// hex number with exact 6 hex digits
}
hex_in_t;

//---------------

static hex_in_t scan_hex ( str_mode_t mode, ccp arg )
{
    hex_in_t hex = { .cheat = ~0 };

    hex.is_dol_offset	= *arg == 'm' || *arg == 'M';
    hex.is_rel_offset	= *arg == 's' || *arg == 'S';
    hex.is_cheat	= *arg == '.';
    if  ( hex.is_dol_offset || hex.is_rel_offset || hex.is_cheat )
	arg++;

    char *end;
    hex.scanned = hex.num = str2ul(arg,&end,16);

    if ( hex.is_dol_offset )
	hex.num = GetDolAddrByOffsetM(mode,hex.num,0,0);
    else if ( hex.is_rel_offset )
	hex.num = GetRelAddrByOffsetM(mode,hex.num,0,0);
    else if ( hex.is_cheat )
    {
	hex.cheat = hex.num & 0xfe000000;
	hex.num   = hex.num & 0x01ffffff | 0x80000000;
    }
    else if ( end - arg == 6 && arg[1] != 'x' && arg[1] != 'X' )
    {
	hex.is_6hex = true;
	hex.num |= 0x80000000;
    }

    return hex;
}

///////////////////////////////////////////////////////////////////////////////

static void print_address ( u32 addr, bool highlight, u32 cheat_code )
{
    const bool is_cheat_code = cheat_code != ~0;
    if (is_cheat_code)
	addr = addr & 0x01ffffff | cheat_code;

    ccp col1, col2;
    if (highlight)
    {
	col1 = colout->value;
	col2 = colout->reset;
    }
    else
	col1 = col2 = "";

    if ( opt_upper || is_cheat_code )
    {
	if (opt_no_0x)
	    printf(" %s%08X%s",col1,addr,col2);
	else if (is_cheat_code)
	    printf("   %s%08X%s",col1,addr,col2);
	else
	    printf(" %s%#010X%s",col1,addr,col2);
    }
    else if (opt_no_0x)
	printf(" %s%08x%s",col1,addr,col2);
    else
	printf(" %s%#010x%s",col1,addr,col2);
}

///////////////////////////////////////////////////////////////////////////////

static void print_offset ( str_mode_t mode, u32 addr, bool highlight )
{
    const int aoff = GetOffsetByAddrM(mode,0,addr);

    if ( aoff < 0 )
	printf("      %s-", opt_no_0x ? "": "  " );
    else
    {
	ccp col1, col2;
	if (highlight)
	{
	    col1 = colout->value;
	    col2 = colout->reset;
	}
	else
	    col1 = col2 = "";

	if ( opt_upper )
	{
	    if (opt_no_0x)
		printf(" %s%6X%s",col1,aoff,col2);
	    else
		printf(" %s%#8X%s",col1,aoff,col2);
	}
	else if (opt_no_0x)
	    printf(" %s%6x%s",col1,aoff,col2);
	else
	    printf(" %s%#8x%s",col1,aoff,col2);
    }
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command port			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_port()
{
    if (brief_count)
	print_header = false;

    if ( verbose > 1 )
    {
	SetupAddrPortDB();
	const addr_port_version_t *vers = GetAddrPortVersion();
	printf("\nAddress Porting Database:\n"
		"  File:       %s\n"
		"  Timestamp:  %s\n"
		"  DB version: %5u\n"
		"  Revision:   %5u\n"
		"  N(records): %5u\n"
		"  Size:       %5s\n"
		,addr_port_file
		,PrintTimeByFormat("%F %T %Z",vers->timestamp)
		,vers->db_version
		,vers->revision
		,vers->n_records
		,PrintSize1000(0,0,vers->n_records*sizeof(addr_port_t),0)
		);
    }

    int index[STR_ZBI_N];
    setup_order(index,STR_ZBI_FIRST);

    if (print_header)
	putchar('\n');
    SetupAddrPortDB();


    //--- port addresses

    int max_fw = 0;
    for ( int md = 0; md <= ASM_M_MODE; md++ )
    {
	const int slen = strlen(addr_size_flags_info[md]);
	if ( max_fw < slen )
	     max_fw = slen;
    }

    int sep_len_3 = 3*(56+max_fw);
    if (opt_no_0x)
	sep_len_3 -= 3*4*2; 
    if (long_count)
	sep_len_3 += 3*28 + ( opt_no_0x ? 0 : 3*8 ); 

    ccp extra = opt_no_0x ? "" : "  ";

    int count = 0;    
    str_mode_t mode = STR_M_PAL;
    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	ccp arg = param->arg;
	if ( !arg || !*arg )
	    continue;

	const KeywordTab_t *cmd = ScanKeyword(0,arg,str_mode_keyword_tab);
	if (cmd)
	{
	    mode = cmd->id;
	    continue;
	}

	if ( print_header && !count++ )
	{
	    printf("%s#%.*s\n%s#  %s%s      %s%s      %s%s      %s%s     where",
		colout->heading, sep_len_3, ThinLine300_3,
		colout->heading, 
		extra, GetStrModeName(index[0]+STR_ZBI_FIRST),
		extra, GetStrModeName(index[1]+STR_ZBI_FIRST),
		extra, GetStrModeName(index[2]+STR_ZBI_FIRST),
		extra, GetStrModeName(index[3]+STR_ZBI_FIRST) );

	    if (long_count)
		printf("%s off(%.1s)%s off(%.1s)%s off(%.1s)%s off(%.1s)",
			extra, GetStrModeName(index[0]+STR_ZBI_FIRST),
			extra, GetStrModeName(index[1]+STR_ZBI_FIRST),
			extra, GetStrModeName(index[2]+STR_ZBI_FIRST),
			extra, GetStrModeName(index[3]+STR_ZBI_FIRST) );

	    printf("  status\n%s#%.*s%s\n",
		colout->heading, sep_len_3, ThinLine300_3, colout->reset );
	}

	if (!strcmp(arg,"-"))
	{
	    if (print_header)
		printf("%s#%.*s%s\n", colout->heading, sep_len_3, ThinLine300_3, colout->reset );
	    continue;
	}

	const hex_in_t hex	= scan_hex(mode,arg);
	const addr_type_t atype	= GetAddressType(mode,hex.num);
	const ccp atname	= GetAddressTypeName(atype);
	const u32 mirror	= IsMirroredAddress(hex.num) ? MIRRORED_ADDRESS_DELTA : 0;
	const u32 addr		= hex.num - mirror;
	const addr_port_t *ap	= GetPortingRecord(mode,addr);

	if (!ap)
	{
	    printf("%s! Can't port %s address 0x%08x (%s)!%s\n",
			colout->warn, GetStrModeName(mode), hex.num, atname, colout->reset );
	}
	else
	{
	    const int idx    = mode - STR_ZBI_FIRST;
	    const int offset = addr - ap->addr[idx];

	    for ( int i = 0; i < STR_ZBI_N; i++ )
	    {
		const u32 val = ap->addr[index[i]] + offset + mirror;
		print_address(val,idx==index[i],hex.cheat);
	    }

	    printf("  %s%-5.5s%s",
			GetAddressTypeColor(colout,atype), atname, colout->reset );

	    if (long_count)
		for ( int i = 0; i < STR_ZBI_N; i++ )
		{
		    const int md = index[i];
		    const u32 addr = ap->addr[md] + offset + mirror;
		    print_offset( md+STR_ZBI_FIRST, addr, idx == md );
		}

	    const int asmode = ( offset < ap->size1 ? ap->size1 : ap->size2 ) & ASM_M_MODE;
	    printf("  %s%u: %s%s\n",
			GetAddressSizeColor(colout,asmode),
			asmode, addr_size_flags_info[asmode],
			colout->reset );
	}
    }

    if (print_header)
    {
	if ( count > 0 )
	    printf("%s#%.*s%s\n\n", colout->heading, sep_len_3, ThinLine300_3, colout->reset );
	else
	    putchar('\n');
    }

    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command where			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_where()
{

    if (brief_count)
	print_header = false;

    int index[STR_ZBI_N];
    setup_order(index,0);


    //--- port addresses

    int sep_len_3 = 3*36;
    if (opt_no_0x)
	sep_len_3 -= 3*2; 
    if (long_count)
	sep_len_3 += 3*31 + ( opt_no_0x ? 0 : 3*8 ); 

    int count = 0;    
    str_mode_t mode = STR_M_PAL;
    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	ccp arg = param->arg;
	if ( !arg || !*arg )
	    continue;

	const KeywordTab_t *cmd = ScanKeyword(0,arg,str_mode_keyword_tab);
	if (cmd)
	{
	    mode = cmd->id;
	    continue;
	}

	if ( print_header && !count++ )
	{
	    ccp offset = long_count ? ( opt_no_0x ? " offset :" : "   offset :" ) : " ";
	    printf("\n%s#%.*s\n%s#%s Address   %s %s %s %s %s %s %s %.*s\n%s#%.*s%s\n",
		colout->heading, sep_len_3, ThinLine300_3,
		colout->heading, 
		opt_no_0x ? "" : "  ",
		GetStrModeName(index[0]), offset,
		GetStrModeName(index[1]), offset,
		GetStrModeName(index[2]), offset,
		GetStrModeName(index[3]), opt_no_0x ? 7 : 9, offset,
		colout->heading, sep_len_3, ThinLine300_3, colout->reset );
	}

	if (!strcmp(arg,"-"))
	{
	    if (print_header)
		printf("%s#%.*s%s\n", colout->heading, sep_len_3, ThinLine300_3, colout->reset );
	    continue;
	}

	const hex_in_t hex = scan_hex(mode,arg);
	if (opt_no_0x)
	    printf(" %8x:",hex.num);
	else
	    printf(" %#10x:",hex.num);

	for ( int i = 0; i < STR_ZBI_N; i++ )
	{
	    const int md = index[i];
	    addr_type_t atype = GetAddressType(md,hex.num);
	    printf(" %s %s%-4.4s%s",
		long_count && i > 0 ? ":" : "",
		GetAddressTypeColor(colout,atype),
		GetAddressTypeName(atype),
		colout->reset );

	    if (long_count)
		print_offset( md, hex.num, false );
	}
	putchar('\n');
    }

    if (print_header)
    {
	if ( count > 0 )
	    printf("%s#%.*s%s\n\n", colout->heading, sep_len_3, ThinLine300_3, colout->reset );
	else
	    putchar('\n');
    }

    return ERR_OK;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command https			///////////////
///////////////////////////////////////////////////////////////////////////////

static int compare_server_patch
	( const str_server_patch_t * a, const str_server_patch_t * b )
{
    int stat = (int)a->mode - (int)b->mode;
    if (!stat)
    {
	stat = (int)a->flags - (int)b->flags;
	if (!stat)
	{
	    stat = (int)a->serv_type - (int)b->serv_type;
	    if (!stat)
		stat = (int)a->offset - (int)b->offset;
	}
    }
    return stat;
}

///////////////////////////////////////////////////////////////////////////////

static enumError cmd_https()
{
 #if HAVE_ASSERT
    {
	const str_server_list_t *sl;
	for ( sl = ServerList; sl->name; sl++ )
	    ASSERT_MSG( sl->domain_len == strlen(sl->domain),
			"ServerList[%i], len = %zu\n",
			(int)(sl-ServerList), strlen(sl->domain) );
    }
 #endif

    enumError max_err = ERR_OK;

    enum { LIST_SIZE = 1000 };
    str_server_patch_t patch_list[LIST_SIZE];
    memset(patch_list,0,sizeof(patch_list));
    uint n_list = 0;

    StringField_t plist = {0};
    CollectExpandParam(&plist,first_param,-1,WM__DEFAULT);

    for ( int argi = 0; argi < plist.used; argi++ )
    {
	ccp arg = plist.field[argi];
	staticr_t str;
	enumError err = LoadSTR(&str,true,arg,opt_ignore>0);
	if ( max_err < err )
	     max_err = err;

	if ( err <= ERR_WARNING && err != ERR_NOT_EXISTS )
	{
	    AnalyzeSTR(&str);
	    //printf("ST=%x,%x\n",str.str_status,str.dol_status);
	    if ( !( str.is_dol ? str.dol_status & DOL_S_ORIG : str.str_status & STR_S_ORIG ))
	    {
		printf("\nIGNORE [NO ORIGINAL] %s:%s\n", GetNameFF(str.fform,0), arg );
		goto endloop;
	    }

	    if ( verbose >= 0 )
		printf("SCAN %s:%s\n", GetNameFF(str.fform,0), arg );

	    const u8 *ptr = str.data;
	    const u8 *end = ptr + str.data_size;
	    while ( ptr < end )
	    {
		uint is_http;
		const str_server_list_t *sl;
		uint off;

		const u8 *ptr_h = memchr(ptr,'h',end-ptr);
		const u8 *ptr_g = memchr(ptr,'g',end-ptr);
		if (!ptr_h)
		{
		    if (!ptr_g)
			break;
		    ptr_h = ptr_g + 1;
		}
		else if (!ptr_g)
		    ptr_g = ptr_h + 1;

		if ( ptr_h < ptr_g )
		{
		    ptr = ptr_h;
		    if (!ptr)
			break;

		    if (!memcmp(ptr,"https://",8))
		    {
			off = 8;
			is_http = 2;
		    }
		    else if (!memcmp(ptr,"http://",7))
		    {
			off = 7;
			is_http = 1;
		    }
		    else if (!strcmp((ccp)ptr,"https"))
		    {
			off = 5;
			is_http = 2;
		    }
		    else
		    {
			ptr++;
			continue;
		    }
		    sl = ServerList;
		    ptr += off;
		    if (*ptr)
		    {
			for ( ; sl->name; sl++ )
			    if ( ptr[sl->domain_len] == '/'
					&& !memcmp(ptr,sl->domain,sl->domain_len) )
			    {
				break;
			    }
			if (!sl->name)
			    continue;
		    }
		    else if ( is_http != 2 )
			continue;
		}
		else
		{
		    ptr = ptr_g;
		    if ( memcmp(ptr,"gs.nintendowifi.net",19)
			|| isalnum((int)ptr[19])
			|| isalnum((int)ptr[-1]) )
		    {
			ptr++;
			continue;
		    }

		    is_http = 0;
		    off = 19;
		    ptr += off;
		    sl = ServerList + SERV_GS;
		}

		noPRINT(" -> %s\n",ptr);
		if ( n_list >= LIST_SIZE )
		    return ERROR0(ERR_FATAL,"Patch list full\n");

		str_server_patch_t *pat = patch_list + n_list++;
		pat->mode	= str.mode;
		pat->flags	= str.is_dol ? STR_F_IS_DOL : 0;
		pat->is_http	= is_http;
		pat->serv_type	= sl->serv_type;
		pat->offset	= ptr - off - str.data;
		pat->url_len	= strlen((ccp)ptr) + off;
		if (is_http)
		    ptr += sl->domain_len;
		pat->param = STRDUP((ccp)ptr);
	    }
	}

      endloop:
	ResetSTR(&str);
    }

    ResetStringField(&plist);

    if ( n_list > 1 )
	qsort( patch_list, n_list, sizeof(*patch_list),
			(qsort_func)compare_server_patch );

    if (long_count>0)
    {
	printf("\nconst str_server_patch_t ServerPatch[%u+1] =\n{\n",n_list);
	str_server_patch_t *pat = patch_list;
	uint i, last_mode = pat->mode, last_flags = pat->flags;
	for ( i = 0; i < n_list; i++, pat++ )
	{
	    if ( last_mode != pat->mode || last_flags != pat->flags )
	    {
		last_mode = pat->mode;
		last_flags = pat->flags;
		putchar('\n');
	    }

	    ccp name = ServerList[pat->serv_type].name;
	    printf("  { STR_M_%s,%u, %u,SERV_%s,%*s 0x%06x, %2u, \"%s\" },\n",
		GetStrModeName(pat->mode),
		pat->flags,
		pat->is_http,
		name, (int)(9-strlen(name)), "",
		pat->offset,
		pat->url_len,
		pat->param );
	}
	printf("\n  {0,0,0,0,0,0,0}\n};\n\n");
    }
    else
    {
	static ccp http_text[] = { "  -- ", "HTTP ", "HTTPS" };
	printf("\n==> %u strings found:\n",n_list);
	uint i;
	const str_server_patch_t *pat = patch_list;
	for ( i = 0; i < n_list; i++, pat++ )
	{
	    const int differ = memcmp(pat,ServerPatch+i,sizeof(*pat));
	    printf(" %c %s %s %s %-10s %06x %2u %s\n",
		differ ? '!' : ' ',
		GetStrModeName(pat->mode),
		pat->flags & STR_F_IS_DOL ? "DOL" : "REL",
		http_text[pat->is_http],
		ServerList[pat->serv_type].name, pat->offset, pat->url_len,
		pat->param );
	    if ( differ && verbose > 0 )
	    {
		HexDump(stdout,5,0,0,sizeof(*pat),pat,sizeof(*pat));
		HexDump(stdout,5,0,0,sizeof(*pat),ServerPatch+i,sizeof(*pat));
	    }
	}
    }

    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command extract			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_extract()
{
    if ( opt_dest && !*opt_dest )
	opt_dest = 0;
    opt_mkdir = true;

    enumError max_err = ERR_OK;
    StringField_t plist = {0};
    CollectExpandParam(&plist,first_param,-1,WM__DEFAULT);

    for ( int argi = 0; argi < plist.used; argi++ )
    {
	ccp arg = plist.field[argi];
	staticr_t str;
	enumError err = LoadSTR(&str,true,arg,opt_ignore>0);

	if ( err <= ERR_WARNING && err != ERR_NOT_EXISTS )
	{
	    char dest[PATH_MAX];
	    StringCat2S(dest,sizeof(dest),arg,".d");

	    if ( verbose >= 0 || testmode )
	    {
		fprintf(stdlog,"%s%sEXTRACT %s:%s -> %s\n",
			verbose > 1 ? "\n" : "",
			testmode ? "WOULD " : "",
			GetNameFF(str.fform,0), str.fname, dest );
		fflush(stdlog);
	    }

	    if ( !opt_overwrite && !opt_update )
	    {
		struct stat st;
		if (!stat(dest,&st))
		    return ERROR0(ERR_ALREADY_EXISTS,"Destination already exists: %s",dest);
	    }

	    ExtractSTR(&str,dest);
	}

	if ( max_err < err )
	     max_err = err;
	ResetSTR(&str);
    }

    ResetStringField(&plist);
    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command tracks			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_tracks()
{
    const int mode = brief_count ? 2 : print_header ? 0 : 1;
    if (!n_param)
    {
	DumpTrackList(stdout,0,mode);
	return ERR_OK;
    }

    enumError max_err = ERR_OK;

    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);

	printf("\n* Tracks of %s\n",param->arg);
	staticr_t str;
	enumError err = LoadSTR(&str,true,param->arg,opt_ignore>0);
	if (err)
	{
	    if ( max_err < err )
		 max_err = err;
	    ResetSTR(&str);
	    continue;
	}

	const u32 * offtab = GetTrackOffsetTab(str.mode);
	if (offtab)
	{
	    PatchSTR(&str);

	    const u8 *d = str.data + *offtab;
	    u32 tab[MKW_N_TRACKS];
	    uint i;
	    for ( i = 0; i < MKW_N_TRACKS; i++ )
		tab[i] = be32( d + i*4 ) - MKW_TRACK_BEG;
	    DumpTrackList(stdout,tab,mode);
	}
	ResetSTR(&str);
    }
    putchar('\n');

    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command arenas			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_arenas()
{
    const int mode = brief_count ? 2 : print_header ? 0 : 1;
    if (!n_param)
    {
	DumpArenaList(stdout,0,mode);
	return ERR_OK;
    }

    enumError max_err = ERR_OK;

    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);

	printf("\n* Arenas of %s\n",param->arg);
	staticr_t str;
	enumError err = LoadSTR(&str,true,param->arg,opt_ignore>0);
	if (err)
	{
	    if ( max_err < err )
		 max_err = err;
	    ResetSTR(&str);
	    continue;
	}

	const u32 * offtab = GetArenaOffsetTab(str.mode);
	if (offtab)
	{
	    PatchSTR(&str);

	    const u8 *d = str.data + *offtab;
	    u32 tab[MKW_N_ARENAS];
	    uint i;
	    for ( i = 0; i < MKW_N_ARENAS; i++ )
		tab[i] = be32( d + i*4 ) - MKW_ARENA_BEG;
	    DumpArenaList(stdout,tab,mode);
	}
	ResetSTR(&str);
    }
    putchar('\n');

    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command files			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_files()
{
    if (!n_param)
    {
	DumpTrackFileList(stdout,0,0);
	return ERR_OK;
    }

    enumError max_err = ERR_OK;

    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);

	printf("\n* File list of %s\n",param->arg);
	staticr_t str;
	enumError err = LoadSTR(&str,true,param->arg,opt_ignore>0);
	if (err)
	{
	    if ( max_err < err )
		 max_err = err;
	    ResetSTR(&str);
	    continue;
	}

	const u32 * track_off_tab = GetTrackOffsetTab(str.mode);
	const u32 * arena_off_tab = GetArenaOffsetTab(str.mode);
	if ( track_off_tab && arena_off_tab )
	{
	    PatchSTR(&str);

	    uint i;
	    u32 track_tab[MKW_N_TRACKS], arena_tab[MKW_N_ARENAS];
	    const u8 *d = str.data + *track_off_tab;
	    for ( i = 0; i < MKW_N_TRACKS; i++ )
		track_tab[i] = be32( d + i*4 ) - MKW_TRACK_BEG;

	    d = str.data + *arena_off_tab;
	    for ( i = 0; i < MKW_N_ARENAS; i++ )
		arena_tab[i] = be32( d + i*4 ) - MKW_ARENA_BEG;
	    DumpTrackFileList(stdout,track_tab,arena_tab);
	}
	ResetSTR(&str);
    }

    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command find			///////////////
///////////////////////////////////////////////////////////////////////////////

static void print_found
	( uint idx1, uint idx2, uint n, const TrackInfo_t * info, char prefix )
{
    DASSERT(info);
    DASSERT(prefix);
    if ( long_count > 2 )
	printf("%c%u.%u %2u %2u %s %s %s %s\n", prefix,
		idx1/n+1, idx1%n+1,
		idx1, idx2,
		info->track_fname, info->sound_n_fname, info->sound_f_fname,
		use_de ? info->name_de : info->name_en );
    else if ( long_count > 1 )
	printf("%c%u.%u %s %s %s %s\n", prefix,
		idx1/n+1, idx1%n+1,
		info->track_fname, info->sound_n_fname, info->sound_f_fname,
		use_de ? info->name_de : info->name_en );
    else if ( long_count > 0 )
	printf("%c%u.%u %s\n", prefix,
		info->def_slot/10, info->def_slot%10,
		use_de ? info->name_de : info->name_en );
    else
	printf("%c%u.%u\n", prefix,
		info->def_slot/10, info->def_slot%10 );
}

///////////////////////////////////////////////////////////////////////////////

static bool find_track ( ccp name, bool allow_arenas )
{
    int idx = ScanTrack(name);
    if ( idx >= 0 )
	print_found(track_pos_r[idx],idx,4,track_info+idx,'T');
    else if (allow_arenas)
    {
	idx = ScanArena(name);
	if ( idx >= 0 )
	    print_found(arena_pos_r[idx],idx,5,arena_info+idx,'A');
    }

    if ( idx < 0 )
    {
	printf("-\n");
	return false;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

static bool find_arena ( ccp name, bool allow_tracks )
{
    int idx = ScanArena(name);
    if ( idx >= 0 )
	print_found(arena_pos_r[idx],idx,5,arena_info+idx,'A');
    else if (allow_tracks)
    {
	idx = ScanTrack(name);
	if ( idx >= 0 )
	    print_found(track_pos_r[idx],idx,4,track_info+idx,'T');
    }

    if ( idx < 0 )
    {
	printf("-\n");
	return false;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

static enumError cmd_find()
{
    if (!n_param)
	return ERROR0(ERR_SYNTAX,"Missing sub command for 'FIND'\n");

    enum { SUBCMD_TRACKS, SUBCMD_ARENAS };

    static const KeywordTab_t tab[] =
    {
	{  SUBCMD_TRACKS,  "TRACKS",	0, 0 },
	{  SUBCMD_ARENAS,  "ARENAS",	0, 0 },
	{  SUBCMD_TRACKS,  "+TRACKS",	0, 1 },
	{  SUBCMD_ARENAS,  "+ARENAS",	0, 1 },
	{ 0,0,0,0 }
    };

    const KeywordTab_t * cmd = ScanKeyword(0,first_param->arg,tab);
    if (!cmd)
	return ERROR0(ERR_SYNTAX,
			"Invalid sub command for 'FIND': %s\n",first_param->arg);

    enumError err = ERR_OK;

    DASSERT(first_param);
    ParamList_t *param;
    for ( param = first_param->next; param; param = param->next )
    {
	char name[100];
	ccp arg = NormalizeTrackName(name,sizeof(name),param->arg);
	if ( *arg || !*name )
	{
	    printf("-\n");
	     err = ERR_NOT_EXISTS;
	}
	else
	{
	    bool stat = cmd->id == SUBCMD_TRACKS
			? find_track( name, cmd->opt != 0 )
			: find_arena( name, cmd->opt != 0 );
	    if (!stat)
		err = ERR_NOT_EXISTS;
	}
    }

    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command points			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_points()
{
    if ( OptionUsed[OPT_POINTS] && n_param )
    {
	ERROR0(ERR_WARNING,
		"%u file parameter%s ignored because of --points\n",
		n_param, n_param == 1 ? "" : "s" );
	n_param = 0;
    }

    uint mode  = brief_count ? 2 : 1;
    u32 cheat_base = 0;
    if ( opt_cheat_region != STR_M_UNKNOWN )
    {
	const VersusPointsInfo_t *vpi = GetVersusPointsInfo(opt_cheat_region);
	DASSERT(vpi);
	mode = vpi->region_code;
	cheat_base = vpi->cheat_base;
    }

    if (!n_param)
    {
	const MkwPointInfo_t *mpi = GetMkwPointInfo(MkwPointsTab);
	if ( mode < 'A' )
	    printf("\nVersus points [%s]:\n",mpi->info);
	PrintMkwPoints( stdout, 2, MkwPointsTab, mode, cheat_base, long_count );
	return ERR_OK;
    }

    enumError max_err = ERR_OK;

    ParamList_t *param;
    for ( param = first_param; param; param = param->next )
    {
	NORMALIZE_FILENAME_PARAM(param);

	printf("\nVersus points of %s\n",param->arg);
	staticr_t str;
	enumError err = LoadSTR(&str,true,param->arg,opt_ignore>0);
	if (err)
	{
	    if ( max_err < err )
		 max_err = err;
	    ResetSTR(&str);
	    continue;
	}

	const uint offset = GetVersusPointsOffset(str.mode);
	if (offset)
	{
	    const MkwPointInfo_t *mpi = GetMkwPointInfo(str.data+offset);
	    printf("Type: %s\n",mpi->info);
	    PrintMkwPoints( stdout, 2, str.data+offset, mode, cheat_base, long_count );
	}
	ResetSTR(&str);
    }
    putchar('\n');

    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command analyze			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_analyze()
{
    enumError max_err = ERR_OK;

    StringField_t plist = {0};
    CollectExpandParam(&plist,first_param,-1,WM__DEFAULT);

    for ( int argi = 0; argi < plist.used; argi++ )
    {
	ccp arg = plist.field[argi];
	printf("\n* Analyze %s\n",arg);
	staticr_t str;
	enumError err = LoadSTR(&str,true,arg,opt_ignore>0);
	if (err)
	{
	    if ( max_err < err )
		 max_err = err;
	    ResetSTR(&str);
	    continue;
	}

	PatchSTR(&str);
	AnalyzeSTR(&str);
	PrintStatusSTR(stdout,2,&str,long_count);
	ResetSTR(&str);
    }
    putchar('\n');

    ResetStringField(&plist);
    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			command patch			///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError cmd_patch()
{
    enumError max_err = ERR_OK;
    StringField_t plist = {0};
    CollectExpandParam(&plist,first_param,-1,WM__DEFAULT);

    for ( int argi = 0; argi < plist.used; argi++ )
    {
	ccp arg = plist.field[argi];
	staticr_t str;
	enumError err = LoadSTR(&str,true,arg,opt_ignore>0);

	if ( err <= ERR_WARNING && err != ERR_NOT_EXISTS )
	{
	    char dest[PATH_MAX];
	    SubstDest(dest,sizeof(dest),arg,opt_dest,0,0,false);

	    if ( verbose >= 0 || testmode )
	    {
		if (strcmp(arg,dest))
		    fprintf(stdlog,"%sPATCH %s -> %s\n",
			    testmode ? "WOULD " : "", arg, dest );
		else
		    fprintf(stdlog,"%sPATCH %s\n",
			    testmode ? "WOULD " : "", dest );
	    }

	    const uint patch_count = PatchSTR(&str);

	    char path_buf[PATH_MAX];
	    ccp dest_fname = arg;
	    if ( opt_dest && *opt_dest )
	    {
		if (IsDirectory(opt_dest,0))
		{
		    ccp slash = strrchr(arg,'/');
		    dest_fname = PathCatPP(path_buf,sizeof(path_buf),opt_dest,
						slash ? slash+1 : arg );
		}
		else
		    dest_fname = opt_dest;
	    }

	    bool create = false;
	    if (patch_count)
	    {
		create = true;
		if ( verbose >= 0 )
		    printf("* %save patched file to: %s:%s\n",
			testmode ? "Would s" : "S",
			GetNameFF(str.fform,0), dest_fname );
	    }
	    else if (strcmp(dest_fname,arg))
	    {
		create = true;
		if ( verbose >= 0 )
		    printf("* File not modified, %scopy to: %s:%s\n",
			testmode ? "would " : "",
			GetNameFF(str.fform,0), dest_fname );
	    }
	    else
	    {
		if ( verbose >= 0 )
		    printf("* File not modified: %s:%s\n",
			GetNameFF(str.fform,0), dest_fname);
	    }

	    if (create)
	    {
		File_t F;
		err = CreateFileOpt(&F,true,dest,testmode,arg);
		if (F.f)
		{
		    SetFileAttrib(&F.fatt,&str.fatt,0);
		    size_t wstat = fwrite(str.data,1,str.data_size,F.f);
		    if ( wstat != str.data_size )
			err = FILEERROR1(&F,
				    ERR_WRITE_FAILED,"Write failed: %s\n",
				    arg);
		}
		ResetFile(&F,opt_preserve);
	    }
	}

	if ( max_err < err )
	     max_err = err;
	ResetSTR(&str);
    }

    ResetStringField(&plist);
    return max_err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////                   check options                 ///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError CheckOptions ( int argc, char ** argv, bool is_env )
{
    TRACE("CheckOptions(%d,%p,%d) optind=%d\n",argc,argv,is_env,optind);

    optind = 0;
    int err = 0;

    for(;;)
    {
      const int opt_stat = getopt_long(argc,argv,OptionShort,OptionLong,0);
      if ( opt_stat == -1 )
	break;

      RegisterOptionByName(&InfoUI_wstrt,opt_stat,1,is_env);

      switch ((enumGetOpt)opt_stat)
      {
	case GO__ERR:		err++; break;
	case GO_AT_DUMMY:	break;

	case GO_VERSION:	version_exit();
	case GO_HELP:		help_exit(false);
	case GO_XHELP:		help_exit(true);
	case GO_CONFIG:		opt_config = optarg;
	case GO_ALLOW_ALL:	allow_all = true; break;
	case GO_COMPATIBLE:	err += ScanOptCompatible(optarg); break;
	case GO_WIDTH:		err += ScanOptWidth(optarg); break;
	case GO_MAX_WIDTH:	err += ScanOptMaxWidth(optarg); break;
	case GO_NO_PAGER:	opt_no_pager = true; break;
	case GO_QUIET:		verbose = verbose > -1 ? -1 : verbose - 1; break;
	case GO_VERBOSE:	verbose = verbose <  0 ?  0 : verbose + 1; break;
	case GO_LOGGING:	logging++; break;
	case GO_EXT_ERRORS:	ext_errors++; break;
	case GO_TIMING:		log_timing++; break;
	case GO_WARN:		err += ScanOptWarn(optarg); break;
	case GO_DE:		use_de = true; break;
	case GO_CT_CODE:	ctcode_enabled = true; break;
	case GO_LE_CODE:	lecode_enabled = true; break; // optional argument ignored
	case GO_COLORS:		err += ScanOptColorize(0,optarg,0); break;
	case GO_NO_COLORS:	opt_colorize = COLMD_OFF; break;

	case GO_CHDIR:		err += ScanOptChdir(optarg); break;
	case GO_CONST:		break; // [[2do]] err += ScanOptConst(optarg); break;

	case GO_UTF_8:		use_utf8 = true; break;
	case GO_NO_UTF_8:	use_utf8 = false; break;

	case GO_TEST:		testmode++; break;
	case GO_FORCE:		force_count++; break;
	case GO_REPAIR_MAGICS:	err += ScanOptRepairMagic(optarg); break;

 #if OPT_OLD_NEW
	case GO_OLD:		opt_new = opt_new>0 ? -1 : opt_new-1; break;
	case GO_STD:		opt_new = 0; break;
	case GO_NEW:		opt_new = opt_new<0 ? +1 : opt_new+1; break;
 #endif
	case GO_EXTRACT:	opt_extract = optarg; break;

	case GO_ESC:		err += ScanEscapeChar(optarg) < 0; break;
	case GO_DEST:		SetDest(optarg,false); break;
	case GO_DEST2:		SetDest(optarg,true); break;
	case GO_OVERWRITE:	opt_overwrite = true; break;
	case GO_NUMBER:		opt_number = true; break;
	case GO_REMOVE_DEST:	opt_remove_dest = true; break;
	case GO_UPDATE:		opt_update = true; break;
	case GO_PRESERVE:	opt_preserve = true; break;
	case GO_IGNORE:		opt_ignore++; break;

	case GO_CLEAN_DOL:	opt_clean_dol = true; break;
	case GO_TRACKS:		err += ScanOptTracks(optarg); break;
	case GO_ARENAS:		err += ScanOptArenas(optarg); break;
	case GO_REGION:		err += ScanOptRegion(optarg); break;
	case GO_VS_REGION:	err += ScanOptVersusRegion(optarg); break;
	case GO_BT_REGION:	err += ScanOptBattleRegion(optarg); break;
	case GO_ALL_RANKS:	err += ScanOptAllRanks(optarg); break;
	case GO_POINTS:		err += ScanOptMkwPoints(optarg,false); break;
	case GO_CHEAT:		err += ScanOptCheatRegion(optarg); break;
	case GO_ADD_LECODE:	AppendStringField(&opt_wcode_list,"@LECODE",false); break;
	case GO_ADD_OLD_LECODE:	AppendStringField(&opt_wcode_list,"@OLDLECODE",false); break;
	case GO_ADD_CTCODE:	opt_add_ctcode = true; break;
	case GO_CT_DIR:		AppendStringField(&ct_dir_list,optarg,false); break;
	case GO_MOVE_D8:	opt_move_d8 = true; break;
	case GO_ADD_SECTION:	AppendStringFieldExpand(&opt_sect_list,optarg,0,WM__DEFAULT); break;
	case GO_FULL_GCH:	opt_full_gch = true; break;

	case GO_GCT_SEP:	opt_gct_sep = true; break;
	case GO_GCT_ASM_SEP:	opt_gct_asm_sep = true; break;
	case GO_GCT_LIST:	opt_gct_list++; break;
	case GO_GCT_MOVE:	err += ScanOptGctMove(optarg); break;
	case GO_GCT_ADDR:	err += ScanOptGctAddr(optarg); break;
	case GO_GCT_SPACE:	err += ScanOptGctSpace(optarg); break;
	case GO_ALLOW_USER_GCH:	err += ScanOptAllowUserGch(optarg); break;
	case GO_GCT_NO_SEP:	opt_gct_scan_sep = false; break;

	case GO_CREATE_SECT:	err += ScanOptCreateSect(optarg); break;
	case GO_WPF:		AppendStringFieldExpand(&opt_wpf_list,optarg,0,WM__DEFAULT); break;

	case GO_HTTPS:		err += ScanOptHttps(optarg); break;
	case GO_DOMAIN:		err += ScanOptDomain(0,optarg); break;
	case GO_WIIMMFI:	err += ScanOptDomain("domain","wiimmfi.de"); break;
	case GO_TWIIMMFI:	err += ScanOptDomain("domain","test.wiimmfi.de"); break;
	case GO_WC24:		opt_wc24 = true; break;
	case GO_WCODE:		err += ScanOptWCode(optarg); break;
	case GO_ADD_WCODE:	AppendStringFieldExpand(&opt_wcode_list,optarg,0,WM__DEFAULT); break;
	case GO_PB_MODE:	err += ScanOptPBMode(optarg); break;
	case GO_PATCHED_BY:	err += ScanOptPatchedBy(optarg); break;
	case GO_VS:		err += ScanOptVS(0,0,optarg); break;
	case GO_VS2:		err += ScanOptVS(0,1,optarg); break;
	case GO_BT:		err += ScanOptVS(1,0,optarg); break;
	case GO_BT2:		err += ScanOptVS(1,1,optarg); break;
	case GO_CANNON:		err += ScanOptCannon(optarg); break;
	case GO_MENO:		opt_meno = true; break;

	case GO_LONG:		long_count++; break;
	case GO_BRIEF:		brief_count++; break;
	case GO_NO_WILDCARDS:	no_wildcards_count++; break;
	case GO_IN_ORDER:	inorder_count++; break;
	case GO_NO_HEADER:	print_header = false; break;
	case GO_SECTIONS:	print_sections++; break;
	case GO_PORT_DB:	opt_port_db = optarg; break;
	case GO_ORDER:		opt_order = optarg; break;
	case GO_NO_0X:		opt_no_0x = true; break;
	case GO_UPPER:		opt_upper = true; break;

	case GO_VADDR:		InsertAddressMemMap(&vaddr,true,optarg,16); break;
	case GO_FADDR:		InsertAddressMemMap(&faddr,true,optarg,16); break;
	case GO_SNAME:		dol_sections |= ScanDolSectionList(optarg,0); break;

	case GO_INT1:
	case GO_INT2:
	case GO_INT3:
	case GO_INT4:
	case GO_INT5:
	case GO_INT6:
	case GO_INT7:
	case GO_INT8:		xparam.format = opt_stat - GO_INT1 + XDUMPF_INT_1; break;
	case GO_FLOAT:		xparam.format = XDUMPF_FLOAT; break;
	case GO_DOUBLE:		xparam.format = XDUMPF_DOUBLE; break;

	case GO_LE:		xparam.endian = DC_LITTLE_ENDIAN; break;
	case GO_BE:		xparam.endian = DC_BIG_ENDIAN; break;

	case GO_ZEROS:		xparam.mode_zero = true; break;
	case GO_HEX:		xparam.mode_dec = false; break;
	case GO_DEC:		xparam.mode_dec = true; break;
	case GO_C_SYNTAX:	xparam.mode_c = true; break;

	case GO_ADDR:		err += ScanOptU64(&xparam.start_addr,"addr",optarg); break;
	case GO_ALIGN:		xparam.mode_align = true; break;
	case GO_TRIGGER:	err += ScanOptU64(&xparam.trigger,"trigger",optarg);
				xparam.have_trigger = true; break;
	case GO_NO_ADDR:	xparam.print_addr = false; break;
	case GO_NO_NUMBERS:	xparam.print_number = false; break;
	case GO_NO_TEXT:	xparam.print_text = false; break;
	case GO_FORMAT:		xparam.print_format = true; break;
	case GO_NO_NULL:	xparam.mode_ignore = true; break;

	case GO_NARROW:		xparam.extra_space = -1; break;
	case GO_BYTES:		xparam.min_width = str2ul(optarg,0,10); break;

	case GO_SMALL:
	    if ( !xparam.min_width )
		xparam.min_width = 8;
	    else if ( xparam.min_width > 1 )
		xparam.min_width /= 2;
	    break;

	case GO_WIDE:
	    if ( xparam.min_width < 32 )
		xparam.min_width = 32;
	    else
		xparam.min_width = ( xparam.min_width / 16 + 1 ) * 16;
	break;

	// no default case defined
	//	=> compiler checks the existence of all enum values
      }
    }

 #ifdef DEBUG
    DumpUsedOptions(&InfoUI_wstrt,TRACE_FILE,11);
    //WriteStringField(stderr,0,&opt_sect_list,"#>> ",0);
 #endif
    NormalizeOptions( verbose > 3 && !is_env );

    if (!is_env)
    {
	if ( opt_wcode == WCODE_AUTO )
	    opt_wcode = wifi_domain_is_wiimmfi || opt_wcode_list.used
				? WCODE_ON : WCODE_OFF;
	if ( opt_wcode >= WCODE_ON && opt_gct_move < OFFON_FORCE )
	    opt_gct_move = OFFON_FORCE;
    }
    return !err ? ERR_OK : ProgInfo.max_error ? ProgInfo.max_error : ERR_SYNTAX;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////                   check command                 ///////////////
///////////////////////////////////////////////////////////////////////////////

static enumError CheckCommand ( int argc, char ** argv )
{
    const KeywordTab_t * cmd_ct = CheckCommandHelper(argc,argv,CommandTab);
    if (!cmd_ct)
	hint_exit(ERR_SYNTAX);

    TRACE("COMMAND FOUND: #%lld = %s\n",(u64)cmd_ct->id,cmd_ct->name1);
    current_command = cmd_ct;

    if (!allow_all)
    {
	enumError err = VerifySpecificOptions(&InfoUI_wstrt,cmd_ct);
	if (err)
	    hint_exit(err);
    }
    WarnDepractedOptions(&InfoUI_wstrt);

    if ( cmd_ct->id != CMD_ARGTEST )
    {
	argc -= optind+1;
	argv += optind+1;

	if ( cmd_ct->id == CMD_TEST )
	    while ( argc-- > 0 )
		AddParam(*argv++);
	else
	    while ( argc-- > 0 )
		AtFileHelper(*argv++,AddParam);
    }

    extern const data_tab_t maindol_data_tab[];

    enumError err = ERR_OK;
    switch ((enumCommands)cmd_ct->id)
    {
	case CMD_VERSION:	version_exit();
	case CMD_HELP:		PrintHelpColor(&InfoUI_wstrt); break;
	case CMD_CONFIG:	err = cmd_config(); break;
	case CMD_ARGTEST:	err = cmd_argtest(argc,argv); break;
	case CMD_EXPAND:	err = cmd_expand(argc,argv); break;
	case CMD_TEST:		err = cmd_test(); break;
	case CMD_COLORS:	err = Command_COLORS(brief_count?-brief_count:long_count,0,0);
					break;
	case CMD_ERROR:		err = cmd_error(); break;
	case CMD_FILETYPE:	err = cmd_filetype(); break;
	case CMD_FILEATTRIB:	err = cmd_fileattrib(); break;
	case CMD_RAWDUMP:	err = cmd_rawdump(maindol_data_tab); break;

	case CMD_DUMP:		err = cmd_dump(false); break;
	case CMD_CDUMP:		err = cmd_dump(true); break;
	case CMD_HEXDUMP:	err = cmd_hexdump(); break;
	case CMD_PORT:		err = cmd_port(); break;
	case CMD_WHERE:		err = cmd_where(); break;
	case CMD_HTTPS:		err = cmd_https(); break;
	case CMD_EXTRACT:	err = cmd_extract(); break;

	case CMD_TRACKS:	err = cmd_tracks(); break;
	case CMD_ARENAS:	err = cmd_arenas(); break;
	case CMD_FILES:		err = cmd_files(); break;
	case CMD_FIND:		err = cmd_find(); break;
	case CMD_POINTS:	err = cmd_points(); break;

	case CMD_ANALYZE:	err = cmd_analyze(); break;
	case CMD_PATCH:		err = cmd_patch(); break;

	// no default case defined
	//	=> compiler checks the existence of all enum values

	case CMD__NONE:
	case CMD__N:
	    help_exit(false);
    }

    return PrintErrorStat(err,verbose,cmd_ct->name1);
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			   main()			///////////////
///////////////////////////////////////////////////////////////////////////////

#if SZS_WRAPPER
    int main_wstrt ( int argc, char ** argv )
#else
    int main ( int argc, char ** argv )
#endif
{
 #if !SZS_WRAPPER
    ArgManager_t am = {0};
    SetupArgManager(&am,LOUP_AUTO,argc,argv,false);
    ExpandAtArgManager(&am,AMXM_SHORT,10,false);
    argc = am.argc;
    argv = am.argv;
 #endif

    tool_name = "wstrt";
    print_title_func = print_title;
    SetupLib(argc,argv,WSTRT_SHORT,VERSION,TITLE);

    //----- process arguments

    if ( argc < 2 )
    {
	printf("\n%s\n%s\nVisit %s%s for more info.\n\n",
		text_logo, TITLE, URI_HOME, WSTRT_SHORT );
	hint_exit(ERR_OK);
    }

    InitializeXDump(&xparam);
    xparam.f		= stdlog;
    xparam.indent	= 2;
    xparam.endian	= DC_BIG_ENDIAN;
    xparam.min_addr_fw	= 8;
    xparam.print_format	= false;
    xparam.print_summary= false;

    enumError err = CheckEnvOptions2("WSTRT_OPT",CheckOptions);
    if (err)
	hint_exit(err);

    err = CheckOptions(argc,argv,false);
    if (err)
	hint_exit(err);

    err = CheckCommand(argc,argv);
    DUMP_TRACE_ALLOC(TRACE_FILE);

    if (SIGINT_level)
	err = ERROR0(ERR_INTERRUPT,"Program interrupted by user.");
    ClosePager();
    return err;
}

//
///////////////////////////////////////////////////////////////////////////////
///////////////			    END				///////////////
///////////////////////////////////////////////////////////////////////////////
