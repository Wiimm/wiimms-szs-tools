
@REVISION           = %u
@%s-HEAD-VALUE    = %#x
@AUTO-ID            = %u

# Activate automatic Y-correction (KCL fall)
@KCL-FALL-WIDTH     =   0        # set to a value >0 (e.g. 200) to enable KCL falling
@KCL-FALL-PRE-ADD   =   0        # add this height or vector before falling
@KCL-FALL-POST-ADD  =   0        # add this height or vector after success
@KCL-FALL-TYPES     = KCL$DRIVE  # bit field of recognized KCL types
	# Best values are:
	#  - KCL$BORDER: All KCL types except effects.
	#  - KCL$DRIVE:  All driveable types like road, boost, ramp, water, ...

# Snap to nearest point of the enemy route and add the vector.
# Activate it with something like: vy(200) => 200 units higher than the enemy point
@HSNAP-TO-ENPT      = $NONE      # horizontal snap without adjust, y-rotation
@SNAP-TO-ENPT       = $NONE      # 3D snap, y-rotation
@SNAP2-TO-ENPT      = $NONE      # 3D snap, x- and y-rotation

# Show points in the track by using objects
# Values: 0=off, -1=use default (block of ghost value), >0=force object with 'id'
@ROUTE-OBJECT       = mode$showrt & %u ? -1 : 0
@ROUTE-OBJECT-SHIFT = $NONE      # v3(0)  # add vector to position
@ROUTE-OBJECT-SCALE = $NONE      # v3(1)  # define vector scale
