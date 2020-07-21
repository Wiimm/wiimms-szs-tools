
# Automatic definition of respawn points
@AUTO-DEFINE	= 0		# 0 | SECT$ENPT | SECT$ITPT
@AUTO-DISTANCE	= 1000.0	# minimal distance to other JGPT
@AUTO-ADJUST	= vy(100)	# add this value to each auto-JGPT
@AUTO-REMOVE	= 0		# 0|1|2

# Column 'flag' is optional and only used internally.
# Syntax:   FLAGS_STRING   or   '(' EXPRESSION ')'
#   F (F$FALL): Point can be used for FALL-DOWN functionality.
#   S (F$SNAP): Point can be used for SNAP functionality.
# Remove the letter or replace it by '-' to disable a feature.
# If the column is missed, all options are enabled by default.

# If you enable EXPORT-FLAGS (set it to 1), the flags are exported using the
# lowest byte of ROTATION.x. The rotation value needs only 4 or 5 digits, so
# this has no disadvantage for the rotation. 5 of the 8 bits are used to
# verify changes by other tools on import. If at least 1 of these bits of all
# points is wrong, import is canceled.
@EXPORT-FLAGS = %d  # 0=disabled, 1=enabled (overridden by option --export-flags)

#JGPT#
#------------------------------------------------------------------------------------
#      _____________position_____________ _______rotation________ related      flags
# idx        x           y           z        x       y       z       id effect
#------------------------------------------------------------------------------------
