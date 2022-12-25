
#FF#
[TRACK-LIST]
#------------------------------------------------------------------------------
#		Track listing
#------------------------------------------------------------------------------

# This is the track listing. 2 formats are supported:
#	old:	SHA1 CUP NAME
#	new:	TYPE DISTRIB_FLAGS LE_FLAGS SHA1 TRACK_SLOT CUP NAME

#   TYPE is either 'vs' or 'bt'. When TYPE is present, then TRACK_SLOT is
#	expected. SHA1 is the SHA1 checksunm of the track.
#
#   DISTRIB_FLAGS is an information used by ct.wiimm.de and wiimmfi.de:
#	B: boost, N: new, A: again, U: update, F: fill
#	d: is _d file, t: title only, h: hidden, o: original track
#
#   LE_FLAGS are LE-CODE track flags:
#	Flag   1:  B: battle arena, V: versus track, r: random slot.
#	Flag   2:  Used in: o: original cup, c custom cup, b: both.
#	Flag   3:  H: header of group, G: group member, X: both.
#	Flags  4:  N: new track, T: texture hack, 2: both.
#	Flags 5+6: A: alias, i: invisible/hidden.
#
#   SHA1 is the sha1 of the track.
#
#   TRACK_SLOT is a number between 0 and 4095 to define the slot number of the
#	track. If <0 or not a number, the slot is undefined.
#
#   CUP is of format "cup.index". An optional 'A' can be used as the first
#	character to indicate an arena. However, this indicator is ignored if
#	TYPE is present. Set it to "-" if the cup is unknown.
#
# All other characters are read in as the NAME of the track.
#
# The new format is compatible with Wiimmfi.de for region limits.

# Please edit the CUP column if missed. Use »CUP.INDEX« (e.g. »1.2« or »20.4«)
# for racing tracks and »'A' CUP.INDEX« (e.g. »A2.5«) for battle arenas.

