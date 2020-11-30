
#FF#
###############################################################################

[SET1]
# Sort order: %u

# This section defines various settings. New settings are placed at the end
# end of the binary data. Unknown data is printed as hexdump, so no data is
# lost even with old tools. A value of 0 means: Not defined/initialized,
# replace the value by the default value (maybe 0 again).

#-------------------------

# Online, item positions are only transmitted correctly, if the positions of
# all 3 axis are between -131071 and +131071. The following vector defines 3
# factors to increase the range at the expense of accuracy. Factors <1.0 are
# ignored and set to 1.0.

@ITEM-POS-FACTOR = v( %5.3f, %5.3f, %5.3f )
