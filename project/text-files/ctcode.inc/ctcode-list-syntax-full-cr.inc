
#---------
# Syntax:
#---------
#
# 8 kinds of records are allowed: %, N, C, S, X, T, H and A
#
#  * Option: '%' name = value
#
#	Set option NAME to VAULE. The follwing options are supported:
#
#	  LE-FLAGS = 0 | 1
#	    Enable or disable support of LE-FLAGS member. At section start
#	    it is disabled. If enabled, the Track definition expects an
#	    additional parameter between 'property_id' and 'file_name'.
#
#  * Nintendo track mode: 'N' mode
#
#	This command can only be used once and only before the first track
#	definition. It defines, how to manage Nintendos original tracks.
#	A numerical expression is expected. The 4 symbols are predefined:
#
#	  N$NONE : Nintendos tracks will NOT inserted automatically.
#	           This is the default.
#
#	  N$HIDE : Nintendos tracks will be defined, but the tracks are not
#	           shown in any Cup. If such track is selected by accident,
#	           it can be played.
#		   THIS MODE IS EXPERIMENTAL AND WILL FREEZE THE WII!
#
#	  N$SHOW : Define automatically 8 Cups with Nintendos tracks.
#
#	  N$SWAP : Define automatically 8 Cups with Nintendos tracks, but
#		   change the cup order, so that all 4 Wii Cups are in the
#		   first row and all 4 retro cups in the second row.
#
#	For HIDE+SHOW+SWAP:
#	    The cup and track names are defined by internal names. If BMG is
#	    read, messages may override the internal names.
#	    The track order (except SWAP), file names, music and property ids
#	    are the same as in original MKWii.
#
#	There are some flags, that are OR'ed to the base mode:
#	  |N$F_HEX	: Replace original filenames by a 2 digit slot hex code.
#	  |N$F_WII	: Prefix Wii track names by 'Wii '
#	  |N$F_<color>	: For BMG output, colorize the console prefix. Possible
#			  color names are: RED1, RED2, RED3, RED4, YELLOW, GREEN,
#			  BLUE1, BLUE2, WHITE and CLEAR (transparent).
#
#  * Cup definition: 'C' name_of_cup
#
#	Close the current cup and fill it with default tracks. Then define a
#	cup name for the next created cup. This command is optional and only
#	used, to store names in the CT-CODE file without any impact.
#
#  * Slot definition: 'S' list
#
#	Define the track slots for the next defined tracks. A list element is
#	either an index 'NUMBER' or a range 'NUMBER1:NUMBER2'. The system will
#	automatically skip invalid and already used slots. All numbers are
#	numerical expressions handled by the parser. If a number or range is
#	followed by an '!', the slots are used for the next tracks ignoring
#	previous usage or reservation.
#
#	Each slot definition overrides the previous one and an empty list will
#	disable the slot list. Slot definiton are only needed to recreate a
#	CT-CODE file with identical track layout.
#
#  * Used definition: 'X' list
#
#	Mark the slots as 'reserved'. A list element is either an index 'NUM'
#	or a range 'NUMBER1:NUMBER2'. Only unused slots will be marked. All
#	numbers are numerical expressions handled by the parser. If a number
#	or range is followed by an '!', the slot is marked as used independent
#	of its previous state.
#
#	The line "X 0x20:0x2b 0x43 0xff" is executed internally at startup.
#
#  * Track definition:
#	'T' music_id ';' property_id ';' file_name ';' info ';' id
#	'T' music_id ';' property_id ';' le_flags ';' file_name ';' info ';' id
#
#	Define a new track. Use the next free slot defined by the previous
#	slot definition, or the first not used slot. The elements are:
#
#	 * music_id
#		The numeric ID of the sound file, a numerical expression is
#		expected. The ID is normalized to avoid impossible values.
#		Track numbers (0-31) are translated to the standard music ID.
#		If a music ID is followed by an '!', the value is used without
#		normalization and transformation.
#		See https://szs.wiimm.de/doc/slot-id for infos about names.
#
#	 * property_id
#		The numeric ID of a basic slot (0-31) for the properties.
#		A numerical expression is expected. Only the lowsest 5 Bits are
#		used to avoid impossible values. If a property ID is followed
#		by an '!', the value is used as entered.
#		See https://szs.wiimm.de/doc/slot-id for infos about names.
#
#	 * le_flags
#		The flags are only allowed and mandatory, if option LE-FLAGS
#		(see above) is enabled. It is a bit field; combine flags by OR.
#		The flags are delivered to LE-CODE:
#		  LE$F_NEW    : Track is marked as »new track«.
#		  LE$F_HEAD   : Track is marked as »head of a group«. If such
#				track is selected by user, a random selection
#				of all group members is done.
#		  LE$F_GROUP  : Track is marked as »additional group member«
#				and is not added to a cup.
#
#	 * file_name
#		The SZS file name without extension as string. If no file name
#		is entered, the string 'slot_##' ('###' for LE-CODE) with a hex
#		slot number is used.
#
#	 * info
#		Optional information (no impact, maybe machine readable) as
#		string. Maybe empty. It is only used for references lists like
#		commands "wctct BMG" or "wctct TRACKS".
#
#	 * id
#		Optional identifier. While creating references, it is assumed,
#		that tracks with equal identifiers are equal. Empty strings
#		and a single "-" means: no identifier available.
#
#  * Hidden track definition:
#	'H' music_id ';' property_id ';' file_name ';' info ';' id
#	'H' music_id ';' property_id ';' le_flags ';' file_name ';' info ';' id
#
#	Like 'T', but hidden:
#	Define a new track, that doesn't appear in any cup. So it is hidden.
#	A hidden track can be defined at any place in this section and doesn't
#	touch any cup or the cup order.
#
#  * Battle arena definition:
#	'A' music_id ';' property_id ';' file_name ';' info ';' id
#	'A' music_id ';' property_id ';' le_flags ';' file_name ';' info ';' id
#
#	Like 'T', but for battle arenas:
#	Overwrite a battle arena slot identified by 'property_id'.
