
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
# all 3 axis are between -131072 and +131071. The following vector defines 3
# stretch factors to increase the range at the expense of accuracy. Factors
# <1.0 are ignored and set to 1.0.

@ITEM-POS-FACTOR = v( %5.3f, %5.3f, %5.3f )

# Wiimmfi and tool mkw-ana support an automatic ban systems, that detect items
# before race start. Unfortunately, there are some tracks where players get
# an item through an item box before race start. And it is precisely these
# tracks that must be marked with value 1 as an exception to avoid banning.

@START-ITEM = %u

# If playing online, the racing time is limited. The original MKW uses 5:00
# and LE-CODE 5:40 by default. A racing track can apply another online limit
# if the following setting is >0. Values between %us%s and %us%s
# are accepted. LPAR settings MIN-ONLINE-SEC and MAX-ONLINE-SEC limit the
# actually used value.

@APPLY-ONLINE-SEC = %u%s

