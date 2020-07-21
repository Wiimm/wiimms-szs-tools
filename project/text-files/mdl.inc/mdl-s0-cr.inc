# This section represents one draw list.
#
# A draw list is a sequence of u8, u16 (1 and 2 byte unsigned integers) and
# float (4 bytes) values divided into commands. For each command the first
# byte declares the length and the meaning of the command:
#
#   #1 : 1 byte, end of command list
#   #2 : 5 bytes, bone mapping
#   #3 : 4+N*6 bytes, weighting
#   #4 : 8 bytes, draw polygon
#   #5 : 5 bytes, indexing
#
# And these are the codings of the commands:
#   #1
#   #2,  u16:weight_index,  u16:parent:weight_index
#   #3,  u16:wight_id,  u8:N,  N*( u16:table_id,  float:weight )
#   #4,  u16:object_index,  u16:material_index,  u16:bone_index,  u8:unknown
#   #5,  u16:weight_id,  u16:weight_index

