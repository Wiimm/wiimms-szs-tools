
#FF#
###############################################################################

[GOBJ]
# Global objects.
# KMP/GOBJ syntax & semantics: https://szs.wiimm.de/doc/kmp/gobj

@REVISION          = %u
@GOBJ-HEAD-VALUE   = %#x
@AUTO-ENEMY-ITEM   =   0	# mode$test ? <item$star,item$mega> : 0
@AUTO-OBJECT-NAME  =   0
@KCL-FALL-WIDTH    =   0	# set to a value >0 (e.g. 200) to enable KCL falling
@KCL-FALL-PRE-ADD  =   0	# add this height or vector before falling
@KCL-FALL-POST-ADD =   0	# add this height or vector after success
@KCL-FALL-TYPES    = KCL$DRIVE	# bit field of recognized KCL types
	# Best values are:
	#  - KCL$BORDER: All KCL types except effects.
	#  - KCL$DRIVE:  All driveable types like road, boost, ramp, water, ...
@SORT-OBJECTS      = SORT$OFF	# SORT$GROUPING SORT$ANGLE SORT$TINY

#------------------------------------------------------------------------------
# Definition objects and references

# There is a new kind of presence flags in progress:
#   http://wiki.tockdom.com/wiki/Extension_of_Presence_Flags
# Since v2.11a, the KMP compiler supports this new extension.
# Here you find some global settings for the new extension:

# If >0: Reset not defined parameters of definitions-object to their default
# values.
@FIX-OBJDEF = 0

# If not DOB$OFF: Sort definitions-objects by object-id and move them to the
# beginning (DOB$BEGIN) or to the end (DOB$END) of all GOBJ elements.
@SORT-OBJDEF = DOB$OFF

# If >0: Clear invalid reference-ids for modes 0 and 1.
# If >1: Clear invalid reference-ids independend of the mode.
@FIX-REFERENCE = 0

# If not DOB$OFF: Sort standard objects with a valid reference-id directly
# before (DOB$BEFORE) or behind (DOB$BEHIND) the related definition-object.
@SORT-REFERENCE = DOB$OFF

# Object statistics:
##--------------------------------------------------------------------------------------------
##count id/hex  name               characteristics
##--------------------------------------------------------------------------------------------
