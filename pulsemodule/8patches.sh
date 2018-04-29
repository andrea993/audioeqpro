#!/bin/bash

wget https://patchwork.freedesktop.org/patch/216000/raw/ -O patch1
wget https://patchwork.freedesktop.org/patch/216003/raw/ -O patch2
wget https://patchwork.freedesktop.org/patch/216001/raw/ -O patch3
wget https://patchwork.freedesktop.org/patch/216002/raw/ -O patch4
wget https://patchwork.freedesktop.org/patch/216004/raw/ -O patch5
wget https://patchwork.freedesktop.org/patch/216007/raw/ -O patch6
wget https://patchwork.freedesktop.org/patch/216006/raw/ -O patch7
wget https://patchwork.freedesktop.org/patch/216005/raw/ -O patch8

patch -p1 < patch1
patch -p1 < patch2
patch -p1 < patch3
patch -p1 < patch4
patch -p1 < patch5
patch -p1 < patch6
patch -p1 < patch7
patch -p1 < patch8
