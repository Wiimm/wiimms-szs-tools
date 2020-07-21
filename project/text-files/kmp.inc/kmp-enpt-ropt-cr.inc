
# Experimental options, tools before v1.60a will throw error messages!
@SHOW-OPTIONS = %d   # >0: force printing of all route options

# AUTO-CONNECT is one of the following AC$* symbols:
# For racing tracks and for battle areenas with manual links setup:
#   AC$OFF
#	Do not any auto connection. Route connection are fully under control
#	of the user.
#   AC$PREV
#	Setup PREV links by NEXT links of other routes. Existing PREV links
#	are cleared before. This is the default and the behavior of all versions
#	before v1.60a.
#
# Modes for automatic route connection in battle arenas:
#   AC$DISPATCH
#	Find dispatch points and create links automatically if not already
#	set manually. This is the default for enemy routes, if dispatch
#	points detected.
#
# Flags: The modes of above can be combined with the following options by
# the OR operator '|' (example: AC$PREV | ACF$FIX)
#   ACF$FIX_PREV
#	Add self-links, if a PREV list is empty.
#   ACF$FIX_NEXT
#	Add self-links, if a NEXT list is empty.
#   ACF$FIX
#	Abbreviation for: ACF$FIX_PREV | ACF$FIX_NEXT
#	This is the default for item routes to avoid a freeze.
#
@AUTO-CONNECT = %s%s

# Syntax of new command with impact to all following $GROUP commands:
#  "$DEF-CLASS" [":"] class_name

# Syntax of new route commands (placed behind $GROUP command):
#  "$PREV"      [":"] [group_name]...
#  "$SETTINGS"  [":"] setting1 setting2 [ "AUTO" | "ROUTE" | "DISPATCH" ]
#  "$CLASS"     [":"] class_name_prev [class_name_next]
#  "$AC-PREV"   [":"] class_name
#  "$AC-NEXT"   [":"] class_name
#  "$ONEWAY"    [":"] "NONE" | "PREV" | "NEXT" | "BOTH"
