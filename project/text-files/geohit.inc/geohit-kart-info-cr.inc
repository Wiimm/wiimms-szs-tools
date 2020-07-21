%s <<<  The first 8 characters are the magic for text files of GeoHit-Kart.
#        <<<  Don't remove them!
#
#------------------------------------------------------------------------------
#
# This is the textual representation of a GeoHitTableKart*.bin file.
# It is defined by Wiimm and scanned by a 2-pass parser.
#
# Hints for decoding to a text file:
#
#  - If either option --no-header (-H) or option --brief (-B) is set, this
#    general description is suppressed.
#
#  - If either option --export (-X) is set once or option --brief (-B) is set
#    twice, a very small text file without descriptions and other not needed
#    parameters is created.
#
#  - If option --long (-l) is set, then additional hidden parameters are
#    included. They are automatically included, if they are differ from the
#    the standard settings.
#
#------------------------------------------------------------------------------
#
# Syntax for section [OBJECTS]:
#
#	OBJ_ID  P1 P2 P3 P4
#
#  OBJ_ID : Id of the object. Use prefix '0x' for hex numbers.
#  P1..P4 : 4 hit parameters, optionally separated by commas.
#           A point ('.') is interpreted as Null, and an equal sign ('=')
#           leaves the value unchanged. Missed parameters are not modified.
#
# See http://wiki.tockdom.com/wiki/GeoHitTableKart.bin
# and http://wiki.tockdom.com/wiki/GeoHitTableKartObj.bin for more details.
#
#------------------------------------------------------------------------------
