
#FF#
###############################################################################

[CKPT]
# Check points
# KMP/CKPT syntax & semantics: https://szs.wiimm.de/doc/kmp/ckpt

@REVISION        = %u
@CKPT-HEAD-VALUE = %#x
@CKPH-HEAD-VALUE = %#x
@AUTO-NEXT       = %u
@AUTO-GROUP-NAME = 0
@AUTO-RESPAWN    = 0	# 0:off, 1:on

@AUTO-MODE = mode$automode
# allowed values: AM$OFF AM$NORM AM$RENUMBER AM$1LAP AM$SHORT AM$UNLIMIT

@ROUTE-OBJECT-MODE   = mode$showck
#--- default settings:
# @ROUTE-OBJECT1       = o$%s
# @ROUTE-OBJECT2       = o$%s
# @ROUTE-OBJECT1-SCALE = v( %4.2f, %4.2f, %4.2f )
# @ROUTE-OBJECT2-SCALE = v( %4.2f, %4.2f, %4.2f )
# @ROUTE-OBJECT1-BASE  = %d
# @ROUTE-OBJECT2-BASE  = %d

#CKPT#
#----------------------------------------------------------------------#-------------------
#      _____left_point______   _____right_point_____ respawn   prev    #   width     direc-
#idx        x           y           x           y          mode   next #  of ckpt     tion
#----------------------------------------------------------------------#-------------------
