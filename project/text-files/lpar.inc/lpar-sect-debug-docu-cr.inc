
#FF#
###############################################################################

[DEBUG-DOCU]
# This section has no impact!

# If the general parameter DEBUG (see section [LECODE-PARAMETERS]) is set to
# a value ≥1, then additional information is added below the speed (even if
# speedometer is disabled). The intention is to debug tracks.

# For DEBUG=1, the output format is defined in this section. Up to %d lines
# can be defined.

#------------------------------------------------------------------------------

# SETUP = DEBUG$...
# Usually, a debug section is edited, so that not specified parameters are
# not modified (same as DEBUG$OFF). With SETUP a section can be initialized:
#
#  DEBUG$OFF      : Do nothing.
#  DEBUG$CLEAR    : Clear all data.
#
#  DEBUG$STANDARD : Use a standard debug line for the player.
#  DEBUG$OPPONENT : Use a standard debug line for the player and one more line
#		    for the first opponent.
#  DEBUG$VERTICAL : Arrange the output vertically for the player.

#------------------------------------------------------------------------------

# LINE = -1 | 0..%d
# LINE selects a line for the next parameters. Lines are numbered from 0 to %d.
# If set to -1, then ignore the following settings.

# ENABLED = 0|1
# ENABLED defines, if this section becomes active. If set to 0 or if all other
# parts are disabled, then the debug line is not printed. So you can disable a
# line without changing the other parameters.

# OPPONENT = 0|1
# It is possible to display the data of the current player (0), or of the
# first opponent (1). The intention is watch onlyine 2 players together.
# The font color for current player if yellow and for the opponent cyan.

# SPACE = 0|1
# ???

# POSITION = 0|1
# If enabled (1), then print the position of the player. Values outside the
# range ±131071 are highlighted to point to a item position bug. The output
# format is »P=x,y,z«.

# CHECK-POINT = 0..3
# If enabled (1,2,3), then print the checpoint info of the player. The output
# format is either »CK=c« (1), »CK=c,k« (2) or »CK=c,k^m«. »c« is the current
# check point. »k« is the last relevant key check point. »m« is the highest
# reached key check point in the current lap. If driving backwards, »k« is
# decreased, but »m« not. 

# RESPAWN = 0|1
# If enabled (1), then print the current respawn point as »R=num«.

# LAP-POS = 0|1
# If enabled (1), then print the current calculated lap position as a floating
# point number with 3 fraction digits (1.234). This number is calculated by
# Mario Kart Wii to compare positions.
