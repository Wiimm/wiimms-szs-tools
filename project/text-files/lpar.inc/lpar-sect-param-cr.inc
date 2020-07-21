
# This section defines general LE-CODE parameters. The corresponding options
# have a higher priority. Remove or comment-out a parameter if its value
# should not be changed. All values will be limited to the allowed ranges.

#------------------------------------------------------------------------------

# First we define a limitation mode. This mode defines the allowed values for
# the other parameters of this section:
#
#   LE$PRODUCTIVE   : Exclude test and experimental values.
#   LE$TESTING      : Allow test values, but exclude experimental values.
#   LE$EXPERIMENTAL : Allow all values.
#   LE$AUTOMATIC    : Enable automatic detection by analyzing all values.
#
# Values set by an option are never limited. See parameter descriptions for
# limitation details.

LIMIT-MODE	= %s

#------------------------------------------------------------------------------

# Define probabilities for engine classes. A list for »100cc,150cc,mirror« is
# expected. If 200cc is enabled, the values are for »150cc,200cc,mirror«. Any
# number is accepted. They are normalized to get a total of 100 percent.
# Use »0,1,0« to force 150cc. Use »0,0,0« to reset to Nintendo's VR based
# choice. Option --engine will override these settings.

ENGINE		= %u,%u,%u

# Enable (1) or disable (0) 200cc support. Option --200cc will override this
# setting. LE-CODE hasn't implemented this feature yet!

ENABLE-200CC	= %u

# Enable (1) or disable (0) the performance monitor at Wii and Wii U. Enabling
# is only possible if LIMIT-MODE is LE$TESTING or LE$EXPERIMENTAL. The special
# value 2 enables it for Dolphin too, but only on LE$EXPERIMENTAL. It doesn't
# work correctly with Dolphin. Option --perf-mon will override this setting.

PERF-MONITOR	= %u

# Enable (1) or disable (0) Time Trial for custom tracks. Option --custom-tt
# will override this setting.

CUSTOM-TT	= %u

# Enable (1) or disable (0) support for Extended Presence Flags. Disable it
# only for tests! If LIMIT-MODE==LE$PRODUCTIVE, XPFLAGS is always enabled.
# Option --xpflags will override this setting.

XPFLAGS		= %u

# Define the number of races that a previously used track is blocked. LE-CODE
# will try to find another possible track among the selected tracks. Values
# between 0 (deactivated at all) and %u are allowed. Chat messages can clear
# the track list and enable or disable this feature.

BLOCK-TRACK	= %u

