
#####################################################################
##                    _______ _______ _______                      ##
##                   |  ___  |____   |  ___  |                     ##
##                   | |   |_|    / /| |   |_|                     ##
##                   | |_____    / / | |_____                      ##
##                   |_____  |  / /  |_____  |                     ##
##                    _    | | / /    _    | |                     ##
##                   | |___| |/ /____| |___| |                     ##
##                   |_______|_______|_______|                     ##
##                                                                 ##
##                       Wiimms SZS Tools                          ##
##                     https://szs.wiimm.de/                       ##
##                                                                 ##
#####################################################################
##                                                                 ##
##   This file is part of the SZS project.                         ##
##   Visit https://szs.wiimm.de/ for project details and sources.  ##
##                                                                 ##
##   Copyright (c) 2011-2022 by Dirk Clemens <wiimm@wiimm.de>      ##
##                                                                 ##
#####################################################################

#----------------------------------------------------------------
# make manual: http://www.gnu.org/software/make/manual/make.html
#----------------------------------------------------------------

#-------------------------------------------------------------------------------
# global settings

MYMAKE			:= $(lastword $(MAKEFILE_LIST))

SHELL			= /usr/bin/env bash

AUTHOR			= Dirk Clemens
TOOLSET_SHORT		= SZS
TOOLSET_LONG		= Wiimms SZS Toolset

WSZST_SHORT		= wszst
WSZST_LONG		= Wiimms SZS Tool
WBMGT_SHORT		= wbmgt
WBMGT_LONG		= Wiimms BMG Tool
WCTCT_SHORT		= wctct
WCTCT_LONG		= Wiimms CT-CODE Tool
WIMGT_SHORT		= wimgt
WIMGT_LONG		= Wiimms Image Tool
WKCLT_SHORT		= wkclt
WKCLT_LONG		= Wiimms KCL Tool
WKMPT_SHORT		= wkmpt
WKMPT_LONG		= Wiimms KMP Tool
WLECT_SHORT		= wlect
WLECT_LONG		= Wiimms LE-CODE Tool
WMDLT_SHORT		= wmdlt
WMDLT_LONG		= Wiimms MDL Tool
WPATT_SHORT		= wpatt
WPATT_LONG		= Wiimms PAT Tool
WSTRT_SHORT		= wstrt
WSTRT_LONG		= Wiimms StaticR Tool

VERSION_NUM		= 2.32a
BETA_VERSION		= 0
			# 0:off  -1:"beta"  >0:"beta#"

URI_HOME		= https://szs.wiimm.de/
URI_DOWNLOAD		= https://szs.wiimm.de/download
URI_FILE		= https://szs.wiimm.de/file
URI_SOURCE		= https://download.wiimm.de/source/wiimms-szs-tools/

ifeq ($(BETA_VERSION),0)
  URI_REPOS		= http://opensvn.wiimm.de/wii/trunk/wiimms-szs-tools/
  URI_VIEWVC		= https://szs.wiimm.de/r/viewvc
else
  URI_REPOS		= http://opensvn.wiimm.de/wii/branches/public/wiimms-szs-tools/
  URI_VIEWVC		= https://szs.wiimm.de/r/viewvc-beta
endif

URI_DOWNLOAD_I386	= $(URI_DOWNLOAD)/$(DISTRIB_I386)
URI_DOWNLOAD_X86_64	= $(URI_DOWNLOAD)/$(DISTRIB_X86_64)
URI_DOWNLOAD_MAC	= $(URI_DOWNLOAD)/$(DISTRIB_MAC)
URI_DOWNLOAD_CYGWIN32	= $(URI_DOWNLOAD)/$(DISTRIB_CYGWIN32)
URI_DOWNLOAD_CYGWIN64	= $(URI_DOWNLOAD)/$(DISTRIB_CYGWIN64)

DOWNLOAD_DIR		= /n/www/szs.wiimm.de/download
EXPORT_DIR		= /cygdrive/n/wit/wiimms-szs-tools/bin

# format for logging messages, $1=job, $2=object, $3=more info
LOGFORMAT		:= \33[36;1m*** %7s %-17s %s\33[0m\n
LOGFORMAT_CC		:= \33[33;1m*** %7s %-17s %s\33[0m\n
LOGFORMAT_TOOL		:= \33[35;1m*** %7s %-17s %s\33[0m\n
LOGFORMAT_INST		:= \33[32;1m*** %7s %-17s %s\33[0m\n

#-------------------------------------------------------------------------------
# version+beta settings

ifeq ($(BETA_VERSION),0)
  BETA_SUFFIX	:=
else ifeq ($(BETA_VERSION),-1)
  BETA_SUFFIX	:= .beta
else
  BETA_SUFFIX	:= .beta$(BETA_VERSION)
endif

VERSION		:= $(VERSION_NUM)$(BETA_SUFFIX)
VERSION_INT	:= $(shell sed 's/\.//g; s/[^0-9]/ /g; s/ .*//' <<< "$(VERSION_NUM)" )

DUMMY		:= $(shell $(SHELL) ./setup.sh $(VERSION) $(VERSION_NUM) $(VERSION_INT) )
include Makefile.setup
#DUMMY		:= $(shell echo "|$(SYSTEM)|$(SYSTEM2)|" >&2)

#------------------------------------------------------------------------------
# include source library definitions

DCLIB_MYSQL := 0
include ./dclib/Makefile.inc

#-------------------------------------------------------------------------------
# compiler settings

PRE		?=
CC		= $(PRE)gcc
STRIP		= $(PRE)strip

#-------------------------------------------------------------------------------
# files

DIR_LIST	=

RM_FILES	= *.{a,o,d,tmp,bak,exe} edit-list/*.log
RM_FILES	+= */*.{tmp,bak} */*/*.{tmp,bak} */*/*/*.{tmp,bak}
RM_FILES2	= templates.sed Makefile.setup

MODE_FILE	= ./_mode.flag
MODE		= $(shell test -s $(MODE_FILE) && cat $(MODE_FILE))
RM_FILES	+= $(MODE_FILE)

UI		= ./src/ui

#-------------------------------------------------------------------------------
# base flags

DEFINES1	+= -DLARGE_FILES -D_FILE_OFFSET_BITS=64
DEFINES1	+= -DWSZST			# compile szs tools (switch for shared sources)
DEFINES1	+= -DDEBUG_ASSERT		# enable ASSERTions in release version too
DEFINES1	+= -DEXTENDED_ERRORS=1		# enable extended error messages (function,line,file)
DEFINES1	+= -DUIOPT_INDEX_SIZE=310	# define max number of options (default 0x100)

CFLAGS		+= -std=gnu99 -fomit-frame-pointer -fno-strict-aliasing -funroll-loops
CFLAGS		+= -Wall -Wno-parentheses -Wno-unused-function
CFLAGS		+= -Wno-deprecated-declarations -Wno-gnu-variable-sized-type-not-at-end
CFLAGS		+= -O3 -Isrc -I$(UI) -I.

DEPFLAGS	+= -MMD

#--- system depedent flags & libs

ifeq ($(SYSTEM),mac)
 CFLAGS		+= -I/usr/local/include -Wno-typedef-redefinition
endif

ifeq ($(SYSTEM_LINUX),1)
 CFLAGS		+= -ffunction-sections -fdata-sections
 LDFLAGS	+= -Xlinker --gc-sections
 #LDFLAGS	+= -Xlinker --print-gc-sections
 LIBS		+= -ltinfo
 LDFLAGS	+= -static-libgcc
endif

ifeq ($(SYSTEM),cygwin)
 LDFLAGS	+= -static-libgcc
endif

ifeq ($(shell expr $(GCC_VERSION) '>=' 7.0 ),1)
ifneq ($(SYSTEM),mac)
  CFLAGS	+= -Wno-format-truncation
endif
endif

ifeq ($(HAVE_SHA),1)
 ifeq ($(SYSTEM_LINUX),1)
  ifneq ($(STATIC),1)
    DEFINES1	+= -DUSE_SHA_LIB
    LIBS	+= -lcrypto
  endif
 else ifeq ($(SYSTEM),cygwin)
    DEFINES1	+= -DUSE_SHA_LIB
    LIBS	+= -lcrypto
 endif
endif

#-------------------------------------------------------------------------------
# tools

MAIN_TOOLS	:= wszst wbmgt wctct wimgt wkclt wkmpt wlect wmdlt wpatt wstrt
TEST_TOOLS	:= wtest
EXTRA_TOOLS	:= 
HELPER_TOOLS	:= gen-ui

WRAPPER_TOOLS	:= $(filter-out wszst, $(MAIN_TOOLS))

ALL_TOOLS	:= $(sort $(MAIN_TOOLS) $(TEST_TOOLS))
ALL_TOOLS_X	:= $(sort $(MAIN_TOOLS) $(TEST_TOOLS) $(EXTRA_TOOLS))

RM_FILES	+= $(ALL_TOOLS_X) $(HELPER_TOOLS) gen-ui

#-------------------------------------------------------------------------------
# tool dependent options and objects

ifeq ($(STATIC),1)
    OPT_STATIC	:= -static -DDCLIB_STATIC=1
else
    OPT_STATIC	:=
endif

#DUMMY		:= $(shell echo "|$(SYSTEM)|$(SYSTEM2)|$(OPT_STATIC)" >&2)

#---------------

TOPT_wszst	:= $(OPT_STATIC)
TOPT_wbmgt	:= $(OPT_STATIC)
TOPT_wctct	:= $(OPT_STATIC)
TOPT_wimgt	:= $(OPT_STATIC)
TOPT_wkclt	:= $(OPT_STATIC)
TOPT_wkmpt	:= $(OPT_STATIC)
TOPT_wlect	:= $(OPT_STATIC)
TOPT_wmdlt	:= $(OPT_STATIC)
TOPT_wpatt	:= $(OPT_STATIC)
TOPT_wstrt	:= $(OPT_STATIC)

#---------------

WRAPPER_OBJ	= $(patsubst %,wrapper-%.o,$(WRAPPER_TOOLS))

XOBJ_IMAGE	= lib-image1.o lib-image2.o lib-image3.o
XOBJ_PARSER	= lib-parser.o lib-parser-func.o
XOBJ_OBJECT	= lib-object.o db-object.o db-file.o lib-bmg.o
XOBJ_KCL	= lib-kcl.o db-kcl.o
XOBJ_KMP	= lib-kmp.o lib-kmp-text.o lib-kmp-pflags.o lib-kmp-diff.o lib-kmp-draw.o
XOBJ_COMMON	= lib-common.o lib-geohit.o lib-itemslot.o lib-minigame.o lib-objflow.o

XOBJ_LIB	= analyze.o lib-szs-create.o lib-analyze.o lib-xbmg.o \
			lib-mdl.o lib-pat.o lib-transform.o \
			lib-ctcode.o lib-lex.o lib-lecode.o lib-ledis.o \
			lib-staticr.o db-dol.o db-ctcode.o getopt.o \
			$(XOBJ_KMP) $(XOBJ_COMMON) $(XOBJ_KCL) lib-kcl-image.o lib-rkg.o \
			$(XOBJ_OBJECT) $(XOBJ_PARSER) $(XOBJ_IMAGE)

TOBJ_gen-ui	= $(XOBJ_OBJECT)
TOBJ_wszst	= $(XOBJ_LIB) $(WRAPPER_OBJ)
TOBJ_wbmgt	= lib-xbmg.o $(XOBJ_OBJECT)
TOBJ_wctct	= $(XOBJ_LIB)
TOBJ_wimgt	= $(XOBJ_IMAGE) $(XOBJ_OBJECT)
TOBJ_wkclt	= lib-transform.o $(XOBJ_KCL) $(XOBJ_PARSER) $(XOBJ_OBJECT)
TOBJ_wkmpt	= $(XOBJ_KMP) lib-transform.o \
			$(XOBJ_KCL) lib-kcl-image.o \
			$(XOBJ_IMAGE) $(XOBJ_PARSER) $(XOBJ_OBJECT)
TOBJ_wlect	= $(XOBJ_LIB)
TOBJ_wmdlt	= lib-mdl.o lib-transform.o $(XOBJ_PARSER) $(XOBJ_OBJECT)
TOBJ_wpatt	= lib-pat.o $(XOBJ_PARSER) $(XOBJ_OBJECT)
TOBJ_wstrt	= lib-staticr.o db-dol.o db-ctcode.o $(XOBJ_OBJECT)
TOBJ_wtest	= $(XOBJ_LIB)

#-------------------------------------------------------------------------------
# sub libs

# libbz2
LIBBZ2_SRC	= $(shell echo src/libbz2/*.c)
LIBBZ2_OBJ	= $(patsubst %.c,%.o,$(LIBBZ2_SRC))

ifneq ($(SYSTEM),mac)
  BZIP2_FLAGS	= -Wno-maybe-uninitialized
endif

# lib summary
LIB_LIST	+= libbz2
LIB_OBJECTS	+= $(LIBBZ2_OBJ)
RM_FILES	+= $(foreach l,$(LIB_LIST),src/$(l)/*.{d,o})

#-------------------------------------------------------------------------------
# object files

TOBJ_ALL	= $(sort $(TOBJ_wszst) $(TOBJ_wbmgt) $(TOBJ_wctct) \
			 $(TOBJ_wimgt) $(TOBJ_wkclt) $(TOBJ_wkmpt) \
			 $(TOBJ_wlect) $(TOBJ_wmdlt) $(TOBJ_wpatt) \
			 $(TOBJ_wstrt) $(TOBJ_wtest) $(TOBJ_szs) )
TOBJ_NO_WRAPPER	= $(filter-out $(WRAPPER_OBJ), $(TOBJ_ALL) )

# objects of tools
MAIN_TOOLS_OBJ	= $(patsubst %,%.o,$(MAIN_TOOLS) $(EXTRA_TOOLS))
OTHER_TOOLS_OBJ	= $(patsubst %,%.o,$(TEST_TOOLS) $(HELPER_TOOLS))

# dclib
DCLIB_DIR	:= ./dclib
DIR_LIST	+= $(DCLIB_DIR)
VPATH		+= $(DCLIB_DIR)
CFLAGS		+= -I$(DCLIB_DIR)

# other objects
SZS_O		:= lib-std.o lib-file.o lib-numeric.o \
		   lib-szs.o lib-checksum.o \
		   lib-rarc.o lib-pack.o lib-rkc.o lib-brres.o lib-breff.o \
		   file-type.o db-mkw.o lib-mkw.o lib-dol.o \
		   sha1dgst.o sha1_one.o lib-bzip2.o \
		   $(DCLIB_O)

# object groups
UI_OBJECTS	= $(sort $(MAIN_TOOLS_OBJ))
C_OBJECTS	= $(sort $(OTHER_TOOLS_OBJ) $(SZS_O) $(TOBJ_NO_WRAPPER))
ASM_OBJECTS	= ssl-asm.o

# all objects + sources
ALL_OBJECTS	= $(sort $(SZS_O) $(ASM_OBJECTS) $(LIB_OBJECTS))
ALL_SOURCES	= $(patsubst %.o,%.c,$(UI_OBJECTS) $(C_OBJECTS) \
			$(WRAPPER_OBJ) $(ASM_OBJECTS))

#-------------------------------------------------------------------------------
# source files

UI_FILES	= ui.def
UI_FILES	+= $(patsubst %,ui-%.c,$(MAIN_TOOLS) $(EXTRA_TOOLS))
UI_FILES	+= $(patsubst %,ui-%.h,$(MAIN_TOOLS) $(EXTRA_TOOLS))

UI_TABS		= $(patsubst %,tab-%.inc,$(MAIN_TOOLS) $(EXTRA_TOOLS))

SETUP_DIR	=  ./setup
SETUP_INFO	=  INSTALL.txt
SETUP_FILES	=  version.h install.sh cygwin-copy.sh szs.def \
		   $(SETUP_INFO) $(CYGWIN_SCRIPTS)
DIR_LIST	+= $(SETUP_DIR)
RM_FILES2	+= $(SETUP_FILES)

TEXT_DIR	=  ./text-files
TEXT_FILES	= $(shell cd $(TEXT_DIR) && echo *.inc)
DIR_LIST	+= $(TEXT_DIR)
RM_FILES2	+= $(TEXT_FILES)

#-------------------------------------------------------------------------------
# lib export

LIBEXPORT	= ./_lib-export

LIBEX_FILES	=  src/lib-*.[ch]
LIBEX_FILES	+= src/system.h src/types.h src/debug.h

#-------------------------------------------------------------------------------

INSTALL_SCRIPTS	= install.sh
RM_FILES	+= $(INSTALL_SCRIPTS)
TEMPLATES	= ./templates
MODULES		= $(TEMPLATES)/module
GEN_TEMPLATE	= ./gen-template.sh
GEN_TEXT_FILE	= ./gen-text-file.sh
DIR_LIST	+= $(TEMPLATES) $(MODULES)

VPATH		+= src src/crypto $(UI)
DIR_LIST	+= src src/crypto $(UI)


#--- external flags & clean

DEFINES		=  $(strip $(DEFINES1) $(MODE) $(XDEF))
CFLAGS		+= $(XFLAGS)
CFLAGS		:= $(strip $(CFLAGS))
LDFLAGS		:= $(strip $(LDFLAGS))
LIBS		+= $(LIBPNG) -lm -lncurses $(XLIBS)

#-------------------------------------------------------------------------------


DISTRIB_RM	= ./szs-v$(VERSION)-r
DISTRIB_BASE	= szs-v$(VERSION)-r$(REVISION_NEXT)
DISTRIB_PATH	= ./$(DISTRIB_BASE)-$(SYSTEM2)
DISTRIB_I386	= $(DISTRIB_BASE)-i386.tar.gz
DISTRIB_X86_64	= $(DISTRIB_BASE)-x86_64.tar.gz
DISTRIB_MAC	= $(DISTRIB_BASE)-mac.tar.gz
DISTRIB_CYGWIN32 = $(DISTRIB_BASE)-cygwin32.zip
DISTRIB_CYGWIN64 = $(DISTRIB_BASE)-cygwin64.zip
DISTRIB_FILES	= gpl-2.0.txt $(INSTALL_SCRIPTS) $(SETUP_INFO)

DOC_FILES	= doc/*.txt
IGNORE_DOC_FILES= HISTORY-v*.txt

SUPPORT_DIR	=  ./support
SUPPORT_FILES	=  $(SUPPORT_DIR)/*.txt $(SUPPORT_DIR)/*.xml
DIR_LIST	+= $(SUPPORT_DIR)

BIN_FILES	= $(MAIN_TOOLS) $(EXTRA_TOOLS)
SHARE_FILES	=

CYGWIN_DIR	= /usr/bin
CYGWIN_TOOLS	= bash chmod cp env grep mkdir realpath regtool stat tr less
CYGWIN_SCRIPTS	= windows-install.sh windows-uninstall.sh windows-uninstall.bat \
		  sendto-install.sh sendto-uninstall.sh
CYGWIN_INSTALL	= $(SETUP_DIR)/windows-install.exe \
		  $(SETUP_DIR)/sendto-install.bat \
		  $(SETUP_DIR)/sendto-uninstall.bat
CYGWIN_UNINSTALL= $(SETUP_DIR)/windows-uninstall.bat

WIN_INSTALL_PATH= Wiimm/SZS

DIR_LIST_BIN	= $(SCRIPTS) bin
DIR_LIST	+= $(DIR_LIST_BIN)
DIR_LIST	+= share makefiles-local edit-list

#
###############################################################################
# default rule

default_rule: all
	@echo "HINT: try 'make help'"

# include this behind the default rule
-include $(ALL_SOURCES:.c=.d)
-include $(TEXT_FILES:.inc=.inc.d)

#
###############################################################################
# local definitions

-include ./dclib/Makefile.rules
-include makefiles-local/Makefile.local.$(SYSTEM)
-include Makefile.user

ifeq ($(WIIMM).$(NOWIIMM).$(HAVE_WORK),1..1)
    DIR_LIST	+= ./work
endif

ifeq ($(WIIMM).$(NOWIIMM).$(HAVE_WORK_SRC),1..1)
    DEFINES1	+= -DHAVE_WIIMM_EXT=1
    CFLAGS	+= -I./work/src
    VPATH	+= ./work/src
    DIR_LIST	+= ./work/src
    TOBJ_wszst	+= lib-vehicle.o db-vehicle.o wcommand.o
    #LIBS	+= -lncurses
endif

#
###############################################################################
# general rules
# -Xlinker -Map=$@.map 

$(ALL_TOOLS_X): %: %.o $(ALL_OBJECTS) $(TOBJ_ALL) Makefile | $(HELPER_TOOLS) $(UI_OBJECTS)
	@printf "$(LOGFORMAT_TOOL)" tool "$@" "$(MODE) $(TOPT_$@)"
	@$(CC) $(CFLAGS) $(DEFINES) $(LDFLAGS) $@.o \
		$(ALL_OBJECTS) $(TOBJ_$@) $(LIBS) $(TOPT_$@) -o $@
	@if test -f $@.exe; then $(STRIP) $@.exe; else $(STRIP) $@; fi

	@mkdir -p bin/$(SYSTEM2) bin/$(SYSTEM2)/debug
	@if test -s $(MODE_FILE) && grep -Fq -e -DDEBUG $(MODE_FILE); \
		then cp -p $@ bin/$(SYSTEM2)/debug/; \
		else cp -p $@ bin/; cp -p $@ bin/$(SYSTEM2)/; fi

#--------------------------

$(HELPER_TOOLS): %: %.o $(ALL_OBJECTS) $(TOBJ_NO_WRAPPER) $(UI_TABS) Makefile
	@printf "$(LOGFORMAT_TOOL)" helper "$@ $(TOBJ_$@)" "$(MODE)"
	@$(CC) $(CFLAGS) $(DEFINES) $(LDFLAGS) $@.o \
		$(ALL_OBJECTS) $(TOBJ_$@) $(LIBS) -o $@

#--------------------------

$(UI_OBJECTS): %.o: %.c ui-%.c ui-%.h version.h Makefile
	@printf "$(LOGFORMAT_CC)" +object "$@" "$(MODE)"
	@$(CC) $(CFLAGS) $(DEPFLAGS) $(DEFINES) -c $< -o $@

#--------------------------

$(C_OBJECTS): %.o: %.c version.h Makefile | $(TEXT_FILES)
	@printf "$(LOGFORMAT_CC)" object "$@" "$(MODE)"
	@$(CC) $(CFLAGS) $(DEPFLAGS) $(DEFINES) -c $< -o $@

#--------------------------

$(WRAPPER_OBJ): %.o: %.c version.h Makefile | $(TEXT_FILES) gen-ui
	@printf "$(LOGFORMAT_CC)" object "$@" "$(MODE)"
	@$(CC) $(CFLAGS) $(DEPFLAGS) $(DEFINES) -c $< -o $@

#--------------------------

$(ASM_OBJECTS): %.o: %.S Makefile
	@printf "$(LOGFORMAT_CC)" asm "$@" "$(MODE)"
	@$(CC) $(CFLAGS) $(DEPFLAGS) $(DEFINES) -c $< -o $@

#--------------------------

$(SETUP_FILES): templates.sed $(SETUP_DIR)/$@
	@printf "$(LOGFORMAT)" create "$@" ""
	@chmod 775 $(GEN_TEMPLATE)
	@$(GEN_TEMPLATE) "$@"

.PHONY : setup-files
setup-files: templates.sed version.h install.sh
	
#--------------------------

$(TEXT_FILES): $(GEN_TEXT_FILE)
	@printf "$(LOGFORMAT)" text "$@" ""
	@chmod 775 $(GEN_TEXT_FILE)
	@$(GEN_TEXT_FILE) $(TEXT_DIR) $@

#--------------------------

$(UI_FILES): gen-ui.c ui.h $(UI_TABS) | gen-ui
	@printf "$(LOGFORMAT)" run gen-ui ""
	@./gen-ui

.PHONY : run-ui
run-ui : gen-ui.c ui.h $(UI_TABS) $(UI_FILES) | gen-ui
#	@printf "$(LOGFORMAT)" run gen-ui
#	@./gen-ui

#
###############################################################################
# lib specific rules

$(LIBBZ2_OBJ): %.o: %.c Makefile
	@printf "$(LOGFORMAT_CC)" object "$(subst src/libbz2/,,$@)" "$(MODE) [libbz2]"
	@$(CC) $(CFLAGS) $(DEPFLAGS) $(BZIP2_FLAGS) $(DEFINES) -c $< -o $@

#
###############################################################################
# specific rules in alphabetic order

.PHONY : all
all: setup-files $(TEXT_FILES) $(HELPER_TOOLS) $(ALL_TOOLS) $(INSTALL_SCRIPTS) $(SETUP_INFO)

.PHONY : all+
all+: clean+ all distrib

#
#--------------------------

.PHONY : ch+
ch+:	chmod chown chgrp

#
#--------------------------

.PHONY : chmod
chmod:
	@printf "$(LOGFORMAT)" chmod 775/664 ""
	@for d in . $(DIR_LIST); do test -d "$$d" && chmod ug+rw "$$d"/*; done || true
	@for d in . $(DIR_LIST); do test -d "$$d" && chmod -f a+x "$$d"/*.sh; done || true
	@find . -type f -name '*.sh' -exec chmod -f a+x {} +
	@for d in $(DIR_LIST); do test -d "$$d" && chmod 775 "$$d"; done || true
	@for t in $(ALL_TOOLS_X); do test -f "$$t" && chmod 775 "$$t"; done || true
	@chmod 664 src/*.[hc] src/*/*.[hc] dclib/*.[hc]

#
#--------------------------

.PHONY : chown
chown:
	@printf "$(LOGFORMAT)" chown "-R $$( stat -c%u . 2>/dev/null || stat -f%u . ) ." ""
	@chown "$$( stat -c%u . 2>/dev/null || stat -f%u . )" *
	@for d in $(DIR_LIST); do test -d "$$d" && \
		chown -R "$$( stat -c%u . 2>/dev/null || stat -f%u . )" "$$d"; done || true

#
#--------------------------

.PHONY : chgrp
chgrp:
	@printf "$(LOGFORMAT)" chgrp "-R $$( stat -c%g . 2>/dev/null || stat -f%g . ) ." ""
	@chgrp "$$( stat -c%u . 2>/dev/null || stat -f%u . )" *
	@for d in $(DIR_LIST); do test -d "$$d" && \
		chgrp -R "$$( stat -c%g . 2>/dev/null || stat -f%g . )" "$$d"; done || true

#
#--------------------------

.PHONY : clean
clean:
	@printf "$(LOGFORMAT)" rm "output files + distrib" ""
	-@rm -fr _start-pos.tmp
	@find . -type d -name test.tmp -exec rm -rf {} +
	@rm -rf pool/tracks.tmp
	@rm -f $(RM_FILES)
	@rm -fr $(DISTRIB_RM)* $(LIBEXPORT)
	@cd dclib && rm -f $(DCLIB_PRIVATE)

.phony : do_clean+
do_clean+: clean
	@printf "$(LOGFORMAT)" rm "test files + template output" ""
	-@rm -f $(RM_FILES2)
	-@rm -fr doc

.PHONY : c+ clean+
c+ clean+: do_clean+

.PHONY : c++ clean++
c++ clean++: do_clean+ st

#
#--------------------------

.PHONY : debug
debug:
	@printf "$(LOGFORMAT)" enable debug "-> define -DDEBUG"
	@rm -f *.o $(ALL_TOOLS_X)
	@echo "-DDEBUG" >>$(MODE_FILE)
	@sort $(MODE_FILE) | uniq > $(MODE_FILE).tmp
# 2 steps to bypass a cygwin mv failure
	@cp $(MODE_FILE).tmp $(MODE_FILE)
	@rm -f $(MODE_FILE).tmp

#
#--------------------------

.PHONY : distrib2
distrib2:
ifeq ($(SYSTEM_LINUX),1)

	@printf "\n---------- DISTRIB LINUX/I386 ----------\n\n"
	@for t in $(ALL_TOOLS_X); do rm -f bin/$$t; done
	@$(MAKE) --no-print-directory clean+
	@M32=1 $(MAKE) --no-print-directory distrib
	@mv "$(DISTRIB_I386)" "save-$(DISTRIB_I386)"

	@printf "\n---------- DISTRIB LINUX/X86_64 ----------\n\n"
	@for t in $(ALL_TOOLS_X); do rm -f bin/$$t; done
	@$(MAKE) --no-print-directory clean+
	@$(MAKE) --no-print-directory distrib
	@mv "save-$(DISTRIB_I386)" "$(DISTRIB_I386)"

else
	@$(MAKE) --no-print-directory clean+ distrib
endif

#-----

.PHONY : distrib
distrib:
ifeq ($(SYSTEM),mac)
	@NOWIIMM=1 $(MAKE) --no-print-directory mac-distrib
else ifeq ($(SYSTEM),cygwin)
	rm -f version.h cygwin-copy.sh templates.sed
	NOWIIMM=1 $(MAKE) --no-print-directory all doc cygwin-copy.sh \
		$(CYGWIN_SCRIPTS) gen-distrib info
else
	NOWIIMM=1 STATIC=1 $(MAKE) --no-print-directory all doc gen-distrib info
endif

#-----

.PHONY : gen-distrib
gen-distrib:
	@printf "$(LOGFORMAT)" create "$(DISTRIB_PATH)" ""

ifeq ($(SYSTEM),cygwin)

	@rm -rf $(DISTRIB_PATH)/* 2>/dev/null || true
	@rm -rf $(DISTRIB_PATH) 2>/dev/null || true
	@mkdir -p $(DISTRIB_PATH)/bin/windows-sendto
	@mkdir -p $(DISTRIB_PATH)/scripts $(DISTRIB_PATH)/tmp
	@mkdir -p $(DISTRIB_PATH)/doc $(DISTRIB_PATH)/support
	@./cygwin-copy.sh
	@printf '@cmd\r\n' >$(DISTRIB_PATH)/bin/szs-console.bat
	@cp -p gpl-2.0.txt $(DISTRIB_PATH)
	@ln -f $(MAIN_TOOLS) $(CYGWIN_SCRIPTS) $(CYGWIN_INSTALL) $(DISTRIB_PATH)/bin
	@ln -f $(SETUP_DIR)/windows-sendto/*.bat $(DISTRIB_PATH)/bin/windows-sendto
	@ln -f $(CYGWIN_INSTALL) $(CYGWIN_UNINSTALL) $(SETUP_INFO) $(DISTRIB_PATH)
	@cp -p $(DOC_FILES) $(DISTRIB_PATH)/doc
	@cp -p $(SUPPORT_FILES) $(DISTRIB_PATH)/support
	@rm -f $(DISTRIB_PATH)/doc/$(IGNORE_DOC_FILES)
	@cp -rp $$( cat ./scripts/distrib.list ) $(DISTRIB_PATH)/scripts/
	@rm -rf $(DISTRIB_PATH)/scripts/*/.svn

	@zip -roq $(DISTRIB_PATH).zip $(DISTRIB_PATH)
	@chmod 664 $(DISTRIB_PATH).zip

else

	@printf "$(LOGFORMAT)" create "$(DISTRIB_PATH)" ""
	@rm -rf $(DISTRIB_PATH)
	@mkdir -p $(DISTRIB_PATH)/bin $(DISTRIB_PATH)/scripts
	@mkdir -p $(DISTRIB_PATH)/doc $(DISTRIB_PATH)/support

	@cp -p $(DISTRIB_FILES) $(DISTRIB_PATH)
	@ln -f $(MAIN_TOOLS) $(DISTRIB_PATH)/bin
	@for t in $(EXTRA_TOOLS); do [[ -f bin/$(SYSTEM)/$$t ]] \
		&& ln -f bin/$(SYSTEM)/$$t $(DISTRIB_PATH)/bin; done || true
	@cp -p $(DOC_FILES) $(DISTRIB_PATH)/doc
	@cp -p $(SUPPORT_FILES) $(DISTRIB_PATH)/support
	@rm -f $(DISTRIB_PATH)/doc/$(IGNORE_DOC_FILES)
	@cp -rp $$( cat ./scripts/distrib.list ) $(DISTRIB_PATH)/scripts/
	@rm -rf $(DISTRIB_PATH)/scripts/*/.svn

	@chmod -R 664 $(DISTRIB_PATH)
	@chmod a+x $(DISTRIB_PATH)/*.sh $(DISTRIB_PATH)/bin*/*
	-@ chmod a+x $(DISTRIB_PATH)/scripts/*.sh 2>/dev/null
	@chmod -R a+X $(DISTRIB_PATH)

	@tar -czf $(DISTRIB_PATH).tar.gz $(DISTRIB_PATH)
	@chmod 664 $(DISTRIB_PATH).tar.gz

endif

#-----

.PHONY : copy-distrib
copy-distrib: distrib2
	@printf "$(LOGFORMAT)" copy "$(DISTRIB_BASE).*" "-> $(DOWNLOAD_DIR)"
	@[[ -s "$(DISTRIB_I386)" ]] \
		&& cp --preserve=time "$(DISTRIB_I386)" "$(DOWNLOAD_DIR)" \
		|| true
	@[[ -s "$(DISTRIB_X86_64)" ]] \
		&& cp --preserve=time "$(DISTRIB_X86_64)" "$(DOWNLOAD_DIR)" \
		|| true
	@[[ -s "$(DISTRIB_CYGWIN32)" ]] \
		&& cp --preserve=time "$(DISTRIB_CYGWIN32)" "$(DOWNLOAD_DIR)" \
		|| true
	@[[ -s "$(DISTRIB_CYGWIN64)" ]] \
		&& cp --preserve=time "$(DISTRIB_CYGWIN64)" "$(DOWNLOAD_DIR)" \
		|| true

#
#--------------------------

.PHONY : doc
doc: $(MAIN_TOOLS) templates.sed gen-doc
	@mkdir -p ./doc

.PHONY : gen-doc
gen-doc:
	@printf "$(LOGFORMAT)" create documentation ""
	@chmod ug+x $(GEN_TEMPLATE)
	@$(GEN_TEMPLATE)

.PHONY : doc+
doc+: doc info

#
#--------------------------

.PHONY : flags
flags:
	@echo ""
	@echo "SYSTEM: |$(SYSTEM)|$(SYSTEM2)|"
	@echo "GCC:    |$(GCC_VERSION)|"
	@echo ""
	@echo "DEFINES: $(DEFINES)"
	@echo ""
	@echo "CFLAGS:  $(CFLAGS)"
	@echo ""
	@echo "LDFLAGS: $(LDFLAGS)"
	@echo ""
	@echo "OPT_STATIC: $(OPT_STATIC)"
	@echo ""
	@echo "LIBS: $(LIBS)"
	@echo ""
	@echo "C_OBJECTS: $(C_OBJECTS)"
	@echo ""
	@echo "UI_FILES: $(UI_FILES)"
	@echo ""
	@echo "ALL_TOOLS: $(ALL_TOOLS)"
	@echo ""
	@echo "WRAPPER_TOOLS: $(WRAPPER_TOOLS)"
	@echo ""
	@echo "ALL_TOOLS_X: $(ALL_TOOLS_X)"
	@echo ""

#
#--------------------------

.PHONY : info
info: szs.def wszst wkclt wkmpt
	@printf "$(LOGFORMAT)" create info.def ""
	@mkdir -p doc
	@./wszst _export makedoc		>  doc/info.def
	@./wkclt _export makedoc functions	>> doc/info.def
	@./wkmpt _export makedoc functions	>> doc/info.def

#
#--------------------------

.PHONY : install
install: all
ifeq ($(SYSTEM),cygwin)
	@[[ -d $(DISTRIB_PATH)/bin ]] || make distrib
	@chmod a+x "$(DISTRIB_PATH)/windows-install.exe"
	@( cd "$(DISTRIB_PATH)" && ./windows-install.exe )
else
	@chmod a+x install.sh
	@./install.sh --make
endif

.PHONY : install+
install+: clean+ all
	@chmod a+x install.sh
	@./install.sh --make

#--------------------------

.PHONY : install2
install2:
ifeq ($(SYSTEM_LINUX),1)

	@printf "\n---------- INSTALL LINUX/I386 ----------\n\n"
	@for t in $(ALL_TOOLS_X); do rm -f bin/$$t; done
	@$(MAKE) --no-print-directory clean+
	@M32=1 $(MAKE) -j --no-print-directory install

ifeq ($(HAVE_INSTBIN_32),1)
	@printf "$(LOGFORMAT)" copy "$(INSTBIN)/* to $(INSTBIN_32)"
	@for f in $(BIN_FILES); do [[ -f $(INSTBIN)/$$f ]] \
		&& cp -p $(INSTBIN)/$$f $(INSTBIN_32); done; true
endif

	@printf "\n---------- INSTALL LINUX/X86_64 ----------\n\n"
	@for t in $(ALL_TOOLS_X); do rm -f bin/$$t; done
	@$(MAKE) --no-print-directory clean+
	@$(MAKE) -j --no-print-directory install doc info

ifeq ($(HAVE_INSTBIN_64),1)
	@printf "$(LOGFORMAT)" copy "$(INSTBIN)/* to $(INSTBIN_64)"
	@for f in $(BIN_FILES); do [[ -f $(INSTBIN)/$$f ]] \
		&& cp -p $(INSTBIN)/$$f $(INSTBIN_64); done; true
endif

else
	@$(MAKE) --no-print-directory clean+ install
endif

#
#--------------------------

.PHONY : export32
export32:
	@printf "\n---------- BUILDING EXPORT (I386) ----------\n\n"
	$(MAKE) clean+
	@rm -rf ./bin/export-i386
	@mkdir -p ./bin/export-i386
	@M32=1 NOWIIMM=1 STATIC=1 $(MAKE) -j --no-print-directory all
	@for f in $(BIN_FILES); do [[ -f ./$$f ]] \
		&& cp -p ./$$f ./bin/export-i386/; done; true

.PHONY : export64
export64:
	@printf "\n---------- BUILDING EXPORT (X86_64) ----------\n\n"
	$(MAKE) clean+
	@rm -rf ./bin/export-x86_64
	@mkdir -p ./bin/export-x86_64
	@NOWIIMM=1 STATIC=1 $(MAKE) -j --no-print-directory all
	@for f in $(BIN_FILES); do [[ -f ./$$f ]] \
		&& cp -p ./$$f ./bin/export-x86_64/; done; true

.PHONY : export
export:
ifeq ($(SYSTEM_LINUX),1)

	$(MAKE) --no-print-directory export32 export64

else ifeq ($(SYSTEM),cygwin)

	@printf "\n---------- BUILDING EXPORT (CYGWIN) ----------\n\n"
	@rm -rf ./bin/export-cygwin
	@mkdir -p ./bin/export-cygwin
	@NOWIIMM=1 $(MAKE) --no-print-directory clean+ all
	@for f in $(BIN_FILES); do [[ -f ./$$f ]] \
		&& cp -p ./$$f ./bin/export-cygwin/; done; true
	@[[ -d "$(EXPORT_DIR)" ]] && cp -r ./bin/export-cygwin "$(EXPORT_DIR)/"

else ifeq ($(SYSTEM),mac)

	@printf "\n---------- BUILDING EXPORT (MAC) ----------\n\n"
	@rm -rf ./bin/export-mac
	@mkdir -p ./bin/export-mac
	@NOWIIMM=1 $(MAKE) --no-print-directory mac-distrib
	@for f in $(BIN_FILES); do [[ -f ./$$f ]] \
		&& cp -p ./$$f ./bin/export-mac/; done; true
	@[[ -d "$(EXPORT_DIR)" ]] && cp -r ./bin/export-mac "$(EXPORT_DIR)/"

else

	@printf "\n---------- BUILDING EXPORT ----------\n\n"
	@rm -rf ./bin/export
	@mkdir -p ./bin/export
	@NOWIIMM=1 STATIC=1 $(MAKE) --no-print-directory clean+ all
	@for f in $(BIN_FILES); do [[ -f ./$$f ]] \
		&& cp -p ./$$f ./bin/export/; done; true

endif

.PHONY : export+
export+: export
	@printf "\n---------- BUILDING EXPORT ----------\n\n"
	@printf "$(LOGFORMAT)" copy "export-bin to DISTRIB & LESERATTE"
	@cp -p bin/export-x86_64/wszst /n/www/download.wiimm.de/florian/bin/wszst
	@cp -p bin/export-x86_64/wszst /n/mkw-fun/distrib/bin/linux64/
	@cp -p bin/export-i386/wszst /n/mkw-fun/distrib/bin/linux32/
	@cp -p bin/export-cygwin/wszst.exe /n/mkw-fun/distrib/bin/cygwin/ || true

#
#--------------------------

.PHONY : libexport
libexport:
	@printf "$(LOGFORMAT)" export "-> $(LIBEXPORT)"
	@rm -rf $(LIBEXPORT)
	@mkdir -p $(LIBEXPORT)
	@cp -p $(LIBEX_FILES) $(LIBEXPORT)

#
#--------------------------

.PHONY : new
new:
	@printf "$(LOGFORMAT)" enable new "-> define -DNEW_FEATURES"
	@rm -f *.o $(ALL_TOOLS_X)
	@echo "-DNEW_FEATURES" >>$(MODE_FILE)
	@sort $(MODE_FILE) | uniq > $(MODE_FILE).tmp
# 2 steps to bypass a cygwin mv failure
	@cp $(MODE_FILE).tmp $(MODE_FILE)
	@rm -f $(MODE_FILE).tmp

#
#--------------------------

.PHONY : predef
predef:
	@gcc -E -dM none.c | sort

#
#--------------------------

templates.sed: Makefile
	@printf "$(LOGFORMAT)" create templates.sed ""
	@printf '%s\n' \
		'/^~/ d;' \
		's|@.@@@|$(VERSION_NUM)|g;' \
		's|@@@@-@@-@@|$(DATE)|g;' \
		's|@@:@@:@@|$(TIME)|g;' \
		's|@@AUTHOR@@|$(AUTHOR)|g;' \
		's|@@SYSTEM@@|$(SYSTEM)|g;' \
		's|@@SYSTEM2@@|$(SYSTEM2)|g;' \
		's|@@TOOLSET-SHORT@@|$(TOOLSET_SHORT)|g;' \
		's|@@TOOLSET-LONG@@|$(TOOLSET_LONG)|g;' \
		's|@@WSZST-SHORT@@|$(WSZST_SHORT)|g;' \
		's|@@WSZST-LONG@@|$(WSZST_LONG)|g;' \
		's|@@WBMGT-SHORT@@|$(WBMGT_SHORT)|g;' \
		's|@@WBMGT-LONG@@|$(WBMGT_LONG)|g;' \
		's|@@WCTCT-SHORT@@|$(WCTCT_SHORT)|g;' \
		's|@@WCTCT-LONG@@|$(WCTCT_LONG)|g;' \
		's|@@WIMGT-SHORT@@|$(WIMGT_SHORT)|g;' \
		's|@@WIMGT-LONG@@|$(WIMGT_LONG)|g;' \
		's|@@WKCLT-SHORT@@|$(WKCLT_SHORT)|g;' \
		's|@@WKCLT-LONG@@|$(WKCLT_LONG)|g;' \
		's|@@WKMPT-SHORT@@|$(WKMPT_SHORT)|g;' \
		's|@@WKMPT-LONG@@|$(WKMPT_LONG)|g;' \
		's|@@WLECT-SHORT@@|$(WLECT_SHORT)|g;' \
		's|@@WLECT-LONG@@|$(WLECT_LONG)|g;' \
		's|@@WMDLT-SHORT@@|$(WMDLT_SHORT)|g;' \
		's|@@WMDLT-LONG@@|$(WMDLT_LONG)|g;' \
		's|@@WPATT-SHORT@@|$(WPATT_SHORT)|g;' \
		's|@@WPATT-LONG@@|$(WPATT_LONG)|g;' \
		's|@@WSTRT-SHORT@@|$(WSTRT_SHORT)|g;' \
		's|@@WSTRT-LONG@@|$(WSTRT_LONG)|g;' \
		's|@@VERSION@@|$(VERSION)|g;' \
		's|@@VERSION-NUM@@|$(VERSION_NUM)|g;' \
		's|@@VERSION-INT@@|$(VERSION_INT)|g;' \
		's|@@VERSION-CODE@@|$(VERSION_CODE)|g;' \
		's|@@BETA-VERSION@@|$(BETA_VERSION)|g;' \
		's|@@BETA-SUFFIX@@|$(BETA_SUFFIX)|g;' \
		's|@@REV@@|$(REVISION)|g;' \
		's|@@REV-NUM@@|$(REVISION_NUM)|g;' \
		's|@@REV-NEXT@@|$(REVISION_NEXT)|g;' \
		's|@@BINTIME@@|$(BINTIME)|g;' \
		's|@@DATE@@|$(DATE)|g;' \
		's|@@TIME@@|$(TIME)|g;' \
		's|@@YEAR@@|$(YEAR)|g;' \
		's|@@INSTALL-PATH@@|$(INSTALL_PATH)|g;' \
		's|@@BIN-FILES@@|$(BIN_FILES)|g;' \
		's|@@SHARE-FILES@@|$(SHARE_FILES)|g;' \
		's|@@CYGWIN-DIR@@|$(CYGWIN_DIR)|g;' \
		's|@@CYGWIN-TOOLS@@|$(CYGWIN_TOOLS)|g;' \
		's|@@WIN-INSTALL-PATH@@|$(WIN_INSTALL_PATH)|g;' \
		's|@@DISTRIB-PATH@@|$(DISTRIB_PATH)|g;' \
		's|@@DISTRIB-I386@@|$(DISTRIB_I386)|g;' \
		's|@@DISTRIB-X86_64@@|$(DISTRIB_X86_64)|g;' \
		's|@@DISTRIB-MAC@@|$(DISTRIB_MAC)|g;' \
		's|@@DISTRIB-CYGWIN32@@|$(DISTRIB_CYGWIN64)|g;' \
		's|@@DISTRIB-CYGWIN64@@|$(DISTRIB_CYGWIN32)|g;' \
		's|@@URI-REPOS@@|$(URI_REPOS)|g;' \
		's|@@URI-VIEWVC@@|$(URI_VIEWVC)|g;' \
		's|@@URI-HOME@@|$(URI_HOME)|g;' \
		's|@@URI-SOURCE@@|$(URI_SOURCE)|g;' \
		's|@@URI-DOWNLOAD@@|$(URI_DOWNLOAD)|g;' \
		's|@@URI-DOWNLOAD-I386@@|$(URI_DOWNLOAD_I386)|g;' \
		's|@@URI-DOWNLOAD-X86_64@@|$(URI_DOWNLOAD_X86_64)|g;' \
		's|@@URI-DOWNLOAD-MAC@@|$(URI_DOWNLOAD_MAC)|g;' \
		's|@@URI-DOWNLOAD-CYGWIN@@|$(URI_DOWNLOAD_CYGWIN)|g;' \
		's|@@HAVE-XSRC@@|$(HAVE_XSRC)|g;' \
		>templates.sed

#
#--------------------------

.PHONY : st
st:
	@printf "$(LOGFORMAT)" svn status
	@[[ -h xsrc ]] && rm xsrc || true
	@test -d .svn && svn status | sort -k2 || true
	@printf "* N="
	@svn diff | grep ^+ | wc -l

#
#--------------------------

.PHONY : test
test:
	@printf "$(LOGFORMAT)" enable test "-> define -DTEST"
	@rm -f *.o $(ALL_TOOLS_X)
	@echo "-DTEST" >>$(MODE_FILE)
	@sort $(MODE_FILE) | uniq > $(MODE_FILE).tmp
# 2 steps to bypass a cygwin mv failure
	@cp $(MODE_FILE).tmp $(MODE_FILE)
	@rm -f $(MODE_FILE).tmp

#
#--------------------------

.PHONY : test-trace
test-trace:
	@printf "$(LOGFORMAT)" enable testtrace "-> define -DTESTTRACE"
	@rm -f *.o $(ALL_TOOLS_X)
	@echo "-DTESTTRACE" >>$(MODE_FILE)
	@sort $(MODE_FILE) | uniq > $(MODE_FILE).tmp
# 2 steps to bypass a cygwin mv failure
	@cp $(MODE_FILE).tmp $(MODE_FILE)
	@rm -f $(MODE_FILE).tmp

#
#--------------------------

.PHONY : tools
tools: $(ALL_TOOLS)

#
#--------------------------

.PHONY : wait
wait:
	@printf "$(LOGFORMAT)" enable wait "-> define -DWAIT_ENABLED"
	@rm -f *.o $(ALL_TOOLS_X)
	@echo "-DWAIT_ENABLED" >>$(MODE_FILE)
	@sort $(MODE_FILE) | uniq > $(MODE_FILE).tmp
# 2 steps to bypass a cygwin mv failure
	@cp $(MODE_FILE).tmp $(MODE_FILE)
	@rm -f $(MODE_FILE).tmp

#
#--------------------------

.PHONY : wrapper
wrapper:
	@printf "$(LOGFORMAT)" create "wrapper scripts"
	@mkdir -p ./bin/wrapper
	@for f in $(WRAPPER_TOOLS); do \
		printf '#!/usr/bin/sh\nwszst %s "$$@"\n' "$$f" >./bin/wrapper/$$f; done
	@chmod 775 ./bin/wrapper/*

#
###############################################################################
# help rule

.PHONY : help
help:
	@echo  ""
	@echo  "$(DATE) $(TIME) - $(VERSION) - svn r$(REVISION):$(REVISION_NEXT)"
	@echo  ""
	@echo  " make		:= make all"
	@echo  " make all	make all tools and install scripts"
	@echo  " make all+	:= make clean+ all distrib"
	@echo  " make _tool_	compile only the named '_tool_' (wszst,...)"
	@echo  " make tools	make all tools"
	@echo  " make libexport create directory '$(LIBEXPORT)' with export files."
	@echo  ""
	@echo  " make clean	remove all output files"
	@echo  " make clean+	make clean & rm test_files & rm template_output"
	@echo  ""
	@echo  " make debug	enable '-DDEBUG'"
	@echo  " make test	enable '-DTEST'"
	@echo  " make new	enable '-DNEW_FEATURES'"
	@echo  " make wait	enable '-DWAIT'"
	@echo  " make testtrace	enable '-DTESTTRACE'"
	@echo  " make flags	print DEFINES, CFLAGS and LDFLAGS"
	@echo  ""
	@echo  " make doc	generate doc files from their templates"
	@echo  " make distrib	make all & build $(DISTRIB_PATH)"
	@echo  " make distrib2	make distrib (i386 + X86_64 for linux)"
	@echo  " make install	make all & copy tools to $(INSTALL_PATH)"
	@echo  " make install+	:= make clean+ install"
	@echo  " make install2	make install+ (i386 + X86_64 for linux)"
	@echo  ""
	@echo  " make chmod	change mode 775/644 for known dirs and files"
	@echo  " make chown	change owner of all dirs+files to owner of ."
	@echo  " make chgrp	change group of all dirs+files to group of ."
	@echo  " make ch+	:= make chmod chown chgrp"
	@echo  ""
	@echo  " make help	print this help"
	@echo  ""
	@echo  "-------------------------------------------------------------------------------"
	@awk -F: '/^.PHONY/ { gsub(/[[:blank:]]*/,"",$$2); print $$2}' $(MYMAKE) \
		| sort | pr -5T -w80
	@echo  "-------------------------------------------------------------------------------"

#	@echo "------"
#	@echo $(TOBJ_ALL)
#	@echo "------"
#	@echo $(C_OBJECTS)
#	@echo "------"
#	@echo $(SZS_O)
#	@echo "------"

