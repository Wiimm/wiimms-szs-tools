
#FF#
###############################################################################

# This second part of section [SETUP] is an example to prepare a KMP source
# file for conditional encoding (compiling). The idea is, that the user
# defines variables with the "--const name=value" option while calling one of
# the tools 'wszst' or 'wkmpt' to create different KMP files.
#
# Set option --no-param (-P) to disable this parameter part.
#
# This example scans 7 parameters. All variables are not special for the KMP
# encoder and fully under user control: The user can remove or rename them or
# can add more parameters. But this values have predefined meanings:
#
#   slot   : Create a KMP for the entered slot.
#            The default value 0 means: Design for allmost all slots.
#            Use it with @if..@else..@endif to support special slots.
#
#   d      : Create a KMP file for a "_d" variant of a track.
#            Use it with @if..@else..@endif to skip objects.
#
#   arena  : If >0, then create a battle arena instead of a racing track.
#
#   test   : If  >0, then test mode is enabled.
#            If <=0, test mode and also all parameters below are disabled.
#
#   ocam   : If >0 and also test>0, then enable test of opening camera #
#            (endless looping until button 'A' is pressed). With the default
#            value 0 all opening scenes are looped. If test mode is disabled,
#            the cameras work as usual.
#
#   showrt : If set and also test>0, points of different sections ar shown as
#            item boxes and all standard item boxes are disabled. Set one or more
#            of the following bits to show the points of the section:
#              1 : ENPT
#              2 : ITPT
#              4 : JGPT
#              8 : CNPT
#             16 : MSPT
#             32 : POTI
#             -1 : enable all section (all bits set!)
#
#   showck : If set and also test>0, show check points as trees (or other
#            user defined objects). See section [CKPT] for details.
#
#   laps   : It controls the number of laps by changing the settinge of the
#            check point modes (CKPT/@AUTO-MODE). Allowed values are:
#              0: Make the race a short as possible.
#              1: Finish the race after 1 lap
#              3: Finish the race after 3 laps (default).
#              4: Never finish the race (unlimited).
#            Value 3 is used, if test mode is disabled.
#
