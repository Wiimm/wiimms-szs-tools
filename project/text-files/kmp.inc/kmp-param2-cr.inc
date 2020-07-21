
# First we define default values (if not set by --const) for all parameters:

  @def slot	?= 0	# Enable support for special slots
  @def d	?= 0	# Enable support for _d files
  @def arena	?= 0	# Enable battle arena support and allow different modes
  @def test	?= 0	# Enable test mode
  @def ocam	?= 0	# Test opening cam, only relevant if test>0
  @def showrt   ?= 0    # Show item boxes for route points.
			# Only relevant if test>0. Set bit combination:
			#   1:ENPT, 2:ITPT, 4:JGPT, 8:CNPT, 0x10:MSPT, 0x20:POTI (-1:all)
  @def showck   ?= 0    # Show objects a check point positions if test>0.
			#   0:off, 1:mandatory, 2:all, 3=left+right
  @def laps	?= 3	# Number of laps (0|1|3|4), only relevant if test>0

# In a second step, global mode variables are defined and normalized.
# All conditions in this file should use these variables.

  @gdef mode$slot	= slot > 0 ? slot : 0
  @gdef mode$d		= d > 0
  @gdef mode$arena	= arena > 0 ? arena : 0

  @gdef mode$test	= test > 0 ? test : 0
  @gdef mode$ocam	= mode$test && ocam >= 1 ? ocam : 0
  @gdef mode$laps	= mode$test && laps >= 0 ? laps : 3
  @gdef mode$showrt	= mode$test ? showrt : 0
  @gdef mode$showck	= mode$test ? showck : 0
  @gdef mode$automode	= select(mode$laps,AM$SHORT,AM$1LAP,AM$OFF,AM$OFF,AM$UNLIMIT)
