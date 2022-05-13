
#FF#
###############################################################################

[DEBUG-DOCU]
# This section is documentation only and has no impact!

# If the general parameter DEBUG (see section [LECODE-PARAMETERS]) is set to
# a value ≥1, then additional information is added below the speed (even if
# speedometer is disabled). The intention is to debug tracks.

# 4 debug displays with up to %u lines each can be defined.

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

# HIDE-SPEEDO = 0 | 1
# This global command is independent from lines. If set and this configuration
# becomes active, then hide the speedometer. This helps if many elements are
# active. Otherwise a smaller font is selected by the system. 
    
#------------------------------------------------------------------------------

# LINE = -1 | 0..%d
# Select a line. All following commands until the next LINE command disable or
# enable the output for the current line. Lines are numbered from 0 to %d. If
# an invalid index is entered, the following commands are ignored. So set
# LINE=-1 to disable a complete line. 

# ENABLED = 0|1
# ENABLED defines, if the current lines becomes active. If set to 0 or if all
# other parts are disabled, then the debug line is not printed. So you can
# disable a line without changing the other parameters.

# OPPONENT = 0|1
# Usually the data of the current player is printed (font color yellow). If
# OPPONENT is enabled, then print the data of the first opponent and use a
# blue font color as indicator. 

# SPACE = 0|1
# Usually the lines are separated by a line terminator (line feed). If SPACE
# is set, then a space is used instead, so that the previous and the current
# line are displayed in one visual line. 

# POSITION = 0|1
#   Output: P=x,y,z
# If enabled then print the current position (x, y and z coordinates) of the
# player. Coordinates outside the range ±131071 (see Item position bug) are
# highlighted by an orange font, or a white font it the position is fixed by
# LEX ITEM-POS-FACTOR.

# CHECK-POINT = 0..3
#   Output: CK=c   |   CK=c,k   |   CK=c,k^m
# If enabled print a check point info. The mode defines how many parts are
# printed. 'c' is the current check point. 'k' is the last relevant key check
# point. 'm' is the highest reached key check point in the current lap. If
# driving backwards, 'k' is decreased, but 'm' not. 

# RESPAWN = 0|1
#   Output: R=r
# If enabled then print the respawn index defined by the current check point. 
# This information may help to test all respawn points of a track. 

# ITEM-POINT = 0|1
#   Output: I=i
# If enabled print the current item point index of the player.

# KCL-TYPE = 0|1
#   Output: K=t,v
# If enabled print current KCL type (t=00-1F) and KCL variant (v=000-7FF).

# LAP-POS = 0|1
#   Output: L=lap
# Print the current lap position as floating point value with 3 digits. The
# integer number reflects the current lap number, and the decimal (0.000 to
# 0.999) the progress within the current lap. This number is calculated by
# Mario Kart Wii to compare positions. Together with the opponent view it
# can help to investigate different paths of a track and to place the check
# points at good positions.

# XPF = 0|1|2
#   Output: X:...
# If enabled, a short (1) or long (2) information about the usage of extended
# presence flags is printed, but only if an information is available. The long
# information is similar to the short one, but with more or extended members.
# See http://wiki.tockdom.com/wiki/LPAR#XPF for details.
