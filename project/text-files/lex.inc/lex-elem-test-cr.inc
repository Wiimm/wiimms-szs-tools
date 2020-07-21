
#FF#
###############################################################################

[TEST]
# Sort order: %u

# This section defines various settings to test special situations.

#-----------------------------------------------------------------------
# The next set defines parameters to test the extended conditions
# (extended presence flags). The settings allow to force special modes.
# Details: http://wiki.tockdom.com/wiki/Extended_presence_flags
#-----------------------------------------------------------------------

# Force offline or online mode. Use one symbol of: LEX$OO_AUTO (default),
# LEX$OO_OFFLINE, LEX$OO_ONLINE.
@OFFLINE-ONLINE = LEX$OO_%s

# If >0: Force number of local players at Wii. It is robust, so values >4
# are accepted, but rounded down to 4 (offline) or 2 (online).
@N-OFFLINE = %u

# If >0: Force number of total players for online modes. It is robust, so
# values >99 are rounded down to 99.
@N-ONLINE = %u

# If between 0 and 15: Use this bit number to check 16-bit words of CONDITIONS
# and ignore previous settings. Good values are: LEX$CB_AUTO, LEX$CB_TIMETRIAL
# (same as LEX$CB_RESERVED1) and LEX$CB_RESERVED2.
@COND-BIT = %d

# Force a game mode. Use one symbol of: LEX$GM_AUTO (default), LEX$GM_BALLOON,
# LEX$GM_COIN, LEX$GM_VERSUS, LEX$GM_ITEMRAIN or LEX$GM_STANDARD.
@GAME-MODE = LEX$GM_%s

# Force an engine mode. Use one symbol of: LEX$EN_AUTO (default),
# LEX$EN_BATTLE, LEX$EN_50CC, LEX$EN_100CC, LEX$EN_150CC or LEX$EN_200CC.
# For mirror modes use LEX$EN_150M or LEX$EN_200M.
@ENGINE = LEX$EN_%s

# If between 1 and 8: Force a random scenario.
@RANDOM = %u
