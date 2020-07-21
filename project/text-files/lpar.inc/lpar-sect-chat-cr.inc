
# LE-CODE supports a special feature for chat messages in private rooms. If a
# message is send by the host and at least one guest is present, then some
# messages change the behavior of the game. This section defines the special
# function.

# The first parameter resets all message to CHAT$OFF (no function).
# The default is value 1. If set to 0, the current map is edited.
@reset = 1

# Set the value to 1 if you want to use the old behavior defined for MKW-Fun.
@legacy = 0

# And now define the functions for different chat messages. The syntax is:
#
#	MESSAGE '=' EXPRESSION1
#	MESSAGE '=' EXPRESSION1 ',' EXPRESSION2
#
# MESSAGE is the letter 'M' followed by a number between 1 and 96. Examples
# are M1, M02, M96. EXPRESSION is any expression supported by the parser.
# Up to 2 modes (separated by a comma) can be defined for each message.
# Usually the following constants or functions are used:
#
#   CHAT$TRACK_BY_HOST : The host selects a tracks and wins the lottery.
#                        The track selection for guests is disabled. 
#   CHAT$ANY_TRACK     : Cancel CHAT$TRACK_BY_HOST.
#
#   CHAT$BLOCK_CLEAR   : Clear the used-tracks list.
#   CHAT$BLOCK_DISABLE : Disable the block-track functionality.
#   CHAT$BLOCK_ENABLE  : Enable the block-track functionality. This is the
#                        default, but have only impact, if BLOCK-TRACK > 0.
#
#   CHAT$KARTS_ONLY    : Each player must select a kart.
#   CHAT$BIKES_ONLY    : Each player must select a bike.
#   CHAT$ANY_VEHICLE   : Cancel vehicle requirement.
#
#   CHAT$USE_ENGINE_1  : Select first engine class  (100cc or 150cc if 200cc enabled)
#   CHAT$USE_ENGINE_2  : Select second engine class (150cc or 200cc if 200cc enabled)
#   CHAT$USE_ENGINE_3  : Select third engine class  (mirror)
#   CHAT$RESET_ENGINE  : Cancel engine selection.
#
#   chat$n_races(N)    : Define the number of races in the next Grand Prix.
#                        N is a number between 1 and 512 (inclusive). 
#
#   chat$vehicle(...)  : Define a chat modus for a vehicle group. 0 to N values
#                        are expected as function parameters. Each parameter is
#                        either from the group VEH$SMALL, VEH$MEDIUM, VEH$LARGE
#                        and VEH$ANY_SIZE for size selections and/or from the
#                        group VEH$KART, VEH$BIKE, VEH$ANY_TYPE for type
#                        selections. VEH$ANY resets the limitations.
~#VEH$OUT_BIKE, VEH$IN_BIKE
#
#   CHAT$RESET         : Reset all settings to their defaults.
#
# See http://wiki.tockdom.com/wiki/LE-CODE/Distribution_Tutorial#chatmsg
# for more details.
