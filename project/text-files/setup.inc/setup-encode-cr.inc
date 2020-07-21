
#------------------------------------------------------------------------------

[encode]

# Encode (and overwrite without notice) the listed files before adding them
# to the new archive. The source is searched under the same filename with the
# correct source extension added or, if this failed, a replaced extension.
#   Example: For "message.bmg" search files "message.bmg.txt" or "message.txt"
# The source file itself is set on the exclude list and will not stored into
# the new archive.
#
# The syntax of this file section is a little bit different as the other file
# sections. Each line begins with a comma separated list of keywords followed
# by an equal sign.
#   Syntax:  COMMA_SEPARATED_FORMAT_LIST = relative_filename
#   Example: BREFT,IA8,3 = ./files/rk_steam
# The first keyword is the file format. The following keywords describe sub
# formats. An additonal integer is a file type specific information:
#  -> For image files it is the number of images (main image + mipmaps).
#     A value of 0 means: Use the tools default (see option --n-mipmaps).
#
# A file is only encoded if the destination does not exist or if the source
# is younger than the destination. Encoding can be controlled by options
# --no-encode, --encode-img and --encode-all (short: --noenc --eimg --eall).

