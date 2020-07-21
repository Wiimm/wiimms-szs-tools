
##############################################################################
#####     All behind a '#' on the same line is a comment and ignored.    #####
#####     See https://szs.wiimm.de/doc/kcl/flagfile for more details.    #####
##############################################################################
# function f()

# First, we define a flag function named f().
# This allows us to define type and variant as 2 values.

@function f # (type,variant)
    @return $1 & 0x1f | $2 << 5
@endfunction

##############################################################################
# function a()

# Here we define function a() to support the flag variant type A.
# For details see http://wiimm.de/r/ctw/KCL_flag

@function a # (type,w,x,y,z)
    @return $1 & 0x1f		# type (5 bits)
	> | ( $2 & 7 ) << 13	# W (3 bits)
	> | ( $3 & 3 ) << 11	# X (2 bits)
	> | ( $4 & 7 ) << 8	# Y (3 bits)
	> | ( $5 & 7 ) << 5	# Z (3 bits)
@endfunction

##############################################################################
# function i()

# The function i() supports fall boundaries.

@function i # (type,z,index)
    @return $1 & 0x1f		# type (5 bits)
	> | ( $2 &    7 ) << 5	# Z (3 bits)
	> | ( $3 & 0xff ) << 8	# index (8 bits)
@endfunction

##############################################################################
# table

#-----------------------------------------------------------------------------
# group_name			= flag_or_expression
#				= f(type,variant)
#				= a(type,w,x,y,z)
#				= i(type,z,index)
#-----------------------------------------------------------------------------
