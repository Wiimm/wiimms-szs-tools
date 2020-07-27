
@REVISION           = %u
@%s-HEAD-VALUE    = %#x
@%s-HEAD-VALUE    = %#x
@AUTO-FILL          =   0.0      # >1 (e.g. 5000): create more points if distance is larger
@AUTO-GROUP-NAME    =   0
@ROTATE-GROUP-LINKS =   0
@ROUTE-OBJECT       = mode$showrt & %u ? -1 : 0  # 0=off, -1=use default, >0=object id
@ROUTE-OBJECT-SHIFT = $NONE      # v3(0)  # add vector to position
@ROUTE-OBJECT-SCALE = $NONE      # v3(1)  # define vector scale

# KCL-FALL (automatic height correction)
# Define the following values here or at any place between the points.
# They are valid until re-defined. Disable this feature by @KCL-FALL-WIDTH=0.
@KCL-FALL-WIDTH     =   0        # set to a value >0 (e.g. 200) to enable KCL falling
@KCL-FALL-PRE-ADD   =   0        # add this height or vector before falling
@KCL-FALL-POST-ADD  =   0        # add this height or vector after success
@KCL-FALL-TYPES     = KCL$DRIVE  # bit field of recognized KCL types
	# Best values are:
	#  - KCL$BORDER: All KCL types except effects.
	#  - KCL$DRIVE:  All driveable types like road, boost, ramp, water, ...

# Column 'flag' is optional and usually only used internally.
# Syntax:   FLAGS_STRING   or   '(' EXPRESSION ')'
#   F (F$FALL): Point can be used for FALL-DOWN functionality.
#   S (F$SNAP): Point can be used for SNAP functionality (ignored by ITPT).
#   J (F$JGPT): Point can be used for automatic JGPT creation.
# Remove the letter or replace it by '-' to disable a feature.
# If the column is missed, all options are enabled by default.

# If you enable EXPORT-FLAGS (set it to 1), the flags are exported using the
# lowest byte of WIDTH FACTOR. The factor needs only 2 or 3 digits, so this
# has no disadvantage on the width factors. 5 of the 8 bits are used to verify
# changes by other tools on import. If at least 1 of these bit of all points
# is wrong, import is canceled.
@EXPORT-FLAGS = %d  # 0=disabled, 1=enabled (overridden by option --export-flags)
