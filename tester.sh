#!/bin/sh

set -v
set -u
prog=./projname

# sourceforge
$prog nagios
echo $?

# pypi
$prog setuptools
echo $?

# google code
$prog googleappengine
echo $?

$prog sadkfsaldh2jh43j3htr2j3h
echo $?
