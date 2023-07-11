
#FF#
###############################################################################

[RITP]
# Sort order: %u

# Normally only the first next-link of KMP:ITPH is used for Bullet Bill and
# Red Shells (except when the shell has already targeted a player). In the
# case of equivalent routes, however, it would be desirable for one of the
# possible routes to always be selected at random. It is important to ensure
# that everyone uses the same route when racing online.
#
# And exactly this can be defined with this section. Therfore you define a
# list with rules. Each rule defines the index of the ITPH element, the number
# of next-links affected, and the type of modification. LE-CODE is very robust
# here and only accepts valid rules on existing elements.

# Rule members:
#   INDEX: Index of KMP:ITPH entry (0..255).
#   NN:    Number of next-links to shuffle (2..6).
#   MODE:  How to shuffle (see below).
#   PARAM: Optional parameter for some modes (see below).

# Modes:
#
#   RITP$OFF
#	Ignore this rule, but store it including PARAM.
#	This mode can be used to temporary deactivate a rule.
#
#   RITP$START
#	Select first next-link by random at race start. The selected next-link
#	must be defined (not -1 == not 0xff), otherwise ignore the rule.
#	PARAM is ignored.
#
# More modes are planned.

# Example:
# ITPH element #10 (zero based index) has defined 3 equivalent routes and 2
# additional routes by its next-links. At race start one of the first 3 links
# should be selected by random as standard route. The rule for this case is:
#
#	10 3 RITP$START
#
# This rule is ignored by LE-CODE, if the addressed ITPH element does not
# exists or if the by random selected next-link is not defined.

