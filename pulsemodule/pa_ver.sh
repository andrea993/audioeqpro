#!/bin/sh

if ! $(which pulseaudio) --version | awk '{ print $2 }' 2>/dev/null; then
	echo "null-pulseaudio-version"
	exit 2
fi

