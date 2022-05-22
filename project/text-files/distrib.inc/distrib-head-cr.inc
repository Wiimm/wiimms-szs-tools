#DISTRIB

# IMPORTANT:
#   Every parameter is copied to the next version of this file. For unknown
#   parameters the new section "User defined parameters" is created.

# Here you can find help:
#   https://szs.wiimm.de/cmd/distribution#desc
#   https://wiki.tockdom.com/wiki/Wiimm%%27s_Custom_Archive/Distribution


	##############################################
	###                                        ###
	###   Upload the final version of this     ###
	###   file to https://ct.wiimm.de/upload   ###
	###                                        ###
	##############################################


#------------------------------------------------------------------------------
#			Comments for personal use
#------------------------------------------------------------------------------

# These first comments are for personal use and will not imported to ct.wiimm.de
# and never visible for other persons. Use it as reminder for yourself.

@COMMENT1 = %s
@COMMENT2 = %s
@COMMENT3 = %s
@COMMENT4 = %s
@COMMENT5 = %s

#------------------------------------------------------------------------------
#			Identify yourself! (private section)
#------------------------------------------------------------------------------

# The next parameters are used to identify you and to contact you on problems.
# All data of this section will never be visible for other persons. It's no
# problem to leave some or all parameters empty.

# A wish for an user name @ ct.wiimm.de, if it ever supports a login.
@USER-CT-WIIMM	= %s

# Your current user name @ Wiimmfi (same as @ wii-hombrew.com).
@USER-WIIMMFI	= %s

# Your current user name @ CT-Wiiki (wiki.tockdom.com).
@USER-CT-WIIKI	= %s

# Other user names, format: service=name
@USER-MISC	= %s

# A space separated list of mail addresses:
@MAIL		= %s

# A note addressed to Wiimm
@NOTE-FOR-WIMMM	= %s

#------------------------------------------------------------------------------
#			Distribution parameters
#------------------------------------------------------------------------------

# Edit the following distribution parameters
# All parameters will be visible at ct.wiimm.de

# Identification of the distribution and its version.
# These first parameters are required!
@NAME		= %s
@VERSION	= %s
@AUTHORS	= %s

# Date of first release in format yyyy-mm-dd
@RELEASE-DATE	= %s

# Additional keywords for the search engine (optional).
# The 4 parameters above are automatically scanned for keywords.
@KEYWORDS	= %s

# ID or UUID of predecessor
@PREDECESSOR	= %s

# Additional infos (optional)
@WIIMMFI-REGION	= %s
@INFO-TEXT	= %s
@INFO-URL	= %s

#------------------------------------------------------------------------------
#		Settings for ct.wiimm.de => edit them
#------------------------------------------------------------------------------

# DISPLAY-MODE decides, how this distribution is displayed.
#   0: Hide the distribution completley (like not defined).
#   1: Hide the distribution, but allow access to overview page by UUID.
#   2: Like 1 + List the distribution name without a link at track info.
#   3: Distribution is fully visible and reachable by links.
# Once imported and mode>0, you can access the page with the following
# private URL: https://ct.wiimm.de/dis/%s
@DISPLAY-MODE	= %d

# DATABASE-NAME decides, which tracknames are used for the distribution list.
#   0: Use the names of this file.
#   1: Use the global names of the track archive. If a track is unknown by
#      the database, then fall back to step 0 and use the name of this file.
@DATABASE-NAME	= %d

# VIEW-COMMENT decides, how the comments in square brackts [] of track names
# are printed at the distribution page.
#   0: Hide the name part beginning from the first "[".
#   1: Show complete name including the [...] part.
@VIEW-COMMENT	= %d

# The comments in square brackts [] of track names are read as comma separated
# list of attributes. The attributes 'new', 'again', 'fill' and 'update' (or shorter
# 'upd') are imported to ct.wiimm.de.
# Each of the next 4 parameters enables a common status column in the track
# list. Maximal one enabled status attribut is printed. 'new' has the highest
# priority and 'update' the lowest. The options are enabled by value '1'.
@ENABLE-NEW	= %d
@ENABLE-AGAIN	= %d
@ENABLE-FILL	= %d
@ENABLE-UPDATE	= %d

# If enabled, boost tracks are highlighted. For tracks known by the database,
# the internal boost status is used. Unknown tracks are marked as boost, if
# either the attribute 'boost' exists, or the first 5 characters of the track
# name are 'boost'. The option is enabled by value '1'.
@ENABLE-BOOST	= %d

#------------------------------------------------------------------------------
#		Automatically created parameters => DO NOT EDIT!
#------------------------------------------------------------------------------

# The UUID (Universally Unique IDentifier) identifies the distribution.
#  => Do not change it under any circumstances!
#  => And don't copy it from another distribution!
@UUID		= %s

# wszst version that wrote this file
@WSZST-VERSION	= %s
@WSZST-REVISION	= %u

# Timestamps for this file
@FIRST-CREATION	= %s
@LAST-UPDATE	= %s
