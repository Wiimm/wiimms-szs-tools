
#FF#
###############################################################################

[KTPT]
# Kart Points: Starting positions of the racers
# KMP/KTPT syntax & semantics: https://szs.wiimm.de/doc/kmp/ktpt

@REVISION          = %u
@KTPT-HEAD-VALUE   = %#x
@KCL-FALL-WIDTH    =   0	# set to a value >0 (e.g. 200) to enable KCL falling
@KCL-FALL-PRE-ADD  =   0	# add this height or vector before falling
@KCL-FALL-POST-ADD =   0	# add this height or vector after success
@KCL-FALL-TYPES    = KCL$DRIVE	# bit field of recognized KCL types
	# Best values are:
	#  - KCL$BORDER: All KCL types except effects.
	#  - KCL$DRIVE:  All driveable types like road, boost, ramp, water, ...

#KTPT#
#--------------------------------------------------------------------------------
#     _____________position______________  _______rotation_______  player
#idx         x           y           z         x       y       z    index unknown
#--------------------------------------------------------------------------------
