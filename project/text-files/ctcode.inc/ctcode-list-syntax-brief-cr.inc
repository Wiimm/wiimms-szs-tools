
# Syntax:
#  '%' name = value
#		: Define an option.
#  'N' mode	: Define mode for Nintendos original tracks:
#			N$NONE | N$HIDE | N$SHOW | N$SWAP
#			OR it with N$F_HEX and/or N$F_WII and/or N$F_<COLOR>
#  'C' name	: Enter a new cup and define its name.
#  'S' list	: Predefine track slots for the next tracks.
#  'X' list	: Mark slots as 'unusable'.
#  'T' music_id; property_id; file_name; [info;] [id]
#  'T' music_id; property_id; le_flags; file_name; [info;] [id]
#		: Define a racing track, See https://szs.wiimm.de/doc/slot-id
#		  for 'music_id' and 'property_id'.
#  'H' music_id; property_id; file_name; [info;] [id]
#  'H' music_id; property_id; le_flags; file_name; [info;] [id]
#		: Define a hidden track, that doesn't appear in any cup.
#  'A' music_id; property_id; file_name; [info;] [id]
#  'A' music_id; property_id; le_flags; file_name; [info;] [id]
#		: Define a battle arena.
