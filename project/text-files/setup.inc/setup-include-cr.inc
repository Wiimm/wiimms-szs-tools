
###############################################################################
#
# Some general rules for the next sections:
#   All pathes are relative to the current base directory (directory that
#   contains the file wszst-setup.txt). Prefix at least pathes that starts
#   not with a letter, digit, point, star or underscore with "./".
#
#------------------------------------------------------------------------------

[include-pattern]

# All files and directories start with a . are hidden (unix like) and excluded
# automatically. But filenames in one of the include sections are included
# independent of this point. Also the include sections takes precedence over
# the exclude sections.

# This section expects file patterns with wildcards.

#------------------------------------------------------------------------------

[include]

# This second include section accpepts only complete file names and none of
# characters have any special meaning.

