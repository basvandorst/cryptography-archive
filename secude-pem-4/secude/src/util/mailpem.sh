#!/bin/sh
set -e
pse=${1-".pse"}
cp $HOME/.xpiperc.$pse $HOME/.xpiperc
xpipe -label "PEM ($pse)" -Wl "PEM ($pse)" -WL "PEM ($pse)"
cp $HOME/.xpiperc $HOME/.xpiperc.$pse
