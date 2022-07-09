
#FF#
[TRACK-LIST]
#------------------------------------------------------------------------------
#		Track listing
#------------------------------------------------------------------------------

# This is the track listing. 2 formats are supported:
#	old:	SHA1 CUP NAME
#	new:	TYPE FLAGS SHA1 TRACK_SLOT CUP NAME
#
# TYPE is either 'vs' or 'bt'. When TYPE is present, then TRACK_SLOT is
#	expected. SHA1 is the SHA1 checksunm of the track.
# FLAGS information about features:
#	d:is _d file / t:title only / h:hidden / o:original track
# SHA1 is the sha1 of the track.
# TRACK_SLOT is a number between 0 and 4095 to define the slot number of the
#	track. If <0 or not a number, the slot is undefined.
# CUP is of format "cup.index". An optional 'A' can be used as the first
#	character to indicate an arena. However, this indicator is ignored if
#	TYPE is present. Set it to "-" if the cup is unknown.
# All other characters are read in as the NAME of the track.
#
# The new format is compatible with Wiimmfi.de for region limits.

# Please edit the CUP column if missed. Use »CUP.INDEX« (e.g. »1.2« or »20.4«)
# for racing tracks and »'A' CUP.INDEX« (e.g. »A2.5«) for battle arenas.

