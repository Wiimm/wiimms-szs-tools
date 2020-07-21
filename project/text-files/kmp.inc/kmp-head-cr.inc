#KMP  <<<  The first 4 characters '#KMP' are the magic for a KMP text file.
#     <<<  Don't remove them!
#
###############################################################################
#
# This is a textual representation of a KMP file used in Nintendos "Mario Kart
# Wii". "Wiimms SZS Tools" can convert binary (raw) and text KMP files in both
# directions. The text parser supports variables, C like expressions and nested
# IF-THEN-ELSE and LOOP structures.
#
# Info about the general parser syntax and semantics:
#   * https://szs.wiimm.de/doc/syntax
#
# Info about the KMP text syntax and semantics:
#   * https://szs.wiimm.de/doc/kmp/syntax
#
# Reference list of KMP parser functions:
#   * https://szs.wiimm.de/doc/kmp/func
#
# Info about the KMP file format:
#   * https://szs.wiimm.de/r/wiki/KMP
#
# Hint:
#   Set one of the options --no-header (-H) or --brief (-B) while decoding
#   to suppress this general description. The --brief option will also disable
#   info parts in the different sections. If --brief is set twice, the output
#   of the unneded sections CKPH, ENPH and ITPH is completly suppressed.
#
