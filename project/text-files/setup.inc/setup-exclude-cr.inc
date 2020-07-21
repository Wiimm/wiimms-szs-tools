
#------------------------------------------------------------------------------

[exclude-pattern]

# Files and directories in this section are ignored by the CREATE command.
# The include sections takes precedence over the exclude sections.

# This section expects file patterns with wildcards.
# Here we exclude all original, backup and temp files:

*.orig
*.bak
*.tmp
*.temp

# Exclude all *.d directories. To enable it, remove "# " at the beginning.
# *.d

# Exclude all PNG files. The intention is to allow the creation of new PNG
# images and mipmap files without any need to edit this setup file. All
# extracted PNG files are automatically listed in the [exclude] section.
# So the exclusion of all PNG is not really needed, but may help. The second
# (disabled) line is an alternative to exclude all mipmap PNG files.

*.png
# *.mm#.png

#------------------------------------------------------------------------------

[exclude]

# Files and directories in this section are ignored by the CREATE command.
# This second exclude section accepts only complete file names and none of
# characters have any special meaning.

