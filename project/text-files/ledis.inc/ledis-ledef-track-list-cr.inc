
#
##################
   [TRACK-LIST]
##################

# A track definition consists of a basic definition (keyword TRACK) and any
# number of additional definitions (keywords FILE, IDENT, NAME and XNAME).

# Normally the first free slot intended for the track type is used. The slot
# for the directly following TRACK definition can be set by the command:
#	SLOT index

# Keyword TRACK introduces a track definition. The syntax is as follows:
#	'TRACK' VARNAME TYPE PROPERTY MUSIC FLAGS
#
# If VARNAME is not '0' then the corresponding variable is assigned the
# current slot number. Variables are needed for cup references in section
# [CUP-LIST]. If cups are created exclusively automatically, then the value
# '0' is a good choice. The exporter uses 'bt' for battle or 'vs' for versus
# followed by the decimal slot number as variable name.
#
# TYPE is either 'vs' (versus) or 'bt' (battle). Both names are defined as
# constants.
#
# PROPERTY is the property slot to use. It is either a number (0-31 for 'vs'
# or 32-41 for 'bt'), or a symbolic name (like A25, T12 or MMM) of a track.
#
# MUSIC is the music-id to use. It is either a property slot number number
# (0-41) or a music id (117-199) or a symbolic name of a track.
#
# FLAGS is either a number or a string. A string is scanned for letters
# (case insensitive) to build a number:
#	'N' : LE$F_NEW   : Track is marked as "new track".
#	'H' : LE$F_HEAD  : Track is marked as "head of a group".
#	'G' : LE$F_GROUP : Track is marked as "group member".
# A track is considered hidden if the group flag (G) is set but the head flag
# (H) is not.

# The keywords FILE, IDENT, NAME and XNAME define strings for the last defined
# track. Syntax:
#	KEYWORD "TEXT"
# The TEXT is assigned to the track. The indention of the exporter is only
# used for clarity.

# There are 2 more commands to define standard arenas or tracks at the current
# position. Best is to use these commands at top of the section:
#
#   STANDARD-BATTLE-ARENAS
#	Define 10 standard battle arenas.
#
#   STANDARD-VERSUS-TRACKS
#	Define 32 standard versus tracks.


#--- Instructions

# If IGNORE-SLOT is set to value 1, then all SLOT instructions are ignored
# until it is reset to 0. This is easier than deleting all SLOT instructions.

IGNORE-SLOT 0

