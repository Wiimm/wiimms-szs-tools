
#FF#
###############################################################################

[STGI]
# Stage information
# KMP/STGI syntax & semantics: https://szs.wiimm.de/doc/kmp/stgi

@REVISION        = %u
@STGI-HEAD-VALUE = %#x

#STGI#
#-----------------------------------------------------------------------------
# Write a floating point number (a '.' is mandatory) instead of the
# 2 last bytes, and it will be rounded and accepted as speed modifier.
#-----------------------------------------------------------------#-----------
#     byte  byte  byte  byte      u32     byte   byte  byte byte  #  last 2
#     laps  pole narrow flare    flare    flare  pad       or     # bytes as
#idx  1..9  1=rt  if 1  1=on     color    alpha    0   speed mod  # speed mod
#-----------------------------------------------------------------#-----------
