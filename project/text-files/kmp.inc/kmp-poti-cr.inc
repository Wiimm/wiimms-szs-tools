
#FF#
###############################################################################

[POTI]
# Routes
# KMP/POTI syntax & semantics: https://szs.wiimm.de/doc/kmp/poti

@REVISION        = %u
@AUTO-ROUTE-NAME = 0

# If a route is closed, 3 global variables are defined for the route.
# 'NAME' is a placeholder for the route name (before renaming happens):
#   NAME.len      = Total route distance over all points.
#   NAME.time     = Total route time in 1/60 seconds calculated by the sum of
#                   all distances and the associated speed values.
#   NAME.time.set = Same value as 'NAME.time' (but see below).

# These 15 commands that are computed after closing a route in this order:
#	@KCL-FALL-WIDTH     = CUBE_WIDTH
#	@KCL-FALL-PRE-ADD   = HEIGHT_OR_VECTOR
#	@KCL-FALL-POST-ADD  = HEIGHT_OR_VECTOR
#	@KCL-FALL-TYPES     = TYPEMASK
#	@XSS                = X1OLD,X1NEW, X2OLD,X2NEW
#	@YSS                = Y1OLD,Y1NEW, Y2OLD,Y2NEW
#	@ZSS		    = Z1OLD,Z1NEW, Z2OLD,Z2NEW
#	@SCALE              = FACTOR [ORIGIN]
#	@SHIFT              = DELTA
#	@HROTATE            = DEGREE [ORIGIN]
#	@REVERSE-ORDER	    = 0|1
#	@SET-ROUTE-TIME     = SECONDS
#	@ROUTE-OBJECT       = 0=OFF | -1=USE_DEFAULT_SETTINGS | >0=OBJECT_ID
#	                    # Example: mode$showrt & %u ? -1 : 0
#	@ROUTE-OBJECT-SHIFT = VECTOR
#	@ROUTE-OBJECT-SCALE = VECTOR
# The commands are reset after computations.

#POTI#
#------------------------------------------------------#--------------------------
#      _____________position_____________  value  un-  # seconds+distance   horiz
#idx         x           y           z     1/60s known #      to next       angle
#------------------------------------------------------#--------------------------
