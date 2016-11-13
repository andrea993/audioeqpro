#!/bin/sh
journalctl -t pulseaudio | grep module-test.c
