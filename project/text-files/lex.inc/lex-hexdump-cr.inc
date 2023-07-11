
#FF#
###############################################################################

[HEXDUMP]
# Sort order: %u

# Unknown sections are stored as hexdump to keep the data. Parameter @MAGIC
# defines the type of the section.

@MAGIC = 0x%08x # %s

# Syntax of the data lines:
#	HEXLINE := [ ADDRESS ':' ] DATA [COMMENT]
#	COMMENT := ( [ ':' | '#' | '/' ) ANY_TEXT
#	DATA    := HEXBYTE | EXPR
#	EXPR    := '(' SZS_TOOLS_EXPRESSION ')'
#
# The address field and the ASCII part (a COMMENT by definition) are for info
# only and ignored on scanning.
#
# The result of SZS_TOOLS_EXPRESSION is stored in network byte order (big
# endian). Integers and floats are stored as 4 bytes, vectors as 3 floats with
# 12 bytes total. Strings are stored as bytes without terminating NULL.

#HEX#
#----+------------------------------------------------------+----------------+
#addr:   data:  byte in hex  |  '(' expression ')'          : ASCII of data  :
#----+------------------------------------------------------+----------------+
