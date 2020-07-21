#!/usr/bin/env bash

#--- pre definitions: enter your local settings

# This is the directory with all message files (/Scene/UI)
SRC_UI_DIR="./ui"

# and this is the destination directory. If it is the same as the source
# than all files are replaced by the patched version.
DEST_UI_DIR="$SRC_UI_DIR"

# This is the path to the message file of above:
PATCH_FILE="bmg-patch.txt"

# Remove the the hash ('#') of the second line for fast operation.
# This makes the file about 25-30% larger but the operation is much faster 
FAST=
#FAST=--fast

#--- more automated setup

TEMP="$(mktemp -d)" || exit 1

#--- patch it in 3 steps: extract szs + patch bmg + create new szs

wszst extract --quiet "$SRC_UI_DIR/"*_?.szs --dest "$TEMP/%N.d"
wbmgt patch   --quiet "$TEMP/"*.d/message/*.bmg --patch "replace,$PATCH_FILE"
wszst create  --quiet --remove "$TEMP/"*.d --dest "$DEST_UI_DIR" $FAST

#--- clean up (remove temp dir)

rm -rf "$TEMP"
