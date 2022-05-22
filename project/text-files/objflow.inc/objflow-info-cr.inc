#OBJFLOW <<<  The first 8 characters are the magic for OBJFLOW text files.
#        <<<  Don't remove them!
#
#------------------------------------------------------------------------------
#
# This is the textual representation of an ObjFlow.bin file.
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
# Beside of emtpy lines and comments, there are 2 kinds of parameters.
# Both start with an keyword (case insensitive):
#
#   'OBJECT' ID, NAME, RESOURCES
#	ID is the object ID to identify the object. Use prefix '0x' for hex
#	numbers. NAME is the object name. If not set, the original name is
#	used. RESOURCES is the resources string. If not set, the original
#	string is used. Commas are mandatory to separate string parameters.
#
#   'PARAM' P0, P1, P2, P3, P4, P5, P6, P7, P8
#	Define up to 9 parameters of the previous defined 'OBJECT'.
#	A point ('.') is interpreted as Null, and an equal sign ('=')
#       leaves the value unchanged. Missed parameters are not modified.
#	Commas are optional if expression is not ambiguous.
#
# See https://wiki.tockdom.com/wiki/ObjFlow.bin for more details.
#
#------------------------------------------------------------------------------
