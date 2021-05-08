#!/usr/bin/env bash

    #####################################################################
    ##                    _______ _______ _______                      ##
    ##                   |  ___  |____   |  ___  |                     ##
    ##                   | |   |_|    / /| |   |_|                     ##
    ##                   | |_____    / / | |_____                      ##
    ##                   |_____  |  / /  |_____  |                     ##
    ##                    _    | | / /    _    | |                     ##
    ##                   | |___| |/ /____| |___| |                     ##
    ##                   |_______|_______|_______|                     ##
    ##                                                                 ##
    ##                        Wiimms SZS Tools                         ##
    ##                      http://szs.wiimm.de/                       ##
    ##                                                                 ##
    #####################################################################
    ##                                                                 ##
    ##   This file is part of the SZS project.                         ##
    ##   Visit http://szs.wiimm.de/ for project details and sources.   ##
    ##                                                                 ##
    ##   Copyright (c) 2011-2019 by Dirk Clemens <wiimm@wiimm.de>      ##
    ##                                                                 ##
    #####################################################################

#------------------------------------------------------------------------------
# sudo?

try_sudo=1
if [[ $1 = --no-sudo ]]
then
    try_sudo=0
    shift
else
    uid="$(id -u 2>/dev/null)" || try_sudo=0
    [[ $uid = 0 ]] && try_sudo=0
fi

if ((try_sudo))
then
    echo "*** need root privileges to install => try sudo ***"
    sudo "$0" --no-sudo "$@"
    exit $?
fi

#------------------------------------------------------------------------------
# settings

SYSTEM="@@SYSTEM@@"
SYSTEM2="@@SYSTEM2@@"

BASE_PATH="@@INSTALL-PATH@@"
BIN_PATH="$BASE_PATH/bin"
SHARE_PATH="$BASE_PATH/share/szs"

BIN_FILES="@@BIN-FILES@@"
SHARE_FILES="@@SHARE-FILES@@"

INST_FLAGS="-p"

#------------------------------------------------------------------------------
# scan config

if [[ -f install-config.txt ]]
then
    res_install=
    res_config=
    res_share=
    . <( ./wszst config --bash --install --config install-config.txt )
    echo "res_install=$res_install"
    echo "res_config=$res_config"
    echo "res_share=$res_share"
fi

#------------------------------------------------------------------------------
# make?

make=0
if [[ $1 = --make ]]
then
    # it's called from make
    make=1
    shift
fi

#------------------------------------------------------------------------------
echo "*** install binaries to $BIN_PATH"

for f in $BIN_FILES
do
    [[ -f bin/$f ]] || continue
    mkdir -p "$BIN_PATH"
    install $INST_FLAGS "bin/$f" "$BIN_PATH/$f"
done

[[ $SYSTEM = mac ]] && xattr -dr com.apple.quarantine "$BIN_PATH" || true

#------------------------------------------------------------------------------
echo "*** install share files to $SHARE_PATH"

mkdir -p "$SHARE_PATH/auto-add/effect"
chmod 775 "$SHARE_PATH/auto-add" "$SHARE_PATH/auto-add/effect"

for f in $SHARE_FILES
do
    install $INST_FLAGS -m 644 share/$f "$SHARE_PATH/$f"
done

#------------------------------------------------------------------------------

exit 0

