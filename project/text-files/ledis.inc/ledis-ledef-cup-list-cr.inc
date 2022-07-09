
#
################
   [CUP-LIST]
################

# It makes no sense not to fill cups with tracks. Therefore, cups are simply
# filled with tracks in order of definition and new cups are created as
# needed. This is usually done with the command APPEND. Battle arenas and
# versus tracks are considered separately and the tracks are assigned
# accordingly.
#
# Battle and versus cup definitions can be done in any order. Only the
# sequences within battle cups and within versus cups are important.
#
# The command NEW-CUP creates a new battle or versus (vs) cup. Any
# unoccupied places in the current cup remain empty at the moment.
#
# At the end, when all commands have been processed, more cups may be added to
# get an even number of battle cups and of versus cups. Then all cups are
# filled with arenas and tracks. This happens first with tracks not yet used
# (if the related option is activated), and then with tracks already used.
#
# Hidden tracks can't be added to a cup.
#
# Syntax:
#
#   APPEND track...
#	Add each valid track reference to the current cup of correct type.
#	If a cup is full, create a new before adding the track.
#
#   NEW-CUP bt|vs [bt|vs]
#	Close the current battle cup (bt) or versus cup (vs). Use both 'bt'
#	and 'vs' to close the cups of both types.

# There are 4 more commands to insert cups at the current position. They are
# usually used before any other cup definition:
#
#   STANDARD-BATTLE-CUPS
#	Append 10 tracks (slots 32 to 41) to the correct cup type. Usually
#	all tracks are of type battle.
#
#   STANDARD-VERSUS-CUPS
#	Append 10 tracks (slots 0 to 31) to the correct cup type. Usually
#	all tracks are of type versus.
#
#   STANDARD-CUPS
#	Short cut for STANDARD-BATTLE-CUPS and STANDARD-VERSUS-CUPS.
#
#   MKWFUN-RANDOM-CUP
#	Append 4 random slots (slots 62 to 65, like MKW-Fun, always versus)
#	to the versus cups.
#
# If no cup was finally defined, then STANDARD-BATTLE-CUPS and/or
# STANDARD-VERSUS-CUPS will be executed automatically.

# There are 3 options to add cups at the end. Set it to NO (=0) or YES (=1)
# at any place in the section:
#
#   ADD-UNUSED-ARENAS = no/yes
#   ADD-UNUSED-TRACKS = no/yes
#   ADD-UNUSED        = no/yes
#	If defined arenas or tracks except hidden tracks were not used in at
#	least one cup, then add cups with references to these missed tracks.
#	CUPS-FOR-MISSED set both other settings in one step.
#

