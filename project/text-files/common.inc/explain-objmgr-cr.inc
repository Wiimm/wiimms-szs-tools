
#------------------------------------------------------------------------------

# Usually, only the data of objects defined by Nintendo can be replaced.
# It is not possible to remove or add an object, or to replace an object
# with one ID by another object with another ID. The following settings
# override this protection for experimental usage.

# @NEW-OBJECT = MODE
#   MODE is one of 4 settings:
#	NEWOBJ$OFF:
#	    Disable experimental mode. This is the default.
#	    Ignore @OBJECT-SLOTS and @MAX-OBJECT-ID.
#	NEWOBJ$REPLACE:
#	    Allow new objects, but keep the total number of slots
#	    at 0x13c. Ignore @OBJECT-SLOTS.
#	NEWOBJ$SHRINK:
#	    Allow new objects and removal of old objects. The total number
#	    of slots may shrink, but not grow above 0x13c.
#	NEWOBJ$GROW:
#	    Allow new objects and removal of old objects. The total number
#	    of slots may shrink or grow (limit 0x400).

# @OBJECT-SLOTS = NUMBER
#   Number of available objects slots. The default is 0x13c. Up to
#   0x400 slots are supported. This setting is ignored for some modes
#   of @NEW-OBJECT.

# @MAX-OBJECT-ID = NUMBER
#   Define the maximim allowed object id. The default value is 0x2f3 and the
#   maximal possible value is 0x3ff.
#   Objects with id larger than this value are ignored. The value defines the
#   size of the reference list of section 2 too.
