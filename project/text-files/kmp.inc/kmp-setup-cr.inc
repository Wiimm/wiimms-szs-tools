
#FF#
###############################################################################

[SETUP]
# This is the general setup and info section. Some parameters are preceeded by
# a hash ('#') to signal the informative only status. This section should
# always be the first one.

# Identify the tool and version, that have written this file:
TOOL     = %s
SYSTEM   = %s
VERSION  = %s
REVISION = %u
DATE     = %s

# If the following info string is not empty it will be
# inserted at the end of the KMP file behind all sections:
INFO = "%s"

# KMP version (offset 0x0c):
KMP-VERSION = %d

# If set to '1', objects with road features will be added to the internal KCL
# for the automatic height calculations. Like --kcl +add-road
KCL-FALL-ADD-ROAD = 0

# Define the battle mode for the next text output:
#  0 : Disable battle support
#  1 : Enable battle support
#  2 : Enable exclusive battle support => remove unneeded sections and parameters
# This setting is overiden by option --battle.
BATTLE-MODE = %d

# Experimental feature in progress!
# If set to 1, an additional KMP section named »WIM0« is created. This new
# section should contain data of this text file like names or flags, that can
# be restored later. At the moment, the new section contains only dummy data
# for compatibility tests with other tools and the Wii. Option --wim0 can
# override this setting.
KMP-WIM0 = %d

# Statistics: Entry counter for each section:
