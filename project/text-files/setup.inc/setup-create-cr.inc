
#------------------------------------------------------------------------------

[create]

# Create (and overwrite without notice) the listed files before adding them to
# the new archive. The source directory is searched under the same filename
# with an appended ".d" or, if this fails, with an extension replaced by ".d".
# If a source directory does not exist, the job is ignored silently.
#   Example: For "my.brres" search directories "my.brres.d" or "my.d"
# The source directory itself is set on the exclude list and will not stored
# into the new archive. Creating can be controlled by the options:
#   --no-recurse  : Do not create archives recursively.
#   --no-encode   : Do not encode files before creating an archive.
#   --encode-img  : Encode all image files (TEX,TPL,BREFT/sub), overrides --no-encode.
#   --encode-all  : Encode all files and ignore timestamps.
#   --pt-dir      : Set `point directory' mode.
#   --u8          : Create an U8 archive.
#   --brres       : Create an BRRES archive.
#   --breff       : Create an BREFF archive.
#   --breft       : Create an BREFT archive.
#   --no-compress : Create an uncompressed file (U8 instead of SZS).
#   ...

