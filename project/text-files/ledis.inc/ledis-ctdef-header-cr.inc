%s

#------------------------------------------------------------------------------
#
# This file was created by »wlect distrib ... CTDEF=...«. It is not guaranteed
# to correctly represent all features of the distribution. This applies in
# particular to the slot arrangement. In addition, only the property slots can
# be defined for the battle arenas.
#
# As an alternative, the use of LEDEF= instead of CTDEF= is recommended.
#
# Due to some new parameters, this file can only be read with the tools since
# version 2.28a.
#
#------------------------------------------------------------------------------

[RACING-TRACK-LIST]

# Define the maximum number of strings of track definition:
%%N-STRINGS	= 4

# Enable support for LE-CODE flags (0=no, 1=yes):
%%LE-FLAGS	= 1

# Auto insert a Wiimm cup with 4 special random slots (0=no, 1=yes):
%%WIIMM-CUP	= %d

# Standard setup
N N$SWAP | N$F_WII

