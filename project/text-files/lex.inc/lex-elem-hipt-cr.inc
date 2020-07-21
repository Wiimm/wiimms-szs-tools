
#FF#
###############################################################################

[HIPT]
# Sort order: %u

# Hide the position tracker based on online mode, the current lap index and
# the current checkpoint of the player.
#
# Each row is a rule. If a player enters a new check point, the first matching
# rule is used. Therefor the current mode (offline or online), the current lap
# index and the index of the current CKPT is compared with the row elements
# COND, LAP, FROM and TO. Lap is 0 until crossing the lap counter the first
# time. This includes the start position.
#
# Row members:
# COND is either 1 for offline only, 2 for online only, or 3 for both cases.
# LAP is compared with the current lap index. Negative values refer to the
# absolute number of laps: -1 is always the last lap and -2 is the penultimate
# lap. This means that the setting remains valid if the number of laps (STGI)
# is changed. The special value 99 means 'match all laps'.
# FROM and TO build an inclusive range to compare the current check point.
# SHOW is either 0 or 1 to hide or show the position tracker. If no rule
# matches, the position tracker is shown (like rule »3 99 0 255 1«).
#
# Notes: Place the most specific rules above and more general rules below on
# the list because of the first-hit search. Parameter @OFFLINE-ONLINE of
# section TEST can be used to force a condition for testing.

#HIPT#
#--------------------------
#           __CKPT__
# cond lap  from  to  show
#--------------------------
