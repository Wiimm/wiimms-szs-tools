
 ###########################################################################
 #                                                                         #
 #                     _____     ____                                      #
 #                    |  __ \   / __ \   _     _ _____                     #
 #                    | |  \ \ / /  \_\ | |   | |  _  \                    #
 #                    | |   \ \| |      | |   | | |_| |                    #
 #                    | |   | || |      | |   | |  ___/                    #
 #                    | |   / /| |   __ | |   | |  _  \                    #
 #                    | |__/ / \ \__/ / | |___| | |_| |                    #
 #                    |_____/   \____/  |_____|_|_____/                    #
 #                                                                         #
 #                       Wiimms source code library                        #
 #                                                                         #
 ###########################################################################
 #                                                                         #
 #        Copyright (c) 2012-2023 by Dirk Clemens <wiimm@wiimm.de>         #
 #                                                                         #
 ###########################################################################

# make manual: https://www.gnu.org/software/make/manual/make.html

#------------------------------------------------------------------------------
# standard object files

DCLIB_O		:= dclib-basics.o dclib-cli.o dclib-color.o dclib-numeric.o \
			dclib-parser.o dclib-file.o dclib-option.o dclib-regex.o \
			dclib-utf8.o dclib-shift-jis.o \
			dclib-vector.o dclib-tables.o dclib-ui.o \
			dclib-xdump.o dclib-debug.o

DCLIB_O_MKW	:= lib-dol.o lib-bmg.o lib-mkw.o

DCLIB_O_WIIMMFI	:= lib-wiimmfi.o lib-gamespy.o lib-natneg.o

# legacy
DCLIB_O1	:= $(DCLIB_O_MKW)
DCLIB_O2	:= $(DCLIB_O_WIIMMFI)

DCLIB_PRIVATE := dclib-terminal* dclib-command* lib-wiimmfi* lib-gamespy* lib-natneg*

#------------------------------------------------------------------------------
# if DCLIB_THREADS not set: disable it

ifeq ($(origin DCLIB_THREAD), undefined)
  DCLIB_THREAD := 0
endif

#------------------------------------------------------------------------------
# if DCLIB_MYSQL not set: enable it if file 'mysql.h' is found

ifeq ($(origin DCLIB_MYSQL), undefined)
  DCLIB_MYSQL := $(shell [[ -f /usr/include/mysql/mysql.h ]]; echo $$((!$$?)))
endif

#------------------------------------------------------------------------------
# if DCLIB_TERMINAL not set: disable it

ifeq ($(origin DCLIB_TERMINAL), undefined)
  DCLIB_TERMINAL := 0
endif

#------------------------------------------------------------------------------
# if DCLIB_NETWORK not set: enable it if DCLIB_MYSQL is also set

ifeq ($(origin DCLIB_NETWORK), undefined)
  DCLIB_NETWORK := $(DCLIB_MYSQL)
endif

#------------------------------------------------------------------------------
# some more setting

ifeq ($(origin DCLIB_HAVE_USABLE_SIZE), undefined)
    DCLIB_HAVE_USABLE_SIZE := $(shell grep -sqw malloc_usable_size /usr/include/malloc.h; echo $$((!$$?)))
endif

#------------------------------------------------------------------------------
# gcc version dependent cflags

DCLIB_GCC	= $(GCC)
ifeq ($(DCLIB_GCC),)
    DCLIB_GCC	:= gcc
endif

DCLIB_GCC_VERSION = $(shell $(DCLIB_GCC) -dumpversion)

ifeq ($(shell expr $(DCLIB_GCC_VERSION) '>=' 7 ),1)
  DCLIB_CFLAGS += -Wno-format-truncation
else
  DCLIB_CFLAGS += -Wno-pragmas -Wno-unknown-pragmas
endif

ifeq ($(shell expr $(DCLIB_GCC_VERSION) '>=' 9 ),1)
  DCLIB_CFLAGS += -Wno-address-of-packed-member
endif

#------------------------------------------------------------------------------
# add object files and guardian macros

ifeq ($(DCLIB_THREAD),1)
    DCLIB_O += dclib-thread.o
    DCLIB_DEFINES += -DDCLIB_THREAD=1
endif

ifeq ($(DCLIB_NETWORK),1)
    DCLIB_O += dclib-network.o dclib-punycode.o
    DCLIB_DEFINES += -DDCLIB_NETWORK=1
else
    DCLIB_MYSQL := 0
endif

ifeq ($(DCLIB_MYSQL),1)
    DCLIB_O += dclib-mysql.o
    DCLIB_DEFINES += -DDCLIB_MYSQL=1
endif

ifeq ($(DCLIB_TERMINAL),1)
    DCLIB_O += dclib-terminal.o dclib-command.o
    DCLIB_DEFINES += -DDCLIB_TERMINAL=1
endif

ifeq ($(DCLIB_HAVE_USABLE_SIZE),1)
    DCLIB_DEFINES += -DDCLIB_HAVE_USABLE_SIZE=1
endif

#------------------------------------------------------------------------------

# support for deprecated DCLIB_FLAGS (2023-04-14)
DCLIB_FLAGS := $(DCLIB_DEFINES) $(DCLIB_CFLAGS)

#------------------------------------------------------------------------------
